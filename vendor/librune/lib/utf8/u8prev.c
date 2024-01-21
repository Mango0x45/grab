#include "rune.h"
#include "utf8.h"

#include "internal/common.h"

const char8_t *
u8prev(rune *ch, const char8_t **p, const char8_t *start)
{
	int off;
	bool match = true;
	const char8_t *s = *p;
	ptrdiff_t d = s - start;

	if (d <= 0) {
		return nullptr;
	} else if (U1(s[-1])) {
		*ch = s[-1];
		off = 1;
	} else if (d > 1 && UC(s[-1]) && U2(s[-2])) {
		*ch = ((s[-2] & 0x1F) << 6) | (s[-1] & 0x3F);
		off = 2;
	} else if (d > 2 && UC(s[-1]) && UC(s[-2]) && U3(s[-3])) {
		*ch = ((s[-3] & 0x0F) << 12) | ((s[-2] & 0x3F) << 6) | (s[-1] & 0x3F);
		off = 3;
	} else if (d > 3 && UC(s[-1]) && UC(s[-2]) && UC(s[-3]) && U4(s[-4])) {
		*ch = ((s[-4] & 0x07) << 18) | ((s[-3] & 0x3F) << 12)
		    | ((s[-2] & 0x3F) << 6) | (s[-1] & 0x3F);
		off = 4;
	} else
		match = false;

	if (match && u8chkr(*ch))
		return *p -= off;

	*ch = RUNE_ERROR;
	return *p--;
}
