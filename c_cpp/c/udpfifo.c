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

#include "servto.h"
#include "event.h"

// #ifdef DRVDEBUG
#define TM_NOTE			1
#define TM_WARN			2
#define TM_ERROR		4
#define TM_DUMPRX		8
#define TM_DUMPTX		16
#define TM_TRACE		32
#define TM_USER			64
#define TM_SERV			128
# define drv_debug(m,...) printf("drv: " m "\n",##__VA_ARGS__)
# define drv_log(t,m,...) printf("drv: " m "\n",##__VA_ARGS__)
// #else
// # define drv_debug(m,...) {}
// # define drv_log(t,m,...) {}
// #endif

#define MINIMAL_TO_DEF		500
#define DATAREPEAT_TO_DEF	500
#define ECHOREPEAT_TO_DEF	500
#define CONNECTION_TO_DEF	3000
#define TIMEOUTS_MULT		1000	// множитель мкс -> мс

#define PTHREAD_DEFAULT_STACK_SIZE (0x10000)

#define XRECVBUF_SIZE 1500
#define XSENDBUF_SIZE 1500

#define UDP_MAX		 	1
#define EVENTS_MAX		1
#define PFDS_SIZE_MAX 	(UDP_MAX+EVENTS_MAX)

#define WORK_SOCKET_IDX		0	// индекс основного рабочего дескриптора 
#define EVENT_KILL_IDX		1 	// индекс декскриптора остановки потока

#define eventKill_raise() event_raise(priv->pfds[EVENT_KILL_IDX].fd)
#define eventKill_end() event_end(priv->pfds[EVENT_KILL_IDX].fd)

/* Причины передач */
typedef enum {
	rCycle,				/* 0 - Циклические данные */
	rData,				/* 1 - Данные с гарантией доставки */
	rAck,				/* 2 - Подтверждение данных с гарантией */
} Reason_e;

/* Тип кадра */
typedef enum {
	tNone,				/* 0 - Не использовать */
	tBase,				/* 1 - Основной кадр обмена */
	tEcho,				/* 2 - Кадр эхо */
} FrameType_e;

/* Вторичный тип кадра */
typedef enum {
	stDef,				/* 0 - Тип по умолчанию */
} FrameSType_e;

/* Коды подтверждения */
typedef enum {
	aOk,				/* 0 - Кадр успешно принят */
	aDouble,			/* 1 - Принят кадр-дубль */
	aBusy,				/* 2 - Процесс приема занят */
	aErr,				/* 3 - Ошибка при обработке данных */
} Ack_e;


#pragma pack(push,1)
/* Заголовок кадра */
typedef struct {
	uint8_t number;		/* Нумерация кадра */
	uint8_t reason;		/* Причина передачи @ref Reason_e */
	uint8_t type;		/* Тип кадра @ref FrameType_e */
	uint8_t ack;		/* Код подтверждения @ref Ack_e */
	uint8_t stype;		/* Вторичный тип кадра @ref FrameSType_e */
} UdpFifoHeader_t;

/* Кадр данных */
typedef struct {
	UdpFifoHeader_t header;		/* Заголовок кадра */
	char data[1];				/* Данные */
} UdpFifoFrame_t;
#pragma pack(pop)


/* Мета данные кадра */
typedef struct {
	UdpFifoFrame_t *frame;
	int size;
} Frame_t;


/*
Таймауты на обработку
*/
typedef struct {
	uint32_t dataRepeatTO;
	uint32_t echoRepeatTO;
	uint32_t connectionTO;
	uint32_t minimalTO;		// минимальный таймаут, в случае отсутствия других	
} Timeouts_t;

/*
Идентификаторы таймаутов
*/
enum {
	eDataRepeatTO,
	eEchoRepeatTO,
	eConnectionTO,
};


typedef struct PrivateStruct_s PrivateStruct_t;
struct PrivateStruct_s {

	


	pthread_t mthread;		// основной поток
	pthread_mutex_t mu;		// мьютекс основного потока 

	/* управляющие дескрипторы */
	struct pollfd pfds[PFDS_SIZE_MAX];
	int pfdsSize;			// размер pfds

	struct sockaddr_in lastAddr; // адрес устройства, с которым взаимодействовали последний раз
	int lastFd;

	Timeouts_t timeouts;	// таймауты
	to_que_t to_que;		// очередь таймаутов
	to_item_t dataRepeatTO;	// таймаут на переповтор отправки кадра data
	to_item_t echoRepeatTO;	// таймаут на переповтор отправки кадра echo
	to_item_t connectionTO;	// таймаут соединения

