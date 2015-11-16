#ifndef INCLUDED_TMUX_EVENT_LIB_H
#define INCLUDED_TMUX_EVENT_LIB_H

#include "compat/queue.h"
#include <stdio.h>

struct OnPaneOutput
{
  LIST_ENTRY( OnPaneOutput ) entries;
  void  (*handle)( unsigned int pane_id, const char* output, void* ctxt );
  void* ctxt;
};

struct OnWindowAdd
{
  LIST_ENTRY( OnWindowAdd ) entries;
  void  (*handle)( unsigned int window_id, void* ctxt );
  void* ctxt;
};

struct OnWindowClose
{
  LIST_ENTRY( OnWindowClose ) entries;
  void  (*handle)( unsigned int window_id, void* ctxt );
  void* ctxt;
};

struct OnWindowRenamed
{
  LIST_ENTRY( OnWindowRenamed ) entries;
  void  (*handle)( unsigned int window_id, const char* name, void* ctxt );
  void* ctxt;
};

struct OnLayoutChange
{
  LIST_ENTRY( OnLayoutChange ) entries;
  void  (*handle)( unsigned int window_id, const char* layout, void* ctxt );
  void* ctxt;
};

struct OnSessionChanged
{
  LIST_ENTRY( OnSessionChanged ) entries;
  void  (*handle)( unsigned int session_id, const char* name, void* ctxt );
  void* ctxt;
};

struct OnSessionRenamed
{
  LIST_ENTRY( OnSessionRenamed ) entries;
  void  (*handle)( unsigned int session_id, const char* name, void* ctxt );
  void* ctxt;
};

void tmux_event_init( );

void register_pane_output_handler( struct OnPaneOutput* handler );
void unregister_pane_output_handler( struct OnPaneOutput* handler );

void register_window_add_handler( struct OnWindowAdd* handler );
void unregister_window_add_handler( struct OnWindowAdd* handler );

void register_window_close_handler( struct OnWindowClose* handler );
void unregister_window_close_handler( struct OnWindowClose* handler );

void register_layout_change_handler( struct OnLayoutChange* handler );
void unregister_layout_change_handler( struct OnLayoutChange* handler );

void register_session_changed_handler( struct OnSessionChanged* handler );
void unregister_session_changed_handler( struct OnSessionChanged* handler );

void register_session_renamed_handler( struct OnSessionRenamed* handler );
void unregister_session_renamed_handler( struct OnSessionRenamed* handler );

void tmux_event_loop( FILE* tmux_control_file_stream );

#endif // INCLUDED_TMUX_EVENT_LIB_H
