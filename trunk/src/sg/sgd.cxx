
#include <stdio.h>
#include "sg.h"

#ifndef TRUE
#define TRUE  1
#define FALSE 0
#endif

void sgdVectorProductVec3 ( sgdVec3 dst, const sgdVec3 a, const sgdVec3 b )
{
  dst[0] = a[1] * b[2] - a[2] * b[1] ;
  dst[1] = a[2] * b[0] - a[0] * b[2] ;
  dst[2] = a[0] * b[1] - a[1] * b[0] ;
}

inline SGDfloat _sgdClampToUnity ( const SGDfloat x )
{
  if ( x >  SGD_ONE ) return  SGD_ONE ;
  if ( x < -SGD_ONE ) return -SGD_ONE ;
  return x ;
}

int sgdCompare3DSqdDist( const sgdVec3 v1, const sgdVec3 v2, const SGDfloat sqd_dist )
{
  sgdVec3 tmp ;

  sgdSubVec3 ( tmp, v2, v1 ) ;

  SGDfloat sqdist = tmp[0] * tmp[0] + tmp[1] * tmp[1] + tmp[2] * tmp[2] ;

  if ( sqdist > sqd_dist ) return  1 ;
  if ( sqdist < sqd_dist ) return -1 ;
  return 0 ;
}

void sgdMakeRotMat4( sgdMat4 mat, const SGDfloat angle, const sgdVec3 axis )
{
  sgdVec3 ax ;
  sgdNormalizeVec3 ( ax, axis ) ; 

  SGDfloat temp_angle = angle * SGD_DEGREES_TO_RADIANS ;
  SGDfloat s = sin ( temp_angle ) ;
  SGDfloat c = cos ( temp_angle ) ;
  SGDfloat t = SGD_ONE - c ;
   
  mat[0][0] = t * ax[0] * ax[0] + c ;
  mat[0][1] = t * ax[0] * ax[1] + s * ax[2] ;
  mat[0][2] = t * ax[0] * ax[2] - s * ax[1] ;
  mat[0][3] = SGD_ZERO ;

  mat[1][0] = t * ax[1] * ax[0] - s * ax[2] ;
  mat[1][1] = t * ax[1] * ax[1] + c ;
  mat[1][2] = t * ax[1] * ax[2] + s * ax[0] ;
  mat[1][3] = SGD_ZERO ;

  mat[2][0] = t * ax[2] * ax[0] + s * ax[1] ;
  mat[2][1] = t * ax[2] * ax[1] - s * ax[0] ;
  mat[2][2] = t * ax[2] * ax[2] + c ;
  mat[2][3] = SGD_ZERO ;

  mat[3][0] = SGD_ZERO ;
  mat[3][1] = SGD_ZERO ;
  mat[3][2] = SGD_ZERO ;
  mat[3][3] = SGD_ONE ;
}


/*********************\
*    sgdBox routines   *
\*********************/


void sgdBox::extend ( const sgdVec3 v )
{
  if ( isEmpty () )
  {
    sgdCopyVec3 ( min, v ) ;
    sgdCopyVec3 ( max, v ) ;
  }
  else
  {
    if ( v[0] < min[0] ) min[0] = v[0] ;
    if ( v[1] < min[1] ) min[1] = v[1] ;
    if ( v[2] < min[2] ) min[2] = v[2] ;
    if ( v[0] > max[0] ) max[0] = v[0] ;
    if ( v[1] > max[1] ) max[1] = v[1] ;
    if ( v[2] > max[2] ) max[2] = v[2] ;
  }
}


void sgdBox::extend ( const sgdBox *b )
{
  if ( b -> isEmpty () )
    return ;

  if ( isEmpty () )
  {
    sgdCopyVec3 ( min, b->getMin() ) ;
    sgdCopyVec3 ( max, b->getMax() ) ;
  }
  else
  {
    extend ( b->getMin() ) ;
    extend ( b->getMax() ) ;
  }
}


void sgdBox::extend ( const sgdSphere *s )
{
  if ( s -> isEmpty () ) 
    return ;

  /*
    In essence, this extends around a box around the sphere - which
    is still a perfect solution because both boxes are axially aligned.
  */

  sgdVec3 x ;

  sgdSetVec3 ( x, s->getCenter()[0]+s->getRadius(),
                 s->getCenter()[1]+s->getRadius(),
                 s->getCenter()[2]+s->getRadius() ) ;
  extend ( x ) ;

  sgdSetVec3 ( x, s->getCenter()[0]-s->getRadius(),
                 s->getCenter()[1]-s->getRadius(),
                 s->getCenter()[2]-s->getRadius() ) ;
  extend ( x ) ;
}


int sgdBox::intersects ( const sgdVec4 plane ) const
{
  /*
    Save multiplies by not redoing Ax+By+Cz+D for each point.
  */

  SGDfloat Ax_min        = plane[0] * min[0] ;
  SGDfloat By_min        = plane[1] * min[1] ;
  SGDfloat Cz_min_plus_D = plane[2] * min[2] + plane[3] ;

  SGDfloat Ax_max        = plane[0] * max[0] ;
  SGDfloat By_max        = plane[1] * max[1] ;
  SGDfloat Cz_max_plus_D = plane[2] * max[2] + plane[3] ;

  /*
    Count the number of vertices on the positive side of the plane.
  */

  int count = ( Ax_min + By_min + Cz_min_plus_D > SGD_ZERO ) +
              ( Ax_min + By_min + Cz_max_plus_D > SGD_ZERO ) +
              ( Ax_min + By_max + Cz_min_plus_D > SGD_ZERO ) +
              ( Ax_min + By_max + Cz_max_plus_D > SGD_ZERO ) +
              ( Ax_max + By_min + Cz_min_plus_D > SGD_ZERO ) +
              ( Ax_max + By_min + Cz_max_plus_D > SGD_ZERO ) +
              ( Ax_max + By_max + Cz_min_plus_D > SGD_ZERO ) +
              ( Ax_max + By_max + Cz_max_plus_D > SGD_ZERO ) ;

  /*
    The plane intersects the box unless all 8 are positive
    or none of them are positive.
  */
              
  return count != 0 && count != 8 ;
}



