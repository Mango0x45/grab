#include <string.h>

#include "builder.h"
#include "utf8.h"

#include "internal/common.h"

struct u8str *
u8strpushstr(struct u8str *b, const char *s)
{
	size_t n = strlen(s);
	if (!u8strgrow(b, b->len + n))
		return nullptr;
	memcpy(b->p + b->len, s, n);
	b->len += n;
	return b;
}
