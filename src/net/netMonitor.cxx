#include "netMonitor.h"


class netMonitorChannel : public netChat
{
  netMonitorServer* server ;
  bool authorized ;
  netBuffer buffer;

  void prompt () ;

  virtual void collectIncomingData	(const char* s, int n) ;
  virtual void foundTerminator (void) ;

public:

  netMonitorChannel ( netMonitorServer* server ) ;

  virtual void handleClose (void)
  {
    printf("%d: Client disconnected.\n",getHandle());
    shouldDelete () ;
    netChat::handleClose () ;
    server -> active = 0 ;
  }
} ;

		
// for now, we ignore any telnet option stuff sent to
// us, and we process the backspace key ourselves.
// gee, it would be fun to write a full-blown line-editing
// environment, etc...

static void clean_line (char* line)
{
  char* dst = line ;
  for ( char* src = line ; *src ; src ++ )
  {
    char ch = *src ;
    if (ch==8 || ch==177)
    {
      // backspace
      if (dst != line)
        dst -- ;
    }
    else if (ch<127)
    {
      *dst++ = *src ;
    }
  }
  *dst = 0 ;
}


netMonitorChannel::netMonitorChannel ( netMonitorServer* _server ) : buffer(512)
{
  server = _server ;
  setTerminator("\r\n");
  
  if ( server -> password && server -> password [0] != 0 )
  {
    authorized = false ;
    push ("Enter password: ") ;
  }
  else
  {
    authorized = true ;
    push ( netFormat("Connected to \"%s\"... Welcome!\r\n", server -> name ) ) ;
    prompt();
  }
}


void netMonitorChannel::prompt ()
{
	push ( server -> prompt ) ;
}


void netMonitorChannel::collectIncomingData	(const char* s, int n)
{
  if ( !buffer.append(s,n) )
  {
    // denial of service.
    push ("BCNU\r\n");
    closeWhenDone();
  }
}

void netMonitorChannel::foundTerminator (void)
{
  char* line = buffer.getData();
  clean_line ( line ) ;
  
  if (!authorized)
  {
    if (strcmp(line,server -> password) == 0)
    {
      authorized = true ;
      push ( netFormat("Connected to \"%s\"... Welcome!\r\n",server -> name) ) ;
      prompt () ;
    }
    else
    {
      close();
    }
  }
  else if (*line == 0)
  {
    prompt();
  }
  else if (*line == 4 || strcmp(line,"exit") == 0)
  {
    push ("BCNU\r\n");  //Be seein' you
    closeWhenDone();
  }
  else
  {
    if ( server -> cmdfunc )
    {
      server -> cmdfunc ( line ) ;
    }
    else
    {
      printf("echo: %s\n",line);

      push(line);
      push(getTerminator());
    }
    
    prompt();
  }
  buffer.remove();
}


void netMonitorServer::handleAccept (void)
{
  if ( !active )
  {
    netAddress addr ;
    int s = accept ( &addr ) ;

    printf("%d: Client %s:%d connected\n",s,addr.getHost(),addr.getPort());

    active = new netMonitorChannel ( this ) ;
    active -> setHandle (s);
  }
}


bool netMonitorServer::push (const char* s)
{
  if ( active )
    return active -> push ( s ) ;
  return false ;
}
