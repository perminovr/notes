
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <netlink/netlink.h>
#include <netlink/route/link.h>
#include <netlink/route/addr.h>
#include <netlink/route/nexthop.h>
#include <netlink/route/route.h>
#include <errno.h>
#include <linux/if_ether.h>

/*
https://www.infradead.org/~tgr/libnl/doc
https://www.infradead.org/~tgr/libnl/doc/api/index.html

modules:
libnl (core) - nl protocol provider
libnl-route - works with physical and virtual network devices, routing and more other
libnl-genl
libnl-idiag
libnl-nf
*/





static int prefixToMask(int prefix, char *buf)
{
    uint32_t mask = prefix ? ~0 << (32 - prefix) : 0;
	mask = htonl(mask);
	inet_ntop(AF_INET, &mask, buf, INET_ADDRSTRLEN);    
}

static uint8_t maskToPrefix(const char *trgMask)
{
	uint8_t ret;
	uint32_t prefix = 0;
	uint32_t mask = 0;
    if (trgMask) {
        if (strlen(trgMask) <= 2) {
            prefix = atoi(trgMask);
            mask = (0xFFFFFFFF << (32 - prefix)) & 0xFFFFFFFF;
            mask = htonl(mask);
        } else {
	        mask = ntohl(inet_addr(trgMask));
			for (int i = 31; i >= 0; --i) {
				if (mask & (1 << i)) {
					prefix++;
				} else {
					break;
				}
			}
        }
    }
	ret = (uint8_t)prefix;
	return ret;
}


static uint32_t getBroadcast(const char *ip, const char *mask)
{
	uint32_t ret = 0;
	if (ip && mask) {
		ret = htonl(inet_addr(ip));
		if (ret) {
			uint8_t prefix = maskToPrefix(mask);
			for (int i = 0; i < 32-prefix; ++i) {
				ret |= (1 << i);
			}
		}
		ret = htonl(ret);
	}
	return ret;
}



int iproadd(const char *iface, const char *destip, const char *mask, const char *via)
{
	int ret = -1;
	struct nl_sock *sk;
	struct nl_cache *cache;
	struct rtnl_link *link;
	struct rtnl_nexthop *nh;
	struct rtnl_route *route;

	if (!iface || !destip || !via) {
		printf("iproadd\n");
		return ret;
	}

	/* nl socket */
	sk = nl_socket_alloc();
	if (!sk) {
		printf("nl_socket_alloc\n");
		return ret;
	}

	/* connect to nl system */
	if (nl_connect(sk, NETLINK_ROUTE) != 0) {
		printf("nl_connect\n");
		goto exit_sk;
	}

	/* get pool of links (links == ifaces) */
	if (rtnl_link_alloc_cache(sk, AF_UNSPEC, &cache) != 0) {
		printf("rtnl_link_alloc_cache\n");
		goto exit_sk;
	}

	/* convert name to idx (rtnl_link_i2name) */
	int ifindex = rtnl_link_name2i(cache, iface);
	if (ifindex <= 0) {
		printf("rtnl_link_name2i\n");
		goto exit_cache;
	}

	/* get link from pool of links */
	link = rtnl_link_get(cache, ifindex);
	if (!link) {
		printf("rtnl_link_get\n");
		goto exit_cache;
	}

	/* route */
	route = rtnl_route_alloc();
	if (!route) {
		printf("rtnl_route_alloc\n");
		goto exit_link;
	}

	/* gateway */
	nh = rtnl_route_nh_alloc();
	if (!nh) {
		printf("rtnl_route_nh_alloc\n");
		goto exit_route;
	}
	
	/* set address */
	{
		struct nl_addr *local_addr;
		if (nl_addr_parse(destip, rtnl_route_get_family(route), &local_addr) != 0) {
			printf("nl_addr_parse\n");
			goto exit_nh;
		}

		/* set mask */
		if (mask) {
			nl_addr_set_prefixlen(local_addr, maskToPrefix(mask));
		}

		/* set route dest addr */
		if (rtnl_route_set_dst(route, local_addr) != 0) {
			printf("rtnl_route_set_dst\n");
			nl_addr_put(local_addr);
			goto exit_nh;
		}
		nl_addr_put(local_addr);
	}

	/* set nexthop */
	{
		/* set iface idx */
		rtnl_route_nh_set_ifindex(nh, ifindex);
			
		/* set gateway addr */
		{
			struct nl_addr *local_addr;
			if (nl_addr_parse(via, rtnl_route_get_family(route), &local_addr) != 0) {
				printf("nl_addr_parse\n");
				goto exit_nh;
			}

			rtnl_route_nh_set_gateway(nh, local_addr);
			nl_addr_put(local_addr);
		}

		/* add nh to route */
		rtnl_route_add_nexthop(route, nh);
	}

	/* add route */
	int error;
	if ((error=rtnl_route_add(sk, route, NLM_F_EXCL)) != 0) {
		printf("%s\n", nl_geterror(error));
		printf("rtnl_route_add\n");
		goto exit_nh;
	}

	/* delete route (via is unnecessary) */
	// system("ip ro");
	// puts("\n");
	// if (rtnl_route_delete(sk, route, 0) != 0) {
	// 	printf("rtnl_route_delete\n");
	// 	goto exit_nh;
	// }
	// system("ip ro");

	ret = 0;

	/* cleanup */
exit_nh:
	rtnl_route_nh_free(nh);
exit_route:
	rtnl_route_put(route);
exit_link:
	rtnl_link_put(link);
exit_cache:
	nl_cache_free(cache);
exit_sk:
	nl_socket_free(sk);
	return ret;
}





