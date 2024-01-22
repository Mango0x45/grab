#include <err.h>
#include <getopt.h>
#include <libgen.h>
#include <limits.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#if GRAB_DO_PCRE
#	include <pcre2posix.h>
#else
#	include <regex.h>
#	ifndef REG_DOTALL
#		define REG_DOTALL 0
#	endif
#	define REG_UCP 0
#	define REG_UTF 0
#	ifndef REG_STARTEND
#		error "REG_STARTEND not defined"
#	endif
#endif

#include <gbrk.h>

#include "compat.h"
#include "da.h"

#define die(...)  err(EXIT_FAILURE, __VA_ARGS__)
#define diex(...) errx(EXIT_FAILURE, __VA_ARGS__)
#define warn(...) \
	do { \
		warn(__VA_ARGS__); \
		rv = EXIT_FAILURE; \
	} while (0)

#define streq(a, b)    (!strcmp(a, b))
#define memeq(a, b, n) (!memcmp(a, b, n))

#define EEARLY "Input string terminated prematurely"

#define DEFCOL_FN "35"
#define DEFCOL_LN "32"
#define DEFCOL_MA "01;31"
#define DEFCOL_SE "36"

struct op {
	char c;
	regex_t pat;
};

struct ops {
	struct op *buf;
	size_t len, cap;
};

struct sv {
	char *p;
	size_t len;
};

typedef unsigned char uchar;
typedef void (*cmd_func)(struct sv, struct ops, size_t, const char *);
typedef void (*put_func)(struct sv, regmatch_t *, const char *);

static void cmdg(struct sv, struct ops, size_t, const char *);
static void cmdx(struct sv, struct ops, size_t, const char *);
static void cmdX(struct sv, struct ops, size_t, const char *);

static void putm(struct sv, regmatch_t *, const char *);
static void putm_nc(struct sv, regmatch_t *, const char *);

#if GIT_GRAB
static FILE *getfstream(int n, char *v[n]);
#endif
static void grab(struct ops, FILE *, const char *);
static struct ops comppat(char *);
static regex_t mkregex(char *, size_t);
static bool islbrk(struct u8view);
static bool sgrvalid(const char *);
static bool xisspace(char);
static char *xstrchrnul(const char *, char);
static char *env_or_default(const char *, const char *);

static int filecnt, rv;
static bool bflag, cflag, nflag, sflag, Uflag, zflag;
static bool fflag = GIT_GRAB;
static put_func putf;

static struct {
	const char *p, *bp;
	size_t col, row;
} pos;

static const cmd_func op_table[UCHAR_MAX] = {
	['g'] = cmdg,
	['G'] = cmdg,
	['x'] = cmdx,
	['X'] = cmdX,
};

static void
usage(const char *s)
{
	fprintf(stderr,
#if GIT_GRAB
	        "Usage: %s [-s | -z] [-bcnU] pattern [glob ...]\n"
#else
	        "Usage: %s [-s | -z] [-bcfnU] pattern [file ...]\n"
#endif
	        "       %s -h\n",
	        s, s);
	exit(EXIT_FAILURE);
}

