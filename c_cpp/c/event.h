#include <sys/poll.h>
#include <sys/eventfd.h>
#include <unistd.h>

#define EVENT_DSIZE 8

static inline int event_create()
{
	return eventfd(0, 0);
}

static inline void event_to_pollfd(int fd, struct pollfd *pfd)
{
	if (pfd) {
		pfd->fd = (fd>0)? fd : 0;
		pfd->events = POLLIN;
	}
}

static inline int event_raise(int fd)
{
	uint8_t buf = 1;
	return write(fd, &buf, EVENT_DSIZE);
}
#define eventX_raise() event_raise(-1)

static inline int event_end(int fd)
{
	uint8_t buf[EVENT_DSIZE];
	return read(fd, &buf, EVENT_DSIZE);
}
#define eventX_end() event_end(-1)