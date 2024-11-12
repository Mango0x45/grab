#include <errno.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include <errors.h>
#include <macros.h>
#include <mbstring.h>
#include <pcre2.h>

#include "exitcodes.h"
#include "globals.h"

void
pcre2_bitch_and_die(int ec, const char *fmt, ...)
{
	/* If we’ve gotten here, we don’t care about writing efficient code */
	ptrdiff_t bufsz = 512;

	for (;;) {
		char *buf = malloc(bufsz);
		if (buf == nullptr)
			cerr(EXIT_FATAL, "malloc:");
		if (pcre2_get_error_message(ec, buf, bufsz) == PCRE2_ERROR_NOMEMORY) {
			free(buf);
			bufsz *= 2;
		} else {
			va_list ap;
			va_start(ap, fmt);
			flockfile(stderr);
			vfprintf(stderr, fmt, ap);
			fprintf(stderr, ": %s\n", buf);
			funlockfile(stderr);
			exit(EXIT_FATAL);
		}
	}
}

int
getenv_posnum(const char *ev, int fallback)
{
	const char *s = getenv(ev);
	if (s != nullptr && *s != 0) {
		const char *endptr;
		errno = 0;
		long n = strtol(s, (char **)&endptr, 10);
		if (errno != 0)
			warn("strtol: %s:", s);
		else if (*endptr != 0 || n <= 0)
			warn("invalid value %s%s%s for %s", lquot, s, rquot, ev);
		else
			return (int)n;
	}
	return fallback;
}

bool
islbrk(u8view_t g)
{
	return ucseq(g, U8("\n"))
	    || ucseq(g, U8("\v"))
	    || ucseq(g, U8("\f"))
	    || ucseq(g, U8("\r\n"))
	    || ucseq(g, U8("\x85"))
	    || ucseq(g, U8("\u2028"))
	    || ucseq(g, U8("\u2029"));
}