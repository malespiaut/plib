#include "netMonitor.h"

void processCommand ( cchar* command, netMonitorChannel* channel )
{
  printf("echo: %s\n",command);

  channel -> push(command);
  channel -> push("\r\n");
}

int
main (int argc, char * argv[])
{
  nlInit(&argc,argv);

  netMonitorServer* m = new netMonitorServer ( "Test", 8888 ) ;
  m -> setPassword ( "foobar" ) ;
  m -> setCommandFunc ( processCommand ) ;

  netChannel::loop(0);
  return 0;
}
