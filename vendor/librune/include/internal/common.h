#ifndef RUNE_INTERNAL_COMMON_H
#define RUNE_INTERNAL_COMMON_H

#if __STDC_VERSION__ >= 202311L
#	define RUNE_IS_23 1
#endif

#if !RUNE_IS_23
#	include <stddef.h> /* IWYU pragma: export */
#	define nullptr NULL
#endif

#ifndef unreachable
#	if RUNE_IS_23
#		include <stddef.h> /* IWYU pragma: export */
#	elif defined(__GNUC__) || defined(__clang__)
#		define unreachable() __builtin_unreachable()
#	else
#		define unreachable() \
			do \
				*(int *)0 = 0; \
			while (0)
#	endif
#endif

#define U1(x) (((x)&0x80) == 0x00)
#define U2(x) (((x)&0xE0) == 0xC0)
#define U3(x) (((x)&0xF0) == 0xE0)
#define U4(x) (((x)&0xF8) == 0xF0)
#define UC(x) (((x)&0xC0) == 0x80)

/* Maximum value of a 1–4-byte long UTF-8 sequence */
#include "rune.h"
#define _1B_MAX RUNE_C(0x00007F)
#define _2B_MAX RUNE_C(0x0007FF)
#define _3B_MAX RUNE_C(0x00FFFF)
#define _4B_MAX RUNE_C(0x10FFFF)

#endif /* !RUNE_INTERNAL_COMMON_H */
