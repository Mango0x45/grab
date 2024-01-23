#include <stdlib.h>

#include "builder.h"

#include "internal/common.h"

struct u8buf *
u8strinit(struct u8buf *b, size_t n)
{
	if (n && !(b->p = malloc(n)))
		return nullptr;
	else
		b->p = nullptr;
	b->len = 0;
	b->cap = n;
	return b;
}