int iproadd2(const char *iface, const char *destip, const char *mask, const char *src)
{
	int ret = -1;
	struct nl_sock *sk;
	struct nl_cache *cache;
	struct rtnl_link *link;
	struct rtnl_nexthop *nh;
	struct rtnl_route *route;

	if (!iface || !destip || !src) {
		printf("iproadd2\n");
		return ret;
	}

	/* nl socket */
	sk = nl_socket_alloc();
	if (!sk) {
		printf("nl_socket_alloc\n");
		return ret;
	}

	/* connect to nl system */
	if (nl_connect(sk, NETLINK_ROUTE) != 0) {
		printf("nl_connect\n");
		goto exit_sk;
	}

	/* get pool of links (links == ifaces) */
	if (rtnl_link_alloc_cache(sk, AF_UNSPEC, &cache) != 0) {
		printf("rtnl_link_alloc_cache\n");
		goto exit_sk;
	}

	/* convert name to idx (rtnl_link_i2name) */
	int ifindex = rtnl_link_name2i(cache, iface);
	if (ifindex <= 0) {
		printf("rtnl_link_name2i\n");
		goto exit_cache;
	}

	/* get link from pool of links */
	link = rtnl_link_get(cache, ifindex);
	if (!link) {
		printf("rtnl_link_get\n");
		goto exit_cache;
	}

	/* route */
	route = rtnl_route_alloc();
	if (!route) {
		printf("rtnl_route_alloc\n");
		goto exit_link;
	}

	/* gateway */
	nh = rtnl_route_nh_alloc();
	if (!nh) {
		printf("rtnl_route_nh_alloc\n");
		goto exit_route;
	}
	
	/* set address */
	struct nl_addr *dst_addr;
	{
		if (nl_addr_parse(destip, AF_INET, &dst_addr) != 0) {
			printf("nl_addr_parse\n");
			goto exit_nh;
		}

		/* set mask */
		if (mask) {
			nl_addr_set_prefixlen(dst_addr, maskToPrefix(mask));
		}

		/* set route dest addr */
		if (rtnl_route_set_dst(route, dst_addr) != 0) {
			printf("rtnl_route_set_dst\n");
			nl_addr_put(dst_addr);
			goto exit_nh;
		}
	}

	/* set src */
	struct nl_addr *src_addr;
	{
		if (nl_addr_parse(src, AF_INET, &src_addr) != 0) {
			printf("nl_addr_parse\n");
			goto exit_nh;
		}

		/* set route dest addr */
		if (rtnl_route_set_pref_src(route, src_addr) != 0) {
			printf("rtnl_route_set_dst\n");
			nl_addr_put(src_addr);
			goto exit_nh;
		}
	}

	/* set nexthop */
	{
		/* set iface idx */
		rtnl_route_nh_set_ifindex(nh, ifindex);
			
		// /* set gateway addr */
		// {
		// 	struct nl_addr *local_addr;
		// 	if (nl_addr_parse(via, rtnl_route_get_family(route), &local_addr) != 0) {
		// 		printf("nl_addr_parse\n");
		// 		goto exit_nh;
		// 	}

		// 	rtnl_route_nh_set_gateway(nh, local_addr);
		// 	nl_addr_put(local_addr);
		// }

		/* add nh to route */
		rtnl_route_add_nexthop(route, nh);
	}

	/* add route */
	int error;
	if ((error=rtnl_route_add(sk, route, NLM_F_EXCL)) != 0) {
		printf("%s\n", nl_geterror(error));
		printf("rtnl_route_add\n");
		goto exit_nh;
	}

	/* delete route (via is unnecessary) */
	// system("ip ro");
	// puts("\n");
	// if (rtnl_route_delete(sk, route, 0) != 0) {
	// 	printf("rtnl_route_delete\n");
	// 	goto exit_nh;
	// }
	// system("ip ro");

	ret = 0;

	nl_addr_put(dst_addr);
	nl_addr_put(src_addr);

	/* cleanup */
exit_nh:
	rtnl_route_nh_free(nh);
exit_route:
	rtnl_route_put(route);
exit_link:
	rtnl_link_put(link);
exit_cache:
	nl_cache_free(cache);
exit_sk:
	nl_socket_free(sk);
	return ret;
}


