
#ifndef _INCLUDED_SSGPARSER_H_
#define _INCLUDED_SSGPARSER_H_


struct _ssgParserSpec
{ // delimiters; Thats is, chars that delimit tokens. 
  const char* delim_chars_skipable ;     // these are "swallowed" by the parser
  const char* delim_chars_non_skipable ; // These are handed to the app.
  const char* open_brace_chars ;
  const char* close_brace_chars ;
  char quote_char ;
  char comment_char ;           // For ex. ';' or '#'
	const char* comment_string;   // For example "//"
} ;


class _ssgParser
{
	enum { MAX_TOKENS = 256 } ; // wk: I have > 32 Tokens per line
  enum { MAX_DELIMITER_CHARS = 256 } ;

  char path[ 256 ] ;
  _ssgParserSpec spec ;
  FILE* fileptr ;

  int linenum ;
  char linebuf[ 4096 ] ;
  char tokbuf[ 4096 ] ;
	char anyDelimiter [ MAX_DELIMITER_CHARS ] ;
  char* tokptr[ MAX_TOKENS ] ;
  int numtok ;
  int curtok ;

	char onechartokenbuf [ 4096 ];
	char *onechartokenbuf_ptr;
	void addOneCharToken ( char *ptr ) ;
  
public :
// general
	void openFile( const char* fname, const _ssgParserSpec* spec = 0 );
  void closeFile();
  
  void error( const char *format, ... );
  void message( const char *format, ... );

// line-by-line API
  int level;

  char* getLine( int startLevel=0 );

  char* parseToken( const char* name );
  SGfloat parseFloat( const char* name );
  int parseInt( const char* name );
  char* parseString( const char* name );
  
  void expect( const char* name );

// line structure independant API
	char *getNextToken( const char* name );
  SGfloat getNextFloat( const char* name );
  int getNextInt( const char* name );
	void expectNextToken( const char* name );
 } ;


#endif