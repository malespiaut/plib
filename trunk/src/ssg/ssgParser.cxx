//
// File parser for SSG/PLIB
// Written by Dave McClurg (dpm@efn.org) in Feb-2000
//


#include "ssgLocal.h"
#include "ssgParser.h"


static _ssgParserSpec default_spec =
{
   "\r\n\t ",  //delim_chars
   0,          //open_brace_chars
   0,          //close_brace_chars
   '"',        //quote_char
   0           //comment_char
} ;


void _ssgParser::error( const char *format, ... )
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


void _ssgParser::message( const char *format, ... )
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


void _ssgParser::openFile( const char* fname, const _ssgParserSpec* _spec )
{
  if ( !_spec ) _spec = &default_spec ;
  memset(this,0,sizeof(_ssgParser));
  memcpy( &spec, _spec, sizeof(spec) );
  fileptr = fopen( _ssgMakePath(path,_ssgModelPath,fname), "rb" );
  if ( ! fileptr )
    error("cannot open file: %s",path);
}


void _ssgParser::closeFile()
{
  fclose( fileptr ) ;
  fileptr = 0 ;
}


char* _ssgParser::getLine( int startLevel )
{
  tokbuf [ 0 ] = 0 ;
  numtok = 0 ;
  curtok = 0 ;

  //get the next line with something on it
  char* ptr = tokbuf ;
  while ( *ptr == 0 )
  {
     linenum++ ;
     if ( fgets ( linebuf, sizeof(linebuf), fileptr ) == NULL )
       return(0) ;

     memcpy( tokbuf, linebuf, sizeof(linebuf) ) ;
     ptr = tokbuf ;
       
     //skip delim_chars
     while ( *ptr && strchr(spec.delim_chars,*ptr) )
       ptr++ ;
  }

  //comment line?
  if ( spec.comment_char && *ptr == spec.comment_char )
    return(ptr);

  //tokenize the line
  numtok = 0 ;
  while ( *ptr )
  {
    if ( *ptr == spec.comment_char )
    {
      *ptr = 0 ;
      break;
    }

    //count the token
    tokptr [ numtok++ ] = ptr ;

    //handle quoted string
    if ( spec.quote_char && *ptr == spec.quote_char )
    {
      ptr++ ;
      while ( *ptr && *ptr != spec.quote_char )
        ptr++ ;
    }

    //adjust level
    if ( spec.open_brace_chars && *ptr && strchr(spec.open_brace_chars,*ptr) )
      level++ ;
    else if ( spec.close_brace_chars && *ptr && strchr(spec.close_brace_chars,*ptr) )
      level-- ;

    //find end of token
    while ( *ptr && !strchr(spec.delim_chars,*ptr) )
      ptr++ ;

    //mark end of token
    while ( *ptr && strchr(spec.delim_chars,*ptr) )
      *ptr++ = 0 ;
  }

  if (level >= startLevel)
    return parseToken (0) ;
  return 0 ;
}


char* _ssgParser::parseToken( const char* name )
{
  char* token = 0 ;
  if ( curtok < numtok )
    token = tokptr [ curtok++ ] ;
  else if ( name )
    error("missing %s",name) ;
  return(token) ;
}


char* _ssgParser::parseString( const char* name )
{
  char* token = 0 ;
  if ( numtok > 0 && spec.quote_char && *tokptr [ curtok ] == spec.quote_char )
  {
    token = tokptr [ curtok++ ] ;

    //knock off the quotes
    token++ ;
    int len = strlen (token) ;
    if (len > 0 && token[len-1] == spec.quote_char)
       token[len-1] = 0 ;
  }
  else if ( name )
    error("missing %s",name) ;
  return(token) ;
}


SGfloat _ssgParser::parseFloat( const char* name )
{
  char* token = parseToken(name);
  SGfloat value = SGfloat(atof(token));
  return( value );
}


int _ssgParser::parseInt( const char* name )
{
  char* token = parseToken(name);
  int value = int(atoi(token));
  return( value );
}


void _ssgParser::expect( const char* name )
{
  char* token = parseToken(name);
  if (strcmp(token,name))
    error("missing %s",name) ;
}
