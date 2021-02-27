#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <net/route.h>
#include <net/if.h>
#include <net/ethernet.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>

#include <netlink/cli/utils.h>
#include <netlink/cli/addr.h>
#include <netlink/cli/link.h>
#include <netlink/cli/route.h>
#include <netlink/route/link.h>


#define TM_NOTE			1
#define TM_WARN			2
#define TM_ERROR		4
#define TM_DUMPRX		8
#define TM_DUMPTX		16
#define TM_TRACE		32
#define TM_USER			64
#define TM_SERV			128
# define netctl_debug(m,...) printf("netctl: " m "\n",##__VA_ARGS__)
# define netctl_log(t,m,...) printf("netctl: " m "\n",##__VA_ARGS__)

/* Ethernet type for Link layer header */
#define ETHERNET_TYPE 0x806
/* Our implementation uses Ethernet as the ARP hardware type */
#define ETHERNET_ARP_TYPE 0x0001
/* Our implementation uses IP as the ARP protocol type */
#define IP_TYPE 0x0800
/* When ARP protocol type is IP its address needs 4 bytes of storage */
#define IP_ADDR_LEN 4
/* ARP option to express reply */
#define REQUEST_OPERATION 1
/* Size for the padding data */
#define ARP_PADDING_SIZE 18



#pragma pack(push, 1)
struct gratuitous_arp_frame {
    /* Ethernet header */
    unsigned char dstEth[ETH_ALEN];
    unsigned char srcEth[ETH_ALEN];
    unsigned short ethType;

    /* ARP Header */
    unsigned short hwType;
    unsigned short protType;

    unsigned char hwAddrLen;
    unsigned char protAddrLen;

    unsigned short operation;

    unsigned char srcHwAddr[ETH_ALEN];
    unsigned char srcProtAddr[IP_ADDR_LEN];

    unsigned char trgHwAddr[ETH_ALEN];
    unsigned char trgProtAddr[IP_ADDR_LEN];
};
#pragma pack(pop)
struct gratuitous_arp {
	int fd;
    struct gratuitous_arp_frame frame;
	struct sockaddr_ll dstAddr;
};



