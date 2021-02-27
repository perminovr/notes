#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>

#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#define container_of(ptr, type, member) ({                      \
        const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
        (type *)( (char *)__mptr - offsetof(type,member) );})

typedef struct que_s que_t;
typedef struct qitem_s qitem_t;

struct qitem_s {
	struct qitem_s *next;
	struct qitem_s *prev;
	que_t *pque;
	uint8_t onHandling;
};

struct que_s {
	int count;
	qitem_t *head;
	qitem_t *tail;
	int onHandling;
	int maxHandling;
	pthread_mutex_t mu;
	qitem_t *ihandling;
};

static inline que_t *que_init(que_t *pque, int maxHandling)
{
	if (!pque) return 0;
	pthread_mutex_init(&pque->mu, 0);
	pque->maxHandling = maxHandling;
	return pque;
}

static void que_add_head0(que_t *pque, qitem_t *item)
{
	if (pque->head) {
		qitem_t *head = pque->head;
		head->prev = item;
		item->next = pque->head;
	} else {
		item->next = NULL;
		pque->tail = item;
	}
	pque->count++;
	item->prev = NULL;
	item->pque = pque;
	pque->head = item;
}

static void que_add_head(que_t *pque, qitem_t *item)
{
	if (item->pque) return;
	pthread_mutex_lock(&pque->mu);
	que_add_head0(pque, item);
	pthread_mutex_unlock(&pque->mu);
}

static inline qitem_t *que_get_tail(que_t *pque)
{
	return pque->tail;
}

static qitem_t *que_get_next_for_handling0(que_t *pque)
{
	qitem_t *ret = 0;
	if (pque->onHandling == pque->maxHandling) {
		return ret;
	}
	if (!pque->ihandling) {
		ret = pque->ihandling = pque->tail;
	} else {
		ret = pque->ihandling = pque->ihandling->prev;
	}
	pque->onHandling++;
	ret->onHandling = 1;
	return ret;
}

static qitem_t *que_get_next_for_handling(que_t *pque)
{
	pthread_mutex_lock(&pque->mu);
	qitem_t *ret = que_get_next_for_handling0(pque);
	pthread_mutex_unlock(&pque->mu);
	return ret;
}

static qitem_t *que_del_tail0(que_t *pque)
{	
	qitem_t *item = pque->tail;
	if (item->prev) {
		qitem_t *prev = item->prev;
		prev->next = NULL;
		pque->tail = item->prev;
	} else {
		pque->tail = NULL;
		pque->head = NULL;
	}
	pque->count--;
	item->next = NULL;
	item->prev = NULL;
	item->pque = NULL;
	return item;
}

static qitem_t *que_del_tail(que_t *pque)
{
	if (!pque->tail) return 0;
	pthread_mutex_lock(&pque->mu);
	qitem_t *ret = que_del_tail0(pque);
	pthread_mutex_unlock(&pque->mu);
	return ret;
}

static qitem_t *que_del_first_handled0(que_t *pque)
{
	if (!pque || !pque->tail) return 0;
	if (!pque->tail->onHandling) return 0;

	qitem_t *item = pque->tail->prev;
	if (pque->onHandling > 0) pque->onHandling--;
	if (item && !item->onHandling) pque->ihandling = 0;
	pque->tail->onHandling = 0;
	return que_del_tail0(pque);
}

static qitem_t *que_del_first_handled(que_t *pque)
{
	if (!pque || !pque->tail) return 0;
	if (!pque->tail->onHandling) return 0;
	pthread_mutex_lock(&pque->mu);
	qitem_t *ret = que_del_first_handled0(pque);
	pthread_mutex_unlock(&pque->mu);
	return ret;
}

static void que_clear_handled(que_t *pque, void (*free_func)(qitem_t *))
{
	pthread_mutex_lock(&pque->mu);
	while (pque->tail && pque->ihandling) {
		qitem_t *ret = que_del_first_handled0(pque);
		if (free_func) free_func(ret);
	}
	pthread_mutex_unlock(&pque->mu);
}

static void que_clear(que_t *pque, void (*free_func)(qitem_t *))
{
	pthread_mutex_lock(&pque->mu);
	while (pque->tail) {
		qitem_t *ret = que_del_tail0(pque);
		if (free_func) free_func(ret);
	}
	pthread_mutex_unlock(&pque->mu);
}

typedef struct {
	int d;
} data_t;

typedef struct {
	data_t *data;
	qitem_t qitem;
} qdata_t;

void free_function(qitem_t *item)
{
	data_t *data = (container_of(item, qdata_t, qitem))->data;
	printf("free_function: %d\n", data->d);
}

int main()
{
	int i;

	#define HANDLING_DATA_SIZE 7
	que_t *queue = que_init( calloc(1, sizeof(que_t)), HANDLING_DATA_SIZE );

	#define DATA_SIZE 20
	data_t *data = calloc(DATA_SIZE, sizeof(data_t));
	qdata_t *qdata = calloc(DATA_SIZE, sizeof(qdata_t));

	for (i = 0; i < DATA_SIZE; ++i) {
		data[i].d = i+100;
		qdata[i].data = &data[i];
		que_add_head(queue, &(qdata[i].qitem));
	}

	for (i = 0; i < HANDLING_DATA_SIZE+1; ++i) {
		qitem_t *item = que_get_next_for_handling(queue);
		if (item) {
			data_t *data = (container_of(item, qdata_t, qitem))->data;
			printf("get for handling: %d\n", data->d);
		} else {
			printf("couldn't get from que\n");
		}
	}

	que_clear_handled(queue, free_function);

	for (i = 0; i < HANDLING_DATA_SIZE+1; ++i) {
		qitem_t *item = que_get_next_for_handling(queue);
		if (item) {
			data_t *data = (container_of(item, qdata_t, qitem))->data;
			printf("get for handling: %d\n", data->d);
		} else {
			printf("couldn't get from que\n");
		}
	}
}