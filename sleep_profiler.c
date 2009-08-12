#define _GNU_SOURCE

#include "sleep_profiler.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <signal.h>
#include <time.h>
#include <string.h>
#include <execinfo.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <sys/syscall.h>

#include <event.h>

static inline
pid_t gettid(void)
{
	return syscall(SYS_gettid);
}

typedef void * (*__start_routine_t)(void *__arg);

struct pthread_create_hook_data {
	__start_routine_t __start_routine;
	void * arg;
};

static __thread
int dump_fd;

static __thread
timer_t profiling_timer;

#define BACKTRACE_SIZE 256

static
void signal_handler(int __signo, siginfo_t *siginfo, void *ucontext)
{
	static char zero;

	int olderrno = errno;
	void* backtrace_buf[BACKTRACE_SIZE];

	int count = backtrace(backtrace_buf, BACKTRACE_SIZE);
	backtrace_symbols_fd(backtrace_buf, count, dump_fd);
	{
		struct event_base *base = event_currently_sleeping();
		if (base) {
			event_diag_pending_events(base, dump_fd);
		}
	}
	write(dump_fd, &zero, sizeof(zero));

	errno = olderrno;
}

static
void *tramp(void *_data)
{
	struct pthread_create_hook_data *data = _data;
	__start_routine_t __start_routine = data->__start_routine;
	void *arg = data->arg;
	free(data);

	{
		sigset_t set;
		int rv;
		struct sigevent se;
		struct itimerspec ispec;

		sigemptyset(&set);
		sigaddset(&set, SIGALRM);
		pthread_sigmask(SIG_UNBLOCK, &set, 0);

		se.sigev_notify = SIGEV_THREAD_ID;
		se._sigev_un._tid = gettid();
		se.sigev_signo = SIGALRM;
		rv = timer_create(CLOCK_REALTIME, &se, &profiling_timer);
		if (rv < 0) {
			perror("timer_create");
			abort();
		}
		memset(&ispec, 0, sizeof(ispec));
		ispec.it_value.tv_nsec = 1000000000/200;
		ispec.it_interval = ispec.it_value;
		timer_settime(profiling_timer, 0, &ispec, 0);
	}

	{
		char name[256] = {0};
		snprintf(name, 255, "profdata_%i", gettid());
		dump_fd = open(name, O_EXCL | O_CREAT | O_WRONLY, 0644);
		if (dump_fd < 0) {
			perror("open");
			abort();
		}
	}

	return __start_routine(arg);
}

void setup_profiling_signal(void)
{
	struct sigaction sa;
	sa.sa_sigaction = signal_handler;
	sa.sa_flags = SA_RESTART | SA_SIGINFO;
	sigemptyset(&sa.sa_mask);

	sigaction(SIGALRM, &sa, 0);
}

int pthread_create_hook(pthread_t *__restrict __newthread,
			__const pthread_attr_t *__restrict __attr,
			void *(*__start_routine) (void *),
			void *__restrict __arg)
{
	struct pthread_create_hook_data *data = malloc(sizeof(struct pthread_create_hook_data));
	data->__start_routine = __start_routine;
	data->arg = __arg;
	return pthread_create(__newthread, __attr, tramp, data);
}
