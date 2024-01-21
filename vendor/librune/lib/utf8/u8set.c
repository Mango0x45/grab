#include <string.h>

#include "utf8.h"

#include "internal/common.h"

size_t
u8set(const char8_t *s, rune ch, size_t n)
{
	int m;
	char8_t buf[U8_LEN_MAX];

	if (n == 0)
		return 0;
	if (ch <= _1B_MAX) {
		memset((char *)s, ch, n);
		return n;
	}
	m = rtou8(buf, ch, sizeof(buf));
	for (size_t i = 0; i < n; i += m)
		memcpy((char *)s + i, buf, m);

	return n - n % m;
}
