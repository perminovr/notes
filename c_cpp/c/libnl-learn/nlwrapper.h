﻿#ifndef NLWRAPPER_H
#define NLWRAPPER_H

#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <net/if.h>
#include <arpa/inet.h>

#include <exception>
#include <cstdarg>
#include <functional>

#include <list>

#include <netlink/netlink.h>
#include <netlink/socket.h>
#include <netlink/cache.h>
#include <netlink/addr.h>
#include <netlink/route/link.h>
#include <netlink/route/addr.h>
#include <netlink/route/nexthop.h>
#include <netlink/route/route.h>
#include <netlink/cli/utils.h>
#include <netlink/cli/addr.h>
#include <netlink/cli/route.h>

extern "C" {
	typedef struct nl_object* nl_object_t;
    typedef struct nl_sock* nl_sock_t;
    typedef struct nl_cache* nl_cache_t;
    typedef struct nl_addr* nl_addr_t;
    typedef struct rtnl_link* rtnl_link_t;
    typedef struct rtnl_addr* rtnl_addr_t;
    typedef struct rtnl_nexthop* rtnl_nexthop_t;
    typedef struct rtnl_route* rtnl_route_t;
    typedef struct nl_msg* nl_msg_t;
    typedef struct nlmsghdr* nlmsghdr_t;

    typedef struct rtattr rtattr_t;
    typedef struct ifinfomsg ifinfomsg_t;
    typedef struct ifaddrmsg ifaddrmsg_t;
    typedef struct rtmsg rtmsg_t;
}

class NlSock;
class NlCache;
class NlAddr;
class RtNlLink;
class RtNlAddr;
class RtNlNextHop;
class RtNlRoute;

extern int ifaceName2i(NlCache &cache, const char *name);
extern char *i2ifaceName(NlCache &cache, int index, char *buf, size_t sz);
extern uint8_t maskToPrefix(const char *trgMask);
extern uint32_t getBroadcast(const char *ip, const char *mask);
extern int prefixToMask(int prefix, char *buf);

class NlSock {
public:
	NlSock() {
		data = nl_socket_alloc();
		if (data) {
			error = nl_connect(data, NETLINK_ROUTE);
		} else { error = -ENOMEM; }
	}
	~NlSock() { nl_socket_free(data); }
	//
	nl_sock_t &operator*() { return data; }
	int error;
	nl_sock_t data;
};

class NlEventSock {
public:
	typedef int (*cb_func)(nl_msg_t msg, void *arg);
    NlEventSock(cb_func cb, void *arg) {
        data = nl_socket_alloc();
        if (data) {
            nl_socket_disable_seq_check(data);
            nl_socket_modify_cb(data, NL_CB_VALID, NL_CB_CUSTOM, cb, arg);
            error = nl_connect(data, NETLINK_ROUTE);
            nl_socket_set_nonblocking(data); // otherwise it is impossible to leave from thread
        } else { error = -ENOMEM; }
    }
    ~NlEventSock() { stopLoop(); nl_socket_free(data); }
    int addMembership(int grp, ...) {
        int ret = error;
        if (data) {
            va_list argptr;
            va_start(argptr, grp);
            ret = nl_socket_add_memberships(data, grp, argptr);
            va_end(argptr);
        }
		return ret;
    }
    void loop() {
        running = true;
        while (running) {
            nl_recvmsgs_default(data);
            usleep(50000);
        }
    }
    void stopLoop() {
        running = false;
    }
    //
    int error;
    nl_sock_t data;
    bool running;
};

class NlCache {
public:
	NlCache(NlSock &sk) {
		data = nullptr;
		this->sk = &sk;
        if (*sk) {
            error = rtnl_link_alloc_cache(*sk, AF_UNSPEC, &data);
        } else { error = -EINVAL; }
	}
	~NlCache() { nl_cache_free(data); }
	nl_cache_t &operator*() { return data; }
	//
	int error;
	NlSock *sk;
	nl_cache_t data;
};

