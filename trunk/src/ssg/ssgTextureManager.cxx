
#include "ssgLocal.h"


ssgTextureManager* ssgTextureManager::current = 0 ;


void ssgTextureManager::make_mip_maps ( GLubyte *image, int xsize, int ysize, int zsize )
{
  if ( ! ((xsize & (xsize-1))==0) ||
       ! ((ysize & (ysize-1))==0) )
  {
    ulSetError ( UL_WARNING, "Map is not a power-of-two in size!" ) ;
    loadDummy () ;
    return ;
  }

  GLubyte *texels [ 20 ] ;   /* One element per level of MIPmap */

  for ( int l = 0 ; l < 20 ; l++ )
    texels [ l ] = NULL ;

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
        ulSetError ( UL_FATAL,
           "SSG: OpenGL will not accept a downsized version ?!?" ) ;
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

    total_texels_loaded += w * h ;

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


void ssgTextureManager::loadDummy ()
{
  GLubyte *image = new GLubyte [ 4 * 3 ] ;

  /* Red and white chequerboard */

  image [ 0 ] = 255 ; image [ 1 ] =  0  ; image [ 2 ] =  0  ;
  image [ 3 ] = 255 ; image [ 4 ] = 255 ; image [ 5 ] = 255 ;
  image [ 6 ] = 255 ; image [ 7 ] = 255 ; image [ 8 ] = 255 ;
  image [ 9 ] = 255 ; image [ 10] =  0  ; image [ 11] =  0  ;

  make_mip_maps ( image, 2, 2, 3 ) ;
}


void ssgTextureManager::addFormat ( const char* extension,
  void (*loadfunc) ( const char* fname ) )
{
  if ( num_formats < MAX_FORMATS )
  {
    formats [ num_formats ] . extension = extension ;
    formats [ num_formats ] . loadfunc = loadfunc ;
    num_formats ++ ;
  }
  else
  {
    ulSetError ( UL_WARNING, "ssgTextureManager::addFormat: too many formats" );
  }
}


void ssgTextureManager::load ( const char *fname )
{
  setAlphaFlag ( FALSE ) ;

  if ( fname == NULL || *fname == '\0' )
    return ;

  //find extension
  const char *extn = & ( fname [ strlen(fname) ] ) ;
  while ( extn != fname && *extn != '/' && *extn != '.' )
    extn-- ;

  if ( *extn != '.' )
  {
    ulSetError ( UL_WARNING, "ssgLoadTexture: Cannot determine file type for '%s'", fname );
    return ;
  }

  for ( _ssgTextureFormat *f = formats; f->extension != NULL; f++ )
    if ( f->loadfunc != NULL &&
         _ssgStrNEqual ( extn, f->extension, strlen(f->extension) ) )
    {
      f->loadfunc( fname ) ;
      return ;
    }

  ulSetError ( UL_WARNING, "ssgLoadTexture: Unrecognised file type '%s'", extn ) ;
  loadDummy () ;
}


void ssgTextureManager::clear ()
{
  for ( int i = 0; i < num_shared_textures; i++ )
    ssgDeRefDelete ( shared_textures[i] ) ;
  num_shared_textures = 0 ;
}


void ssgTextureManager::add ( ssgTexture* tex )
{
  if ( tex && num_shared_textures < MAX_SHARED_TEXTURES )
  {
    tex -> ref() ;
    shared_textures [ num_shared_textures++ ] = tex ;
  }
}


ssgTexture* ssgTextureManager::find ( const char* fname )
{
  for ( int i = 0 ; i < num_shared_textures ; i++ )
  {
    ssgTexture *tex = shared_textures [ i ] ;
    if ( _ssgStrEqual ( fname, tex->getFilename() ) )
	    return tex ;
  }
  return NULL ;
}
