#include <stdio.h>
#include <stdlib.h>

#if !defined(WIN32) || defined(__CYGWIN__)
#include <unistd.h>
#endif

#if defined(WIN32)
#include <windows.h>
#endif

#include <string.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <netinet/in.h>
#ifndef __CYGWIN__
#include <netinet/tcp.h>
#endif
#include <netdb.h>
#include <sys/uio.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/errno.h>
#ifdef __sgi
#include <errno.h>
#endif

#if !defined(STDC_HEADERS) && !defined(socklen_t)
#define socklen_t int
#endif

#include "ul.h"

#ifndef SHUT_RDWR
#define SHUT_RDWR 2
#endif

/*
  UDP Networking Class ulNet
*/

ulUDPConnection::ulUDPConnection ()
{
  sockfd   = 0    ;
  in_addr  = NULL ;
  out_addr = NULL ;
}


ulUDPConnection::~ulUDPConnection ()
{
  disconnect () ;
}


void ulUDPConnection::disconnect ()
{
#ifdef WIN32
	return;
#else
  if ( sockfd > 0 )
    shutdown ( sockfd, SHUT_RDWR ) ;

  delete in_addr  ;
  delete out_addr ;
  sockfd   = 0    ;
  in_addr  = NULL ;
  out_addr = NULL ;
#endif
}


int ulUDPConnection::connect ( char *hostname, int _port )
{
#ifdef WIN32
	return 1;
#else
  in_addr  = new sockaddr_in ;
  out_addr = new sockaddr_in ;
  port     = _port ;
  sockfd   = socket ( AF_INET, SOCK_DGRAM, IPPROTO_UDP ) ;

  if ( sockfd < 0 )
  {
    perror ( "socket" ) ;
    fprintf ( stderr, "net: Failed to open a net connection.\n" ) ;
    exit ( 1 ) ;
  }

  /*
    We have to pick a transmit port and a receive port.

    We'll pick the lower number to send on if our hostname
    is alphabetically lower than the other machine.
  */

  char myname [ 256 ] ;

  if ( gethostname ( myname, 256 ) != 0 )
  {
    perror ( "gethostname" ) ;
    fprintf ( stderr, "net: Failed to find this machine's hostname.\n" ) ;
    exit ( 1 ) ;
  }

  int delta = strcmp ( myname, hostname ) ;

  if ( delta == 0 )
  {
    fprintf ( stderr, "net: '%s' is this machine!\n", hostname ) ;
    exit ( 1 ) ;
  }

  if ( delta < 0 )
  {
    iport = port   ;
    oport = port+1 ;
  }
  else
  {
    iport = port+1 ;
    oport = port   ;
  }

  memset ( (char *) in_addr, 0, sizeof( sockaddr_in ) ) ;
  in_addr->sin_family      = AF_INET ;
  in_addr->sin_port        = htons ( iport ) ;
  in_addr->sin_addr.s_addr = htonl(INADDR_ANY) ;

  memset ( (char *) out_addr, 0, sizeof ( sockaddr_in ) ) ;
  out_addr->sin_family     = AF_INET ;
  out_addr->sin_port       = htons ( oport ) ;

  hostent *host = gethostbyname ( hostname ) ;

  if ( ! host )
  {
    fprintf ( stderr, "No match for host: '%s'\n", hostname ) ;
    exit ( 1 ) ;
  }
  
  fprintf ( stderr, "Found host %s at %u.%u.%u.%u\n", hostname,
      (unsigned int) host->h_addr_list[0][0] & 0xFF,
      (unsigned int) host->h_addr_list[0][1] & 0xFF,
      (unsigned int) host->h_addr_list[0][2] & 0xFF,
      (unsigned int) host->h_addr_list[0][3] & 0xFF ) ;

  memcpy( & (out_addr->sin_addr), host->h_addr_list[0],
                                  sizeof ( out_addr->sin_addr ) ) ;
  
  fprintf ( stderr,"Looking for other player on %s on port %d/%d\n",
                                              hostname, iport, oport ) ;

  if ( bind ( sockfd, (struct sockaddr *) in_addr, sizeof(sockaddr_in)) <0)
  {
    perror ( "bind" ) ;
    fprintf ( stderr, "net: Failed to bind to port %d.\n", iport ) ;
    exit ( 1 ) ;
  }

  fcntl ( sockfd, F_SETFL, FNDELAY ) ;

  return 1 ;
#endif
}


int ulUDPConnection::sendMessage ( char *mesg, int length )
{
#ifdef WIN32
  return 1;
#else
  while ( 1 )
  {
    int r = sendto ( sockfd, mesg, length, 0, (sockaddr *) out_addr,
                                               sizeof(sockaddr_in) ) ;

    if ( r == length )
      return 1 ;

    if ( r < 0 && errno != EAGAIN && errno != EWOULDBLOCK &&
                                     errno != ECONNREFUSED )
    {
      perror ( "Sendto" ) ; 
      fprintf ( stderr, "net: Error in sending data.\n" ) ;
      exit ( 1 ) ;
    }

    if ( r >= 0 )
    {
      perror ( "Sendto" ) ; 
      fprintf ( stderr, "net: Unable to send enough data.\n" ) ;
      exit ( 1 ) ;
    }
  }
#endif
}



int ulUDPConnection::recvMessage ( char *mesg, int length )
{
#ifdef WIN32
  return 0;
#else
  socklen_t len = sizeof ( in_addr ) ;

  int r = recvfrom ( sockfd, mesg, length, 0, (sockaddr *) in_addr, &len );

  if ( r < 0 && errno != EAGAIN      &&
                errno != EWOULDBLOCK &&
                errno != ECONNREFUSED )
    perror ( "RecvFrom" ) ;

  return r ;
#endif
}

