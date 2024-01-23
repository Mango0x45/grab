#include <string.h>

#include "builder.h"
#include "utf8.h"

#include "internal/common.h"

struct u8buf *
u8strpushu8(struct u8buf *b, struct u8view v)
{
	if (!u8strgrow(b, b->len + v.len))
		return nullptr;
	memcpy(b->p + b->len, v.p, v.len);
	b->len += v.len;
	return b;
}
