#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <assert.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>

#define mt_log(m,...) printf("mramtest: " m "\n",##__VA_ARGS__)

#define MRAM_ADDR	0x3000000
#define MRAM_SZ		(4096*512)

typedef struct {
	int fd;
	unsigned int *self;
	size_t sz;
} mram_t;

int mmap_mram(mram_t *mram)
{
	int rc;
	int fd = open("/dev/mem", O_RDWR | O_SYNC);
	if (fd == -1) {
		mt_log("mmap_mram -- /dev/mem : %s\n", strerror(errno));
		return fd;
	} else {
		off_t *filesize;
		unsigned int page_mask;
		unsigned int page_size;
		unsigned int *mem_pointer;
		unsigned int alloc_mem_size;

		page_size=getpagesize();
		alloc_mem_size=(((MRAM_SZ/page_size)+1)*page_size);
		page_mask=(page_size - 1);

		mem_pointer = mmap(NULL, alloc_mem_size,
			PROT_READ | PROT_WRITE,MAP_SHARED,
			fd, (MRAM_ADDR & ~page_mask));

		if (mem_pointer == MAP_FAILED) {
			close(fd);
			mt_log("mmap_mram -- /dev/mem : %s\n", strerror(errno));
			return -1;
		}

		mram->fd = fd;
		mram->self = mem_pointer;
		mram->sz = alloc_mem_size;
	}

	return 0;
}

size_t write_mram(mram_t *mram, size_t offset, const char *data, size_t size)
{
	size_t rem = mram->sz - offset;
	size_t sz = (size > rem)? rem : size;
	unsigned char *addr = ((unsigned char*)mram->self) + offset;
	memcpy(addr, data, sz);
	return sz;
}

size_t read_mram(mram_t *mram, size_t offset, char *data, size_t size)
{
	size_t rem = mram->sz - offset;
	size_t sz = (size > rem)? rem : size;
	unsigned char *addr = ((unsigned char*)mram->self) + offset;
	memcpy(data, addr, sz);
	return sz;
}

int munmap_mram(mram_t *mram)
{
	int rc;
	rc = munmap(mram->self, mram->sz);
	rc = close(mram->fd);
	return rc;
}

int main(int argc, char **argv)
{
	return 0;
}