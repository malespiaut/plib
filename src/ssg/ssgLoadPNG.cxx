
#include "ssgLocal.h"

#ifdef SSG_LOAD_PNG_SUPPORTED

#include "glpng.h"

bool ssgLoadPNG ( const char *fname, ssgTextureInfo* info )
{
  pngInfo info;
  if (!pngLoad(fname, PNG_BUILDMIPMAP, PNG_ALPHA, &info)) {
    ulSetError ( UL_WARNING, "ssgLoadTexture: Failed to load '%s'.", fname ) ;
    return false ;
  }
  if ( info != NULL )
  {
    info -> width = info.Width ;
    info -> height = info.Height ;
    info -> depth = info.Depth ;
    info -> alpha = info.Alpha ;
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
