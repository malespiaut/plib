
#include "ul.h"

static char            _ulErrorBuffer [ 1024 ] = { '\0' } ;
static ulErrorCallback _ulErrorCB = 0 ;

static const char* _ulSeverityText [ UL_MAX_SEVERITY ] =
{
  "DEBUG",
  "WARNING",
  "FATAL",
};
 

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



