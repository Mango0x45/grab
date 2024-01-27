#if __has_include(<features.h>)
#	include <features.h>
#	ifdef __GLIBC__
#		define _POSIX_C_SOURCE 200809L
#	endif
#endif

#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "cbs.h"

#define CC "cc"
#define WARNINGS \
	"-Wall", "-Wextra", "-Wpedantic", "-Werror", "-Wno-parentheses", \
		"-Wno-pointer-sign", "-Wno-attributes"
#define CFLAGS_ALL WARNINGS, "-pipe", "-std=c2x"
#define CFLAGS_DBG CFLAGS_ALL, "-DGRAB_DEBUG", "-Og", "-g", "-ggdb3"
#ifdef __APPLE__
#	define CFLAGS_RLS CFLAGS_ALL, "-O3"
#else
#	define CFLAGS_RLS CFLAGS_ALL, "-O3", "-march=native", "-mtune=native"
#endif
#define PREFIX "/usr/local"

#define streq(a, b) (!strcmp(a, b))
#define CMDPRC(c) \
	do { \
		int ec; \
		cmdput(c); \
		if ((ec = cmdexec(c)) != EXIT_SUCCESS) \
			diex("%s terminated with exit-code %d", *c._argv, ec); \
		cmdclr(&c); \
	} while (0)

static char *mkoutpath(const char *);

static bool lflag, Pflag, rflag;

int
main(int argc, char **argv)
{
	int opt;
	cmd_t c = {0};
	struct option longopts[] = {
		{"lto",     no_argument, nullptr, 'l'},
		{"no-pcre", no_argument, nullptr, 'P'},
		{"release", no_argument, nullptr, 'r'},
		{nullptr,   0,           nullptr, 0  },
	};

	cbsinit(argc, argv);
	rebuild();

	while ((opt = getopt_long(argc, argv, "lPr", longopts, nullptr)) != -1) {
		switch (opt) {
		case 'l':
			lflag = true;
			break;
		case 'P':
			Pflag = true;
			break;
		case 'r':
			rflag = true;
			break;
		default:
			fputs("Usage: make [-lPd]\n", stderr);
			exit(EXIT_FAILURE);
		}
	}

	argc -= optind;
	argv += optind;

	if (argc > 0) {
		if (streq(*argv, "clean")) {
			cmdadd(&c, "find", ".", "-name", "grab", "-or", "-name", "git-grab",
			       "-or", "-name", "*.[ao]", "-delete");
			CMDPRC(c);
		} else if (streq(*argv, "install")) {
			char *bin, *man;
			bin = mkoutpath("/bin");
			man = mkoutpath("/share/man/man1");
			cmdadd(&c, "mkdir", "-p", bin, man);
			CMDPRC(c);
			cmdadd(&c, "cp", "grab", "git-grab", bin);
			CMDPRC(c);
			cmdadd(&c, "cp", "man/grab.1", "man/git-grab.1", man);
			CMDPRC(c);
		}
	} else {
		cmd_t c = {0};
		struct strv sv = {0};

		if (foutdated("vendor/librune/make", "vendor/librune/make.c")) {
			cmdadd(&c, CC, "-lpthread", "-o", "vendor/librune/make",
			       "vendor/librune/make.c");
			CMDPRC(c);
		}

		if (!fexists("vendor/librune/librune.a")) {
			cmdadd(&c, "vendor/librune/make");
			if (rflag)
				cmdadd(&c, "-r");
			if (lflag)
				cmdadd(&c, "-l");
			CMDPRC(c);
		}

		if (foutdated("./grab", "src/grab.c", "src/compat.h", "src/da.h",
		              "vendor/librune/librune.a"))
		{
			env_or_default(&sv, "CC", CC);
			if (rflag)
				env_or_default(&sv, "CFLAGS", CFLAGS_RLS);
			else
				env_or_default(&sv, "CFLAGS", CFLAGS_DBG);

			for (int i = 0; i < 2; i++) {
				char buf[] = "-DGIT_GRAB=X";
				buf[sizeof(buf) - 2] = i + '0';

				cmdaddv(&c, sv.buf, sv.len);
				if (lflag)
					cmdadd(&c, "-flto");
#ifdef __GLIBC__
				cmdadd(&c, "-D_POSIX_C_SOURCE=200809L");
#endif
				cmdadd(&c, "-Ivendor/librune/include", buf);
				if (!Pflag) {
					struct strv pc = {0};
					cmdadd(&c, "-DGRAB_DO_PCRE=1");
					if (pcquery(&pc, "libpcre2-posix", PKGC_CFLAGS | PKGC_LIBS))
						cmdaddv(&c, pc.buf, pc.len);
					else
						cmdadd(&c, "-lpcre2-posix");
					strvfree(&pc);
				}
				cmdadd(&c, "-o", i == 0 ? "grab" : "git-grab", "src/grab.c",
				       "vendor/librune/librune.a");
				CMDPRC(c);
			}

			strvfree(&sv);
		}
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
