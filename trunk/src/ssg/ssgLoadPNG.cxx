
#include "ssgLocal.h"

#ifdef SSG_LOAD_PNG_SUPPORTED
#include "glpng.h"
#endif

void ssgLoadPNG ( const char *fname )
{
#ifdef SSG_LOAD_PNG_SUPPORTED
  pngInfo info;
  if (!pngLoad(fname, PNG_BUILDMIPMAP, PNG_ALPHA, &info)) {
    ulSetError ( UL_WARNING, "ssgLoadTexture: Failed to load '%s'.", fname ) ;
    return ;
  }
  _ssgSetTextureAlphaFlag ( info.Alpha > 0 ) ;
#else
  ulSetError ( UL_WARNING, "ssgLoadTexture: '%s' - you need glpng for PNG format support",
        fname ) ;

  ssgLoadDummyTexture () ;
#endif
}