int ipaddradd(const char *iface, const char *ip, const char *mask)
{
	int ret = -1;
	struct nl_sock *sk;
	struct rtnl_addr *addr;
	struct nl_cache *cache;
	struct rtnl_link *link;

	if (!iface || !ip || !mask) {
		printf("ipaddradd\n");
		return ret;
	}

	/* nl socket */
	sk = nl_socket_alloc();
	if (!sk) {
		printf("nl_socket_alloc\n");
		return ret;
	}

	/* connect to nl system */
	if (nl_connect(sk, NETLINK_ROUTE) != 0) {
		printf("nl_connect\n");
		goto exit_sk;
	}

	/* get pool of links (links == ifaces) */
	if (rtnl_link_alloc_cache(sk, AF_UNSPEC, &cache) != 0) {
		printf("rtnl_link_alloc_cache\n");
		goto exit_sk;
	}

	/* convert name to idx (rtnl_link_i2name) */
	int ifindex = rtnl_link_name2i(cache, iface);
	if (ifindex <= 0) {
		printf("rtnl_link_name2i\n");
		goto exit_cache;
	}

	/* get link from pool of links */
	link = rtnl_link_get(cache, ifindex);
	if (!link) {
		printf("rtnl_link_get\n");
		goto exit_cache;
	}

	/* work with link flags (std iface flags: IFF_* (net/if.h)) */
	/* see also rtnl_link_flags2str & rtnl_link_str2flags */
	{ 		
		unsigned int link_flags = rtnl_link_get_flags(link);
		/* if down -> up */
		if (!(link_flags & IFF_UP)) {
			struct rtnl_link *changes = rtnl_link_alloc();
			rtnl_link_set_flags(changes, IFF_UP);
			if (rtnl_link_change(sk, link, changes, 0) != 0) {
				printf("rtnl_link_change IFF_UP\n");
				rtnl_link_put(changes);
				goto exit_link;
			}
			rtnl_link_put(changes);
		}
		/* -promisc -> promisc */
		if (!(link_flags & IFF_PROMISC)) {
			struct rtnl_link *changes = rtnl_link_alloc();
			rtnl_link_set_flags(changes, IFF_PROMISC);
			if (rtnl_link_change(sk, link, changes, 0) != 0) {
				printf("rtnl_link_change IFF_PROMISC\n");
				rtnl_link_put(changes);
				goto exit_link;
			}
			rtnl_link_put(changes);
		}
	}

	/* address object */
	addr = rtnl_addr_alloc();
	if (!addr) {
		printf("rtnl_addr_alloc\n");
		goto exit_link;
	}
	/* set iface idx for address */
	rtnl_addr_set_ifindex(addr, ifindex);
	
	/* set address */
	{
		struct nl_addr *local_addr;
		if (nl_addr_parse(ip, rtnl_addr_get_family(addr), &local_addr) != 0) {
			printf("nl_addr_parse\n");
			goto exit_addr;
		}

		if (rtnl_addr_set_local(addr, local_addr) != 0) {
			printf("rtnl_addr_set_local\n");
			nl_addr_put(local_addr);
			goto exit_addr;
		}
		nl_addr_put(local_addr);
	}

	/* set mask */
	{
		rtnl_addr_set_prefixlen(addr, maskToPrefix(mask));
	}

	/* set broadcast */
	{
		struct nl_addr *local_addr = nl_addr_alloc(16);
		uint32_t brd = getBroadcast(ip, mask);
		if (nl_addr_set_binary_addr(local_addr, &brd, 4) != 0) {
			printf("nl_addr_set_binary_addr\n");
			nl_addr_put(local_addr);
			goto exit_addr;
		}
		nl_addr_set_family(local_addr, rtnl_addr_get_family(addr));
		if (rtnl_addr_set_broadcast(addr, local_addr) != 0) {
			printf("rtnl_addr_set_broadcast\n");
			nl_addr_put(local_addr);
			goto exit_addr;
		}
		nl_addr_put(local_addr);
	}

	/* add address */
	if (rtnl_addr_add(sk, addr, NLM_F_EXCL) != 0) {
		printf("rtnl_addr_add\n");
		goto exit_addr;
	}

	/* del address (brd is unnecessary) */
	// system("ip addr");
	// puts("\n");
	// if (rtnl_addr_delete(sk, addr, 0) != 0) {
	// 	printf("rtnl_addr_delete\n");
	// 	goto exit_addr;
	// }
	// system("ip addr");

	ret = 0;

	/* cleanup */
exit_addr:
	rtnl_addr_put(addr);
exit_link:
	rtnl_link_put(link);
exit_cache:
	nl_cache_free(cache);
exit_sk:
	nl_socket_free(sk);
	return ret;
}


