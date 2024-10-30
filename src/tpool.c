/* Thread pool implementation mostly copied from cbs.h */

#include <errno.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>

#include <alloc.h>
#include <array.h>
#include <errors.h>
#include <macros.h>

#include "tpool.h"
#include "work.h"

#include <stdio.h>

static int nproc(void);
static void *tpwork(void *);

static pthread_t thread_buffer[32];

extern const char *lquot, *rquot;

int
nproc(void)
{
	errno = 0;

	/* Grab the number of processors available on the users system.  If we can
	   we query sysconf() but fallback to 1 for systems that don’t support the
	   sysconf() method.  The user can also override this via the GRAB_NPROCS
	   environment variable, and if that’s invalid then we just issue a
	   diagnostic and default to 1.

	   We don’t want to error on an invalid value for GRAB_NPROCS because we
       might be running this tool as part of an editor plugin for example where
       finding the root cause of your regexp-search failing may not be so
       trivial. */

	const char *ev = getenv("GRAB_NPROCS");
	if (ev != nullptr && *ev != 0) {
		const char *endptr;
		long n = strtol(ev, (char **)&endptr, 10);
		if (errno == 0 && *endptr == 0)
			return (int)n;
		if (errno != 0)
			warn("strtol: %s:", ev);
		if (*endptr != 0)
			warn("Invalid value for %s%s%s for GRAB_NPROCS", lquot, ev, rquot);
		return 1;
	}

#ifdef _SC_NPROCESSORS_ONLN
	return (int)sysconf(_SC_NPROCESSORS_ONLN);
#else
	return 1;
#endif
}

int
tpinit(tpool_t *tp, const char **files, ptrdiff_t filecnt)
{
	tp->files = files;
	tp->filecnt = filecnt;
	tp->tcnt = nproc();
	tp->tcnt = MIN(tp->tcnt, filecnt);
	tp->wi = 0;

	if (tp->tcnt <= 32)
		tp->thrds = thread_buffer;
	else if ((tp->thrds = malloc(sizeof(*tp->thrds) * tp->tcnt)) == nullptr)
		err("malloc:");

	/* If for whatever reason some threads fail to be created, we don’t
       panic but instead just continue using the threads that were able
       to spawn.  If all threads fail to spawn we return 0 and the caller
       will resort to single-threaded behaviour. */

	int n = 0;
	for (int i = 0; i < tp->tcnt; i++) {
		if ((errno = pthread_create(tp->thrds + n, nullptr, tpwork, tp)) != 0)
			warn("failed to create thread:");
		else
			n++;
	}
	return n;
}

void
tpfree(tpool_t *tp)
{
	for (int i = 0; i < tp->tcnt; i++)
		pthread_join(tp->thrds[i], nullptr);

#if DEBUG
	if (tp->thrds != thread_buffer)
		free(tp->thrds);
#endif
}

void *
tpwork(void *arg)
{
	tpool_t *tp = arg;

	allocator_t mem = init_heap_allocator(nullptr);
	unsigned char *buf = array_new(mem, typeof(*buf), 4096);

	for (;;) {
		ptrdiff_t i = atomic_fetch_add(&tp->wi, 1);
		if (i >= tp->filecnt)
			break;
		process_file(tp->files[i], &buf);
	}

	flockfile(stdout);
	fwrite(buf, 1, array_len(buf), stdout);
	funlockfile(stdout);

#if DEBUG
	array_free(buf);
#endif
	return nullptr;
}
