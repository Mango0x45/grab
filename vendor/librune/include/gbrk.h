#ifndef RUNE_GBRK_H
#define RUNE_GBRK_H

#define _RUNE_NEEDS_U8VIEW 1
#include "internal/types.h" /* IWYU pragma: export */

size_t u8glen(const char8_t *, size_t);
size_t u8gnext(struct u8view *, const char8_t **, size_t *);

#endif /* !RUNE_GBRK_H */
