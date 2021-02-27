#ifndef SERVTOH
#define SERVTOH

#include <stdint.h>

typedef struct to_item_s to_item_t;

/* Структура очереди тайм-аутов */
typedef struct {
	uint16_t count;				// Количество таймаутов
	to_item_t *first;			// Первый элемент в списке таймаутов
	to_item_t *last;			// Последний элемент в списке таймаутов
} to_que_t;

#pragma pack(1)
/* Структура элемента тайм-аута */
struct to_item_s {
	int id;						// Идентификатор
	uint64_t ttc;				// Таймаут
	uint32_t stat;				// Состояние элемента таймаута
	void *data;					// Указатель на данные
	to_que_t *pque;				// Указатель на очередь
	to_item_t *prev;			// Указатель на предыдущий элемент таймаута
	to_item_t *next;			// Указатель на следующий элемент таймаута
};
#pragma pack()

/* Функция определения минимального тайм-аута */
extern uint32_t to_poll(to_que_t *que);

/* Функция определения хозяина очереди */
extern to_que_t *to_owner(to_item_t *pi);

/* Функция обработки таймаутов */
extern to_item_t *to_check(to_que_t *que);

/* Функция инициализации управляющей структуры */
extern void to_initque(to_que_t *que);

/* Функция для удаления элемента из очереди */
extern void to_delete(to_que_t *que, to_item_t *pi);

/* Функция инициализации элемента тайм-аута */
extern void to_inititem(to_item_t *pi, void *data, int id);

/* Функция для добавления элемента в очередь */
extern int to_append(to_que_t *que, to_item_t *pi, uint32_t to);

/* Функция перепостановки тайм-аута */
extern int to_reappend(to_que_t *que, to_item_t *pi, uint32_t to);

#endif