int
main(int argc, char **argv)
{
	int opt;
	struct ops ops;
	struct option longopts[] = {
		{"byte-offset",   no_argument, nullptr, 'b'},
		{"color",         no_argument, nullptr, 'c'},
#if GIT_GRAB
		{"filenames",     no_argument, nullptr, 'f'},
#endif
		{"help",          no_argument, nullptr, 'h'},
		{"newline",       no_argument, nullptr, 'n'},
		{"strip-newline", no_argument, nullptr, 's'},
		{"no-unicode",    no_argument, nullptr, 'U'},
		{"zero",          no_argument, nullptr, 'z'},
		{nullptr,         0,           nullptr, 0  },
	};

#if GIT_GRAB
	char *entry = nullptr;
	size_t len;
	ssize_t nr;
	FILE *flist;
	const char *opts = "bchnsUz";
#else
	const char *opts = "bcfhnsUz";
#endif

	argv[0] = basename(argv[0]);
	if (argc < 2)
		usage(argv[0]);

	setlocale(LC_ALL, "");

	while ((opt = getopt_long(argc, argv, opts, longopts, nullptr)) != -1) {
		switch (opt) {
		case 'b':
			bflag = true;
			break;
		case 'c':
			cflag = true;
			break;
#if !GIT_GRAB
		case 'f':
			fflag = true;
			break;
#endif
		case 'n':
			nflag = true;
			break;
		case 's':
			sflag = true;
			break;
		case 'U':
#if GRAB_DO_PCRE
			Uflag = true;
			break;
#else
			errx(2, "program not built with PCRE support");
#endif
		case 'z':
			zflag = true;
			break;
		case 'h':
			execlp("man", "man", "1", argv[0], nullptr);
			die("execlp: man 1 %s", argv[0]);
		default:
			usage(argv[0]);
		}
	}

	if (sflag && zflag)
		usage(argv[0]);

	argc -= optind;
	argv += optind;
	filecnt = argc - 1;

	if (!cflag && isatty(STDOUT_FILENO) == 1
	    && !env_or_default("NO_COLOR", nullptr))
	{
		cflag = !streq(env_or_default("TERM", ""), "dumb");
	}

	putf = cflag ? putm : putm_nc;
	ops = comppat(argv[0]);

#if GIT_GRAB
	if ((flist = getfstream(argc - 1, argv + 1)) == nullptr)
		die("getfstream");
	while ((nr = getdelim(&entry, &len, '\0', flist)) > 0) {
		FILE *fp;

		if ((fp = fopen(entry, "r")) == nullptr)
			warn("fopen: %s", entry);
		else {
			grab(ops, fp, entry);
			fclose(fp);
		}
	}
	if (ferror(flist))
		warn("getdelim");
#else
	if (argc == 1)
		grab(ops, stdin, "-");
	else {
		for (int i = 1; i < argc; i++) {
			FILE *fp;

			if (streq(argv[i], "-")) {
				grab(ops, stdin, "-");
			} else if ((fp = fopen(argv[i], "r")) == nullptr) {
				warn("fopen: %s", argv[i]);
			} else {
				grab(ops, fp, argv[i]);
				fclose(fp);
			}
		}
	}
#endif

#ifdef GRAB_DEBUG
	for (size_t i = 0; i < ops.len; i++)
		regfree(&ops.buf[i].pat);
	free(ops.buf);
#endif

	return rv;
}

struct ops
comppat(char *s)
{
	struct ops ops;

	dainit(&ops, 8);
	while (*s && xisspace(*s))
		s++;
	if (!*s)
		diex(EEARLY);

	do {
		char delim;
		char *p;
		struct op op;

		op.c = *s;
		if (!op_table[(uchar)op.c])
			diex("Invalid operator ‘%c’", *s);
		if (!(delim = *++s))
			diex(EEARLY);

		p = ++s;
		s = xstrchrnul(s, delim);
		op.pat = mkregex(p, s - p);
		dapush(&ops, op);

		if (*s)
			s++;
		while (*s && xisspace(*s))
			s++;
	} while (*s && *(s + 1));

	return ops;
}

void
grab(struct ops ops, FILE *stream, const char *filename)
{
	size_t n;
	struct {
		char *buf;
		size_t len, cap;
	} chars = {0};

	do {
		static_assert(sizeof(char) == 1, "sizeof(char) != 1; wtf?");
		chars.cap += BUFSIZ;
		if ((chars.buf = realloc(chars.buf, chars.cap)) == nullptr)
			die("realloc");
		chars.len += n = fread(chars.buf + chars.len, 1, BUFSIZ, stream);
	} while (n == BUFSIZ);

	if (ferror(stream))
		warn("fread: %s", filename);
	else {
		struct sv sv = {
			.p = chars.buf,
			.len = chars.len,
		};
		pos.col = pos.row = 1;
		pos.bp = pos.p = chars.buf;
		op_table[(uchar)ops.buf[0].c](sv, ops, 0, filename);
	}

	free(chars.buf);
}

void
cmdg(struct sv sv, struct ops ops, size_t i, const char *filename)
{
	int r;
	regmatch_t rm = {
		.rm_so = 0,
		.rm_eo = sv.len,
	};
	struct op op = ops.buf[i];

	r = regexec(&op.pat, sv.p, 1, &rm, REG_STARTEND);
	if ((r == REG_NOMATCH && op.c == 'g') || (r != REG_NOMATCH && op.c == 'G'))
		return;

	if (i + 1 == ops.len)
		putf(sv, op.c == 'g' ? &rm : nullptr, filename);
	else
		op_table[(uchar)ops.buf[i + 1].c](sv, ops, i + 1, filename);
}

