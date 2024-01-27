/* IWYU pragma: private */

#include <stddef.h> /* IWYU pragma: export */
#include <stdint.h>

typedef unsigned char char8_t;
typedef uint_least32_t rune;

#if _RUNE_NEEDS_U8VIEW && !_RUNE_HAS_U8VIEW
struct u8view {
	const char8_t *p;
	size_t len;
};
#	define _RUNE_HAS_U8VIEW 1
#endif
