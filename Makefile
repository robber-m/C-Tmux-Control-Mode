CFLAGS=-Wall -Werror -g

.PHONY: examples                                                                                                                                                                                                                                                                      
examples: tmux_event_lib.o
	$(MAKE) -C $@

clean:
	$(MAKE) -C examples clean
	rm -f tmux_event_lib.o

%.o: %.c
		$(CC) -c -o $@ $^ $(CFLAGS)
