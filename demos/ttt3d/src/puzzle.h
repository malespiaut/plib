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


#define MAX_GRID     4


class Puzzle 
{
  sgCoord pos ;

  int sel  [ 3 ] ;

  ssgTransform *spin ;
  ssgTransform *selector ;
  ssgTransform *ground ;

  Cell *cell [ MAX_GRID ][ MAX_GRID ][ MAX_GRID ] ;

  int game_state ;

public:
   Puzzle () ;
  ~Puzzle () ;

  void reset () ;
  void update () ;

  int  getGameState () { return game_state ; }
  void setGameState ( int gs ) { game_state = gs ; }

  int gameOver () { return game_state != STILL_PLAYING ; }

  void cursor ( int x, int y, int z )
  {
    sel[0] = x & 3 ;
    sel[1] = y & 3 ;
    sel[2] = z & 3 ;
  }

  int getCx () { return sel[0] ; }
  int getCy () { return sel[1] ; }
  int getCz () { return sel[2] ; }

  void step_up    () { pos . xyz [ 1 ] += 0.5f ; spin->setTransform(&pos); }
  void step_down  () { pos . xyz [ 1 ] -= 0.5f ; spin->setTransform(&pos); }
  void spin_up    () { pos . hpr [ 2 ] += 5.0f ; spin->setTransform(&pos); }
  void spin_down  () { pos . hpr [ 2 ] -= 5.0f ; spin->setTransform(&pos); }
  void spin_left  () { pos . hpr [ 0 ] -= 5.0f ; spin->setTransform(&pos); }
  void spin_right () { pos . hpr [ 0 ] += 5.0f ; spin->setTransform(&pos); }

  void cursor_up    () { cursor(sel[0],sel[1],sel[2]+1) ; }
  void cursor_down  () { cursor(sel[0],sel[1],sel[2]-1) ; }
  void cursor_in    () { cursor(sel[0],sel[1]+1,sel[2]) ; }
  void cursor_out   () { cursor(sel[0],sel[1]-1,sel[2]) ; }
  void cursor_left  () { cursor(sel[0]-1,sel[1],sel[2]) ; }
  void cursor_right () { cursor(sel[0]+1,sel[1],sel[2]) ; }

  void put_empty() { cell[sel[0]][sel[1]][sel[2]]->set(WIRE_CELL) ; }
  void put_X    () { cell[sel[0]][sel[1]][sel[2]]->set(   X_CELL) ; }
  void put_O    () { cell[sel[0]][sel[1]][sel[2]]->set(   O_CELL) ; }

  void put_empty ( int x, int y, int z ) { cell[x][y][z] -> set (WIRE_CELL);}
  void put_X     ( int x, int y, int z ) { cell[x][y][z] -> set ( X_CELL) ; }
  void put_O     ( int x, int y, int z ) { cell[x][y][z] -> set ( O_CELL) ; }

  ssgTransform *getSSG () { return spin ; }
} ;