static int PrefixToMask(int prefix, unsigned char *buf)
{
    uint32_t mask = prefix ? ~0 << (32 - prefix) : 0;
	mask = htonl(mask);
	memcpy(buf, &mask, IP_ADDR_LEN); // to bytes
	//inet_ntop(AF_INET, &mask, buf, INET_ADDRSTRLEN); // to string
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
			int i;
	        mask = ntohl(inet_addr(trgMask));
			for (i = 31; i >= 0; --i) {
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



static int GetMAC(const char *devName, unsigned char *buf)
{
	int ret = -1;
	struct nl_sock *sk;
	struct nl_cache *cache;
	struct rtnl_link *link;
	struct nl_addr *addr;

    if (!devName || !buf) {
        netctl_log(TM_ERROR, "GetMAC -- devName=%p buf=%p", devName, buf);
        return -1;
    }

	if ( (sk = nl_socket_alloc()) ) {
		if (nl_connect(sk, NETLINK_ROUTE) == 0) {
			if (rtnl_link_alloc_cache(sk, AF_UNSPEC, &cache) == 0) {
				if ( (link = rtnl_link_get_by_name(cache, devName)) ) {
					if ( (addr = rtnl_link_get_addr(link)) ) {
						char str[32];
						char *res = nl_addr2str(addr, str, 32);
						if (res) {
							if (6 == sscanf(str, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", 
									// &buf[5],&buf[4],&buf[3],&buf[2],&buf[1],&buf[0]) ) 
									&buf[0],&buf[1],&buf[2],&buf[3],&buf[4],&buf[5]) ) 
							{
								ret = 0;
							}
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



static int SetMAC(const char *devName, unsigned char *mac)
{
	int ret = -1;
	struct nl_sock *sk;
	struct nl_cache *cache;
	struct rtnl_link *link;
	struct rtnl_link *link_changes;
	struct nl_addr *addr;

    if (!devName || !mac) {
        netctl_log(TM_ERROR, "GetMAC -- devName=%p mac=%p", devName, mac);
        return -1;
    }

	if ( (sk = nl_socket_alloc()) ) {
		if (nl_connect(sk, NETLINK_ROUTE) == 0) {
			if (rtnl_link_alloc_cache(sk, AF_UNSPEC, &cache) == 0) {
				if ( (link = rtnl_link_get_by_name(cache, devName)) ) {
					if ( (addr = nl_addr_build(AF_LLC, mac, ETH_ALEN)) ) {
						if ( (link_changes = rtnl_link_alloc()) ) {
							rtnl_link_set_addr(link_changes, addr);
							if (rtnl_link_change (sk, link, link_changes, 0) == 0) {
								ret = 0;
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



static int PrepareGraArp(struct gratuitous_arp *arp, const char *devName)
{
    if (!arp || !devName) {
        netctl_log(TM_ERROR, "PrepareGraArp -- devName=%p arp=%p", devName, arp);
        return -1;
    }

	bzero(arp, sizeof(struct gratuitous_arp));

	int fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ARP));
	if (fd >= 0) {
		
		memset(arp->frame.dstEth, 0xFF, ETH_ALEN);
		GetMAC(devName, arp->frame.srcEth);
		arp->frame.ethType = htons(ETHERNET_TYPE);
		arp->frame.hwType = htons(ETHERNET_ARP_TYPE);
		arp->frame.protType = htons(IP_TYPE);
		arp->frame.hwAddrLen = ETH_ALEN;
		arp->frame.protAddrLen = IP_ADDR_LEN;
		arp->frame.operation = htons(ETHERNET_ARP_TYPE);
		memcpy(arp->frame.srcHwAddr, arp->frame.srcEth, ETH_ALEN);
		// (arp->frame.srcProtAddr);
		memcpy(arp->frame.trgHwAddr, arp->frame.dstEth, ETH_ALEN);	
		// (arp->frame.trgProtAddr);	

		arp->dstAddr.sll_family = AF_PACKET;
		arp->dstAddr.sll_protocol = htons(ETH_P_ARP);
		arp->dstAddr.sll_ifindex = (int)if_nametoindex(devName);
		memcpy(arp->dstAddr.sll_addr, arp->frame.dstEth, ETH_ALEN);
	}
	return fd;
}

static int SendGraArp(struct gratuitous_arp *arp, const char *ip)
{
	struct in_addr source_addr = {0};

    if (!arp || !ip) {
        netctl_log(TM_ERROR, "SendGraArp -- arp=%p ip=%p", arp, ip);
        return -1;
    }
		
	source_addr.s_addr = inet_addr(ip);
	memcpy(arp->frame.srcProtAddr, &source_addr, IP_ADDR_LEN);
	memcpy(arp->frame.trgProtAddr, &source_addr, IP_ADDR_LEN);
	int sent = sendto(arp->fd, &arp->frame, sizeof(struct gratuitous_arp_frame), 0, 
			(struct sockaddr *)&arp->dstAddr, sizeof(struct sockaddr_ll));
	return (sent > 0)? 0 : -1;
}

static inline void FreeGraArp(struct gratuitous_arp *arp)
{
	if (arp) {
		close(arp->fd);
	}
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
	
    if (!net || !mask || !src || !dgw || !devName) {
        netctl_log(TM_ERROR, "FindDefaultGwAndRoute -- net=%p mask=%p src=%p dgw=%p devName=%p", net, mask, src, dgw, devName);
        return -1;
    }

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
				rtnl_route_delete(prms->sk, route, 0);
			}
			if ( prms->delDefault && strstr(sad, "none") != 0 && prefix == 0 ) {
				rtnl_route_delete(prms->sk, route, 0);
			}
		}
	}
}

static int SwitchAllRoute(const char *devName, const char *net, const char *mask, const char *src, const char *dgw)
{
	int error;
	int ret = -1;
	struct nl_sock *sk;
	struct nl_cache *cache;
	struct rtnl_link *link;
	struct rtnl_nexthop *nh;
	struct rtnl_route *route;
	struct nl_addr *dst_addr = 0;
	struct nl_addr *src_addr = 0;
	struct nl_addr *gw_addr = 0;
	
    if (!devName || !net || !mask || !src || !dgw) {
        netctl_log(TM_ERROR, "SwitchAllRoute -- devName=%p net=%p mask=%p src=%p dgw=%p", devName, net, mask, src, dgw);
        return -1;
    }

		printf(" SwitchAllRoute\n  devName=%s\n  net=%s\n  mask=%s\n  src=%s\n  dgw=%s\n", devName, net, mask, src, dgw);

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
			// add routes
			if (rtnl_link_alloc_cache(sk, AF_UNSPEC, &cache) == 0) {
				// link up
				{
					struct rtnl_link *changes = rtnl_link_alloc();
					if (changes) {
                   		rtnl_link_set_flags(changes, IFF_UP);
						if ( (link = rtnl_link_get_by_name(cache, devName)) ) {
							if (rtnl_link_change(sk, link, changes, 0) == 0) {
								ret = 0;
							}
						}
						rtnl_link_put(changes);
					}
                    
				}
				if (ret == 0) { // link is up
					ret = -1; // reset
					// add 1 route to specified network
					if ( (route = rtnl_route_alloc()) ) {
						if (nl_addr_parse(net, AF_INET, &dst_addr) == 0) { // network
							nl_addr_set_prefixlen(dst_addr, prefix);
							if (rtnl_route_set_dst(route, dst_addr) == 0) {
								if (nl_addr_parse(src, AF_INET, &src_addr) == 0) { // source
									nl_addr_set_prefixlen(src_addr, prefix);
									if (rtnl_route_set_pref_src(route, src_addr) == 0) {
										if ( (nh = rtnl_route_nh_alloc()) ) {
											rtnl_route_nh_set_ifindex(nh, rtnl_link_name2i(cache, devName));
											rtnl_route_add_nexthop(route, nh);
											if ( (error=rtnl_route_add(sk, route, NLM_F_CREATE)) == 0) { // add route
												ret = 0;
											}
											//printf("err = %s\n", nl_geterror(error));
											//rtnl_route_nh_free(nh);
										}
									}
								}
							}
						}
						if (dst_addr) { nl_addr_put(dst_addr); dst_addr = 0; }
						if (src_addr) { nl_addr_put(src_addr); src_addr = 0; }
						rtnl_route_put(route);
					}
				}
				// add default route via gw
				if (ret == 0) { // route has been added
					ret = -1; // reset
					if ( (route = rtnl_route_alloc()) ) {
						if (nl_addr_parse("none", AF_INET, &dst_addr) == 0) { // all networks
							nl_addr_set_prefixlen(dst_addr, 0);
							if (rtnl_route_set_dst(route, dst_addr) == 0) {
								if ( (nh = rtnl_route_nh_alloc()) ) {
									if (nl_addr_parse(dgw, AF_INET, &gw_addr) == 0) { // gateway
										rtnl_route_nh_set_gateway(nh, gw_addr);
										rtnl_route_nh_set_ifindex(nh, rtnl_link_name2i(cache, devName));
										rtnl_route_add_nexthop(route, nh);
										if ( (error=rtnl_route_add(sk, route, NLM_F_EXCL)) == 0) { // add route
											ret = 0;
										}
										//printf("err = %s\n", nl_geterror(error));
									}
									//rtnl_route_nh_free(nh);
								}
							}
						}
						if (dst_addr) { nl_addr_put(dst_addr); dst_addr = 0; }
						if (gw_addr) { nl_addr_put(gw_addr); src_addr = 0; }
						rtnl_route_put(route);
					}
				}
				nl_cache_free(cache);
			}
		}
		nl_socket_free(sk);
	}
	(void)error;
	return ret;
}



struct EmitAboutAddrPrms {
	struct nl_sock *sk;
	int newDevIdx;
	int oldDevIdx;
	struct gratuitous_arp *arp;
};

static void emitAboutAddr(struct nl_object *obj, void *p)
{
	struct EmitAboutAddrPrms *prms = (struct EmitAboutAddrPrms *)p;
	struct rtnl_addr *addr = (struct rtnl_addr *)obj;
	if (rtnl_addr_get_family(addr) == AF_INET) {
		int idx = rtnl_addr_get_ifindex(addr);
		if (idx == prms->newDevIdx || idx == prms->oldDevIdx) {
			struct nl_addr *laddr = rtnl_addr_get_local(addr);
			if (laddr) {
				char s_addr[32];
				char *sa = nl_addr2str(laddr, s_addr, 32);
				if (sa) {
					char *sl = strchr(s_addr, '/');
					*sl = 0;
					SendGraArp(prms->arp, sa);
				}
			}
		}
	}
}

static int EmitNewDevice(const char *newDev, const char *oldDev)
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
	
    if (!newDev) {
        netctl_log(TM_ERROR, "EmitNewDevice -- newDev=%p", newDev);
        return -1;
    }

	if ( (sk = nl_socket_alloc()) ) {
		if (nl_connect(sk, NETLINK_ROUTE) == 0) {
			if ( (cache = nl_cli_addr_alloc_cache(sk)) ) {
				struct EmitAboutAddrPrms prms = {0};
				struct gratuitous_arp arp;
				arp.fd = PrepareGraArp(&arp, newDev);
				prms.sk = sk;
				prms.newDevIdx = if_nametoindex(newDev);
				if (oldDev) prms.oldDevIdx = if_nametoindex(oldDev);
				prms.arp = &arp;
				nl_cache_foreach(cache, emitAboutAddr, &prms);
				FreeGraArp(&arp);
				nl_cache_free(cache);
				ret = 0;
			}
		}
		nl_socket_free(sk);
	}
	return ret;
}