int getMac(const char *iface)
{
	int ret = -1;
	struct nl_sock *sk;
	struct nl_cache *cache;
	struct rtnl_link *link;
	struct nl_addr *addr;

	if (!iface) {
		printf("getMac\n");
		return ret;
	}

	/* nl socket */
	sk = nl_socket_alloc();
	if (!sk) {
		printf("nl_socket_alloc\n");
		return ret;
	}

	/* connect to nl system */
	if (nl_connect(sk, NETLINK_ROUTE) != 0) {
		printf("nl_connect\n");
		goto exit_sk;
	}

	/* get pool of links (links == ifaces) */
	if (rtnl_link_alloc_cache(sk, AF_UNSPEC, &cache) != 0) {
		printf("rtnl_link_alloc_cache\n");
		goto exit_sk;
	}

	/* convert name to idx (rtnl_link_i2name) */
	int ifindex = rtnl_link_name2i(cache, iface);
	if (ifindex <= 0) {
		printf("rtnl_link_name2i\n");
		goto exit_cache;
	}

	/* get link from pool of links */
	link = rtnl_link_get(cache, ifindex);
	if (!link) {
		printf("rtnl_link_get\n");
		goto exit_cache;
	}

	addr = rtnl_link_get_addr(link);
	if (!addr) {
		printf("rtnl_link_get\n");
		goto exit_link;
	}

	printf("family %d\n", nl_addr_get_family(addr));

	char buf[32];
	char *res = nl_addr2str(addr, buf, 32);
	if (res) {
		printf("%s\n", res);
	}

exit_addr:	
	nl_addr_put(addr);
exit_link:
	//rtnl_link_put(link); // will be removed by nl_cache_free - del all objects
exit_cache:
	nl_cache_free(cache);
exit_sk:
	nl_socket_free(sk);
	return ret;
}


int setMac(const char *devName)
{
	int ret = -1;
	struct nl_sock *sk;
	struct nl_cache *cache;
	struct rtnl_link *link;
	struct rtnl_link *link_changes;
	struct nl_addr *addr;

	// 30:9c:23:6c:b2:3b <- real
	// 30:9c:23:6c:b2:3c

	char *buf = "30:9c:23:6c:b2:3b";
	char mac[ETH_ALEN];
	sscanf(buf, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", 
									&mac[0],&mac[1],&mac[2],&mac[3],&mac[4],&mac[5]);

	printf("%hhx:%hhx:%hhx:%hhx:%hhx:%hhx \n",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);

	if ( (sk = nl_socket_alloc()) ) {
		if (nl_connect(sk, NETLINK_ROUTE) == 0) {
			if (rtnl_link_alloc_cache(sk, AF_UNSPEC, &cache) == 0) {
				if ( (link = rtnl_link_get_by_name(cache, devName)) ) {
					if ( (addr = nl_addr_build(AF_LLC, mac, ETH_ALEN)) ) {
						if ( (link_changes = rtnl_link_alloc()) ) {
							rtnl_link_set_addr(link_changes, addr);
							if (rtnl_link_change (sk, link, link_changes, 0) == 0) {
								ret = 0;
								printf("setMac done\n");
							}
							rtnl_link_put(link_changes);
						}
						nl_addr_put(addr);
					}
					//rtnl_link_put(link);
				}
				nl_cache_free(cache);
			}
		}
		nl_socket_free(sk);
	}
	return ret;
}




