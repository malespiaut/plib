/*
     This file is part of ExPoser - A Tool for Animating PLIB Critters.
     Copyright (C) 2001  Steve Baker

     ExPoser is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published by
     the Free Software Foundation; either version 2 of the License, or
     (at your option) any later version.

     ExPoser is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU General Public License for more details.

     You should have received a copy of the GNU General Public License
     along with ExPoser; if not, write to the Free Software
     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA

     For further information visit http://plib.sourceforge.net

     $Id$
*/


#define ROOT_BONE  9999
 
class Bone : public ssgBase
{
public:
  int           id        ;
  int           parent    ;
  ssgTransform *effector  ;

  sgVec3        xlate     ;
  sgMat4        netMatrix ;

  sgVec3             vx [ 2 ] ;  /* Relative to joint */
  sgVec3        orig_vx [ 2 ] ;  /* Absolute */

  puGroup *widget ;
  puOneShot *rs ;
  puButton  *hb ;
  puButton  *pb ;
  puButton  *rb ;
  puDial    *sh ;
  puDial    *sp ;
  puDial    *sr ;
  puInput   *na ;
  sgVec4 colour ;
  Bone () ;

  void read  ( FILE *fd ) ;
  void write ( FILE *fd ) ;
  void createJoint () ;

  void computeTransform ( Event *prev, Event *next, float tim ) ;

  void setAngle  ( int which, float a ) ;
  void setAngles ( float h, float p, float r ) ;
  void setAngles ( sgVec3 a ) ;

  float *getXYZ () { return xlate ; }

  float *getDialAngles () ;

  sgCoord *getXForm ( Event *prev, Event *next, float tim ) ;
  sgCoord *getXForm () ;

  void transform ( sgVec3 dst, sgVec3 src ) ;
  void swapEnds() ;
  void init ( ssgLeaf *l, sgMat4 newmat, short vv[2], int id ) ;
  void print ( FILE *fd, int which ) ;
  ssgBranch *generateGeometry ( int root ) ;
} ;
 

void init_bones      () ;
void setShowAngle    ( float a ) ;

float *getCurrTranslate () ;
Bone  *getBone   ( int i ) ;

ssgSimpleState *getBoneState () ;

