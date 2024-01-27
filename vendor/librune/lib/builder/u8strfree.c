#include <stdlib.h>

#include "builder.h"

void
u8strfree(struct u8str b)
{
	free(b.p);
}
