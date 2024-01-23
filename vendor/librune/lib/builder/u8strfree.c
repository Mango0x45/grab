#include <stdlib.h>

#include "builder.h"

void
u8strfree(struct u8buf b)
{
	free(b.p);
}
