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


#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "game.h"

static char board [4][4][4] ;
static int last_x = -1 ;
static int last_y = -1 ;
static int last_z = -1 ;

#define NUM_ROWS  ( 3 * 16 + 3 * 8 + 4 )

static char *row       [ NUM_ROWS ][ 4 ] ;
static char  rowptr    [ 4 ][ 4 ][ 4 ][ 8 ] ;
static char  rowptrctr [ 4 ][ 4 ][ 4 ] ;


static void setupRowArray ()
{
  int x, y, z;

  for ( x = 0 ; x < 4 ; x++ )
    for ( y = 0 ; y < 4 ; y++ )
      for ( z = 0 ; z < 4 ; z++ )
      {
        rowptrctr [ x ][ y ][ z ] = 0 ;

        for ( int i = 0 ; i < 8 ; i++ )
          rowptr [ x ][ y ][ z ][ i ] = 127 ;
      }

  int r = 0 ;

  /* 16 Z axis rows */

  for ( x = 0 ; x < 4 ; x++ )
    for ( y = 0 ; y < 4 ; y++ )
    {
      for ( z = 0 ; z < 4 ; z++ )
      {
        row [ r ][ z ] = & board [ x ][ y ][ z ] ;
        rowptr [ x ][ y ][ z ][ rowptrctr[x][y][z]++ ] = r ;
      }
      r++ ;
    }

  /* 16 Y axis rows */

  for ( x = 0 ; x < 4 ; x++ )
    for ( z = 0 ; z < 4 ; z++ )
    {
      for ( y = 0 ; y < 4 ; y++ )
      {
        row [ r ][ y ] = & board [ x ][ y ][ z ] ;
        rowptr [ x ][ y ][ z ][ rowptrctr[x][y][z]++ ] = r ;
      }

      r++ ;
    }

  /* 16 Y axis rows */

  for ( z = 0 ; z < 4 ; z++ )
    for ( y = 0 ; y < 4 ; y++ )
    {
      for ( x = 0 ; x < 4 ; x++ )
      {
        row [ r ][ x ] = & board [ x ][ y ][ z ] ;
        rowptr [ x ][ y ][ z ][ rowptrctr[x][y][z]++ ] = r ;
      }

      r++ ;
    }

  /* 8 xy plane diagonals */

  for ( z = 0 ; z < 4 ; z++ )
  {
    int xy;

    for ( xy = 0 ; xy < 4 ; xy++ )
    {
      row [ r ][ xy ] = & board [ xy ][ xy ][ z ] ;
      rowptr [ xy ][ xy ][ z ][ rowptrctr[xy][xy][z]++ ] = r ;
    }

    r++ ;

    for ( xy = 0 ; xy < 4 ; xy++ )
    {
      row [ r ][ xy ] = & board [ xy ][ 3 - xy ][ z ] ;
      rowptr [ xy ][3-xy][ z ][ rowptrctr[xy][3-xy][z]++ ] = r ;
    }

    r++ ;
  }

  /* 8 xz plane diagonals */

  for ( y = 0 ; y < 4 ; y++ )
  {
    int xz;

    for ( xz = 0 ; xz < 4 ; xz++ )
    {
      row [ r ][ xz ] = & board [ xz ][ y ][ xz ] ;
      rowptr [ xz ][ y ][ xz ][ rowptrctr[xz][y][xz]++ ] = r ;
    }

    r++ ;

    for ( xz = 0 ; xz < 4 ; xz++ )
    {
      row [ r ][ xz ] = & board [ xz ][ y ][ 3 - xz ] ;
      rowptr [ xz ][ y ][3-xz][ rowptrctr[xz][y][3-xz]++ ] = r ;
    }

    r++ ;
  }

  /* 8 yz plane diagonals */

  for ( x = 0 ; x < 4 ; x++ )
  {
    int yz;
    for ( yz = 0 ; yz < 4 ; yz++ )
    {
      row [ r ][ yz ] = & board [ x ][ yz ][ yz ] ;
      rowptr [ x ][ yz ][ yz ][ rowptrctr[x][yz][yz]++ ] = r ;
    }

    r++ ;

    for ( yz = 0 ; yz < 4 ; yz++ )
    {
      row [ r ][ yz ] = & board [ x ][ yz ][ 3 - yz ] ;
      rowptr [ x ][ yz ][3-yz][ rowptrctr[x][yz][3-yz]++ ] = r ;
    }

    r++ ;
  }

  /* 4 grand diagonals. */
  
  int xyz;
  for ( xyz = 0 ; xyz < 4 ; xyz++ )
  {
    row [ r ][ xyz ] = & board [ xyz ][ xyz ][ xyz ] ;
    rowptr [ xyz ][ xyz ][ xyz ][ rowptrctr[xyz][xyz][xyz]++ ] = r ;
  }

  r++ ;

  for ( xyz = 0 ; xyz < 4 ; xyz++ )
  {
    row [ r ][ xyz ] = & board [ xyz ][ xyz ][ 3-xyz ] ;
    rowptr [ xyz ][ xyz ][ 3-xyz ][ rowptrctr[xyz][xyz][3-xyz]++ ] = r ;
  }

  r++ ;

  for ( xyz = 0 ; xyz < 4 ; xyz++ )
  {
    row [ r ][ xyz ] = & board [ xyz ][ 3-xyz ][ xyz ] ;
    rowptr [ xyz ][ 3-xyz ][ xyz ][ rowptrctr[xyz][3-xyz][xyz]++ ] = r ;
  }

  r++ ;

  for ( xyz = 0 ; xyz < 4 ; xyz++ )
  {
    row [ r ][ xyz ] = & board [ xyz ][ 3-xyz ][ 3-xyz ] ;
    rowptr [ xyz ][ 3-xyz ][ 3-xyz ][ rowptrctr[xyz][3-xyz][3-xyz]++ ] = r ;
  }

  r++ ;

  if ( r != NUM_ROWS )
  {
    fprintf ( stderr, "%d rows?!?\n", r ) ;
    exit ( 1 ) ;
  }
}


