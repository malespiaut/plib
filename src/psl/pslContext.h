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

  pslVariable   variable [ MAX_VARIABLE ] ;
  pslVariable   stack    [ MAX_STACK    ] ; 
  int            sp ;
  pslAddress    pc ;

public:

  pslContext ( pslProgram *p )
  {
    code       = p -> getCode       () ;
    extensions = p -> getExtensions () ;
    program    = p ;
    reset () ;
  }

  ~pslContext () {} ;

  void pushInt      ( int          x ) { stack [ sp++ ] . i = x ; }
  void pushFloat    ( float        x ) { stack [ sp++ ] . f = x ; }
  void pushVariable ( pslVariable x ) { stack [ sp++ ]     = x ; }

  void         popVoid     () {                --sp       ; }
  int          popInt      () { return stack [ --sp ] . i ; }
  float        popFloat    () { return stack [ --sp ] . f ; }
  pslVariable popVariable () { return stack [ --sp ]     ; }

  pslResult step () ;

  void reset ()
  {
    memset ( variable, 0, MAX_VARIABLE * sizeof ( pslVariable ) ) ;
    sp = 0 ;
    pc = 0 ;
  }
} ;


