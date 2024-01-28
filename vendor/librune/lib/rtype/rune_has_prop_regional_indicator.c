/* This file is autogenerated by gen/rtype-prop; DO NOT EDIT. */

#include "rtype.h"

#include "internal/common.h"

#if BIT_LOOKUP
static const unsigned _BitInt(LATIN1_MAX + 1) mask = 0x0uwb;
#endif

static const struct {
	rune lo, hi;
} lookup_tbl[] = {
	{0x01F1E6, 0x01F1FF},
};

#define TYPE      bool
#define TABLE     lookup_tbl
#define DEFAULT   false
#define HAS_VALUE 0
#include "internal/rtype/lookup-func.h"

bool
rune_has_prop_regional_indicator(rune ch)
{
	return
#if BIT_LOOKUP
		ch <= LATIN1_MAX ? (mask & ch) :
#endif
		lookup(ch);
}
