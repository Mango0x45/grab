#include "rune.h"
#define _RUNE_NO_MACRO_WRAPPER 1
#include "utf8.h"

#include "internal/common.h"

char8_t *
u8chk(const char8_t *s, size_t n)
{
	while (n) {
		rune ch;
		int m = u8tor(&ch, s);

		if (ch == RUNE_ERROR)
			return (char8_t *)s;
		n -= m;
	}

	return nullptr;
}
