#include <stddef.h>

#include "internal/common.h"

#ifndef TYPE
#	error "TYPE if not defined"
#endif
#ifndef DEFAULT
#	error "DEFAULT is not defined"
#endif
#ifndef TABLE
#	error "TABLE is not defined"
#endif
#ifndef HAS_VALUE
#	error "HAS_VALUE is not defined"
#endif

[[gnu::always_inline]] static TYPE
lookup(rune ch)
{
	ptrdiff_t lo, hi;

#ifdef LATIN1_TABLE
	if (ch <= LATIN1_MAX)
		return LATIN1_TABLE[ch];
#endif

	lo = 0;
	hi = lengthof(TABLE) - 1;

	while (lo <= hi) {
		ptrdiff_t i = (lo + hi) / 2;

		if (ch < TABLE[i].lo)
			hi = i - 1;
		else if (ch > TABLE[i].hi)
			lo = i + 1;
		else
#if HAS_VALUE
			return TABLE[i].val;
#else
			return true;
#endif
	}

	return DEFAULT;
}
