
#include "fntLocal.h"

FILE *curr_image_fd ;
int isSwapped = FALSE ;

static void tex_make_mip_maps ( GLubyte *image, int xsize,
                                     int ysize, int zsize )
{
  GLubyte *texels [ 20 ] ;   /* One element per level of MIPmap */

  for ( int l = 0 ; l < 20 ; l++ )
    texels [ 0 ] = NULL ;

  texels [ 0 ] = image ;

  int lev ;

  for ( lev = 0 ; (( xsize >> (lev+1) ) != 0 ||
                   ( ysize >> (lev+1) ) != 0 ) ; lev++ )
  {
    /* Suffix '1' is the higher level map, suffix '2' is the lower level. */

    int l1 = lev   ;
    int l2 = lev+1 ;
    int w1 = xsize >> l1 ;
    int h1 = ysize >> l1 ;
    int w2 = xsize >> l2 ;
    int h2 = ysize >> l2 ;

    if ( w1 <= 0 ) w1 = 1 ;
    if ( h1 <= 0 ) h1 = 1 ;
    if ( w2 <= 0 ) w2 = 1 ;
    if ( h2 <= 0 ) h2 = 1 ;

    texels [ l2 ] = new GLubyte [ w2 * h2 * zsize ] ;

    for ( int x2 = 0 ; x2 < w2 ; x2++ )
      for ( int y2 = 0 ; y2 < h2 ; y2++ )
        for ( int c = 0 ; c < zsize ; c++ )
        {
          int x1   = x2 + x2 ;
          int x1_1 = ( x1 + 1 ) % w1 ;
          int y1   = y2 + y2 ;
          int y1_1 = ( y1 + 1 ) % h1 ;

	  int t1 = texels [ l1 ] [ (y1   * w1 + x1  ) * zsize + c ] ;
	  int t2 = texels [ l1 ] [ (y1_1 * w1 + x1  ) * zsize + c ] ;
	  int t3 = texels [ l1 ] [ (y1   * w1 + x1_1) * zsize + c ] ;
	  int t4 = texels [ l1 ] [ (y1_1 * w1 + x1_1) * zsize + c ] ;

          texels [ l2 ] [ (y2 * w2 + x2) * zsize + c ] =
                                           ( t1 + t2 + t3 + t4 ) / 4 ;
        }
  }

  texels [ lev+1 ] = NULL ;

  if ( ! ((xsize & (xsize-1))==0) ||
       ! ((ysize & (ysize-1))==0) )
  {
    fprintf ( stderr, "TXFloader: TXF Map is not a power-of-two in size!\n" ) ;
    exit ( 1 ) ;
  }

  glPixelStorei ( GL_UNPACK_ALIGNMENT, 1 ) ;

  int map_level = 0 ;

#ifdef PROXY_TEXTURES_ARE_NOT_BROKEN
  int ww ;

  do
  {
    glTexImage2D  ( GL_PROXY_TEXTURE_2D,
                     map_level, zsize, xsize, ysize, FALSE /* Border */,
                            (zsize==1)?GL_LUMINANCE:
                            (zsize==2)?GL_LUMINANCE_ALPHA:
                            (zsize==3)?GL_RGB:
                                       GL_RGBA,
                            GL_UNSIGNED_BYTE, NULL ) ;

    glGetTexLevelParameteriv ( GL_PROXY_TEXTURE_2D, 0,GL_TEXTURE_WIDTH, &ww ) ;

    if ( ww == 0 )
    {
      delete texels [ 0 ] ;
      xsize >>= 1 ;
      ysize >>= 1 ;

      for ( int l = 0 ; texels [ l ] != NULL ; l++ )
        texels [ l ] = texels [ l+1 ] ;

      if ( xsize < 64 && ysize < 64 )
      {
        fprintf ( stderr,
                "FNT: OpenGL will not accept a font texture?!?\n" ) ;
        exit ( 1 ) ;
      }
    }
  } while ( ww == 0 ) ;
#endif

  for ( int i = 0 ; texels [ i ] != NULL ; i++ )
  {
    int w = xsize>>i ;
    int h = ysize>>i ;

    if ( w <= 0 ) w = 1 ;
    if ( h <= 0 ) h = 1 ;

    glTexImage2D  ( GL_TEXTURE_2D,
                     map_level, zsize, w, h, FALSE /* Border */,
                            (zsize==1)?GL_LUMINANCE:
                            (zsize==2)?GL_LUMINANCE_ALPHA:
                            (zsize==3)?GL_RGB:
                                       GL_RGBA,
                            GL_UNSIGNED_BYTE, (GLvoid *) texels[i] ) ;
    map_level++ ;
    delete texels [ i ] ;
  }
}



