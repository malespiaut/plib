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


class pslContext
{
  pslOpcode    *code       ;
  pslExtension *extensions ;
  pslProgram   *program    ;

  pslVariable  variable [ MAX_VARIABLE ] ;
  pslValue     stack    [ MAX_STACK    ] ; 
  int          sp ;
  pslAddress   pc ;

public:

  pslContext ( pslProgram *p )
  {
    code       = p -> getCode       () ;
    extensions = p -> getExtensions () ;
    program    = p ;

    reset () ;
  }

  ~pslContext () {} ;

  void pushInt    ( int              x ) { stack [ sp++ ] . set ( x ) ; }
  void pushFloat  ( float            x ) { stack [ sp++ ] . set ( x ) ; }
  void pushString ( const char      *x ) { stack [ sp++ ] . set ( x ) ; }
  void pushNumber ( const pslNumber *x ) { stack [ sp++ ] . set ( x ) ; }

  void   popVoid    () {                --sp                  ; }
  int    popInt     () { return stack [ --sp ] . getInt    () ; }
  float  popFloat   () { return stack [ --sp ] . getFloat  () ; }
  char  *popString  () { return stack [ --sp ] . getString () ; }

  void   popNumber  ( pslNumber *v ) { v -> set ( & stack [ --sp ] ) ; }

  pslResult step  () ;

  pslResult trace ()
  {
    program -> getCompiler () -> printInstruction ( stdout, pc ) ;
    fflush ( stdout ) ;
    return step () ;
  }

  void reset ()
  {
    for ( int i = 0 ; i < MAX_VARIABLE ; i++ )
      variable [ i ] . reset () ;

    sp = 0 ;
    pc = 0 ;
  }
} ;


