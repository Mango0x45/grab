#include <stdlib.h>

#include "builder.h"

#include "internal/common.h"

struct u8buf *
u8strfit(struct u8buf *b)
{
	return (b->p = realloc(b->p, b->len)) ? b : nullptr;
}
