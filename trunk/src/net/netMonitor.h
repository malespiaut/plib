/****
* NAME
*   netMonitor - network monitor server
*
* DESCRIPTION
*   netMonitor is a telnet command port with
*   password authorization.  It can be paired
*   with and used to remotely admin another server.
*
* AUTHORS
*   Sam Rushing <rushing@nightmare.com> - original version for Medusa
*   Dave McClurg <dpm@efn.org> - modified for use in Pegasus
*
* CREATION DATE
*   Dec-2000
*
****/

#ifndef NET_MONITOR_H
#define NET_MONITOR_H

#include "netChat.h"


class netMonitorServer : private netChannel
{
  char* name ;
  char* password ;
  char* prompt ;
  void (*cmdfunc)(cchar*) ;
  class netMonitorChannel* active ;

  friend class netMonitorChannel ;

  virtual bool writable (void) { return false ; }
  virtual void handleAccept (void) ;
  
public:

  netMonitorServer( cchar* _name, int port )
  {
    name = strdup(_name);
    password = strdup("") ;
		prompt = strdup(">>> ");
    cmdfunc = 0 ;
    active = 0 ;

		open () ;
		bind ("", port);
		listen (1);

    printf("Monitor \"%s\" started on port %d\n",name,port);
  }

  ~netMonitorServer()
  {
    ::free(name);
    ::free(password) ;
    ::free(prompt) ;
  }

  cchar* getPassword () const { return password; }
  void setPassword ( cchar* string )
  {
    ::free(password) ;
    password = strdup ( string?string:"" ) ;
  }

  void setPrompt ( cchar* string )
  {
    ::free(prompt) ;
    prompt = strdup ( string?string:"" ) ;
  }

  void setCommandFunc ( void (*func)(cchar*) )
  {
    cmdfunc = func ;
  }

  bool push (const char* s) ;
} ;

#endif // NET_MONITOR_H
