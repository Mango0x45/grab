#ifndef RUNE_UTF8_H
#define RUNE_UTF8_H

#include "internal/qmacros.h"
#define _RUNE_NEEDS_U8VIEW 1
#include "internal/types.h" /* IWYU pragma: export */

#define U8_LEN_MAX (4)

int u8tor(rune *, const char8_t *);
int u8tor_uc(rune *, const char8_t *);

int rtou8(char8_t *, rune, size_t);
size_t u8set(char8_t *, rune, size_t);

char8_t *u8chk(const char8_t *, size_t);
[[unsequenced]] bool u8chkr(rune);

[[unsequenced]] int u8wdth(rune);

size_t u8len(const char8_t *, size_t);

int u8next(rune *, const char8_t **, size_t *);
int u8prev(rune *, const char8_t **, const char8_t *);

char8_t *u8chr(const char8_t *, rune, size_t);
char8_t *u8rchr(const char8_t *, rune, size_t);

size_t u8spn(const char8_t *, size_t, const rune *, size_t);
size_t u8bspn(const char8_t *, size_t, const rune *, size_t);
size_t u8cspn(const char8_t *, size_t, const rune *, size_t);
size_t u8cbspn(const char8_t *, size_t, const rune *, size_t);

#if _RUNE_MACRO_WRAP
#	define u8chk(s, n)      _RUNE_Q_PTR(u8chk, (s), (s), (n))
#	define u8chr(s, ch, n)  _RUNE_Q_PTR(u8chr, (s), (s), (ch), (n))
#	define u8rchr(s, ch, n) _RUNE_Q_PTR(u8rchr, (s), (s), (ch), (n))
#endif

#endif /* !RUNE_UTF8_H */
