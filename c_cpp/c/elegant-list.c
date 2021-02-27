#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct list_s list_t;
struct list_s {
	int v;
	list_t *next;
};

list_t *newitem(int v)
{
	list_t *ret = (list_t *)calloc(1, sizeof(list_t));
	ret->v = v;
	return ret;
}

void list_pb(list_t *h, list_t *n)
{
	list_t **lastItem = &(h);
	while (*lastItem) {
		lastItem = &((*lastItem)->next);
	}
	*lastItem = n;
	printf("%p\n", n);
}

int main()
{
	int i = 0;
	list_t *h = newitem(i++);

	list_pb( h, newitem(i++) );
	list_pb( h, newitem(i++) );
	list_pb( h, newitem(i++) );
	list_pb( h, newitem(i++) );

	for (list_t *it = h; it; it = it->next) {
		printf("%p %d\n", it, it->v);
	}

	return 0;
}