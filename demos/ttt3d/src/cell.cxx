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


#include "p3d.h"
#include <plib/ssgAux.h>

static ssgBranch *wire_cube = NULL ;
static ssgBranch *   X_cube = NULL ;
static ssgBranch *   O_cube = NULL ;


ssgTransform *makeGround ()
{
  ssgTransform *ground = new ssgTransform () ;

  sgVec4 *colour = new sgVec4 [ 1 ] ;
  sgVec3 *normal = new sgVec3 [ 1 ] ;
  sgVec2 *tcoord = new sgVec2 [ 4 ] ;
  sgVec3 *scoord = new sgVec3 [ 4 ] ;

  sgSetVec4 ( colour[0], 1, 1, 1, 1 ) ;

  sgSetVec2 ( tcoord [ 0 ],  0,  0 ) ;
  sgSetVec2 ( tcoord [ 1 ], 25,  0 ) ;
  sgSetVec2 ( tcoord [ 2 ], 25, 25 ) ;
  sgSetVec2 ( tcoord [ 3 ],  0, 25 ) ;

  sgSetVec3 ( normal [ 0 ], 0, 0, 1 ) ;

  sgSetVec3 ( scoord [ 0 ], -60, -60, -5.6 ) ;
  sgSetVec3 ( scoord [ 1 ],  60, -60, -5.6 ) ;
  sgSetVec3 ( scoord [ 2 ],  60,  60, -5.6 ) ;
  sgSetVec3 ( scoord [ 3 ], -60,  60, -5.6 ) ;

  ssgVTable *vt1 =  new ssgVTable ( (GLenum) GL_TRIANGLE_FAN,
                               4, scoord,
                               1, normal,
                               4, tcoord,
                               1, colour ) ;
  
  vt1 -> setState ( ground_gst ) ; ground -> addKid ( vt1 ) ;

  colour = new sgVec4 [ 1 ] ;
  normal = new sgVec3 [ 1 ] ;
  tcoord = new sgVec2 [ 6 ] ;
  scoord = new sgVec3 [ 6 ] ;

  sgSetVec4 ( colour[0], 1, 1, 1, 1 ) ;

  sgSetVec2 ( tcoord [ 0 ], 0, 0 ) ;
  sgSetVec2 ( tcoord [ 1 ], 1, 0 ) ;
  sgSetVec2 ( tcoord [ 2 ], 1, 1 ) ;
  sgSetVec2 ( tcoord [ 3 ], 0, 1 ) ;
  sgSetVec2 ( tcoord [ 4 ], 1, 1 ) ;
  sgSetVec2 ( tcoord [ 5 ], 1, 0 ) ;

  sgSetVec3 ( normal [ 0 ], 0, 0, 1 ) ;

  sgSetVec3 ( scoord [ 0 ], -2, -2, -2.5 ) ;
  sgSetVec3 ( scoord [ 1 ],  1, -2, -2.5 ) ;
  sgSetVec3 ( scoord [ 2 ],  1,  1, -2.5 ) ;
  sgSetVec3 ( scoord [ 3 ], -2,  1, -2.5 ) ;
  sgSetVec3 ( scoord [ 4 ],  1,  1, -2.5 ) ;
  sgSetVec3 ( scoord [ 5 ],  1, -2, -2.5 ) ;

  vt1 =  new ssgVTable ( (GLenum) GL_TRIANGLE_FAN,
                               6, scoord,
                               1, normal,
                               6, tcoord,
                               1, colour ) ;
  
  vt1 -> setState ( ctrls_gst ) ; ground -> addKid ( vt1 ) ;

  return ground ;
} ;