void
cmdx(struct sv sv, struct ops ops, size_t i, const char *filename)
{
	regmatch_t rm = {
		.rm_so = 0,
		.rm_eo = sv.len,
	};
	struct op op = ops.buf[i];

	do {
		struct sv nsv;

		if (regexec(&op.pat, sv.p, 1, &rm, REG_STARTEND) == REG_NOMATCH)
			break;
		nsv = (struct sv){
			.p = sv.p + rm.rm_so,
			.len = rm.rm_eo - rm.rm_so,
		};
		if (i + 1 == ops.len)
			putf(nsv, nullptr, filename);
		else
			op_table[(uchar)ops.buf[i + 1].c](nsv, ops, i + 1, filename);

		if (rm.rm_so == rm.rm_eo)
			rm.rm_eo++;
		rm = (regmatch_t){
			.rm_so = rm.rm_eo,
			.rm_eo = sv.len,
		};
	} while (rm.rm_so < rm.rm_eo);
}

void
cmdX(struct sv sv, struct ops ops, size_t i, const char *filename)
{
	regmatch_t rm = {
		.rm_so = 0,
		.rm_eo = sv.len,
	};
	regmatch_t prev = {
		.rm_so = 0,
		.rm_eo = 0,
	};
	struct op op = ops.buf[i];

	do {
		struct sv nsv;

		if (regexec(&op.pat, sv.p, 1, &rm, REG_STARTEND) == REG_NOMATCH)
			break;

		if (prev.rm_so || prev.rm_eo || rm.rm_so) {
			nsv = (struct sv){
				.p = sv.p + prev.rm_eo,
				.len = rm.rm_so - prev.rm_eo,
			};
			if (nsv.len) {
				if (i + 1 == ops.len)
					putf(nsv, nullptr, filename);
				else
					op_table[(uchar)ops.buf[i + 1].c](nsv, ops, i + 1,
					                                  filename);
			}
		}

		prev = rm;
		if (rm.rm_so == rm.rm_eo)
			rm.rm_eo++;
		rm = (regmatch_t){
			.rm_so = rm.rm_eo,
			.rm_eo = sv.len,
		};
	} while (rm.rm_so < rm.rm_eo);

	if (prev.rm_eo < rm.rm_eo) {
		struct sv nsv = {
			.p = sv.p + rm.rm_so,
			.len = rm.rm_eo - rm.rm_so,
		};
		if (i + 1 == ops.len)
			putf(nsv, nullptr, filename);
		else
			op_table[(uchar)ops.buf[i + 1].c](nsv, ops, i + 1, filename);
	}
}

void
putm(struct sv sv, regmatch_t *rm, const char *filename)
{
	static const char *fn, *ln, *ma, *se;

	if (cflag && !fn) {
		char *optstr;
		if ((optstr = env_or_default("GRAB_COLORS", nullptr))) {
			enum {
				OPT_FN,
				OPT_LN,
				OPT_MA,
				OPT_SE,
			};
			/* clang-format off */
			static char *const tokens[] = {
				[OPT_FN] = "fn",
				[OPT_LN] = "ln",
				[OPT_MA] = "ma",
				[OPT_SE] = "se",
				nullptr
			};
			/* clang-format on */

			while (*optstr) {
				char *val;
				switch (getsubopt(&optstr, tokens, &val)) {
				case OPT_FN:
					if (sgrvalid(val))
						fn = val;
					break;
				case OPT_LN:
					if (sgrvalid(val))
						fn = val;
					break;
				case OPT_MA:
					if (sgrvalid(val))
						ma = val;
					break;
				case OPT_SE:
					if (sgrvalid(val))
						se = val;
					break;
				default:
					warnx("invalid color value -- '%s'", val);
					rv = EXIT_FAILURE;
				}
			}
		}

		if (!fn)
			fn = DEFCOL_FN;
		if (!ln)
			ln = DEFCOL_LN;
		if (!ma)
			ma = DEFCOL_MA;
		if (!se)
			se = DEFCOL_SE;
	}

	if (fflag || filecnt > 1) {
		char sep = zflag ? '\0' : ':';
		printf("\33[%sm%s\33[0m"  /* filename */
		       "\33[%sm%c\33[0m", /* separator */
		       fn, filename, se, sep);

		if (bflag) {
			printf("\33[%sm%td\33[0m" /* byte offset */
			       "\33[%sm%c\33[0m", /* separator */
			       ln, sv.p - pos.bp, se, sep);
		} else {
			struct u8view v;
			size_t len = sv.p - pos.p;

			while (u8gnext(&v, (const char8_t **)&pos.p, &len)) {
				if (islbrk(v)) {
					pos.col = 1;
					pos.row++;
				} else
					pos.col++;
			}

			printf("\33[%sm%zu\33[0m" /* row */
			       "\33[%sm%c\33[0m"  /* separator */
			       "\33[%sm%zu\33[0m" /* column */
			       "\33[%sm%c\33[0m", /* separator */
			       ln, pos.row, se, sep, ln, pos.col, se, sep);
		}
	}
	if (rm) {
		fwrite(sv.p, 1, rm->rm_so, stdout);
		printf("\33[%sm%.*s\33[0m", ma, (int)(rm->rm_eo - rm->rm_so),
		       sv.p + rm->rm_so);
		fwrite(sv.p + rm->rm_eo, 1, sv.len - rm->rm_eo, stdout);
	} else
		fwrite(sv.p, 1, sv.len, stdout);
	if (!(sflag && sv.p[sv.len - 1] == '\n'))
		putchar(zflag ? '\0' : '\n');
}

