/*
     PLIB - A Suite of Portable Game Libraries
     Copyright (C) 1998-2004  Steve Baker
 
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

     $Id: ssgAnimTransform.cxx,v 
*/

// Written by Wolfram Kuss in Oct 2004

#include "ssgLocal.h"

float _ssgGlobTime = 0.0f;

const char *ssgAnimTransform::getTypeName (void) { return "ssgAnimTransform" ; }

void ssgAnimTransform::copy_from ( ssgAnimTransform *src, int clone_flags )
{
  selectBank ( src->getCurrBank () ) ; 
  mode = src->mode;
  //ARGH!!!!!!!!!!!!!!!!!!!! transformations.copy_from(static_cast<ssgSimpleList *>(&src->transformations), clone_flags);
  ssgBranch::copy_from ( src, clone_flags ) ;
}

ssgBase *ssgAnimTransform::clone ( int clone_flags )
{
  ssgAnimTransform *b = new ssgAnimTransform ;
  b -> copy_from ( this, clone_flags ) ;
  return b ;
}



ssgAnimTransform::ssgAnimTransform (void)
{
  type = ssgTypeAnimTransform () ;
  curr_bank = 0.0f ; 
  mode = SSGTWEEN_REPEAT;
}


ssgAnimTransform::~ssgAnimTransform (void)
{
  removeAllKids () ;
}
// ********* done till here *************

void ssgAnimTransform::cull ( sgFrustum *f, sgMat4 m, int test_needed )
{
  // calculate the transformation

// comment is partly obsolete:
// You need to have a object with animation loaded into memory for the following function to make sense.
// Animation can be discreet (in steps) or "smooth" (fluid).
// If you loaded an animation made of several meshes, it will use a selector and therefore always be "discreet"
// If you loaded some transformation matrices like some rotations to rotate the flap of a plane,
// the define DISCREET_STEPS will determine whether the animation is smooth (new code, WK) or 
// in steps ("old" code, Dave McClurg). 
//
// The anim_frame is for discreet animation and an index determining which matrix / which selector will be used.
// For the smooth animation, the parameter curr_time_ms is used that should be the current time,
// Typically in MilliSeconds. However, you can speed up or slow down time.
// You could even stop changing curr_time_ms to freeze the animation.
//
// typically, you will call ssgSetAnimTime directly before you call ssgCullAndDraw 
//void ssgSetAnimTime( ssgEntity *e, int anim_frame, long curr_time_ms )

  
  int num = transformations. getNum () ;
  if ( num > 0 )
  {
#ifdef DISCREET_STEPS
    int frame = curr_bank ;
    if ( frame >= num )
      frame = num-1 ;
    transformations. selection = frame ;
    setTransform ( *( transformations. get ( transformations. selection ) ) ) ;
#else
    /* fluid motion */
    ///// copied from ssgTween.cxx and changed var names 

    //if(global_mode)
    curr_bank = _ssgGlobTime;
    
    if ( curr_bank < 0.0f ) curr_bank = 0.0f ;

    int   state1 = (int) floor ( curr_bank ) ;
    int   state2 = state1 + 1 ;
    float tween  = curr_bank - (float) state1 ;

    if ( mode == SSGTWEEN_REPEAT )
    {
      state1 %= num ;
      state2 %= num ;
    }
    else
    {
      if ( state1 >= num ) state1 = num - 1 ;
      if ( state2 >= num ) state2 = num - 1 ;
    }

    if ( state1 == state2    ) tween  = 0.0f;
    /////

    sgMat4 XForm, *pmFrame1;
    pmFrame1 = transformations. get ( state1 );
    sgMat4 *pmFrame2 = transformations. get ( state2 );
    for(int i=0;i<4;i++)
      for(int j=0; j<4; j++)
        XForm[i][j] = tween * ((*pmFrame1)[i][j]) + 
               (1.0f-tween) * ((*pmFrame2)[i][j]);
    setTransform ( XForm );
#endif
  }

  ssgTransform::cull ( f, m, test_needed ) ;
}


// ********* below: done *************

int ssgAnimTransform::load ( FILE *fd )
{
  _ssgReadFloat ( fd, & curr_bank ) ;
  int int_mode;
  _ssgReadInt ( fd, &int_mode );
  mode = int_mode;
  transformations.load( fd );

  return ssgBranch::load ( fd ) ;
}


int ssgAnimTransform::save ( FILE *fd )
{
  _ssgWriteFloat ( fd, curr_bank ) ;
  int int_mode = mode;
  _ssgWriteInt ( fd, int_mode );
  transformations.save( fd );

  return ssgBranch::save ( fd ) ;
}

 
 
void ssgAnimTransform::print ( FILE *fd, char *indent, int how_much )
{
  if ( how_much == 0 )
    return ;
 
  fprintf ( fd, "%sCurrent Bank = %f\n", indent, curr_bank );
  fprintf ( fd, "%sMode = %d\n", indent, (int)mode );
  if ( how_much > 1 )
    transformations.print( fd, indent, how_much );
 
  ssgBranch::print ( fd, indent, how_much ) ;
}


