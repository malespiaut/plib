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


void ungetToken ( const char *s )
{
  if ( unget_stack_depth >= MAX_UNGET-1 )
  {
    ulSetError ( UL_WARNING,
          "PSL: Too many ungetTokens! This must be an *UGLY* PSL program!" ) ;
    exit ( -1 ) ;
  }

  strcpy ( ungotten_token[unget_stack_depth++], s ) ;
}


