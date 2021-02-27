
#include <stdint.h>

typedef struct que_item_s que_item_t;
struct que_item_s {
};

typedef struct que_s que_t;
struct que_s {
	int maxSize;		// set it
	que_item_t *items;	// alloc it (que_item_t *)calloc(N, sizeof(que_item_t));
	int first;
	int last;	// following by last item
};

static inline int que_get_size(que_t *p)
{
	return (p->first > p->last)?
		p->maxSize - p->first + p->last :
		p->last - p->first;
}

static void que_add_tail(que_t *p, que_item_t *item)
{
	memcpy(&(p->items[p->last]), item, sizeof(que_item_t));
	// ----------------------------------------
	// ....f....l
	if (p->last == p->maxSize-1) {
		p->last = 0;
		// f........l
		if (p->last == p->first) {
			p->first = 1;
			// lf........
		}
	// ----------------------------------------
	// ...lf.....
	} else if (p->last == (p->first-1)) {
		p->last = p->first;
		p->first++;
		if (p->first >= p->maxSize) {
			p->first = 0;
			// f........l
		}
	}
	// ----------------------------------------
	// ....f..l..
	// ....l..f..
	else {
		p->last++;
	}
}

static void que_rm_head(que_t *p)
{
	if (que_get_size(p) > 0) {
		int it = p->first + 1;
		p->first = (it >= p->maxSize)?
			it - p->maxSize : it;
	}
}

static inline que_item_t *que_get_head(que_t *p)
{
	return (que_get_size(p) > 0)? &(p->items[p->first]) : 0;
}

static que_item_t *que_get_by_index(que_t *p, int idx)
{
	int it = p->first + idx;
	return (it >= p->maxSize)?
		&(p->items[it - p->maxSize]) :
		&(p->items[it]);
}