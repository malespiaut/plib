// This is an example of a UDP server that accepts messages coming from a
// client.
//
// UDP is a "fire and forget" protocal so the client and server can be
// started up in any order.  The client doesn't care if the server
// receives the messages and the server doesn't care if the client is
// sending messages, so either side can be killed and restarted at any
// time.

#include <stdio.h>
#include <plib/net.h>


void help ()
{
  fprintf ( stderr, "net_udp_server: Usage -\n" ) ;
  fprintf ( stderr, "\n" ) ;
  fprintf ( stderr, "   net_udp_server [-h] [-p port] [hostname]\n" ) ;
  fprintf ( stderr, "\n" ) ;
  fprintf ( stderr, "Where:\n" ) ;
  fprintf ( stderr, "  -h       -- Help (displays this message).\n" ) ;
  fprintf ( stderr, "  -p port  -- Set the port number (def=5501).\n" ) ;
  fprintf ( stderr, "  hostname -- Set the client host name (def=any host).\n");
  fprintf ( stderr, "\n" ) ;
  exit ( 0 ) ;
}


int main( int argc, char **argv )
{
  int port = 5501 ;
  char host [ 256 ] = "" ;

  for ( int i = 1 ; i < argc ; i++ )
  {
    if ( argv [ i ][ 0 ] == '-' || argv [ i ][ 0 ] == '+' )
      switch ( argv [ i ][ 1 ] )
      {
        case 'p' : port = atoi ( argv [ ++i ] ) ; break ;
        case 'h' :
        default  : help () ; break ;
      }
    else
      strcpy ( host, argv [ i ] ) ;
  }

  if ( host [ 0 ] == '\0' )
    fprintf ( stderr, "Listening to any host on port %d\n", port ) ;
  else
    fprintf ( stderr, "Listening to host '%s' on port %d\n", host, port ) ;
    
  // Must call this before any other net stuff

  netInit () ;

  netSocket *sock = new netSocket () ;

  if ( ! sock -> open( false ) )   // open a UDP socket
  {
    printf ( "error opening socket\n" ) ;
    return -1 ;
  }

  sock -> setBlocking ( false ) ;

  if ( sock -> bind( host, port ) == -1 )
  {
    printf ( "error binding to port %s:%d\n", host, port ) ;
    return -1 ;
  }

  char msg [ 256 ] ;
  int maxlen = 256 ;
  int len ;

  while ( true )
  {
    if ( (len = sock -> recv(msg, maxlen, 0)) >= 0 )
    {
      msg[len] = '\0' ;
      printf ( "msg received = %s\n", msg ) ;
    }
  }

  return 0 ;
}

