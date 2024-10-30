#define _GNU_SOURCE
#include <errno.h>
#include <glob.h>
#include <langinfo.h>
#include <libgen.h>
#include <locale.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#if __has_include(<features.h>)
#	include <features.h>
#endif

#include "cbs.h"

#define flagset(o) (flags & UINT32_C(1)<<(o)-'a')

[[noreturn, gnu::format(printf, 1, 2)]]
static void err(const char *, ...);
static void build_mlib(void);

static uint32_t flags;
static const char *argv0;

static char *cflags_req[] = {
	"-Wall",
	"-Wextra",
	"-Wpedantic",
	"-Wvla",
	"-Wno-attributes",
	"-Wno-empty-body",          /* Annoying when debugging */
	"-Wno-pointer-sign",
	"-Wno-parentheses",
	"-Ivendor/mlib/include",
	"-pipe",
	"-std=c23",
#ifdef __linux__
	"-D_FILE_OFFSET_BITS=64",
#endif
#ifdef __GLIBC__
	"-D_GNU_SOURCE",
#endif
	"-DPCRE2_CODE_UNIT_WIDTH=8",
};

static char *cflags_dbg[] = {
	"-DDEBUG=1",
	"-fsanitize=address,undefined",
#if __GNUC__ && __APPLE__
	"-ggdb2",
#else
	"-ggdb3",
#endif
	"-O0",
};

static char *cflags_rls[] = {
	"-DNDEBUG=1",
	"-flto",
#ifdef __APPLE__
	"-mcpu=native",
#else
	"-march=native",
	"-mtune=native",
#endif
	"-O3",
};

int
main(int argc, char **argv)
{
	cbsinit(argc, argv);
	rebuild();
	setlocale(LC_ALL, "");

	argv0 = basename(argv[0]);

	int opt;
	while ((opt = getopt(argc, argv, "mr")) != -1) {
		switch (opt) {
		case '?':
usage:
			fprintf(stderr,
				"Usage: %s [-mr]\n"
				"       %s clean | distclean\n",
				*argv, *argv);
			exit(EXIT_FAILURE);
		default:
			flags |= UINT32_C(1) << opt-'a';
		}
	}

	argc -= optind;
	argv += optind;

	if (argc > 1)
		goto usage;

	if (argc == 1) {
		struct strs cmd = {};
		if (strcmp(argv[0], "clean") == 0) {
			strspushl(&cmd, "rm", "-f", "grab", "git-grab");
		} else if (strcmp(argv[0], "distclean") == 0) {
			strspushl(&cmd, "rm", "-f", "grab", "git-grab", "make");
			if (fexists("vendor/mlib/make")) {
				cmdput(cmd);
				if (cmdexec(cmd) != EXIT_SUCCESS)
					exit(EXIT_FAILURE);
				strszero(&cmd);
				strspushl(&cmd, "vendor/mlib/make", "clean");
			}
		} else {
			err(strcmp(nl_langinfo(CODESET), "UTF-8") == 0
				? "invalid subcommand — ‘%s’"
				: "invalid subcommand -- `%s'",
				argv[0]);
		}

		cmdput(cmd);
		return cmdexec(cmd);
	}

	build_mlib();

	glob_t g;
	if (glob("src/*.c", 0, nullptr, &g))
		err("glob:");

	struct strs cmd = {};
	for (char ch = '0'; ch <= '1'; ch++) {
		strspushenvl(&cmd, "CC", "cc");
		strspush(&cmd, cflags_req, lengthof(cflags_req));
		if (flagset('r'))
			strspushenv(&cmd, "CFLAGS", cflags_rls, lengthof(cflags_rls));
		else
			strspushenv(&cmd, "CFLAGS", cflags_dbg, lengthof(cflags_dbg));

		char buf[] = "-DGIT_GRAB=X";
		buf[sizeof(buf) - 2] = ch;
		strspushl(&cmd, buf);

		if (!pcquery(&cmd, "libpcre2-8", PC_CFLAGS | PC_LIBS))
			strspushl(&cmd, "-lpcre2-8");

		strspushl(&cmd, "-o", ch == '0' ? "grab" : "git-grab");
		strspush(&cmd, g.gl_pathv, g.gl_pathc);
		strspushl(&cmd, "vendor/mlib/libmlib.a");
		cmdput(cmd);
		if (cmdexec(cmd) != EXIT_SUCCESS)
			exit(EXIT_FAILURE);

		strszero(&cmd);
	}

	return EXIT_SUCCESS;
}

void
build_mlib(void)
{
	struct strs cmd = {};
	if (flagset('m') || !fexists("vendor/mlib/make")) {
		strspushenvl(&cmd, "CC", "cc");
		strspushl(&cmd, "-std=c23", "-o", "vendor/mlib/make", "vendor/mlib/make.c");
		cmdput(cmd);
		if (cmdexec(cmd) != EXIT_SUCCESS)
			exit(EXIT_FAILURE);
		strszero(&cmd);
	}
	strspushl(&cmd, "./vendor/mlib/make");
	if (flagset('m'))
		strspushl(&cmd, "-f");
	if (flagset('r'))
		strspushl(&cmd, "-r");
	cmdput(cmd);
	if (cmdexec(cmd) != EXIT_SUCCESS)
		exit(EXIT_FAILURE);
	strsfree(&cmd);
}

void
err(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	int save = errno;
	flockfile(stderr);
	fprintf(stderr, "%s: ", argv0);
	vfprintf(stderr, fmt, ap);
	if (fmt[strlen(fmt) - 1] == ':')
		fprintf(stderr, " %s", strerror(save));
	fputc('\n', stderr);
	funlockfile(stderr);
	va_end(ap);
	exit(EXIT_FAILURE);
}
