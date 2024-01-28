#include "rtype.h"

#include "internal/rtype/cat.h"

#define DEFAULT      UC_CN
#define HAS_VALUE    1
#define LATIN1_TABLE rtype_cat_lat1_tbl
#define TABLE        rtype_cat_tbl
#define TYPE         enum unicat
#include "internal/rtype/lookup-func.h"

bool
runeis(rune ch, enum unicat c)
{
	return lookup(ch) & c;
}
