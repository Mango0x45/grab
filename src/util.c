#include <stddef.h>
#include <stdlib.h>

#include <errors.h>
#include <pcre2.h>

#include "exitcodes.h"

void
pcre2_bitch_and_die(int ec, const char *fmt)
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
		} else
			cerr(EXIT_FATAL, fmt, buf);
	}
}