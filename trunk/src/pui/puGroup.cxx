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


#include "puLocal.h"

#define PUSTACK_MAX 100

static int currGroup = -1 ;
static puGroup *groupStack [ PUSTACK_MAX ] ;

void puPushGroup ( puGroup *in )
{
  if ( currGroup < PUSTACK_MAX )
    groupStack [ ++currGroup ] = in ;
  else
    ulSetError ( UL_WARNING, "PUI: Too many puGroups open at once!" ) ;
}


void  puPopGroup ( void )
{
  if ( currGroup > 0 )
    --currGroup ;
  else 
    ulSetError ( UL_WARNING, "PUI: puGroup stack is empty!" ) ;
}

int  puNoGroup ( void )
{
  return currGroup < 0 ;
}


puGroup *puGetCurrGroup ( void )
{
  if ( currGroup < 0 )
  {
    ulSetError ( UL_WARNING, "PUI: No Group!" ) ;
    return NULL ;
  }

  return groupStack [ currGroup ] ;
}

void puGroup::remove ( puObject *obj )
{
  if ( dlist == NULL )
    return ;

  /* Are we the first object in the list */

  if ( obj -> prev == NULL )
    dlist = obj -> next ;
  else
    obj -> prev -> next = obj -> next ;

  /* Are we the last object in the list */

  if ( obj -> next != NULL )
    obj -> next -> prev = obj -> prev ;

  obj -> next = NULL ;
  obj -> prev = NULL ;

  num_children-- ;
  recalc_bbox () ;
}

void puGroup::empty ( void )
{
  puObject *obj = dlist ;
  while ( obj != NULL )
  {
    if ( obj->getType () & PUCLASS_GROUP )
    {
      puGroup *group = (puGroup *)obj ;
      group->empty () ;
    }

    if ( obj->getUserData () )
    {
      puGroup *group = (puGroup *)( obj->getUserData () ) ;
      group->empty () ;
      obj->setUserData ( NULL ) ;
    }

    puObject *temp = obj->getNextObject () ;
    delete obj ;
    obj = temp ;
  }

  dlist = NULL ;
}

void puGroup::add ( puObject *new_obj )
{
  if ( dlist == NULL )
  {
    dlist = new_obj ;
    new_obj -> next = NULL ;
    new_obj -> prev = NULL ;
  }
  else
  {
    puObject *last ;

    for ( last = dlist ; last->next != NULL ; last = last->next )
      /* Search for end of list. */ ;

    last -> next = new_obj ;
    new_obj -> prev = last ;
    new_obj -> next = NULL ;
  }

  num_children++ ;
  recalc_bbox () ;
}

int puGroup::checkKey ( int key, int updown )
{
  if ( dlist == NULL || ! isVisible () || ! isActive () )
    return FALSE ;

  puObject *bo ;

  /*
    We have to walk the list backwards to ensure that
    the click order is the same as the DRAW order.
  */

  for ( bo = dlist ; bo->next != NULL ; bo = bo->next )
    /* Find the last object in our list. */ ;

  for ( ; bo != NULL ; bo = bo->prev )
    if ( bo -> checkKey ( key, updown ) )
      return TRUE ;

  return FALSE ;
}

int puGroup::checkHit ( int button, int updown, int x, int y )
{
  if ( dlist == NULL || ! isVisible () || ! isActive () )
    return FALSE ;

  /*
    This might be a bit redundant - but it's too hard to keep
    track of changing abox sizes when daughter objects are
    changing sizes.
  */

  recalc_bbox () ;

  puObject *bo ;

  x -= abox.min[0] ;
  y -= abox.min[1] ;

  /*
    We have to walk the list backwards to ensure that
    the click order is the same as the DRAW order.
  */

  if ( !mouse_active )
  {
    /* Find the last object in our list. */

    for ( bo = dlist ; bo->next != NULL ; bo = bo->next ) ;

    for ( ; bo != NULL ; bo = bo->prev )
      if ( bo -> checkHit ( button, updown, x, y ) )
        return TRUE ;
  }

  /*
    If right mouse button is pressed, save mouse coordinates for
    dragging and dropping.  Do this only if the "floating" flag is set.
  */

  if ( mouse_active || ( isHit ( x+abox.min[0], y+abox.min[1]) &&
       floating && ( button == PU_RIGHT_BUTTON ) ) )
  {
    puMoveToLast ( this );

    /*
      Return (x, y) to coordinates of parent interface to avoid "jumping" of
      present interface as mouse drags
    */

    x += abox.min[0] ;
    y += abox.min[1] ;

    if ( updown == PU_DOWN )
    {
      mouse_x = x;  /* Save mouse coordinates for dragging */
      mouse_y = y;

      mouse_active = TRUE ;

      return TRUE ;
    }
    else if ( updown == PU_DRAG )
    {
      int curr_x, curr_y;

      getPosition ( &curr_x, &curr_y );
      setPosition ( curr_x+x-mouse_x, curr_y+y-mouse_y );  /* Move to new position */

      mouse_x = x;  /* Save new coordinates */
      mouse_y = y;

      return TRUE ;
    }
    else if ( updown == PU_UP )
    {
      mouse_active = FALSE ;

      return TRUE ;
    }
  }

  return FALSE ;
}


