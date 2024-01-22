/* This compatibility header was generated by the libcompat gen script.
   You can find it over at https://git.sr.ht/~mango/libcompat. */

#ifndef LIBCOMPAT_COMPAT_H
#define LIBCOMPAT_COMPAT_H

#if __STDC_VERSION__ >= 202311L
#	define LIBCOMPAT_IS_23 1
#endif

#if !LIBCOMPAT_IS_23
#	include <stdbool.h> /* IWYU pragma: export */
#endif

#if !LIBCOMPAT_IS_23
#	ifndef NULL
#		include <stddef.h> /* IWYU pragma: export */
#	endif
#	define nullptr NULL
#endif

#if !LIBCOMPAT_IS_23
#	ifdef static_assert
#		undef static_assert
#	endif
#	define static_assert(e, ...) _Static_assert(e, ""__VA_ARGS__)
#endif

#endif /* !LIBCOMPAT_COMPAT_H */
