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


class netMonitorServer ;


class netMonitorChannel : public netChat
{
  netMonitorServer* server ;
  bool authorized ;
  netBuffer buffer;

  void prompt () ;

  virtual void handleConnect (void) ;
  virtual void handleClose (void)
  {
    printf("%d: Client disconnected.\n",getHandle());
    shouldDelete () ;
  }

  virtual void collectIncomingData	(const char* s, int n) ;
  virtual void foundTerminator (void) ;

public:

  netMonitorChannel ( netMonitorServer* server ) ;
} ;

		
class netMonitorServer : public netChannel
{
  char* name ;
  char* password ;
  char* prompt ;
  void (*cmdfunc)(cchar*, netMonitorChannel*) ;

  friend class netMonitorChannel ;

  virtual bool writable (void)
  {
    return false ;
  }

  virtual void handleAccept (void)
  {
    netAddress addr ;
    int s = accept ( &addr ) ;
		printf("%d: Client %s:%d connected\n",s,addr.getHost(),addr.getPort());

    netMonitorChannel * mc = new netMonitorChannel ( this ) ;
    mc -> setHandle (s);
  }

public:

  netMonitorServer( cchar* _name, int port )
  {
    name = strdup(_name);
    password = strdup("") ;
		prompt = strdup(">>> ");
    cmdfunc = 0 ;

		open ();
		bind ("localhost", port);
		listen (5);

    printf("Monitor server \"%s\" started on port %d\n",name,port);
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

  void setCommandFunc ( void (*func)(cchar*, netMonitorChannel*) )
  {
    cmdfunc = func ;
  }
} ;

#endif // NET_MONITOR_H