int fntTexFont::loadTXF ( char *fname, GLenum mag, GLenum min )
{
  if ( glGetCurrentContext () == NULL )
  {
    fprintf ( stderr,
    "FATAL: FNT font loader called without a valid OpenGL context.\n");
    exit ( 1 ) ;
  }

  FILE *fd ;

  if ( (fd = fopen ( fname, "rb" )) == NULL )
  {
    fprintf ( stderr, "fntLoadTXF: Failed to open '%s' for reading.\n",
                            fname ) ;
    return FNT_FALSE ;
  }

  curr_image_fd = fd ;

  unsigned char magic [ 4 ] ;

  if ( (int)fread ( &magic, sizeof (unsigned int), 1, fd ) != 1 )
  {
    fprintf ( stderr, "fntLoadTXF: '%s' an empty file!\n", fname ) ;
    return FNT_FALSE ;
  }

  if ( magic [ 0 ] != 0xFF || magic [ 1 ] != 't' ||
       magic [ 2 ] != 'x'  || magic [ 3 ] != 'f' )
  {
    fprintf ( stderr, "fntLoadTXF: '%s' is not a 'txf' font file.\n", fname ) ;
    return FNT_FALSE ;
  }

  isSwapped  = FALSE ;
  int endianness  = _fnt_readInt () ;

  isSwapped  = ( endianness != 0x12345678 ) ;

  int format      = _fnt_readInt () ;
  int tex_width   = _fnt_readInt () ;
  int tex_height  = _fnt_readInt () ;
  int max_height  = _fnt_readInt () ;
                    _fnt_readInt () ;
  int num_glyphs  = _fnt_readInt () ;

  int w = tex_width  ;
  int h = tex_height ;

  float xstep = 0.5f / (float) w ;
  float ystep = 0.5f / (float) h ;

  int i, j ;

  /*
    Load the TXF_Glyph array
  */

  TXF_Glyph glyph ;

  for ( i = 0 ; i < num_glyphs ; i++ )
  {
    glyph . ch      = _fnt_readShort () ;

    glyph .  w      = _fnt_readByte  () ;
    glyph .  h      = _fnt_readByte  () ;
    glyph . x_off   = _fnt_readByte  () ;
    glyph . y_off   = _fnt_readByte  () ;
    glyph . step    = _fnt_readByte  () ;
    glyph . unknown = _fnt_readByte  () ;
    glyph . x       = _fnt_readShort () ;
    glyph . y       = _fnt_readShort () ;

    setGlyph ( (char) glyph.ch,
          (float)  glyph.x                 / (float) w + xstep,
          (float)( glyph.x + glyph.w )     / (float) w + xstep,
          (float)  glyph.y                 / (float) h + ystep,
          (float)( glyph.y + glyph.h )     / (float) h + ystep,
          (float)  glyph.x_off             / (float) max_height,
          (float)( glyph.x_off + glyph.w ) / (float) max_height,
          (float)  glyph.y_off             / (float) max_height,
          (float)( glyph.y_off + glyph.h ) / (float) max_height ) ;
  }

  exists [ ' ' ] = FALSE ;

  /*
    Load the image part of the file
  */

  int ntexels = w * h ;

  unsigned char *teximage  ;
  unsigned char *texbitmap ;

  switch ( format )
  {
    case FNT_BYTE_FORMAT:
      {
        unsigned char *orig = new unsigned char [ ntexels ] ;

        if ( (int)fread ( orig, 1, ntexels, fd ) != ntexels )
        {
          fprintf ( stderr, "fntLoadTXF: Premature EOF in '%s'.\n", fname ) ;
          return FNT_FALSE ;
        }

        teximage = new unsigned char [ 2 * ntexels ] ;

        for ( i = 0 ; i < ntexels ; i++ )
        {
	  teximage [ i*2     ] = orig [ i ] ;
	  teximage [ i*2 + 1 ] = orig [ i ] ;
        }

        delete orig ;
      }
      break ;
   
    case FNT_BITMAP_FORMAT:
      {
        int stride = (w + 7) >> 3;

        texbitmap = new unsigned char [ stride * h ] ;

        if ( (int)fread ( texbitmap, 1, stride * h, fd ) != stride * h )
        {
          delete texbitmap ;
          fprintf ( stderr, "fntLoadTXF: Premature EOF in '%s'.\n", fname ) ;
          return FNT_FALSE ;
	}

        teximage = new unsigned char [ 2 * ntexels ] ;

        for ( i = 0 ; i < 2 * ntexels ; i++ )
	  teximage [ i ] = 0 ;

        for (i = 0; i < h; i++)
	  for (j = 0; j < w; j++)
	    if (texbitmap[i * stride + (j >> 3)] & (1 << (j & 7)))
	    {
	      teximage[(i * w + j) * 2    ] = 255;
	      teximage[(i * w + j) * 2 + 1] = 255;
	    }

        delete texbitmap ;
      }
      break ;

    default:
      fprintf ( stderr, "fntLoadTXF: Unrecognised format type in '%s'.\n",
                          fname ) ;
      return FNT_FALSE ;
  }

  fclose ( fd ) ;

  fixed_pitch = FALSE ;

#ifdef GL_VERSION_1_1
  glGenTextures ( 1, & texture ) ;
  glBindTexture ( GL_TEXTURE_2D, texture ) ;
#else
  /* This is only useful on some ancient SGI hardware */
  glGenTexturesEXT ( 1, & texture ) ;
  glBindTextureEXT ( GL_TEXTURE_2D, texture ) ;
#endif

  tex_make_mip_maps ( teximage, w, h, 2 ) ;

  glTexEnvi ( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE ) ;

  glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag ) ;
  glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min ) ;
  glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP ) ;
  glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP ) ;
#ifdef GL_VERSION_1_1
  glBindTexture ( GL_TEXTURE_2D, 0 ) ;
#else
  glBindTextureEXT ( GL_TEXTURE_2D, 0 ) ;
#endif
  
  return FNT_TRUE ;
}


