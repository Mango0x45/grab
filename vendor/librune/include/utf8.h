#ifndef RUNE_UTF8_H
#define RUNE_UTF8_H

#if __STDC_VERSION__ < 202311L
#	include <stdbool.h> /* IWYU pragma: export */
#	define _RUNE_UNSEQUENCED
#else
#	define _RUNE_UNSEQUENCED [[unsequenced]]
#endif

#define _RUNE_NEEDS_U8VIEW 1
#include "internal/types.h" /* IWYU pragma: export */

#define U8_LEN_MAX (4)

int rtou8(char8_t *, rune, size_t);
int u8tor(rune *, const char8_t *);
int u8tor_uc(rune *, const char8_t *);

const char8_t *u8chk(const char8_t *, size_t);
_RUNE_UNSEQUENCED bool u8chkr(rune);

_RUNE_UNSEQUENCED int u8wdth(rune);

size_t u8len(const char8_t *, size_t);

const char8_t *u8next(rune *, const char8_t **, size_t *);
const char8_t *u8prev(rune *, const char8_t **, const char8_t *);

const char8_t *u8chr(const char8_t *, rune, size_t);
const char8_t *u8rchr(const char8_t *, rune, size_t);

size_t u8set(const char8_t *, rune, size_t);

/* clang-format off */
size_t   u8spn(const char8_t *, size_t, const rune *, size_t);
size_t  u8bspn(const char8_t *, size_t, const rune *, size_t);
size_t  u8cspn(const char8_t *, size_t, const rune *, size_t);
size_t u8cbspn(const char8_t *, size_t, const rune *, size_t);

#define   U8SPN(S, N, P)   u8spn((S), (N), (P), (sizeof(P) / sizeof(*(P))))
#define  U8BSPN(S, N, P)  u8bspn((S), (N), (P), (sizeof(P) / sizeof(*(P))))
#define  U8CSPN(S, N, P)  u8cspn((S), (N), (P), (sizeof(P) / sizeof(*(P))))
#define U8CBSPN(S, N, P) u8cbspn((S), (N), (P), (sizeof(P) / sizeof(*(P))))
/* clang-format on */

#endif /* !RUNE_UTF8_H */
