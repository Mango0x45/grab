#include <err.h>
#include <getopt.h>
#include <libgen.h>
#include <limits.h>
#include <locale.h>
#include <stddef.h>
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
#include <rune.h>
#include <utf8.h>

#include "compat.h"
#include "da.h"

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define lengthof(a) (sizeof(a) / sizeof(*(a)))

#define die(...)  err(EXIT_FAILURE, __VA_ARGS__)
#define diex(...) errx(EXIT_FAILURE, __VA_ARGS__)
#define warn(...) \
	do { \
		warn(__VA_ARGS__); \
		rv = EXIT_FAILURE; \
	} while (0)

#define streq(a, b)    (!strcmp(a, b))
#define memeq(a, b, n) (!memcmp(a, b, n))

#define DEFCOL_FN "35"
#define DEFCOL_HL "01;31"
#define DEFCOL_LN "32"
#define DEFCOL_SE "36"

struct matches {
	struct sv *buf;
	size_t len, cap;
};

struct op {
	char c;
	regex_t pat;
#ifdef GRAB_DEBUG
	bool alloced;
#endif
};

struct ops {
	struct op *buf;
	size_t len, cap;
};

struct sv {
	char8_t *p;
	size_t len;
};

typedef unsigned char uchar;
typedef void cmd_func(struct sv, struct matches *, struct ops, size_t,
                      const char *);
typedef void put_func(struct sv, struct matches *, const char *);

static cmd_func cmdg, cmdh, cmdH, cmdx, cmdX;
static put_func putm, putm_nc;

#if GIT_GRAB
static FILE *getfstream(int n, char *v[n]);
#endif
static void grab(struct ops, FILE *, const char *);
static struct ops comppat(char8_t *);
static regex_t mkregex(char8_t *, size_t);
static bool islbrk(struct u8view);
static bool sgrvalid(const char *);
static bool xisspace(char);
static int svposcmp(const void *, const void *);
static char *env_or_default(const char *, const char *);

static int filecnt, rv;
static bool bflag, cflag, nflag, iflag, sflag, Uflag, zflag;
static bool fflag = GIT_GRAB;
static put_func *putf;

static struct {
	const char8_t *p, *bp;
	size_t col, row;
} pos;

static cmd_func *op_table[UCHAR_MAX] = {
	['g'] = cmdg, ['G'] = cmdg, ['h'] = cmdh,
	['H'] = cmdH, ['x'] = cmdx, ['X'] = cmdX,
};

static void
usage(const char *s)
{
	fprintf(stderr,
#if GIT_GRAB
	        "Usage: %s [-s | -z] [-bcinU] pattern [glob ...]\n"
#else
	        "Usage: %s [-s | -z] [-bcfinU] pattern [file ...]\n"
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
		{"ignore-case",   no_argument, nullptr, 'i'},
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
	const char *opts = "bchinsUz";
#else
	const char *opts = "bcfhinsUz";
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
		case 'i':
			iflag = true;
			break;
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
	if (!(flist = getfstream(argc - 1, argv + 1)))
		die("getfstream");
	while ((nr = getdelim(&entry, &len, '\0', flist)) > 0) {
		FILE *fp;

		if (!(fp = fopen(entry, "r")))
			warn("fopen: %s", entry);
		else {
			grab(ops, fp, entry);
			fclose(fp);
		}
	}
	if (ferror(flist))
		warn("getdelim");
	fclose(flist);
#else
	if (argc == 1)
		grab(ops, stdin, "-");
	else {
		for (int i = 1; i < argc; i++) {
			FILE *fp;

			if (streq(argv[i], "-")) {
				grab(ops, stdin, "-");
			} else if (!(fp = fopen(argv[i], "r"))) {
				warn("fopen: %s", argv[i]);
			} else {
				grab(ops, fp, argv[i]);
				fclose(fp);
			}
		}
	}
#endif

#ifdef GRAB_DEBUG
#	if GIT_GRAB
	free(entry);
#	endif
	for (size_t i = 0; i < ops.len; i++) {
		if (ops.buf[i].alloced)
			regfree(&ops.buf[i].pat);
	}
	free(ops.buf);
#endif

	return rv;
}

struct ops
comppat(char8_t *s)
{
	struct ops ops;

	dainit(&ops, 8);
	while (*s && xisspace(*s))
		s++;
	if (!*s)
		diex("Input string terminated prematurely");

	do {
		int w;
		rune ch;
		size_t len;
		char8_t *p;
		struct op op;

		/* Grab the operator and delimiter.  All operators are ASCII, but
		   u8tor() is used to parse it so that we get properly formed error
		   messages when someone uses a non-ASCII operator. */
		w = u8tor(&ch, s);
		if (ch == RUNE_ERROR)
			diex("Invalid UTF-8 sequence near ‘%02hhX’", s[-1]);
		if (w > 1 || !op_table[ch])
			diex("Invalid operator ‘%.*s’", w, s);
		op.c = *s++;

		s += u8tor(&ch, s);
		if (ch == RUNE_ERROR)
			diex("Invalid UTF-8 sequence near ‘%02hhX’", s[-1]);
		if (ch == '\0')
			diex("Input string terminated prematurely");

		/* Find the closing delimiter.  The user is allowed to omit the closing
		   delimiter if this is the last operation in the query pattern. */
		p = s;
		len = strlen(s);
		if (!(s = (char8_t *)u8chr(s, ch, len)))
			s = p + len;

		if (s - p == 0) {
			if (op.c != 'h')
				diex("Empty regex given to ‘%c’", op.c);
			if (ops.len == 0)
				diex("Empty ‘h’ is not allowed as the first operator");
			op.pat = ops.buf[ops.len - 1].pat;
		} else
			op.pat = mkregex(p, s - p);

#if GRAB_DEBUG
		op.alloced = s - p == 0;
#endif

		dapush(&ops, op);

		if (*s) {
			s += u8tor(&ch, s);
			if (ch == RUNE_ERROR)
				diex("Invalid UTF-8 sequence near ‘%02hhX’", s[-1]);
		}
		while (*s && xisspace(*s))
			s++;
	} while (*s);

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
		if (!(chars.buf = realloc(chars.buf, chars.cap)))
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
		struct matches ms;

		dainit(&ms, 4);
		pos.col = pos.row = 1;
		pos.bp = pos.p = chars.buf;
		op_table[(uchar)ops.buf[0].c](sv, &ms, ops, 0, filename);
		free(ms.buf);
	}

	free(chars.buf);
}

