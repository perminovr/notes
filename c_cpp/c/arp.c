
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <net/ethernet.h>

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
struct gratuitous_arp {
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



int sendGraArp(const char *devName, const char *ip)
{
	struct gratuitous_arp arp = {0};
	struct sockaddr_ll addr = {0};
	struct in_addr source_addr = {0};

	int fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ARP));
	if (fd >= 0) {
		source_addr.s_addr = inet_addr(ip);
		// a4:17:31:0d:8e:25 todo
		arp.srcEth[0] = 0xa4;
		arp.srcEth[1] = 0x17;
		arp.srcEth[2] = 0x31;
		arp.srcEth[3] = 0x0d;
		arp.srcEth[4] = 0x8e;
		arp.srcEth[5] = 0x25;

		memset(arp.dstEth, 0xFF, ETH_ALEN);
		// (arp.srcEth);
		arp.ethType = htons(ETHERNET_TYPE);
		arp.hwType = htons(ETHERNET_ARP_TYPE);
		arp.protType = htons(IP_TYPE);
		arp.hwAddrLen = ETH_ALEN;
		arp.protAddrLen = IP_ADDR_LEN;
		arp.operation = htons(ETHERNET_ARP_TYPE);
		memcpy(arp.srcHwAddr, arp.srcEth, ETH_ALEN);
		memcpy(arp.srcProtAddr, &source_addr, IP_ADDR_LEN);
		memcpy(arp.trgHwAddr, arp.dstEth, ETH_ALEN);
		memcpy(arp.trgProtAddr, &source_addr, IP_ADDR_LEN);
		
		addr.sll_family = AF_PACKET;
		addr.sll_protocol = htons(ETH_P_ARP);
		addr.sll_ifindex = 3; // todo eth idx
		memcpy(addr.sll_addr, arp.dstEth, ETH_ALEN);		

		int sent = sendto(fd, &arp, sizeof(struct gratuitous_arp), 0, 
				(struct sockaddr *)&addr, sizeof(struct sockaddr_ll));

		close(fd);
		return (sent > 0)? 0 : -1;
	}
	return -1;
}


int main()
{
	printf ( "res = %d\n" ,sendGraArp("wlp3s0", "10.10.0.1") );
	return 0;
}