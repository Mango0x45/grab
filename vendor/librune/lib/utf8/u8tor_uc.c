#include <stddef.h>

#include "utf8.h"

#include "internal/common.h"

int
u8tor_uc(rune *ch, const char8_t *s)
{
	if (U1(s[0])) {
		*ch = s[0];
		return 1;
	} else if (U2(s[0])) {
		*ch = ((s[0] & 0x1F) << 6) | (s[1] & 0x3F);
		return 2;
	} else if (U3(s[0])) {
		*ch = ((s[0] & 0x0F) << 12) | ((s[1] & 0x3F) << 6) | (s[2] & 0x3F);
		return 3;
	} else if (U4(s[0])) {
		*ch = ((s[0] & 0x07) << 18) | ((s[1] & 0x3F) << 12)
		    | ((s[2] & 0x3F) << 6) | (s[3] & 0x3F);
		return 4;
	}

	unreachable();
}