/*
struct nl_msg {
	int 				nm_protocol 
	int 				nm_flags 
	struct sockaddr_nl 	nm_src 
	struct sockaddr_nl 	nm_dst 
	struct ucred 		nm_creds 
	struct nlmsghdr * 	nm_nlh 
	size_t 				nm_size 
	int 				nm_refcnt
};
http://man7.org/linux/man-pages/man7/rtnetlink.7.html
*/

#define GETRTA(p,size) (struct rtattr *)(((char*)p) + NLMSG_ALIGN(size))

static int linkevent_cb(struct nl_msg *msg, void *arg)
{
	struct nlmsghdr *hdr = nlmsg_hdr(msg);
	uint16_t type = hdr->nlmsg_type;

	if (type == RTM_NEWLINK || type == RTM_DELLINK) {
		struct ifinfomsg *ifi = (struct ifinfomsg *)NLMSG_DATA(hdr);
		struct rtattr *rth = GETRTA(ifi, sizeof(struct ifinfomsg));
		int rtl = NLMSG_PAYLOAD(hdr, sizeof(struct ifinfomsg));

		while (rtl && RTA_OK(rth, rtl)) {
			uint16_t rta_len = rth->rta_len;
			uint16_t rta_type = rth->rta_type;
			uint8_t *rta_data = RTA_DATA(rth);

			if (rta_type == IFLA_IFNAME) {
				printf("link %s: %s\n", 
					(ifi->ifi_flags & IFF_UP)? "up" : "down",
					(char *)rta_data);
				return 0;
			}

			rth = RTA_NEXT(rth, rtl);
		}
	}

	if (type == RTM_NEWADDR || type == RTM_DELADDR) {
		struct ifaddrmsg *ifa = (struct ifaddrmsg *)NLMSG_DATA(hdr);
		struct rtattr *rth = GETRTA(ifa, sizeof(struct ifaddrmsg));
		int rtl = NLMSG_PAYLOAD(hdr, sizeof(struct ifaddrmsg));

		const char *iface = 0;
		const char *ip = 0;
		while (rtl && RTA_OK(rth, rtl)) {
			uint16_t rta_len = rth->rta_len;
			uint16_t rta_type = rth->rta_type;
			uint8_t *rta_data = RTA_DATA(rth);
		
			if (rta_type == IFA_LABEL) {
				iface = (const char *)rta_data;
			}
			
			if (rta_type == IFA_LOCAL) {
				ip = (const char *)rta_data;
			}

			if (iface && ip) {
				char ipstr[INET_ADDRSTRLEN];
				inet_ntop(AF_INET, ip, ipstr, INET_ADDRSTRLEN);
				printf("%s ip(%s:%u): %s/%hhu ; flags: %hhu\n", 
					(type == RTM_NEWADDR)? "add" : "del",
					iface, ifa->ifa_index, 
					(char *)ipstr, ifa->ifa_prefixlen, ifa->ifa_flags);
				return 0;
			}

			rth = RTA_NEXT(rth, rtl);
		}
	}

	if (type == RTM_NEWROUTE || type == RTM_DELROUTE) {
		struct rtmsg *rtm = (struct rtmsg *)NLMSG_DATA(hdr);
		struct rtattr *rth = GETRTA(rtm, sizeof(struct rtmsg));
		int rtl = NLMSG_PAYLOAD(hdr, sizeof(struct rtmsg));

		int iface = 0;
		const char *ip = 0;
		const char *gw = 0;
		while (rtl && RTA_OK(rth, rtl)) {
			uint16_t rta_len = rth->rta_len;
			uint16_t rta_type = rth->rta_type;
			uint8_t *rta_data = RTA_DATA(rth);
		
			if (rta_type == RTA_OIF) {
				iface = *((int *)rta_data);
			}
			
			if (rta_type == RTA_DST) {
				ip = (const char *)rta_data;
			}
			
			if (rta_type == RTA_GATEWAY) {
				gw = (const char *)rta_data;
			}

			rth = RTA_NEXT(rth, rtl);
		}
		if (iface && gw) {
			char ifacestr[IF_NAMESIZE];
			char ip_cstr[INET_ADDRSTRLEN];
			char gw_cstr[INET_ADDRSTRLEN];
			const char *pip;
			if (ip) {
				inet_ntop(AF_INET, ip, ip_cstr, INET_ADDRSTRLEN);
				pip = ip_cstr;
			} else {
				pip = "default";
			}
			inet_ntop(AF_INET, gw, gw_cstr, INET_ADDRSTRLEN);
			printf("%s route(%s:%u): dst - %s/%d ; gw - %s\n", 
					(type == RTM_NEWROUTE)? "add" : "del",
					if_indextoname(iface, ifacestr), iface, 
					(char *)pip, rtm->rtm_dst_len,
					gw_cstr);
			return 0;
		}
		return 0;
	}

    return 0;
}