class NlAddr {
public:
	NlAddr() {
		error = 0;
		data = nl_addr_alloc(16);
		if (!data) { error = -ENOMEM; }
	}
	NlAddr(const char *ip, const char *mask = 0, int family = AF_INET) {
		error = 0;
		if (!ip || !strlen(ip)) ip = "default";
		error = nl_addr_parse(ip, family, &data);
		if (error == 0 && mask) {
			nl_addr_set_prefixlen(data, maskToPrefix(mask));
		}
	}
	NlAddr(uint32_t ip, const char *mask = 0, int family = AF_INET) {
		error = 0;
		data = nl_addr_alloc(16);
		if (data) {
			nl_addr_set_family(data, family);
			error = nl_addr_set_binary_addr(data, &ip, 4);
			if (error == 0 && mask) {
				nl_addr_set_prefixlen(data, maskToPrefix(mask));
			}
		} else { error = -ENOMEM; }
	}
	NlAddr(RtNlLink &link);
	char *toString(char *buf, size_t sz) {
        if (!buf) return nullptr;
		return nl_addr2str(data, buf, sz);
	}
    ~NlAddr() { if (data) nl_addr_put(data); }
	nl_addr_t &operator*() { return data; }
	//
	int error;
	nl_addr_t data;
};

class RtNlBase {
public:
	RtNlBase() {
		error = 0;
		sk = nullptr;
		cache = nullptr;
	}
	RtNlBase(NlSock &sk) {
		error = 0;
		setSock(sk);
	}
	void setSock(NlSock &sk) {
		this->sk = &sk;
        if (*sk) {
            this->cache = new NlCache(sk);
            if (this->cache) { error = this->cache->error; }
            else { error = -ENOMEM; }
        } else { error = -EINVAL; }
	}
	virtual ~RtNlBase() { delete this->cache; }
	int error;
	NlSock *sk;
	NlCache *cache;
};

class RtNlLink : public RtNlBase {
public:
	RtNlLink() : RtNlBase() {
        needFree = false;
		data = rtnl_link_alloc();
		if (!data) { error = -ENOMEM; }
        else { needFree = true; }
	}
	RtNlLink(NlSock &sk) : RtNlBase(sk) {
        needFree = false;
		data = rtnl_link_alloc();
		if (!data) { error = -ENOMEM; }
        else { needFree = true; }
	}
	RtNlLink(NlSock &sk, int ifaceIdx) : RtNlBase(sk) {
        needFree = false;
		if (cache) {
			data = rtnl_link_get(cache->data, ifaceIdx);
			if (!data) { error = -ENOENT; }			
		}
	}
	RtNlLink(NlSock &sk, const char *iface) : RtNlBase(sk) {
        needFree = false;
		if (cache) {
			if (iface) {
				data = rtnl_link_get_by_name(cache->data, iface);
                if (!data) { error = -ENOENT; }
			} else error = -EINVAL;
		}
	}
	virtual ~RtNlLink() { if (needFree) rtnl_link_put(data); }
	unsigned int flags() {
		if (data) { return rtnl_link_get_flags(data); }
		else { return 0; }
	}
	int setFlags(unsigned int flags) {
		if (this->sk) {
			if (this->data) {
				RtNlLink changes;
                if (*changes) {
                    rtnl_link_set_flags(*changes, flags);
                    return rtnl_link_change(this->sk->data, this->data, *changes, 0);
                } else return -changes.error;
			} else return -ENOENT;
		} else return -ENOTSOCK;
	}
	int up() {
		unsigned int flags = this->flags();
		if ( !(flags & IFF_UP) )
			return setFlags(IFF_UP);
		else return 0;
	}
	int down() {
		unsigned int flags = this->flags();
		if ( (flags & IFF_UP) )
			return setFlags(flags & ~IFF_UP);
		else return 0;
	}
	int promisc(bool enable) {
		unsigned int flags = this->flags();
		if ( !(flags & IFF_PROMISC) && enable )
			return setFlags(IFF_PROMISC);
		else if ( (flags & IFF_PROMISC) && !enable )
			return setFlags(flags & ~IFF_PROMISC);
		else return 0;
	}
	rtnl_link_t &operator*() { return data; }
	//
	rtnl_link_t data;
	bool needFree;
};