/**********************\
*  sgdSphere routines   *
\**********************/

void sgdSphere::extend ( const sgdVec3 v )
{
  if ( isEmpty () )
  {
    sgdCopyVec3 ( center, v ) ;
    radius = 0.0f ;
    return ;
  }

  SGDfloat d = sgdDistanceVec3 ( center, v ) ;

  if ( d <= radius )  /* Point is already inside sphere */
    return ;

  SGDfloat new_radius = (radius + d) / 2.0f ;  /* Grow radius */

  SGDfloat ratio = (new_radius - radius) / d ;

  center[0] += (v[0]-center[0]) * ratio ;    /* Move center */
  center[1] += (v[1]-center[1]) * ratio ;
  center[2] += (v[2]-center[2]) * ratio ;

  radius = new_radius ;
}


void sgdSphere::extend ( const sgdBox *b )
{
  if ( b -> isEmpty () )
    return ;

  if ( isEmpty() )
  {
    sgdAddVec3   ( center, b->getMin(), b->getMax() ) ;
    sgdScaleVec3 ( center, 0.5f ) ;
    radius = sgdDistanceVec3 ( center, b->getMax() ) ;
    return ;
  }

  /*
    I can't think of a faster way to get an
    utterly minimal sphere.

    The tighter algorithm:- enclose each
    of eight vertices of the box in turn - it
    looks like being pretty costly.
    [8 sqrt()'s]

    The looser algorithm:- enclose the box
    with an empty sphere and then do a
    sphere-extend-sphere. This algorithm
    does well for close-to-cube boxes, but
    makes very poor spheres for long, thin
    boxes.
    [2 sqrt()'s]
  */

#ifdef DONT_REALLY_NEED_A_TIGHT_SPHERE_EXTEND_BOX

  /* LOOSER/FASTER sphere-around-sphere-around-box */
  sgdSphere s ;
  s.empty   ()    ;
  s.enclose ( b ) ;  /* Fast because s is empty */
    enclose ( s ) ;

#else

  /* TIGHTER/EXPENSIVE sphere-around-eight-points */
  sgdVec3 x ;
                                                        extend ( b->getMin() ) ;
  sgdSetVec3 ( x, b->getMin()[0],b->getMin()[1],b->getMax()[2] ) ; extend ( x ) ;
  sgdSetVec3 ( x, b->getMin()[0],b->getMax()[1],b->getMin()[2] ) ; extend ( x ) ;
  sgdSetVec3 ( x, b->getMin()[0],b->getMax()[1],b->getMax()[2] ) ; extend ( x ) ;
  sgdSetVec3 ( x, b->getMax()[0],b->getMin()[1],b->getMin()[2] ) ; extend ( x ) ;
  sgdSetVec3 ( x, b->getMax()[0],b->getMin()[1],b->getMax()[2] ) ; extend ( x ) ;
  sgdSetVec3 ( x, b->getMax()[0],b->getMax()[1],b->getMin()[2] ) ; extend ( x ) ;
                                                        extend ( b->getMax() ) ;
#endif
}


void sgdSphere::extend ( const sgdSphere *s )
{
  if ( s->isEmpty () )
    return ;

  if ( isEmpty () )
  {
    sgdCopyVec3 ( center, s->getCenter() ) ;
    radius = s->getRadius() ;
    return ;
  }

  /* 
    d == The distance between the sphere centers
  */

  SGDfloat d = sgdDistanceVec3 ( center, s->getCenter() ) ;

  if ( d + s->getRadius() <= radius )  /* New sphere is already inside this one */
    return ;

  if ( d + radius <= s->getRadius() )  /* New sphere completely contains this one */
  {
    sgdCopyVec3 ( center, s->getCenter() ) ;
    radius = s->getRadius() ;
    return ;
  } 

  /*
    Build a new sphere that completely contains the other two:

    The center point lies halfway along the line between
    the furthest points on the edges of the two spheres.
    Computing those two points is ugly - so we'll use similar
    triangles
  */

  SGDfloat new_radius = (radius + d + s->getRadius() ) / 2.0f ;

  SGDfloat ratio = ( new_radius - radius ) / d ;

  center[0] += ( s->getCenter()[0] - center[0] ) * ratio ;
  center[1] += ( s->getCenter()[1] - center[1] ) * ratio ;
  center[2] += ( s->getCenter()[2] - center[2] ) * ratio ;
  radius = new_radius ;
}


int sgdSphere::intersects ( const sgdBox *b ) const
{
  sgdVec3 closest ;

  if ( b->getMin()[0] > center[0] ) closest[0] = b->getMin()[0] ; else
  if ( b->getMax()[0] < center[0] ) closest[0] = b->getMax()[0] ; else
                                    closest[0] = center[0] ;

  if ( b->getMin()[1] > center[1] ) closest[1] = b->getMin()[1] ; else
  if ( b->getMax()[1] < center[1] ) closest[1] = b->getMax()[1] ; else
                                    closest[1] = center[1] ;

  if ( b->getMin()[2] > center[2] ) closest[2] = b->getMin()[2] ; else
  if ( b->getMax()[2] < center[2] ) closest[2] = b->getMax()[2] ; else
                                    closest[2] = center[2] ;

  return sgdCompare3DSqdDist ( closest, center, sgdSquare ( radius ) ) <= 0 ;
}


