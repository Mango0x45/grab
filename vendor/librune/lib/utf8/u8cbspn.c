#include "utf8.h"

size_t
u8cbspn(const char8_t *s, size_t n, const rune *p, size_t m)
{
	rune ch;
	size_t k = 0;

	while (u8next(&ch, &s, &n)) {
		for (size_t i = 0; i < m; i++) {
			if (p[i] == ch)
				goto found;
		}

		k += u8wdth(ch);
	}

found:
	return k;
}
