# md2roff 1 2017-05-09 GNU

## NAME
md2roff \- converts markdown documents to roff (man, mdoc, mm, mom).

## SYNOPSIS
md2roff [*OPTION*]... [*FILE*]...

## DESCRIPTION
*md2roff* converts the input files to groff (with man package) format
and prints the result to *stdout*. If the *FILE* is `-` then it reads
from *stdin*.

## OPTIONS

#### -h, --help
displays a short-help text and exits

#### -v, --version
displays the program version, copyright and license information and exists.

#### -n, --man
use man package (linux man pages, default), see [groff_man 7](man)

#### -m, --mm
use mm package, see [groff_mm 7](man)

#### -d, --mdoc
use mdoc package (BSD man pages), see [groff_mdoc 7](man)

#### -o, --mom
use mom package, see [groff_mom 7](man)

## NOTES
1. If the documents starts with `# ` then creates the TH command with this;
otherwise there will be a default TH with the file-name. Actually only the
title (name) and section are required.
```
# title section date source manual
```
2. If you write man page, use `####` for each option to automatically convert
the section to '.TP' as in GNU's manuals.
3. Use special link type (man) to link man pages.

## BUGS
A lot. Fix and send.

## EXAMPLE
View a markdown as console man page:
```
$ md2roff mytext.md | groff -Tutf8 -man | $PAGER
```

View a markdown as postscript man page:
```
$ md2roff mytext.md | groff -Tps -man | okular -
```

This document is example, its written in markdown and produce a nice man page.

## COPYRIGHT
Copyright (C) 2017 Free Software Foundation, Inc.
License GPLv3+: GNU GPL version 3 or later (http://gnu.org/licenses/gpl.html).
This is free software: you are free to change and redistribute it.
There is NO WARRANTY, to the extent permitted by law.

## AUTHOR
Written by Nicholas Christopoulos (nereus@freemail.gr).

## SEE ALSO
[groff_man 7](man), [man-pages 7](man),
[groff_man 7](man), [groff_mdoc 7](man), 
[groff_mm 7](man), [groff_mom 7](man).
