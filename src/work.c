#include <sys/mman.h>
#include <sys/stat.h>

#include <errno.h>
#include <fcntl.h>
#include <stdatomic.h>
#include <stdbit.h>
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
#include <rune.h>
#include <unicode/string.h>

#include "exitcodes.h"
#include "globals.h"
#include "util.h"
#include "work.h"

#define DEFINE_OPERATOR(fn) \
	void operator_##fn(ptrdiff_t opi, u8view_t sv, u8view_t **hl)
#define array_extend_sv(xs, sv) \
	array_extend((xs), (sv).p, (ptrdiff_t)(sv).len)

typedef struct {
	u8view_t buf;
	struct {
		ptrdiff_t row;
		ptrdiff_t col;
	};
} pos_state_t;

static void compute_pos(const char8_t *p, pos_state_t *ps);
static bool has_lbrk_p(u8view_t sv);
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
static thread_local unsigned char **buf;

static typeof(operator_dispatch) *operators[] = {
	['g'] = operator_g,
	['G'] = operator_G,
	['h'] = operator_h,
	['H'] = operator_H,
	['x'] = operator_x,
	['X'] = operator_X,
};



void
process_file(const char *locl_filename, unsigned char **locl_buf)
{
	ptrdiff_t baselen;
	static thread_local off_t basecap;

	filename = locl_filename;
	buf = locl_buf;

	int fd = streq(filename, "-") ? STDIN_FILENO : open(filename, O_RDONLY);
	if (fd == -1) {
		warn("open: %s:", filename);
		goto err;
	}

#if !defined(__APPLE__) && !defined(__OpenBSD__)
	(void)posix_fadvise(fd, 0, 0, POSIX_FADV_SEQUENTIAL | POSIX_FADV_WILLNEED);
#endif

	struct stat st;
	if (fstat(fd, &st) == -1) {
		warn("fstat: %s:", filename);
		goto err;
	}

	/* We need to assert for st.st_size > 0 as some files (such as various files
       under /proc on Linux) are generated ‘on the fly’ and will report a
       filesize of 0.  Despite being regular files, these files need to be
       treated as if they are special. */
	if (S_ISREG(st.st_mode) && st.st_size > 0) {
#if __linux__
		(void)readahead(fd, 0, st.st_size);
#endif
		if (st.st_size > basecap) {
			basecap = st.st_size;
			if ((baseptr = realloc(baseptr, st.st_size)) == nullptr)
				cerr(EXIT_FATAL, "realloc:");
		}
		(void)madvise(baseptr, st.st_size, POSIX_MADV_SEQUENTIAL);

		ptrdiff_t nw = 0;
		for (;;) {
			ssize_t nr = read(fd, baseptr + nw, st.st_size - nw);
			if (nr == -1) {
				if (errno == EINTR)
					continue;
				warn("read: %s:", filename);
				goto err;
			}
			if (nr == 0)
				break;
			nw += nr;
		}
		baselen = st.st_size;
	} else {
		ptrdiff_t nw = 0;
		for (;;) {
			ptrdiff_t want = nw + st.st_blksize;
			if (want > basecap) {
				if (want & ((ptrdiff_t)1 << (PTRDIFF_WIDTH - 2))) {
					errno = EOVERFLOW;
					cerr(EXIT_FATAL, "%s:", __func__);
				}
				basecap = (ptrdiff_t)stdc_bit_ceil((size_t)want);
				if ((baseptr = realloc(baseptr, basecap)) == nullptr)
					cerr(EXIT_FATAL, "realloc:");
			}
			ssize_t nr = read(fd, baseptr + nw, st.st_blksize);
			if (nr == -1) {
				if (errno == EINTR)
					continue;
				warn("read: %s:", filename);
				goto err;
			}
			if (nr == 0)
				break;
			nw += nr;
		}
		baselen = nw;
	}

	/* Shouldn’t need more than 32 ever… */
	allocator_t mem = init_heap_allocator(nullptr);
	static thread_local u8view_t *hl = nullptr;
	if (hl == nullptr)
		hl = array_new(mem, typeof(*hl), 32);

	operator_dispatch(0, (u8view_t){baseptr, baselen}, &hl);

	if (fd != -1)
		(void)close(fd);
#if DEBUG
	free(baseptr);
	array_free(hl);
	baseptr = nullptr;
	basecap = 0;
	hl = nullptr;
#endif
	return;

err:
	if (fd != -1)
		(void)close(fd);
	atomic_store(&rv, EXIT_WARNING);
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
	int n = pcre2_jit_match(ops[opi].re, sv.p, sv.len, 0, PCRE2_NOTEMPTY,
	                        md, nullptr);
	pcre2_match_data_free(md);

	if (n == PCRE2_ERROR_NOMATCH)
		return;
	if (n < 0)
		pcre2_bitch_and_die(n, "failed to match regex");

	operator_dispatch(opi + 1, sv, hl);
}

