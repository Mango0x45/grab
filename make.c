#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "cbs.h"

#define CC "cc"
#define CFLAGS \
	"-Wall", "-Wextra", "-Wpedantic", "-Werror", "-D_POSIX_C_SOURCE=200809", \
		"-pipe"
#define CFLAGS_DEBUG "-DGRAB_DEBUG", "-g", "-ggdb3"
#ifdef __APPLE__
#	define CFLAGS_RELEASE "-O3"
#else
#	define CFLAGS_RELEASE "-O3", "-march=native", "-mtune=native"
#endif
#define PREFIX "/usr/local"

#define streq(a, b) (!strcmp(a, b))
#define cmdprc(c) \
	do { \
		int ec; \
		cmdput(c); \
		if ((ec = cmdexec(c)) != EXIT_SUCCESS) \
			diex("%s terminated with exit-code %d", *c._argv, ec); \
		cmdclr(&c); \
	} while (0)

static size_t _strlcat(char *, const char *, size_t);
static char *_mkoutpath(const char **, size_t);
#define mkoutpath(...) \
	_mkoutpath((const char **)_vtoa(__VA_ARGS__), lengthof(_vtoa(__VA_ARGS__)))

int
main(int argc, char **argv)
{
	int opt;
	bool debug;
	cmd_t c = {0};

	cbsinit(argc, argv);
	rebuild();

	while ((opt = getopt(argc, argv, "d")) != -1) {
		switch (opt) {
		case 'd':
			debug = true;
			break;
		default:
			fputs("Usage: make [-d]\n", stderr);
			exit(EXIT_FAILURE);
		}
	}

	argc -= optind;
	argv += optind;

	if (argc > 0) {
		if (streq(*argv, "clean")) {
			cmdadd(&c, "rm", "-f", "grab");
			cmdprc(c);
		} else if (streq(*argv, "install")) {
			char *bin, *man;
			bin = mkoutpath("/bin");
			man = mkoutpath("/share/man/man1");
			cmdadd(&c, "mkdir", "-p", bin, man);
			cmdprc(c);
			cmdadd(&c, "cp", "grab", "git-grab", bin);
			cmdprc(c);
			cmdadd(&c, "cp", "grab.1", man);
			cmdprc(c);
		}
	} else {
		if (foutdated("grab", "grab.c", "da.h")) {
			for (int i = 0; i < 2; i++) {
				char buf[] = "-DGIT_GRAB=X";
				buf[sizeof(buf) - 2] = i + '0';

				cmdadd(&c, CC, CFLAGS);
				cmdadd(&c, buf);
#ifdef CBS_IS_C23
				cmdadd(&c, "-DGRAB_IS_C23=1");
#endif
				if (debug)
					cmdadd(&c, CFLAGS_DEBUG);
				else
					cmdadd(&c, CFLAGS_RELEASE);
				cmdadd(&c, "-o", i == 0 ? "grab" : "git-grab", "grab.c");
				cmdprc(c);
			}
		}
	}

	return EXIT_SUCCESS;
}

char *
_mkoutpath(const char **xs, size_t n)
{
#define trycat(s) \
	do { \
		if (_strlcat(buf, (s), PATH_MAX) >= PATH_MAX) \
			goto toolong; \
	} while (0)

	char *p, *buf;

	buf = bufalloc(NULL, PATH_MAX, sizeof(char));
	buf[0] = 0;

	if (p = getenv("DESTDIR"), p && *p)
		trycat(p);

	p = getenv("PREFIX");
	trycat(p && *p ? p : PREFIX);

	for (size_t i = 0; i < n; i++)
		trycat(xs[i]);

	return buf;

toolong:
	errno = ENAMETOOLONG;
	die(__func__);
}

size_t
_strlcat(char *dst, const char *src, size_t size)
{
	char *d = dst;
	const char *s = src;
	size_t n = size;
	size_t dlen;

	while (n-- != 0 && *d != '\0')
		d++;

	dlen = d - dst;
	n = size - dlen;

	if (n == 0) {
		while (*s++)
			;
		return dlen + (s - src - 1);
	}

	while (*s != '\0') {
		if (n != 1) {
			*d++ = *s;
			n--;
		}
		s++;
	}

	*d = '\0';

	return dlen + (s - src);
}
