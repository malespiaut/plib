/*
     This file is part of TTT3D - Steve's 3D TicTacToe Player.
     Copyright (C) 2001  Steve Baker

     TTT3D is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published by
     the Free Software Foundation; either version 2 of the License, or
     (at your option) any later version.

     TTT3D is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU General Public License for more details.

     You should have received a copy of the GNU General Public License
     along with TTT3D; if not, write to the Free Software
     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA

     For further information visit http://plib.sourceforge.net

     $Id$
*/


ssgTransform *makeBlueWireCube() ;
ssgTransform *makeGround() ;

#define WIRE_CELL 0
#define    X_CELL 1
#define    O_CELL 2

class Cell
{
  sgCoord c ;
  ssgTransform *posn ;
  ssgSelector  *cell ;
  int what ;

public:

  Cell ( int x, int y, int z, int size ) ;

  ssgBranch *getSSG () { return posn ; }

  int  get () { return what ; }

  void set ( int n )
  {
    what = n ;
    cell -> selectStep ( n ) ;
  }

} ;


