/*
     This file is part of P-GUIDE -
     PUI-based Graphical User Interface Designer.
     Copyright (C) 2002  John F. Fay

     P-GUIDE is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published by
     the Free Software Foundation; either version 2 of the License, or
     (at your option) any later version.

     P-GUIDE is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU General Public License for more details.

     You should have received a copy of the GNU General Public License
     along with P-GUIDE; if not, write to the Free Software
     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA

     $Id$
*/

// Create Widget function for GUI Builder

#include <plib/pu.h>

puObject *createWidget ( int type )
{
  static char *box_labels [] = { NULL } ;

  if ( type & PUCLASS_SELECTBOX        ) return new puSelectBox ( 0, 0, 90, 20, box_labels ) ;
  if ( type & PUCLASS_COMBOBOX         ) return new puComboBox ( 0, 0, 90, 40, box_labels ) ;
  if ( type & PUCLASS_LARGEINPUT       ) return new puLargeInput ( 0, 0, 90, 40, 2, 20, FALSE ) ;
  if ( type & PUCLASS_VERTMENU         ) return new puVerticalMenu ( 0, 0 ) ;
  if ( type & PUCLASS_TRISLIDER        ) return new puTriSlider ( 0, 0, 90 ) ;
  if ( type & PUCLASS_BISLIDER         ) return new puBiSlider ( 0, 0, 90 ) ;
  if ( type & PUCLASS_FILESELECTOR     ) return new puFileSelector ( 0, 0, 90, 40, 2, "." ) ;
  if ( type & PUCLASS_DIAL             ) return new puDial ( 0, 0, 40 ) ;
  if ( type & PUCLASS_LISTBOX          ) return new puListBox ( 0, 0, 90, 40 ) ;
  if ( type & PUCLASS_ARROW            ) return new puArrowButton ( 0, 0, 20, 20, PUARROW_UP ) ;
  if ( type & PUCLASS_DIALOGBOX        ) return new puDialogBox ( 0, 0 ) ;
  if ( type & PUCLASS_SLIDER           ) return new puSlider ( 0, 0, 90 ) ;
  if ( type & PUCLASS_BUTTONBOX        ) return new puButtonBox ( 0, 0, 90, 50, box_labels, FALSE ) ;
  if ( type & PUCLASS_INPUT            ) return new puInput ( 0, 0, 90, 20 ) ;
  if ( type & PUCLASS_MENUBAR          ) return new puMenuBar () ;
  if ( type & PUCLASS_POPUPMENU        ) return new puPopupMenu ( 0, 0 ) ;
  if ( type & PUCLASS_POPUP            ) return new puPopup ( 0, 0 ) ;
  if ( type & PUCLASS_ONESHOT          ) return new puOneShot ( 0, 0, 90, 20 ) ;
  if ( type & PUCLASS_BUTTON           ) return new puButton ( 0, 0, 90, 20 ) ;
  if ( type & PUCLASS_TEXT             ) return new puText ( 0, 0 ) ;
  if ( type & PUCLASS_FRAME            ) return new puFrame ( 0, 0, 90, 50 ) ;
  if ( type & PUCLASS_INTERFACE        ) return new puInterface ( 0, 0 ) ;
  if ( type & PUCLASS_GROUP            ) return new puGroup ( 0, 0 ) ;

  return (puObject *)NULL ;
}

