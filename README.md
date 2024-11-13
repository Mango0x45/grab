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

If you want to build with optimizations enabled, you can pass the `-r`
flag.

```sh
$ ./make -r
```


## Description

```
GRAB(1)                  General Commands Manual                 GRAB(1)

NAME
       grab, git grab — search for patterns in files

SYNOPSIS
       grab [-H never | multi | always] [-bcilLpsUz] pattern [file ...]
       grab -h

       git  grab  [-H never | multi | always] [-bcilLpsUz] pattern [glob
            ...]
       git grab -h

DESCRIPTION
       The grab utility searches for text matching the given pattern  in
       the files listed on the command-line, printing the matches to the
       standard-output.    Unlike  the  grep(1)  utility,  grab  is  not
       strictly line-oriented; the structure of matches is  left  up  to
       the  user to define.  For more details on the pattern syntax, see
       “Pattern Syntax”.

       The git grab utility is identical to the grab utility except that
       it takes globs matching files as command-line  arguments  instead
       of  files,  and processes all non-binary files in the current git
       repository that match the provided globs.  If no globs  are  pro‐
       vided,  all  non-binary  files  in the current git repository are
       processed.

       grab will read from the files provided on the  command-line.   If
       no  files  are provided, the standard-input will be read instead.
       The special filename ‘-’ can also be provided,  which  represents
       the standard-input.

       Similar  to  the grep(1) utility matches are printed to the stan‐
       dard output.  They are additionally prefixed with the name of the
       file in which pattern was matched, as well as the location of the
       match.

       The options are as follows:

       -b, --byte-offset
               Report the positions of pattern  matches  as  the  (zero-
               based) byte offset of the match from the beginning of the
               file.

               This option is useful if your text editor (such as vim(1)
               or  emacs(1))  supports  jumping directly to a given byte
               offset/position.

               This is the default behaviour if the  -L  option  is  not
               provided.

       -c, --color
               Force  colored output, even if the output device is not a
               TTY.  This is useful when piping the output of grab  into
               a pager such as less(1).

               This  option  takes precedence over the environment vari‐
               ables described in “ENVIRONMENT” that relate to the usage
               of color.

       -h, --help
               Display help information by opening this manual page.

       -H, --header-line=when
               Control the usage of a dedicated header line,  where  the
               filename  and  match  position are printed on a dedicated
               line above the match.  The  available  options  for  when
               are:

               never   never use a dedicated header line
               always  always use a dedicated header line
               multi   use a dedicated header line when the matched pat‐
                       tern spans multiple lines

       -i, --ignore-case
               Match patterns case-insensitively.

       -l, --literal
               Treat  patterns  as literal strings, i.e. don’t interpret
               them as regular expressions.

       -L, --line-position
               Report the positions of matches as  a  (one-based)  line-
               and column position separated by a colon.

               This  option  may  be  ill-advised in many circumstances.
               See “BUGS” for more details.

       -p, --predicate
               Return an exit status indicating if  a  match  was  found
               without  writing any output to the standard output.  When
               simply checking for the presence of a pattern in  an  in‐
               put,  this  option is far more efficient than redirecting
               output to /dev/null.

       -s, --strip-newline
               Don’t print a newline at the end of a match if the  match
               already  ends  in  a  newline.  This can make output seem
               more ‘natural’, as many matches will already have  termi‐
               nating newlines.

       -U, --no-unicode
               Don’t  use  Unicode properties when matching \d, \w, etc.
               Recognize only ASCII values instead.

       -z, --zero
               Separate output data by null bytes (‘\0’) instead of new‐
               lines.  This option can be used to process  matches  con‐
               taining newlines.

   Pattern Syntax
       A pattern is a sequence of whitespace-separated commands.  A com‐
       mand  is a sequence of an operator, an opening delimiter, a regu‐
       lar expression, a closing delimter, and zero-or-more flags.   The
       last command of a pattern if given no flags need not have a clos‐
       ing delimter.

       The supported operators are as follows:

       g       Keep matches that match the given regex.
       G       Keep matches that don’t match the given regex.
       h       Highlight  substrings  in  matches  that  match the given
               regex.
       H       Highlight substrings in  matches  that  don’t  match  the
               given regex.
       x       Select everything that matches the given regex.
       X       Select everything that doesn’t match the given regex.

       An  example  pattern  to match all numbers that contain a ‘3’ but
       aren’t ‘1337’ could be ‘x/[0-9]+/ g/3/ G/^1337$/’.  In that  pat‐
       tern,  ‘x/[0-9]+/’ selects all numbers in the input, ‘g/3/’ keeps
       only those matches that contain the  number  3,  and  ‘G/^1337$/’
       filters out the specific number 1337.

       The  opening-  and  closing-delimiter used for each given command
       can be any valid UTF-8 codepoint.  As  a  result,  the  following
       pattern using the delimiters ‘|’, ‘.’, and ‘ä’ is well-formed:

             x|[0-9]+| g.3. Gä^1337$ä

       Delimeters   also   respect  the  Unicode  ‘Bidirectional  Paired
       Bracket’ property.  This means that alongside the previous  exam‐
       ples, the following non-exhaustive list of character pairs may be
       used as opening- and closing delimiters:

       •   ｢…｣
       •   ⟮…⟯
       •   ⟨…⟩

       It is not recommended that you use characters that have a special
       meaning in regular expression syntax as delimiters, unless you’re
       using literal patterns via the -l option or the ‘l’ command flag.

       Operators  are not allowed to take empty regular expression argu‐
       ments with one exception: ‘h’.  When given an empty  regular  ex‐
       pression  argument, the ‘h’ operator assumes the same regular ex‐
       pression as the previous operator.  This allows you to avoid  du‐
       plication  in  the  common  case where a user wishes to highlight
       text matched by a ‘g’ or ‘x’  operator.   The  following  example
       pattern  selects  all words that have a capital letter, and high‐
       lights the capital letter(s):

             x/\w+/ g/\p{Lu}/ h//

       The empty ‘h’ operator is not permitted as the first operator  in
       a pattern.

       While  various  command-line options exist to alter the behaviour
       of patterns such as -i to enable case-insensitive matching or  -U
       to disable Unicode support, various different options can also be
       set  at the command-level by appending a command with one-or-more
       flags.  As an example, one could match all sequences  of  one-or-
       more  non-whitespace characters that contain the case-insensitive
       literal string ‘[hi]’ by using the following pattern:

             x/\S+/ g/[hi]/li

       The currently supported flags are as follows:

       i/I     enable or disable case-insensitive matching respectively
       l/L     enable or disable treating the supplied regex as a  fixed
               string
       u/U     enable or disable Unicode support respectively

ENVIRONMENT
       Do not display any colored output when set to a non-empty string,
       even  if  the  standard-output  is  a terminal.  This environment
       variable takes precedence over CLICOLOR_FORCE.
       Force display of colored output when set to a  non-empty  string,
       even if the standard-output isn’t a terminal.
       If  set to ‘dumb’ disables colored output, taking precedence over
       all other environment variables.

EXIT STATUS
       The grab utility exits with one of the following values:

             0       One or more matches were selected.
             1       No matches were selected.
             2       A non-fatal error occured, such as failure to  read
                     a file.
             >2      A fatal error occured.

EXAMPLES
       List all your systems CPU flags, sorted and without duplicates:

             $  grab  'x/^flags.*?$/ x/\w+/ G/^flags$/' </proc/cpuinfo |
             sort -u

       Search for a pattern in multiple files without printing filenames
       or position information:

             $ cat file1 file2 file3 | grab 'x/pattern/'

       Search for usages of an ‘<hb-form-text>’ Vue component — but only
       those which are being passed a ‘placeholder’ property — searching
       all files in the current git-repository:

             $  git   grab   'x/<hb-form-text.*?>/   g/\bplaceholder\b/'
             '*.vue'

       Extract  bibliographic  references  from mdoc(7) formatted manual
       pages:

             $ grab 'x/(^\.%.*?\n)+/' foo.1 bar.1

SEE ALSO
       git(1), grep(1), pcre2syntax(3), regex(7)

       Rob       Pike,       Structural       Regular       Expressions,
       https://doc.cat-v.org/bell_labs/structural_regexps/se.pdf,   AT&T
       Bell Laboratories, Murray Hill, New Jersey 07974, 1987.

       SGR                                                   Parameters:
       https://en.wikipedia.org/wiki/ANSI_escape_code#SGR

AUTHORS
       Thomas Voss <mail@thomasvoss.com>

NOTES
       When pattern matching with literal strings you should avoid using
       delimeters  that  are  contained  within the search string as any
       backslashes used to escape the delimeters will be searched for in
       the text literally.

BUGS
       The pattern string provided as a command-line argument as well as
       the provided input files must be encoded as UTF-8.  No other  en‐
       codings  are  supported unless they are UTF-8 compatible, such as
       ASCII.

       The -L option has incredibly poor performance compared to the  -b
       option, especially with very large inputs.

Grab 3.0.0                  13 November, 2024                    GRAB(1)
```


[1]: https://doc.cat-v.org/bell_labs/structural_regexps/se.pdf
