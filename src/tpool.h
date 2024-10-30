#ifndef GRAB_TPOOL_H
#define GRAB_TPOOL_H

#include <pthread.h>
#include <stdatomic.h>
#include <stddef.h>

typedef struct {
	int tcnt;
	pthread_t *thrds;
	atomic_ptrdiff_t wi;
	const char **files;
	ptrdiff_t filecnt;
} tpool_t;

int tpinit(tpool_t *tp, const char **files, ptrdiff_t filecnt);
void tpfree(tpool_t *tp);

#endif /* !GRAB_TPOOL_H */
