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

#define MAX_UNGET   16

static FILE *defaultFile ;
static char ungotten_token [ MAX_UNGET ][ MAX_TOKEN ] ;
static int  unget_stack_depth = 0 ;


void pslSetDefaultFile ( FILE *fd )
{
  defaultFile = fd ;
}


void pslGetToken ( char *res, FILE *fd )
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

    if ( c == '/' )
    {
      int d = getc ( fd ) ;

      if ( d == '/' ) /* C++ style comment */
      {
        do
        {
          d = getc ( fd ) ;
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
            d = getc ( fd ) ;
          } while ( d != '*' && d != -1 ) ;

          c = getc ( fd ) ;

          /* If you get two stars in a row - unget the second one */

          if ( c == '*' )
            ungetc ( '*', fd ) ;

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

      c = getc ( fd ) ;

      if ( tp >= MAX_TOKEN - 1 )
      {
        ulSetError ( UL_WARNING,
                 "PSL: Input string is bigger than %d characters!",
                                                       MAX_TOKEN - 1 ) ;
        tp-- ;
      }
    } while ( ( isBkSlash || c != '"' ) && c != -1 ) ;

    if ( c == -1 )
      ulSetError ( UL_WARNING,
               "PSL: Missing \\\" character" ) ;
   
    /* The trailing quotes character is not included into the string */
    res [ tp ] = '\0' ;
    return ;
  }

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


void pslUngetToken ( const char *s )
{
  if ( unget_stack_depth >= MAX_UNGET-1 )
  {
    ulSetError ( UL_WARNING,
          "PSL: Too many ungetTokens! This must be an *UGLY* PSL program!" ) ;
    exit ( -1 ) ;
  }

  strcpy ( ungotten_token[unget_stack_depth++], s ) ;
}


