/*
     This file is part of TTT3D - Steve's 3D TicTacToe Player.
     Copyright (C) 2001  Steve Baker

     TTT3D is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published by
     the Free Software Foundation; either version 2 of the License, or
     (at your option) any later version.

     TTT3D is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU General Public License for more details.

     You should have received a copy of the GNU General Public License
     along with TTT3D; if not, write to the Free Software
     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA

     For further information visit http://plib.sourceforge.net

     $Id$
*/


struct Material
{
public:
  ssgSimpleState **gst ;

  char *texture_map  ;
  int   clamp_tex    ;
  int   transparency ;
  float alpha_ref    ;
  int   lighting     ;
  float friction     ;
  unsigned int flags ;

  int  isNull () { return gst == NULL ; } ;
  void install ( int index ) ;
  
  ssgState *getState    () { return *gst ; }
  char     *getTexFname () { return texture_map ; }
} ;


void initMaterials () ;
/*Material *getMaterial ( ssgState *s ) ;
Material *getMaterial ( ssgLeaf  *l ) ;*/

extern ssgSimpleState *default_gst, *O_gst, *X_gst, *ground_gst, *ctrls_gst ;

