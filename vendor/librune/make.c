#include <errno.h>
#include <glob.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define CBS_PTHREAD
#include "cbs.h"

#define CC         "cc"
#define WARNINGS   "-Wall", "-Wextra", "-Wpedantic", "-Werror", "-Wno-attributes"
#define CFLAGS_ALL WARNINGS, "-pipe", "-std=c2x"
#define CFLAGS_DBG CFLAGS_ALL, "-g", "-ggdb3", "-Og"
#ifdef __APPLE__
#	define CFLAGS_RLS CFLAGS_ALL, "-O3"
#else
#	define CFLAGS_RLS CFLAGS_ALL, "-O3", "-march=native", "-mtune=native"
#endif

#define cmdprc(c) \
	do { \
		int ec; \
		cmdput(c); \
		if ((ec = cmdexec(c)) != EXIT_SUCCESS) \
			diex("%s terminated with exit-code %d", *c._argv, ec); \
		cmdclr(&c); \
	} while (0)

#define streq(a, b) (!strcmp(a, b))

static void work(void *);
static int globerr(const char *, int);

static bool lflag, rflag;

int
main(int argc, char **argv)
{
	int opt;

	cbsinit(argc, argv);
	rebuild();

	while ((opt = getopt(argc, argv, "lr")) != -1) {
		switch (opt) {
		case 'l':
			lflag = true;
			break;
		case 'r':
			rflag = true;
			break;
		default:
			fprintf(stderr, "Usage: %s [-lr]\n", *argv);
			exit(EXIT_FAILURE);
		}
	}

	argc -= optind;
	argv += optind;

	if (argc >= 1) {
		if (streq(*argv, "clean")) {
			cmd_t c = {0};
			cmdadd(&c, "find", ".", "-name", "*.[ao]", "-delete");
			cmdprc(c);
		} else {
			diex("invalid subcommand -- '%s'", *argv);
			exit(EXIT_FAILURE);
		}
	} else {
		cmd_t c = {0};
		size_t n;
		glob_t g;
		tpool_t tp;

		if (glob("lib/*/*.c", 0, globerr, &g))
			die("glob");

		if ((n = nproc()) == -1) {
			if (errno)
				die("nproc");
			n = 8;
		}

		tpinit(&tp, n);
		for (size_t i = 0; i < g.gl_pathc; i++)
			tpenq(&tp, work, g.gl_pathv[i], NULL);
		tpwait(&tp);
		tpfree(&tp);

		for (size_t i = 0; i < g.gl_pathc; i++)
			g.gl_pathv[i][strlen(g.gl_pathv[i]) - 1] = 'o';

		if (foutdatedv("librune.a", (const char **)g.gl_pathv, g.gl_pathc)) {
			cmdadd(&c, "ar", "rcs", "librune.a");
			cmdaddv(&c, g.gl_pathv, g.gl_pathc);
			cmdprc(c);
		}

		globfree(&g);
	}

	return EXIT_SUCCESS;
}

void
work(void *p)
{
	cmd_t c = {0};
	char *dst, *src = p;
	struct strv sv = {0};

	if (!(dst = strdup(src)))
		die("strdup");
	dst[strlen(dst) - 1] = 'o';

	if (foutdated(dst, src)) {
		env_or_default(&sv, "CC", CC);
		if (rflag)
			env_or_default(&sv, "CFLAGS", CFLAGS_RLS);
		else
			env_or_default(&sv, "CFLAGS", CFLAGS_DBG);
		cmdaddv(&c, sv.buf, sv.len);
		if (lflag)
			cmdadd(&c, "-flto");
		cmdadd(&c, "-Iinclude", "-fPIC", "-o", dst, "-c", src);
		cmdprc(c);
	}

	free(dst);
}

int
globerr(const char *s, int e)
{
	errno = e;
	die("glob: %s", s);
}
