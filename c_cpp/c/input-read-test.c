#include <unistd.h>
#include <ioctl.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>

static char* device = "/dev/input/event7"

typedef struct {
      struct timeval time;
      unsigned short type;
      unsigned short code;
      unsigned int value; 
} input_event_t;


int main() {
	input_event_t ie;
	int fd, res;
	timeval tv;
	fd_set rfds;
	
	fd = open("/dev/input/event3", O_RDWR | O_NOCTTY);

	while (1) {
		tv.tv_sec = 1;
		tv.tv_usec = 1000;
		FD_ZERO(&rfds);
		FD_SET(fd, &rfds);
		res = select(fd + 1, &rfds, NULL, NULL, &tv);
		if (res > 0) {
			res = read(fd, &ie, sizeof(input_event_t));
			printf("");
		}
	}

	return 0;
}
