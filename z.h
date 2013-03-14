#ifndef Z_H
	#define Z_H
/* 一个内存管理库 */

#include <stdlib.h>

void* zalloc(size_t size);
void* zrealloc(void* p, size_t size);
void zfree(void*p);
#endif