/************************\
*   sgdFrustum routines   *
\************************/

void sgdFrustum::update ()
{
  if ( fabs ( ffar - nnear ) < 0.1 )
  {
    fprintf ( stderr, "sgdFrustum: Can't support depth of view <0.1 units.\n");
    return ;
  }

  if ( hfov != SGD_ZERO && vfov != SGD_ZERO )
  {
    if ( fabs ( hfov ) < 0.1 || fabs ( vfov ) < 0.1 )
    {
      fprintf ( stderr, "sgdFrustum: Can't support fields of view narrower than 0.1 degrees.\n");
      return ;
    }

    /* Corners of screen relative to eye... */
  
    right = nnear * tan ( hfov * SGD_DEGREES_TO_RADIANS / SGD_TWO ) ;
    top   = nnear * tan ( vfov * SGD_DEGREES_TO_RADIANS / SGD_TWO ) ;
    left  = -right ;
    bot   = -top   ;
  }

  /*
    Compute plane equations for the four sloping faces of the frustum.

    These are useful for FrustContains(sphere) tests.

    Noting that those planes always go through the origin, their 'D'
    components will always be zero - so the plane equation is really
    just the normal - which is the cross-product of two edges of each face,
    and since we can pick two edges that go through the origin, the
    vectors for the edges are just the normalised corners of the near plane.
  */

  sgdVec3 v1, v2, v3, v4 ;

  sgdSetVec3 ( v1, left , top, -nnear ) ;
  sgdSetVec3 ( v2, right, top, -nnear ) ;
  sgdSetVec3 ( v3, left , bot, -nnear ) ;
  sgdSetVec3 ( v4, right, bot, -nnear ) ;

  sgdNormaliseVec3 ( v1 ) ;
  sgdNormaliseVec3 ( v2 ) ;
  sgdNormaliseVec3 ( v3 ) ;
  sgdNormaliseVec3 ( v4 ) ;

  /*
    Take care of the order of the parameters so that all the planes
    are oriented facing inwards...
  */

  sgdVectorProductVec3 (   top_plane, v1, v2 ) ;
  sgdVectorProductVec3 ( right_plane, v2, v4 ) ;
  sgdVectorProductVec3 (   bot_plane, v4, v3 ) ;
  sgdVectorProductVec3 (  left_plane, v3, v1 ) ;

  /* 
    At this point, you could call

      glMatrixMode ( GL_PROJECTION ) ;
      glLoadIdentity () ;
      glFrustum      ( left, right, bot, top, nnear, ffar ) ;

    Or...

      pfMakePerspFrust ( frust, left, right, bot, top ) ;
      pfFrustNearFar   ( frust, nnear, ffar ) ;

    Or...

      just use the matrix we generate below:
  */

  /* Width, height, depth */

  SGDfloat w = right - left ;
  SGDfloat h = top   - bot  ;
  SGDfloat d = ffar  - nnear ;

  mat[0][0] =  SGD_TWO * nnear / w ;
  mat[0][1] =  SGD_ZERO ;
  mat[0][2] =  SGD_ZERO ;
  mat[0][3] =  SGD_ZERO ;

  mat[1][0] =  SGD_ZERO ;
  mat[1][1] =  SGD_TWO * nnear / h ;
  mat[1][2] =  SGD_ZERO ;
  mat[1][3] =  SGD_ZERO ;

  mat[2][0] =  ( right + left ) / w ;
  mat[2][1] =  ( top   + bot  ) / h ;
  mat[2][2] = -( ffar  + nnear ) / d ;
  mat[2][3] = -SGD_ONE ;

  mat[3][0] =  SGD_ZERO ;
  mat[3][1] =  SGD_ZERO ;
  mat[3][2] = -SGD_TWO * nnear * ffar/ d ;
  mat[3][3] =  SGD_ZERO ;
}


#define OC_LEFT_SHIFT   0
#define OC_RIGHT_SHIFT  1
#define OC_TOP_SHIFT    2
#define OC_BOT_SHIFT    3
#define OC_NEAR_SHIFT   4
#define OC_FAR_SHIFT    5

#define OC_ALL_ON_SCREEN 0x3F
#define OC_OFF_TRF      ((1<<OC_TOP_SHIFT)|(1<<OC_RIGHT_SHIFT)|(1<<OC_FAR_SHIFT))
#define OC_OFF_BLN      ((1<<OC_BOT_SHIFT)|(1<<OC_LEFT_SHIFT)|(1<<OC_NEAR_SHIFT))

int sgdFrustum::getOutcode ( const sgdVec3 pt ) const
{
  /* Transform the point by the Frustum's transform. */

  sgdVec4 tmp ;

  tmp [ 0 ] = pt [ 0 ] ;
  tmp [ 1 ] = pt [ 1 ] ;
  tmp [ 2 ] = pt [ 2 ] ;
  tmp [ 3 ] =  SGD_ONE  ;

  sgdXformPnt4 ( tmp, tmp, mat ) ;

  /*
    No need to divide by the 'w' component since we are only checking for
    results in the range 0..1
  */

  return (( tmp[0] <=  tmp[3] ) << OC_RIGHT_SHIFT ) |
         (( tmp[0] >= -tmp[3] ) << OC_LEFT_SHIFT  ) |
         (( tmp[1] <=  tmp[3] ) << OC_TOP_SHIFT   ) |
         (( tmp[1] >= -tmp[3] ) << OC_BOT_SHIFT   ) |
         (( tmp[2] <=  tmp[3] ) << OC_FAR_SHIFT   ) |
         (( tmp[2] >= -tmp[3] ) << OC_NEAR_SHIFT  ) ;
}

