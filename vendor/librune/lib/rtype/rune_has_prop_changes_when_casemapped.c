/* This file is autogenerated by gen/rtype-prop; DO NOT EDIT. */

#include "rtype.h"

#include "internal/common.h"

#if BIT_LOOKUP
static const unsigned _BitInt(LATIN1_MAX + 1) mask = 0xFF7FFFFFFF7FFFFF002000000000000007FFFFFE07FFFFFE0000000000000000uwb;
#endif

static const struct {
	rune lo, hi;
} lookup_tbl[] = {
	{0x000041, 0x00005A},
	{0x000061, 0x00007A},
	{0x0000B5, 0x0000B5},
	{0x0000C0, 0x0000D6},
	{0x0000D8, 0x0000F6},
	{0x0000F8, 0x000137},
	{0x000139, 0x00018C},
	{0x00018E, 0x00019A},
	{0x00019C, 0x0001A9},
	{0x0001AC, 0x0001B9},
	{0x0001BC, 0x0001BD},
	{0x0001BF, 0x0001BF},
	{0x0001C4, 0x000220},
	{0x000222, 0x000233},
	{0x00023A, 0x000254},
	{0x000256, 0x000257},
	{0x000259, 0x000259},
	{0x00025B, 0x00025C},
	{0x000260, 0x000261},
	{0x000263, 0x000263},
	{0x000265, 0x000266},
	{0x000268, 0x00026C},
	{0x00026F, 0x00026F},
	{0x000271, 0x000272},
	{0x000275, 0x000275},
	{0x00027D, 0x00027D},
	{0x000280, 0x000280},
	{0x000282, 0x000283},
	{0x000287, 0x00028C},
	{0x000292, 0x000292},
	{0x00029D, 0x00029E},
	{0x000345, 0x000345},
	{0x000370, 0x000373},
	{0x000376, 0x000377},
	{0x00037B, 0x00037D},
	{0x00037F, 0x00037F},
	{0x000386, 0x000386},
	{0x000388, 0x00038A},
	{0x00038C, 0x00038C},
	{0x00038E, 0x0003A1},
	{0x0003A3, 0x0003D1},
	{0x0003D5, 0x0003F5},
	{0x0003F7, 0x0003FB},
	{0x0003FD, 0x000481},
	{0x00048A, 0x00052F},
	{0x000531, 0x000556},
	{0x000561, 0x000587},
	{0x0010A0, 0x0010C5},
	{0x0010C7, 0x0010C7},
	{0x0010CD, 0x0010CD},
	{0x0010D0, 0x0010FA},
	{0x0010FD, 0x0010FF},
	{0x0013A0, 0x0013F5},
	{0x0013F8, 0x0013FD},
	{0x001C80, 0x001C88},
	{0x001C90, 0x001CBA},
	{0x001CBD, 0x001CBF},
	{0x001D79, 0x001D79},
	{0x001D7D, 0x001D7D},
	{0x001D8E, 0x001D8E},
	{0x001E00, 0x001E9B},
	{0x001E9E, 0x001E9E},
	{0x001EA0, 0x001F15},
	{0x001F18, 0x001F1D},
	{0x001F20, 0x001F45},
	{0x001F48, 0x001F4D},
	{0x001F50, 0x001F57},
	{0x001F59, 0x001F59},
	{0x001F5B, 0x001F5B},
	{0x001F5D, 0x001F5D},
	{0x001F5F, 0x001F7D},
	{0x001F80, 0x001FB4},
	{0x001FB6, 0x001FBC},
	{0x001FBE, 0x001FBE},
	{0x001FC2, 0x001FC4},
	{0x001FC6, 0x001FCC},
	{0x001FD0, 0x001FD3},
	{0x001FD6, 0x001FDB},
	{0x001FE0, 0x001FEC},
	{0x001FF2, 0x001FF4},
	{0x001FF6, 0x001FFC},
	{0x002126, 0x002126},
	{0x00212A, 0x00212B},
	{0x002132, 0x002132},
	{0x00214E, 0x00214E},
	{0x002160, 0x00217F},
	{0x002183, 0x002184},
	{0x0024B6, 0x0024E9},
	{0x002C00, 0x002C70},
	{0x002C72, 0x002C73},
	{0x002C75, 0x002C76},
	{0x002C7E, 0x002CE3},
	{0x002CEB, 0x002CEE},
	{0x002CF2, 0x002CF3},
	{0x002D00, 0x002D25},
	{0x002D27, 0x002D27},
	{0x002D2D, 0x002D2D},
	{0x00A640, 0x00A66D},
	{0x00A680, 0x00A69B},
	{0x00A722, 0x00A72F},
	{0x00A732, 0x00A76F},
	{0x00A779, 0x00A787},
	{0x00A78B, 0x00A78D},
	{0x00A790, 0x00A794},
	{0x00A796, 0x00A7AE},
	{0x00A7B0, 0x00A7CA},
	{0x00A7D0, 0x00A7D1},
	{0x00A7D6, 0x00A7D9},
	{0x00A7F5, 0x00A7F6},
	{0x00AB53, 0x00AB53},
	{0x00AB70, 0x00ABBF},
	{0x00FB00, 0x00FB06},
	{0x00FB13, 0x00FB17},
	{0x00FF21, 0x00FF3A},
	{0x00FF41, 0x00FF5A},
	{0x010400, 0x01044F},
	{0x0104B0, 0x0104D3},
	{0x0104D8, 0x0104FB},
	{0x010570, 0x01057A},
	{0x01057C, 0x01058A},
	{0x01058C, 0x010592},
	{0x010594, 0x010595},
	{0x010597, 0x0105A1},
	{0x0105A3, 0x0105B1},
	{0x0105B3, 0x0105B9},
	{0x0105BB, 0x0105BC},
	{0x010C80, 0x010CB2},
	{0x010CC0, 0x010CF2},
	{0x0118A0, 0x0118DF},
	{0x016E40, 0x016E7F},
	{0x01E900, 0x01E943},
};

#define TYPE      bool
#define TABLE     lookup_tbl
#define DEFAULT   false
#define HAS_VALUE 0
#include "internal/rtype/lookup-func.h"

bool
rune_has_prop_changes_when_casemapped(rune ch)
{
	return
#if BIT_LOOKUP
		ch <= LATIN1_MAX ? (mask & ch) :
#endif
		lookup(ch);
}
