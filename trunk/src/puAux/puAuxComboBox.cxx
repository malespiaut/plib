/*
     PLIB - A Suite of Portable Game Libraries
     Copyright (C) 1998,2002  Steve Baker

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

     $Id$
*/


#include "puAuxLocal.h"

UL_RTTI_DEF1(puaComboBox,puGroup)


void puaComboBox::input_cb ( puObject *inp )
{
  puaComboBox *cbox = (puaComboBox *) inp -> getUserData () ;

  if ( strcmp ( inp->getStringValue (), cbox->getStringValue () ) )
  {
    cbox->update_current_item () ;
    cbox->setCallbackSource ( PUACOMBOBOX_CALLBACK_INPUT ) ;
    cbox -> invokeCallback () ;
  }
}

void puaComboBox::input_active_cb ( puObject *inp )
{
  puaComboBox *cbox = (puaComboBox *) inp -> getUserData () ;

  cbox->update_current_item () ;
  cbox->setCallbackSource ( PUACOMBOBOX_CALLBACK_INPUT ) ;
  cbox -> invokeActiveCallback () ;
}

void puaComboBox::input_down_cb ( puObject *inp )
{
  puaComboBox *cbox = (puaComboBox *) inp -> getUserData () ;

  cbox->update_current_item () ;
  cbox->setCallbackSource ( PUACOMBOBOX_CALLBACK_INPUT ) ;
  cbox -> invokeCallback () ;
}

void puaComboBox::handle_arrow ( puObject *arrow )
{
  puaComboBox *cbox = (puaComboBox *) arrow -> getUserData () ;

  if ( ! cbox -> __getPopupMenu () -> isVisible () )
    cbox -> __getPopupMenu () -> reveal () ;
  else
    cbox -> __getPopupMenu () -> hide () ;

  /* arrow -> setStyle ( - arrow -> getStyle () ) ; */
}

void puaComboBox::handle_popup ( puObject *popupm )
{
  puaComboBox *cbox = (puaComboBox *) popupm -> getUserData () ;

  cbox -> setCurrentItem ( popupm -> getLegend () ) ;
}


void puaComboBox::update_widgets ( void )
{
  if ( curr_item >= 0 )
  /* There are some items */
  {
    setValue ( list[curr_item] ) ;

    arrow_btn -> activate () ;
  }
  else
  /* There aren't any items */
  {
    setValue ( "" ) ;

    arrow_btn -> greyOut () ;
  }

  input->setValue ( getStringValue () ) ;
}


void puaComboBox::update_current_item ( void )
{
  if ( strcmp ( getStringValue (), input->getStringValue () ) )
  {
    /* User has typed in an arbitrary string; see if it is in the list */
    int i ;
    for ( i = 0 ; i < num_items ; i++ )
    {
      if ( !strcmp ( list[i], input->getStringValue () ) )
      {
        /* ... yes, it is ! */
        curr_item = i ;
        break ;
      }
    }

    if ( ( i == num_items ) || ( num_items == 0 ) )
    {
      /* Didn't find it in the list--or the list is empty */
      char **old_list = list ;
      list = new char * [ num_items + 2 ] ;
      for ( i = 0; i < num_items; i++ )  /* Copy over the old list */
        list[i] = old_list[i] ;

      delete old_list ;

      num_items++ ;
      int len = strlen ( input->getStringValue () ) + 1 ;
      curr_item = num_items - 1 ;
      list[curr_item] = new char [ len ] ;
      memcpy ( list[curr_item], input->getStringValue (), len ) ;
      list[num_items] = NULL ;
      newList ( list ) ;
      update_widgets () ;
    }

    setValue ( input->getStringValue () ) ;
  }
}


void puaComboBox::newList ( char ** _list )
{
  // Delete the existing list--if it has not been passed in as the argument
  int i ;
  if ( list && ( list != _list ) )
  {
    for ( i = 0; i < num_items; i++ )
      delete list[i] ;

    delete list ;
    list = NULL ;
  }

  popup_menu -> empty () ;

  if ( _list == NULL )
    num_items = 0 ;
  else
  {
    for ( num_items = 0 ; _list[num_items] != NULL ; num_items++ )
      /* Count number of items */ ;

    if ( num_items > 0 )
    {
      if ( list != _list )
      {
        list = new char * [ num_items + 1 ] ;  /* '+ 1' to capture the trailing NULL */
        for ( i = 0; i < num_items; i++ )
        {
          int len = strlen ( _list[i] ) + 1 ;  /* "+ 1" to capture the \0 */
          list[i] = new char [ len ] ;
          memcpy ( list[i], _list[i], len * sizeof(char) ) ;
        }

        list[num_items] = NULL ;
      }

      int dummy, h ;
      int old_height = abox.max[1] - abox.min[1] ;

      puPushGroup ( popup_menu ) ;

      for ( i = (num_items - 1) ; i >= 0 ; i-- )
      {
        puObject *menu_item = popup_menu -> add_item ( list[i], handle_popup ) ;

        menu_item -> setUserData ( this ) ;
        menu_item -> getSize ( &dummy, &h ) ;
        menu_item -> setSize ( abox.max[0] - abox.min[0], h ) ;
      }

      popup_menu -> close() ;

      /* Position popup menu correctly */

      popup_menu -> getSize ( &dummy, &h ) ;

      if ( (abox.min[1] - h) >= 0 )
      {
        popup_menu -> setPosition ( 0, 0 - h ) ;

        arrow_btn  -> setArrowType ( PUARROW_DOWN ) ;
      }
      else
      {
        popup_menu -> setPosition ( 0, old_height ) ;

        arrow_btn  -> setArrowType ( PUARROW_UP   ) ;
      }
    }
  }

  if ( ( curr_item >= num_items ) || ( curr_item < 0 ) )
  {
    curr_item = ( num_items > 0 ? 0 : - 1 ) ;
    update_widgets () ;
  }
}