void
putm_nc(struct sv sv, regmatch_t *rm, const char *filename)
{
	(void)rm;

	if (fflag || filecnt > 1) {
		char sep = zflag ? '\0' : ':';
		printf("%s%c", filename, sep);

		if (bflag)
			printf("%td%c", sv.p - pos.bp, sep);
		else {
			struct u8view v;
			size_t len = sv.p - pos.p;

			while (u8gnext(&v, (const char8_t **)&pos.p, &len)) {
				if (islbrk(v)) {
					pos.col = 1;
					pos.row++;
				} else
					pos.col++;
			}

			printf("%zu%c%zu%c", pos.row, sep, pos.col, sep);
		}
	}
	fwrite(sv.p, 1, sv.len, stdout);
	if (!(sflag && sv.p[sv.len - 1] == '\n'))
		putchar(zflag ? '\0' : '\n');
}

bool
islbrk(struct u8view v)
{
	return *v.p == '\n' || (v.len == 2 && memeq(v.p, "\r\n", 2));
}

bool
sgrvalid(const char *s)
{
	if (!s || !*s)
		return false;
	do {
		if ((*s < '0' || *s > '9') && *s != ';')
			return false;
	} while (*++s);

	return true;
}

regex_t
mkregex(char *s, size_t n)
{
	char c = s[n];
	int ret, cflags;
	regex_t r;

	s[n] = 0;
	cflags = REG_EXTENDED | REG_UTF | (nflag ? REG_NEWLINE : REG_DOTALL);
	if (!Uflag)
		cflags |= REG_UCP;
	if ((ret = regcomp(&r, s, cflags)) != 0) {
		char emsg[256];
		regerror(ret, &r, emsg, sizeof(emsg));
		diex("Failed to compile regex: %s", emsg);
	}
	s[n] = c;

	return r;
}

#if GIT_GRAB
FILE *
getfstream(int argc, char *argv[argc])
{
	pid_t pid;
	int fds[2];
	enum {
		FD_R,
		FD_W,
	};

	if (pipe(fds) == -1)
		die("pipe");

	switch (pid = fork()) {
	case -1:
		die("fork");
	case 0:;
		size_t len = argc + 5;
		char **args;

		close(fds[FD_R]);
		if (dup2(fds[FD_W], STDOUT_FILENO) == -1)
			die("dup2");
		close(fds[FD_W]);

		if (!(args = malloc(len * sizeof(char *))))
			die("malloc");
		args[0] = "git";
		args[1] = "ls-files";
		args[2] = "-z";
		args[3] = "--";
		memcpy(args + 4, argv, argc * sizeof(char *));
		args[len - 1] = nullptr;

		execvp("git", args);
		die("execvp: git ls-files -z");
	}

	close(fds[FD_W]);
	return fdopen(fds[FD_R], "r");
}
#endif

char *
env_or_default(const char *e, const char *d)
{
	const char *s = getenv(e);
	return (char *)(s && *s ? s : d);
}

bool
xisspace(char c)
{
	return c == ' ' || c == '\t' || c == '\n';
}

char *
xstrchrnul(const char *s, char c)
{
	for (; *s; s++) {
		if (*s == '\\')
			s++;
		else if (*s == c)
			break;
	}
	return (char *)s;
}
