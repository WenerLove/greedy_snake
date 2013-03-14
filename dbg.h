#ifndef DBG_H
	#define DBG_H

/* 便于调试的宏 */

/* 调试用的宏 */
#ifdef DEBUG
	#include <assert.h>

	#define E(format,args...)	\
		fprintf(stderr, format, ##args)
	#define EA(condition, format, args...)	\
		if(!(condition))	E(format, ##args);
#else
	#define assert(a) ((void)0)
	#define E(format,args...)	assert()
	#define EA(condition, format, args...)	assert()
#endif

/* include once */
#endif