void
cmdg(struct sv sv, struct matches *ms, struct ops ops, size_t i,
     const char *filename)
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
		putf(sv, ms, filename);
	else
		op_table[(uchar)ops.buf[i + 1].c](sv, ms, ops, i + 1, filename);
}

void
cmdh(struct sv sv, struct matches *ms, struct ops ops, size_t i,
     const char *filename)
{
	regmatch_t rm = {
		.rm_so = 0,
		.rm_eo = sv.len,
	};
	struct op op = ops.buf[i];

	do {
		if (regexec(&op.pat, sv.p, 1, &rm, REG_STARTEND) == REG_NOMATCH)
			break;

		dapush(ms, ((struct sv){sv.p + rm.rm_so, rm.rm_eo - rm.rm_so}));

		if (rm.rm_so == rm.rm_eo)
			rm.rm_eo++;
		rm = (regmatch_t){
			.rm_so = rm.rm_eo,
			.rm_eo = sv.len,
		};
	} while (rm.rm_so < rm.rm_eo);

	if (i + 1 == ops.len)
		putf(sv, ms, filename);
	else {
		size_t save = ms->len;
		op_table[(uchar)ops.buf[i + 1].c](sv, ms, ops, i + 1, filename);
		ms->len = save;
	}
}

void
cmdH(struct sv sv, struct matches *ms, struct ops ops, size_t i,
     const char *filename)
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
			if (nsv.len)
				dapush(ms, nsv);
		}

		prev = rm;
		if (rm.rm_so == rm.rm_eo)
			rm.rm_eo++;
		rm = (regmatch_t){
			.rm_so = rm.rm_eo,
			.rm_eo = sv.len,
		};
	} while (rm.rm_so < rm.rm_eo);

	if (prev.rm_eo < rm.rm_eo)
		dapush(ms, ((struct sv){sv.p + rm.rm_so, rm.rm_eo - rm.rm_so}));

	if (i + 1 == ops.len)
		putf(sv, ms, filename);
	else
		op_table[(uchar)ops.buf[i + 1].c](sv, ms, ops, i + 1, filename);
}

void
cmdx(struct sv sv, struct matches *ms, struct ops ops, size_t i,
     const char *filename)
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
			putf(nsv, ms, filename);
		else {
			size_t save = ms->len;
			op_table[(uchar)ops.buf[i + 1].c](nsv, ms, ops, i + 1, filename);
			ms->len = save;
		}

		if (rm.rm_so == rm.rm_eo)
			rm.rm_eo++;
		rm = (regmatch_t){
			.rm_so = rm.rm_eo,
			.rm_eo = sv.len,
		};
	} while (rm.rm_so < rm.rm_eo);
}

void
cmdX(struct sv sv, struct matches *ms, struct ops ops, size_t i,
     const char *filename)
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
					putf(nsv, ms, filename);
				else
					op_table[(uchar)ops.buf[i + 1].c](nsv, ms, ops, i + 1,
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
			putf(nsv, ms, filename);
		else
			op_table[(uchar)ops.buf[i + 1].c](nsv, ms, ops, i + 1, filename);
	}
}

