
#include "ssgLocal.h"

void ssgTexture::copy_from ( ssgTexture *src, int clone_flags )
{
  ssgBase::copy_from ( src, clone_flags ) ;
  setFilename ( src -> getFilename () ) ;
  setFilenameFromModel ( src -> getFilenameFromModel () ) ;
  handle = src -> getHandle () ;

  /*
    If clone_flags is SSG_CLONE_TEXTURE then we should
    extract the texels from OpenGL and recreate them.
    ...someday...
  */
}

ssgBase *ssgTexture::clone ( int clone_flags )
{
  ssgTexture *b = new ssgTexture ;
  b -> copy_from ( this, clone_flags ) ;
  return b ;
}


ssgTexture::ssgTexture ()
{
  type |= SSG_TYPE_TEXTURE ;
  filename = NULL ;
  handle = 0 ;
}

void ssgTexture::print ( FILE *fd, char *ident, int how_much )
{
  fprintf ( fd, "%s%s: %s\n", ident, getTypeName (), getFilename () ) ;
}


int ssgTexture::load ( FILE *fd )
{
  _ssgReadUInt ( fd, & handle ) ;
  return ssgBase::load ( fd ) ;
}


int ssgTexture::save ( FILE *fd )
{
  _ssgWriteUInt ( fd, handle ) ;
  return ssgBase::save ( fd ) ;
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


ssgTexture::ssgTexture ( const char *fname, GLubyte *image, int xsize, int ysize, int zsize, 
		     int wrapu, int wrapv )
// fname is used when saving the ssgSimpleState to disc.
// If there is no associated file, you can use a dummy name.
{
  type |= SSG_TYPE_TEXTURE ;

#ifdef GL_VERSION_1_1
  glGenTextures ( 1, & handle ) ;
  glBindTexture ( GL_TEXTURE_2D, handle ) ;
#else
  /* This is only useful on some ancient SGI hardware */
  glGenTexturesEXT ( 1, & handle ) ;
  glBindTextureEXT ( GL_TEXTURE_2D, handle ) ;
#endif

  filename = NULL ;
	filename_from_model = NULL;
  setFilename ( fname ) ;
	
  //ssgLoadTexture( getFilename() ) ;
	make_mip_maps ( image, xsize, ysize, zsize );
	setDefaultGlParams(wrapu, wrapv, TRUE);
}




ssgTexture::ssgTexture ( const char *fname, int wrapu, int wrapv,
	     int mipmap )
{
  type |= SSG_TYPE_TEXTURE ;

#ifdef GL_VERSION_1_1
  glGenTextures ( 1, & handle ) ;
  glBindTexture ( GL_TEXTURE_2D, handle ) ;
#else
  /* This is only useful on some ancient SGI hardware */
  glGenTexturesEXT ( 1, & handle ) ;
  glBindTextureEXT ( GL_TEXTURE_2D, handle ) ;
#endif

  filename = NULL ;
	filename_from_model = NULL;
  setFilename ( fname ) ;
	
  ssgLoadTexture( getFilename() ) ;
	setDefaultGlParams(wrapu, wrapv, mipmap);
}