/*
  TCP Networking Class ulNet
  by Ben Woodhead
*/

ulTCPConnection::ulTCPConnection ()
{
  in_addr  = NULL ;
  out_addr = NULL ;

  sockfd   = 0    ;
  c_sockfd = 0    ;
}

ulTCPConnection::~ulTCPConnection ()
{
  disconnect () ;
}


void ulTCPConnection::disconnect ()
{
#ifdef WIN32
	return;
#else
  if ( sockfd > 0 )
    close ( sockfd ) ;

  if ( c_sockfd > 0 )
    close ( c_sockfd ) ;

  sockfd   = 0    ;
  c_sockfd = 0    ;

  delete in_addr  ;
  delete out_addr ;
  in_addr  = NULL ;
  out_addr = NULL ;

#endif
}

int ulTCPConnection::startServer (int _port )
{
/* 
I have no idea what the return 1 for windows is.
Steve wrote part of this.
I don't think networking works in windows.
Ben
*/

//
// Creating variables and defining type of socket.
//
#ifdef WIN32
	return 1;
#else
  in_addr  = new sockaddr_in ;
  out_addr = new sockaddr_in ;
  port     = _port ;
  sockfd   = socket ( AF_INET, SOCK_STREAM, 0) ;

  if ( sockfd < 0 ) /* Test to insure that socket is open */
  {
    perror ( "socket" ) ;
    fprintf ( stderr, "net: Failed to open a net connection.\n" ) ;
    exit ( 1 ) ;
  }
  in_addr->sin_family      = AF_INET ;
  in_addr->sin_addr.s_addr = htonl(INADDR_ANY) ;
  in_addr->sin_port        = htons ( port ) ;

  if (bind(sockfd, (struct sockaddr *) in_addr, sizeof(sockaddr_in)) < 0)
  {
     fprintf (stderr, "Net: Unable to bind to port.\n" ) ;
     exit(2);
  }

  listen(sockfd, 5);

  for (;;)  /* FIXME: can someone change to a more elegant loop */
  {
        socklen_t len = sizeof(sockaddr_in);
        c_sockfd = accept( sockfd, (struct sockaddr *) out_addr, &len) ;
	
	if (c_sockfd == -1) /* don't ask me, i don't know */
		continue;     /* does continue jump you out of loop */
  } /* End of loop.*/

return(0); /* Should this be an exit as well */

#endif /* Endif for Win32 */

}

int ulTCPConnection::connect ( char *hostname, int _port )
{
#ifdef WIN32
	return 1;
#else
  out_addr = new sockaddr_in ;
  port     = _port ;
  sockfd   = socket ( AF_INET, SOCK_STREAM, 0 ) ;

  if ( sockfd < 0 )
  {
    perror ( "socket" ) ;
    fprintf ( stderr, "net: Failed to open a net connection.\n" ) ;
    exit ( 1 ) ;
  }

  memset ( (char *) out_addr, 0, sizeof ( sockaddr_in ) ) ;
  out_addr->sin_family     = AF_INET ;
  out_addr->sin_port       = htons ( port ) ;

  hostent *host = gethostbyname ( hostname ) ;

  if ( ! host )
  {
    fprintf ( stderr, "No match for host: '%s'\n", hostname ) ;
    exit ( 1 ) ;
  }
  
  fprintf ( stderr, "Found host %s at %u.%u.%u.%u\n", hostname,
      (unsigned int) host->h_addr_list[0][0] & 0xFF,
      (unsigned int) host->h_addr_list[0][1] & 0xFF,
      (unsigned int) host->h_addr_list[0][2] & 0xFF,
      (unsigned int) host->h_addr_list[0][3] & 0xFF ) ;

  memcpy( & (out_addr->sin_addr), host->h_addr_list[0],
                                  sizeof ( out_addr->sin_addr ) ) ;
  
  fprintf ( stderr,"Looking for other player on %s on port %d\n",
                                              hostname, port ) ;

  if ( ::connect( sockfd, (struct sockaddr *) out_addr, sizeof(sockaddr_in)) <0)
  {
    fprintf ( stderr, "net: Failed to connect to port %d.\n", port ) ;
    exit ( 1 ) ;
  }
  fcntl ( sockfd, F_SETFL, FNDELAY ) ;
  return 1 ;
#endif

}


int ulTCPConnection::sendMessage ( char *mesg, int length )
{
int r = 0;
#ifdef WIN32
  return 1;
#else
  do {
    r = send ( sockfd, mesg, length, 0 ) ;
    if ( r == length )
      return 0 ;

    if ( r < 0 && errno != EAGAIN && errno != EWOULDBLOCK &&
                                     errno != ECONNREFUSED )
    {
      fprintf ( stderr, "net: Error in sending data.\n" ) ;
      return  1 ;
    }

    if ( r >= 0 )
    {
      fprintf ( stderr, "net: Unable to send enough data.\n" ) ;
      return  2 ;
    } 

  } while ( length != r );

#endif /* Win32 */
return 0;
}


int ulTCPConnection::recvMessage ( char *mesg, int length )
{
#ifdef WIN32
  return 0;
#else

  int r = recv ( sockfd, mesg, length, 0 ) ;
  if ( r < 0 && errno != EAGAIN      &&
                errno != EWOULDBLOCK &&
                errno != ECONNREFUSED )

  return r ;
#endif
return 0;
}

