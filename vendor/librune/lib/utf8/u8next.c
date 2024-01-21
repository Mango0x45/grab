#include "utf8.h"

#include "internal/common.h"

const char8_t *
u8next(rune *ch, const char8_t **s, size_t *n)
{
	int m;

	if (*n == 0)
		return nullptr;
	*n -= m = u8tor_uc(ch, *s);
	return *s += m;
}
