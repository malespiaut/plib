//
//  UL - utility library
//
//  Contains:
//  - error message routines
//  - more to come (basic types, endian support, version ID)
//
//  Note: No library is actually created.
//  To generate code you must #define _UL_GENERATE_CODE_
//  in exactly one place.  Currently ssg.cxx does this.
//  If you don't use ssg then you will need to do
//  this on the application side.
//

#ifndef _INCLUDED_UL_H_
#define _INCLUDED_UL_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#ifdef WIN32
#include <windows.h>
#endif

#include <assert.h>

enum ulSeverity
{
  UL_DEBUG,    // Messages that can safely be ignored.
  UL_WARNING,  // Messages that are important.
  UL_FATAL,    // Errors that we cannot recover from.
  UL_MAX_SEVERITY
} ;

void ulInit ( void ) ;

void ulSetError ( int severity, const char *fmt, ... ) ;
char* ulGetError ( void ) ;
void ulClearError ( void ) ;

typedef void (*ulErrorCallback)( int severity, char* msg ) ;
ulErrorCallback ulGetErrorCallback ( void ) ;
void ulSetErrorCallback ( ulErrorCallback cb ) ;

//
//  Note: No library is actually created.
//  To generate code you must #define _UL_GENERATE_CODE_
//  in exactly one place.  Currently ssg.cxx does this.
//  If you don't use ssg then you will need to do
//  this on the application side.
//

#ifdef _UL_GENERATE_CODE_

static char _ulErrorBuffer [ 256 ] ;
static ulErrorCallback _ulErrorCB = 0 ;
static const char* _ulSeverityText [ UL_MAX_SEVERITY ] =
{
  "DEBUG",
  "WARNING",
  "FATAL",
};

void ulInit ( void )
{
  _ulErrorBuffer [0] = 0 ;
}

void ulSetError ( int severity, const char *fmt, ... )
{
  va_list argp;
  va_start ( argp, fmt ) ;
  vsprintf ( _ulErrorBuffer, fmt, argp ) ;
  va_end ( argp ) ;

  if ( _ulErrorCB )
  {
    (*_ulErrorCB)( severity, _ulErrorBuffer ) ;
  }
  else
  {
    fprintf ( stderr, "%s: %s\n",
       _ulSeverityText[ severity ], _ulErrorBuffer ) ;
    if ( severity == UL_FATAL )
      exit (1) ;
  }
}

char* ulGetError ( void )
{
  return _ulErrorBuffer ;
}

void ulClearError ( void )
{
  _ulErrorBuffer [0] = 0 ;
}

ulErrorCallback ulGetErrorCallback ( void )
{
  return _ulErrorCB ;
}

void ulSetErrorCallback ( ulErrorCallback cb )
{
  _ulErrorCB = cb ;
}

#endif
#endif

