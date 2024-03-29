/* This file is autogenerated by gen/rtype-prop; DO NOT EDIT. */

#include "rtype.h"

#include "internal/common.h"

#if BIT_LOOKUP
static const unsigned _BitInt(LATIN1_MAX + 1) mask = 0x8000000080000088435AFE000000007800000178000001FC00FFFE00000000uwb;
#endif

static const struct {
	rune lo, hi;
} lookup_tbl[] = {
	{0x000021, 0x00002F},
	{0x00003A, 0x000040},
	{0x00005B, 0x00005E},
	{0x000060, 0x000060},
	{0x00007B, 0x00007E},
	{0x0000A1, 0x0000A7},
	{0x0000A9, 0x0000A9},
	{0x0000AB, 0x0000AC},
	{0x0000AE, 0x0000AE},
	{0x0000B0, 0x0000B1},
	{0x0000B6, 0x0000B6},
	{0x0000BB, 0x0000BB},
	{0x0000BF, 0x0000BF},
	{0x0000D7, 0x0000D7},
	{0x0000F7, 0x0000F7},
	{0x002010, 0x002027},
	{0x002030, 0x00203E},
	{0x002041, 0x002053},
	{0x002055, 0x00205E},
	{0x002190, 0x00245F},
	{0x002500, 0x002775},
	{0x002794, 0x002BFF},
	{0x002E00, 0x002E7F},
	{0x003001, 0x003003},
	{0x003008, 0x003020},
	{0x003030, 0x003030},
	{0x00FD3E, 0x00FD3F},
	{0x00FE45, 0x00FE46},
};

#define TYPE      bool
#define TABLE     lookup_tbl
#define DEFAULT   false
#define HAS_VALUE 0
#include "internal/rtype/lookup-func.h"

bool
rune_has_prop_pattern_syntax(rune ch)
{
	return
#if BIT_LOOKUP
		ch <= LATIN1_MAX ? (mask & ch) :
#endif
		lookup(ch);
}
