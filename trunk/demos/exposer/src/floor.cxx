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
  glColor4f ( 1, 1, 1, 1 ) ;

  glBegin ( GL_QUADS ) ;
    glTexCoord2f ( -30, timebox->getGroundPosition() - 30 ) ;
    glVertex3f ( -30, -30, z_coord ) ;
    glTexCoord2f (  30, timebox->getGroundPosition() - 30 ) ;
    glVertex3f (  30, -30, z_coord ) ;
    glTexCoord2f (  30, timebox->getGroundPosition() + 30 ) ;
    glVertex3f (  30,  30, z_coord ) ;
    glTexCoord2f ( -30, timebox->getGroundPosition() + 30 ) ;
    glVertex3f ( -30,  30, z_coord ) ;
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


