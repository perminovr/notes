#include "servto.h"
#include <string.h>
#include <sys/time.h>
#include <time.h> 

uint64_t getsystick(void)
{
	struct timespec tv;
	clock_gettime(CLOCK_MONOTONIC,&tv);
	return ((uint64_t)tv.tv_sec*1000000L) + (uint64_t)tv.tv_nsec/1000L;
}

/// Функция инициализации управляющей структуры
//\ param que - указатель на управляющую структуру
void to_initque(to_que_t *que)
{
	que->count=0;
	que->last=NULL;
	que->first=NULL;
}

/// Функция инициализации элемента тайм-аута
//\ param pi - указатель на элемент
//\ param data - указатель на данные
//\ param id - идентификатор данных
void to_inititem(to_item_t *pi, void *data, int id)
{
	pi->id=id;
	pi->stat=0;
	pi->data=data;
	pi->pque=NULL;
	pi->next=NULL;
	pi->prev=NULL;
}

/// Функция для добавления элемента в очередь
//\ param que - указатель на управляющую структуру
//\ param pi - указатель на элемент таймаута
//\ param to - величина тайм-аута
int to_append(to_que_t *que, to_item_t *pi, uint32_t to)
{
	uint64_t ttc;
	to_item_t *p1;
	to_item_t *p2;
	
	if (pi->stat) return 0;
	
	p2=NULL;
	p1=que->last;
	
	/* Текущее время с учетом таймаута */
	ttc=getsystick()+to;
	
	/* Пробегаем по списку тайм-аутов */
	for (;p1;p2=p1,p1=p1->next) {
		if (p1->ttc<=ttc) break;
	}
	
	if (p1) p1->prev=pi;
	else que->first=pi;
	
	if (p2) p2->next=pi;
	else que->last=pi;
	
	pi->stat=1;
	pi->ttc=ttc;
	pi->next=p1;
	pi->prev=p2;
	pi->pque=que;
	que->count++;
	
	return 1;
}

/// Функция для удаления элемента из очереди
//\ param que - указатель на управляющую структуру
//\ param pi - указатель на элемент таймаута
void to_delete(to_que_t *que, to_item_t *pi)
{
	to_item_t *prev;
	to_item_t *next;

	if (!pi->stat) return;

	prev=pi->prev;
	next=pi->next;

	if (prev) prev->next=next;
	else que->last=next;

	if (next) next->prev=prev;
	else que->first=prev;

	pi->stat=0;
	pi->prev=NULL;
	pi->next=NULL;
	pi->pque=NULL;
	que->count--;
}

/// Функция перепостановки тайм-аута
//\ param que - указатель на управляющую структуру
//\ param pi - указатель на элемент таймаута
//\ param to - величина тайм-аута
int to_reappend(to_que_t *que, to_item_t *pi, uint32_t to)
{
	to_delete(que,pi);
	return to_append(que,pi,to);
}

/// Функция определения минимального тайм-аута
//\ param que - указатель на управляющую структуру
uint32_t to_poll(to_que_t *que)
{
	uint64_t ttc;
	int diff;
	to_item_t *first;

	first=que->first;
	if (!first) return -1;

	/* Текущее время */
	ttc=getsystick();

	/* Вычисляем разницу */
	diff=first->ttc-ttc;

	return (diff<=0)? 0:diff;
}

/// Функция обработки таймаутов
//\ param que - указатель на управляющую структуру
to_item_t *to_check(to_que_t *que)
{
	uint64_t ttc;
	to_item_t *first;
	
	first=que->first;
	if (!first) return NULL;
	
	/* Текущее время */
	ttc=getsystick();
	
	if (first->ttc<=ttc) return first;
	
	return NULL;
}

/// Функция определения хозяина очереди
//\ param pi - указатель на элемент
/// return указатель на очередь
to_que_t *to_owner(to_item_t *pi)
{
	return pi->pque;
}