
#include "ssgLocal.h"
#ifdef WIN32
#include <windows.h>
#endif

#include <GL/gl.h>

void ssgBaseTransform::copy_from ( ssgBaseTransform *src, int clone_flags )
{
  ssgBranch::copy_from ( src, clone_flags ) ;

  src -> getTransform     ( transform ) ;
  src -> getLastTransform ( last_transform ) ;
  last_updated = src -> last_updated ;
  first_time   = src -> first_time   ;
}



ssgBaseTransform::ssgBaseTransform (void)
{
  type |= SSG_TYPE_BASETRANSFORM ;
  last_updated = -9999999 ;
  first_time = TRUE ;
  sgMakeIdentMat4 ( transform ) ;
  sgMakeIdentMat4 ( last_transform ) ;
}

ssgBaseTransform::~ssgBaseTransform (void)
{
}


int ssgBaseTransform::load ( FILE *fd )
{
  _ssgReadMat4 ( fd, transform ) ;
  updateTransform () ;
  first_time = TRUE ;
  return ssgBranch::load(fd) ;
}

int ssgBaseTransform::save ( FILE *fd )
{
  _ssgWriteMat4 ( fd, transform ) ;
  return ssgBranch::save(fd) ;
}


