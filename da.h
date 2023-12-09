/*
 * Simple & stupid dynamic array single-header implementation.  You can use the
 * macros defined in this file with any structure that has the following fields:
 *
 *  struct dyn_array {
 *      T *buf  // Array of items
 *      N  len  // Length of array
 *      N  cap  // Capacity of array
 *  }
 *
 * The type ‘T’ is whatever type you want to store.  The type ‘N’ is any numeric
 * type, most likely ‘size_t’.
 *
 * You should include ‘err.h’ and ‘stdlib.h’ along with this file.  If you want
 * to use da_remove(), include ‘string.h’.  The da_remove() macro also doesn’t
 * bother with shrinking your array when the length is far lower than the
 * capacity.  If you care about that, do it yourself.
 *
 *
 * Macro Overview
 * ――――――――――――――
 * The argument ‘a’ to all of the below macros is a pointer to the dynamic array
 * structure.
 *
 * da_init(a, n)             Initialize the array with a capacity of ‘n’ items.
 * da_append(a, x)           Append the item ‘x’ to the array
 * da_remove(a, x)           Remove the item ‘x’ from the array
 * da_remove_range(a, x, y)  Remove the items between the range [x, y)
 */

#ifndef MANGO_DA_H
#define MANGO_DA_H

#define __da_s(a) (sizeof(*(a)->buf))

#define da_init(a, n) \
	do { \
		(a)->cap = n; \
		(a)->len = 0; \
		(a)->buf = malloc((a)->cap * __da_s(a)); \
		if ((a)->buf == NULL) \
			err(EXIT_FAILURE, "malloc"); \
	} while (0)

#define da_append(a, x) \
	do { \
		if ((a)->len >= (a)->cap) { \
			(a)->cap = (a)->cap * 2 + 1; \
			(a)->buf = realloc((a)->buf, (a)->cap * __da_s(a)); \
			if ((a)->buf == NULL) \
				err(EXIT_FAILURE, "realloc"); \
		} \
		(a)->buf[(a)->len++] = (x); \
	} while (0)

#define da_remove(a, i) da_remove_range((a), (i), (i) + 1)

#define da_remove_range(a, i, j) \
	do { \
		memmove((a)->buf + (i), (a)->buf + (j), ((a)->len - (j)) * __da_s(a)); \
		(a)->len -= j - i; \
	} while (0)

#endif /* !MANGO_DA_H */
