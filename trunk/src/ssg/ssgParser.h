
#ifndef _INCLUDED_SSGPARSER_H_
#define _INCLUDED_SSGPARSER_H_


struct _ssgParserSpec
{
  const char* delim_chars ;
  const char* open_brace_chars ;
  const char* close_brace_chars ;
  char quote_char ;
  char comment_char ;
} ;


class _ssgParser
{
  enum { MAX_TOKENS = 32 } ;

  char path[ 256 ] ;
  _ssgParserSpec spec ;
  FILE* fileptr ;

  int linenum ;
  char linebuf[ 256 ] ;

  char tokbuf[ 256 ] ;
  char* tokptr[ MAX_TOKENS ] ;
  int numtok ;
  int curtok ;
  
public :

  int level;

  void openFile( const char* fname, const _ssgParserSpec* spec = 0 );
  void closeFile();

  char* getLine( int startLevel=0 );

  char* parseToken( const char* name );
  SGfloat parseFloat( const char* name );
  int parseInt( const char* name );
  char* parseString( const char* name );
  
  void expect( const char* name );
  
  void error( const char *format, ... );
  void message( const char *format, ... );
} ;


#endif