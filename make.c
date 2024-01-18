#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "cbs.h"
#include "src/compat.h"

#define CC           "cc"
#define CFLAGS       "-Wall", "-Wextra", "-Wpedantic", "-Werror", "-pipe"
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

static char *mkoutpath(const char *);

static bool dflag, Pflag;

int
main(int argc, char **argv)
{
	int opt;
	cmd_t c = {0};

	cbsinit(argc, argv);
	rebuild();

	while ((opt = getopt(argc, argv, "dP")) != -1) {
		switch (opt) {
		case 'd':
			dflag = true;
			break;
		case 'P':
			Pflag = true;
			break;
		default:
			fputs("Usage: make [-dP]\n", stderr);
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
			cmdadd(&c, "cp", "man/grab.1", "man/git-grab.1", man);
			cmdprc(c);
		}
	} else {
		struct strv cc = {0};
		struct strv cflags = {0};

		env_or_default(&cc, "CC", CC);
		if (dflag)
			env_or_default(&cflags, "CFLAGS", CFLAGS, CFLAGS_DEBUG);
		else
			env_or_default(&cflags, "CFLAGS", CFLAGS, CFLAGS_RELEASE);

		for (int i = 0; i < 2; i++) {
			char buf[] = "-DGIT_GRAB=X";
			buf[sizeof(buf) - 2] = i + '0';

			cmdaddv(&c, cc.buf, cc.len);
			cmdaddv(&c, cflags.buf, cflags.len);
			cmdadd(&c, buf);
			if (!Pflag) {
				struct strv pc = {0};
				cmdadd(&c, "-DGRAB_DO_PCRE=1");
				if (pcquery(&pc, "libpcre2-posix", PKGC_CFLAGS | PKGC_LIBS))
					cmdaddv(&c, pc.buf, pc.len);
				else
					cmdadd(&c, "-lpcre2-posix");
				strvfree(&pc);
			}
			cmdadd(&c, "-o", i == 0 ? "grab" : "git-grab", "src/grab.c");
			cmdprc(c);
		}

		strvfree(&cc);
		strvfree(&cflags);
	}

	return EXIT_SUCCESS;
}

char *
mkoutpath(const char *s)
{
	char *p, *buf;

	buf = bufalloc(NULL, PATH_MAX, sizeof(char));
	buf[0] = 0;

	if (p = getenv("DESTDIR"), p && *p) {
		if (strlcat(buf, p, PATH_MAX) >= PATH_MAX)
			goto toolong;
	}

	p = getenv("PREFIX");
	if (strlcat(buf, p && *p ? p : PREFIX, PATH_MAX) >= PATH_MAX)
		goto toolong;
	if (strlcat(buf, s, PATH_MAX) >= PATH_MAX)
		goto toolong;

	return buf;

toolong:
	errno = ENAMETOOLONG;
	die(__func__);
}
