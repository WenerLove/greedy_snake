/* 一个内存管理函数 */

#include <stdlib.h>
#include <string.h>

#include "dbg.h"

#include "z.h"

void* zalloc(size_t size)
{
	void* p;
	p = malloc(size);
	/* if(NULL == p) */
	/* 	return NULL; */
	assert(p);
	memset(p,0,size);
return p;
}

void* zrealloc(void* p, size_t size)
{
	p = realloc(p,size);
	assert(p);
return p;
}

void zfree(void*p)
{
	free(p);
}

