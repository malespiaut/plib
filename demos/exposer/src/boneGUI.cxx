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


#include "exposer.h"

static puSlider *XtranslateSlider ;
static puSlider *YtranslateSlider ;
static puSlider *ZtranslateSlider ;
static puInput  *XtranslateInput  ;
static puInput  *YtranslateInput  ;
static puInput  *ZtranslateInput  ;


static void jointHeadingCB ( puObject *ob )
{
  Bone *bone = (Bone *) (ob->getUserData()) ;
  float a ; ob -> getValue ( & a ) ;
 
  bone->setAngle ( 0, a * 360.0f - 180.0f ) ;
  setShowAngle ( a * 360.0f - 180.0f ) ;
}
 
 
static void jointPitchCB ( puObject *ob )
{
  Bone *bone = (Bone *) (ob->getUserData()) ;
  float a ; ob -> getValue ( & a ) ;
 
  bone->setAngle ( 1, a * 360.0f - 180.0f ) ;
  setShowAngle ( a * 360.0f - 180.0f ) ;
}
 
 
static void jointRollCB ( puObject *ob )
{
  Bone *bone = (Bone *) (ob->getUserData()) ;
  float a ; ob -> getValue ( & a ) ;
 
  bone->setAngle ( 2, a * 360.0f - 180.0f ) ;
  setShowAngle ( a * 360.0f - 180.0f ) ;
}
 
 
void hide_headingCB ( puObject *ob )
{
  Bone *bone = (Bone *) (ob->getUserData()) ;
 
  if ( ob -> getValue () )
    bone -> sh -> hide () ;
  else
    bone -> sh -> reveal () ;
}
 
 
void hide_pitchCB ( puObject *ob )
{
  Bone *bone = (Bone *) (ob->getUserData()) ;
 
  if ( ob -> getValue () )
    bone -> sp -> hide () ;
  else
    bone -> sp -> reveal () ;
}
                                                                                 
void hide_rollCB ( puObject *ob )
{
  Bone *bone = (Bone *) (ob->getUserData()) ;
 
  if ( ob -> getValue () )
    bone -> sr -> hide () ;
  else
    bone -> sr -> reveal () ;
}
 
 
static void resetCB ( puObject *ob )
{
  Bone *bone = (Bone *) (ob->getUserData()) ;
 
  bone -> setAngles ( 0, 0, 0 ) ;
}


void syncTranslators ( sgVec3 trans )
{
  XtranslateSlider -> setValue ( trans [ 0 ] / 5.0f + 0.5f ) ;
  YtranslateSlider -> setValue ( trans [ 1 ] / 5.0f + 0.5f ) ;
  ZtranslateSlider -> setValue ( trans [ 2 ] / 5.0f + 0.5f ) ;

  if ( ! XtranslateInput -> isAcceptingInput () )
    XtranslateInput  -> setValue ( trans [ 0 ] ) ;
  if ( ! YtranslateInput -> isAcceptingInput () )
    YtranslateInput  -> setValue ( trans [ 1 ] ) ;
  if ( ! ZtranslateInput -> isAcceptingInput () )
    ZtranslateInput  -> setValue ( trans [ 2 ] ) ;

}


static void currTranslateTxtXCB ( puObject *sl )
{
  float v = sl -> getFloatValue () ;

  if ( eventList->getCurrentEvent() == NULL ) return ;

  sgVec3 xyz ;

  eventList->getCurrentEvent() -> getTranslate ( xyz ) ;
  xyz [ 0 ] = v ;
  eventList->getCurrentEvent() -> setTranslate ( xyz ) ;

  syncTranslators ( xyz ) ;
}



static void currTranslateTxtYCB ( puObject *sl )
{
  float v = sl -> getFloatValue () ;

  if ( eventList->getCurrentEvent() == NULL ) return ;

  sgVec3 xyz ;

  eventList->getCurrentEvent() -> getTranslate ( xyz ) ;
  xyz [ 1 ] = v ;
  eventList->getCurrentEvent() -> setTranslate ( xyz ) ;

  syncTranslators ( xyz ) ;
}



static void currTranslateTxtZCB ( puObject *sl )
{
  float v = sl -> getFloatValue () ;

  if ( eventList->getCurrentEvent() == NULL ) return ;

  sgVec3 xyz ;

  eventList->getCurrentEvent() -> getTranslate ( xyz ) ;
  xyz [ 2 ] = v ;
  eventList->getCurrentEvent() -> setTranslate ( xyz ) ;

  syncTranslators ( xyz ) ;
}




static void currTranslateXCB ( puObject *sl )
{
  float v = (((puSlider *)sl) -> getFloatValue () - 0.5 ) * 5.0f ;

  if ( eventList->getCurrentEvent() == NULL ) return ;

  sgVec3 xyz ;

  eventList->getCurrentEvent() -> getTranslate ( xyz ) ;
  xyz [ 0 ] = v ;
  eventList->getCurrentEvent() -> setTranslate ( xyz ) ;

  syncTranslators ( xyz ) ;
}