static void clearBoard ()
{
  for ( int x = 0 ; x < 4 ; x++ )
    for ( int y = 0 ; y < 4 ; y++ )
      for ( int z = 0 ; z < 4 ; z++ )
        board [ x ][ y ][ z ] = ' ' ;
}


/*void printBoard ( int winrow )
{
  int x, y, z;
	
  for ( int i = 0 ; i < 50 ; i++ )
    printf ( "\n" ) ;

  printf ( "                         ~~~ 3D TicTacToe ~~~\n" ) ;
  printf ( "                            by Steve Baker\n");
  printf ( "\n" ) ;
  printf ( "\n" ) ;

  for ( y = 0 ; y < 4 ; y++ )
  {
    for ( z = 0 ; z < 4 ; z++ )
    {
      for ( x = 0 ; x < 4 ; x++ )
      {
        printf ( "   " ) ;

        if ( x != 3 )
          printf ( "|" ) ;
      }
      printf ( "   " ) ;
    }
    printf ( "\n" ) ;

    for ( z = 0 ; z < 4 ; z++ )
    {
      for ( x = 0 ; x < 4 ; x++ )
      {
        int q ;

        for ( q = 0 ; q < 4 ; q++ )
          if ( row[winrow][q] == & board[x][y][z] )
            break ;

        if ( ( x == last_x && y == last_y && z == last_z ) ||
             q < 4 )
          printf ( "*%c*", board [ x ][ y ][ z ] ) ;
        else
          printf ( " %c ", board [ x ][ y ][ z ] ) ;

        if ( x != 3 )
          printf ( "|" ) ;
      }
      printf ( "   " ) ;
    }
    printf ( "\n" ) ;

    for ( z = 0 ; z < 4 ; z++ )
    {
      for ( x = 0 ; x < 4 ; x++ )
      {
        if ( board [ x ][ y ][ z ] == ' ' )
        {
	  if ( y != 3 )
	  {
	    if ( x+y*4+z*16+1 < 10 )
	      printf ( "_%d_", x + y * 4 + z * 16 + 1 ) ;
	    else
	      printf ( "%2d_", x + y * 4 + z * 16 + 1 ) ;
	  }
	  else
	    printf ( "%2d ", x + y * 4 + z * 16 + 1 ) ;
        }
	else
	if ( y != 3 )
	  printf ( "___" ) ;
        else
	  printf ( "   " ) ;

        if ( x != 3 )
          printf ( "|" ) ;
      }
      printf ( "   " ) ;
    }
    printf ( "\n" ) ;
  }

  printf ( "\n" ) ;
  printf ( "\n" ) ;
}*/



