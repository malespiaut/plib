
#include "ssgLocal.h"


void ssgTexture::copy_from ( ssgTexture *src, int clone_flags )
{
  ssgBase::copy_from ( src, clone_flags ) ;

  wrapu = src -> wrapu ;
  wrapv = src -> wrapv ;
  mipmap = src -> mipmap ;

  setFilename ( src -> getFilename () ) ;

  alloc_handle () ;
  ssgLoadTexture( filename ) ;
  has_alpha = _ssgAlphaFlag ;
	setDefaultGlParams(wrapu, wrapv, mipmap);
}


ssgBase *ssgTexture::clone ( int clone_flags )
{
  ssgTexture *b = new ssgTexture ;
  b -> copy_from ( this, clone_flags ) ;
  return b ;
}


extern void make_mip_maps ( GLubyte *image, int xsize, int ysize, int zsize );

void ssgTexture::setDefaultGlParams(int wrapu, int wrapv, int mipmap)
{
  glTexEnvi ( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE ) ;
  
  glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR ) ;
  glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
    mipmap ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR ) ;
  glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapu ? GL_REPEAT : GL_CLAMP ) ;
  glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapv ? GL_REPEAT : GL_CLAMP ) ;
#ifdef GL_VERSION_1_1
  glBindTexture ( GL_TEXTURE_2D, 0 ) ;
#else
  glBindTextureEXT ( GL_TEXTURE_2D, 0 ) ;
#endif
}


ssgTexture::ssgTexture ()
{
  type |= SSG_TYPE_TEXTURE ;

  filename = NULL ;
  filename_from_model = NULL ;

  own_handle = FALSE ;
  handle = 0 ;

	wrapu = TRUE ;
  wrapv = TRUE ;
  mipmap = TRUE ;
  has_alpha = false ;
}


ssgTexture::ssgTexture ( const char *fname, GLubyte *image, int xsize, int ysize, int zsize, 
		     int _wrapu, int _wrapv )
// fname is used when saving the ssgSimpleState to disc.
// If there is no associated file, you can use a dummy name.
{
  type |= SSG_TYPE_TEXTURE ;

  filename = NULL ;
  filename_from_model = NULL ;

  own_handle = FALSE ;
  handle = 0 ;

	wrapu = _wrapu ;
  wrapv = _wrapv ;
  mipmap = TRUE ;

  setFilename ( fname ) ;

  alloc_handle () ;
  //ssgLoadTexture( filename ) ;
  has_alpha = (zsize == 4) ;
	make_mip_maps ( image, xsize, ysize, zsize );
	setDefaultGlParams(wrapu, wrapv, TRUE);
}


ssgTexture::ssgTexture ( const char *fname, int _wrapu, int _wrapv, int _mipmap )
{
  type |= SSG_TYPE_TEXTURE ;

  filename = NULL ;
  filename_from_model = NULL ;

  own_handle = FALSE ;
  handle = 0 ;

	wrapu = _wrapu ;
  wrapv = _wrapv ;
  mipmap = _mipmap ;

  setFilename ( fname ) ;

  alloc_handle () ;
  ssgLoadTexture( filename ) ;
  has_alpha = _ssgAlphaFlag ;
	setDefaultGlParams(wrapu, wrapv, mipmap);
}


ssgTexture::~ssgTexture (void)
{
  free_handle () ;
}


void ssgTexture::alloc_handle ()
{
  free_handle () ;

  own_handle = TRUE ;

#ifdef GL_VERSION_1_1
  glGenTextures ( 1, & handle ) ;
  glBindTexture ( GL_TEXTURE_2D, handle ) ;
#else
  /* This is only useful on some ancient SGI hardware */
  glGenTexturesEXT ( 1, & handle ) ;
  glBindTextureEXT ( GL_TEXTURE_2D, handle ) ;
#endif
}


void ssgTexture::free_handle ()
{
  if ( handle != 0 )
  {
    if ( own_handle )
    {
#ifdef GL_VERSION_1_1
      glDeleteTextures ( 1, & handle ) ;
#else
      /* This is only useful on some ancient SGI hardware */
      glDeleteTexturesEXT ( 1, & handle ) ;
#endif
    }

    own_handle = FALSE ;
    handle = 0 ;
  }
}


void ssgTexture::setHandle ( GLuint _handle )
{
  free_handle () ;

  own_handle = FALSE ;
  handle = _handle ;
}


void ssgTexture::print ( FILE *fd, char *ident, int how_much )
{
  fprintf ( fd, "%s%s: %s\n", ident, getTypeName (), getFilename () ) ;
}


int ssgTexture::load ( FILE *fd )
{
  delete filename ;

  _ssgReadString ( fd, & filename ) ;
  _ssgReadInt    ( fd, & wrapu    ) ;
  _ssgReadInt    ( fd, & wrapv    ) ;
  _ssgReadInt    ( fd, & mipmap   ) ;

  alloc_handle () ;
  ssgLoadTexture( filename ) ;
  has_alpha = _ssgAlphaFlag ;
	setDefaultGlParams(wrapu, wrapv, mipmap);

  return ssgBase::load ( fd ) ;
}


int ssgTexture::save ( FILE *fd )
{
  _ssgWriteString ( fd, filename ) ;
  _ssgWriteInt    ( fd, wrapu    ) ;
  _ssgWriteInt    ( fd, wrapv    ) ;
  _ssgWriteInt    ( fd, mipmap   ) ;

  return ssgBase::save ( fd ) ;
}
