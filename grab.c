#include <assert.h>
#include <err.h>
#include <libgen.h>
#include <limits.h>
#include <locale.h>
#include <regex.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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
typedef void (*cmd_func)(struct sv, struct ops, size_t);

static void cmdg(struct sv, struct ops, size_t);
static void cmdx(struct sv, struct ops, size_t);
static void cmdy(struct sv, struct ops, size_t);

static void grab(struct ops, FILE *, const char *);
static void putsv(struct sv);
static regex_t mkregex(char *, size_t);
static struct ops comppat(char *);

static bool xisspace(char);
static char *xstrchrnul(const char *, char);

static char delim = '\n';
static int rv = EXIT_SUCCESS;

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
	        "Usage: %s [-z] pattern [file ...]\n"
	        "       %s -h\n",
	        s, s);
	exit(EXIT_FAILURE);
}

int
main(int argc, char **argv)
{
	int opt;
	struct ops ops;

	argv[0] = basename(argv[0]);
	if (argc < 2)
		usage(argv[0]);

	setlocale(LC_ALL, "");

	while ((opt = getopt(argc, argv, "hz")) != -1) {
		switch (opt) {
		case 'h':
			execlp("man", "man", "1", argv[0], NULL);
			die("execlp: man 1 %s", argv[0]);
		case 'z':
			delim = '\0';
			break;
		default:
			usage(argv[0]);
		}
	}

	argc -= optind;
	argv += optind;

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
		op_table[(uchar)ops.buf[0].c](sv, ops, 0);
	}

	free(chars.buf);
}

void
cmdg(struct sv sv, struct ops ops, size_t i)
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
		putsv(sv);
	else
		op_table[(uchar)ops.buf[i + 1].c](sv, ops, i + 1);
}

void
cmdx(struct sv sv, struct ops ops, size_t i)
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
			putsv(nsv);
		else
			op_table[(uchar)ops.buf[i + 1].c](nsv, ops, i + 1);

		if (pm.rm_so == pm.rm_eo)
			pm.rm_eo++;
		pm = (regmatch_t){
			.rm_so = pm.rm_eo,
			.rm_eo = sv.len,
		};
	} while (pm.rm_so < pm.rm_eo);
}

void
cmdy(struct sv sv, struct ops ops, size_t i)
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
				putsv(nsv);
			else
				op_table[(uchar)ops.buf[i + 1].c](nsv, ops, i + 1);
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
			putsv(nsv);
		else
			op_table[(uchar)ops.buf[i + 1].c](nsv, ops, i + 1);
	}
}

void
putsv(struct sv sv)
{
	fwrite(sv.p, 1, sv.len, stdout);
	putchar(delim);
}

regex_t
mkregex(char *s, size_t n)
{
	char c = s[n];
	int ret;
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
	if ((ret = regcomp(&r, s, REG_EXTENDED | REG_NEWLINE)) != 0) {
		char emsg[128];
		regerror(ret, &r, emsg, sizeof(emsg));
		diex("Failed to compile regex: %s", emsg);
	}
	s[n] = c;

	return r;
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
