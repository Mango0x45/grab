# librune — easy Unicode in a post-ASCII world

Librune is a C library that aims to make interacting with Unicode and
UTF-8 easy in C.  There are no plans at the moment to support UTF-16 or
-32, but they may be supported if such a usecase ever comes up.

This library has been tested to build and work properly on both C99 and
C23.


## Terminology

This library uses the term ‘rune’ to refer to a single Unicode-codepoint,
and defines a `rune` datatype which is an unsigned integer type which
represents a rune (shocker).


## Headers

This library contains the following headers:

1. `gbrk.h` — grapheme-iteration functions
2. `rune.h` — rune-constants, -macros, and -functions
3. `utf8.h` — UTF-8 encoding, decoding, iteration, etc.
4. `builder.h` — string building functions


## Installation

There is no ‘intended’ way in which this library should be installed,
used, and distributed.  This library is primarily written for myself, and
I prefer to vendor it in my projects.  You may choose to install it as a
shared and/or static library.  You’re an engineer aren’t you?  Figure it
out.
