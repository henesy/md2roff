# md2roff

A utility to convert markdown documents to Unix \*roff format (man, mdoc, mm and mom packages).

License: GPL v3+
Requires:
* Any C99 compiler

## Status

Alpha but it does the job.

80% : man format
60% : mdoc format
60% : mom format
--- : mm format

## Install

```shell
# git clone https://github.com/nereusx/md2roff.git
# cd md2roff
# make && make install
```

Note: Edit `Makefile` to set the destination directory.

## Usage

Example:
```
% md2roff myfile.md > myfile.man
```

For more, please read the [md2roff.md](https://github.com/nereusx/md2roff/blob/master/md2roff.md) file.

## COPYRIGHT
Copyright (C) 2017 Free Software Foundation, Inc.
License GPLv3+: GNU GPL version 3 or later (http://gnu.org/licenses/gpl.html).
This is free software: you are free to change and redistribute it.
There is NO WARRANTY, to the extent permitted by law.

## AUTHOR
Written by Nicholas Christopoulos (nereus@freemail.gr).




