#include "rtype.h"

bool
risletter(rune ch)
{
	return runeis(ch, UC_L);
}
