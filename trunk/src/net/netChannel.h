/****
* NAME
*   netChannel - network channel class
*
* DESCRIPTION
*   netChannel is adds event-handling to the low-level
*   netSocket class.  Otherwise, it can be treated as
*   a normal non-blocking socket object.
*
*   The direct interface between the netPoll() loop and
*   the channel object are the handleReadEvent and
*   handleWriteEvent methods. These are called
*   whenever a channel object 'fires' that event.
*
*   The firing of these low-level events can tell us whether
*   certain higher-level events have taken place, depending on
*   the timing and state of the connection.
*
* AUTHORS
*   Sam Rushing <rushing@nightmare.com> - original version for Medusa
*   Dave McClurg <dpm@efn.org> - modified for use in Pegasus
*
* CREATION DATE
*   Dec-2000
*
****/

#ifndef NET_CHANNEL_H
#define NET_CHANNEL_H

#include "netSocket.h"

class netChannel : public netSocket
{
  bool closed, connected, accepting, write_blocked, should_delete ;
  netChannel* next_channel ;
  
  friend bool netPoll (u32 timeout);

public:

  netChannel () ;
  virtual ~netChannel () ;

  void setHandle (int s, bool is_connected = true);
  bool isConnected () const { return connected; }
  bool isClosed () const { return closed; }
  void shouldDelete () { should_delete = true ; }

  // --------------------------------------------------
  // socket methods
  // --------------------------------------------------
  
  bool  open    ( void ) ;
  void  close   ( void ) ;
  int   listen  ( int backlog ) ;
  int   connect ( cchar* host, int port ) ;
  int   send    ( const void * buf, int size, int flags = 0 ) ;
  int   recv    ( void * buf, int size, int flags = 0 ) ;

  // poll() eligibility predicates
  virtual bool readable (void) { return (connected || accepting); }
  virtual bool writable (void) { return (!connected || write_blocked); }
  
  // --------------------------------------------------
  // event handlers
  // --------------------------------------------------
  
  void handleReadEvent (void);
  void handleWriteEvent (void);
  
  // These are meant to be overridden.
  virtual void handleClose (void) {
    //fprintf(stderr,"Network: %d: unhandled close\n",getHandle());
  }
  virtual void handleRead (void) {
    fprintf(stderr,"Network: %d: unhandled read\n",getHandle());
  }
  virtual void handleWrite (void) {
    fprintf(stderr,"Network: %d: unhandled write\n",getHandle());
  }
  virtual void handleAccept (void) {
    fprintf(stderr,"Network: %d: unhandled accept\n",getHandle());
  }
  virtual void handleError (int error) {
    fprintf(stderr,"Network: %d: errno: %s(%d)\n",getHandle(),strerror(errno),errno);
  }

  static bool poll (u32 timeout = 0 ) ;
  static void loop (u32 timeout = 0 ) ;
};

#endif // NET_CHANNEL_H
