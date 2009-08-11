#ifndef SLEEP_PROFILER_H
#define SLEEP_PROFILER_H

#include <pthread.h>

extern int pthread_create_hook(pthread_t *__restrict __newthread,
			       __const pthread_attr_t *__restrict __attr,
			       void *(*__start_routine) (void *),
			       void *__restrict __arg);

extern void setup_profiling_signal(void);


#endif
