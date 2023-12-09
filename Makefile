.POSIX:

WARNINGS = -Wall -Wextra -Werror -Wpedantic

CC = cc
CFLAGS = $(WARNINGS) -pipe -O3 -march=native -mtune=native

all: grab
grab: grab.c

debug:
	$(CC) $(WARNINGS) -DGRAB_DEBUG -g -ggdb3 -o grab grab.c

clean:
	rm -f grab
