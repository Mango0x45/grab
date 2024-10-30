#include <sys/mman.h>
#include <sys/stat.h>

#include <stdatomic.h>
#include <stdckdint.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <alloc.h>
#include <array.h>
#include <errors.h>
#include <macros.h>
#include <mbstring.h>
#include <pcre2.h>
#include <unicode/string.h>

#include "exitcodes.h"
#include "flags.h"
#include "work.h"

#define DEFINE_OPERATOR(fn) \
	void operator_##fn(ptrdiff_t opi, u8view_t sv, u8view_t **hl)
#define array_extend_sv(xs, sv) \
	array_extend((xs), (sv).p, (ptrdiff_t)(sv).len)

typedef struct {
	ptrdiff_t row, col;
} pos_t;

static pos_t compute_pos(const char8_t *p);
static bool islbrk(u8view_t g);
static int svposcmp(const void *a, const void *b);
static void write_match_to_buffer(u8view_t sv, u8view_t *hl);

static DEFINE_OPERATOR(dispatch);
static DEFINE_OPERATOR(g);
static DEFINE_OPERATOR(G);
static DEFINE_OPERATOR(h);
static DEFINE_OPERATOR(H);
static DEFINE_OPERATOR(x);
static DEFINE_OPERATOR(X);

static thread_local const char *filename;
static thread_local char8_t *baseptr;
static thread_local const char8_t *last_match;
static thread_local unsigned char **buf;

static typeof(operator_dispatch) *operators[] = {
	['g'] = operator_g,
	['G'] = operator_G,
	['h'] = operator_h,
	['H'] = operator_H,
	['x'] = operator_x,
	['X'] = operator_X,
};

extern atomic_int rv;
extern op_t *ops;
extern bool cflag;
extern typeof(pcre2_match) *pcre2_match_fn;



void
process_file(const char *locl_filename, unsigned char **locl_buf)
{
	filename = locl_filename;
	buf = locl_buf;

	FILE *fp = streq(filename, "-") ? stdin : fopen(filename, "r");
	if (fp == nullptr) {
		warn("fopen: %s:", filename);
		atomic_store(&rv, EXIT_WARNING);
		return;
	}

	allocator_t mem = init_heap_allocator(nullptr);
	if (baseptr == nullptr)
		baseptr = array_new(mem, char8_t, 0x1000);
	size_t bufsz = array_cap(baseptr);
	last_match = baseptr;

	do {
		static_assert(sizeof(char8_t) == 1, "sizeof(char8_t) != 1; wtf?");
		baseptr = array_resz(baseptr, bufsz += BUFSIZ); /* TODO: Bounds checking */
		size_t n = fread(baseptr + array_len(baseptr), 1, BUFSIZ, fp);
		array_hdr(baseptr)->len += n;
	} while (!feof(fp));

	if (ferror(fp)) {
		warn("fread: %s:", filename);
		atomic_store(&rv, EXIT_WARNING);
		goto out;
	}

	/* Shouldn’t need more than 32 ever… */
	static thread_local u8view_t *hl = nullptr;
	if (hl == nullptr)
		hl = array_new(mem, typeof(*hl), 32);

	operator_dispatch(0, (u8view_t){baseptr, array_len(baseptr)}, &hl);
#if DEBUG
	array_free(baseptr);
	baseptr = nullptr;
	array_free(hl);
	hl = nullptr;
#else
	array_hdr(baseptr)->len = 0;
	array_hdr(hl)->len = 0;
#endif

out:
	if (fp != stdin)
		(void)fclose(fp);
}



DEFINE_OPERATOR(dispatch)
{
	if (array_len(ops) == opi) {
		if (flags.p)
			exit(EXIT_SUCCESS);
		atomic_compare_exchange_strong(&rv, &(int){EXIT_NOMATCH}, EXIT_SUCCESS);
		write_match_to_buffer(sv, *hl);
	} else /* Cast to silence GCC warning */
		operators[(unsigned char)ops[opi].c](opi, sv, hl);
}

DEFINE_OPERATOR(g)
{
	pcre2_match_data *md =
		pcre2_match_data_create_from_pattern(ops[opi].re, nullptr);
	int n = pcre2_match_fn(ops[opi].re, sv.p, sv.len, 0, PCRE2_NOTEMPTY,
	                       md, nullptr);
	pcre2_match_data_free(md);

	/* This should never happen */
	if (n == 0)
		cerr(EXIT_FATAL, "PCRE2 match data too small");
	if (n == PCRE2_ERROR_NOMATCH)
		return;
	if (n < 0)
		; /* TODO: Handle error */

	operator_dispatch(opi + 1, sv, hl);
}

DEFINE_OPERATOR(G)
{
	/* TODO: Can we reuse match data? */
	pcre2_match_data *md =
		pcre2_match_data_create_from_pattern(ops[opi].re, nullptr);
	int n = pcre2_match_fn(ops[opi].re, sv.p, sv.len, 0, PCRE2_NOTEMPTY,
	                       md, nullptr);
	pcre2_match_data_free(md);

	/* This should never happen */
	if (n == 0)
		cerr(EXIT_FATAL, "PCRE2 match data too small");
	if (n == PCRE2_ERROR_NOMATCH)
		operator_dispatch(opi + 1, sv, hl);
	if (n < 0)
		; /* TODO: Handle error */
}

