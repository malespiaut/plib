/*
     PLIB - A Suite of Portable Game Libraries
     Copyright (C) 2001  Steve Baker
 
     This library is free software; you can redistribute it and/or
     modify it under the terms of the GNU Library General Public
     License as published by the Free Software Foundation; either
     version 2 of the License, or (at your option) any later version.
 
     This library is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
     Library General Public License for more details.
 
     You should have received a copy of the GNU Library General Public
     License along with this library; if not, write to the Free
     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 
     For further information visit http://plib.sourceforge.net

     $Id$
*/


#include "ssgAux.h"
#include <string.h>

static int patch [][ 17 ] =
{ 
  /* Rim: */
    { 2, 102, 103, 104, 105,
        4,   5,   6,   7,
        8,   9,  10,  11,
       12,  13,  14,  15 },
  /* Body: */
    {  2, 12,  13,  14,  15,
       16,  17,  18,  19,
       20,  21,  22,  23,
       24,  25,  26,  27 },
    {  2, 24,  25,  26,  27,
       29,  30,  31,  32,
       33,  34,  35,  36,
       37,  38,  39,  40 },
  /* Lid: */
    {  2, 96,  96,  96,  96,
       97,  98,  99, 100,
      101, 101, 101, 101,
        0,   1,   2,   3 },
    {  2,  0,   1,   2,   3,
      106, 107, 108, 109,
      110, 111, 112, 113,
      114, 115, 116, 117 },
  /* Handle: */
    {  1, 41,  42,  43,  44,
       45,  46,  47,  48,
       49,  50,  51,  52,
       53,  54,  55,  56 },
    {  1, 53,  54,  55,  56,
       57,  58,  59,  60,
       61,  62,  63,  64,
       28,  65,  66,  67 },
  /* Spout: */
    {  1, 68,  69,  70,  71,
       72,  73,  74,  75,
       76,  77,  78,  79,
       80,  81,  82,  83 },
    {  1, 80,  81,  82,  83,
       84,  85,  86,  87,
       88,  89,  90,  91,
       92,  93,  94,  95 },
  /* Bottom: */
    { 2, 118, 118, 118, 118,
      124, 122, 119, 121,
      123, 126, 125, 120,
       40,  39,  38,  37 },

  /* End Marker: */
    { -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1 }
} ; 
 