	Frame_t rxFrame;		// буфер приема
	Frame_t txFrame;		// буфер отправки

	/* нумерация кадров */
	uint8_t rdnum;
	uint8_t wrnum;
	uint8_t rdone;

	uint8_t isBusy;
	uint8_t initDone;
	uint8_t running;
	uint8_t link;

	uint32_t cntRepeat;
};


const int RECVBUF_SIZE_MAX = (XRECVBUF_SIZE-sizeof(UdpFifoHeader_t));
const int SENDBUF_SIZE_MAX = (XSENDBUF_SIZE-sizeof(UdpFifoHeader_t));


static uint32_t getMinTo(PrivateStruct_t *priv)
{
	uint32_t to = to_poll(&priv->to_que);
	if (to > 0) {
		to /= 1000;
		to++;
	}
	if (to == -1 || to > priv->timeouts.minimalTO) {
		to = priv->timeouts.minimalTO;
	}
	return to;
}


/// Проверка нумерации кадра на приеме
//\ param num - нумерация кадра
//\ return - признак дубля пакета 1-дубль,0-уникален
static Ack_e rdnum(PrivateStruct_t *priv, uint8_t num, int busy)
{
	Ack_e dub;
	if (num) {
		dub = (priv->rdnum == num)? 
			aDouble : aOk;
		priv->rdone = 0;
	} else {
		dub = (priv->rdone == 0)?
			aOk : aDouble;
		priv->rdone = 1;
	}
	if (!busy)
		priv->rdnum = num;
	else
		dub = aBusy;
	
	return dub;
}


/// Установка нумерации кадра на передачу
//\ return - номер передающего пакета
static uint8_t wrnum(PrivateStruct_t *priv)
{
	uint8_t num = priv->wrnum;
	if (!num) {
		priv->wrnum = 2;
	} else {
		priv->wrnum++;
		if (!priv->wrnum) {
			priv->wrnum = 1;
		}
	}
	return num;
}


static int sendRawByIpPort(int sock, const char *ip, uint16_t port, const void *buf, int size)
{
	static struct sockaddr_in addr = {0};
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = inet_addr(ip);
	int res = sendto(sock, buf, size, 0, (const struct sockaddr *)&addr, sizeof(struct sockaddr_in));
	return res;
}


static int sendRawByAddr(int sock, struct sockaddr_in *addr, const void *buf, int size)
{
	addr->sin_family = AF_INET;
	int res = sendto(sock, buf, size, 0, (const struct sockaddr *)addr, sizeof(struct sockaddr_in));
	return res;
}


static inline int resetSendBuf(PrivateStruct_t *priv)
{
	priv->txFrame.size = 0;
}


static inline int copyToSend(PrivateStruct_t *priv, const void *buf, int size)
{
	if (buf && size) {
		int rem = SENDBUF_SIZE_MAX - priv->txFrame.size;
		int sz = (size > rem)? rem : size;
		memcpy(priv->txFrame.frame->data + priv->txFrame.size, buf, sz);
		priv->txFrame.size += sz;
		return sz;
	}
	return 0;
}


static int sendPraparedFrame(PrivateStruct_t *priv, int sendFd, struct sockaddr_in *addr)
{
	to_reappend(&priv->to_que, &priv->dataRepeatTO, priv->timeouts.dataRepeatTO * TIMEOUTS_MULT);
	return ( sendRawByAddr(sendFd, addr, priv->txFrame.frame, priv->txFrame.size + sizeof(UdpFifoHeader_t)) > 0 )?
		0 : -1;
}


static void prepareToSendFrame(PrivateStruct_t *priv, Frame_t *fmeta, FrameType_e ftype, Reason_e reason)
{
	UdpFifoHeader_t *header = &fmeta->frame->header;
	header->number = wrnum(priv);
	header->type = ftype;
	header->reason = reason;
	header->ack = aOk;
	/* data already inside */
	priv->isBusy = 1;
}


static int sendBaseFrame(PrivateStruct_t *priv, int sendFd, struct sockaddr_in *addr)
{
	if (!priv || !addr)
		return -EINVAL;
	priv->lastAddr = *addr;
	priv->lastFd = sendFd;
	prepareToSendFrame(priv, &priv->txFrame, tBase, rData);
	return sendPraparedFrame(priv, sendFd, addr);
}