int
svposcmp(const void *a, const void *b)
{
	struct sv *A, *B;
	A = (struct sv *)a;
	B = (struct sv *)b;
	return A->p != B->p ? A->p - B->p : A->len < B->len ? -1 : A->len != B->len;
}

void
putm(struct sv sv, struct matches *ms, const char *filename)
{
	const char8_t *p;
	struct matches valid;
	static const char *fn, *hl, *ln, *se;

	if (cflag && !fn) {
		char *optstr;
		if ((optstr = env_or_default("GRAB_COLORS", nullptr))) {
			enum {
				OPT_FN,
				OPT_HL,
				OPT_LN,
				OPT_SE,
			};
			/* clang-format off */
			static char *const tokens[] = {
				[OPT_FN] = "fn",
				[OPT_HL] = "hl",
				[OPT_LN] = "ln",
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
				case OPT_HL:
					if (sgrvalid(val))
						hl = val;
					break;
				case OPT_LN:
					if (sgrvalid(val))
						fn = val;
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
		if (!hl)
			hl = DEFCOL_HL;
		if (!ln)
			ln = DEFCOL_LN;
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

			while (u8gnext(&v, &pos.p, &len)) {
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

	/* Here we need to take all the views of regions to highlight, and try
	   to merge them into a simpler form.  This happens in two steps:

	   1. Sort the views by their starting position in the matched text.
	   2. Merge overlapping views.

	   After this process we should have the most reduced possible set of
	   views.  The next part is to actually print the highlighted regions
	   possible which requires bounds-checking as highlighted regions may
	   begin before or end after the matched text when using patterns such
	   as ‘h/.+/ x/.$/’. */

	dainit(&valid, ms->len);
	qsort(ms->buf, ms->len, sizeof(*ms->buf), svposcmp);
	memcpy(valid.buf, ms->buf, ms->len * sizeof(*ms->buf));
	valid.len = ms->len;

	for (size_t i = 0; i + 1 < valid.len;) {
		ptrdiff_t d;
		struct sv *a, *b;

		a = valid.buf + i;
		b = valid.buf + i + 1;
		d = a->p + a->len - b->p;

		if (d >= 0) {
			a->len += MAX(b->len - d, 0);
			daremove(&valid, i + 1);
		} else
			i++;
	}

	for (size_t i = 0; i < valid.len; i++) {
		struct sv *m = valid.buf + i;
		if (m->p + m->len < sv.p || m->p >= sv.p + sv.len) {
			daremove(&valid, i);
			i--;
			continue;
		}

		if (m->p < sv.p) {
			m->len -= sv.p - m->p;
			m->p = sv.p;
		}
		m->len = MIN(m->len, (size_t)(sv.p + sv.len - m->p));
	}

	p = sv.p;
	for (size_t i = 0; i < valid.len; i++) {
		struct sv m = valid.buf[i];
		printf("%.*s\33[%sm%.*s\33[0m", (int)(m.p - p), p, hl, (int)m.len, m.p);
		p = m.p + m.len;
	}
	fwrite(p, 1, sv.p + sv.len - p, stdout);

	if (!(sflag && sv.p[sv.len - 1] == '\n'))
		putchar(zflag ? '\0' : '\n');
	free(valid.buf);
}

void
putm_nc(struct sv sv, struct matches *ms, const char *filename)
{
	(void)ms;

	if (fflag || filecnt > 1) {
		char sep = zflag ? '\0' : ':';
		printf("%s%c", filename, sep);

		if (bflag)
			printf("%td%c", sv.p - pos.bp, sep);
		else {
			struct u8view v;
			size_t len = sv.p - pos.p;

			while (u8gnext(&v, &pos.p, &len)) {
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
mkregex(char8_t *s, size_t n)
{
	int ret, cflags;
	regex_t r;
	char8_t c = s[n];

	s[n] = 0;
	cflags = REG_EXTENDED | REG_UTF | (nflag ? REG_NEWLINE : REG_DOTALL);
	if (iflag)
		cflags |= REG_ICASE;
	if (!Uflag)
		cflags |= REG_UCP;
	if (ret = regcomp(&r, s, cflags)) {
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
		size_t len;
		char **args;
		static const char *git_grep_args[] = {
			"git", "grep", "--cached", "-Ilz", "",
		};

		len = argc + lengthof(git_grep_args) + 1;

		close(fds[FD_R]);
		if (dup2(fds[FD_W], STDOUT_FILENO) == -1)
			die("dup2");
		close(fds[FD_W]);

		if (!(args = malloc(len * sizeof(char *))))
			die("malloc");
		memcpy(args, git_grep_args, sizeof(git_grep_args));
		memcpy(args + 5, argv, argc * sizeof(char *));
		args[len - 1] = nullptr;

		execvp("git", args);
		die("execvp: git grep --cached -Ilz ''");
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
