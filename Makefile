PREFIX=/usr/local
LIBS = -lc 
CFLAGS = -std=c99

all: md2roff md2roff.1

md2roff: md2roff.c
	$(CC) $(CFLAGS) md2roff.c -o md2roff $(LDFLAGS) $(LIBS)

md2roff.1.gz: md2roff.md md2roff
	./md2roff md2roff.md > md2roff.1
	gzip -f md2roff.1

install: md2roff md2roff.1.gz
	install -m 0755 -s md2roff $(PREFIX)/bin
	install -m 0644 md2roff.1.gz $(PREFIX)/man/man1

clean:
	-@rm *.o md2roff md2roff.1 md2roff.1.gz
