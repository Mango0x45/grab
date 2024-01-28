/* This file is autogenerated by gen/rtype-prop; DO NOT EDIT. */

#include "rtype.h"

#include "internal/common.h"

#if BIT_LOOKUP
static const unsigned _BitInt(LATIN1_MAX + 1) mask = 0xFF7FFFFF80000000002000000000000007FFFFFE000000000000000000000000uwb;
#endif

static const struct {
	rune lo, hi;
} lookup_tbl[] = {
	{0x000061, 0x00007A},
	{0x0000B5, 0x0000B5},
	{0x0000DF, 0x0000F6},
	{0x0000F8, 0x0000FF},
	{0x000101, 0x000101},
	{0x000103, 0x000103},
	{0x000105, 0x000105},
	{0x000107, 0x000107},
	{0x000109, 0x000109},
	{0x00010B, 0x00010B},
	{0x00010D, 0x00010D},
	{0x00010F, 0x00010F},
	{0x000111, 0x000111},
	{0x000113, 0x000113},
	{0x000115, 0x000115},
	{0x000117, 0x000117},
	{0x000119, 0x000119},
	{0x00011B, 0x00011B},
	{0x00011D, 0x00011D},
	{0x00011F, 0x00011F},
	{0x000121, 0x000121},
	{0x000123, 0x000123},
	{0x000125, 0x000125},
	{0x000127, 0x000127},
	{0x000129, 0x000129},
	{0x00012B, 0x00012B},
	{0x00012D, 0x00012D},
	{0x00012F, 0x00012F},
	{0x000131, 0x000131},
	{0x000133, 0x000133},
	{0x000135, 0x000135},
	{0x000137, 0x000137},
	{0x00013A, 0x00013A},
	{0x00013C, 0x00013C},
	{0x00013E, 0x00013E},
	{0x000140, 0x000140},
	{0x000142, 0x000142},
	{0x000144, 0x000144},
	{0x000146, 0x000146},
	{0x000148, 0x000149},
	{0x00014B, 0x00014B},
	{0x00014D, 0x00014D},
	{0x00014F, 0x00014F},
	{0x000151, 0x000151},
	{0x000153, 0x000153},
	{0x000155, 0x000155},
	{0x000157, 0x000157},
	{0x000159, 0x000159},
	{0x00015B, 0x00015B},
	{0x00015D, 0x00015D},
	{0x00015F, 0x00015F},
	{0x000161, 0x000161},
	{0x000163, 0x000163},
	{0x000165, 0x000165},
	{0x000167, 0x000167},
	{0x000169, 0x000169},
	{0x00016B, 0x00016B},
	{0x00016D, 0x00016D},
	{0x00016F, 0x00016F},
	{0x000171, 0x000171},
	{0x000173, 0x000173},
	{0x000175, 0x000175},
	{0x000177, 0x000177},
	{0x00017A, 0x00017A},
	{0x00017C, 0x00017C},
	{0x00017E, 0x000180},
	{0x000183, 0x000183},
	{0x000185, 0x000185},
	{0x000188, 0x000188},
	{0x00018C, 0x00018C},
	{0x000192, 0x000192},
	{0x000195, 0x000195},
	{0x000199, 0x00019A},
	{0x00019E, 0x00019E},
	{0x0001A1, 0x0001A1},
	{0x0001A3, 0x0001A3},
	{0x0001A5, 0x0001A5},
	{0x0001A8, 0x0001A8},
	{0x0001AD, 0x0001AD},
	{0x0001B0, 0x0001B0},
	{0x0001B4, 0x0001B4},
	{0x0001B6, 0x0001B6},
	{0x0001B9, 0x0001B9},
	{0x0001BD, 0x0001BD},
	{0x0001BF, 0x0001BF},
	{0x0001C4, 0x0001C4},
	{0x0001C6, 0x0001C7},
	{0x0001C9, 0x0001CA},
	{0x0001CC, 0x0001CC},
	{0x0001CE, 0x0001CE},
	{0x0001D0, 0x0001D0},
	{0x0001D2, 0x0001D2},
	{0x0001D4, 0x0001D4},
	{0x0001D6, 0x0001D6},
	{0x0001D8, 0x0001D8},
	{0x0001DA, 0x0001DA},
	{0x0001DC, 0x0001DD},
	{0x0001DF, 0x0001DF},
	{0x0001E1, 0x0001E1},
	{0x0001E3, 0x0001E3},
	{0x0001E5, 0x0001E5},
	{0x0001E7, 0x0001E7},
	{0x0001E9, 0x0001E9},
	{0x0001EB, 0x0001EB},
	{0x0001ED, 0x0001ED},
	{0x0001EF, 0x0001F1},
	{0x0001F3, 0x0001F3},
	{0x0001F5, 0x0001F5},
	{0x0001F9, 0x0001F9},
	{0x0001FB, 0x0001FB},
	{0x0001FD, 0x0001FD},
	{0x0001FF, 0x0001FF},
	{0x000201, 0x000201},
	{0x000203, 0x000203},
	{0x000205, 0x000205},
	{0x000207, 0x000207},
	{0x000209, 0x000209},
	{0x00020B, 0x00020B},
	{0x00020D, 0x00020D},
	{0x00020F, 0x00020F},
	{0x000211, 0x000211},
	{0x000213, 0x000213},
	{0x000215, 0x000215},
	{0x000217, 0x000217},
	{0x000219, 0x000219},
	{0x00021B, 0x00021B},
	{0x00021D, 0x00021D},
	{0x00021F, 0x00021F},
	{0x000223, 0x000223},
	{0x000225, 0x000225},
	{0x000227, 0x000227},
	{0x000229, 0x000229},
	{0x00022B, 0x00022B},
	{0x00022D, 0x00022D},
	{0x00022F, 0x00022F},
	{0x000231, 0x000231},
	{0x000233, 0x000233},
	{0x00023C, 0x00023C},
	{0x00023F, 0x000240},
	{0x000242, 0x000242},
	{0x000247, 0x000247},
	{0x000249, 0x000249},
	{0x00024B, 0x00024B},
	{0x00024D, 0x00024D},
	{0x00024F, 0x000254},
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
	{0x000371, 0x000371},
	{0x000373, 0x000373},
	{0x000377, 0x000377},
	{0x00037B, 0x00037D},
	{0x000390, 0x000390},
	{0x0003AC, 0x0003CE},
	{0x0003D0, 0x0003D1},
	{0x0003D5, 0x0003D7},
	{0x0003D9, 0x0003D9},
	{0x0003DB, 0x0003DB},
	{0x0003DD, 0x0003DD},
	{0x0003DF, 0x0003DF},
	{0x0003E1, 0x0003E1},
	{0x0003E3, 0x0003E3},
	{0x0003E5, 0x0003E5},
	{0x0003E7, 0x0003E7},
	{0x0003E9, 0x0003E9},
	{0x0003EB, 0x0003EB},
	{0x0003ED, 0x0003ED},
	{0x0003EF, 0x0003F3},
	{0x0003F5, 0x0003F5},
	{0x0003F8, 0x0003F8},
	{0x0003FB, 0x0003FB},
	{0x000430, 0x00045F},
	{0x000461, 0x000461},
	{0x000463, 0x000463},
	{0x000465, 0x000465},
	{0x000467, 0x000467},
	{0x000469, 0x000469},
	{0x00046B, 0x00046B},
	{0x00046D, 0x00046D},
	{0x00046F, 0x00046F},
	{0x000471, 0x000471},
	{0x000473, 0x000473},
	{0x000475, 0x000475},
	{0x000477, 0x000477},
	{0x000479, 0x000479},
	{0x00047B, 0x00047B},
	{0x00047D, 0x00047D},
	{0x00047F, 0x00047F},
	{0x000481, 0x000481},
	{0x00048B, 0x00048B},
	{0x00048D, 0x00048D},
	{0x00048F, 0x00048F},
	{0x000491, 0x000491},
	{0x000493, 0x000493},
	{0x000495, 0x000495},
	{0x000497, 0x000497},
	{0x000499, 0x000499},
	{0x00049B, 0x00049B},
	{0x00049D, 0x00049D},
	{0x00049F, 0x00049F},
	{0x0004A1, 0x0004A1},
	{0x0004A3, 0x0004A3},
	{0x0004A5, 0x0004A5},
	{0x0004A7, 0x0004A7},
	{0x0004A9, 0x0004A9},
	{0x0004AB, 0x0004AB},
	{0x0004AD, 0x0004AD},
	{0x0004AF, 0x0004AF},
	{0x0004B1, 0x0004B1},
	{0x0004B3, 0x0004B3},
	{0x0004B5, 0x0004B5},
	{0x0004B7, 0x0004B7},
	{0x0004B9, 0x0004B9},
	{0x0004BB, 0x0004BB},
	{0x0004BD, 0x0004BD},
	{0x0004BF, 0x0004BF},
	{0x0004C2, 0x0004C2},
	{0x0004C4, 0x0004C4},
	{0x0004C6, 0x0004C6},
	{0x0004C8, 0x0004C8},
	{0x0004CA, 0x0004CA},
	{0x0004CC, 0x0004CC},
	{0x0004CE, 0x0004CF},
	{0x0004D1, 0x0004D1},
	{0x0004D3, 0x0004D3},
	{0x0004D5, 0x0004D5},
	{0x0004D7, 0x0004D7},
	{0x0004D9, 0x0004D9},
	{0x0004DB, 0x0004DB},
	{0x0004DD, 0x0004DD},
	{0x0004DF, 0x0004DF},
	{0x0004E1, 0x0004E1},
	{0x0004E3, 0x0004E3},
	{0x0004E5, 0x0004E5},
	{0x0004E7, 0x0004E7},
	{0x0004E9, 0x0004E9},
	{0x0004EB, 0x0004EB},
	{0x0004ED, 0x0004ED},
	{0x0004EF, 0x0004EF},
	{0x0004F1, 0x0004F1},
	{0x0004F3, 0x0004F3},
	{0x0004F5, 0x0004F5},
	{0x0004F7, 0x0004F7},
	{0x0004F9, 0x0004F9},
	{0x0004FB, 0x0004FB},
	{0x0004FD, 0x0004FD},
	{0x0004FF, 0x0004FF},
	{0x000501, 0x000501},
	{0x000503, 0x000503},
	{0x000505, 0x000505},
	{0x000507, 0x000507},
	{0x000509, 0x000509},
	{0x00050B, 0x00050B},
	{0x00050D, 0x00050D},
	{0x00050F, 0x00050F},
	{0x000511, 0x000511},
	{0x000513, 0x000513},
	{0x000515, 0x000515},
	{0x000517, 0x000517},
	{0x000519, 0x000519},
	{0x00051B, 0x00051B},
	{0x00051D, 0x00051D},
	{0x00051F, 0x00051F},
	{0x000521, 0x000521},
	{0x000523, 0x000523},
	{0x000525, 0x000525},
	{0x000527, 0x000527},
	{0x000529, 0x000529},
	{0x00052B, 0x00052B},
	{0x00052D, 0x00052D},
	{0x00052F, 0x00052F},
	{0x000561, 0x000587},
	{0x0013F8, 0x0013FD},
	{0x001C80, 0x001C88},
	{0x001D79, 0x001D79},
	{0x001D7D, 0x001D7D},
	{0x001D8E, 0x001D8E},
	{0x001E01, 0x001E01},
	{0x001E03, 0x001E03},
	{0x001E05, 0x001E05},
	{0x001E07, 0x001E07},
	{0x001E09, 0x001E09},
	{0x001E0B, 0x001E0B},
	{0x001E0D, 0x001E0D},
	{0x001E0F, 0x001E0F},
	{0x001E11, 0x001E11},
	{0x001E13, 0x001E13},
	{0x001E15, 0x001E15},
	{0x001E17, 0x001E17},
	{0x001E19, 0x001E19},
	{0x001E1B, 0x001E1B},
	{0x001E1D, 0x001E1D},
	{0x001E1F, 0x001E1F},
	{0x001E21, 0x001E21},
	{0x001E23, 0x001E23},
	{0x001E25, 0x001E25},
	{0x001E27, 0x001E27},
	{0x001E29, 0x001E29},
	{0x001E2B, 0x001E2B},
	{0x001E2D, 0x001E2D},
	{0x001E2F, 0x001E2F},
	{0x001E31, 0x001E31},
	{0x001E33, 0x001E33},
	{0x001E35, 0x001E35},
	{0x001E37, 0x001E37},
	{0x001E39, 0x001E39},
	{0x001E3B, 0x001E3B},
	{0x001E3D, 0x001E3D},
	{0x001E3F, 0x001E3F},
	{0x001E41, 0x001E41},
	{0x001E43, 0x001E43},
	{0x001E45, 0x001E45},
	{0x001E47, 0x001E47},
	{0x001E49, 0x001E49},
	{0x001E4B, 0x001E4B},
	{0x001E4D, 0x001E4D},
	{0x001E4F, 0x001E4F},
	{0x001E51, 0x001E51},
	{0x001E53, 0x001E53},
	{0x001E55, 0x001E55},
	{0x001E57, 0x001E57},
	{0x001E59, 0x001E59},
	{0x001E5B, 0x001E5B},
	{0x001E5D, 0x001E5D},
	{0x001E5F, 0x001E5F},
	{0x001E61, 0x001E61},
	{0x001E63, 0x001E63},
	{0x001E65, 0x001E65},
	{0x001E67, 0x001E67},
	{0x001E69, 0x001E69},
	{0x001E6B, 0x001E6B},
	{0x001E6D, 0x001E6D},
	{0x001E6F, 0x001E6F},
	{0x001E71, 0x001E71},
	{0x001E73, 0x001E73},
	{0x001E75, 0x001E75},
	{0x001E77, 0x001E77},
	{0x001E79, 0x001E79},
	{0x001E7B, 0x001E7B},
	{0x001E7D, 0x001E7D},
	{0x001E7F, 0x001E7F},
	{0x001E81, 0x001E81},
	{0x001E83, 0x001E83},
	{0x001E85, 0x001E85},
	{0x001E87, 0x001E87},
	{0x001E89, 0x001E89},
	{0x001E8B, 0x001E8B},
	{0x001E8D, 0x001E8D},
	{0x001E8F, 0x001E8F},
	{0x001E91, 0x001E91},
	{0x001E93, 0x001E93},
	{0x001E95, 0x001E9B},
	{0x001EA1, 0x001EA1},
	{0x001EA3, 0x001EA3},
	{0x001EA5, 0x001EA5},
	{0x001EA7, 0x001EA7},
	{0x001EA9, 0x001EA9},
	{0x001EAB, 0x001EAB},
	{0x001EAD, 0x001EAD},
	{0x001EAF, 0x001EAF},
	{0x001EB1, 0x001EB1},
	{0x001EB3, 0x001EB3},
	{0x001EB5, 0x001EB5},
	{0x001EB7, 0x001EB7},
	{0x001EB9, 0x001EB9},
	{0x001EBB, 0x001EBB},
	{0x001EBD, 0x001EBD},
	{0x001EBF, 0x001EBF},
	{0x001EC1, 0x001EC1},
	{0x001EC3, 0x001EC3},
	{0x001EC5, 0x001EC5},
	{0x001EC7, 0x001EC7},
	{0x001EC9, 0x001EC9},
	{0x001ECB, 0x001ECB},
	{0x001ECD, 0x001ECD},
	{0x001ECF, 0x001ECF},
	{0x001ED1, 0x001ED1},
	{0x001ED3, 0x001ED3},
	{0x001ED5, 0x001ED5},
	{0x001ED7, 0x001ED7},
	{0x001ED9, 0x001ED9},
	{0x001EDB, 0x001EDB},
	{0x001EDD, 0x001EDD},
	{0x001EDF, 0x001EDF},
	{0x001EE1, 0x001EE1},
	{0x001EE3, 0x001EE3},
	{0x001EE5, 0x001EE5},
	{0x001EE7, 0x001EE7},
	{0x001EE9, 0x001EE9},
	{0x001EEB, 0x001EEB},
	{0x001EED, 0x001EED},
	{0x001EEF, 0x001EEF},
	{0x001EF1, 0x001EF1},
	{0x001EF3, 0x001EF3},
	{0x001EF5, 0x001EF5},
	{0x001EF7, 0x001EF7},
	{0x001EF9, 0x001EF9},
	{0x001EFB, 0x001EFB},
	{0x001EFD, 0x001EFD},
	{0x001EFF, 0x001F07},
	{0x001F10, 0x001F15},
	{0x001F20, 0x001F27},
	{0x001F30, 0x001F37},
	{0x001F40, 0x001F45},
	{0x001F50, 0x001F57},
	{0x001F60, 0x001F67},
	{0x001F70, 0x001F7D},
	{0x001F80, 0x001F87},
	{0x001F90, 0x001F97},
	{0x001FA0, 0x001FA7},
	{0x001FB0, 0x001FB4},
	{0x001FB6, 0x001FB7},
	{0x001FBE, 0x001FBE},
	{0x001FC2, 0x001FC4},
	{0x001FC6, 0x001FC7},
	{0x001FD0, 0x001FD3},
	{0x001FD6, 0x001FD7},
	{0x001FE0, 0x001FE7},
	{0x001FF2, 0x001FF4},
	{0x001FF6, 0x001FF7},
	{0x00214E, 0x00214E},
	{0x002170, 0x00217F},
	{0x002184, 0x002184},
	{0x0024D0, 0x0024E9},
	{0x002C30, 0x002C5F},
	{0x002C61, 0x002C61},
	{0x002C65, 0x002C66},
	{0x002C68, 0x002C68},
	{0x002C6A, 0x002C6A},
	{0x002C6C, 0x002C6C},
	{0x002C73, 0x002C73},
	{0x002C76, 0x002C76},
	{0x002C81, 0x002C81},
	{0x002C83, 0x002C83},
	{0x002C85, 0x002C85},
	{0x002C87, 0x002C87},
	{0x002C89, 0x002C89},
	{0x002C8B, 0x002C8B},
	{0x002C8D, 0x002C8D},
	{0x002C8F, 0x002C8F},
	{0x002C91, 0x002C91},
	{0x002C93, 0x002C93},
	{0x002C95, 0x002C95},
	{0x002C97, 0x002C97},
	{0x002C99, 0x002C99},
	{0x002C9B, 0x002C9B},
	{0x002C9D, 0x002C9D},
	{0x002C9F, 0x002C9F},
	{0x002CA1, 0x002CA1},
	{0x002CA3, 0x002CA3},
	{0x002CA5, 0x002CA5},
	{0x002CA7, 0x002CA7},
	{0x002CA9, 0x002CA9},
	{0x002CAB, 0x002CAB},
	{0x002CAD, 0x002CAD},
	{0x002CAF, 0x002CAF},
	{0x002CB1, 0x002CB1},
	{0x002CB3, 0x002CB3},
	{0x002CB5, 0x002CB5},
	{0x002CB7, 0x002CB7},
	{0x002CB9, 0x002CB9},
	{0x002CBB, 0x002CBB},
	{0x002CBD, 0x002CBD},
	{0x002CBF, 0x002CBF},
	{0x002CC1, 0x002CC1},
	{0x002CC3, 0x002CC3},
	{0x002CC5, 0x002CC5},
	{0x002CC7, 0x002CC7},
	{0x002CC9, 0x002CC9},
	{0x002CCB, 0x002CCB},
	{0x002CCD, 0x002CCD},
	{0x002CCF, 0x002CCF},
	{0x002CD1, 0x002CD1},
	{0x002CD3, 0x002CD3},
	{0x002CD5, 0x002CD5},
	{0x002CD7, 0x002CD7},
	{0x002CD9, 0x002CD9},
	{0x002CDB, 0x002CDB},
	{0x002CDD, 0x002CDD},
	{0x002CDF, 0x002CDF},
	{0x002CE1, 0x002CE1},
	{0x002CE3, 0x002CE3},
	{0x002CEC, 0x002CEC},
	{0x002CEE, 0x002CEE},
	{0x002CF3, 0x002CF3},
	{0x002D00, 0x002D25},
	{0x002D27, 0x002D27},
	{0x002D2D, 0x002D2D},
	{0x00A641, 0x00A641},
	{0x00A643, 0x00A643},
	{0x00A645, 0x00A645},
	{0x00A647, 0x00A647},
	{0x00A649, 0x00A649},
	{0x00A64B, 0x00A64B},
	{0x00A64D, 0x00A64D},
	{0x00A64F, 0x00A64F},
	{0x00A651, 0x00A651},
	{0x00A653, 0x00A653},
	{0x00A655, 0x00A655},
	{0x00A657, 0x00A657},
	{0x00A659, 0x00A659},
	{0x00A65B, 0x00A65B},
	{0x00A65D, 0x00A65D},
	{0x00A65F, 0x00A65F},
	{0x00A661, 0x00A661},
	{0x00A663, 0x00A663},
	{0x00A665, 0x00A665},
	{0x00A667, 0x00A667},
	{0x00A669, 0x00A669},
	{0x00A66B, 0x00A66B},
	{0x00A66D, 0x00A66D},
	{0x00A681, 0x00A681},
	{0x00A683, 0x00A683},
	{0x00A685, 0x00A685},
	{0x00A687, 0x00A687},
	{0x00A689, 0x00A689},
	{0x00A68B, 0x00A68B},
	{0x00A68D, 0x00A68D},
	{0x00A68F, 0x00A68F},
	{0x00A691, 0x00A691},
	{0x00A693, 0x00A693},
	{0x00A695, 0x00A695},
	{0x00A697, 0x00A697},
	{0x00A699, 0x00A699},
	{0x00A69B, 0x00A69B},
	{0x00A723, 0x00A723},
	{0x00A725, 0x00A725},
	{0x00A727, 0x00A727},
	{0x00A729, 0x00A729},
	{0x00A72B, 0x00A72B},
	{0x00A72D, 0x00A72D},
	{0x00A72F, 0x00A72F},
	{0x00A733, 0x00A733},
	{0x00A735, 0x00A735},
	{0x00A737, 0x00A737},
	{0x00A739, 0x00A739},
	{0x00A73B, 0x00A73B},
	{0x00A73D, 0x00A73D},
	{0x00A73F, 0x00A73F},
	{0x00A741, 0x00A741},
	{0x00A743, 0x00A743},
	{0x00A745, 0x00A745},
	{0x00A747, 0x00A747},
	{0x00A749, 0x00A749},
	{0x00A74B, 0x00A74B},
	{0x00A74D, 0x00A74D},
	{0x00A74F, 0x00A74F},
	{0x00A751, 0x00A751},
	{0x00A753, 0x00A753},
	{0x00A755, 0x00A755},
	{0x00A757, 0x00A757},
	{0x00A759, 0x00A759},
	{0x00A75B, 0x00A75B},
	{0x00A75D, 0x00A75D},
	{0x00A75F, 0x00A75F},
	{0x00A761, 0x00A761},
	{0x00A763, 0x00A763},
	{0x00A765, 0x00A765},
	{0x00A767, 0x00A767},
	{0x00A769, 0x00A769},
	{0x00A76B, 0x00A76B},
	{0x00A76D, 0x00A76D},
	{0x00A76F, 0x00A76F},
	{0x00A77A, 0x00A77A},
	{0x00A77C, 0x00A77C},
	{0x00A77F, 0x00A77F},
	{0x00A781, 0x00A781},
	{0x00A783, 0x00A783},
	{0x00A785, 0x00A785},
	{0x00A787, 0x00A787},
	{0x00A78C, 0x00A78C},
	{0x00A791, 0x00A791},
	{0x00A793, 0x00A794},
	{0x00A797, 0x00A797},
	{0x00A799, 0x00A799},
	{0x00A79B, 0x00A79B},
	{0x00A79D, 0x00A79D},
	{0x00A79F, 0x00A79F},
	{0x00A7A1, 0x00A7A1},
	{0x00A7A3, 0x00A7A3},
	{0x00A7A5, 0x00A7A5},
	{0x00A7A7, 0x00A7A7},
	{0x00A7A9, 0x00A7A9},
	{0x00A7B5, 0x00A7B5},
	{0x00A7B7, 0x00A7B7},
	{0x00A7B9, 0x00A7B9},
	{0x00A7BB, 0x00A7BB},
	{0x00A7BD, 0x00A7BD},
	{0x00A7BF, 0x00A7BF},
	{0x00A7C1, 0x00A7C1},
	{0x00A7C3, 0x00A7C3},
	{0x00A7C8, 0x00A7C8},
	{0x00A7CA, 0x00A7CA},
	{0x00A7D1, 0x00A7D1},
	{0x00A7D7, 0x00A7D7},
	{0x00A7D9, 0x00A7D9},
	{0x00A7F6, 0x00A7F6},
	{0x00AB53, 0x00AB53},
	{0x00AB70, 0x00ABBF},
	{0x00FB00, 0x00FB06},
	{0x00FB13, 0x00FB17},
	{0x00FF41, 0x00FF5A},
	{0x010428, 0x01044F},
	{0x0104D8, 0x0104FB},
	{0x010597, 0x0105A1},
	{0x0105A3, 0x0105B1},
	{0x0105B3, 0x0105B9},
	{0x0105BB, 0x0105BC},
	{0x010CC0, 0x010CF2},
	{0x0118C0, 0x0118DF},
	{0x016E60, 0x016E7F},
	{0x01E922, 0x01E943},
};

#define TYPE      bool
#define TABLE     lookup_tbl
#define DEFAULT   false
#define HAS_VALUE 0
#include "internal/rtype/lookup-func.h"

bool
rune_has_prop_changes_when_titlecased(rune ch)
{
	return
#if BIT_LOOKUP
		ch <= LATIN1_MAX ? (mask & ch) :
#endif
		lookup(ch);
}