int sgdFrustum::contains ( const sgdVec3 pt ) const
{
  return getOutcode ( pt ) == OC_ALL_ON_SCREEN ;
}


int sgdFrustum::contains ( const sgdSphere *s ) const
{
  /*
    Lop off half the database (roughly) with a quick near-plane test - and
    lop off a lot more with a quick far-plane test
  */

  if ( -s->getCenter() [ 2 ] + s->getRadius() < nnear ||
       -s->getCenter() [ 2 ] - s->getRadius() > ffar )
    return SGD_OUTSIDE ;

  /*
    OK, so the sphere lies between near and far.

    Measure the distance of the center point from the four sides of the frustum,
    if it's outside by more than the radius then it's history.

    It's tempting to do a quick test to see if the center point is
    onscreen using sgdFrustumContainsPt - but that takes a matrix transform
    which is 16 multiplies and 12 adds - versus this test which does the
    whole task using only 12 multiplies and 8 adds.
  */

  SGDfloat sp1 = sgdScalarProductVec3 (  left_plane, s->getCenter() ) ;
  SGDfloat sp2 = sgdScalarProductVec3 ( right_plane, s->getCenter() ) ;
  SGDfloat sp3 = sgdScalarProductVec3 (   bot_plane, s->getCenter() ) ;
  SGDfloat sp4 = sgdScalarProductVec3 (   top_plane, s->getCenter() ) ;

  if ( -sp1 >= s->getRadius() || -sp2 >= s->getRadius() ||
       -sp3 >= s->getRadius() || -sp4 >= s->getRadius() )
    return SGD_OUTSIDE ;
  
  /*
    If it's inside by more than the radius then it's *completely* inside
    and we can save time elsewhere if we know that for sure.
  */

  if ( -s->getCenter() [ 2 ] - s->getRadius() > nnear &&
       -s->getCenter() [ 2 ] + s->getRadius() < ffar &&
       sp1 >= s->getRadius() && sp2 >= s->getRadius() &&
       sp3 >= s->getRadius() && sp4 >= s->getRadius() )
    return SGD_INSIDE ;

  return SGD_STRADDLE ;
}


void sgdMakeCoordMat4 ( sgdMat4 m, const SGDfloat x, const SGDfloat y, const SGDfloat z, const SGDfloat h, const SGDfloat p, const SGDfloat r )
{
  double ch, sh, cp, sp, cr, sr, srsp, crsp, srcp ;

  if ( h == SGD_ZERO )
  {
    ch = SGD_ONE ;
    sh = SGD_ZERO ;
  }
  else
  {
    sh = sin( h * SGD_DEGREES_TO_RADIANS) ;
    ch = cos( h * SGD_DEGREES_TO_RADIANS) ;
  }

  if ( p == SGD_ZERO )
  {
    cp = SGD_ONE ;
    sp = SGD_ZERO ;
  }
  else
  {
    sp = sin( p * SGD_DEGREES_TO_RADIANS) ;
    cp = cos( p * SGD_DEGREES_TO_RADIANS) ;
  }

  if ( r == SGD_ZERO )
  {
    cr   = SGD_ONE ;
    sr   = SGD_ZERO ;
    srsp = SGD_ZERO ;
    srcp = SGD_ZERO ;
    crsp = sp ;
  }
  else
  {
    sr   = sin( r * SGD_DEGREES_TO_RADIANS) ;
    cr   = cos( r * SGD_DEGREES_TO_RADIANS) ;
    srsp = sr * sp ;
    crsp = cr * sp ;
    srcp = sr * cp ;
  }

  m[0][0] =  ch * cr - sh * srsp ;
  m[1][0] = -sh * cp ;
  m[2][0] =  sr * ch + sh * crsp ;
  m[3][0] =  x ;

  m[0][1] =  cr * sh + srsp * ch ;
  m[1][1] =  ch * cp ;
  m[2][1] =  sr * sh - crsp * ch ;
  m[3][1] =  y ;

  m[0][2] = -srcp ;
  m[1][2] =  sp ;
  m[2][2] =  cr * cp ;
  m[3][2] =  z ;

  m[0][3] =  SGD_ZERO ;
  m[1][3] =  SGD_ZERO ;
  m[2][3] =  SGD_ZERO ;
  m[3][3] =  SGD_ONE ;
}


void sgdMakeTransMat4 ( sgdMat4 m, const sgdVec3 xyz )
{
  m[0][1] = m[0][2] = m[0][3] =
  m[1][0] = m[1][2] = m[1][3] =
  m[2][0] = m[2][1] = m[2][3] = SGD_ZERO ;
  m[0][0] = m[1][1] = m[2][2] = m[3][3] = SGD_ONE ;
  sgdCopyVec3 ( m[3], xyz ) ;
}


void sgdMakeTransMat4 ( sgdMat4 m, const SGDfloat x, const SGDfloat y, const SGDfloat z )
{
  m[0][1] = m[0][2] = m[0][3] =
  m[1][0] = m[1][2] = m[1][3] =
  m[2][0] = m[2][1] = m[2][3] = SGD_ZERO ;
  m[0][0] = m[1][1] = m[2][2] = m[3][3] = SGD_ONE ;
  sgdSetVec3 ( m[3], x, y, z ) ;
}


