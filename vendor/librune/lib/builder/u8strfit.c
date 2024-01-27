#include <stdlib.h>

#include "builder.h"

#include "internal/common.h"

struct u8str *
u8strfit(struct u8str *b)
{
	return (b->p = realloc(b->p, b->len)) ? b : nullptr;
}
