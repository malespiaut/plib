#include <plib/netBuffer.h>


class HTTPClient : public netBufferChannel
{
public:

  HTTPClient ( cchar* host, cchar* path ) //: out_buffer(512)
  {
		create ();
		connect (host, 80);

    cchar* s = netFormat ( "GET %s HTTP/1.0\r\n\r\n", path );
    bufferSend( s, strlen(s) ) ;
  }

  virtual void handleBufferRead (netBuffer& buffer)
  {
    const char* s = buffer.getData();
    printf("%s",s);
    while (*s)
      fputc(*s++,stdout);

    buffer.remove();
  }
} ;


int
main (int argc, char * argv[])
{
  netInit(&argc,argv);

  HTTPClient* hc = new HTTPClient( "www.dynamix.com", "/index.html" );

  netChannel::loop(0);
  return 0;
}
