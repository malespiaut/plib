#include "netRoomMaster.h"
#include "netMonitor.h"


netRoomMasterServer* master = 0 ;


void processCommand ( cchar* _line, netMonitorChannel* channel )
{
  if ( strlen(_line) > 255 )
    return;
  char line [ 256 ] ;
  strcpy ( line, _line ) ;
  printf("echo: %s\n",line);

  /*
  parse the arguments
  */
  enum { MAX_ARGS = 10 } ;
  char* argv [ MAX_ARGS+1 ] ;
  int argc = 0 ;
  char* argp = strtok ( line, " \t" ) ;
  while ( argp != NULL && argc < MAX_ARGS )
  {
    argv [ argc ++ ] = argp ;
    argp = strtok ( NULL, " \t" ) ;
  }
  argv [ argc ] = NULL ;
  if ( argc == 0 )
    return;

  netRoomServerList* servers = & master -> servers ;

  cchar* command = argv [ 0 ] ;
  if (strcmp(command,"list") == 0)
  {
    const netRoomServerInfo* s = servers -> get ( 0 ) ;
    for ( ; s != NULL ; s = servers -> getNext () )
      s -> push ( channel ) ;
    channel -> push (".\r\n");
  }
  else if (strcmp(command,"clear") == 0)
  {
    servers -> clear () ;
  }
  else
  {
    if (strcmp(command,"help") != 0)
    {
      channel -> push("unknown command: ");
      channel -> push(command);
      channel -> push("\r\n");
    }

    channel -> push("help\r\n");
    channel -> push("exit\r\n");
    channel -> push("list\r\n");
    channel -> push("clear\r\n");
    channel -> push (".\r\n");
  }
}


//----------------------------------------------------------------------
int main (int argc, char * argv[])
{
  int port = 8787 ;
  if (argc >= 2)
    port = atoi (argv[1]);

  netInit(&argc,argv);

  master = new netRoomMasterServer ( port ) ;

  netMonitorServer* monitor ;
  monitor = new netMonitorServer ( "Master", port+1 ) ;
  monitor -> setPassword ( "foobar" ) ;
  monitor -> setCommandFunc ( processCommand ) ;

  netChannel::loop (0) ;
  return 0;
}
