#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <ifaddrs.h>
#include <linux/if_packet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define PIPE_NAME "/tmp/test-npipe"

int main()
{
	unlink(PIPE_NAME);
	mkfifo(PIPE_NAME, S_IRUSR | S_IWUSR);
	int fd1 = open(PIPE_NAME, O_RDWR);
	int rc = write(fd1, "123", 3);
	int fd2 = open(PIPE_NAME, O_RDONLY | O_NONBLOCK);
	rc = write(fd1, "456", 4);
	char buf[64] = {0};
	rc = read(fd2, buf, 64);
	printf("%s\n", buf);
	return rc;
}