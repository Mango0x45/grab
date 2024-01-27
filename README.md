# Grab — A better grep

Grab is a more powerful version of the well-known Grep utility, making
use of structural regular expressions as described by Rob Pike in [this
paper][1].  Grab allows you to be far more precise with your searching
than Grep, as it doesn’t constrain itself to working only on individual
lines.


## Installation

To install grab, all you need is a C compiler:

```sh
$ cc -o make make.c  # Bootstrap the build script
$ ./make  # Build the project
$ ./make install  # Install the project
```

By default Grab is linked against the PCRE2 library for PCRE support.  If
you prefer to simply use POSIX EREs, you can pass `--no-pcre` to the
build script:

```sh
$ ./make --no-pcre
```

If you want to build with optimizations enabled, you can pass the
`--release` flag.  You can additionally pass the `--lto` flag to enable
link-time optimizations:

```sh
$ ./make --release --lto
```


## Description

Grab invocations must include a pattern string which specifies which text
to match.  A pattern string consists of one or more commands.  A command
is an operator followed by a delimiter, a regular expression (regex), and
then terminated by the same delimiter.  The last delimiter of the last
command is optional.

For example, a pattern string may look like ‘`x/[a-z]+/ g.foo. v/bar/`’.

The available operators are ‘g’, ‘G’, ‘h’, ‘H’, ‘x’, and ‘X’.  The ‘g’
and ‘G’ operators are filter operators, the ‘h’ and ‘H’ operators are
highlighting operators, and ‘x’ and ‘X’ are selection operators.

You probably want to begin your pattern with a selection operator.  By
default the entire contents of the file you’re searching through will be
selected, but you probably want to shrink that down to a specific query.
With ‘x’ you can specify what text you want to select in the file.  For
example ‘`x/[0-9]+/`’ will select all numbers:

```sh
echo 'foo12bar34baz' | grab 'x/[0-9]+/'
# ⇒ 12
# ⇒ 34
```

The ‘X’ operator works in reverse, selecting everything that _doesn’t_
match the given regex:

```sh
echo 'foo12bar34baz' | grab 'X/[0-9]+/'
# ⇒ foo
# ⇒ bar
# ⇒ baz
```

You can additionally use filter operators to keep or discard certain
results.  The ‘g’ operator will filter out any results that don’t match
the given regex, while the ‘G’ operator will do the opposite.  To select
all numbers that contain a ‘3’ we can thus do:

``` sh
echo 'foo12bar34baz' | grab 'x/[0-9]+/ g/3/'
# ⇒ 34

# If we had used ‘x’ instead of ‘g’, the result would have just been ‘3’.
# Filter operators do not modify the selections; they merely filter them.
```

Likewise to select all numbers that don’t contain a ‘3’:

```sh
echo 'foo12bar34baz' | grab 'x/[0-9]+/ G/3/'
# ⇒ 12
```

You can also chain these together.  To get all numbers in a file that
contain a ‘3’ but aren’t the specific number ‘1337’, we could do the
following:

```sh
grab 'x/[0-9]+/ g/3/ G/^1337$/' /foo/bar
```

The final set of operators are highlighting operators.  They don’t change
the text that is ultimately matched in any manner, but instead highlight
the matched text in the output.  If given the empty regular expression
the ‘h’ operator will highlight according to the same regular expression
as the previous operator.

The following examples select words with a capital letter, and highlights
all the capital letters:

```sh
# These are both the same
grab 'x/\w+/ g/[A-Z]/ h/[A-Z]/' /foo/bar
grab 'x/\w+/ g/[A-Z]/ h//'      /foo/bar
```


## Examples

### Get a list of your CPU flags.

```sh
# With Grep
grep '^flags' /proc/cpuinfo \
| sed 's/flags:\t*: //; y/ /\n/' \
| sort \
| uniq

# With Grab
grab 'x/^flags.*/ x/\w+/ G/flags/' /proc/cpuinfo \
| sort \
| uniq
```

1) Select lines that start with ‘flags’: `x/^flags.*/`
2) Select all the words: `x/\w+/`
3) Filter out the word ‘flags’: `v/flags/`


### Find `<my-tag>` tags with the attribute `data-attr` in a Git repo

```sh
git grab 'x/<my-tag.*?>/ g/data-attr/' '*.html'
```

1) Select all tags matching `<my-tag>`
2) Filter out tags without `data-attr`


## Additional Options

The Grab utility has a few options that may be helpful for your usecase.
For more detailed documentation, see the Grab manual with `man grab`.


[1]: https://doc.cat-v.org/bell_labs/structural_regexps/se.pdf
