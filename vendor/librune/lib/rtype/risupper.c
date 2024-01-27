#include "rtype.h"

bool
risupper(rune ch)
{
	return runeis(ch, UC_LU);
}
