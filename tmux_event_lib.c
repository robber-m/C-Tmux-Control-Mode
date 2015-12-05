#include "tmux_event_lib.h"
#include <stdlib.h>
/* I just want to scope out a basic event system which speaks the tmux control
 * mode text protocol. Maybe I will eventually use this abstraction for i3mux? */

#if 0
CONTROL MODE
     tmux offers a textual interface called control mode.  This allows
     applications to communicate with tmux using a simple text-only proto‐
     col.

     In control mode, a client sends tmux commands or command sequences
     terminated by newlines on standard input.  Each command will produce
     one block of output on standard output.  An output block consists of
     a %begin line followed by the output (which may be empty).  The out‐
     put block ends with a %end or %error.  %begin and matching %end or
     %error have two arguments: an integer time (as seconds from epoch)
     and command number.  For example:

           %begin 1363006971 2
           0: ksh* (1 panes) [80x24] [layout b25f,80x24,0,0,2] @2 (active)
           %end 1363006971 2

     In control mode, tmux outputs notifications.  A notification will
     never occur inside an output block.

     The following notifications are defined:

     %exit [reason]
             The tmux client is exiting immediately, either because it is
             not attached to any session or an error occurred.  If
             present, reason describes why the client exited.

     %layout-change window-id window-layout
             The layout of a window with ID window-id changed.  The new
             layout is window-layout.

     %output pane-id value
             A window pane produced output.  value escapes non-printable
             characters and backslash as octal \xxx.

     %session-changed session-id name
             The client is now attached to the session with ID session-id,
             which is named name.

     %session-renamed name
             The current session was renamed to name.

     %sessions-changed
             A session was created or destroyed.

     %unlinked-window-add window-id
             The window with ID window-id was linked to the current ses‐
             sion.

     %window-close window-id
             The window with ID window-id closed.

     %window-renamed window-id name
             The window with ID window-id was renamed to name.
#endif

/* ---- DEFINITIONS ---- */

/* stream read definitions */
char         s[ BUFSIZ ]; // stream read buffer

/* pane output event definitions */
unsigned int pane_id;
char*        output;
LIST_HEAD( PaneOutputEventHandlerList, OnPaneOutput ) pane_output_handlers;

/* window add event definitions */
unsigned int window_id;
LIST_HEAD( WindowAddEventHandlerList, OnWindowAdd ) window_add_handlers;

/* window close event definitions */
// unsigned int window_id;  -- defined above
LIST_HEAD( WindowCloseEventHandlerList, OnWindowClose ) window_close_handlers;

/* window renamed event definitions */
// unsigned int window_id;  -- defined above
char*        name;
LIST_HEAD( WindowRenamedEventHandlerList, OnWindowRenamed ) window_renamed_handlers;

/* layout change event definitions */
// unsigned int window_id;  -- defined above
char*        layout;
LIST_HEAD( LayoutChangeEventHandlerList, OnLayoutChange ) layout_change_handlers;

/* session changed event definitions */
unsigned int session_id;
// char*        name;       -- defined above
LIST_HEAD( SessionChangedEventHandlerList, OnSessionChanged ) session_changed_handlers;

/* session changed event definitions */
// unsigned int session_id; -- defined above
// char*        name;       -- defined above
LIST_HEAD( SessionRenamedEventHandlerList, OnSessionRenamed ) session_renamed_handlers;

/* command response event definitions */
unsigned int timestamp;
unsigned int command_number;
SIMPLEQ_HEAD( CommandResponseEventHandlerList, OnCommandResponse ) command_response_handlers;

/* ---- END DEFINITIONS ---- */

void tmux_event_init( )
{
  LIST_INIT( &pane_output_handlers );
  LIST_INIT( &window_add_handlers );
  LIST_INIT( &window_close_handlers );
  LIST_INIT( &window_renamed_handlers );
  LIST_INIT( &layout_change_handlers );
  LIST_INIT( &session_changed_handlers );
  LIST_INIT( &session_renamed_handlers );
  SIMPLEQ_INIT( &command_response_handlers );
}

