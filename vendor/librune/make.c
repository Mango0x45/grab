#include <errno.h>
#include <glob.h>
#include <libgen.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define CBS_PTHREAD
#include "cbs.h"

#define CC "cc"
#ifdef __APPLE__
#	define CFLAGS "-Wall", "-Wextra", "-Wpedantic", "-Werror", "-pipe", "-O3"
#else
#	define CFLAGS \
		"-Wall", "-Wextra", "-Wpedantic", "-Werror", "-pipe", "-O3", \
			"-march=native", "-mtune=native"
#endif

#define cmdprc(c) \
	do { \
		int ec; \
		cmdput(c); \
		if ((ec = cmdexec(c)) != EXIT_SUCCESS) \
			diex("%s terminated with exit-code %d", *c._argv, ec); \
		cmdclr(&c); \
	} while (0)

static void work(void *);
static int globerr(const char *, int);

int
main(int argc, char **argv)
{
	cmd_t c = {0};
	size_t n;
	glob_t g;
	tpool_t tp;

	cbsinit(argc, argv);
	rebuild();

	if (chdir(dirname(*argv)) == -1)
		die("chdir");

	if (glob("lib/utf8/*.c", 0, globerr, &g)
	    || glob("lib/gbrk/*.c", GLOB_APPEND, globerr, &g))
	{
		die("glob");
	}

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

	cmdadd(&c, "ar", "rcs", "librune.a");
	for (size_t i = 0; i < g.gl_pathc; i++)
		g.gl_pathv[i][strlen(g.gl_pathv[i]) - 1] = 'o';
	cmdaddv(&c, g.gl_pathv, g.gl_pathc);
	cmdprc(c);

	globfree(&g);
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

	env_or_default(&sv, "CC", CC);
	env_or_default(&sv, "CFLAGS", CFLAGS);
	cmdaddv(&c, sv.buf, sv.len);
	cmdadd(&c, "-Iinclude", "-fPIC", "-o", dst, "-c", src);
	cmdprc(c);

	free(dst);
}

int
globerr(const char *s, int e)
{
	errno = e;
	die("glob: %s", s);
}
