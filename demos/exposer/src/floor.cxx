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

static unsigned char Texture0 [] =
{
  0, 0, 0, 0, 255, 255, 255, 255,
  0, 0, 0, 0, 255, 255, 255, 255,
  0, 0, 0, 0, 255, 255, 255, 255,
  0, 0, 0, 0, 255, 255, 255, 255,
  255, 255, 255, 255, 0, 0, 0, 0,
  255, 255, 255, 255, 0, 0, 0, 0,
  255, 255, 255, 255, 0, 0, 0, 0,
  255, 255, 255, 255, 0, 0, 0, 0
} ;

static unsigned char Texture1 [] =
{
  0, 0, 255, 255,
  0, 0, 255, 255,
  255, 255, 0, 0,
  255, 255, 0, 0
} ;

static unsigned char Texture2 [] =
{
  0, 255,
  255, 0
} ;

static unsigned char Texture3 [] =
{
  127
} ;


void Floor::draw ()
{
  glMatrixMode ( GL_PROJECTION) ; _ssgCurrentContext->loadProjectionMatrix() ;
  glMatrixMode ( GL_MODELVIEW ) ; _ssgCurrentContext->loadModelviewMatrix () ;

  glDisable ( GL_LIGHTING   ) ;
  glEnable  ( GL_TEXTURE_2D ) ;
  glEnable  ( GL_CULL_FACE  ) ;
  glBindTexture ( GL_TEXTURE_2D, texhandle ) ;
  glColor4f ( 1.0f, 1.0f, 1.0f, 1.0f ) ;

  glBegin ( GL_QUADS ) ;
    glTexCoord2f ( -30.0f, timebox->getGroundPosition() - 30.0f ) ;
    glVertex3f ( -30.0f, -30.0f, z_coord ) ;
    glTexCoord2f (  30.0f, timebox->getGroundPosition() - 30.0f ) ;
    glVertex3f (  30.0f, -30.0f, z_coord ) ;
    glTexCoord2f (  30.0f, timebox->getGroundPosition() + 30.0f ) ;
    glVertex3f (  30.0f,  30.0f, z_coord ) ;
    glTexCoord2f ( -30.0f, timebox->getGroundPosition() + 30.0f ) ;
    glVertex3f ( -30.0f,  30.0f, z_coord ) ;
  glEnd () ;

  glDisable ( GL_TEXTURE_2D ) ;
  glEnable  ( GL_LIGHTING   ) ;
}


Floor::Floor ()
{
  glGenTextures   ( 1, & texhandle ) ;

  glBindTexture   ( GL_TEXTURE_2D, texhandle ) ;
    glTexEnvi       ( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE ) ;
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST ) ;
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
					      GL_LINEAR_MIPMAP_LINEAR ) ;
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT ) ;
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT ) ;
    glTexImage2D  ( GL_TEXTURE_2D, 0, 1, 8, 8,
		    FALSE, GL_LUMINANCE, GL_UNSIGNED_BYTE, Texture0 ) ;
    glTexImage2D  ( GL_TEXTURE_2D, 1, 1, 4, 4,
		    FALSE, GL_LUMINANCE, GL_UNSIGNED_BYTE, Texture1 ) ;
    glTexImage2D  ( GL_TEXTURE_2D, 2, 1, 2, 2,
		    FALSE, GL_LUMINANCE, GL_UNSIGNED_BYTE, Texture2 ) ;
    glTexImage2D  ( GL_TEXTURE_2D, 3, 1, 1, 1,
		    FALSE, GL_LUMINANCE, GL_UNSIGNED_BYTE, Texture3 ) ;
  glBindTexture ( GL_TEXTURE_2D, 0 ) ;
}