static int sendEchoFrame(PrivateStruct_t *priv, struct sockaddr_in *addr)
{
	if (!priv || !addr)
		return -1;
	prepareToSendFrame(priv, &priv->txFrame, tEcho, rCycle);
	to_reappend(&priv->to_que, &priv->echoRepeatTO, priv->timeouts.echoRepeatTO * TIMEOUTS_MULT);
	return ( sendRawByAddr(priv->pfds[WORK_SOCKET_IDX].fd, addr, 
		priv->txFrame.frame, priv->txFrame.size + sizeof(UdpFifoHeader_t)) > 0 )?
		0 : -1;
}


static int sendAck(int fd, struct sockaddr_in *addr, uint8_t frameNum, FrameType_e type, Ack_e ack)
{
	UdpFifoHeader_t ans;
	ans.number = frameNum;
	ans.reason = rAck;
	ans.type = type;
	ans.ack = ack;
	return sendRawByAddr(fd, addr, &ans, sizeof(UdpFifoHeader_t));
}


static void setLink(PrivateStruct_t *priv, int link)
{
	if (!priv->link) {
	}
	priv->link = link;
}


static void disconnect(PrivateStruct_t *priv) 
{
	setLink(priv, 0);
	to_delete(&priv->to_que, &priv->dataRepeatTO);
	to_delete(&priv->to_que, &priv->echoRepeatTO);
	to_delete(&priv->to_que, &priv->connectionTO);
	bzero(&priv->lastAddr, sizeof(priv->lastAddr));
}


static void *mainLoop(void *data)
{
	PrivateStruct_t *priv = (PrivateStruct_t *)data;
	const size_t rbuf_size = RECVBUF_SIZE_MAX;
	char rbuf[rbuf_size];

	while (!priv->running) {
		usleep(1000);
	}

	int res, i;
	while (priv->running) {
		res = poll(priv->pfds, priv->pfdsSize, getMinTo(priv));
		// events
		if (res > 0) {
			for (i = 0; i < priv->pfdsSize; ++i) {
				if (priv->pfds[i].revents & POLLIN) {
					switch (i) {
						// sys events
						case EVENT_KILL_IDX: {
							eventKill_end();
							pthread_exit(0);
						} break;
						// network events
						default: {
							// get frame
							struct sockaddr_in addr;
							socklen_t addr_len = sizeof(struct sockaddr_in);
							res = recvfrom(priv->pfds[i].fd, rbuf, rbuf_size, 0, (struct sockaddr *)&addr, &addr_len);
							if (res <= 0) {
								continue;
							}
							// char *ip = inet_ntoa(*(struct in_addr*)&addr.sin_addr.s_addr);
							// uint16_t port = htons(addr.sin_port);

							// handle frame
							pthread_mutex_lock(&priv->mu);
							switch (i) {
								case WORK_SOCKET_IDX: {
									UdpFifoFrame_t *rxframe = (UdpFifoFrame_t *)&rbuf;
									UdpFifoHeader_t *header = &rxframe->header;
									uint8_t framenum = header->number; // for ans & ack									

									// connection ok
									setLink(priv, 1);
									to_reappend(&priv->to_que, &priv->connectionTO, priv->timeouts.connectionTO * TIMEOUTS_MULT);

									int busy = (priv->rxFrame.size != 0);
									Ack_e ack = (header->reason != rAck)?
											rdnum(priv, header->number, busy) : aOk; // compute rdnum and set ack for ans								

									switch (header->type) {
										case tEcho: {
											switch (header->reason) {
												case rData: {
													sendAck(priv->pfds[i].fd, &addr, framenum, header->type, ack);
												} break;
												case rAck: {
													if (priv->txFrame.frame->header.type == tEcho) {
														priv->isBusy = 0;
														to_delete(&priv->to_que, &priv->echoRepeatTO);
													}
												} break;
											}
										} break;
										case tBase: {
											switch (header->reason) {
												case rData: /* need to ans */ {
													sendAck(priv->pfds[i].fd, &addr, framenum, header->type, ack);
												} // no break
												case rCycle: /* without ans */ {
													if (ack == aOk) {
														switch (header->stype) {
															case stDef: {
																priv->rxFrame.size = res - sizeof(UdpFifoHeader_t);
																memcpy(priv->rxFrame.frame->data, rxframe->data, priv->rxFrame.size);
															} break;
														}
													}							
												} break;
												case rAck: /* timeup */ {
													switch (header->ack) {
														case aDouble:
														case aOk: {
															UdpFifoHeader_t *header = &priv->txFrame.frame->header;
															if (header->number == framenum) {
																priv->isBusy = 0;
																to_delete(&priv->to_que, &priv->dataRepeatTO);
															}
														} break;
													}
												} break;
											}
										} break;
									}									
								} break;
							}
							pthread_mutex_unlock(&priv->mu);
						} break;
					}
				}
			}
		} // res != 0
		// timeouts
		else if (res == 0) {
			for (;;) {
				to_item_t *item = to_check(&priv->to_que);
				if (!item) break;
				pthread_mutex_lock(&priv->mu);
				to_delete(&priv->to_que, item);
				switch (item->id) {
					case eDataRepeatTO: {
						priv->cntRepeat++;
						sendBaseFrame(priv, priv->lastFd, &priv->lastAddr);
					} break;
					case eEchoRepeatTO: {
						sendEchoFrame(priv, &priv->lastAddr);
					} break;
					case eConnectionTO: {
						setLink(priv, 0);
					} break;
					default: {} break;
				}
				pthread_mutex_unlock(&priv->mu);
			}
		}
	}
}




