#include "rune.h"
#include "utf8.h"

#include "internal/common.h"

int
u8tor(rune *ch, const char8_t *s)
{
	int n = 0;

	if (U1(s[0])) {
		*ch = s[0];
		n = 1;
	} else if (U2(s[0]) && UC(s[1])) {
		*ch = ((s[0] & 0x1F) << 6) | (s[1] & 0x3F);
		n = 2;
	} else if (U3(s[0]) && UC(s[1]) && UC(s[2])) {
		*ch = ((s[0] & 0x0F) << 12) | ((s[1] & 0x3F) << 6) | (s[2] & 0x3F);
		n = 3;
	} else if (U4(s[0]) && UC(s[1]) && UC(s[2]) && UC(s[3])) {
		*ch = ((s[0] & 0x07) << 18) | ((s[1] & 0x3F) << 12)
		    | ((s[2] & 0x3F) << 6) | (s[3] & 0x3F);
		n = 4;
	}

	if (n && u8chkr(*ch))
		return n;

	*ch = RUNE_ERROR;
	return 1;
}
