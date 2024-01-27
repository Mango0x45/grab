#include "rtype.h"

bool
ristitle(rune ch)
{
	return runeis(ch, UC_LT);
}
