#include <err.h>
#include <getopt.h>
#include <libgen.h>
#include <limits.h>
#include <locale.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#if !GRAB_IS_C23
#	include <assert.h>
#	include <stdbool.h>
#endif

#include "da.h"

#ifndef REG_STARTEND
#	error "REG_STARTEND not defined"
#endif

#define die(...)  err(EXIT_FAILURE, __VA_ARGS__);
#define diex(...) errx(EXIT_FAILURE, __VA_ARGS__);
#define warn(...) \
	do { \
		warn(__VA_ARGS__); \
		rv = EXIT_FAILURE; \
	} while (0)

#define EEARLY "Input string terminated prematurely"

struct op {
	char c;
	regex_t pat;
};

struct ops {
	struct op *buf;
	size_t len, cap;
};

struct chars {
	char *buf;
	size_t len, cap;
};

struct sv {
	char *p;
	size_t len;
};

typedef unsigned char uchar;
typedef void (*cmd_func)(struct sv, struct ops, size_t, const char *);

static void cmdg(struct sv, struct ops, size_t, const char *);
static void cmdx(struct sv, struct ops, size_t, const char *);
static void cmdy(struct sv, struct ops, size_t, const char *);

static void grab(struct ops, FILE *, const char *);
static void putm(struct sv, const char *);
static regex_t mkregex(char *, size_t);
static struct ops comppat(char *);
static char *env_or_default(const char *, const char *);

static bool xisspace(char);
static char *xstrchrnul(const char *, char);

static int filecnt, rv;
static bool color, fflag, nflag, zflag;

static const cmd_func op_table[UCHAR_MAX] = {
	['g'] = cmdg,
	['v'] = cmdg,
	['x'] = cmdx,
	['y'] = cmdy,
};

static const char esc_table[UCHAR_MAX] = {
	['\\'] = '\\', ['a'] = '\a', ['b'] = '\b', ['f'] = '\f',
	['n'] = '\n',  ['r'] = '\r', ['t'] = '\t', ['v'] = '\v',
};

static void
usage(const char *s)
{
	fprintf(stderr,
	        "Usage: %s [-fnz] pattern [file ...]\n"
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
		{"help",      no_argument, 0, 'h'},
		{"filenames", no_argument, 0, 'f'},
		{"newline",   no_argument, 0, 'n'},
		{"zero",      no_argument, 0, 'z'},
	};

	argv[0] = basename(argv[0]);
	if (argc < 2)
		usage(argv[0]);

	setlocale(LC_ALL, "");

	while ((opt = getopt_long(argc, argv, "fhnz", longopts, NULL)) != -1) {
		switch (opt) {
		case 'f':
			fflag = true;
			break;
		case 'h':
			execlp("man", "man", "1", argv[0], NULL);
			die("execlp: man 1 %s", argv[0]);
		case 'n':
			nflag = true;
			break;
		case 'z':
			zflag = true;
			break;
		default:
			usage(argv[0]);
		}
	}

	argc -= optind;
	argv += optind;
	filecnt = argc - 1;

	if (isatty(STDOUT_FILENO) == 1 && env_or_default("NO_COLOR", NULL))
		color = strcmp(env_or_default("TERM", ""), "dumb") == 0;

	ops = comppat(argv[0]);
	if (argc == 1)
		grab(ops, stdin, "-");
	else {
		for (int i = 1; i < argc; i++) {
			FILE *fp;

			if (strcmp(argv[i], "-") == 0) {
				grab(ops, stdin, "-");
			} else if ((fp = fopen(argv[i], "r")) == NULL) {
				warn("fopen: %s", argv[i]);
			} else {
				grab(ops, fp, argv[i]);
				fclose(fp);
			}
		}
	}

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
#define skip_ws(p) for (; *(p) && xisspace(*(p)); (p)++)
	struct ops ops;

	da_init(&ops, 8);
	skip_ws(s);
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
		da_append(&ops, op);

		if (*s)
			s++;
		skip_ws(s);
	} while (*s && *(s + 1));

	return ops;
#undef skip_ws
}

void
grab(struct ops ops, FILE *stream, const char *filename)
{
	size_t n;
	struct chars chars = {0};

	do {
		static_assert(sizeof(char) == 1, "sizeof(char) != 1; wtf?");
		chars.cap += BUFSIZ;
		if ((chars.buf = realloc(chars.buf, chars.cap)) == NULL)
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
		op_table[(uchar)ops.buf[0].c](sv, ops, 0, filename);
	}

	free(chars.buf);
}

