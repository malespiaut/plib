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
  GLXFBConfig *configs = glXGetFBConfigs(dpy, scn, &cnt);
  if (!configs)
  {
    perror("glXGetFBConfigs");
    exit(1);
  }
  fprintf(stderr,"glXGetFBConfigs returned %p (%d matches)\n", configs, cnt);
  assert(cnt);
  GLXFBConfig fbc = configs[0];
  XVisualInfo *vinf = glXGetVisualFromFBConfig(dpy, fbc);
  if (!vinf)
  {  
    perror("glXGetVisualFromFBConfig");
    exit(1);
  }
  fprintf
  (
    stderr,
    "visualid=0x%lx (depth=%d,R/G/B=%lx/%lx/%lx)\n",
    vinf->visualid,
    vinf->depth,
    vinf->red_mask,
    vinf->green_mask,
    vinf->blue_mask
  );
  GLXContext cx = glXCreateNewContext(dpy, fbc, GLX_RGBA_TYPE, 0, GL_TRUE);
  if (!cx)
  {
    perror("glXCreateNewContext");
    exit(1);
  }
  else
  {
    fprintf(stderr,"glX context created\n");
  }
  int attrlist[] =
  {
    GLX_PBUFFER_WIDTH, 1,
    GLX_PBUFFER_HEIGHT, 1,
    0
  };
  GLXPbuffer pBuffer = glXCreatePbuffer(dpy, fbc, attrlist);
  fprintf(stderr,"pBuffer = %lx\n", pBuffer);

  bool ok=glXMakeContextCurrent(dpy, pBuffer, pBuffer, cx);
  if (!ok)
  {
    perror("glXMakeContextCurrent");
    exit(1);
  }

  // Initialize plib
  fprintf(stderr,"Initializing plib...\n");
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