DEFINE_OPERATOR(h)
{
	if (flags.p) {
		operator_dispatch(opi + 1, sv, hl);
		return;
	}

	pcre2_match_data *md =
		pcre2_match_data_create_from_pattern(ops[opi].re, nullptr);
	u8view_t sv_save = sv;
	ptrdiff_t origlen = array_len(*hl);
	for (;;) {
		int n = pcre2_match_fn(ops[opi].re, sv.p, sv.len, 0,
		                       PCRE2_NOTEMPTY, md, nullptr);
		/* This should never happen */
		if (n == 0)
			cerr(EXIT_FATAL, "PCRE2 match data too small");
		if (n == PCRE2_ERROR_NOMATCH)
			break;
		if (n < 0)
			; /* TODO: Handle error */

		size_t *ov = pcre2_get_ovector_pointer(md);
		array_push(hl, ((u8view_t){sv.p + ov[0], ov[1] - ov[0]}));
		VSHFT(&sv, ov[1]);
	}
	pcre2_match_data_free(md);
	operator_dispatch(opi + 1, sv_save, hl);
	array_hdr(*hl)->len = origlen;
}

DEFINE_OPERATOR(H)
{
	if (flags.p) {
		operator_dispatch(opi + 1, sv, hl);
		return;
	}

	pcre2_match_data *md =
		pcre2_match_data_create_from_pattern(ops[opi].re, nullptr);
	u8view_t sv_save = sv;
	ptrdiff_t origlen = array_len(*hl);
	for (;;) {
		int n = pcre2_match_fn(ops[opi].re, sv.p, sv.len, 0, PCRE2_NOTEMPTY,
		                       md, nullptr);
		/* This should never happen */
		if (n == 0)
			cerr(EXIT_FATAL, "PCRE2 match data too small");
		if (n == PCRE2_ERROR_NOMATCH)
			break;
		if (n < 0)
			; /* TODO: Handle error */

		size_t *ov = pcre2_get_ovector_pointer(md);
		array_push(hl, ((u8view_t){sv.p, ov[0]}));
		VSHFT(&sv, ov[1]);
	}
	pcre2_match_data_free(md);
	operator_dispatch(opi + 1, sv_save, hl);
	array_hdr(*hl)->len = origlen;
}

DEFINE_OPERATOR(x)
{
	pcre2_match_data *md =
		pcre2_match_data_create_from_pattern(ops[opi].re, nullptr);
	for (;;) {
		int n = pcre2_match_fn(ops[opi].re, sv.p, sv.len, 0, PCRE2_NOTEMPTY,
		                       md, nullptr);
		/* This should never happen */
		if (n == 0)
			cerr(EXIT_FATAL, "PCRE2 match data too small");
		if (n == PCRE2_ERROR_NOMATCH)
			break;
		if (n < 0)
			; /* TODO: Handle error */

		size_t *ov = pcre2_get_ovector_pointer(md);
		operator_dispatch(opi + 1, (u8view_t){sv.p + ov[0], ov[1] - ov[0]}, hl);
		VSHFT(&sv, ov[1]);
	}
	pcre2_match_data_free(md);
}

DEFINE_OPERATOR(X)
{
	pcre2_match_data *md =
		pcre2_match_data_create_from_pattern(ops[opi].re, nullptr);
	for (;;) {
		int n = pcre2_match_fn(ops[opi].re, sv.p, sv.len, 0, PCRE2_NOTEMPTY,
		                       md, nullptr);
		/* This should never happen */
		if (n == 0)
			cerr(EXIT_FATAL, "PCRE2 match data too small");
		if (n == PCRE2_ERROR_NOMATCH)
			break;
		if (n < 0)
			; /* TODO: Handle error */

		size_t *ov = pcre2_get_ovector_pointer(md);
		if (ov[0] != 0)
			operator_dispatch(opi + 1, (u8view_t){sv.p, ov[0]}, hl);
		VSHFT(&sv, ov[1]);
	}
	if (sv.len != 0)
		operator_dispatch(opi + 1, sv, hl);
	pcre2_match_data_free(md);
}



