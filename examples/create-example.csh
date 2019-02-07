#!/bin/csh -f

set src = Markdown-1.0.1.md
set bas = m
set fms = ( man mdoc mm mom )

foreach f ( $fms )
	../md2roff --$f  $src > ${bas}.$f
	groff ${bas}.$f > ${bas}.${f}.ps
end

