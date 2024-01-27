#include "rtype.h"

bool
risdigit(rune ch)
{
	return runeis(ch, UC_ND);
}