static sgVec3 vertex [] =
{
  {  0.2000,  0.0000, 2.70000 },  /* 00 */
  {  0.2000, -0.1120, 2.70000 },
  {  0.1120, -0.2000, 2.70000 },
  {  0.0000, -0.2000, 2.70000 },
  {  1.3375,  0.0000, 2.53125 },
  {  1.3375, -0.7490, 2.53125 },
  {  0.7490, -1.3375, 2.53125 },
  {  0.0000, -1.3375, 2.53125 },
  {  1.4375,  0.0000, 2.53125 },
  {  1.4375, -0.8050, 2.53125 },

  {  0.8050, -1.4375, 2.53125 },  /* 10 */
  {  0.0000, -1.4375, 2.53125 },
  {  1.5000,  0.0000, 2.40000 },
  {  1.5000, -0.8400, 2.40000 },
  {  0.8400, -1.5000, 2.40000 },
  {  0.0000, -1.5000, 2.40000 },
  {  1.7500,  0.0000, 1.87500 },
  {  1.7500, -0.9800, 1.87500 },
  {  0.9800, -1.7500, 1.87500 },
  {  0.0000, -1.7500, 1.87500 },

  {  2.0000,  0.0000, 1.35000 },  /* 20 */
  {  2.0000, -1.1200, 1.35000 },
  {  1.1200, -2.0000, 1.35000 },
  {  0.0000, -2.0000, 1.35000 },
  {  2.0000,  0.0000, 0.90000 },
  {  2.0000, -1.1200, 0.90000 },
  {  1.1200, -2.0000, 0.90000 },
  {  0.0000, -2.0000, 0.90000 },
  { -2.0000,  0.0000, 0.90000 },
  {  2.0000,  0.0000, 0.45000 },
  
  {  2.0000, -1.1200, 0.45000 },  /* 30 */
  {  1.1200, -2.0000, 0.45000 },
  {  0.0000, -2.0000, 0.45000 },
  {  1.5000,  0.0000, 0.22500 },
  {  1.5000, -0.8400, 0.22500 },
  {  0.8400, -1.5000, 0.22500 },
  {  0.0000, -1.5000, 0.22500 },
  {  1.5000,  0.0000, 0.15000 },
  {  1.5000, -0.8400, 0.15000 },
  {  0.8400, -1.5000, 0.15000 },
  
  {  0.0000, -1.5000, 0.15000 },  /* 40 */
  { -1.6000,  0.0000, 2.02500 },
  { -1.6000, -0.3000, 2.02500 },
  { -1.5000, -0.3000, 2.25000 },
  { -1.5000,  0.0000, 2.25000 },
  { -2.3000,  0.0000, 2.02500 },
  { -2.3000, -0.3000, 2.02500 },
  { -2.5000, -0.3000, 2.25000 },
  { -2.5000,  0.0000, 2.25000 },
  { -2.7000,  0.0000, 2.02500 },

  { -2.7000, -0.3000, 2.02500 },  /* 50 */
  { -3.0000, -0.3000, 2.25000 },
  { -3.0000,  0.0000, 2.25000 },
  { -2.7000,  0.0000, 1.80000 },
  { -2.7000, -0.3000, 1.80000 },
  { -3.0000, -0.3000, 1.80000 },
  { -3.0000,  0.0000, 1.80000 },
  { -2.7000,  0.0000, 1.57500 },
  { -2.7000, -0.3000, 1.57500 },
  { -3.0000, -0.3000, 1.35000 },
  
  { -3.0000,  0.0000, 1.35000 },  /* 60 */
  { -2.5000,  0.0000, 1.12500 },
  { -2.5000, -0.3000, 1.12500 },
  { -2.6500, -0.3000, 0.93750 },
  { -2.6500,  0.0000, 0.93750 },
  { -2.0000, -0.3000, 0.90000 },
  { -1.9000, -0.3000, 0.60000 },
  { -1.9000,  0.0000, 0.60000 },
  {  1.7000,  0.0000, 1.42500 },
  {  1.7000, -0.6600, 1.42500 },

  {  1.7000, -0.6600, 0.60000 },  /* 70 */
  {  1.7000,  0.0000, 0.60000 },
  {  2.6000,  0.0000, 1.42500 },
  {  2.6000, -0.6600, 1.42500 },
  {  3.1000, -0.6600, 0.82500 },
  {  3.1000,  0.0000, 0.82500 },
  {  2.3000,  0.0000, 2.10000 },
  {  2.3000, -0.2500, 2.10000 },
  {  2.4000, -0.2500, 2.02500 },
  {  2.4000,  0.0000, 2.02500 },

  {  2.7000,  0.0000, 2.40000 },  /* 80 */
  {  2.7000, -0.2500, 2.40000 },
  {  3.3000, -0.2500, 2.40000 },
  {  3.3000,  0.0000, 2.40000 },
  {  2.8000,  0.0000, 2.47500 },
  {  2.8000, -0.2500, 2.47500 },
  {  3.5250, -0.2500, 2.49375 },
  {  3.5250,  0.0000, 2.49375 },
  {  2.9000,  0.0000, 2.47500 },
  {  2.9000, -0.1500, 2.47500 },
  
  {  3.4500, -0.1500, 2.51250 },  /* 90 */
  {  3.4500,  0.0000, 2.51250 },
  {  2.8000,  0.0000, 2.40000 },
  {  2.8000, -0.1500, 2.40000 },
  {  3.2000, -0.1500, 2.40000 },
  {  3.2000,  0.0000, 2.40000 },
  {  0.0000,  0.0000, 3.15000 },
  {  0.8000,  0.0000, 3.15000 },
  {  0.8000, -0.4500, 3.15000 },
  {  0.4500, -0.8000, 3.15000 },

  {  0.0000, -0.8000, 3.15000 },  /* 100 */
  {  0.0000,  0.0000, 2.85000 },
  {  1.4000,  0.0000, 2.40000 },
  {  1.4000, -0.7840, 2.40000 },
  {  0.7840, -1.4000, 2.40000 },
  {  0.0000, -1.4000, 2.40000 },
  {  0.4000,  0.0000, 2.55000 },
  {  0.4000, -0.2240, 2.55000 },
  {  0.2240, -0.4000, 2.55000 },
  {  0.0000, -0.4000, 2.55000 },

  {  1.3000,  0.0000, 2.55000 },  /* 110 */
  {  1.3000, -0.7280, 2.55000 },
  {  0.7280, -1.3000, 2.55000 },
  {  0.0000, -1.3000, 2.55000 }, /* Next four verts kludged to make lid fit */
  {  1.4000,  0.0000, 2.40000 }, /*  {  1.3000,  0.0000, 2.40000 }, */
  {  1.4000, -0.7840, 2.40000 }, /*  {  1.3000, -0.7280, 2.40000 }, */
  {  0.7840, -1.4000, 2.40000 }, /*  {  0.7280, -1.3000, 2.40000 }, */
  {  0.0000, -1.4000, 2.40000 }, /*  {  0.0000, -1.3000, 2.40000 }, */
  {  0.0000,  0.0000, 0.00000 },
  {  1.4250, -0.7980, 0.00000 },

  {  1.5000,  0.0000, 0.07500 },  /* 120 */
  {  1.4250,  0.0000, 0.00000 },
  {  0.7980, -1.4250, 0.00000 },
  {  0.0000, -1.5000, 0.07500 },
  {  0.0000, -1.4250, 0.00000 },
  {  1.5000, -0.8400, 0.07500 },
  {  0.8400, -1.5000, 0.07500 }
} ;


