#define _RUNE_NO_MACRO_WRAPPER 1
#include "utf8.h"

int
u8next(rune *ch, const char8_t **s, size_t *n)
{
	int m = 0;

	if (*n) {
		m = u8tor_uc(ch, *s);
		*n -= m;
		*s += m;
	}

	return m;
}
