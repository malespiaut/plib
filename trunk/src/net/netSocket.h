/****
* NAME
*   netSocket - network sockets
*
* DESCRIPTION
*   netSocket is a thin C++ wrapper over bsd sockets to
*   facilitate porting to other platforms
*
* AUTHOR
*   Dave McClurg <dpm@efn.org>
*
* CREATION DATE
*   Dec-2000
*
****/

#ifndef NET_SOCKET_H
#define NET_SOCKET_H


#include "ul.h"

#if defined(__CYGWIN__) || !defined (WIN32)
#include <errno.h>
#endif


/*
 * Define Basic types
 */

typedef float f32;
typedef double f64;

typedef signed char s8;
typedef signed short s16;
typedef signed int s32;

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

typedef const char cchar;
typedef const void cvoid;


/*
 * Socket address, internet style.
 */
class netAddress
{
  s16     sin_family;
  u16     sin_port;
  u32     sin_addr;
  char    sin_zero[8];

public:
  netAddress () {}
  netAddress ( cchar* host, int port ) ;

  cchar* getHost () const ;
  int getPort() const ;
};


/*
 * Socket type
 */
class netSocket
{
  int handle ;

public:

  netSocket () : handle (-1) {}
  virtual ~netSocket () {}

  int getHandle () const { return handle; }
  void setHandle (int _handle) { handle = _handle ; }
  
  bool  create      ( bool stream=true ) ;
  int   bind        ( cchar* host, int port ) ;
  int   listen	    ( int backlog ) ;
  int   accept      ( netAddress* addr ) ;
  int   connect     ( cchar* host, int port ) ;
  int   send		    ( const void * buffer, int size, int flags = 0 ) ;
  int   sendto      ( const void * buffer, int size, int flags, const netAddress* to ) ;
  int   recv		    ( void * buffer, int size, int flags = 0 ) ;
  int   recvfrom    ( void * buffer, int size, int flags, netAddress* from ) ;
  void  close		    ( void ) ;

  void setBlocking ( bool blocking ) ;

  static bool isNonBlockingError () ;
  static int select ( netSocket** reads, netSocket** writes, int timeout ) ;
} ;


int netInit ( int* argc, char** argv ) ;
cchar* netFormat ( cchar* fmt, ... ) ;


#endif // NET_SOCKET_H