int linkevent()
{
	int ret = -1;
	struct nl_sock *sk;

	/* nl socket */
	sk = nl_socket_alloc();
	if (!sk) {
		printf("nl_socket_alloc\n");
		return ret;
	}

	/* notifications do not use sequence numbers */
	nl_socket_disable_seq_check(sk);

	void *arg = 0;

	/* will be called for each notification */	
	if (nl_socket_modify_cb(sk, NL_CB_VALID, NL_CB_CUSTOM, linkevent_cb, arg) != 0) {
		printf("nl_socket_modify_cb\n");
		goto exit_sk;
	}

	/* connect to nl system */
	if (nl_connect(sk, NETLINK_ROUTE) != 0) {
		printf("nl_connect\n");
		goto exit_sk;
	}

	/* sub on link group */
	if (nl_socket_add_memberships(sk, RTNLGRP_LINK, 0) != 0) {
		printf("nl_socket_add_memberships1\n");
		goto exit_sk;
	}

	/* sub on ip link group */
	if (nl_socket_add_memberships(sk, RTNLGRP_IPV4_IFADDR, 0) != 0) {
		printf("nl_socket_add_memberships2\n");
		goto exit_sk;
	}

	/* */
	if (nl_socket_add_memberships(sk, RTNLGRP_IPV4_ROUTE, 0) != 0) {
		printf("nl_socket_add_memberships3\n");
		goto exit_sk;
	}

	/* */
	if (nl_socket_add_memberships(sk, RTNLGRP_IPV4_MROUTE, 0) != 0) {
		printf("nl_socket_add_memberships3\n");
		goto exit_sk;
	}

	/* loop */
	while (1) {
		nl_recvmsgs_default(sk);
	}

	ret = 0;

	/* cleanup */
exit_sk:
	nl_socket_free(sk);
	return ret;
}












#define IP_ADDR_LEN 4

static int PrefixToMask(int prefix, unsigned char *buf)
{
    uint32_t mask = prefix ? ~0 << (32 - prefix) : 0;
	mask = htonl(mask);
	memcpy(buf, &mask, IP_ADDR_LEN);
	//inet_ntop(AF_INET, &mask, buf, INET_ADDRSTRLEN);    
}

static uint8_t MaskToPrefix(const char *trgMask)
{
	uint8_t ret;
	uint32_t prefix = 0;
	uint32_t mask = 0;
    if (trgMask) {
        if (strlen(trgMask) <= 2) {
            prefix = atoi(trgMask);
            mask = (0xFFFFFFFF << (32 - prefix)) & 0xFFFFFFFF;
            mask = htonl(mask);
        } else {
	        mask = ntohl(inet_addr(trgMask));
			for (int i = 31; i >= 0; --i) {
				if (mask & (1 << i)) {
					prefix++;
				} else {
					break;
				}
			}
        }
    }
	ret = (uint8_t)prefix;
	return ret;
}



struct FindDefaultGwAndRoutePrms {
	struct nl_sock *sk;
	struct rtnl_nexthop *dgw;
	uint32_t xorRes;
	struct {
		int devIdx;
		unsigned char net[IP_ADDR_LEN]; 
		unsigned char mask[IP_ADDR_LEN]; 
		unsigned char src[IP_ADDR_LEN]; 
		unsigned char dgw[IP_ADDR_LEN];
		int ok;
	} result;
};

