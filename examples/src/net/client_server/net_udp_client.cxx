/*
     PLIB - A Suite of Portable Game Libraries
     Copyright (C) 2001  Steve Baker

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published by
     the Free Software Foundation; either version 2 of the License, or
     (at your option) any later version.

     This program is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU General Public License for more details.

     You should have received a copy of the GNU General Public License
     along with this program; if not, write to the Free Software
     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA

     For further information visit http://plib.sourceforge.net

     $Id$
*/

// This is an example of a UDP client that blasts messages at a
// server.
//
// UDP is a "fire and forget" protocal so the client and server can be
// started up in any order.  The client doesn't care if the server
// receives the messages and the server doesn't care if the client is
// sending messages, so either side can be killed and restarted at any
// time.

#include <stdio.h>
#include <plib/net.h>
#include <plib/ul.h>

void help ()
{
  fprintf ( stderr, "net_udp_client: Usage -\n" ) ;
  fprintf ( stderr, "\n" ) ;
  fprintf ( stderr, "   net_udp_client [-h] [-p port] [hostname]\n" ) ;
  fprintf ( stderr, "\n" ) ;
  fprintf ( stderr, "Where:\n" ) ;
  fprintf ( stderr, "  -h       -- Help (displays this message).\n" ) ;
  fprintf ( stderr, "  -p port  -- Set the port number (def=5501).\n" ) ;
  fprintf ( stderr, "  hostname -- Set the host name (def=localhost).\n" ) ;
  fprintf ( stderr, "\n" ) ;
  exit ( 0 ) ;
}

int main ( int argc, char **argv )
{
  int port = 5501 ;
  char host[256] = "localhost" ;
  
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

  fprintf ( stderr, "Talking to host '%s' on port %d\n", host, port ) ;

  // Must call this before any other net stuff

  netInit() ;

  netSocket *sock = new netSocket () ;

  if ( ! sock -> open ( false ) )    // open a UDP socket
  {
    printf ( "error opening socket\n" ) ;
    return -1 ;
  }

  sock -> setBlocking ( false ) ;

  if ( sock -> connect( host, port ) == -1 )
  {
    printf ( "error connecting to %s:%d\n", host, port ) ;
    return -1 ;
  }

  char msg[256] = "Hello world!" ;

  int len = strlen ( msg ) ;

  while ( true )
  {
    ulSleep ( 1 ) ;
    sock -> send( msg, len, 0 );
    printf("msg sent = %s\n", msg);
  }
	
  return 0;
}


