CC     = gcc
CFLAGS = -Wall -Werror -g -I../
EXES   = $(shell find . -name '*.c' | sed 's/\.c//g')

all: $(EXES)
clean:
	rm -f *.o $(EXES)

%: %.c
	$(CC) $^ -o $@ $(CFLAGS) ../tmux_event_lib.o