class RtNlAddr : public RtNlBase {
public:
	RtNlAddr() : RtNlBase() {
		data = rtnl_addr_alloc();
		if (!data) { error = -ENOMEM; }
	}
	RtNlAddr(NlSock &sk) : RtNlBase(sk) {
		data = rtnl_addr_alloc();
		if (!data) { error = -ENOMEM; }
	}
	// RtNlAddr(NlSock &sk, int ifaceIdx, NlAddr &addr) : RtNlBase(sk) {
	// 	if (cache) {
	// 		data = rtnl_addr_get(cache->data, ifaceIdx, *addr);
	// 		if (!data) { error = -ENOENT; }
	// 	}
	// }
	// RtNlAddr(NlSock &sk, const char *iface, NlAddr &addr) : RtNlBase(sk) {
	// 	if (cache) {
	// 		if (iface) {
	// 			data = rtnl_addr_get(cache->data, ifaceName2i(*cache, iface), *addr);
	// 			if (!data) { error = -ENOENT; }
	// 		} else error = -EINVAL;
	// 	}
	// }
	virtual ~RtNlAddr() { rtnl_addr_put(data); }
	static int delByLabel(NlSock &sk, const char *label) {
        if (*sk && label) {
			struct DelByLabelArg {
				nl_sock_t sk;
				const char *label;
			};
			DelByLabelArg arg = {
				.sk = *sk,
				.label = label
			};
            nl_cache_t cache = nl_cli_addr_alloc_cache(*sk);
			nl_cache_foreach(cache, [](nl_object_t obj, void *p) {
				DelByLabelArg *arg = (DelByLabelArg *)p;
				rtnl_addr_t addr = (rtnl_addr_t)obj;
				char *l = rtnl_addr_get_label(addr);
				if ( strstr(l, arg->label) != 0 )
					rtnl_addr_delete(arg->sk, addr, 0);
			}, &arg);
			nl_cache_free(cache);
			return 0;
        } else { return -EINVAL; }
	}
    int setIfaceIdx(int ifaceIdx) {
        if (data) {
            if (ifaceIdx > 0) {
                rtnl_addr_set_ifindex(data, ifaceIdx);
                return 0;
            } else return -EINVAL;
        } else return error;
	}
	int setIface(const char *iface) {
        if (cache) {
			if (iface) {
                if (data) {
                    rtnl_addr_set_ifindex(data, ifaceName2i(*cache, iface));
                    return 0;
                } else return error;
			} else return -EINVAL;
		} else return -ENOTSOCK;
	}
	int setLabel(const char *label, const char *iface) {
        if (cache) {
			if (label && iface) {
                if (data) {
					char buf[32];
					sprintf(buf, "%s:%s", label, iface);
					if (strlen(buf) < 16) {
						return rtnl_addr_set_label(data, buf);
					} else return -EINVAL;
                } else return error;
			} else return -EINVAL;
		} else return -ENOTSOCK;
	}
	int setLabel(const char *label, int iface) {
        if (cache) {
			if (data) {
				char buf[16];
				const char *p = i2ifaceName(*cache, iface, buf, 16);
				if (p) {
                    return setLabel(label, p);
				} else return -EINVAL;
			} else return error;
		} else return -ENOTSOCK;
	}
	int setAddress(const char *ip, int family = AF_INET) {
		if (ip) {
			NlAddr nl_addr(ip, 0, family);
            if (data) {
                return error = rtnl_addr_set_local(data, *nl_addr);
            } else return error;
		} else return -EINVAL;
	}
	int setMask(const char *mask) {
		if (mask) {
            if (data) {
                rtnl_addr_set_prefixlen(data, maskToPrefix(mask));
                return 0;
            } else return error;
		} else return -EINVAL;
	}
	int setBroadcast(const char *ip, const char *mask, int family = AF_INET) {
        if (ip && mask) {
            if (data) {
                uint32_t brd = getBroadcast(ip, mask);
                NlAddr nl_addr(brd, 0, family);
                return rtnl_addr_set_broadcast(data, *nl_addr);
            } else return error;
		} else return -EINVAL;
	}
	int add() {
        if (this->sk && this->sk->data) {
            if (data) {
                return rtnl_addr_add(this->sk->data, data, NLM_F_REPLACE);
            } else return error;
        } else return -ENOTSOCK;
	}
	int del() {
        if (this->sk && this->sk->data) {
            if (data) {
                int res = rtnl_addr_delete(this->sk->data, data, 0);
                return (res == -ENODEV)? 0 : res;
            } else return error;
        } else return -ENOTSOCK;
	}
	int addComplete(int ifaceIdx, const char *ip, const char *mask, const char *label = nullptr, int family = AF_INET) {
        if (this->sk && this->sk->data) {
            int ret;
            if ( (ret = setIfaceIdx(ifaceIdx)) != 0 )
                return ret;
			if (label && (ret = setLabel(label, ifaceIdx)) != 0)
                return ret;
            if ( (ret = setAddress(ip, family)) != 0 )
                return ret;
            if ( (ret = setMask(mask)) != 0 )
                return ret;
//            if ( (ret = setBroadcast(ip, mask, family)) != 0 )
//                return ret;
            return add();
		} else return -ENOTSOCK;
	}
	int addComplete(const char *iface, const char *ip, const char *mask, const char *label = nullptr, int family = AF_INET) {
        if (this->cache) {
			return addComplete(ifaceName2i(*cache, iface), ip, mask, label, family);
        } else return -ENOTSOCK;
	}
	int delComplete(int ifaceIdx, const char *ip, const char *mask, int family = AF_INET) {
        if (this->sk && this->sk->data) {
            int ret;
            if ( (ret = setIfaceIdx(ifaceIdx)) != 0 )
                return ret;
            if ( (ret = setAddress(ip, family)) != 0 )
                return ret;
            if ( (ret = setMask(mask)) != 0 )
                return ret;
            return del();
		} else return -ENOTSOCK;
	}
	int delComplete(const char *iface, const char *ip, const char *mask, int family = AF_INET) {
        if (this->cache) {
			return delComplete(ifaceName2i(*cache, iface), ip, mask, family);
        } else return -ENOTSOCK;
	}
	unsigned int flags() {
		if (data) { return rtnl_addr_get_flags(data); }
		else { return 0; }
	}
	int setFlags(unsigned int flags) {
        if (this->sk && this->sk->data) {
            if (data) {
                rtnl_addr_set_flags(data, flags);
                return 0;
            } else return error;
		} else return -ENOTSOCK;
	}
	rtnl_addr_t &operator*() { return data; }
	//
	rtnl_addr_t data;
};