void puGroup::draw ( int dx, int dy )
{
  if ( ! isVisible () )
    return ;

  for ( puObject *bo = dlist ; bo != NULL ; bo = bo->next )
  {
    /* June 16th, 98, Shammi :
     * The next if statement checks if the object is
     * a menu bar and makes sure it is repositioned
     * correctly.
     */

    if ( bo->getType() & PUCLASS_MENUBAR )
    {
      int obWidth, obHeight ;
      bo -> getSize ( &obWidth, &obHeight ) ;
      bo -> setPosition ( 0, puGetWindowHeight() - obHeight ) ;
    }

    bo -> draw ( dx + abox.min[0], dy + abox.min[1] ) ;
  }
}


void puGroup::recalc_bbox ( void ) 
{
  puBox contents ;
  contents . empty () ;

  for ( puObject *bo = dlist ; bo != NULL ; bo = bo->next )
    contents . extend ( bo -> getBBox() ) ;

  if ( contents . isEmpty () )
  {
    abox . max[0] = abox . min[0] ;
    abox . max[1] = abox . min[1] ;
  }
  else
  {
    abox . max[0] = abox . min[0] + contents . max[0] ;
    abox . max[1] = abox . min[1] + contents . max[1] ;
  }

  puObject::recalc_bbox () ;
}


void puGroup::doHit ( int, int, int, int )
{
}


puGroup::~puGroup ()
{
  for ( puObject *bo = dlist ; bo != NULL ; /* Nothing */ )
  {
    dlist = bo    ;
    bo = bo->next ;
    delete dlist  ;
  }

  if ( this == puActiveWidget () )
    puDeactivateWidget () ;
}


void puGroup::setChildStyle ( int childs, int style, int recursive = FALSE )
{
  puObject *curr_obj ;

  for ( curr_obj = getFirstChild () ;
        curr_obj != NULL ;
        curr_obj = curr_obj->getNextObject () )
  {
    if ( ( recursive == TRUE ) && ( curr_obj->getType () & PUCLASS_GROUP ) )
      ((puGroup*)curr_obj)->setChildStyle ( childs, style ) ;
    else
    {
      if ( curr_obj->getType () & childs )
        curr_obj->setStyle ( style ) ;
    }
  }
}

void puGroup::setChildBorderThickness ( int childs, int t, int recursive = FALSE )
{
  puObject *curr_obj ;

  for ( curr_obj = getFirstChild () ;
        curr_obj != NULL ;
        curr_obj = curr_obj->getNextObject () )
  {
    if ( ( recursive == TRUE ) && ( curr_obj->getType () & PUCLASS_GROUP ) )
      ((puGroup*)curr_obj)->setChildBorderThickness ( childs, t ) ;
    else
    {
      if ( curr_obj->getType () & childs )
        curr_obj->setBorderThickness ( t ) ;
    }
  }
}

void puGroup::setChildColour ( int childs, int which, float r, float g, float b, float a = 1.0f, int recursive = FALSE )
{
  puObject *curr_obj ;

  for ( curr_obj = getFirstChild () ;
        curr_obj != NULL ;
        curr_obj = curr_obj->getNextObject () )
  {
    if ( ( recursive == TRUE ) && ( curr_obj->getType () & PUCLASS_GROUP ) )
      ((puGroup*)curr_obj)->setChildColour ( childs, which, r, g, b, a ) ;
    else
    {
      if ( curr_obj->getType () & childs )
        curr_obj->setColour ( which, r, g, b, a ) ;
    }
  }
}

void puGroup::setChildColourScheme ( int childs, float r, float g, float b, float a = 1.0f, int recursive = FALSE )
{
  puObject *curr_obj ;

  for ( curr_obj = getFirstChild () ;
        curr_obj != NULL ;
        curr_obj = curr_obj->getNextObject () )
  {
    if ( ( recursive == TRUE ) && ( curr_obj->getType () & PUCLASS_GROUP ) )
      ((puGroup*)curr_obj)->setChildColourScheme ( childs, r, g, b, a ) ;
    else
    {
      if ( curr_obj->getType () & childs )
        curr_obj->setColourScheme ( r, g, b, a ) ;
    }
  }
}