void
write_match_to_buffer(u8view_t sv, u8view_t *hl)
{
	const u8view_t COL_FN = !flags.c ? U8("") : U8("\33[35m");
	const u8view_t COL_HL = !flags.c ? U8("") : U8("\33[01;31m");
	const u8view_t COL_LN = !flags.c ? U8("") : U8("\33[32m");
	const u8view_t COL_SE = !flags.c ? U8("") : U8("\33[36m");
	const u8view_t COL_RS = !flags.c ? U8("") : U8("\33[0m");

	if (
#if GIT_GRAB
		true
#else
		flags.do_header
#endif
	) {
		char sep = flags.z ? 0 : ':';

		size_t filenamesz = strlen(filename);

		array_extend_sv(buf, COL_FN);
		array_extend(buf, filename, (ptrdiff_t)filenamesz);
		array_extend_sv(buf, COL_RS);

		array_extend_sv(buf, COL_SE);
		array_push(buf, sep);
		array_extend_sv(buf, COL_RS);

		/* GCC thinks ‘offset’ can overflow because our offsets have type
		   ptrdiff_t which if negative would have a ‘-’ in the front, but
		   we know that the match positions can’t be negative so it’s
		   safe to ignore. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-overflow"

		int offsetsz;
		char offset[/* len(INT64_MAX - 1) */ 19];
		if (flags.l) {
			pos_t p = compute_pos(sv.p);

			offsetsz = sprintf(offset, "%td", p.row + 1);
			array_extend_sv(buf, COL_LN);
			array_extend(buf, offset, offsetsz);
			array_extend_sv(buf, COL_RS);

			array_extend_sv(buf, COL_SE);
			array_push(buf, sep);
			array_extend_sv(buf, COL_RS);

			offsetsz = sprintf(offset, "%td", p.col + 1);
			array_extend_sv(buf, COL_LN);
			array_extend(buf, offset, offsetsz);
			array_extend_sv(buf, COL_RS);
		} else {
			offsetsz = sprintf(offset, "%td", sv.p - baseptr);
			array_extend_sv(buf, COL_LN);
			array_extend(buf, offset, offsetsz);
			array_extend_sv(buf, COL_RS);
		}

		array_extend_sv(buf, COL_SE);
		array_push(buf, sep);
		array_extend_sv(buf, COL_RS);
	}

#pragma GCC diagnostic pop

	/* Here we need to take all the views of regions to highlight, and try
	   to merge them into a simpler form.  This happens in two steps:

	   1. Sort the views by their starting position in the matched text.
	   2. Merge overlapping views.

	   After this process we should have the most reduced possible set of
	   views.  The next part is to actually print the highlighted regions
	   possible which requires bounds-checking as highlighted regions may
	   begin before or end after the matched text when using patterns such
	   as ‘h/.+/ x/.$/’. */

	static thread_local u8view_t *sorted = nullptr;
	if (sorted == nullptr) {
		allocator_t mem = init_heap_allocator(nullptr);
		ptrdiff_t buflen = array_len(hl);
		buflen = MAX(buflen, 16);
		sorted = array_new(mem, typeof(*sorted), buflen);
	} else
		array_hdr(sorted)->len = 0;

	array_extend(&sorted, hl, array_len(hl));
	qsort(sorted, array_len(sorted), sizeof(*sorted), svposcmp);

	for (ptrdiff_t i = 0, len = array_len(sorted); i < len; i++) {
		ptrdiff_t Δ;
		u8view_t h = sorted[i];

		if ((Δ = h.p - sv.p) < 0)
			VSHFT(&h, -Δ);
		if ((Δ = (h.p + h.len) - (sv.p + sv.len)) > 0)
			h.len -= Δ;
		if (h.len <= 0)
			continue;

		array_extend(buf, sv.p, h.p - sv.p);
		array_extend_sv(buf, COL_HL);
		array_extend_sv(buf, h);
		array_extend_sv(buf, COL_RS);

		Δ = h.p - sv.p + h.len;
		VSHFT(&sv, Δ);
	}
	array_extend_sv(buf, sv);

#if DEBUG
	array_free(sorted);
	sorted = nullptr;
#endif

	if (flags.z)
		array_push(buf, 0);
	else {
		ptrdiff_t bufsz = array_len(*buf);
		if (!flags.s || bufsz == 0 || (*buf)[bufsz - 1] != '\n')
			array_push(buf, '\n');
	}
}

pos_t
compute_pos(const char8_t *ptr)
{
	static thread_local pos_t p;
	if (last_match == baseptr)
		p.row = p.col = 0;
	u8view_t g, sv = {last_match, PTRDIFF_MAX};
	while (sv.p < ptr) {
		ucsgnext(&g, &sv);
		if (islbrk(g)) {
			p.row++;
			p.col = 0;
		} else
			p.col = ucswdth(g, p.col, 8);  /* TODO: Configurable tabsize? */
	}
	last_match = sv.p;
	return p;
}

bool
islbrk(u8view_t g)
{
	return ucseq(g, U8("\n"))
	    || ucseq(g, U8("\v"))
	    || ucseq(g, U8("\f"))
	    || ucseq(g, U8("\r\n"))
	    || ucseq(g, U8("\x85"))
	    || ucseq(g, U8("\u2028"))
	    || ucseq(g, U8("\u2029"));
}

int
svposcmp(const void *a_, const void *b_)
{
	const u8view_t *a = a_,
	               *b = b_;
	ptrdiff_t Δ = a->p - b->p;
	return Δ == 0 ? (ptrdiff_t)a->len - (ptrdiff_t)b->len : Δ;
}
