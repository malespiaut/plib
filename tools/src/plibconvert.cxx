// plibconvert.cxx
//
// Written by Bram Stolk
// Published as part of Steve Baker's PLIB under GPL
//
// Build with :
// g++ -o plibconvert plibconvert.cxx -lplibssg -lplibsg -lplibul -lGL


#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <GL/glx.h>

#include <plib/ssg.h>


int main(int argc, char *argv[])
{
  if (argc!=3)
  {
    fprintf(stderr,"Usage: %s source destination\n", argv[0]);
    exit(1);
  }

  assert(strcmp(argv[1], argv[2]));

#ifdef GLX_VERSION_1_3
  // Although we will not be doing any rendering, we must create
  // a valid rendering context before we can call ssgInit()
  Display *dpy = XOpenDisplay(0);
  int scn=DefaultScreen(dpy);
  int cnt;
  GLXFBConfig *cfg = glXGetFBConfigs(dpy, scn, &cnt);
  fprintf(stderr,"glXGetFBConfigs returned %p (%d matches)\n", cfg, cnt);
  assert(cnt);
  int attrlist[] =
  {
    GLX_PBUFFER_WIDTH, 1,
    GLX_PBUFFER_HEIGHT, 1,
    0
  };
  GLXPbufferSGIX pBuffer = glXCreatePbuffer(dpy, cfg[0], attrlist);
  fprintf(stderr,"pBuffer = %p\n", pBuffer);
  GLXContext cx = glXCreateNewContext(dpy, cfg[0], GLX_RGBA_TYPE, 0, GL_TRUE);
  glXMakeCurrent(dpy, pBuffer, cx);

  // Initialize plib
  ssgInit();
  ssgTexturePath ( "." ) ;
  ssgModelPath   ( "." ) ;

  ssgEntity *e = ssgLoad(argv[1]);	// Load
  e->print(stdout);			// Inform
  ssgSave(argv[2], e);			// Store
#else
  fprintf(stderr,"This util uses a pbuffer as context for plib. To build the source, you need glx protocol 1.3 or higher.\n");
#endif
}

