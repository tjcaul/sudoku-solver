#ifndef DEBUG_H
#define DEBUG_H

#include <stdio.h>

#ifdef DEBUG
	#define debug(fmt, ...) printf("[DEBUG] %s:%d:\t" fmt, __FILE__, __LINE__, ##__VA_ARGS__)
#else
	#define debug(...)
#endif

#ifdef DMALLOC
	#include <dmalloc.h>
#endif

#endif