static int value [4][4] =  /* Value of 1000000 means play here to win. */
{
  {    1, 3,10, 1000000 },   /* No    enemy, 0,1,2,3 of ours */
  {    2, 0, 0,       0 },   /* One   enemy, 0,1,2,3 of ours */
  {   20, 0, 0,       0 },   /* Two   enemy, 0,1,2,3 of ours */
  { 1000, 0, 0,       0 },   /* Three enemy, 0,1,2,3 of ours */
} ;



static int win = STILL_PLAYING ;


static int countRow ( int r )
{
  int x = 0 ;
  int o = 0 ;

  for ( int i = 0 ; i < 4 ; i++ )
  {
    if ( *(row[r][i]) == 'X' ) x++ ;
    if ( *(row[r][i]) == 'O' ) o++ ;
  }

  if ( x == 4 ) { /* printBoard (r) ; */
                  printf ( "** Human wins! **\n\n" ) ;
                  win = HUMAN_WIN ;
                  return 0 ; } 

  if ( o == 4 ) { /* printBoard (r) ; */
                  printf ( "** Computer wins! **\n\n" ) ;
                  win = COMPUTER_WIN ;
                  return 0 ; } 

  return value [ x ][ o ] ;
}



/*int getLoc ( char *msg )
{

  while ( 1 )
  {
    int res = -1 ;
    char s [ 21 ] ;

    printf ( "%s\n[1..64]: ", msg ) ;
    fgets ( s, 20, stdin ) ;

    char *p = s ;

    while ( *p == ' ' || *p == '\t' ) p++ ;

    if ( isdigit ( *p ) )
    {
      res = atoi ( p ) - 1 ;

      if ( res >= 0 && res <= 63 && board[res%4][(res%16)/4][res/16] == ' ' )
        return res ;

      if ( res >= 0 && res <= 63 )
	printf ( "You can't play there - it's already taken!\n" ) ;
      else
	printf ( "Please enter a number in the range 0 to 63.\n" ) ;
    }
    else
      printf ( "Please type a NUMBER in the range 0 to 63.\n" ) ;
  }
}*/



void game_init ()
{
  win = STILL_PLAYING ;
  setupRowArray () ;
  clearBoard () ;
}


char get_board_entry ( int x, int y, int z )
{
  return board[x][y][z] ;
}


int game_update ( int x, int y, int z )
{
  int q;

  if ( win != STILL_PLAYING )
    game_init () ;

  /* printBoard (-1) ; */

  board[x][y][z] = 'X' ;

  int rc = rowptrctr[x][y][z] ;

  for ( q = 0 ; q < rc ; q++ )
  {
    int r = rowptr[x][y][z][q] ;

    countRow ( r ) ;  /* Just to check for a win! */

    if ( win != STILL_PLAYING )
      return win ;
  }

  int best     = -1 ;
  int best_sum =  0 ;

  for ( int p = 0 ; p < 64 ; p++ )
  {
    x = p % 4 ;
    y = (p % 16)/4 ;
    z = p/16 ;

    if ( board[x][y][z] != ' ' )
      continue ;

    rc = rowptrctr[x][y][z] ;

    int sum = 0 ;

    for ( int q = 0 ; q < rc ; q++ )
    {
      int r = rowptr[x][y][z][q] ;

      sum += countRow ( r ) ;  /* Just to check for a win! */

      if ( win != STILL_PLAYING )
      {
        board[x][y][z] = 'O' ;
        return win ;
      }
    }

    if ( sum >= best_sum )
    {
      best = p ;
      best_sum = sum ;
    }
  }

  if ( best == -1 )
  {
    /* printBoard (-1) ; */
    printf ( "** It's a draw! **\n\n" ) ;
    return DRAWN_GAME ;
  }

  x = best % 4 ;
  y = (best % 16)/4 ;
  z = best/16 ;

  board[x][y][z] = 'O' ;

  last_x = x ;
  last_y = y ;
  last_z = z ;

  rc = rowptrctr[x][y][z] ;

  for ( q = 0 ; q < rc ; q++ )
  {
    int r = rowptr[x][y][z][q] ;

    countRow ( r ) ;  /* Just to check for a win! */

    if ( win != STILL_PLAYING )
      return win ;
  }

  return STILL_PLAYING ;
}


