CC     = gcc
CFLAGS = -Wall -Werror -ggdb3 -I../
EXES   = $(basename $(wildcard *.c))

all: $(EXES)
clean:
	rm -f *.o $(EXES)

%: %.c
	$(CC) $^ -o $@ $(CFLAGS) ../tmux_event_lib.o

