#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/eventfd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/if_packet.h>



#define UDP_MAX         1
#define TCP_MAX         1   // prg as tcp client
#define SERVERS_MAX     1   // prg as tcp server
#define CLIENTS_MAX     5   // clients per server
#define SSDP_REQ_MAX    10
#define PFDS_SIZE_MAX	(UDP_MAX+SSDP_REQ_MAX+TCP_MAX+SERVERS_MAX+CLIENTS_MAX)
#define PORT_FIND	    1901
#define PORT_LISTEN	    10020
#define PORT_SEND	    10030
#define IP_FIND		    "239.255.255.251"



typedef struct {
	pthread_t mthread;
	pthread_mutex_t mu;

    struct pollfd pfds[PFDS_SIZE_MAX];
	int pfds_size;

	int udp_start_idx;
	int udp_size;

	int ssdp_start_idx;
	int ssdp_size;

	int tcp_start_idx;
	int tcp_size;

	int tcps_start_idx;
	int tcps_size;

	int tcpc_start_idx;
	int tcpc_size;
} ctl_t;




static void SendUDPData(int fd, const char *ip, const void *msg, size_t len)
{
    struct sockaddr_in addr;
	addr.sin_family = AF_INET; 
	addr.sin_port = htons(PORT_SEND); 
	addr.sin_addr.s_addr = inet_addr(ip);
    int res = sendto(fd, msg, len, 0, (const struct sockaddr *)&addr, sizeof(struct sockaddr_in)); 
}



static void SendSSDPRequest(ctl_t *this)
{
	int i;
	struct sockaddr_in addr = {0};
	char *request="SSDP REQUEST";
	addr.sin_family = AF_INET;
	addr.sin_port = htons(PORT_FIND);
	addr.sin_addr.s_addr = inet_addr(IP_FIND);

	for (i = this->ssdp_start_idx; i < this->ssdp_size + this->ssdp_start_idx; ++i) {
		if (sendto(this->pfds[i].fd, request, strlen(request), 0, (struct sockaddr*)&addr, sizeof(struct sockaddr_in)) < 0) {
			printf("sendto err\n");
		}
	}
}



static void ConnectToServer(int fd, const char *ip)
{
    struct sockaddr_in addr;
	addr.sin_family = AF_INET; 
	addr.sin_port = htons(PORT_SEND); 
	addr.sin_addr.s_addr = inet_addr(ip);
    if (connect(fd, (const struct sockaddr *)&addr, sizeof(struct sockaddr_in)) < 0) {
        ;
    }
}



static void SendTCPData(int fd, const void *msg, size_t len)
{
    if (send(fd, msg,len, 0) < 0) {
        ;
    }
}



static void *Loop(void *data)
{
	ctl_t *this = (ctl_t*)data;
	const size_t buf_sz = 4096;
	uint8_t buf[buf_sz];
    struct sockaddr addr;
    socklen_t addrlen;

	int res, i;
	for (;;) {
		res = poll(this->pfds, this->pfds_size, -1);
        // network
		if (res > 0) {
			for (i = 0; i < this->pfds_size; ++i) {
				if (this->pfds[i].revents == POLLIN) {
                    // tcp server incoming connection 
					if (i < (this->tcps_start_idx+this->tcps_size) && i >= this->tcps_start_idx) {
                        int fd = accept(this->pfds[i].fd, &addr, &addrlen);
                        // do not accept
                        if (this->tcpc_size == CLIENTS_MAX) {
                            // todo decline
                            continue;
                        }
                        this->pfds[this->pfds_size].fd = fd;
                        this->pfds[this->pfds_size].events |= POLLIN;
                        this->pfds_size++;
                        this->tcpc_size++;
                    } else {
                        res = recvfrom(this->pfds[i].fd, buf, buf_sz, 0, (struct sockaddr *)&addr, &addrlen);
                        // ...
                        // udp
                        if (i < (this->udp_start_idx+this->udp_size) && i >= this->udp_start_idx) {
                            ;
                        }
                        // ssdp
                        if (i < (this->ssdp_start_idx+this->ssdp_size) && i >= this->ssdp_start_idx) {
                            ;
                        }
                        // tcp
                        if (i < (this->tcp_start_idx+this->tcp_size) && i >= this->tcp_start_idx) {
                            // connection closed by server
                            if (res == 0) {
                                ;
                            } 
                            // error => broken pipe?
                            else if (res < 0) {
                                ;
                            }
                            // data from client
                            else {
                                ;
                            }
                        }
                        // tcp clients
                        if (i < (this->tcpc_start_idx+this->tcpc_size) && i >= this->tcpc_start_idx) {
                            // connection closed by client
                            if (res == 0) {
                                ;
                            } 
                            // error => broken pipe?
                            else if (res < 0) {
                                ;
                            }
                            // data from client
                            else {
                                ;
                            }
                        }
                    }
                }
            }
        } 
        // timeouts
        else if (res == 0) {

        }
    }
Loop_exit:
    pthread_exit(0);
}



static int TCP_client_init(ctl_t *this)
{
    int fd;
    int val = 1;

    this->tcp_start_idx = this->pfds_size;

    {
        fd = socket(AF_INET, SOCK_STREAM, 0);
        this->pfds[this->pfds_size].fd = fd;
        this->pfds[this->pfds_size].events |= POLLIN;
        this->pfds_size++;
        this->tcp_size++;

        if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) < 0) {
            goto TCP_client_init_error;
        }
    }
    
    return 0;
TCP_client_init_error:
    return -1;
}



