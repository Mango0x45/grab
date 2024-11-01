#ifndef GRAB_GLOBALS_H
#define GRAB_GLOBALS_H

#include <stdatomic.h>

#include <pcre2.h>

#include "exitcodes.h"

typedef struct {
	char c;
	pcre2_code *re;
#if DEBUG
	bool free_me;
#endif
} op_t;

typedef struct {
	bool b : 1;
	bool c : 1;
	bool i : 1;
	bool l : 1;
	bool p : 1;
	bool s : 1;
	bool U : 1;
	bool z : 1;

#if !GIT_GRAB
	bool do_header : 1;
#endif
} flags_t;

#if MAIN_C
	#define maybe_extern
#else
	#define maybe_extern extern
#endif

maybe_extern flags_t flags;
maybe_extern int grab_tabsize;
maybe_extern op_t *ops;
#if MAIN_C
	atomic_int rv = EXIT_NOMATCH;
	const char *lquot = "`", *rquot = "'";
#else
	extern atomic_int rv;
	extern const char *lquot, *rquot;
#endif

#endif /* !GRAB_GLOBALS_H */