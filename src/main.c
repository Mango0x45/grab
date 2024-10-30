#include <langinfo.h>
#include <locale.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <alloc.h>
#include <array.h>
#include <cli.h>
#include <errors.h>
#include <macros.h>
#include <mbstring.h>
#include <pcre2.h>
#include <unicode/prop.h>
#include <unicode/string.h>

#include "exitcodes.h"
#include "tpool.h"
#include "work.h"

#define MAIN_C 1
#include "flags.h"

static bool use_color_p(void);
static op_t *pattern_comp(u8view_t pat);
#if GIT_GRAB
static FILE *getfstream(int globc, char **globv);
#endif

atomic_int rv = EXIT_NOMATCH;
op_t *ops;
/* For use in diagnostic messages */
const char *lquot = "`", *rquot = "'";

/* We need to use different matching functions depending on if we’re using JIT
   matching or not */
typeof(pcre2_match) *pcre2_match_fn;

static char emsg[256]; /* Buffer for PCRE2 error messages */
/* TODO: Use the LUT in work.c */
static const bool opchars[] = {
	['g'] = true,
	['G'] = true,
	['h'] = true,
	['H'] = true,
	['x'] = true,
	['X'] = true,
};

int
main(int argc, char **argv)
{
	mlib_setprogname(argv[0]);
	setlocale(LC_ALL, "");

	if (streq(nl_langinfo(CODESET), "UTF-8")) {
		lquot = u8"‘";
		rquot = u8"’";
	}

	optparser_t parser = mkoptparser(argv);
	static const cli_opt_t opts[] = {
		{'c', U8C("color"),         CLI_NONE},
		{'h', U8C("help"),          CLI_NONE},
		{'i', U8C("ignore-case"),   CLI_NONE},
		{'l', U8C("line"),          CLI_OPT},
		{'p', U8C("predicate"),     CLI_NONE},
		{'s', U8C("strip-newline"), CLI_NONE},
		{'U', U8C("no-unicode"),    CLI_NONE},
		{'z', U8C("zero"),          CLI_NONE},
	};

	for (;;) {
		rune opt = optparse(&parser, opts, lengthof(opts));
		if (opt == 0)
			break;
		switch (opt) {
		case 'c':
			flags.c = true;
			break;
		case 'h':
			execlp("man", "man", "1", mlib_progname(), nullptr);
			err("execlp: man 1 %s:", mlib_progname());
		case 'i':
			flags.i = true;
			break;
		case 'l':
			flags.l = true;
			break;
		case 'p':
			flags.p = true;
			break;
		case 's':
			flags.s = true;
			break;
		case 'U':
			flags.U = true;
			break;
		case 'z':
			flags.z = true;
			break;
		case -1:
			warn(parser.errmsg);
			goto usage;
		}
	}

	if (flags.p && flags.s) {
		warn("-p and -s are mutually exclusive");
		goto usage;
	}
	if (flags.p && flags.z) {
		warn("-p and -z are mutually exclusive");
		goto usage;
	}
	if (flags.s && flags.z) {
		warn("-s and -z are mutually exclusive");
		goto usage;
	}

	argc -= parser.optind;
	argv += parser.optind;

	if (argc == 0) {
	usage:
		usage("[-p | -s | -z] [-cilU] pattern [file ...]", "-h");
		exit(EXIT_FATAL);
	}

	flags.c = flags.c || use_color_p();
	ops = pattern_comp((u8view_t){*argv, strlen(*argv)});

	allocator_t mem = init_heap_allocator(nullptr);

#if GIT_GRAB
	argc--;
	argv++;

	FILE *fstream = getfstream(argc, argv);
	if (fstream == nullptr)
		cerr(EXIT_FATAL, "getfstream:");

	const char **filenames = array_new(mem, typeof(*filenames), 1024);

	size_t len;
	ssize_t nr;
	char *file = nullptr;
	while ((nr = getdelim(&file, &len, 0, fstream)) > 0) {
		/* TODO: Would an arena improve performance? */
		const char *s = strdup(file);
		if (s == nullptr)
			cerr(EXIT_FATAL, "strdup:");
		array_push(&filenames, s);
	}
#else
	if (argc == 1)
		argv = (static char *[]){"-"};
	else {
		argc--;
		argv++;
		flags.do_header = true;
	}
#endif

	tpool_t tp;
	int thrds = tpinit(&tp,
#if GIT_GRAB
		filenames, array_len(filenames)
#else
		(const char **)argv, argc
#endif
	);

    /* Failed to spawn threads */
	if (thrds == 0) {
		unsigned char *buf = array_new(mem, typeof(*buf), 4096);
		for (int i = 0; i < argc; i++) {
			process_file(argv[i], &buf);
			fwrite(buf, 1, array_len(buf), stdout);
			array_hdr(buf)->len = 0;
		}
#if DEBUG
		array_free(buf);
#endif
	}

	if (thrds != 0)
		tpfree(&tp);
#if DEBUG
	pcre2_jit_free_unused_memory(nullptr);
	array_foreach (ops, op) {
		if (op->free_me)
			pcre2_code_free(op->re);
	}
	array_free(ops);
#if GIT_GRAB
	array_foreach (filenames, f)
		free(f);
	array_free(filenames);
#endif
#endif
	return rv;
}

