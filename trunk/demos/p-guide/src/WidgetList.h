/*
     This file is part of P-GUIDE -
     PUI-based Graphical User Interface Designer.
     Copyright (C) 2002, 2006  John F. Fay

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

// Data Structure Definition

#ifndef WIDGET_LIST_H
#define WIDGET_LIST_H

#include <plib/pu.h>
#include <plib/puAux.h>

#ifdef FREEGLUT_IS_PRESENT
#  include <GL/freeglut.h>
#else
#  ifdef __APPLE__
#    include <GLUT/glut.h>
#  else
#    include <GL/glut.h>
#  endif
#endif


// PUI Widget List for Main Window

struct WidgetList
{
  puObject *obj ;
  char *object_type_name ;
  int object_type ;
  char *label_text ;
  char *legend_text ;
  short callbacks ;
  char object_name [ PUSTRING_MAX ] ;
  bool visible ;
  bool locked ;
  int layer ;  // GUI layer:  0 - in back, positive nubmers - greater in front of lesser
  WidgetList *next ;
  /* Additional data for the extended properties of widgets */
  char *items ;
  char *allowed ;
  int intval1 ;
  int intval2 ;
  bool boolval1 ;
  bool boolval2 ;
  bool boolval3 ;
  float floatval1 ;
  float floatval2 ;
  float floatval3 ;
  float floatval4 ;
  float floatval5 ;
  float floatval6 ;
} ;


#endif  // WIDGET_LIST_H

