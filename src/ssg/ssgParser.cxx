//
// File parser for SSG/PLIB
// Written by Dave McClurg (dpm@efn.org) in Feb-2000
//

#define AM_IN_SSGPARSER_CXX 1

#include "ssgLocal.h"
#include "ssgParser.h"


static _ssgParserSpec default_spec =
{
   "\r\n\t ",  // delim_chars_skipable
   0,          // delim_chars_non_skipable
   0,          // open_brace_chars
   0,          // close_brace_chars
   '"',        // quote_char
   0,          // comment_char
	 "//"        // comment_string
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
	anyDelimiter[0] = 0;
	int length = 0;
	if ( spec.delim_chars_skipable != NULL )
	{ length +=strlen ( spec.delim_chars_skipable);
	  strcat(anyDelimiter, spec.delim_chars_skipable);
	}
	if ( spec.delim_chars_non_skipable  != NULL )
	{ length += strlen ( spec.delim_chars_non_skipable ) ;
	  strcat ( anyDelimiter, spec.delim_chars_non_skipable ) ;
	}
	if ( spec.open_brace_chars  != NULL )
	{ length +=strlen ( spec.open_brace_chars );
	  strcat ( anyDelimiter, spec.open_brace_chars );
	}
	if ( spec.close_brace_chars  != NULL )
	{ length +=strlen ( spec.close_brace_chars ) ;
	  strcat ( anyDelimiter, spec.close_brace_chars ) ;
	}
	assert ( length < MAX_DELIMITER_CHARS );
}


void _ssgParser::closeFile()
{
  fclose( fileptr ) ;
  fileptr = 0 ;
}
// wk start


char* _ssgParser::getNextToken( const char* name )
{
	while(!( curtok < numtok ))
	{	//int startLevel = level;
	  //printf("Forcing!\n");
		if(getLine( -999 ) == NULL) // -999
		{	if ( name )
				error("missing %s",name) ;
			return NULL;
		}
		assert(curtok==1);
		curtok=0; // redo the get one token that getLine does
	}
  char* token = 0 ;
  assert ( curtok < numtok );
  token = tokptr [ curtok++ ] ;
	return(token) ;
}

SGfloat _ssgParser::getNextFloat( const char* name )
{
  char* token = getNextToken(name);
  SGfloat value = SGfloat(atof(token));
  return( value );
}

int _ssgParser::getNextInt( const char* name )
{
  char* token = getNextToken(name);
  int value = int(atoi(token));
  return( value );
}


void _ssgParser::expectNextToken( const char* name )
{
  char* token = getNextToken(name);
  if (strcmp(token,name))
    error("missing %s",name) ;
}

void _ssgParser::addOneCharToken ( char *ptr ) 
{
	assert( (long)onechartokenbuf_ptr- (long)onechartokenbuf < 4096 ) ; // Buffer overflow
	
	onechartokenbuf_ptr [ 0 ] = *ptr;
	onechartokenbuf_ptr [ 1 ] = 0;
	tokptr [ numtok++ ] = onechartokenbuf_ptr;
	onechartokenbuf_ptr += 2; // prepare for nect onechartoken
}

char *mystrchr( const char *string, int c )
// like strchr, but string may be NULL
{
	if (string == NULL )
		return NULL;
	else
		return strchr( string, c );
}


// wk stop
char* _ssgParser::getLine( int startLevel )
{
	// throw away old tokens
  tokbuf [ 0 ] = 0 ;
  numtok = 0 ;
  curtok = 0 ;
	onechartokenbuf_ptr = onechartokenbuf ;
	
  //get the next line with something on it
  char* ptr = tokbuf , *tptr;
  while ( *ptr == 0 )
  {
		linenum++ ;
		if ( fgets ( linebuf, sizeof(linebuf), fileptr ) == NULL )
		 return(0) ;

		memcpy( tokbuf, linebuf, sizeof(linebuf) ) ;
		ptr = tokbuf ;

		// check for comments
		tptr=strchr(tokbuf, spec.comment_char);
		if ( tptr != NULL )
			*tptr = 0;
		if ( spec.comment_string != NULL )
		{
			tptr=strstr(tokbuf, spec.comment_string);
			if ( tptr != NULL )
				*tptr = 0;
		}



 
		//skip delim_chars
		if ( spec.delim_chars_skipable != NULL )
			while ( *ptr && strchr(spec.delim_chars_skipable,*ptr) )
				ptr++ ;
  }

  //tokenize the line
  numtok = 0 ;
  while ( *ptr )
  {
     //skip delim_chars
		if ( spec.delim_chars_skipable != NULL )
			while ( *ptr && strchr(spec.delim_chars_skipable,*ptr) )
				ptr++ ;

		if ( *ptr == 0 )
			break; // only skipable stuff left, dont create another token.
  
		// now unnessary?:
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
    if ( spec.open_brace_chars && *ptr && mystrchr(spec.open_brace_chars,*ptr) )
      level++ ;
    else if ( spec.close_brace_chars && *ptr && mystrchr(spec.close_brace_chars,*ptr) )
      level-- ;
		else
			//find end of token
			while ( *ptr && !strchr(anyDelimiter,*ptr) )
				ptr++ ;
		
		if ( *ptr != 0 )
			if ( ptr == tokptr [ numtok-1 ] )
			{ // we dont want tokens of length zero
				char *a=mystrchr(spec.delim_chars_skipable,*ptr);
				assert(NULL==mystrchr(spec.delim_chars_skipable,*ptr));
				// ptr is non-skipable, return it as token of length one
				numtok--;                  // remove zero-length token
				addOneCharToken ( ptr ) ;  // and add new token instead
				*ptr++ = 0;
				continue;
			}

    //mark end of token
		if( *ptr && ( mystrchr(spec.delim_chars_non_skipable,*ptr) 
			        || mystrchr(spec.open_brace_chars,*ptr)
							|| mystrchr(spec.close_brace_chars,*ptr) ) )
		{ 
			// ptr is non-skipable, return it as token of length one
			// additional to the one already in tokptr [ numtok-1 ].
			addOneCharToken ( ptr ) ;
			*ptr++ = 0;
		}
		if ( spec.delim_chars_skipable != NULL )
			while ( *ptr && strchr(spec.delim_chars_skipable,*ptr) )
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
