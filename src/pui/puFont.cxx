
#include "puLocal.h"

/*
  A set of puFont's to implement the GLUT BitMap set...
*/

puFont PUFONT_8_BY_13        ( GLUT_BITMAP_8_BY_13        ) ;
puFont PUFONT_9_BY_15        ( GLUT_BITMAP_9_BY_15        ) ;
puFont PUFONT_TIMES_ROMAN_10 ( GLUT_BITMAP_TIMES_ROMAN_10 ) ;
puFont PUFONT_TIMES_ROMAN_24 ( GLUT_BITMAP_TIMES_ROMAN_24 ) ;
puFont PUFONT_HELVETICA_10   ( GLUT_BITMAP_HELVETICA_10   ) ;
puFont PUFONT_HELVETICA_12   ( GLUT_BITMAP_HELVETICA_12   ) ;
puFont PUFONT_HELVETICA_18   ( GLUT_BITMAP_HELVETICA_18   ) ;



int puFont::getStringWidth ( char *str )
{
  if ( str == NULL )
    return 0 ;

  if ( glut_font_handle != (GlutFont) 0 )
  {
    int res = 0 ;

    while ( *str != '\0' )
    {
      res += glutBitmapWidth ( glut_font_handle, *str ) ;
      str++ ;
    }

    return res ;
  }

  if ( fnt_font_handle != NULL )
  {
    float r, l ;
    fnt_font_handle -> getBBox ( str, pointsize, slant, &l, &r, NULL, NULL ) ;
    return (int) ( r - l ) ;
  }

  return 0 ;
}


static int getGLUTStringHeight ( GlutFont glut_font_handle )
{
  if ( glut_font_handle == GLUT_BITMAP_8_BY_13        ) return  9 ;
  if ( glut_font_handle == GLUT_BITMAP_9_BY_15        ) return 10 ;
  if ( glut_font_handle == GLUT_BITMAP_TIMES_ROMAN_10 ) return  7 ;
  if ( glut_font_handle == GLUT_BITMAP_TIMES_ROMAN_24 ) return 17 ;
  if ( glut_font_handle == GLUT_BITMAP_HELVETICA_10   ) return  8 ;
  if ( glut_font_handle == GLUT_BITMAP_HELVETICA_12   ) return  9 ;
  if ( glut_font_handle == GLUT_BITMAP_HELVETICA_18   ) return 14 ;
  return 0 ;
}

int puFont::getStringHeight ( char *s )
{
  /* Height *excluding* descender */
   
  if ( glut_font_handle != (GlutFont) 0 )
  {
    int i = getGLUTStringHeight ( glut_font_handle ) ;
    int num_lines = 1 ;

    for ( char *p = s ; *p != '\0' ; p++ )
      if ( *p == '\n' )
        num_lines++ ;

    return i * num_lines ;
  }

  if ( fnt_font_handle != NULL )
  {
    float t, b ;
    fnt_font_handle -> getBBox ( s, pointsize, slant, NULL, NULL, &b, &t ) ;
    return (int) ( t - b ) ;
  }

  return 0 ;
}


int puFont::getStringHeight ()
{
  return getStringHeight ( "K" ) ;
}


int puFont::getStringDescender ()
{
  if ( glut_font_handle != (GlutFont) 0 )
  {
    if ( glut_font_handle == GLUT_BITMAP_8_BY_13        ) return 2 ;
    if ( glut_font_handle == GLUT_BITMAP_9_BY_15        ) return 3 ;
    if ( glut_font_handle == GLUT_BITMAP_TIMES_ROMAN_10 ) return 2 ;
    if ( glut_font_handle == GLUT_BITMAP_TIMES_ROMAN_24 ) return 5 ;
    if ( glut_font_handle == GLUT_BITMAP_HELVETICA_10   ) return 2 ;
    if ( glut_font_handle == GLUT_BITMAP_HELVETICA_12   ) return 3 ;
    if ( glut_font_handle == GLUT_BITMAP_HELVETICA_18   ) return 4 ;
    return 0 ;
  }

  if ( fnt_font_handle != NULL )
  {
    float b ;
    fnt_font_handle -> getBBox ( "y", pointsize, slant, NULL, NULL, &b, NULL ) ;
    return (int) -b ;
  }

  return 0 ;
}


void puFont::drawString ( char *str, int x, int y )
{
  if ( str == NULL )
    return ;

  if ( glut_font_handle != (GlutFont) 0 )
  {
    glRasterPos2f( (float)x, (float)y ) ; 

    while ( *str != '\0' )
    {
      if (*str == '\n')
      {
        y -= getStringHeight() * 4 / 3 ;
    	glRasterPos2f( (float)x, (float)y ) ; 
      }
      else
        glutBitmapCharacter ( glut_font_handle, *str ) ;

      str++ ;
    }
    return ;
  }

  if ( fnt_font_handle != NULL )
  {
    sgVec3 curpos ;
    sgSetVec3 ( curpos, (float)x, (float)y, 0.0f ) ;

    glPushAttrib( GL_COLOR_BUFFER_BIT ); // NHV
      glEnable    ( GL_ALPHA_TEST   ) ;
      glEnable    ( GL_BLEND        ) ;
      glAlphaFunc ( GL_GREATER, 0.1 ) ;
      glBlendFunc ( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA ) ;

      fnt_font_handle -> begin () ;
        fnt_font_handle -> puts ( curpos, pointsize, slant, str ) ;
      fnt_font_handle -> end () ;

    glPopAttrib () ;
    glDisable ( GL_TEXTURE_2D ) ;
    return ;
  }
}

