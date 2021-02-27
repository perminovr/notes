
#ifndef MEMDBG_H
#define MEMDBG_H

#define _GNU_SOURCE
#define __GNU_SOURCE
#define __USE_GNU

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

#define MEMDBG

#ifdef MEMDBG
	extern void * (*memdbgfn_calloc)(size_t nmemb, size_t size);
	extern void * (*memdbgfn_malloc)(size_t size);
	extern void   (*memdbgfn_free)(void *ptr);
	extern void * (*memdbgfn_realloc)(void *ptr, size_t size);
	extern void * (*memdbgfn_memalign)(size_t blocksize, size_t bytes);
#endif

#define memdbg_calloc(num,sz) ({ \
	void *p = memdbgfn_calloc(num,sz);\
	printf("memdbg_alloc %p %u memdbg_calloc %s:%d\n", p, num*sz, __FILE__, __LINE__);\
	p;\
})

#define memdbg_malloc(sz) ({ \
	void *p = memdbgfn_malloc(sz);\
	printf("memdbg_alloc %p %u memdbg_malloc %s:%d\n", p, sz, __FILE__, __LINE__);\
	p;\
})

#define memdbg_free(p) ({ \
	printf("memdbg_free %p memdbg_free %s:%d\n", p, __FILE__, __LINE__);\
	memdbgfn_free(p);\
})

#define memdbg_realloc(p,sz) ({ \
	printf("memdbg_free %p memdbg_realloc %s:%d\n", p, __FILE__, __LINE__);\
	void *p = memdbgfn_realloc(p,sz);\
	printf("memdbg_alloc %p %u memdbg_realloc %s:%d\n", p, sz, __FILE__, __LINE__);\
	p;\
})

#ifdef MEMDBG
	#define calloc(num,sz) memdbg_calloc(num,sz)
	#define malloc(sz) memdbg_malloc(sz)
	#define free(p) memdbg_free(p)
	#define realloc(p,sz) memdbg_realloc(p,sz)
	extern void memdbg_init();
#else
	static void memdbg_init() {;}
#endif

#endif // MEMDBG_H