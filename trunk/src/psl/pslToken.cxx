/*
     PLIB - A Suite of Portable Game Libraries
     Copyright (C) 1998,2002  Steve Baker

     This library is free software; you can redistribute it and/or
     modify it under the terms of the GNU Library General Public
     License as published by the Free Software Foundation; either
     version 2 of the License, or (at your option) any later version.

     This library is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
     Library General Public License for more details.

     You should have received a copy of the GNU Library General Public
     License along with this library; if not, write to the Free Software
     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA

     For further information visit http://plib.sourceforge.net

     $Id$
*/


#include "pslLocal.h"

static char ungotten_token [ MAX_UNGET ][ MAX_TOKEN ] ;
static int  unget_token_stack_depth = 0 ;

int pslCompiler::getChar ()
{
  /*
    If we got a newline then we have to test to see whether
    a '#' preprocessor directive is on this line.
  */
 
  int c = _pslGetChar () ;

  if ( c == '\n' )
  {
    int d = _pslGetChar () ;
 
    if ( d == '#' )
      return doPreProcessorCommand () ;

    _pslUnGetChar ( d ) ;
  }
 
  /*
    All done - return the character.
  */
 
  return c ;
}


void pslCompiler::doIncludeStatement ()
{
  char token [ MAX_TOKEN ] ;
  char *p ;

  getToken ( token ) ;

  if ( token[0] == '"' )
    p = token + 1 ;
  else
  if ( token[0] == '<' )
  {
    p = token ;

    do
    {
      *p = getChar () ;

    } while ( *(p++) != '>' ) ;

    *(p-1) = '\0' ;

    p = token ;
  }
  else
  {
    error ( "Illegal character after '#include'" ) ;
    return ;
  }

  /*
    Skip to the end of this line of text BEFORE we hand
    control over to the next file.
  */

  int c ;
  do { c = getChar () ; } while ( c != '\n' && c != -1 ) ;

  _pslPushDefaultFile ( p ) ;
}



int pslCompiler::doPreProcessorCommand ()
{
  char token [ MAX_TOKEN ] ;

  getToken ( token ) ;

  /* #include?? */

  if ( strcmp ( token, "include" ) == 0 )
  {
    doIncludeStatement () ;
    return getChar () ;
  }

  if ( strcmp ( token, "define"  ) == 0 )
  {
  }
  else
  if ( strcmp ( token, "ifdef"   ) == 0 )
  {
  }
  else
  if ( strcmp ( token, "endif"   ) == 0 )
  {
  }
  else
  if ( strcmp ( token, "else"    ) == 0 )
  {
  }
  else
    error ( "Unrecognised preprocessor directive '%s'", token ) ;

  /* Skip to the end of this line. */

  int c ;

  do { c = getChar () ; } while ( c != '\n' && c != -1 ) ; 

  return c ;
}


void pslCompiler::getToken ( char *res )
{
  /* WARNING -- RECURSIVE -- WARNING -- RECURSIVE -- WARNING -- RECURSIVE */

  if ( unget_token_stack_depth > 0 )
  {
    strcpy ( res, ungotten_token [ --unget_token_stack_depth ] ) ;
    return ;
  }

  int c ;

  do
  {
    c = getChar () ;

    if ( c < 0 )
    {
      res [ 0 ] = '\0' ;
      return ;
    }

    if ( c == '/' )
    {
      int d = getChar () ;

      if ( d == '/' ) /* C++ style comment */
      {
        do
        {
          d = getChar () ;
        } while ( d != '\n' && d != -1 ) ;

        c = ' ' ;
      }
      else
      if ( d == '*' ) /* C style comment */
      {
        /*
          YUK! This is *so* horrible to get right.
          Just think about this case! ==> **/

        do
        {
          /* Search for a star or an EOF */

          do
          {
            d = getChar () ;
          } while ( d != '*' && d != -1 ) ;

          c = getChar () ;

          /* If you get two stars in a row - unget the second one */

          if ( c == '*' )
            _pslUnGetChar ( '*' ) ;

        } while ( c != '/' ) ;

        c = ' ' ;
      }
    }
  } while ( isspace ( c ) ) ;

  int tp = 0 ;

  if ( c == '"' )
  {
    int isBkSlash = FALSE ;

    do
    {
      if ( c == '\\' )
      {
        if ( isBkSlash )
        {
          isBkSlash = FALSE ;
          res [ tp++ ] = c ;
        }
        else
          isBkSlash = TRUE ;
      }
      else    
      if ( isBkSlash )
      {
        switch ( c )
        {
          case '0' : res [ tp++ ] = '\0' ; break ;
          case 'r' : res [ tp++ ] = '\r' ; break ;
          case 't' : res [ tp++ ] = '\t' ; break ;
          case 'n' : res [ tp++ ] = '\n' ; break ;
          case 'f' : res [ tp++ ] = '\f' ; break ;
          case 'b' : res [ tp++ ] = '\b' ; break ;
          case 'a' : res [ tp++ ] = '\a' ; break ;
          default: res [ tp++ ] =   c  ; break ;
        }

        isBkSlash = FALSE ;
      }
      else
        res [ tp++ ] = c ;

      c = getChar () ;

      if ( tp >= MAX_TOKEN - 1 )
      {
        error ( "Input string is bigger than %d characters!",
                                                       MAX_TOKEN - 1 ) ;
        tp-- ;
      }
    } while ( ( isBkSlash || c != '"' ) && c != -1 ) ;

    if ( c == -1 )
      error ( "Missing \\\" character" ) ;
   
    /* The trailing quotes character is not included into the string */
    res [ tp ] = '\0' ;
    return ;
  }

  while ( isalnum ( c ) || c == '.' || c == '_' )
  {
    res [ tp++ ] = c ;
    c = getChar () ;

    if ( tp >= MAX_TOKEN - 1 )
    {
      error ( "Input string is bigger than %d characters!",
                                                     MAX_TOKEN - 1 ) ;
      tp-- ;
    }
  }

  if ( tp > 0 )
  {
    _pslUnGetChar ( c ) ;
    res [ tp ] = '\0' ;
  }
  else
  {
    res [ 0 ] = c ;
    res [ 1 ] = '\0' ;
  }
}


void pslCompiler::ungetToken ( const char *s )
{
  if ( unget_token_stack_depth >= MAX_UNGET-1 )
  {
    error ( "Too many ungetTokens! This must be an *UGLY* PSL program!" ) ;
    exit ( -1 ) ;
  }

  strcpy ( ungotten_token[unget_token_stack_depth++], s ) ;
}


