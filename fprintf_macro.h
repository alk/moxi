#ifndef FPRINTF_MACRO_H
#define FPRINTF_MACRO_H

#include <time.h>
#include <sys/time.h>
#include <stdarg.h>

static inline
int fprintf_tstamp(FILE *f, const char *fmt, ...)
{
	int rv;
	va_list list;
	long long t;
	struct timeval tv;
	va_start(list, fmt);
	gettimeofday(&tv, 0);
	t = tv.tv_sec*1000000LL + tv.tv_usec;
	fprintf(f, "[%lld]", t);
	rv = vfprintf(f, fmt, list);
	va_end(list);
	return rv;
}

#define fprintf fprintf_tstamp

#endif
