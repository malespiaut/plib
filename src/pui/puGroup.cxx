
#include "puLocal.h"

#define PUSTACK_MAX 100

static int currGroup = -1 ;
static puGroup *groupStack [ PUSTACK_MAX ] ;

void puPushGroup ( puGroup *in )
{
  if ( currGroup < PUSTACK_MAX )
    groupStack [ ++currGroup ] = in ;
  else
    fprintf ( stderr, "PUI: Too many puGroups open at once!\n" ) ;
}


void  puPopGroup ( void )
{
  if ( currGroup > 0 )
    --currGroup ;
  else 
    fprintf ( stderr, "PUI: puGroup stack is empty!\n" ) ;
}

int  puNoGroup ( void )
{
  return currGroup < 0 ;
}


puGroup *puGetCurrGroup ( void )
{
  if ( currGroup < 0 )
  {
    fprintf ( stderr, "PUI: No Group!\n" ) ;
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

  for ( bo = dlist ; bo->next != NULL ; bo = bo->next )
    /* Find the last object in our list. */ ;

  for ( ; bo != NULL ; bo = bo->prev )
    if ( bo -> checkHit ( button, updown, x, y ) )
      return TRUE ;

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
  for ( puObject *bo = dlist ; bo != NULL ; bo = bo->next )
    delete bo ;
}



