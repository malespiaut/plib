
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


#include "ssgLocal.h"

void ssgTweenController::copy_from ( ssgTweenController *src, int clone_flags )
{
  selectBank ( src->getCurrBank () ) ;
  ssgBranch::copy_from ( src, clone_flags ) ;
}

ssgBase *ssgTweenController::clone ( int clone_flags )
{
  ssgTweenController *b = new ssgTweenController ;
  b -> copy_from ( this, clone_flags ) ;
  return b ;
}



ssgTweenController::ssgTweenController (void)
{
  type = ssgTypeTweenController () ;
  curr_bank = 0.0f ;
}


ssgTweenController::~ssgTweenController (void)
{
  removeAllKids () ;
}


void ssgTweenController::cull ( sgFrustum *f, sgMat4 m, int test_needed )
{
  float tmp = _ssgGetCurrentTweenState () ;
  _ssgSetCurrentTweenState ( curr_bank ) ;
 
  ssgBranch::cull ( f, m, test_needed ) ;

  _ssgSetCurrentTweenState ( tmp ) ;
}



int ssgTweenController::load ( FILE *fd )
{
  _ssgReadFloat ( fd, & curr_bank ) ;

  return ssgBranch::load ( fd ) ;
}


int ssgTweenController::save ( FILE *fd )
{
  _ssgWriteFloat ( fd, curr_bank ) ;
  return ssgBranch::save ( fd ) ;
}