static void findDefaultGwAndRouteFromCache(struct nl_object *obj, void *p)
{
	struct FindDefaultGwAndRoutePrms *prms = (struct FindDefaultGwAndRoutePrms *)p;
	struct rtnl_route *route = (struct rtnl_route *)obj;
	uint32_t table = rtnl_route_get_table(route);
	uint8_t family = rtnl_route_get_family(route);
	if (table == RT_TABLE_MAIN && family == AF_INET) {
		struct nl_addr *addr_dst = rtnl_route_get_dst(route);
		char s_addr_dst[32];
		char *sad = nl_addr2str(addr_dst, s_addr_dst, 32);
		uint8_t prefix = nl_addr_get_prefixlen(addr_dst);
		if (sad) {
			if ( !prms->dgw) {				
				if ( strstr(sad, "none") != 0 && prefix == 0 ) {
					prms->dgw = rtnl_route_nexthop_n(route, 0);
					prms->xorRes = 0xffffffff;
				}
			} else {
				if ( strstr(sad, "none") == 0 && prefix != 0 ) {
					struct rtnl_nexthop *nh = rtnl_route_nexthop_n(route, 0);
					if (nh) {
						struct nl_addr *addr = rtnl_route_get_dst(route);
						if (addr) {
							union {
								char b[4];
								uint32_t d;
							} u_addr, u_dgw;
							uint32_t xorRes;

							struct nl_addr *dgw = rtnl_route_nh_get_gateway(prms->dgw);
							if (dgw) {
								memcpy(u_addr.b, nl_addr_get_binary_addr(addr), IP_ADDR_LEN);
								memcpy(u_dgw.b, nl_addr_get_binary_addr(dgw), IP_ADDR_LEN);
								xorRes = u_addr.d ^ u_dgw.d;

								if (prms->xorRes > xorRes) {
									struct nl_addr *src_addr = rtnl_route_get_pref_src(route);
									if (src_addr) {
										memcpy(prms->result.net, nl_addr_get_binary_addr(addr_dst), IP_ADDR_LEN);
										PrefixToMask(prefix, prms->result.mask);
										memcpy(prms->result.src, nl_addr_get_binary_addr(src_addr), IP_ADDR_LEN);
										memcpy(prms->result.dgw, nl_addr_get_binary_addr(dgw), IP_ADDR_LEN);
										prms->result.devIdx = rtnl_route_nh_get_ifindex(nh);
										prms->xorRes = xorRes;
										prms->result.ok = 1;
									}
								}
							}
						}						
					}
				}
			}
		}
	}
}

static int FindDefaultGwAndRoute(char *devName, char *net, char *mask, char *src, char *dgw)
{
	int ret = -1;
	struct nl_sock *sk;
	struct nl_cache *cache;
	struct rtnl_link *link;
	struct nl_addr *addr;

	if ( (sk = nl_socket_alloc()) ) {
		if (nl_connect(sk, NETLINK_ROUTE) == 0) {
			// del all routes to specified network
			// del default route
			if (rtnl_route_alloc_cache(sk, AF_UNSPEC, 0, &cache) == 0) {
				struct FindDefaultGwAndRoutePrms prms = {0};
				prms.sk = sk;
				nl_cache_foreach(cache, findDefaultGwAndRouteFromCache, &prms);
				if (prms.result.ok) {
					if_indextoname(prms.result.devIdx, devName);
					inet_ntop(AF_INET, prms.result.net, net, INET_ADDRSTRLEN); 
					inet_ntop(AF_INET, prms.result.mask, mask, INET_ADDRSTRLEN); 
					inet_ntop(AF_INET, prms.result.src, src, INET_ADDRSTRLEN); 
					inet_ntop(AF_INET, prms.result.dgw, dgw, INET_ADDRSTRLEN);
					ret = 0;
				}
				nl_cache_free(cache);
			}
		}
		nl_socket_free(sk);
	}
	return ret;	
}



struct DelRoutesByNetPrms {
	struct nl_sock *sk;
	const char *ip;
	uint8_t prefix;
	uint8_t delDefault;
};

static void delRoutesByNet(struct nl_object *obj, void *p)
{
	struct DelRoutesByNetPrms *prms = (struct DelRoutesByNetPrms *)p;
	struct rtnl_route *route = (struct rtnl_route *)obj;
	uint32_t table = rtnl_route_get_table(route);
	uint8_t family = rtnl_route_get_family(route);
	if (table == RT_TABLE_MAIN && family == AF_INET) {
		struct nl_addr *addr_dst = rtnl_route_get_dst(route);
		char s_addr_dst[32];
		char *sad = nl_addr2str(addr_dst, s_addr_dst, 32);
		uint8_t prefix = nl_addr_get_prefixlen(addr_dst);
		if (sad) {
			if ( strstr(sad, prms->ip) != 0 && prms->prefix == prefix ) {
				printf("del %s\n", sad);
				rtnl_route_delete(prms->sk, route, 0);
			}
			if ( prms->delDefault && strstr(sad, "none") != 0 && prefix == 0 ) {
				printf("del default\n");
				rtnl_route_delete(prms->sk, route, 0);
			}
		}
	}
}

