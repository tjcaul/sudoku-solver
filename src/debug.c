#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include "debug.h"

#ifdef DEBUG_MEM
void *malloc (size_t sz)
{
	printf("[DEBUG_MEM]: malloc(%zd)\n", sz);
	void *(*real_malloc)(size_t) = dlsym(RTLD_NEXT, "malloc");
	return real_malloc(sz);
}

void free (void *addr)
{
	printf("[DEBUG_MEM]: free(%p)\n", addr);
	void (*real_free)(void *) = dlsym(RTLD_NEXT, "free");
	real_free(addr);
}

#endif
