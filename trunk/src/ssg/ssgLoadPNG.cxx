
#include "ssgLocal.h"

#ifdef SSG_LOAD_PNG_SUPPORTED

#include <gl/glpng.h>

bool ssgLoadPNG ( const char *fname, ssgTextureInfo* info )
{
  pngInfo png_info;
  if (!pngLoad(fname, PNG_BUILDMIPMAP, PNG_ALPHA, &png_info)) {
    ulSetError ( UL_WARNING, "ssgLoadTexture: Failed to load '%s'.", fname ) ;
    return false ;
  }
  if ( info != NULL )
  {
    info -> width = png_info.Width ;
    info -> height = png_info.Height ;
    info -> depth = png_info.Depth ;
    info -> alpha = png_info.Alpha ;
  }
  return true ;
}

#else

bool ssgLoadPNG ( const char *fname, ssgTextureInfo* info )
{
  ulSetError ( UL_WARNING, "ssgLoadTexture: '%s' - you need glpng for PNG format support",
        fname ) ;
  return false ;
}

#endif
