#include "tmux_event_lib.h"
#include <stdio.h>

void handle_layout_change( unsigned int window, const char* layout, void* ctxt )
{
  printf( "Layout changed for window %u - %s\n", window, layout );
}

struct OnLayoutChange layout_change_handler = { { NULL }, handle_layout_change, NULL };

int main( ) {
  tmux_event_init( );
  register_layout_change_handler( &layout_change_handler );
  tmux_event_loop( stdin );
  return 0;
}