void sgdSetCoord ( sgdCoord *dst, const sgdMat4 src )
{
  sgdCopyVec3 ( dst->xyz, src[3] ) ;
    
  sgdMat4 mat ;

  SGDfloat s = sgdLengthVec3 ( src[0] ) ;

  if ( s <= 0.00001 )
  {
    fprintf ( stderr, "sgdMat4ToCoord: ERROR - Bad Matrix.\n" ) ;
    sgdSetVec3 ( dst -> hpr, SGD_ZERO, SGD_ZERO, SGD_ZERO ) ;
    return ;
  }

  sgdScaleMat4 ( mat, src, SGD_ONE / s ) ;
    
  dst->hpr[1] = asin ( _sgdClampToUnity ( mat[1][2] ) ) ;

  SGDfloat cp = cos ( dst->hpr[1] ) ;
    
  /* If pointing nearly vertically up - then heading is ill-defined */

  if ( cp > -0.00001 && cp < 0.00001 )
  {
    SGDfloat cr = _sgdClampToUnity ( mat[0][1] ) ; 
    SGDfloat sr = _sgdClampToUnity (-mat[2][1] ) ;

    dst->hpr[0] = SGD_ZERO ;
    dst->hpr[2] = atan2 ( sr, cr ) ;
  }
  else
  {
    SGDfloat sr = _sgdClampToUnity ( -mat[0][2] / cp ) ;
    SGDfloat cr = _sgdClampToUnity (  mat[2][2] / cp ) ;
    SGDfloat sh = _sgdClampToUnity ( -mat[1][0] / cp ) ;
    SGDfloat ch = _sgdClampToUnity (  mat[1][1] / cp ) ;
  
    if ( (sh == SGD_ZERO && ch == SGD_ZERO) || (sr == SGD_ZERO && cr == SGD_ZERO) )
    {
      cr = _sgdClampToUnity ( mat[0][1] ) ;
      sr = _sgdClampToUnity (-mat[2][1] ) ;

      dst->hpr[0] = SGD_ZERO ;
    }
    else
      dst->hpr[0] = atan2 ( sh, ch ) ;

    dst->hpr[2] = atan2 ( sr, cr ) ;
  }

  sgdScaleVec3 ( dst->hpr, SGD_RADIANS_TO_DEGREES ) ;
}


void sgdMakeNormal(sgdVec3 dst, const sgdVec3 a, const sgdVec3 b, const sgdVec3 c )
{
  sgdVec3 ab ; sgdSubVec3 ( ab, b, a ) ; sgdNormaliseVec3 ( ab ) ;
  sgdVec3 ac ; sgdSubVec3 ( ac, c, a ) ; sgdNormaliseVec3 ( ac ) ;
  sgdVectorProductVec3 ( dst, ab,ac ) ; sgdNormaliseVec3 ( dst ) ; /* XXX DO WE REALLY NEED THIS? */
}


void sgdPreMultMat4( sgdMat4 dst, const sgdMat4 src )
{
  sgdMat4 mat ;
  sgdMultMat4 ( mat, src, dst ) ;
  sgdCopyMat4 ( dst, mat ) ;
}

void sgdPostMultMat4( sgdMat4 dst, const sgdMat4 src )
{
  sgdMat4 mat ;
  sgdMultMat4 ( mat, dst, src ) ;
  sgdCopyMat4 ( dst, mat ) ;
}

void sgdMultMat4( sgdMat4 dst, const sgdMat4 m1, const sgdMat4 m2 )
{
  if (((m1[0][3] != SGD_ZERO) |
       (m1[1][3] != SGD_ZERO) |
       (m1[2][3] != SGD_ZERO) |
       (m2[0][3] != SGD_ZERO) |
       (m2[1][3] != SGD_ZERO) |
       (m2[2][3] != SGD_ZERO) ) == 0x0 &&
        m1[3][3] == SGD_ONE &&
        m2[3][3] == SGD_ONE)
  {
    for ( int j = 0 ; j < 3 ; j++ )
    {
      dst[0][j] = (m1[0][0] * m2[0][j] +
                   m1[0][1] * m2[1][j] +
                   m1[0][2] * m2[2][j]);

      dst[1][j] = (m1[1][0] * m2[0][j] +
                   m1[1][1] * m2[1][j] +
                   m1[1][2] * m2[2][j]);

      dst[2][j] = (m1[2][0] * m2[0][j] +
                   m1[2][1] * m2[1][j] +
                   m1[2][2] * m2[2][j]);

      dst[3][j] = (m1[3][0] * m2[0][j] +
                   m1[3][1] * m2[1][j] +
                   m1[3][2] * m2[2][j] +
                   m2[3][j]);
    }

    dst[0][3] = SGD_ZERO ;
    dst[1][3] = SGD_ZERO ;
    dst[2][3] = SGD_ZERO ;
    dst[3][3] = SGD_ONE ;
  }
  else
  {
    for ( int j = 0 ; j < 4 ; j++ )
    {
      dst[0][j] = (m1[0][0] * m2[0][j] +
                   m1[0][1] * m2[1][j] +
                   m1[0][2] * m2[2][j] +
                   m1[0][3] * m2[3][j]);

      dst[1][j] = (m1[1][0] * m2[0][j] +
                   m1[1][1] * m2[1][j] +
                   m1[1][2] * m2[2][j] +
                   m1[1][3] * m2[3][j]);

      dst[2][j] = (m1[2][0] * m2[0][j] +
                   m1[2][1] * m2[1][j] +
                   m1[2][2] * m2[2][j] +
                   m1[2][3] * m2[3][j]);

      dst[3][j] = (m1[3][0] * m2[0][j] +
                   m1[3][1] * m2[1][j] +
                   m1[3][2] * m2[2][j] +
                   m1[3][3] * m2[3][j]);
    }
  }
}



