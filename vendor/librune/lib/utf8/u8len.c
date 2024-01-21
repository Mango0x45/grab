#include "utf8.h"

size_t
u8len(const char8_t *s, size_t n)
{
	rune unused;
	size_t m = 0;

	while (u8next(&unused, &s, &n))
		m++;

	return m;
}
