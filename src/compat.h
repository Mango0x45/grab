/* This compatibility header was generated by the libcompat gen script.
   You can find it over at https://git.sr.ht/~mango/libcompat. */

#ifndef LIBCOMPAT_COMPAT_H
#define LIBCOMPAT_COMPAT_H

#if __STDC_VERSION__ >= 202311L
#	define LIBCOMPAT_IS_23 1
#endif

#if !LIBCOMPAT_IS_23
#	include <stdbool.h>
#endif

#if !LIBCOMPAT_IS_23
#	ifndef NULL
#		include <stddef.h>
#	endif
#	define nullptr NULL
#endif

#if !LIBCOMPAT_IS_23
#	define static_assert(e, ...) _Static_assert(e, ""__VA_ARGS__)
#endif

#endif /* !LIBCOMPAT_COMPAT_H */