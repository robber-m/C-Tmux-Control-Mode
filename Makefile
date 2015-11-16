CFLAGS=-I.

print_layout_changes: tmux_event_lib.o
		$(CC) -o examples/$@ examples/$@.c $< $(CFLAGS)

%.o: %.c
		$(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm -f *.o examples/*.o examples/print_layout_changes