static int TCP_server_init(ctl_t *this)
{
    int fd;
    int val = 1;
    struct sockaddr_in addr = {0};

    this->tcps_start_idx = this->pfds_size;

    {
        fd = socket(AF_INET, SOCK_STREAM, 0);
        this->pfds[this->pfds_size].fd = fd;
        this->pfds[this->pfds_size].events |= POLLIN;
        this->pfds_size++;
        this->tcps_size++;

        if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) < 0) {
            goto TCP_server_init_error;
        }
        { // keep alive
            if (setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &val, sizeof(val)) < 0) {
                goto TCP_server_init_error;
            }
            // if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPIDLE, &val, sizeof(val)) < 0) {
            //     goto TCP_server_init_error;
            // }
            // if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPINTVL, &val, sizeof(val)) < 0) {
            //     goto TCP_server_init_error;
            // }
            // if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPCNT, &val, sizeof(val)) < 0) {
            //     goto TCP_server_init_error;
            // }
        }     

        addr.sin_family = AF_INET;
        addr.sin_port = htons(PORT_LISTEN);
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        if (bind(fd, (struct sockaddr*)&addr, sizeof(struct sockaddr_in)) < 0) {
            goto TCP_server_init_error;
        }

        if (listen(fd, CLIENTS_MAX) < 0) {
            goto TCP_server_init_error;
        }

        if (this->tcps_size == SERVERS_MAX) {
            ;
        }
    }

    // todo
    this->tcpc_start_idx = this->pfds_size;

    return 0;
TCP_server_init_error:
    return -1;
}



static int UDP_init(ctl_t *this)
{
    int fd;
    int val = 1;
    struct sockaddr_in addr = {0};

    this->udp_start_idx = this->pfds_size;

    {
        fd = socket(PF_INET, SOCK_DGRAM, 0);
        this->pfds[this->pfds_size].fd = fd;
        this->pfds[this->pfds_size].events |= POLLIN;
        this->pfds_size++;
        this->udp_size++;

        if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) < 0) {
            goto UDP_init_error;
        }

        addr.sin_family = AF_INET;
        addr.sin_port = 0; // any // htons(PORT);
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        if (bind(fd, (struct sockaddr*)&addr, sizeof(struct sockaddr_in)) < 0) {
            goto UDP_init_error;
        }

        if (this->udp_size == UDP_MAX) {
            ;
        }
    }

    return 0;
UDP_init_error:
    return -1;
}



static int SSDP_init(ctl_t *this)
{
    int fd;
    int val = 1;
    char loopch = 0;
    struct ifaddrs *ifap, *j;
    struct ip_mreqn group = {{0}};
    struct sockaddr_in addr = {0};

    this->ssdp_start_idx = this->pfds_size;

    // todo
    printf("SSDP sockets:\n");

    if ( getifaddrs(&ifap) != 0 ) {
        return -1;
    }
    for (j = ifap; j != NULL; j = j->ifa_next) {
        if (j->ifa_addr == NULL || j->ifa_addr->sa_family != AF_INET)
            continue;
        if (strcmp(j->ifa_name, "enp2s0f0") != 0) // todo
            continue;

        struct in_addr ip = ((struct sockaddr_in*)(j->ifa_addr))->sin_addr;
        unsigned int idx = if_nametoindex(j->ifa_name);    

        // todo    
        printf("    name: %s (%u), ip: %s\n", j->ifa_name, idx, inet_ntoa(ip));
            
        fd = socket(PF_INET, SOCK_DGRAM, 0);
        this->pfds[this->pfds_size].fd = fd;
        this->pfds[this->pfds_size].events |= POLLIN;
        this->pfds_size++;
        this->ssdp_size++;

        if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) < 0) {
            goto SSDP_init_error;
        }
        if (setsockopt(fd, IPPROTO_IP, IP_MULTICAST_LOOP, &loopch, sizeof(loopch)) < 0) {
            goto SSDP_init_error;
        }
        group.imr_multiaddr.s_addr = inet_addr(IP_FIND);
        group.imr_ifindex = idx;
        if (setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &group, sizeof(struct ip_mreqn)) < 0) {
            goto SSDP_init_error;
        }
        if (setsockopt(fd, IPPROTO_IP, IP_MULTICAST_IF, &group, sizeof(struct ip_mreqn)) < 0) {
            goto SSDP_init_error;
        }

        addr.sin_family = AF_INET;
        addr.sin_port = 0; // any // htons(PORT_FIND);
        addr.sin_addr.s_addr = ip.s_addr;
        if (bind(fd, (struct sockaddr*)&addr, sizeof(struct sockaddr_in)) < 0) {
            goto SSDP_init_error;
        }

        if (this->ssdp_size == SSDP_REQ_MAX) {
            break;
        }
    }
    freeifaddrs(ifap);

    return 0;
SSDP_init_error:
    freeifaddrs(ifap);
    return -1;
}



static ctl_t *init(void)
{
    ctl_t *this = (ctl_t*)calloc(1, sizeof(ctl_t));

    if (UDP_init(this) != 0) {
        goto init_exit;
    }

    if (SSDP_init(this) != 0) {
        goto init_exit;
    }

    if (TCP_client_init(this) != 0) {
        goto init_exit;
    }

    if (TCP_server_init(this) != 0) {
        goto init_exit;
    }

    // thread
	if (pthread_mutex_init(&this->mu, 0) < 0) {
		goto init_exit;
	}
	if (pthread_create(&this->mthread, 0, Loop, (void*)this) < 0) {
		goto init_exit;
	}
	if (pthread_detach(this->mthread) < 0) {
		goto init_exit;
	}

    return this;

init_exit:
    printf("init error\n");
    return 0;
}



int main() 
{
    ctl_t *this = init();
    if (!this) {
        return 1;
    }

    for (;;) {
        usleep(1000000);
    }
}