void sgdTransposeNegateMat4 ( sgdMat4 dst, const sgdMat4 src )
{
  /* Poor man's invert - can be used when matrix is a simple rotate-translate */

  dst[0][0] = src[0][0] ;
  dst[1][0] = src[0][1] ;
  dst[2][0] = src[0][2] ;
  dst[3][0] = - sgdScalarProductVec3 ( src[3], src[0] ) ;

  dst[0][1] = src[1][0] ;
  dst[1][1] = src[1][1] ;
  dst[2][1] = src[1][2] ;
  dst[3][1] = - sgdScalarProductVec3 ( src[3], src[1] ) ;
                                                                               
  dst[0][2] = src[2][0] ;                                                      
  dst[1][2] = src[2][1] ;                                                      
  dst[2][2] = src[2][2] ;                                                      
  dst[3][2] = - sgdScalarProductVec3 ( src[3], src[2] ) ;
                                                                               
  dst[0][3] = SGD_ZERO ;
  dst[1][3] = SGD_ZERO ;                                                        
  dst[2][3] = SGD_ZERO ;                                                        
  dst[3][3] = SGD_ONE  ;
}

void sgdTransposeNegateMat4 ( sgdMat4 dst )
{
  sgdMat4 src ;
  sgdCopyMat4 ( src, dst ) ;
  sgdTransposeNegateMat4 ( dst, src ) ;
}


void sgdInvertMat4 ( sgdMat4 dst, const sgdMat4 src )
{
  sgdMat4 tmp ;

  sgdCopyMat4 ( tmp, src ) ;
  sgdMakeIdentMat4 ( dst ) ;

  for ( int i = 0 ; i != 4 ; i++ )
  {
    SGDfloat val = tmp[i][i] ;
    int ind = i ;
    int j ;

    for ( j = i + 1 ; j != 4 ; j++ )
    {
      if ( fabs ( tmp[i][j] ) > fabs(val) )
      {
        ind = j;
        val = tmp[i][j] ;
      }
    }

    if ( ind != i )
    {                   /* swap columns */
      for ( j = 0 ; j != 4 ; j++ )
      {
        SGDfloat t ;
        t = dst[j][i]; dst[j][i] = dst[j][ind]; dst[j][ind] = t ;
        t = tmp[j][i]; tmp[j][i] = tmp[j][ind]; tmp[j][ind] = t ;
      }
    }

#ifndef DBL_EPSILON
#define DBL_EPSILON 1.19209290e-07f
#endif
    // if ( val == SG_ZERO)
    if ( fabs(val) <= DBL_EPSILON )
    {
      fprintf ( stderr, "sg: ERROR - Singular matrix, no inverse!\n" ) ;
      sgdMakeIdentMat4 ( dst ) ;  /* Do *something* */
      return;
    }

    SGDfloat ival = SGD_ONE / val ;

    for ( j = 0 ; j != 4 ; j++ )
    {
      tmp[j][i] *= ival ;
      dst[j][i] *= ival ;
    }

    for (j = 0; j != 4; j++)
    {
      if ( j == i )
        continue ;

      val = tmp[i][j] ;

      for ( int k = 0 ; k != 4 ; k++ )
      {
        tmp[k][j] -= tmp[k][i] * val ;
        dst[k][j] -= dst[k][i] * val ;
      }
    }
  }
}



void sgdXformVec3 ( sgdVec3 dst, const sgdVec3 src, const sgdMat4 mat )
{
  SGDfloat t0 = src[ 0 ] ;
  SGDfloat t1 = src[ 1 ] ;
  SGDfloat t2 = src[ 2 ] ;

  dst[0] = ( t0 * mat[ 0 ][ 0 ] +
             t1 * mat[ 1 ][ 0 ] +
             t2 * mat[ 2 ][ 0 ] ) ;

  dst[1] = ( t0 * mat[ 0 ][ 1 ] +
             t1 * mat[ 1 ][ 1 ] +
             t2 * mat[ 2 ][ 1 ] ) ;

  dst[2] = ( t0 * mat[ 0 ][ 2 ] +
             t1 * mat[ 1 ][ 2 ] +
             t2 * mat[ 2 ][ 2 ] ) ;
}


void sgdXformPnt3 ( sgdVec3 dst, const sgdVec3 src, const sgdMat4 mat )
{
  SGDfloat t0 = src[ 0 ] ;
  SGDfloat t1 = src[ 1 ] ;
  SGDfloat t2 = src[ 2 ] ;

  dst[0] = ( t0 * mat[ 0 ][ 0 ] +
             t1 * mat[ 1 ][ 0 ] +
             t2 * mat[ 2 ][ 0 ] +
                  mat[ 3 ][ 0 ] ) ;

  dst[1] = ( t0 * mat[ 0 ][ 1 ] +
             t1 * mat[ 1 ][ 1 ] +
             t2 * mat[ 2 ][ 1 ] +
                  mat[ 3 ][ 1 ] ) ;

  dst[2] = ( t0 * mat[ 0 ][ 2 ] +
             t1 * mat[ 1 ][ 2 ] +
             t2 * mat[ 2 ][ 2 ] +
                  mat[ 3 ][ 2 ] ) ;
}


