
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/select.h>
#include <linux/input.h> // коды и типы

static const char* device = "/dev/input/event7";

// структура события
typedef struct {
      struct timeval time; 	// время поступления события (с, мкс)
      unsigned short type;  // тип:
      unsigned short code;  // код:
      unsigned int value;   // значение:
} input_event_t;


int main() {
	input_event_t ie;
	int fd, res;
	// для организации неблокирубщего чтения
	struct timeval tv;
	fd_set rfds;

	// открываем эвенет канал
	fd = open(device, O_RDWR | O_NOCTTY);

	while (1) {
		// не ждем, а берем только готовое
		tv.tv_sec = 0;
		tv.tv_usec = 0;
		// сброс и установка
		FD_ZERO(&rfds);
		FD_SET(fd, &rfds);
		// проверка на поступление события
		res = select(fd + 1, &rfds, NULL, NULL, &tv);
		if (res > 0) {
			// чтение события
			res = read(fd, &ie, sizeof(input_event_t));
			printf("time(s): %d, time(us): %d, type: %d, code: %d, val: %d\n",
					ie.time.tv_sec,
					ie.time.tv_usec,
					ie.type,
					ie.code,
					ie.value);
		}
		usleep(100);
	}

	return 0;
}


// ********************************************************************************************** //



#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <linux/input.h> // коды и типы
#include <poll.h>

static const char* device = "/dev/input/event7";

// структура события
typedef struct {
      struct timeval time; 	// время поступления события (с, мкс)
      unsigned short type;  // тип:
      unsigned short code;  // код:
      unsigned int value;   // значение:
} input_event_t;


int main() {
	int res;
	input_event_t ie;
	// для организации неблокирубщего чтения
	struct pollfd pfd;
	int timeout_msecs = 0;

	// открываем эвенет канал
	pfd.fd = open(device, O_RDWR | O_NOCTTY);
	pfd.events = POLLIN | POLLRDNORM | POLLRDBAND | POLLPRI; // входящие события (POLLIN достаточно)

	while (1) {
		res = poll(&pfd, 1, timeout_msecs);
		if (res > 0) {
			// чтение события
			res = read(pfd.fd, &ie, sizeof(input_event_t));
			printf("time(s): %d, time(us): %d, type: %d, code: %d, val: %d\n",
					ie.time.tv_sec,
					ie.time.tv_usec,
					ie.type,
					ie.code,
					ie.value);
		}
		usleep(100);
	}

	return 0;
}


