
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

class pslFile ;

static pslFile *defaultFileStack [ MAX_INCLUDE_DEPTH ] ;
static int      fileStackPointer = 0 ;

class pslFile
{
  FILE *fd ;
  char *fname ;

  char ungetStack [ MAX_UNGET_CHAR ] ;
  int  ungetSP  ;

  int  line_no ;

public:

  pslFile ( FILE *_fd, const char *_fname )
  {
    fname = ulStrDup ( _fname ) ;

    fd = _fd ;
    ungetSP  = 0 ;
    line_no = 0 ;

    if ( fileStackPointer >= MAX_INCLUDE_DEPTH - 1 )
      fprintf ( stderr, "PSL: ERROR - Too many nested #includes" ) ;
    else
      defaultFileStack [ fileStackPointer++ ] = this ;

    /*
      This is a kludge to get preprocessor directives to see
      a '#' as the very first character in the file as if it came
      immediately after a newline...also accounted for by line_no
      starting at zero instead of 1.
    */

    unGetChar ( '\n' ) ;
    line_no = 0 ;
  }
 
  ~pslFile ()
  {
    if ( fileStackPointer <= 0 )
      fprintf ( stderr, "PSL: ERROR - #include stack underflow?!?" ) ;
    else
    {
      if ( defaultFileStack [ --fileStackPointer ] != this )
        fprintf ( stderr, "PSL: ERROR - #include nesting problem?!?" ) ;
      else
        fclose ( fd ) ;
    }

    delete [] fname ;
  }

  void unGetChar ( int c )
  {
    if ( ungetSP < MAX_UNGET_CHAR-1 )
      ungetStack [ ungetSP++ ] = c ;
    else
      fprintf ( stderr, "PSL: ERROR - Too many ungotten characters?!?" ) ;

    if ( c == '\n' ) line_no-- ;
  }

  int getChar ()
  {
    int c ;

    /* Get a new character - either from the 'undo' stack or from the file.  */

    if ( ungetSP > 0 )
      c = ungetStack [ --ungetSP ] ;
    else
      c = getc ( fd ) ;

    if ( c == '\n' ) line_no++ ;

    return c ;
  }

  int   getLineNo () const { return line_no ; }
  char *getFname  () const { return fname   ; }
} ;


char *_pslGetFname ()
        { return defaultFileStack [ fileStackPointer - 1 ] -> getFname () ; } 
int  _pslGetLineNo ()
        { return defaultFileStack [ fileStackPointer - 1 ] -> getLineNo () ; } 

void _pslPopDefaultFile  () { delete defaultFileStack [ fileStackPointer - 1 ] ; } 
void _pslPushDefaultFile ( FILE *fd, const char *fname ) { new pslFile ( fd, fname ) ; }

void _pslPushDefaultFile ( const char *fname )
{
  FILE *fd = fopen ( fname, "ra" ) ;

  if ( fd == NULL )
  {
    fprintf ( stderr, "PSL: ERROR - Can't open #include'ed file '%s'", fname ) ;
    return ;
  }

  _pslPushDefaultFile ( fd, fname ) ;
}



int _pslGetChar ()
{
  /* WARNING -- RECURSIVE -- WARNING -- RECURSIVE -- WARNING -- RECURSIVE */

  int c = defaultFileStack [ fileStackPointer-1 ] -> getChar () ;

  /*
    If we hit the end of the current file, then if this is an
    included file, just pop the file stack and carry on reading
    from the place we left off.  Otherwise just return an EOF.
  */

  if ( c < 0 )   /* EOF */
  {
    if ( fileStackPointer > 1 )
    {
      _pslPopDefaultFile () ;
      return _pslGetChar () ;
    }

    return -1 ;
  }

  return c ;
}


void _pslUnGetChar ( int c )
{
  defaultFileStack [ fileStackPointer-1 ] -> unGetChar ( c ) ;
}