void sgdXformPnt4 ( sgdVec4 dst, const sgdVec4 src, const sgdMat4 mat )
{
  SGDfloat t0 = src[ 0 ] ;
  SGDfloat t1 = src[ 1 ] ;
  SGDfloat t2 = src[ 2 ] ;
  SGDfloat t3 = src[ 3 ] ;

  dst[0] = ( t0 * mat[ 0 ][ 0 ] +
             t1 * mat[ 1 ][ 0 ] +
             t2 * mat[ 2 ][ 0 ] +
             t3 * mat[ 3 ][ 0 ] ) ;

  dst[1] = ( t0 * mat[ 0 ][ 1 ] +
             t1 * mat[ 1 ][ 1 ] +
             t2 * mat[ 2 ][ 1 ] +
             t3 * mat[ 3 ][ 1 ] ) ;

  dst[2] = ( t0 * mat[ 0 ][ 2 ] +
             t1 * mat[ 1 ][ 2 ] +
             t2 * mat[ 2 ][ 2 ] +
             t3 * mat[ 3 ][ 2 ] ) ;

  dst[3] = ( t0 * mat[ 0 ][ 3 ] +
             t1 * mat[ 1 ][ 3 ] +
             t2 * mat[ 2 ][ 3 ] +
             t3 * mat[ 3 ][ 3 ] ) ;
}


void sgdFullXformPnt3 ( sgdVec3 dst, const sgdVec3 src, const sgdMat4 mat )
{
  sgdVec4 tmp ;

  tmp [ 0 ] = src [ 0 ] ;
  tmp [ 1 ] = src [ 1 ] ;
  tmp [ 2 ] = src [ 2 ] ;
  tmp [ 3 ] =   SGD_ONE  ;

  sgdXformPnt4 ( tmp, tmp, mat ) ;
  sgdScaleVec3 ( dst, tmp, SGD_ONE / tmp [ 3 ] ) ;
}

void sgdHPRfromVec3 ( sgdVec3 hpr, sgdVec3 src )
{
  sgdVec3 tmp ;
  sgdCopyVec3 ( tmp, src ) ;
  sgdNormaliseVec3 ( tmp ) ;
  hpr[0] = -atan2 ( tmp [ 0 ], tmp [ 1 ] ) * SGD_RADIANS_TO_DEGREES ;
  hpr[1] = -atan2 ( tmp [ 2 ], sqrt ( sgdSquare ( tmp [ 0 ] ) +
                                      sgdSquare ( tmp [ 1 ] ) ) ) *
                                             SGD_RADIANS_TO_DEGREES ;
  hpr[2] = 0.0f ;
}



/*
  Quaternion routines are Copyright (C) 1999
  Kevin B. Thompson <kevinbthompson@yahoo.com>
  Modified by Sylvan W. Clebsch <sylvan@stanford.edu>
*/


void sgdQuatToAngleAxis ( SGDfloat *angle,
                          SGDfloat *x, SGDfloat *y, SGDfloat *z,
                          const sgdQuat *src )
{
  sgdVec3 axis ;

  sgdQuatToAngleAxis ( angle, axis, src ) ;

  *x = axis [ 0 ] ;
  *y = axis [ 1 ] ;
  *z = axis [ 2 ] ;
}


void sgdQuatToAngleAxis ( SGDfloat *angle, sgdVec3 axis, const sgdQuat *src )
{
  SGDfloat a = acos ( src->w ) ;
  SGDfloat s = sin  ( a ) ;

  *angle = a * SGD_RADIANS_TO_DEGREES * SGD_TWO ;

  if ( s == SGD_ZERO )
    sgdSetVec3 ( axis, SGD_ZERO, SGD_ZERO, SGD_ONE );
  else
  {
    sgdSetVec3   ( axis, src->x, src->y, src->z ) ;
    sgdScaleVec3 ( axis, SGD_ONE / s ) ;
  }
}


void sgdMakeQuat ( sgdQuat *dst, const sgdVec3 hpr )
{
  /* SWC - added double version */
  sgdVec3 temp_hpr;

  temp_hpr[0] = hpr[0] * SG_DEGREES_TO_RADIANS / SG_TWO ;
  temp_hpr[1] = hpr[1] * SG_DEGREES_TO_RADIANS / SG_TWO ;
  temp_hpr[2] = hpr[2] * SG_DEGREES_TO_RADIANS / SG_TWO ;

  SGDfloat sh = -(SGDfloat) sin ( temp_hpr[0] ) ; SGDfloat ch = (SGDfloat) cos ( temp_hpr[0] ) ;
  SGDfloat sp = -(SGDfloat) sin ( temp_hpr[1] ) ; SGDfloat cp = (SGDfloat) cos ( temp_hpr[1] ) ;
  SGDfloat sr = -(SGDfloat) sin ( temp_hpr[2] ) ; SGDfloat cr = (SGDfloat) cos ( temp_hpr[2] ) ;

  SGDfloat cpch = cp * ch;
  SGDfloat spsh = sp * sh;

  dst->w = cr * cpch + sr * spsh;
  dst->x = sr * cpch - cr * spsh;
  dst->y = cr * sp * ch + sr * cp * sh;
  dst->z = cr * cp * sh - sr * sp * ch;
}


void sgdMakeQuat ( sgdQuat *dst, const SGDfloat angle, const SGDfloat x, const SGDfloat y, const SGDfloat z )
{
  SGDfloat temp_angle = angle * SGD_DEGREES_TO_RADIANS / SGD_TWO ;

  SGDfloat s = sin ( temp_angle ) ;

  dst->w = cos ( temp_angle ) ;
  dst->x = s * x ;
  dst->y = s * y ;
  dst->z = s * z ;
}