static void currTranslateYCB ( puObject *sl )
{
  float v = (((puSlider *)sl) -> getFloatValue () - 0.5 ) * 5.0f ;

  if ( eventList->getCurrentEvent() == NULL ) return ;

  sgVec3 xyz ;

  eventList->getCurrentEvent() -> getTranslate ( xyz ) ;
  xyz [ 1 ] = v ;
  eventList->getCurrentEvent() -> setTranslate ( xyz ) ;

  syncTranslators ( xyz ) ;
}



static void currTranslateZCB ( puObject *sl )
{
  float v = (((puSlider *)sl) -> getFloatValue () - 0.5 ) * 5.0f ;

  if ( eventList->getCurrentEvent() == NULL ) return ;

  sgVec3 xyz ;

  eventList->getCurrentEvent() -> getTranslate ( xyz ) ;
  xyz [ 2 ] = v ;
  eventList->getCurrentEvent() -> setTranslate ( xyz ) ;

  syncTranslators ( xyz ) ;
}


void initBoneGUI ()
{
  puText   *message ;

  ZtranslateInput  =  new puInput ( 5, 485, 80, 505 ) ;
  ZtranslateInput  -> setCallback ( currTranslateTxtZCB ) ;

  ZtranslateSlider = new puSlider ( 80, 485, 120, FALSE ) ;
  ZtranslateSlider -> setCBMode   ( PUSLIDER_DELTA ) ;
  ZtranslateSlider -> setDelta    ( 0.01    ) ;
  ZtranslateSlider -> setCallback ( currTranslateZCB ) ;
  message = new puText ( 205,485 ) ; message->setLabel ( "Z" ) ; 

  YtranslateInput  =  new puInput ( 5, 505, 80, 525 ) ;
  YtranslateInput  -> setCallback ( currTranslateTxtYCB ) ;

  YtranslateSlider = new puSlider ( 80, 505, 120, FALSE ) ;
  YtranslateSlider -> setCBMode   ( PUSLIDER_DELTA ) ;
  YtranslateSlider -> setDelta    ( 0.01    ) ;
  YtranslateSlider -> setCallback ( currTranslateYCB ) ;
  message = new puText ( 205,505 ) ; message->setLabel ( "Y" ) ; 

  XtranslateInput  =  new puInput ( 5, 525, 80, 545 ) ;
  XtranslateInput  -> setCallback ( currTranslateTxtXCB ) ;

  XtranslateSlider = new puSlider ( 80, 525, 120, FALSE ) ;
  XtranslateSlider -> setCBMode   ( PUSLIDER_DELTA ) ;
  XtranslateSlider -> setDelta    ( 0.01    ) ;
  XtranslateSlider -> setCallback ( currTranslateXCB ) ;
  message = new puText ( 205,525 ) ; message->setLabel ( "X" ) ; 
}


void Bone::createJoint ()
{
  widget = new puGroup ( 0, 0 ) ;
  rs = new puOneShot ( 0, 0, "x" ) ;
  hb = new puButton  (20, 0, "H" ) ;
  pb = new puButton  (40, 0, "P" ) ;
  rb = new puButton  (60, 0, "R" ) ;
  sh = new puDial  (  80, 0, 40 ) ;
  sp = new puDial  ( 120, 0, 40 ) ;
  sr = new puDial  ( 160, 0, 40 ) ;
  na = new puInput ( 0,20,80,40 ) ;
 
  na->setUserData ( this ) ;
  na->setColourScheme ( colour[0], colour[1], colour[2], 0.5f ) ;
 
  rs->setUserData ( this ) ;
  rs->setCallback ( resetCB ) ;
  rs->setColourScheme ( colour[0], colour[1], colour[2], 0.5f ) ;
 
  hb->setUserData ( this ) ;
  hb->setCallback ( hide_headingCB ) ;
  hb->setColourScheme ( colour[0], colour[1], colour[2], 0.5f ) ;
 
  pb->setUserData ( this ) ;
  pb->setCallback ( hide_pitchCB ) ;
  pb->setColourScheme ( colour[0], colour[1], colour[2], 0.5f ) ;
 
  rb->setUserData ( this ) ;
  rb->setCallback ( hide_rollCB ) ;
  rb->setColourScheme ( colour[0], colour[1], colour[2], 0.5f ) ;
 
  sh->setUserData ( this ) ;
  sh->setValue ( 0.5f ) ;
  sh->setCallback ( jointHeadingCB ) ;
  sh->setColourScheme ( colour[0], colour[1], colour[2], 0.5f ) ;
 
  sp->setUserData ( this ) ;
  sp->setValue ( 0.5f ) ;
  sp->setCallback ( jointPitchCB ) ;
  sp->setColourScheme ( colour[0], colour[1], colour[2], 0.5f ) ;
 
  sr->setUserData ( this ) ;
  sr->setValue ( 0.5f ) ;
  sr->setCallback ( jointRollCB ) ;
  sr->setColourScheme ( colour[0], colour[1], colour[2], 0.5f ) ;
 
  widget -> close () ;
  widget -> hide  () ;
}