op_t *
pattern_comp(u8view_t pat)
{
	allocator_t mem = init_heap_allocator(nullptr);
	op_t *ops = array_new(mem, op_t, 16);

	for (;;) {
		int w;
		rune ch;

		while ((w = ucsnext(&ch, &pat)) != 0) {
			if (!uprop_is_pat_ws(ch)) {
				VSHFT(&pat, -w);
				break;
			}
		}
		if (pat.len == 0)
			break;

		/* Grab the operator.  We grab the entire next grapheme for
		   better error messages in the case that someone tries to use a
		   non-ASCII grapheme as an operator for whatever reason. */

		op_t op;
		u8view_t g;

		(void)ucsgnext(&g, &pat);
		if (g.len != 1 || *g.p >= lengthof(opchars) || !opchars[*g.p]) {
			cerr(EXIT_FATAL, "Invalid operator %s%.*s%s",
				lquot, SV_PRI_ARGS(g), rquot);
		}
		op.c = (char)*g.p;

		/* Unlike with the operator, we parse the delimeter as a rune
		   instead of a grapheme.  This makes it easier for users to
		   write patterns that match combining characters.  This _may_ be
		   subject to change in the future but for now this is the
		   rationale.  Alongside standard delimeters, if the opening
		   delimeter is a bracket or some other form of paired-bracket
		   (as determined by Unicode) then the closing delimeter is set
		   to the right-hand form of the bracket.  This means that the
		   following are both valid delimeted patterns:

		       /regex/
		       ｢regex｣ */

		rune ldelim, rdelim;
		if ((w = ucsnext(&ldelim, &pat)) == 0)
			cerr(EXIT_FATAL, "Premature end of pattern");
		rdelim = uprop_get_bpb(ldelim);

		/* Find the right delimeter, which is optional for the last
		   operator */
		/* TODO: Change u8view_t.len to ptrdiff_t and use -1 here */
		u8view_t re = {pat.p, (size_t)-1};
		while ((w = ucsnext(&ch, &pat)) != 0) {
			if (ch == rdelim) {
				re.len = pat.p - re.p - w;
				break;
			}
		}
		if (re.len == (size_t)-1)
			re.len = pat.p - re.p;
		if (re.len == 0) {
			if (op.c != 'h') {
				cerr(EXIT_FATAL, "%s%c%s operator given empty regex",
					lquot, op.c, rquot);
			}
			if (array_len(ops) == 0) {
				cerr(EXIT_FATAL,
					"%sh%s operator given empty regex as the first operator",
					lquot, rquot);
			}
			op.re = ops[array_len(ops) - 1].re;
#if DEBUG
			op.free_me = false;
#endif
		} else {
			int ec;
			size_t eoff;
			uint32_t reopts = PCRE2_DOTALL | PCRE2_MATCH_INVALID_UTF | PCRE2_UTF;
			if (flags.i)
				reopts |= PCRE2_CASELESS;
			if (!flags.U)
				reopts |= PCRE2_UCP;
			op.re = pcre2_compile(re.p, re.len, reopts, &ec, &eoff, nullptr);
			if (op.re == nullptr) {
				/* TODO: Ensure the buffer is large enough for the error message */
				(void)pcre2_get_error_message(ec, emsg, sizeof(emsg));
				cerr(EXIT_FATAL, "Failed to compile regex: %s", emsg);
			}
			if ((ec = pcre2_jit_compile(op.re, PCRE2_JIT_COMPLETE)) != 0) {
				/* TODO: Ensure the buffer is large enough for the error message */
				(void)pcre2_get_error_message(ec, emsg, sizeof(emsg));
				warn("Failed to JIT compile regex: %s", emsg);
				rv = EXIT_WARNING;
				pcre2_match_fn = pcre2_match;
			} else
				pcre2_match_fn = pcre2_jit_match;
#if DEBUG
			op.free_me = true;
#endif
		}
		array_push(&ops, op);
	}

	if (array_len(ops) == 0)
		err("Empty pattern");

	return ops;
}

bool
use_color_p(void)
{
	const char *ev = getenv("TERM");
	if (ev != nullptr && streq(ev, "dumb"))
		return false;
	if ((ev = getenv("NO_COLOR")) != nullptr && *ev != 0)
		return false;
	if ((ev = getenv("CLICOLOR_FORCE")) != nullptr && *ev != 0)
		return true;
	return isatty(STDOUT_FILENO);
}

#if GIT_GRAB
FILE *
getfstream(int globc, char **globv)
{
	pid_t pid;
	int fds[2];
	enum { R, W };

	if (pipe(fds) == 1)
		cerr(EXIT_FATAL, "pipe:");

	switch (pid = fork()) {
	case -1:
		cerr(EXIT_FATAL, "fork:");
	case 0:
		static const char *git_grep_argv[] = {
			"git", "grep", "-Ilz", "",
		};

		close(fds[R]);
		if (dup2(fds[W], STDOUT_FILENO) == -1)
			cerr(EXIT_FATAL, "dup2:");
		close(fds[W]);

		size_t argc = globc + lengthof(git_grep_argv) + 1;
		char **argv = malloc(argc * sizeof(char *));
		if (argv == nullptr)
			cerr(EXIT_FATAL, "malloc:");
		memcpy(argv, git_grep_argv, sizeof(git_grep_argv));
		memcpy(argv + lengthof(git_grep_argv), globv, globc * sizeof(char *));
		argv[argc - 1] = nullptr;

		execvp("git", argv);
		cerr(EXIT_FATAL, "execvp: git grep -Ilz '':");
	}

	close(fds[W]);
	return fdopen(fds[R], "r");
}
#endif
