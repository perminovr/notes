#include <stdio.h>

#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <net/if.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/poll.h>
#include <sys/eventfd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/ether.h>


void DumpData(int fd, const void* data, size_t size);


int main(int argc, char *argv[])
{
    int sockopt = 1;
    struct ifreq _iface = {0};
	int idx = if_nametoindex(argv[1]);
    int fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	ioctl(fd, SIOCGIFHWADDR, &_iface);

    // promisc mode
    struct packet_mreq mreq = {0};
    mreq.mr_ifindex = idx;
    mreq.mr_type = PACKET_MR_PROMISC;

    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof(sockopt));
    setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE, argv[1], IFNAMSIZ-1);
    setsockopt(fd, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mreq, sizeof(mreq));

	struct pollfd pfds;
	pfds.events = POLLIN;
	pfds.fd = fd;

	char buf[ETH_FRAME_LEN];
	int res;
    for (;;) {
        res = poll(&pfds, 1, -1);
        if (res > 0) {
			if (pfds.revents == POLLIN) {
				int r = recv(pfds.fd, buf, ETH_FRAME_LEN, 0);
				if (r) {
					DumpData(1, buf, r);
				}
			}
		}
	}

	return(0);
}


void DumpData(int fd, const void* data, size_t size)
{
    #define DUMPHEX_WIDTH 16
	char ascii[DUMPHEX_WIDTH+1];
	size_t i, j;
	ascii[DUMPHEX_WIDTH] = '\0';
	for (i = 0; i < size; ++i) {
		dprintf(fd, "%02X ", ((uint8_t*)data)[i]);
		if (((uint8_t*)data)[i] >= ' ' && ((uint8_t*)data)[i] <= '~') {
			ascii[i % DUMPHEX_WIDTH] = ((uint8_t*)data)[i];
		} else {
			ascii[i % DUMPHEX_WIDTH] = '.';
		}
		if ((i+1) % 8 == 0 || i+1 == size) {
			dprintf(fd, " ");
			if ((i+1) % DUMPHEX_WIDTH == 0) {
				dprintf(fd, "|  %s \n", ascii);
			} else if (i+1 == size) {
				ascii[(i+1) % DUMPHEX_WIDTH] = '\0';
				if ((i+1) % DUMPHEX_WIDTH <= 8) {
					dprintf(fd, " ");
				}
				for (j = (i+1) % DUMPHEX_WIDTH; j < DUMPHEX_WIDTH; ++j) {
					dprintf(fd, "   ");
				}
				dprintf(fd, "|  %s \n", ascii);
			}
		}
	}
}