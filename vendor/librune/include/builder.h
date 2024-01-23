#ifndef RUNE_BUILDER_H
#define RUNE_BUILDER_H

#define _RUNE_NEEDS_U8VIEW 1
#include "internal/types.h" /* IWYU pragma: export */

struct u8buf {
	char8_t *p;
	size_t len, cap;
};

struct u8buf *u8strinit(struct u8buf *, size_t);
struct u8buf *u8strgrow(struct u8buf *, size_t);
struct u8buf *u8strfit(struct u8buf *);
void u8strfree(struct u8buf);

struct u8buf *u8strpushr(struct u8buf *, rune);
struct u8buf *u8strpushstr(struct u8buf *, const char *);
struct u8buf *u8strpushu8(struct u8buf *, struct u8view);

#if __STDC_VERSION__ >= 201112L
#	define u8strpush(b, x) \
		_Generic((x), \
		    char: u8strpushr, \
		    int: u8strpushr, \
		    rune: u8strpushr, \
		    char *: u8strpushstr, \
		    struct u8view: u8strpushu8)((b), (x))
#endif

#endif /* !RUNE_BUILDER_H */