int drv_get_data(PrivateStruct_t *priv, char *buf, int bufSz)
{
	if (!priv || !buf) {
		return -EINVAL;
	}

	pthread_mutex_lock(&priv->mu);
	int ret = (bufSz < priv->rxFrame.size)? 
			bufSz : priv->rxFrame.size;

	if (ret > 0) { // todo circle buffer
		int remainder = priv->rxFrame.size - ret;
		memcpy(buf, priv->rxFrame.frame->data, ret);
		if (remainder > 0) {
			char tmp[RECVBUF_SIZE_MAX];
			memcpy(tmp, priv->rxFrame.frame->data + ret, remainder);
			memcpy(priv->rxFrame.frame->data, tmp, remainder);
		}
		priv->rxFrame.size = (remainder > 0)? 
			remainder : 0;
	}
	pthread_mutex_unlock(&priv->mu);

	return ret;
}


int drv_send_data_to_host(PrivateStruct_t *priv, uint32_t ip, uint16_t port, const char *buf, int bufSz)
{
	int ret = 0;
	if (!priv || !ip || !buf) {
		return -EINVAL;
	}
	if (priv->isBusy) {
		return -EBUSY;
	}

	pthread_mutex_lock(&priv->mu);
	resetSendBuf(priv);
	if (copyToSend(priv, buf, bufSz) > 0) {
		static struct sockaddr_in addr = {0};
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);
		addr.sin_addr.s_addr = ip;
		ret = sendBaseFrame(priv, priv->pfds[WORK_SOCKET_IDX].fd, &addr);
	}
	pthread_mutex_unlock(&priv->mu);

	return ret;
}


int drv_send_data(PrivateStruct_t *priv, const char *buf, int bufSz)
{
	return drv_send_data_to_host(priv, priv->lastAddr.sin_addr.s_addr, htons(priv->lastAddr.sin_port), buf, bufSz);
}


