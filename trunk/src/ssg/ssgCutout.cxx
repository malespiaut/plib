
#include "ssgLocal.h"

void ssgCutout::copy_from ( ssgCutout *src, int clone_flags )
{
  ssgBranch::copy_from ( src, clone_flags ) ;
  point_rotate = src -> isPointRotate () ;
}

ssgCutout *ssgCutout::clone ( int clone_flags )
{
  ssgCutout *b = new ssgCutout ;
  b -> copy_from ( this, clone_flags ) ;
  return b ;
}


ssgCutout::ssgCutout (int pntrot)
{
  point_rotate = pntrot ;
  type |= SSG_TYPE_CUTOUT ;
}

ssgCutout::~ssgCutout ()
{
}


void ssgCutout::cull ( sgFrustum *f, sgMat4 m, int test_needed )
{
  if ( ( getTraversalMask () & SSGTRAV_CULL ) == 0 )
    return ;

  sgMat4 tmp ;

  if ( point_rotate )
  {
    /*
      Trash the current viewpoint transform and replace it
      with one that only contains the OpenGL axis swap
      kludge along with the current translation.

      This prevents the object from rotating relative to the
      screen so that the modelled X/Z axis are always parallel
      to the screen X/Y axes.
    */

    sgCopyMat4 ( tmp, _ssgOpenGLAxisSwapMatrix ) ;
    sgCopyVec3 ( tmp[3], m[3] ) ;
  }
  else
  {
    sgCopyMat4 ( tmp, m ) ;

    /*
      Figure out where the Z axis of the model ends up - that
      isn't changed by the billboarding process.

      Next, figure out where the X axis should go - which
      will be at right angles to the Z axis - and at right
      angles to the view vector. A cross product achieves
      that.

      Finally, figure out where the Y axis should go - which
      will be at right angles to the new X and Z axes. A second
      cross product sorts that out.

      Notice that the SSG coordinate system's Z axis is really
      GL's Y axis.
    */

    sgVec3 x_axis ;
    sgVec3 y_axis ;
    sgVec3 z_axis ;

    sgSetVec3  ( y_axis, 0.0f, 0.0f, -1.0f ) ;
    sgCopyVec3 ( z_axis, tmp[ 2 ] ) ;

    sgVectorProductVec3 ( x_axis, y_axis, z_axis ) ;
    sgVectorProductVec3 ( y_axis, z_axis, x_axis ) ;

    sgNormaliseVec3 ( x_axis ) ;
    sgNormaliseVec3 ( y_axis ) ;  /* Optional if your cutout is flat */

    /*
      Now we know where we want the three axes to end up,
      change the matrix to make it so.
    */

    sgCopyVec3 ( tmp[0], x_axis ) ;
    sgCopyVec3 ( tmp[1], y_axis ) ;
  }

  _ssgPushMatrix ( tmp ) ;

  glPushMatrix   () ;
  glLoadMatrixf  ( (float *) tmp ) ;
  ssgBranch::cull ( f, tmp, test_needed ) ;
  glPopMatrix () ;

  _ssgPopMatrix  () ;
}


void ssgCutout::hot ( sgVec3 s, sgMat4 m, int test_needed )
{
  ssgBranch::hot ( s, m, test_needed ) ;
}



void ssgCutout::isect ( sgSphere *s, sgMat4 m, int test_needed )
{
  ssgBranch::isect ( s, m, test_needed ) ;
}



int ssgCutout::load ( FILE *fd )
{
  _ssgReadInt ( fd, & point_rotate ) ;
  return ssgBranch::load(fd) ;
}

int ssgCutout::save ( FILE *fd )
{
  _ssgWriteInt ( fd, point_rotate ) ;
  return ssgBranch::save(fd) ;
}



