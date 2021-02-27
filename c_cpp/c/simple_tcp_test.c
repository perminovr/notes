
#include <ctype.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <pthread.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ifaddrs.h>
#include <errno.h>
#include <string.h>
#include <sys/poll.h>
#include <sys/eventfd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/prctl.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <stdarg.h>
#include <semaphore.h>
#include <unistd.h>

int main(int argc, char **argv)
{
	int fd;
	int rc;
	int i;
	char buf[1024];
	struct sockaddr_in server;
	struct sockaddr_in client;

	if (argc > 1) {
		fd = socket(AF_INET, SOCK_STREAM, 0);

		if (strcmp(argv[1], "client") == 0) {
			// client.sin_addr.s_addr = inet_addr("20.21.0.3");
			// client.sin_family = AF_INET;
			// client.sin_port = htons( 0 );
			// if( bind(fd, (struct sockaddr *)&client, sizeof(client)) < 0)
			// 	{ goto exit; }
			server.sin_addr.s_addr = inet_addr("10.14.0.91");
			server.sin_family = AF_INET;
			server.sin_port = htons( 4444 );
			if ( connect(fd, (struct sockaddr *)&server, sizeof(server)) < 0)
				{ goto exit; }
			for (;;) {
				usleep(1000*1000);
				strcpy(buf, "hello to server");
				rc = send(fd, buf, strlen(buf), 0);
				if (rc < 0) printf("failed to send: %s\n", strerror(errno));
				rc = recv(fd, buf, sizeof(buf), 0);
				if (rc > 0) {
					printf("server: %s\n", buf);
				}
			}
		} else {
			server.sin_addr.s_addr = inet_addr("20.21.0.2");
			server.sin_family = AF_INET;
			server.sin_port = htons( 4444 );
			if( bind(fd, (struct sockaddr *)&server, sizeof(server)) < 0)
				{ goto exit; }
			listen(fd, 1);
			for (;;) {
				socklen_t addrlen = sizeof(client);
				int cfd = accept(fd, (struct sockaddr *)&client, &addrlen);
				if (cfd >= 0) {
					printf("client accepted\n");
					for (;;) {
						rc = recv(cfd, buf, sizeof(buf), 0);
						if (rc > 0) {
							printf("client: %s\n", buf);
							strcpy(buf, "hello to client");
							rc = send(cfd, buf, strlen(buf), 0);
							if (rc < 0) printf("failed to send: %s\n", strerror(errno));
						} else if (rc < 0) {
							printf("close client\n");
							close(cfd);
							break;
						}
					}
				}
			}
		}
	}
	printf("exit\n");
	return 0;
exit:
	printf("failed: %s \n", strerror(errno));
	return 1;
}
