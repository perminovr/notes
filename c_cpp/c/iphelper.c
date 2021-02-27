
#if defined (__cplusplus)
#include <iostream>
#include <exception>
#endif

#include <getopt.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <string.h>


static int Usage();
typedef struct option option_t;


static void print(const char *msg)
{
#if defined (__cplusplus)
	std::cout << msg << std::endl;
#else
	printf("%s\n", msg);
#endif
}

static int _atoi(const char *str, int defaultVal)
{
#if defined (__cplusplus)
	int ret;	
	try {
		ret = atoi(str);
	} catch (std::exception &ex) {
		ret = defaultVal;
	}
	return ret;
#else
    (void)defaultVal;
	return atoi(str);
#endif
}

static uint8_t maskToPrefix(const char *strMask)
{
	uint8_t ret;
	uint32_t prefix = 0;
	uint32_t mask = 0;
	if (strMask) {
		if (strlen(strMask) <= 2) {
			prefix = _atoi(strMask, 32);
			mask = (0xFFFFFFFF << (32 - prefix)) & 0xFFFFFFFF;
			mask = htonl(mask);
		} else {
			mask = ntohl(inet_addr(strMask));
			for (int i = 31; i >= 0; --i) {
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

static int prefixToMask(int prefix, char *buf)
{
	if (prefix >= 0 && prefix <= 32) {
		uint32_t mask = prefix ? (0xFFFFFFFF << (32 - prefix)) & 0xFFFFFFFF : 0;
		mask = htonl(mask);
		return inet_ntop(AF_INET, &mask, buf, INET_ADDRSTRLEN)? 0 : -1;
	}
	return 0;
}

static uint32_t broadcast(const char *ip, uint8_t prefix)
{
	uint32_t ret = 0;
	if (ip) {
		ret = htonl(inet_addr(ip));
		if (ret) {
			for (int i = 0; i < 32-prefix; ++i) {
				ret |= (1 << i);
			}
		}
		ret = htonl(ret);
	}
	return ret;
}

static uint32_t subnet(const char *ip, uint8_t prefix)
{
	uint32_t mask = (0xFFFFFFFF << (32-prefix));
	return htonl(htonl(inet_addr(ip)) & mask);
}

int main(int argc, char **argv)
{
	option_t loptions[] = {
		{"operation", required_argument, 0, 'o'},
		{"ip", required_argument, 0, 'i'},
		{"mask", required_argument, 0, 'm'},
		// {"ddddd", no_argument, 0, 'd'},
		{0, 0, 0, 0}
	};

	const char 
		*operation = 0,
		*ip = 0,
		*mask = 0;

	while (1) {
		int option_index = 0;
		int c = getopt_long(argc, argv, "o:i:m:", loptions, &option_index);
		if (c == -1)
			break;
		switch (c) {
			case 'o': { operation = optarg; } break;
			case 'i': { ip = optarg; } break;
			case 'm': { mask = optarg; } break;
			default: { } break;
		}
	}

	if (!operation) return Usage();
	char print_buf[32] = "";

	if (strcmp(operation, "ptm") == 0) {
		if (!mask) return Usage();
		if (strlen(mask) > 2) return Usage();
		prefixToMask(_atoi(mask, 32), print_buf);
	} else

	if (strcmp(operation, "mtp") == 0) {
		if (!mask) return Usage();
		if (strlen(mask) < 7) return Usage();
		uint8_t prefix = maskToPrefix(mask);
		sprintf(print_buf, "%u", prefix);
	} else

	if (strcmp(operation, "sn") == 0) {
		if (!ip || !mask) return Usage();
		if (strlen(ip) < 7) return Usage();
		uint8_t prefix = (strlen(mask) <= 2)?
			_atoi(mask, 32) : maskToPrefix(mask);
		uint32_t res = subnet(ip, prefix);
		strcpy(print_buf, inet_ntoa( *((struct in_addr*)&res) ));
	} else

	if (strcmp(operation, "brd") == 0) {
		if (!ip || !mask) return Usage();
		if (strlen(ip) < 7) return Usage();
		uint8_t prefix = (strlen(mask) <= 2)?
			_atoi(mask, 32) : maskToPrefix(mask);
		uint32_t res = broadcast(ip, prefix);
		strcpy(print_buf, inet_ntoa( *((struct in_addr*)&res) ));
	}

	if (!strlen(print_buf)) return Usage();

	print(print_buf);
	return 0;
}


static int Usage()
{
	print(
			"Usage:\n"
			"	-o|--operation\n"
			"		ptm		prefix to mask: requires --mask PREFIX\n"
			"		mtp		mask to prefix: requires --mask IP MASK\n"
			"		sn		subnet: requires --ip IP --mask IP MASK or --ip IP --mask PREFIX\n"
			"		brd		broadcast: requires --ip IP --mask IP MASK or --ip IP --mask PREFIX\n"
			"	-i|--ip IP ADDRESS\n"
			"	-m|--mask IP MASK or PREFIX\n"
	);
	return -1;
}