int drv_init(PrivateStruct_t *priv, uint32_t ip, uint16_t port)
{
	int fd;

	if (!priv || priv->initDone) {
		return -1;
	}

	// working sock
	{
		struct sockaddr_in localSock = {0};

		fd = socket(PF_INET, SOCK_DGRAM, 0);
		priv->pfds[WORK_SOCKET_IDX].fd = fd;
		priv->pfds[WORK_SOCKET_IDX].events = POLLIN;
		priv->pfdsSize++;

		localSock.sin_family = AF_INET;
		localSock.sin_port = htons(port);
		localSock.sin_addr.s_addr = ip;

		if (bind(fd, (struct sockaddr*)&localSock, sizeof(struct sockaddr_in)) < 0) {
			drv_log(TM_ERROR, "drv_init bind failed: %s", strerror(errno));
			return -1;
		}
	}

	// other fds
	{
		// kill thread fd
		event_to_pollfd(event_create(), &priv->pfds[EVENT_KILL_IDX]);
		priv->pfdsSize++;
	}

	// инициализация таймаутов
	{
		uint32_t i, min;
		uint32_t *t = (uint32_t*)(&priv->timeouts);
		const uint32_t sz = sizeof(Timeouts_t)/sizeof(uint32_t);
		memset(t, 0xFF, sizeof(Timeouts_t));
		to_initque(&priv->to_que);
		to_inititem(&priv->dataRepeatTO, 0, eDataRepeatTO);
		to_inititem(&priv->echoRepeatTO, 0, eEchoRepeatTO);
		to_inititem(&priv->connectionTO, 0, eConnectionTO);
		priv->timeouts.dataRepeatTO = DATAREPEAT_TO_DEF;
		priv->timeouts.echoRepeatTO = ECHOREPEAT_TO_DEF;
		priv->timeouts.connectionTO = CONNECTION_TO_DEF;
		priv->timeouts.minimalTO = MINIMAL_TO_DEF;
		for (i = 0; i < sz; ++i) {
			min = t[i];
			if (priv->timeouts.minimalTO > min) {
				priv->timeouts.minimalTO = min;
			}
		}
	}

	// инициализация потока
	{
		priv->running = 1; // run thread
		if (pthread_mutex_init(&priv->mu, 0) < 0) {
			drv_log(TM_ERROR, "drv_init pthread_mutex_init failed: %s", strerror(errno));
			return -1;
		}
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_attr_setstacksize(&attr, PTHREAD_DEFAULT_STACK_SIZE);
		if (pthread_create(&priv->mthread, &attr, mainLoop, (void*)priv) < 0) {
			drv_log(TM_ERROR, "drv_init pthread_create failed: %s", strerror(errno));
			return -1;
		}
	}

	priv->initDone = 1;
	return 0;
}


int drv_deinit(PrivateStruct_t *priv)
{
	int i;
	if (priv && priv->initDone) {
		eventKill_raise();
		pthread_join(priv->mthread, 0);
		for (i = 0; i < priv->pfdsSize; ++i) {
			close(priv->pfds[i].fd);
		}
		pthread_mutex_destroy(&priv->mu);
	}
	priv->initDone = 0;
}


PrivateStruct_t *drv_create()
{
	PrivateStruct_t *priv = (PrivateStruct_t*)calloc(1, sizeof(PrivateStruct_t));

	if (priv) {
		priv->rxFrame.frame = (UdpFifoFrame_t *)calloc(1, XRECVBUF_SIZE);
		if (!priv->rxFrame.frame) {
			drv_log(TM_ERROR, "drv_create rxFrame failed: %s", strerror(errno));
			return priv;
		}
		priv->txFrame.frame = (UdpFifoFrame_t *)calloc(1, XSENDBUF_SIZE);
		if (!priv->txFrame.frame) {
			drv_log(TM_ERROR, "drv_create txFrame failed: %s", strerror(errno));
			return priv;
		}
	} else {
		drv_log(TM_ERROR, "drv_create priv failed: %s", strerror(errno));
	}

	return priv;
}


void drv_free(PrivateStruct_t *priv)
{
	if (priv) {
		drv_deinit(priv);
		free(priv->rxFrame.frame);
		free(priv->txFrame.frame);
		free(priv);
	}
}





int drv_get_data(PrivateStruct_t *priv, char *buf, int bufSz);
int drv_send_data_to_host(PrivateStruct_t *priv, uint32_t ip, uint16_t port, const char *buf, int bufSz);
int drv_send_data(PrivateStruct_t *priv, const char *buf, int bufSz);
int drv_init(PrivateStruct_t *priv, uint32_t ip, uint16_t port);
int drv_deinit(PrivateStruct_t *priv);
PrivateStruct_t *drv_create();
void drv_free(PrivateStruct_t *priv);


int main(int argc, char **argv)
{
	PrivateStruct_t *priv = drv_create();

	if (argc < 6) {
		printf("args: myip myport hip hport msg\n");
		return 1;
	}
	uint32_t myip = inet_addr(argv[1]);
	uint16_t myport = atoi(argv[2]);

	drv_init(priv, myip, myport);

	uint32_t hip = inet_addr(argv[3]);
	uint16_t hport = atoi(argv[4]);

	const char *msg = argv[5];

	char buf[1500] = "";
	int cnt = 0;	
	for (;;) {
		drv_get_data(priv, buf, 1500);
		if (strlen(buf)) {
			printf("msg: %s\n", buf);
		}
		usleep(1000000);
		sprintf(buf, "%s ; my cnt=%d", msg, cnt++);
		drv_send_data_to_host(priv, hip, hport, buf, strlen(buf)+1);
		strcpy(buf, "");
	}

	drv_free(priv);
	return 0;
}