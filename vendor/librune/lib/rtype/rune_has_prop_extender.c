/* This file is autogenerated by gen/rtype-prop; DO NOT EDIT. */

#include "rtype.h"

#include "internal/common.h"

#if BIT_LOOKUP
static const unsigned _BitInt(LATIN1_MAX + 1) mask = 0x8000000000000000000000000000000000000000000000uwb;
#endif

static const struct {
	rune lo, hi;
} lookup_tbl[] = {
	{0x0000B7, 0x0000B7},
	{0x0002D0, 0x0002D1},
	{0x000640, 0x000640},
	{0x0007FA, 0x0007FA},
	{0x000B55, 0x000B55},
	{0x000E46, 0x000E46},
	{0x000EC6, 0x000EC6},
	{0x00180A, 0x00180A},
	{0x001843, 0x001843},
	{0x001AA7, 0x001AA7},
	{0x001C36, 0x001C36},
	{0x001C7B, 0x001C7B},
	{0x003005, 0x003005},
	{0x003031, 0x003035},
	{0x00309D, 0x00309E},
	{0x0030FC, 0x0030FE},
	{0x00A015, 0x00A015},
	{0x00A60C, 0x00A60C},
	{0x00A9CF, 0x00A9CF},
	{0x00A9E6, 0x00A9E6},
	{0x00AA70, 0x00AA70},
	{0x00AADD, 0x00AADD},
	{0x00AAF3, 0x00AAF4},
	{0x00FF70, 0x00FF70},
	{0x010781, 0x010782},
	{0x01135D, 0x01135D},
	{0x0115C6, 0x0115C8},
	{0x011A98, 0x011A98},
	{0x016B42, 0x016B43},
	{0x016FE0, 0x016FE1},
	{0x016FE3, 0x016FE3},
	{0x01E13C, 0x01E13D},
	{0x01E944, 0x01E946},
};

#define TYPE      bool
#define TABLE     lookup_tbl
#define DEFAULT   false
#define HAS_VALUE 0
#include "internal/rtype/lookup-func.h"

bool
rune_has_prop_extender(rune ch)
{
	return
#if BIT_LOOKUP
		ch <= LATIN1_MAX ? (mask & ch) :
#endif
		lookup(ch);
}