class RtNlNextHop : public RtNlBase {
public:
	RtNlNextHop() : RtNlBase() {
		data = rtnl_route_nh_alloc();
		if (!data) { error = -ENOMEM; }
	}
	RtNlNextHop(NlSock &sk) : RtNlBase(sk) {
		data = rtnl_route_nh_alloc();
		if (!data) { error = -ENOMEM; }
	}
	virtual ~RtNlNextHop() { rtnl_route_nh_free(data); }
    int setIfaceIdx(int ifaceIdx) {
        if (data) {
            if (ifaceIdx > 0) {
                rtnl_route_nh_set_ifindex(data, ifaceIdx);
                return 0;
            } else return -EINVAL;
        } else return error;
	}
	int setIface(const char *iface) {
		if (this->cache) {
			if (iface) {
                if (data) {
                    rtnl_route_nh_set_ifindex(data, ifaceName2i(*cache, iface));
                    return 0;
                } else return error;
			} else return -EINVAL;
		} else return -ENOTSOCK;
	}
	int setGateway(const char *gw, int family = AF_INET) {
		if (gw) {
            if (data) {
                NlAddr nl_addr(gw, 0, family);
                if (*nl_addr) {
                    rtnl_route_nh_set_gateway(data, *nl_addr);
                    return 0;
                } else return nl_addr.error;
            } else return error;
		} else return -EINVAL;
	}
	rtnl_nexthop_t &operator*() { return data; }
	//
	rtnl_nexthop_t data;
};

