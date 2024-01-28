#include "rtype.h"

#include "internal/common.h"

static constexpr bool latin1_space_tbl[LATIN1_MAX + 1] = {
	['\t'] = true, ['\n'] = true, ['\v'] = true, ['\f'] = true,
	['\r'] = true, [' '] = true,  [0x85] = true, [0xA0] = true,
};

bool
risspace(rune ch)
{
	if (ch <= LATIN1_MAX)
		return latin1_space_tbl[ch];
	return runeis(ch, UC_Z);
}
