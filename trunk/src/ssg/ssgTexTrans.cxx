
#include "ssgLocal.h"
#ifdef WIN32
#include <windows.h>
#endif

#include <GL/gl.h>

void ssgTexTrans::copy_from ( ssgTexTrans *src, int clone_flags )
{
  ssgBaseTransform::copy_from ( src, clone_flags ) ;
}

ssgTexTrans *ssgTexTrans::clone ( int clone_flags )
{
  ssgTexTrans *b = new ssgTexTrans ;
  b -> copy_from ( this, clone_flags ) ;
  return b ;
}


ssgTexTrans::ssgTexTrans ( sgCoord *c )
{
  type |= SSG_TYPE_TEXTRANS ;
  setTransform ( c ) ;
}

ssgTexTrans::ssgTexTrans (void)
{
  type |= SSG_TYPE_TEXTRANS ;
}

ssgTexTrans::~ssgTexTrans (void)
{
}

void ssgTexTrans::setTransform ( sgVec3 xyz )
{
  sgMakeTransMat4 ( transform, xyz ) ;
}

void ssgTexTrans::setTransform ( sgCoord *xform, float sx, float sy, float sz  )
{
  sgMakeCoordMat4 ( transform, xform ) ;
  sgScaleVec3 ( transform[0], sx ) ;
  sgScaleVec3 ( transform[1], sy ) ;
  sgScaleVec3 ( transform[2], sz ) ;
}

void ssgTexTrans::setTransform ( sgCoord *xform )
{
  sgMakeCoordMat4 ( transform, xform ) ;
}

void ssgTexTrans::setTransform ( sgMat4 xform )
{
  sgCopyMat4 ( transform, xform ) ;
}

void ssgTexTrans::cull ( sgFrustum *f, sgMat4 m, int test_needed )
{
  int cull_result = cull_test ( f, m, test_needed ) ;

  if ( cull_result == SSG_OUTSIDE )
    return ;

  _ssgLoadTexMatrix ( transform ) ;
  glMatrixMode ( GL_TEXTURE ) ;
  glLoadMatrixf ( (float *) transform ) ;
  glMatrixMode ( GL_MODELVIEW ) ;

  for ( ssgEntity *e = getKid ( 0 ) ; e != NULL ; e = getNextKid() )
    e -> cull ( f, m, cull_result != SSG_INSIDE ) ;

  glMatrixMode ( GL_TEXTURE ) ;
  glLoadIdentity () ;
  glMatrixMode ( GL_MODELVIEW ) ;
  _ssgUnloadTexMatrix () ;
}



int ssgTexTrans::load ( FILE *fd )
{
  return ssgBaseTransform::load(fd) ;
}

int ssgTexTrans::save ( FILE *fd )
{
  return ssgBaseTransform::save(fd) ;
}



