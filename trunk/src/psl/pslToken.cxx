
#include "pslLocal.h"

#define MAX_UNGET   16

static FILE *defaultFile ;
static char ungotten_token [ MAX_UNGET ][ MAX_TOKEN ] ;
static int  unget_stack_depth = 0 ;


void setDefaultFile ( FILE *fd )
{
  defaultFile = fd ;
}


void getToken ( char *res, FILE *fd )
{
  if ( unget_stack_depth > 0 )
  {
    strcpy ( res, ungotten_token [ --unget_stack_depth ] ) ;
    return ;
  }

  if ( fd == NULL ) fd = defaultFile ;

  int c ;

  do
  {
    c = getc ( fd ) ;

    if ( c < 0 )
    {
      res [ 0 ] = '\0' ;
      return ;
    }
  } while ( isspace ( c ) ) ;

  int tp = 0 ;

  while ( isalnum ( c ) || c == '.' || c == '_' )
  {
    res [ tp++ ] = c ;
    c = getc ( fd ) ;

    if ( tp >= MAX_TOKEN - 1 )
    {
      ulSetError ( UL_WARNING,
               "PSL: Input string is bigger than %d characters!",
                                                     MAX_TOKEN - 1 ) ;
      tp-- ;
    }
  }

  if ( tp > 0 )
  {
    ungetc ( c, fd ) ;
    res [ tp ] = '\0' ;
  }
  else
  {
    res [ 0 ] = c ;
    res [ 1 ] = '\0' ;
  }
}


void ungetToken ( char *s )
{
  if ( unget_stack_depth >= MAX_UNGET-1 )
  {
    ulSetError ( UL_WARNING,
          "PSL: Too many ungetTokens! This must be an *UGLY* PSL program!" ) ;
    exit ( -1 ) ;
  }

  strcpy ( ungotten_token[unget_stack_depth++], s ) ;
}


