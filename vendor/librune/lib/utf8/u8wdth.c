#include "utf8.h"

#include "internal/common.h"

int
u8wdth(rune ch)
{
	return ch <= _1B_MAX ? 1
	     : ch <= _2B_MAX ? 2
	     : ch <= _3B_MAX ? 3
	     : ch <= _4B_MAX ? 4
	                     : 0;
}