static int SwitchAllRoute(const char *devName, const char *net, const char *mask, const char *src, const char *dgw)
{
	int ret = -1;
	struct nl_sock *sk;
	struct nl_cache *cache;
	struct rtnl_link *link;
	struct rtnl_nexthop *nh;
	struct rtnl_route *route;
	struct nl_addr *dst_addr = 0;
	struct nl_addr *src_addr = 0;
	struct nl_addr *gw_addr = 0;

	printf(" devName=%s\n net=%s\n mask=%s\n src=%s\n dgw=%s\n", devName, net, mask, src, dgw);

	if ( (sk = nl_socket_alloc()) ) {
		if (nl_connect(sk, NETLINK_ROUTE) == 0) {
			uint32_t prefix = MaskToPrefix(mask);
			// del all routes to specified network
			// del default route
			if (rtnl_route_alloc_cache(sk, AF_UNSPEC, 0, &cache) == 0) {
				struct DelRoutesByNetPrms prms;
				prms.sk = sk;
				prms.ip = net;
				prms.prefix = prefix;
				prms.delDefault = 1;
				nl_cache_foreach(cache, delRoutesByNet, &prms);
				nl_cache_free(cache);
			}
			if (rtnl_link_alloc_cache(sk, AF_UNSPEC, &cache) == 0) {
				// add 1 route to specified network
				if ( route = rtnl_route_alloc() ) {
					if (nl_addr_parse(net, AF_INET, &dst_addr) == 0) { // network
						nl_addr_set_prefixlen(dst_addr, prefix);
						if (rtnl_route_set_dst(route, dst_addr) == 0) {
							if (nl_addr_parse(src, AF_INET, &src_addr) == 0) { // source
								//nl_addr_set_prefixlen(src_addr, prefix);
								if (rtnl_route_set_pref_src(route, src_addr) == 0) {
									if ( nh = rtnl_route_nh_alloc() ) {
										rtnl_route_nh_set_ifindex(nh, rtnl_link_name2i(cache, devName));
										rtnl_route_add_nexthop(route, nh);
										if ( rtnl_route_add(sk, route, NLM_F_CREATE) == 0) { // add route
											ret = 0;
										}
									}
								}
							}
						}
					}
					if (dst_addr) nl_addr_put(dst_addr);
					if (src_addr) nl_addr_put(src_addr);
					rtnl_route_put(route);
				}
				// add default route via gw
				if (ret == 0) { // route has been added
					ret = -1; // reset
					if ( route = rtnl_route_alloc() ) {
						if (nl_addr_parse("none", AF_INET, &dst_addr) == 0) { // all networks
							nl_addr_set_prefixlen(dst_addr, 0);
							if (rtnl_route_set_dst(route, dst_addr) == 0) {
								if ( nh = rtnl_route_nh_alloc() ) {
									if (nl_addr_parse(dgw, AF_INET, &gw_addr) == 0) { // gateway
										rtnl_route_nh_set_gateway(nh, gw_addr);
										rtnl_route_nh_set_ifindex(nh, rtnl_link_name2i(cache, devName));
										rtnl_route_add_nexthop(route, nh);
										int error;
										if ( rtnl_route_add(sk, route, NLM_F_EXCL) == 0) { // add route
											printf("rtnl_route_add done 2\n");
											ret = 0;
										}
									}
									//rtnl_route_nh_free(nh);
								}
							}
						}
						if (dst_addr) nl_addr_put(dst_addr);
						if (gw_addr) nl_addr_put(gw_addr);
						rtnl_route_put(route);
					}
				}
				nl_cache_free(cache);
			}
		}
		nl_socket_free(sk);
	}
	return ret;
}










int main() 
{
	// ipaddradd("enp2s0f0","50.50.12.13","255.255.0.0");
	// iproadd("enp0s25","50.50.0.0","255.255.0.0","110.10.0.2");
	// iproadd("wlp3s0","default",0,"192.168.1.1");
	// linkevent();
	// getMac("eths0");
	// setMac("eths0");
	// delAllRouteByNet("90.90.0.0", "255.255.0.0");
	// FindDefaultGwAndRoute();

// 	iproadd2("ethd0", "10.15.0.0", "255.255.0.0", "10.15.4.90");
// return 0;
	char *newDevName = "eths0";
	char devName[IF_NAMESIZE];
	char net[INET_ADDRSTRLEN]; 
	char mask[INET_ADDRSTRLEN]; 
	char src[INET_ADDRSTRLEN]; 
	char dgw[INET_ADDRSTRLEN];
	FindDefaultGwAndRoute(devName, net, mask, src, dgw);
	printf(" devName=%s\n net=%s\n mask=%s\n src=%s\n dgw=%s\n", devName, net, mask, src, dgw);

	SwitchAllRoute(newDevName, net, mask, src, dgw);


	return 0;
}