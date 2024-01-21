#include "rune.h"
#include "utf8.h"

bool
u8chkr(rune ch)
{
	return !((ch >= 0xD800 && ch <= 0xDFFF) || ch == 0xFFFE || ch == 0xFFFF
	         || ch > RUNE_MAX);
}