/* ---- EVENT REGISTRATION FUNCTIONS ---- */

void register_pane_output_handler( struct OnPaneOutput* handler )
{
  LIST_INSERT_HEAD( &pane_output_handlers, handler, entries );
}

void unregister_pane_output_handler( struct OnPaneOutput* handler )
{
  LIST_REMOVE( handler, entries );
}

void register_window_add_handler( struct OnWindowAdd* handler )
{
  LIST_INSERT_HEAD( &window_add_handlers, handler, entries );
}

void unregister_window_add_handler( struct OnWindowAdd* handler )
{
  LIST_REMOVE( handler, entries );
}

void register_window_close_handler( struct OnWindowClose* handler )
{
  LIST_INSERT_HEAD( &window_close_handlers, handler, entries );
}

void unregister_window_close_handler( struct OnWindowClose* handler )
{
  LIST_REMOVE( handler, entries );
}

void register_window_renamed_handler( struct OnWindowRenamed* handler )
{
  LIST_INSERT_HEAD( &window_renamed_handlers, handler, entries );
}

void unregister_window_renamed_handler( struct OnWindowRenamed* handler )
{
  LIST_REMOVE( handler, entries );
}

void register_layout_change_handler( struct OnLayoutChange* handler )
{
  LIST_INSERT_HEAD( &layout_change_handlers, handler, entries );
}

void unregister_layout_change_handler( struct OnLayoutChange* handler )
{
  LIST_REMOVE( handler, entries );
}

void register_session_changed_handler( struct OnSessionChanged* handler )
{
  LIST_INSERT_HEAD( &session_changed_handlers, handler, entries );
}

void unregister_session_changed_handler( struct OnSessionChanged* handler )
{
  LIST_REMOVE( handler, entries );
}

void register_session_renamed_handler( struct OnSessionRenamed* handler )
{
  LIST_INSERT_HEAD( &session_renamed_handlers, handler, entries );
}

void unregister_session_renamed_handler( struct OnSessionRenamed* handler )
{
  LIST_REMOVE( handler, entries );
}

/* ---- END EVENT REGISTRATION FUNCTIONS ---- */

void send_tmux_command( FILE* tmux_control_input_stream,
                        const char* command,
                        struct OnCommandResponse* handler ) {
  /* Insert the command response into the list */
  /* NOTE: Until I decide what to do, I am assuming handler is not NULL */
  /* TODO: Decide what I want to do about commands that don't expect a response */
  /* TODO: Should I allocate a temporary OnCommandResponse object with a NULL
   *       handler? How do I know if I can free the object after dequeuing it? */
  SIMPLEQ_INSERT_TAIL( &command_response_handlers, handler, entries );
  fputs( command, tmux_control_input_stream );
  fflush( tmux_control_input_stream );
}

#define HANDLE_EVENTS( head, ... ) do {                                       \
  __typeof__( (head)->lh_first ) handler;                                     \
  LIST_FOREACH( handler, head, entries )                                      \
  {                                                                           \
    handler->handle( __VA_ARGS__, handler->ctxt );                            \
  }                                                                           \
} while(0)

/* NOTE: For now, the handler will need to handle malformed input */
void handle_command_response( FILE* tmux_control_output_stream )
{
  while( fgets( s, sizeof( s ), tmux_control_output_stream ) != NULL )
  {
    if( sscanf( s, "%%end %u %u %*u", &timestamp, &command_number ) == 2 )
    {
      /* return to normal event processing loop */
      return;
    }
    else if( sscanf( s, "%%error %u %u %*u", &timestamp, &command_number ) == 2 )
    {
      /* TODO: What do I do with errors? */
      /* return to normal event processing loop */
      return;
    }
    else if( !SIMPLEQ_EMPTY( &command_response_handlers ) ) {
      /* pass the command response to the handler at the front of the queue */
      /* NOTE: Passing responses line by line for now */
      struct OnCommandResponse* handler = SIMPLEQ_FIRST( &command_response_handlers );
      handler->handle( s, handler->ctxt );
    }
    else {
      /* TODO: What do I do with errors? */
    }
  }
}

