# Grab — A better grep

Grab is a more powerful version of the well-known Grep utility, making
use of structural regular expressions as described by Rob Pike in [this
paper][1].  Grab allows you to be far more precise with your searching
than Grep, as it doesn’t constrain itself to working only on individual
lines.

Grab invokations must include a pattern string which specifies which text
to match.  A pattern string consists of one or more commands.  A command
is an operator followed by a delimiter, a regular expression (regex), and
then terminated by the same delimiter.  The last delimiter of the last
command is optional.

For example, a pattern string may look like ‘`x/[a-z]+/ g.foo. v/bar/`’.

The available operators are ‘g’, ‘v’, and ‘x’.  The ‘x' operator iterates
over all matches of the corresponding regex.  This means that to print
all numbers in a file, you can use the pattern string ‘`x/[0-9]+/`’.  The
‘g’ and ‘v’ operators are filters.  The ‘g’ operator discards all results
that don’t match the given regex, while the ‘v’ operator discards all
results that *do* match the given regex.  This means that to select all
numbers in a file that contain a ‘3’ but are not ‘1337’, you can use the
pattern string ‘`x/[0-9]+/ g/3/ v/^1337$/`’.


## Examples

Get a list of your CPU flags.

```sh
# With Grep
grep '^flags' /proc/cpuinfo \
| sed 's/flags:\t*: //; y/ /\n/' \
| sort \
| uniq

# With Grab
grab 'x/^flags.*/ x/\w+/ v/flags/' /proc/cpuinfo \
| sort \
| uniq
```

1) Select lines that start with ‘flags’: `x/^flags.*/`
2) Select all the words: `x/\w+/`
3) Filter out the word ‘flags’: `v/flags/`


## Additional Options

The Grab utility has a few options that may be helpful for your usecase.
For more detailed documentation, see the Grab manual with `man grab`.


[1]: https://doc.cat-v.org/bell_labs/structural_regexps/se.pdf
