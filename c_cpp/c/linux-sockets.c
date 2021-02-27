#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <netinet/in.h>       
#include <sys/ioctl.h>
#include <net/if.h>
#include <string.h>


char rawbuf[255];


char msg[] = "\x4e\x4f\x54\x49\x46\x59\x20\x2a\x20\x48\x54\x54\x50\x2f\x31\x2e\x31\x0d\x0a\x48\x4f\x53\x54\x3a\x20\x32\x33\x39\x2e\x32\x35\x35\x2e\x32\x35\x35\x2e\x32\x35\x30\x3a\x31\x39\x30\x30\x0d\x0a\x43\x41\x43\x48\x45\x2d\x43\x4f\x4e\x54\x52\x4f\x4c\x3a\x20\x6d\x61\x78\x2d\x61\x67\x65\x3d\x31\x30\x30\x0d\x0a\x4c\x4f\x43\x41\x54\x49\x4f\x4e\x3a\x20\x68\x74\x74\x70\x3a\x2f\x2f\x31\x39\x32\x2e\x31\x36\x38\x2e\x31\x2e\x31\x3a\x31\x39\x30\x30\x2f\x69\x67\x64\x2e\x78\x6d\x6c\x0d\x0a\x4e\x54\x3a\x20\x75\x75\x69\x64\x3a\x39\x66\x30\x38\x36\x35\x62\x33\x2d\x66\x35\x64\x61\x2d\x34\x61\x64\x35\x2d\x38\x35\x62\x37\x2d\x37\x34\x30\x34\x36\x33\x37\x66\x64\x66\x33\x37\x0d\x0a\x4e\x54\x53\x3a\x20\x73\x73\x64\x70\x3a\x61\x6c\x69\x76\x65\x0d\x0a\x53\x45\x52\x56\x45\x52\x3a\x20\x69\x70\x6f\x73\x2f\x37\x2e\x30\x20\x55\x50\x6e\x50\x2f\x31\x2e\x30\x20\x54\x4c\x2d\x57\x52\x39\x34\x31\x4e\x2f\x32\x2e\x30\x0d\x0a\x55\x53\x4e\x3a\x20\x75\x75\x69\x64\x3a\x39\x66\x30\x38\x36\x35\x62\x33\x2d\x66\x35\x64\x61\x2d\x34\x61\x64\x35\x2d\x38\x35\x62\x37\x2d\x37\x34\x30\x34\x36\x33\x37\x66\x64\x66\x33\x37\x0d\x0a\x0d\x0a";



int func1(char *ip)
{
	int sock;
	struct sockaddr_in addr;

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) {
		perror("socket");
		return (1);
	}

	struct ifreq ifr;

	memset(&ifr, 0, sizeof(ifr));
	snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "eth1");

	addr.sin_family = AF_INET;
	addr.sin_port = htons(1900);
	addr.sin_addr.s_addr = inet_addr(ip);

	ifr.ifr_broadaddr = (*((struct sockaddr *)&addr));
	if (setsockopt(sock, SOL_SOCKET, SO_BINDTODEVICE, (void *)&ifr, sizeof(ifr)) < 0) {
	    	printf("bind\n");
		return 2;
	}

	int broadcastEnable=1;

	if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable)) < 0) {
		printf("brcst\n");
		return 3;
	}

	addr.sin_family = AF_INET;
	addr.sin_port = htons(1900);
	addr.sin_addr.s_addr = inet_addr(ip);

	if ( connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0 ) {
		printf("connect\n");
		return 23;
	}

	send(sock, msg, sizeof(msg), 0);

	close(sock);

	return 0;
}


int func2(char *ip)
{
	int sock;
	struct sockaddr_in addr;

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) {
		perror("socket");
		return (1);
	}

	struct ifreq ifr;

	memset(&ifr, 0, sizeof(ifr));
	snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "eth1");

	ifr.ifr_broadaddr = (*((struct sockaddr *)&addr));
	if (setsockopt(sock, SOL_SOCKET, SO_BINDTODEVICE, (void *)&ifr, sizeof(ifr)) < 0) {
	    	printf("bind\n");
		return 2;
	}

	addr.sin_family = AF_INET;
	addr.sin_port = htons(1900);
	addr.sin_addr.s_addr = inet_addr(ip);

	if ( connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0 ) {
		printf("connect\n");
		return 23;
	}

	send(sock, msg, sizeof(msg), 0);

	close(sock);

	return 0;
}


int func3(char *ip)
{
	int sock;
	struct sockaddr_in addr;

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) {
		perror("socket");
		return (1);
	}

	struct ifreq ifr;

	memset(&ifr, 0, sizeof(ifr));
	snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "eth1");

	ifr.ifr_broadaddr = (*((struct sockaddr *)&addr));
	if (setsockopt(sock, SOL_SOCKET, SO_BINDTODEVICE, (void *)&ifr, sizeof(ifr)) < 0) {
	    	printf("bind\n");
		return 2;
	}

	addr.sin_family = AF_INET;
	addr.sin_port = htons(5001);
	addr.sin_addr.s_addr = inet_addr(ip);

	if ( connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0 ) {
		printf("connect\n");
		return 23;
	}

	send(sock, msg, sizeof(msg), 0);

	close(sock);

	return 0;
}


int main()
{
	func1("192.168.255.255");
	usleep(1000000);
	return 0;
}

/* server
int main()
{
	int sock, listener;
	struct sockaddr_in addr;
	char buf[1024];
	int bytes_read;
	int i;

	for (i = 0; i < 255; i++)
		rawbuf[i] = (char)i;

	listener = socket(AF_INET, SOCK_STREAM, 0);
	if (listener < 0) {
		perror("socket");
		return 1;
	}

	addr.sin_family = AF_INET;
	addr.sin_port = htons(5001);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(listener, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("bind");
		return 2;
	}

	listen(listener, 1);

	while (1)
	{
		sock = accept(listener, NULL, NULL);
		if (sock < 0) {
		    perror("accept");
		    return 3;
		}

		while(1) {
		    	bytes_read = recv(sock, buf, 1024, 0);
		    	if (bytes_read <= 0) 
				break;
			printf("recvd %d\n", bytes_read);
			for (i = 0; i < 255; i++) {
				if (buf[i] != (char)i) {
					printf("recv byte fail at: %d\nvalue: %d\n", i, buf[i]);
					break;
				}
			}
			send(sock, rawbuf, 255, 0);
		}

		close(sock);
	}

	return 0;
}*/