int puaComboBox::getCurrentItem ( void )
{
  update_current_item () ;

  return curr_item ;
}

void puaComboBox::setCurrentItem ( const char *item_ptr )
{
  int i ;

  for ( i = 0 ; i < num_items ; i++ )
  {
    if ( list[i] == item_ptr )
    {
      setCurrentItem ( i ) ;

      break ;
    }
  }
}


void puaComboBox::draw ( int dx, int dy )
{
  if ( !visible || ( window != puGetWindow () ) ) return ;

  draw_label ( dx, dy ) ;

  puGroup::draw ( dx, dy ) ;
}

int puaComboBox::checkHit ( int button, int updown, int x, int y )
{
  if ( input -> checkHit ( button, updown, x-abox.min[0], y-abox.min[1] ) )
  {
    popup_menu -> hide () ;
    puSetActiveWidget ( this, x, y ) ;

    return TRUE ;
  }
  else
    return puGroup::checkHit ( button, updown, x, y ) ;
}

int puaComboBox::checkKey ( int key, int updown )
{
  if ( updown == PU_UP || ! input -> isAcceptingInput () ||
       ! isVisible () || ! isActive () || ( window != puGetWindow () ) )
    return FALSE ;

  switch ( key )
  {
    case PU_KEY_HOME      :
      setCurrentItem ( 0 ) ;
      break ;

    case PU_KEY_END       :
      setCurrentItem ( num_items - 1 ) ;
      break ;

    case PU_KEY_UP        :
      setCurrentItem ( getCurrentItem () + 1 ) ;
      break ;

    case PU_KEY_DOWN      :
      setCurrentItem ( getCurrentItem () - 1 ) ;
      break ;

    case PU_KEY_PAGE_UP   :
    case PU_KEY_PAGE_DOWN :
      if ( ! popup_menu -> isVisible () )
        popup_menu -> reveal () ;
      else
        popup_menu -> hide () ;

      break ;

    default :
      return input -> checkKey ( key, updown ) ;
      break ;
  }

  return TRUE ;
}


puaComboBox::puaComboBox ( int minx, int miny, int maxx, int maxy,
                           char **entries, int editable ) :
   puGroup( minx, miny )
{
  type |= PUCLASS_COMBOBOX ;

  char *stringval ;
  int arrow_width = (int) ( (maxy-miny) / 1.5f ) ;

  input = new puInput ( 0, 0, maxx-minx - arrow_width, maxy-miny ) ;
  input -> setUserData ( this ) ;
  input -> setCallback ( input_cb ) ;
  input -> setActiveCallback ( input_active_cb ) ;
  input -> setDownCallback ( input_down_cb ) ;

  input -> setStyle ( PUSTYLE_SMALL_SHADED ) ;

  if ( ! editable )
    input -> disableInput () ;

  /* Share 'string' value with input box */
  input -> getValue ( &stringval ) ;
//  setValuator ( stringval ) ;

  arrow_btn = new puArrowButton ( maxx-minx - arrow_width, 0,
                                  maxx-minx, maxy-miny,
                                  PUARROW_DOWN ) ;
  arrow_btn -> setStyle ( PUSTYLE_SMALL_SHADED ) ;
  arrow_btn -> setUserData ( this ) ;
  arrow_btn -> setCallback ( handle_arrow ) ;

  popup_menu = new puPopupMenu ( 0, 0 ) ;
  popup_menu -> close () ;

  close () ;

  list = NULL ;
  curr_item = 0 ;
  num_items = 0 ;
  callback_source = PUACOMBOBOX_CALLBACK_NONE ;
  newList ( entries ) ;
  update_widgets () ;
}

void puaComboBox::setSize ( int w, int h )
{
  int arrow_width = (int) ( float(h) / 1.5f ) ;
  input->setSize ( w - arrow_width, h ) ;
  arrow_btn->setPosition ( w - arrow_width, 0 ) ;
  arrow_btn->setSize ( arrow_width, h ) ;
}

