# librune — easy Unicode in a post-ASCII world

Librune is a C library that aims to make interacting with Unicode and
UTF-8 easy in C.  There are no plans at the moment to support UTF-16 or
-32, but they may be supported if such a usecase ever comes up.

This library requires C23.


## Terminology

This library uses the term ‘rune’ to refer to a single Unicode-codepoint,
and defines a `rune` datatype which is an unsigned integer type which
represents a rune (shocker).


## Headers

This library contains the following headers:

- `builder.h` — string building functions
- `gbrk.h` — grapheme-iteration functions
- `rtype.h` — rune categorization à la `ctype.h`
- `rune.h` — rune-constants, -macros, and -functions
- `utf8.h` — UTF-8 encoding, decoding, iteration, etc.


## Compilation

This library comes with a build script in the form of `make.c`.  To build
the library all you need is a C compiler.  The build script will build a
static library called ‘librune.a’.

```sh
# Make sure to link with pthread
cc -lpthread -o make make.c
./make
```

If you want to build the library in release-mode (optimizations enabled),
simply pass the `-r` flag to the build script:

```sh
./make -r
```

You can also pass the `-l` flag to enable link-time optimizations:

```sh
./make -lr
```


## Installation

There is no ‘intended’ way in which this library should be installed,
used, and distributed.  This library is primarily written for myself, and
I prefer to vendor it in my projects.  You may choose to install it as a
shared and/or static library.  You’re an engineer aren’t you?  Figure it
out.
