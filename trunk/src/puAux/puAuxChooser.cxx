/*
     PUI Auxiliary Widget Library
     Derived from PLIB, the Portable Game Library by Steve Baker.
     Copyright (C) 1998,2002,2004  Steve Baker

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

UL_RTTI_DEF(puaChooser)



puaChooser::puaChooser ( int _x1, int _y1, int _x2, int _y2, char *legend )
{
  x1 = _x1 ; x2 = _x2 ; y1 = _y1 ; y2 = _y2 ;

  chooser_button = new puButton ( x1, y1, x2, y2 ) ;
  chooser_button->setValue      ( FALSE  ) ;
  chooser_button->setLegend     ( legend ) ;
  chooser_button->setUserData   ( this   ) ;
  chooser_button->setCallback   ( static_popup_cb ) ;

  popup_menu = new puPopupMenu ( x1, y1 ) ;
  popup_menu -> setUserData    ( this ) ;
  popup_menu -> setCallback    ( static_menu_cb ) ;
}


void puaChooser::add_item ( char *str, puCallback _cb, void *_user_data )
{
  popup_menu -> add_item ( str, _cb, _user_data ) ;
}


void puaChooser::close ()
{
  popup_menu -> close () ;
  // int x, y, w, h ;
  // popup_menu -> getPosition ( &x, &y ) ;
  // popup_menu -> getSize     ( &w, &h ) ;
  popup_menu -> setPosition ( x1, y2 ) ;
}


void puaChooser::popup_cb ()
{
  chooser_button -> setValue ( TRUE ) ;

  if ( popup_menu -> isVisible () )
    popup_menu -> hide () ;
  else
    popup_menu -> reveal () ;
}


void puaChooser::menuCleanup ( const char *s )
{
  popup_menu     -> hide     () ;
  chooser_button -> setValue ( FALSE ) ;

  if ( s != NULL )
    chooser_button -> setLegend ( s ) ;
}


void puaChooser::menuCleanup ( puObject *ob )
{
  puObject *p = ob -> getParent () ;
  ((puaChooser*)( p -> getUserData () )) -> menuCleanup ( ob -> getLegend () ) ;
}


void puaChooser::static_popup_cb ( puObject *ob )
{
  ((puaChooser*)( ob -> getUserData () )) -> popup_cb () ;
}



void puaChooser::static_menu_cb ( puObject *ob )
{
  ((puaChooser*)( ob -> getUserData () )) -> menuCleanup ( (const char *)NULL ) ;
}