void sgdMakeQuat ( sgdQuat *dst, const SGDfloat angle, const sgdVec3 axis )
{
  sgdMakeQuat ( dst, angle, axis[0], axis[1], axis[2] ) ;
}


void sgdMultQuat ( sgdQuat *dst, const sgdQuat *a, const sgdQuat *b )
{
  /* [ ww' - v.v', vxv' + wv' + v'w ] */

#if 0
  dst->w = a->w * b->w - (a->x * b->x + a->y * b->y + a->z * b->z) ;
  dst->x = a->y * b->z -  a->z * b->y + a->w * b->x + b->w * a->x ;
  dst->y = a->z * b->x -  a->x * b->z + a->w * b->y + b->w * a->y ;
  dst->z = a->x * b->y -  a->y * b->x + a->w * b->z + b->w * a->z ;
#else
  /* SWC - Reduce from 16 to 12 muls */
  SGDfloat t[8];

  t[0] = (a->w + a->x) * (b->w + b->x);
  t[1] = (a->z - a->y) * (b->y - b->z);
  t[2] = (a->x - a->w) * (b->y - b->z);
  t[3] = (a->y + a->z) * (b->x - b->w);
  t[4] = (a->x + a->z) * (b->x + b->y);
  t[5] = (a->x - a->z) * (b->x - b->y);
  t[6] = (a->w + a->y) * (b->w - b->z);
  t[7] = (a->w - a->y) * (b->w + b->z);

  dst->w =  t[1] + ((-t[4] - t[5] + t[6] + t[7]) * 0.5f);
  dst->x =  t[0] - (( t[4] + t[5] + t[6] + t[7]) * 0.5f);
  dst->y = -t[2] + (( t[4] - t[5] + t[6] - t[7]) * 0.5f);
  dst->z = -t[3] + (( t[4] - t[5] - t[6] + t[7]) * 0.5f);
#endif
}


void sgdMakeRotMat4 ( sgdMat4 dst, const sgdQuat *q )
{
#if 0
  SGDfloat two_xx = SGD_TWO * q->x * q->x ;
  SGDfloat two_xy = SGD_TWO * q->x * q->y ;
  SGDfloat two_xz = SGD_TWO * q->x * q->z ;

  SGDfloat two_wx = SGD_TWO * q->w * q->x ;
  SGDfloat two_wy = SGD_TWO * q->w * q->y ;
  SGDfloat two_wz = SGD_TWO * q->w * q->z ;

  SGDfloat two_yy = SGD_TWO * q->y * q->y ;
  SGDfloat two_yz = SGD_TWO * q->y * q->z ;

  SGDfloat two_zz = SGD_TWO * q->z * q->z ;
#else
  /* SWC - Reduce from 18 to 9 muls */
  SGDfloat two_xx = q->x * (q->x + q->x) ;
  SGDfloat two_xy = q->x * (q->y + q->y) ;
  SGDfloat two_xz = q->x * (q->z + q->z) ;

  SGDfloat two_wx = q->w * (q->x + q->x) ;
  SGDfloat two_wy = q->w * (q->y + q->y) ;
  SGDfloat two_wz = q->w * (q->z + q->z) ;

  SGDfloat two_yy = q->y * (q->y + q->y) ;
  SGDfloat two_yz = q->y * (q->z + q->z) ;

  SGDfloat two_zz = q->z * (q->z + q->z) ;
#endif

  sgdSetVec4 ( dst[0], SGD_ONE-(two_yy+two_zz), two_xy-two_wz, two_xz+two_wy, SGD_ZERO ) ;
  sgdSetVec4 ( dst[1], two_xy+two_wz, SGD_ONE-(two_xx+two_zz), two_yz-two_wx, SGD_ZERO ) ;
  sgdSetVec4 ( dst[2], two_xz-two_wy, two_yz+two_wx, SGD_ONE-(two_xx+two_yy), SGD_ZERO ) ;
  sgdSetVec4 ( dst[3], SGD_ZERO, SGD_ZERO, SGD_ZERO, SGD_ONE ) ;
}


void sgdSlerpQuat( sgdQuat *dst, const sgdQuat *from, const sgdQuat *to, const SGDfloat t )
{
  SGDfloat    sign, co, scale0, scale1;

  /* SWC - Interpolate between two quaternions */

  co = from->x * to->x + from->y * to->y + from->z * to->z + from->w * to->w;

  if( co < SGD_ZERO )
  {
    co = -co;
    sign = -SGD_ONE;
  }
  else
    sign = SGD_ONE;

  if( co < SGD_ONE )
  {
    SGDfloat o = (SGDfloat)acos( co );
    SGDfloat so = (SGDfloat)sin( o );

    scale0 = (SGDfloat)sin( (SGD_ONE - t) * o ) / so;
    scale1 = (SGDfloat)sin( t * o ) / so;
  }
  else
  {
    scale0 = SGD_ONE - t;
    scale1 = t;
  }

  dst->x = scale0 * from->x + scale1 * ((sign > SGD_ZERO) ? to->w : -to->w);
  dst->y = scale0 * from->y + scale1 * ((sign > SGD_ZERO) ? to->w : -to->x);
  dst->z = scale0 * from->z + scale1 * ((sign > SGD_ZERO) ? to->w : -to->y);
  dst->w = scale0 * from->w + scale1 * ((sign > SGD_ZERO) ? to->w : -to->z);
}

