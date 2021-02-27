
#include "memdbg.h"

#ifdef MEMDBG

#define __USE_GNU
#include <dlfcn.h>
#include <stdio.h>

void * (*memdbgfn_calloc)(size_t nmemb, size_t size);
void * (*memdbgfn_malloc)(size_t size);
void   (*memdbgfn_free)(void *ptr);
void * (*memdbgfn_realloc)(void *ptr, size_t size);
void * (*memdbgfn_memalign)(size_t blocksize, size_t bytes);

void memdbg_init()
{
	memdbgfn_calloc = dlsym(RTLD_NEXT, "calloc");
	memdbgfn_malloc = dlsym(RTLD_NEXT, "malloc");
	memdbgfn_realloc = dlsym(RTLD_NEXT, "realloc");
	memdbgfn_free = dlsym(RTLD_NEXT, "free");
	if (!memdbgfn_calloc) printf("!!memdbgfn_calloc\n");
	if (!memdbgfn_malloc) printf("!!memdbgfn_malloc\n");
	if (!memdbgfn_realloc) printf("!!memdbgfn_realloc\n");
	if (!memdbgfn_free) printf("!!memdbgfn_free\n");
}

#endif