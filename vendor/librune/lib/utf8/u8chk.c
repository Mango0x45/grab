#include "rune.h"
#include "utf8.h"

#include "internal/common.h"

const char8_t *
u8chk(const char8_t *s, size_t n)
{
	while (n) {
		rune ch;
		int m = u8tor(&ch, s);

		if (ch == RUNE_ERROR)
			return s;
		n -= m;
	}

	return nullptr;
}