DEFINE_OPERATOR(G)
{
	pcre2_match_data *md =
		pcre2_match_data_create_from_pattern(ops[opi].re, nullptr);
	int n = pcre2_jit_match(ops[opi].re, sv.p, sv.len, 0, PCRE2_NOTEMPTY,
	                        md, nullptr);
	pcre2_match_data_free(md);

	if (n == PCRE2_ERROR_NOMATCH)
		operator_dispatch(opi + 1, sv, hl);
	if (n < 0)
		pcre2_bitch_and_die(n, "failed to match regex");
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
		int n = pcre2_jit_match(ops[opi].re, sv.p, sv.len, 0,
		                        PCRE2_NOTEMPTY, md, nullptr);
		if (n == PCRE2_ERROR_NOMATCH)
			break;
		if (n < 0)
			pcre2_bitch_and_die(n, "failed to match regex");

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
		int n = pcre2_jit_match(ops[opi].re, sv.p, sv.len, 0, PCRE2_NOTEMPTY,
		                        md, nullptr);
		if (n == PCRE2_ERROR_NOMATCH)
			break;
		if (n < 0)
			pcre2_bitch_and_die(n, "failed to match regex");

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
		int n = pcre2_jit_match(ops[opi].re, sv.p, sv.len, 0, PCRE2_NOTEMPTY,
		                        md, nullptr);
		if (n == PCRE2_ERROR_NOMATCH)
			break;
		if (n < 0)
			pcre2_bitch_and_die(n, "failed to match regex");

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
		int n = pcre2_jit_match(ops[opi].re, sv.p, sv.len, 0, PCRE2_NOTEMPTY,
		                        md, nullptr);
		if (n == PCRE2_ERROR_NOMATCH)
			break;
		if (n < 0)
			pcre2_bitch_and_die(n, "failed to match regex");

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
		pos_state_t ps = {.buf = {baseptr, PTRDIFF_MAX}};

		if (flags.b) {
			offsetsz = sprintf(offset, "%td", sv.p - baseptr);
			array_extend_sv(buf, COL_LN);
			array_extend(buf, offset, offsetsz);
			array_extend_sv(buf, COL_RS);
		} else {
			compute_pos(sv.p, &ps);

			offsetsz = sprintf(offset, "%td", ps.row + 1);
			array_extend_sv(buf, COL_LN);
			array_extend(buf, offset, offsetsz);
			array_extend_sv(buf, COL_RS);

			array_extend_sv(buf, COL_SE);
			array_push(buf, sep);
			array_extend_sv(buf, COL_RS);

			offsetsz = sprintf(offset, "%td", ps.col + 1);
			array_extend_sv(buf, COL_LN);
			array_extend(buf, offset, offsetsz);
			array_extend_sv(buf, COL_RS);
		}

		array_extend_sv(buf, COL_SE);
		array_push(buf, sep);
		array_extend_sv(buf, COL_RS);

		switch (flags.H) {
		case HDR_ALWAYS:
			array_push(buf, '\n');
			break;
		case HDR_MULTI:
			if (has_lbrk_p(sv))
				array_push(buf, '\n');
		}
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

void
compute_pos(const char8_t *p, pos_state_t *ps)
{
	u8view_t g;
	while (ps->buf.p < p) {
		ucsgnext(&g, &ps->buf);
		if (islbrk(g)) {
			ps->row++;
			ps->col = 0;
		} else
			ps->col = ucswdth(g, ps->col, grab_tabsize);
	}
}

bool
has_lbrk_p(u8view_t sv)
{
	rune ch;
	while (ucsnext(&ch, &sv) != 0) {
		switch (ch) {
		case '\r':     /* Not *really* a newline, but it can mess with output */
		case '\n':
		case '\v':
		case '\f':
		case 0x85:
		case RUNE_C(0x2028):
		case RUNE_C(0x2029):
			return true;
		}
	}
	return false;
}

int
svposcmp(const void *a_, const void *b_)
{
	const u8view_t *a = a_,
	               *b = b_;
	ptrdiff_t Δ = a->p - b->p;
	return Δ == 0 ? (ptrdiff_t)a->len - (ptrdiff_t)b->len : Δ;
}