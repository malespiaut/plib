
#include "ssgLocal.h"

void ssgAxisTransform::copy_from ( ssgAxisTransform *src, int clone_flags ) 
{
  ssgBaseTransform::copy_from ( src, clone_flags ) ;

  sgCopyVec3 ( rotation_axis  , src->rotation_axis   ) ;
  sgCopyVec3 ( rotation_center, src->rotation_center ) ;
  limit_low  = src->limit_low  ;
  limit_high = src->limit_high ;
}

ssgBase *ssgAxisTransform::clone ( int clone_flags )
{
  ssgAxisTransform *b = new ssgAxisTransform ;
  b -> copy_from ( this, clone_flags ) ;
  return b;
}

ssgAxisTransform::ssgAxisTransform (void)
{
  type |= SSG_TYPE_AXISTRANSFORM ;
  sgZeroVec3 ( rotation_axis ) ;
  limit_low = limit_high = 0.f;  
}

ssgAxisTransform::ssgAxisTransform ( sgVec3 axis, sgVec3 center )
{
  type |= SSG_TYPE_AXISTRANSFORM ;
  setAxis   ( axis   );
  setCenter ( center );
  limit_low = limit_high = 0.f;
}

ssgAxisTransform::~ssgAxisTransform (void)
{
}

void ssgAxisTransform::setAxis ( sgVec3 axis )
{
  sgCopyVec3 ( rotation_axis, axis ) ;
}

float *ssgAxisTransform::getAxis (void)
{
  return rotation_axis;
}

void ssgAxisTransform::setCenter ( sgVec3 center )
{
  sgCopyVec3 ( rotation_center, center ) ;
}

float *ssgAxisTransform::getCenter (void)
{
  return rotation_center;
}

void ssgAxisTransform::setRotation ( float rot )
{
  sgMat4 rot_mat ;
  sgMat4 trans1, trans2;
  sgVec3 neg_center;

  sgNegateVec3 ( neg_center, rotation_center ) ;

  sgMakeTransMat4 ( trans1 , neg_center      ) ;
  sgMakeTransMat4 ( trans2 , rotation_center ) ;
  sgMakeRotMat4   ( rot_mat, rot, rotation_axis ) ;
  sgPreMultMat4  ( rot_mat, trans1 ) ;
  sgPostMultMat4 ( rot_mat, trans2 ) ;

  setTransform ( rot_mat ) ;
}

void ssgAxisTransform::setRotationLimits ( float low, float high )
{
  limit_low  = low  ;
  limit_high = high ;
}

void ssgAxisTransform::setLinearRotation ( float rot )
{
  assert( rot >= 0.0f && rot <= 1.0f );
  setRotation( limit_low + (limit_high - limit_low) * rot ) ;
}

int ssgAxisTransform::load ( FILE *fd )
{
  _ssgReadFloat ( fd, 3, rotation_axis   ) ;
  _ssgReadFloat ( fd, 3, rotation_center ) ;
  _ssgReadFloat ( fd, & limit_low  ) ;
  _ssgReadFloat ( fd, & limit_high ) ;
  return ssgTransform::load(fd) ;
}

int ssgAxisTransform::save ( FILE *fd )
{
  _ssgWriteFloat ( fd, 3, rotation_axis   ) ;
  _ssgWriteFloat ( fd, 3, rotation_center ) ;
  _ssgWriteFloat ( fd, limit_low  ) ;
  _ssgWriteFloat ( fd, limit_high ) ;
  return ssgTransform::save(fd) ;
}
