#include <plib/netChat.h>

/*
**  simple example server that reads lines of text from the client
**  and echoes them back. An echo server is usually running on TCP
**  port 7 of Unix machines. 
*/

class EchoChannel : public netChat
{
  netBuffer buffer ;

public:

  EchoChannel() : buffer(512) { setTerminator("\r\n"); }

  virtual void collectIncomingData	(const char* s, int n)
  {
    buffer.append(s,n);
  }

  virtual void foundTerminator (void)
  {
    const char* s = buffer.getData();
    printf("echo: %s\n",s);

    push(s);
    push(getTerminator());

    buffer.remove();
  }
} ;

class EchoServer : private netChannel
{
  virtual bool writable (void) { return false ; }

  virtual void handleAccept (void)
  {
    netAddress addr ;
    int handle = accept ( &addr ) ;
		printf("Client %s:%d connected\n",addr.getHost(),addr.getPort());

    EchoChannel * ec = new EchoChannel;
    ec->setHandle ( handle ) ;
  }

public:

  EchoServer ( int port )
  {
		open () ;
		bind ("", port) ;
		listen (5) ;

    printf ( "Echo Server started on port %d\n", port ) ;
  }
} ;

int
main (int argc, char * argv[])
{
  int port = 8000 ;
  
  if (argc > 1)
    port = atoi (argv[1]);

  netInit(&argc,argv);

  new EchoServer ( port ) ;

  netChannel::loop(0);
  return 0;
}
