#include "netSocket.h"
#include <winsock.h>

netAddress::netAddress ( cchar* host, int port )
{
	memset(this, 0, sizeof(netAddress));

  sin_family = AF_INET ;
  sin_port = htons (port);

  /* Convert a string specifying a host name or one of a few symbolic
  ** names to a numeric IP address.  This usually calls gethostbyname()
  ** to do the work; the names "" and "<broadcast>" are special.
  */

	if (host[0] == '\0') {
		sin_addr = INADDR_ANY;
	}
	if (host[0] == '<' && strcmp(host, "<broadcast>") == 0) {
		sin_addr = INADDR_BROADCAST;
	}
  else
  {
  	int d1, d2, d3, d4;
  	char ch;
  	if (sscanf(host, "%d.%d.%d.%d%c", &d1, &d2, &d3, &d4, &ch) == 4 &&
  	    0 <= d1 && d1 <= 255 && 0 <= d2 && d2 <= 255 &&
  	    0 <= d3 && d3 <= 255 && 0 <= d4 && d4 <= 255) {
  		sin_addr = htonl(
  			((long) d1 << 24) | ((long) d2 << 16) |
  			((long) d3 << 8) | ((long) d4 << 0));
  	}
    else  //let's try gethostbyname()
    {
    	struct hostent *hp = gethostbyname(host);
    	if (hp != NULL)
      {
      	memcpy((char *) &sin_addr, hp->h_addr, hp->h_length);
      }
      else  //failure
      {
        sin_addr = INADDR_ANY;
      }
    }
  }
}

/* Create a string object representing an IP address.
   This is always a string of the form 'dd.dd.dd.dd' (with variable
   size numbers). */

cchar* netAddress::getHost () const
{
  static char buf [32];
	long x = ntohl(sin_addr);
	sprintf(buf, "%d.%d.%d.%d",
		(int) (x>>24) & 0xff, (int) (x>>16) & 0xff,
		(int) (x>> 8) & 0xff, (int) (x>> 0) & 0xff );
  return buf;
}


int netAddress::getPort() const
{
  return ntohs(sin_port);
}


bool
netSocket::create ( bool stream )
{
  int type = stream? SOCK_STREAM: SOCK_DGRAM ;
  int protocol = 0 ;
  handle = ::socket ( AF_INET, type, protocol ) ;
  return (handle != -1);
}

void
netSocket::setBlocking ( bool blocking )
{
  int res = SOCKET_ERROR ;
  u_long block = blocking? 1: 0;
  ::ioctlsocket(handle, FIONBIO, &block);
}

int
netSocket::bind ( cchar* host, int port )
{
  netAddress addr ( host, port ) ;
  return ::bind(handle,(const sockaddr*)&addr,sizeof(netAddress));
}

int
netSocket::listen ( int backlog )
{
  assert ( handle != -1 ) ;
  return ::listen(handle,backlog);
}

int
netSocket::accept ( netAddress* addr )
{
  int addr_len = sizeof(netAddress) ;
  return ::accept(handle,(sockaddr*)addr,&addr_len);
}

int
netSocket::connect ( cchar* host, int port )
{
  netAddress addr ( host, port ) ;
  return ::connect(handle,(const sockaddr*)&addr,sizeof(netAddress));
}

int
netSocket::send (const void * buffer, int size, int flags)
{
  return ::send (handle, (cchar*)buffer, size, flags);
}

int
netSocket::sendto ( const void * buffer, int size, int flags, const netAddress* to )
{
  return ::sendto(handle,(cchar*)buffer,size,flags,(const sockaddr*)to,sizeof(netAddress));
}

int
netSocket::recv (void * buffer, int size, int flags)
{
  return ::recv (handle, (char*)buffer, size, flags);
}

int
netSocket::recvfrom ( void * buffer, int size, int flags, netAddress* from )
{
  int fromlen = sizeof(netAddress) ;
  return ::recvfrom(handle,(char*)buffer,size,flags,(sockaddr*)from,&fromlen);
}

void
netSocket::close (void)
{
  if ( handle != -1 )
  {
    ::closesocket( handle );
    handle = -1 ;
  }
}

bool
netSocket::isNonBlockingError ()
{
#ifdef WIN32
  int wsa_errno = WSAGetLastError();
  if ( wsa_errno != 0 )
  {
    WSASetLastError(0);
    fprintf(stderr,"WSAGetLastError() => %d\n",wsa_errno);
    switch (wsa_errno) {
    case WSAEWOULDBLOCK: // always == NET_EAGAIN?
    case WSAEALREADY:
    case WSAEINPROGRESS:
      return true;
    }
  }
  return false;
#else
  switch (errno) {
  case EWOULDBLOCK: // always == NET_EAGAIN?
  case EALREADY:
  case EINPROGRESS:
    return true;
  }
  return false;
#endif
}

int
netSocket::select ( netSocket** reads, netSocket** writes, int timeout )
{
  fd_set r,w;
  
  FD_ZERO (&r);
  FD_ZERO (&w);

  int i, k ;
  int num = 0 ;

  for ( i=0; reads[i]; i++ )
  {
    int fd = reads[i]->getHandle();
    FD_SET (fd, &r);
    num++;
  }

  for ( i=0; writes[i]; i++ )
  {
    int fd = writes[i]->getHandle();
    FD_SET (fd, &w);
    num++;
  }

  if (!num)
    return num ;

  /* Set up the timeout */
  struct timeval tv ;
  tv.tv_sec = timeout/1000;
  tv.tv_usec = (timeout%1000)*1000;

  // It bothers me that select()'s first argument does not appear to
  // work as advertised... [it hangs like this if called with
  // anything less than FD_SETSIZE, which seems wasteful?]
  
  // Note: we ignore the 'exception' fd_set - I have never had a
  // need to use it.  The name is somewhat misleading - the only
  // thing I have ever seen it used for is to detect urgent data -
  // which is an unportable feature anyway.

  ::select (FD_SETSIZE, &r, &w, 0, &tv);

  //remove sockets that had no activity

  num = 0 ;

  for ( k=i=0; reads[i]; i++ )
  {
    int fd = reads[i]->getHandle();
    if (FD_ISSET (fd, &r)) {
      reads[k++] = reads[i];
      num++;
    }
  }
  reads[k] = NULL ;

  for ( k=i=0; writes[i]; i++ )
  {
    int fd = writes[i]->getHandle();
    if (FD_ISSET (fd, &w)) {
      writes[k++] = writes[i];
      num++;
    }
  }
  writes[k] = NULL ;

  return num ;
}

/* Init/Exit functions */
static void netExit ( void )
{
	/* Clean up windows networking */
	if ( WSACleanup() == SOCKET_ERROR ) {
		if ( WSAGetLastError() == WSAEINPROGRESS ) {
			WSACancelBlockingCall();
			WSACleanup();
		}
	}
}

int netInit ( int* argc, char** argv )
{
  assert ( sizeof(sockaddr_in) == sizeof(netAddress) ) ;

	/* Start up the windows networking */
	WORD version_wanted = MAKEWORD(1,1);
	WSADATA wsaData;

	if ( WSAStartup(version_wanted, &wsaData) != 0 ) {
		fprintf(stderr,"Couldn't initialize Winsock 1.1\n");
		return(-1);
	}
  atexit( netExit ) ;
	return(0);
}