void ssgaTeapot::regenerate ()
{
  removeAllKids () ;

  for ( int i = 0 ; patch[i][0] >= 0 ; i++ )
  {
    ssgaPatch *p = new ssgaPatch ( ntriangles / 32 ) ;
    int j ;

    for ( j = 0 ; j < 16 ; j++ )
    {
      sgVec3 xyz ;
      sgVec4 rgba = { 1,1,1,1 } ;
      sgVec2 uv ;

      uv [ 0 ] = (float)(j&3)/3.0f ;
      uv [ 1 ] = (float)(j>>2)/3.0f ;

      sgScaleVec3 ( xyz, vertex[patch[i][j+1]], 1.0f/2.5f ) ;
      xyz [ 0 ] *= -1.0f ;

      p -> setControlPoint ( j>>2, j&3, xyz, uv, rgba ) ;
    }

    p -> setKidState ( getKidState () ) ;
    p -> setKidCallback ( SSG_CALLBACK_PREDRAW , getKidPreDrawCB  () ) ;
    p -> setKidCallback ( SSG_CALLBACK_POSTDRAW, getKidPostDrawCB () ) ;
    p -> setColour  ( colour ) ;
    p -> setCenter  ( getCenter () ) ;
    p -> setSize    ( getSize   () ) ;
    p -> regenerate () ;

    /* Make a mirror image in the Y axis */

    sgVec3 xyz ;
    sgVec2 uv ;
    sgVec4 rgba ;

    ssgaPatch *p2 = new ssgaPatch ( ntriangles / 32 ) ;

    for ( j = 0 ; j < 16 ; j++ )
    {
      p  -> getControlPoint ( j>>2, 3-(j&3), xyz, uv, rgba ) ; 
      xyz [ 1 ] *= -1.0f ;
      p2 -> setControlPoint ( j>>2, j&3, xyz, uv, rgba ) ; 
    }

    p2 -> setKidState ( getKidState () ) ;
    p2 -> setKidCallback ( SSG_CALLBACK_PREDRAW , getKidPreDrawCB  () ) ;
    p2 -> setKidCallback ( SSG_CALLBACK_POSTDRAW, getKidPostDrawCB () ) ;
    p2 -> setColour  ( colour ) ;
    p2 -> setCenter  ( getCenter () ) ;
    p2 -> setSize    ( getSize   () ) ;
    p2 -> regenerate () ;

    if ( patch[i][0] == 2 )
    {
      /* Make a mirror images in the X axis */

      ssgaPatch *p3 = new ssgaPatch ( ntriangles / 32 ) ;
      ssgaPatch *p4 = new ssgaPatch ( ntriangles / 32 ) ;

      for ( j = 0 ; j < 16 ; j++ )
      {
        p  -> getControlPoint ( j>>2, 3-(j&3), xyz, uv, rgba ) ; 
        xyz [ 0 ] *= -1.0f ;
        p3 -> setControlPoint ( j>>2, j&3, xyz, uv, rgba ) ; 

        p2 -> getControlPoint ( j>>2, 3-(j&3), xyz, uv, rgba ) ; 
        xyz [ 0 ] *= -1.0f ;
        p4 -> setControlPoint ( j>>2, j&3, xyz, uv, rgba ) ; 
      }

      p3 -> setKidState ( getKidState () ) ;
      p3 -> setKidCallback ( SSG_CALLBACK_PREDRAW , getKidPreDrawCB  () ) ;
      p3 -> setKidCallback ( SSG_CALLBACK_POSTDRAW, getKidPostDrawCB () ) ;
      p3 -> setColour  ( colour ) ;
      p3 -> setCenter  ( getCenter () ) ;
      p3 -> setSize    ( getSize   () ) ;
      p3 -> regenerate () ;

      p4 -> setKidState ( getKidState () ) ;
      p4 -> setKidCallback ( SSG_CALLBACK_PREDRAW , getKidPreDrawCB  () ) ;
      p4 -> setKidCallback ( SSG_CALLBACK_POSTDRAW, getKidPostDrawCB () ) ;
      p4 -> setColour  ( colour ) ;
      p4 -> setCenter  ( getCenter () ) ;
      p4 -> setSize    ( getSize   () ) ;
      p4 -> regenerate () ;

      for ( j = 0 ; j < p3 -> getNumKids () ; j++ )
      {
        addKid ( p3->getKid(j) ) ;
        addKid ( p4->getKid(j) ) ;
      }

      p3 -> removeAllKids () ;
      p4 -> removeAllKids () ;
      delete p3 ;
      delete p4 ;
    }

    for ( j = 0 ; j < p -> getNumKids () ; j++ )
    {
      addKid ( p->getKid(j) ) ;
      addKid ( p2->getKid(j) ) ;
    }

    p  -> removeAllKids () ;
    p2 -> removeAllKids () ;
    delete p ;
    delete p2 ;
  }

  recalcBSphere () ;
}


