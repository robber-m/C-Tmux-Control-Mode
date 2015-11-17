CC     = gcc
CFLAGS = -Wall -Werror -g -I../
EXES   = $(basename $(wildcard *.c))

all: $(EXES)
clean:
	rm -f *.o $(EXES)

%: %.c
	$(CC) $^ -o $@ $(CFLAGS) ../tmux_event_lib.o

