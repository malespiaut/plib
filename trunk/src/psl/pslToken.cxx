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

static FILE *defaultFile ;
static char ungotten_token [ MAX_UNGET ][ MAX_TOKEN ] ;
static int  unget_stack_depth = 0 ;


void pslCompiler::setDefaultFile ( FILE *fd )
{
  defaultFile = fd ;
}


int pslCompiler::getChar ( FILE *fd )
{
  int c = getc ( fd ) ;

  if ( c == '\n' ) line_no++ ;

  return c ;
}


int pslCompiler::unGetChar ( int c, FILE *fd )
{
  int res = ungetc ( c, fd ) ;

  if ( c == '\n' ) line_no-- ;

  return res ;
}


void pslCompiler::getToken ( char *res, FILE *fd )
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
    c = getChar ( fd ) ;

    if ( c < 0 )
    {
      res [ 0 ] = '\0' ;
      return ;
    }

    if ( c == '/' )
    {
      int d = getChar ( fd ) ;

      if ( d == '/' ) /* C++ style comment */
      {
        do
        {
          d = getChar ( fd ) ;
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
            d = getChar ( fd ) ;
          } while ( d != '*' && d != -1 ) ;

          c = getChar ( fd ) ;

          /* If you get two stars in a row - unget the second one */

          if ( c == '*' )
            unGetChar ( '*', fd ) ;

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
          case 'n' : res [ tp++ ] = '\n' ; break ;
          case 'f' : res [ tp++ ] = '\f' ; break ;
          case 'b' : res [ tp++ ] = '\b' ; break ;
          case 'a' : res [ tp++ ] = '\a' ; break ;
          default: res [ tp++ ] =   c  ; break ;
        }
      }
      else
        res [ tp++ ] = c ;

      c = getChar ( fd ) ;

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
    c = getChar ( fd ) ;

    if ( tp >= MAX_TOKEN - 1 )
    {
      error ( "Input string is bigger than %d characters!",
                                                     MAX_TOKEN - 1 ) ;
      tp-- ;
    }
  }

  if ( tp > 0 )
  {
    unGetChar ( c, fd ) ;
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
  if ( unget_stack_depth >= MAX_UNGET-1 )
  {
    error ( "Too many ungetTokens! This must be an *UGLY* PSL program!" ) ;
    exit ( -1 ) ;
  }

  strcpy ( ungotten_token[unget_stack_depth++], s ) ;
}