/* NOTE: Would it be better to do this if/else stuff with a hash on the first
 * word in the line and a switch statement? */
void tmux_event_loop( FILE* tmux_control_output_stream )
{
  /* TODO: Do I need to register a few fake command response handlers because
   *       tmux sends me a few random begin and ends? I don't like that...
   *       Maybe I should issue a command of my own with a specific expected
   *       response body that will let me identify a time when it is okay to
   *       start using the response handler queue. */
  /* TODO: Register a command with a known expected output. The handler will
   *       determine whether or not the output for the command is the output
   *       we are expecting from our sync command. If it is the output we
   *       expect, set a flag that indicates it is okay to start removing
   *       command response handlers from the queue because we have finally
   *       synchronized our output stream with our queued response handlers */
  while( fgets( s, sizeof( s ), tmux_control_output_stream ) != NULL )
  {
    if( sscanf( s, "%%output %%%u%*c%m[^\n]", &pane_id, &output ) == 2 )
    {
      /* tmux pane output event */
      HANDLE_EVENTS( &pane_output_handlers, pane_id, output );
      free( output );
    }
    else if( sscanf( s, "%%window-add @%u", &window_id ) == 1 )
    {
      /* tmux window add event */
      HANDLE_EVENTS( &window_add_handlers, window_id );
    }
    else if( sscanf( s, "%%window-close @%u", &window_id ) == 1 )
    {
      /* tmux window close event */
      HANDLE_EVENTS( &window_close_handlers, window_id );
    }
    else if( sscanf( s, "%%window-renamed @%u %m[^\n]", &window_id, &name ) == 2 )
    {
      /* tmux window renamed event */
      HANDLE_EVENTS( &window_renamed_handlers, window_id, name );
      free( name );
    }
    else if( sscanf( s, "%%layout-change @%u %m[^\n]", &window_id, &layout ) == 2 )
    {
      /* tmux layout change event */
      HANDLE_EVENTS( &layout_change_handlers, window_id, layout );
      free( layout );
    }
    else if( sscanf( s, "%%session-changed $%u %m[^\n]", &session_id, &name ) == 2 )
    {
      /* tmux session changed event */
      HANDLE_EVENTS( &session_changed_handlers, session_id, name );
      free( name );
    }
    else if( sscanf( s, "%%session-renamed $%u %m[^\n]", &session_id, &name ) == 2 )
    {
      /* tmux session renamed event */
      HANDLE_EVENTS( &session_renamed_handlers, session_id, name );
      free( name );
    }
    else if( sscanf( s, "%%begin %u %u %*u", &timestamp, &command_number ) == 2 )
    /* TODO: Should I do this check with strncmp instead? */
    {
      /* tmux begin command response event */
      /* NOTE: All commands should also enqueue a response handler prior to
       *       sending the command. The appropriate response handler to use would
       *       be the first one we enqueued (FIFO). Also note that we may need to
       *       perform some synchronization around enqueuing the response handler
       *       and sending the command to tmux if we eventually allow multihreaded
       *       command submission. */
      
      /* TODO: If we receive an error message, need to return the error somehow */
      /* pass the command response to the response handler line by line */
      handle_command_response( tmux_control_output_stream );
      if( !SIMPLEQ_EMPTY( &command_response_handlers ) ) {
        /* we have handled the command response so we need to dequeue the handler */
        SIMPLEQ_REMOVE_HEAD( &command_response_handlers, entries );
      }
    }
    else {
      /* unhandled event */
      /* TODO: unlinked-window-add */
      /* TODO: sessions-changed */
      /* TODO: exit */
      /* TODO: Handle responses to user specified commands.
       *       My current thought is to add a queue for sending/receiving commands.
       *       If a receive handler is provided with the command to send, the
       *       receive handler will be added to the receive handling queue.
       *       Each receive handler will be executed once then removed from the queue. */
    }
  }
}