void
cmdg(struct sv sv, struct ops ops, size_t i, const char *filename)
{
	int r;
	regmatch_t pm = {
		.rm_so = 0,
		.rm_eo = sv.len,
	};
	struct op op = ops.buf[i];

	r = regexec(&op.pat, sv.p, 1, &pm, REG_STARTEND);
	if ((r == REG_NOMATCH && op.c == 'g') || (r != REG_NOMATCH && op.c == 'v'))
		return;

	if (i + 1 == ops.len)
		putm(sv, filename);
	else
		op_table[(uchar)ops.buf[i + 1].c](sv, ops, i + 1, filename);
}

void
cmdx(struct sv sv, struct ops ops, size_t i, const char *filename)
{
	regmatch_t pm = {
		.rm_so = 0,
		.rm_eo = sv.len,
	};
	struct op op = ops.buf[i];

	do {
		struct sv nsv;

		if (regexec(&op.pat, sv.p, 1, &pm, REG_STARTEND) == REG_NOMATCH)
			break;
		nsv = (struct sv){.p = sv.p + pm.rm_so, .len = pm.rm_eo - pm.rm_so};
		if (i + 1 == ops.len)
			putm(nsv, filename);
		else
			op_table[(uchar)ops.buf[i + 1].c](nsv, ops, i + 1, filename);

		if (pm.rm_so == pm.rm_eo)
			pm.rm_eo++;
		pm = (regmatch_t){
			.rm_so = pm.rm_eo,
			.rm_eo = sv.len,
		};
	} while (pm.rm_so < pm.rm_eo);
}

void
cmdy(struct sv sv, struct ops ops, size_t i, const char *filename)
{
	regmatch_t pm = {
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

		if (regexec(&op.pat, sv.p, 1, &pm, REG_STARTEND) == REG_NOMATCH)
			break;

		if (prev.rm_so || prev.rm_eo || pm.rm_so) {
			nsv = (struct sv){
				.p = sv.p + prev.rm_eo,
				.len = pm.rm_so - prev.rm_eo,
			};
			if (i + 1 == ops.len)
				putm(nsv, filename);
			else
				op_table[(uchar)ops.buf[i + 1].c](nsv, ops, i + 1, filename);
		}

		prev = pm;
		if (pm.rm_so == pm.rm_eo)
			pm.rm_eo++;
		pm = (regmatch_t){
			.rm_so = pm.rm_eo,
			.rm_eo = sv.len,
		};
	} while (pm.rm_so < pm.rm_eo);

	if (prev.rm_eo < pm.rm_eo) {
		struct sv nsv = {
			.p = sv.p + pm.rm_so,
			.len = pm.rm_eo - pm.rm_so,
		};
		if (i + 1 == ops.len)
			putm(nsv, filename);
		else
			op_table[(uchar)ops.buf[i + 1].c](nsv, ops, i + 1, filename);
	}
}

void
putm(struct sv sv, const char *filename)
{
	static const char *fnc, *sepc;

	if (!fnc) {
		fnc = env_or_default("GRAB_COLOR_FNAME", "35");
		sepc = env_or_default("GRAB_COLOR_SEP", "36");
	}

	if (fflag || filecnt > 1) {
		if (color) {
			printf("\33[%sm%s\33[%sm%c\33[0m", fnc, filename, sepc,
			       zflag ? '\0' : ':');
		} else
			printf("%s%c", filename, zflag ? '\0' : ':');
	}
	fwrite(sv.p, 1, sv.len, stdout);
	putchar(zflag ? '\0' : '\n');
}

regex_t
mkregex(char *s, size_t n)
{
	char c = s[n];
	int ret, cflags;
	regex_t r;

	for (size_t i = 0; i < n - 1; i++) {
		if (s[i] == '\\') {
			char c = esc_table[(uchar)s[i + 1]];
			if (c) {
				for (size_t j = i; j < n - 1; j++)
					s[j] = s[j + 1];
				s[i] = c;
				n--;
			}
		}
	}

	s[n] = 0;
	cflags = REG_EXTENDED;
	if (nflag)
		cflags |= REG_NEWLINE;
	if ((ret = regcomp(&r, s, cflags)) != 0) {
		char emsg[128];
		regerror(ret, &r, emsg, sizeof(emsg));
		diex("Failed to compile regex: %s", emsg);
	}
	s[n] = c;

	return r;
}

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
