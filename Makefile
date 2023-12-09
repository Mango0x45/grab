.POSIX:

WARNINGS = -Wall -Wextra -Werror -Wpedantic

CC = cc
CFLAGS = $(WARNINGS) -pipe -O3 -march=native -mtune=native

PREFIX = /usr/local
DPREFIX = $(DESTDIR)$(PREFIX)

all: grab
grab: grab.c

debug:
	$(CC) $(WARNINGS) -DGRAB_DEBUG -g -ggdb3 -o grab grab.c

install:
	mkdir -p $(DPREFIX)/bin $(DPREFIX)/share/man/man1 
	cp grab $(DPREFIX)/bin
	cp grab.1 $(DPREFIX)/share/man/man1

clean:
	rm -f grab