ssgTransform *makeBlueWireCube()
{
  ssgTransform *sel_cube = new ssgTransform () ;

  sgVec4 *colour = new sgVec4 [ 1 ] ;
  sgVec3 *normal = new sgVec3 [ 1 ] ;
  sgVec2 *tcoord = new sgVec2 [ 1 ] ;
  sgVec3 *scoord = new sgVec3 [ 8 ] ;

  sgSetVec4 ( colour[0], 1, 0, 1, 1 ) ;
  sgSetVec3 ( normal[0], 1, 0, 0 ) ;
  sgSetVec2 ( tcoord[0], 0, 0 ) ;

  sgSetVec3 ( scoord [ 0 ], -0.5, -0.5, -0.5 ) ;
  sgSetVec3 ( scoord [ 1 ], -0.5, -0.5,  0.5 ) ;
  sgSetVec3 ( scoord [ 2 ],  0.5, -0.5,  0.5 ) ;
  sgSetVec3 ( scoord [ 3 ],  0.5, -0.5, -0.5 ) ;

  sgSetVec3 ( scoord [ 4 ], -0.5,  0.5, -0.5 ) ;
  sgSetVec3 ( scoord [ 5 ], -0.5,  0.5,  0.5 ) ;
  sgSetVec3 ( scoord [ 6 ],  0.5,  0.5,  0.5 ) ;
  sgSetVec3 ( scoord [ 7 ],  0.5,  0.5, -0.5 ) ;

  unsigned short *sindex = new unsigned short [ 7 ] ;
  unsigned short *cindex = new unsigned short [ 1 ] ;
  unsigned short *nindex = new unsigned short [ 1 ] ;
  unsigned short *tindex = new unsigned short [ 1 ] ;

  sindex [ 0 ] = 0 ;
  sindex [ 1 ] = 1 ;
  sindex [ 2 ] = 2 ;
  sindex [ 3 ] = 6 ;
  sindex [ 4 ] = 7 ;
  sindex [ 5 ] = 3 ;
  sindex [ 6 ] = 2 ;
  
  cindex [ 0 ] = 0 ;
  nindex [ 0 ] = 0 ;
  tindex [ 0 ] = 0 ;

  ssgVTable *vt1 =  new ssgVTable ( (GLenum) GL_LINE_STRIP,
                                7, sindex, scoord,
                                1, nindex, normal,
                                1, tindex, tcoord,
                                1, cindex, colour ) ;
  
  sindex = new unsigned short [ 5 ] ;
  
  sindex [ 0 ] = 3 ;
  sindex [ 1 ] = 0 ;
  sindex [ 2 ] = 4 ;
  sindex [ 3 ] = 5 ;
  sindex [ 4 ] = 1 ;

  ssgVTable *vt2 =  new ssgVTable ( (GLenum) GL_LINE_STRIP,
                                5, sindex, scoord,
                                1, nindex, normal,
                                1, tindex, tcoord,
                                1, cindex, colour ) ;

  sindex = new unsigned short [ 2 ] ;
  
  sindex [ 0 ] = 5 ;
  sindex [ 1 ] = 6 ;

  ssgVTable *vt3 =  new ssgVTable ( (GLenum) GL_LINE_STRIP,
                                2, sindex, scoord,
                                1, nindex, normal,
                                1, tindex, tcoord,
                                1, cindex, colour ) ;

  sindex = new unsigned short [ 2 ] ;
  
  sindex [ 0 ] = 4 ;
  sindex [ 1 ] = 7 ;

  ssgVTable *vt4 =  new ssgVTable ( (GLenum) GL_LINE_STRIP,
                                2, sindex, scoord,
                                1, nindex, normal,
                                1, tindex, tcoord,
                                1, cindex, colour ) ;

  vt1 -> setState ( default_gst ) ; sel_cube -> addKid ( vt1 ) ;
  vt2 -> setState ( default_gst ) ; sel_cube -> addKid ( vt2 ) ;
  vt3 -> setState ( default_gst ) ; sel_cube -> addKid ( vt3 ) ;
  vt4 -> setState ( default_gst ) ; sel_cube -> addKid ( vt4 ) ;

  return sel_cube ;
}


static void makeWireCube()
{
  wire_cube = new ssgBranch () ;

  sgVec4 *colour = new sgVec4 [ 1 ] ;
  sgVec3 *normal = new sgVec3 [ 1 ] ;
  sgVec2 *tcoord = new sgVec2 [ 1 ] ;
  sgVec3 *scoord = new sgVec3 [ 4 ] ;

  sgSetVec4 ( colour[0], 1.0, 1.0, 0.0, 0.7f ) ;
  sgSetVec3 ( normal[0], 1, 0, 0 ) ;
  sgSetVec2 ( tcoord[0], 0, 0 ) ;

  sgSetVec3 ( scoord [ 0 ], -0.5, -0.5, -0.5 ) ;
  sgSetVec3 ( scoord [ 1 ], -0.5,  0.5, -0.5 ) ;
  sgSetVec3 ( scoord [ 2 ],  0.5,  0.5, -0.5 ) ;
  sgSetVec3 ( scoord [ 3 ],  0.5, -0.5, -0.5 ) ;

  unsigned short *sindex = new unsigned short [ 4 ] ;
  unsigned short *cindex = new unsigned short [ 1 ] ;
  unsigned short *nindex = new unsigned short [ 1 ] ;
  unsigned short *tindex = new unsigned short [ 1 ] ;

  sindex [ 0 ] = 0 ;
  sindex [ 1 ] = 1 ;
  sindex [ 2 ] = 2 ;
  sindex [ 3 ] = 3 ;
  
  cindex [ 0 ] = 0 ;
  nindex [ 0 ] = 0 ;
  tindex [ 0 ] = 0 ;

  ssgVTable *vt = new ssgVTable ( (GLenum) GL_LINE_LOOP,
                  4, sindex, scoord, 1, nindex, normal,
                  1, tindex, tcoord, 1, cindex, colour ) ;
  
  vt -> setState ( default_gst ) ;
  wire_cube -> addKid ( vt ) ;
}


static ssgBranch *X ()
{
  ssgaCube *XXX = new ssgaCube () ;

  XXX -> setSize ( 0.5 ) ;
  XXX -> setKidState ( X_gst ) ;

  return XXX ;
}


static ssgBranch *O ()
{
  ssgaSphere *OOO = new ssgaSphere ( 300 ) ;

  OOO -> setSize ( 0.7 ) ;
  OOO -> setKidState ( O_gst ) ;

  return OOO ;
}


static void makeXcube ()
{
  X_cube = new ssgBranch () ;
  X_cube -> addKid ( X () ) ;
}


static void makeOcube ()
{
  O_cube = new ssgBranch () ;
  O_cube -> addKid ( O () ) ;
}


Cell::Cell ( int x, int y, int z, int size )
{
  if ( wire_cube == NULL ) makeWireCube() ;
  if (    X_cube == NULL ) makeXcube () ;
  if (    O_cube == NULL ) makeOcube () ;

  sgSetCoord ( &c, (float)x - (float) size / 2.0f + 0.5f,
		   (float)y - (float) size / 2.0f + 0.5f,
		   (float)z - (float) size / 2.0f + 0.5f, 0,0,0 ) ;

  posn = new ssgTransform ( & c ) ;
  cell = new ssgSelector  () ;

  posn -> addKid ( cell ) ;

  /* Don't change the order of these without changing the defines */

  cell -> addKid ( wire_cube ) ;
  cell -> addKid (    X_cube ) ;
  cell -> addKid (    O_cube ) ;

  what = WIRE_CELL ;
  cell -> selectStep ( what ) ;
}