class RtNlRoute : public RtNlBase {
public:
	RtNlRoute() : RtNlBase() {
		data = rtnl_route_alloc();
		if (!data) { error = -ENOMEM; }
	}
	RtNlRoute(NlSock &sk) : RtNlBase(sk) {
		data = rtnl_route_alloc();
		if (!data) { error = -ENOMEM; }
	}
	virtual ~RtNlRoute() {
        if (data) {
            for (auto &x : nhlist) // libnl free owned nh, but nh free by self destructor
                rtnl_route_remove_nexthop(data, x);
            rtnl_route_put(data);
        }
	}
	static int delWithoutSrc(NlSock &sk) {
        if (*sk) {
            nl_cache_t cache = nullptr;
			rtnl_route_alloc_cache(*sk, AF_UNSPEC, 0, &cache);
			if (cache) {
				nl_cache_foreach(cache, [](nl_object_t obj, void *p) {
					nl_sock_t sk = (nl_sock_t)p;
					rtnl_route_t route = (rtnl_route_t)obj;
					uint32_t table = rtnl_route_get_table(route);
					uint8_t family = rtnl_route_get_family(route);
					if (table == RT_TABLE_MAIN && family == AF_INET) {
						nl_addr_t addr1 = rtnl_route_get_dst(route);
						nl_addr_t addr2 = rtnl_route_get_pref_src(route);
						char buf1[32];
						char buf2[32];
						char *ip1 = nl_addr2str(addr1, buf1, 32);
						char *ip2 = nl_addr2str(addr2, buf2, 32);
						printf("%s %s\n", ip1, ip2); // => 'none'
						// rtnl_route_delete(sk, route, 0); // todo
					}
				}, *sk);
				nl_cache_free(cache);
				return 0;
			} else { return -ENOMEM; }
        } else { return -EINVAL; }
	}
    int addNexthop(RtNlNextHop &nh) {
        if (*nh) {
            if (data) {
                nhlist.push_back(*nh);
                rtnl_route_add_nexthop(data, *nh);
                return 0;
            } else return error;
        } else return -EINVAL;
	}
	int setAddress(const char *destip, const char *destmask = 0, int family = AF_INET) {
        if (data) {
            NlAddr nl_addr(destip, destmask, family);
            if (*nl_addr) {
                return rtnl_route_set_dst(data, *nl_addr);
            } else return nl_addr.error;
        } else return error;
	}
    int setFlags(uint32_t flags) {
        if (data) {
            rtnl_route_set_flags(data, flags);
            return 0;
        } else return error;
	}
    int getFlags(uint32_t &flags) {
        if (data) {
            flags = rtnl_route_get_flags(data);
            return 0;
        } else return error;
	}
    int setPriority(uint32_t prio) {
        if (data) {
            rtnl_route_set_priority(data, prio);
            return 0;
        } else return error;
	}
    int getPriority(uint32_t &prio) {
        if (data) {
            prio = rtnl_route_get_priority(data);
            return 0;
        } else return error;
	}
	int add() {
        if (this->sk && this->sk->data) {
            if (data) {
                return rtnl_route_add(this->sk->data, data, NLM_F_REPLACE);
            } else return error;
        } else return -ENOTSOCK;
	}
	int del() {
        if (this->sk && this->sk->data) {
            if (data) {
                int res = rtnl_route_delete(this->sk->data, data, 0);
                return (res == -ENODEV)? 0 : res;
            } else return error;
        } else return -ENOTSOCK;
	}
	int addSimple(int ifaceIdx, const char *destip, const char *gw, const char *destmask = 0, int family = AF_INET) {
        if (this->sk && this->sk->data) {
            if (gw && strlen(gw)) {
                int ret;
                RtNlNextHop nh(*this->sk);
                if ( (ret = nh.setIfaceIdx(ifaceIdx)) != 0 )
                    return ret;
                if ( (ret = nh.setGateway(gw, family)) != 0 )
                    return ret;
                if ( (ret = addNexthop(nh)) != 0 )
                    return ret;
                if ( (ret = setAddress(destip, destmask, family)) != 0 )
                    return ret;
                return add();
            } else return -EINVAL;
		} else return -ENOTSOCK;
	}
	int addSimple(const char *iface, const char *destip, const char *gw, const char *destmask = 0, int family = AF_INET) {
		if (this->cache)
			return addSimple(ifaceName2i(*cache, iface), destip, gw, destmask, family);
		else return -ENOTSOCK;
	}
	int delSimple(int ifaceIdx, const char *destip, const char *gw, const char *destmask = 0, int family = AF_INET) {
        if (this->sk) {
            if (gw && strlen(gw)) {
                int ret;
                RtNlNextHop nh(*this->sk);
                if ( (ret = nh.setIfaceIdx(ifaceIdx)) != 0 )
                    return ret;
                if ( (ret = nh.setGateway(gw, family)) != 0 )
                    return ret;
                if ( (ret = addNexthop(nh)) != 0 )
                    return ret;
                if ( (ret = setAddress(destip, destmask, family)) != 0 )
                    return ret;
                return del();
            } else return -EINVAL;
		} else return -ENOTSOCK;
	}
	int delSimple(const char *iface, const char *destip, const char *gw, const char *destmask = 0, int family = AF_INET) {
		if (this->cache)
			return delSimple(ifaceName2i(*cache, iface), destip, gw, destmask, family);
		else return -ENOTSOCK;
	}
	rtnl_route_t &operator*() { return data; }
	//
	rtnl_route_t data;
	std::list <rtnl_nexthop_t> nhlist;
};

#endif // NLWRAPPER_H
