//
// File parser for SSG/PLIB
// Written by Dave McClurg (dpm@efn.org) in Feb-2000
//


#include <stdarg.h>
#include "ssgLocal.h"
#include "ssgParser.h"


void _ssgParser::error( cchar *format, ... )
{
  char msgbuff[ 255 ];
  va_list argp;

  char* msgptr = msgbuff;
  if (linenum)
  {
    msgptr += sprintf ( msgptr,"%s, line %d: ",
      path, linenum );
  }

  va_start( argp, format );
  vsprintf( msgptr, format, argp );
  va_end( argp );

  ulSetError ( UL_WARNING, "%s", msgbuff ) ;
}


void _ssgParser::message( cchar *format, ... )
{
  char msgbuff[ 255 ];
  va_list argp;

  char* msgptr = msgbuff;
  if (linenum)
  {
    msgptr += sprintf ( msgptr,"%s, line %d: ",
      path, linenum );
  }

  va_start( argp, format );
  vsprintf( msgptr, format, argp );
  va_end( argp );

  ulSetError ( UL_DEBUG, "%s", msgbuff ) ;
}


void _ssgParser::openFile( cchar* fname )
{
  memset(this,0,sizeof(_ssgParser));
  ptr = fopen( _ssgMakePath(path,_ssgModelPath,fname), "rb" );
  if ( ! ptr )
    error("cannot open file: %s",path);
}


void _ssgParser::closeFile()
{
  fclose( ptr ) ;
  ptr = 0 ;
}


char* _ssgParser::getLine( int startLevel )
{
  linenum++ ;
  if ( fgets ( linebuf, sizeof(linebuf), ptr ) == NULL )
    return(0) ;
    
  //adjust level
  if (strchr(linebuf,'{') != 0)
    level++ ;
  else if (strchr(linebuf,'}') != 0)
    level-- ;

  if (level >= startLevel)
  {
    memcpy( tokbuf, linebuf, sizeof(linebuf) ) ;
    tokptr = tokbuf ;
    
    return parseToken (0) ;
  }
  return 0 ;
}


char* _ssgParser::parseToken( cchar* name )
{
  static cchar delims[] = "\r\n\t ";

  //skip delims
  while ( *tokptr && strchr(delims,*tokptr) )
    tokptr++ ;

  char* token = 0 ;
  if ( *tokptr )
  {
    token = tokptr ;
    
    //find end of token
    while ( *tokptr && !strchr(delims,*tokptr) )
      tokptr++ ;
    if ( *tokptr )
      *tokptr++ = 0 ;
  }
  else if ( name )
    error("missing %s",name) ;
  return(token) ;
}


char* _ssgParser::parseString( cchar* name )
{
  //find and return a double quoted string in the linebuf
  static char str[256];
  int maxlen = sizeof(str);
  char* start = strchr(linebuf,'"');
  if (!start)
    error("missing %s",name);
  start++;
  char* end = strchr(start,'"');
  if (!end)
    error("missing %s",name);
  int len = end-start;
  if (len >= maxlen)
    error("string too long");
  memcpy(str,start,len);
  memset(str+len,0,maxlen-len);
  return(str);
}


f32 _ssgParser::parseFloat( cchar* name )
{
  char* token = parseToken(name);
  f32 value = f32(atof(token));
  return( value );
}


s32 _ssgParser::parseInt( cchar* name )
{
  char* token = parseToken(name);
  s32 value = s32(atoi(token));
  return( value );
}


void _ssgParser::expect( cchar* name )
{
  char* token = parseToken(name);
  if (strcmp(token,name))
    error("missing %s",name) ;
}
