
#include <stdio.h>
#include "sg.h"

#ifndef TRUE
#define TRUE  1
#define FALSE 0
#endif

void sgVectorProductVec3 ( sgVec3 dst, const sgVec3 a, const sgVec3 b )
{
  dst[0] = a[1] * b[2] - a[2] * b[1] ;
  dst[1] = a[2] * b[0] - a[0] * b[2] ;
  dst[2] = a[0] * b[1] - a[1] * b[0] ;
}

inline SGfloat _sgClampToUnity ( const SGfloat x )
{
  if ( x >  SG_ONE ) return  SG_ONE ;
  if ( x < -SG_ONE ) return -SG_ONE ;
  return x ;
}

int sgCompare3DSqdDist( const sgVec3 v1, const sgVec3 v2, const SGfloat sqd_dist )
{
  sgVec3 tmp ;

  sgSubVec3 ( tmp, v2, v1 ) ;

  SGfloat sqdist = tmp[0] * tmp[0] + tmp[1] * tmp[1] + tmp[2] * tmp[2] ;

  if ( sqdist > sqd_dist ) return  1 ;
  if ( sqdist < sqd_dist ) return -1 ;
  return 0 ;
}

void sgMakeRotMat4( sgMat4 mat, const SGfloat angle, const sgVec3 axis )
{
  sgVec3 ax ;
  sgNormalizeVec3 ( ax, axis ) ; 

  SGfloat temp_angle = angle * SG_DEGREES_TO_RADIANS ;
  SGfloat s = (SGfloat) sin ( temp_angle ) ;
  SGfloat c = (SGfloat) cos ( temp_angle ) ;
  SGfloat t = SG_ONE - c ;
   
  mat[0][0] = t * ax[0] * ax[0] + c ;
  mat[0][1] = t * ax[0] * ax[1] + s * ax[2] ;
  mat[0][2] = t * ax[0] * ax[2] - s * ax[1] ;
  mat[0][3] = SG_ZERO ;

  mat[1][0] = t * ax[1] * ax[0] - s * ax[2] ;
  mat[1][1] = t * ax[1] * ax[1] + c ;
  mat[1][2] = t * ax[1] * ax[2] + s * ax[0] ;
  mat[1][3] = SG_ZERO ;

  mat[2][0] = t * ax[2] * ax[0] + s * ax[1] ;
  mat[2][1] = t * ax[2] * ax[1] - s * ax[0] ;
  mat[2][2] = t * ax[2] * ax[2] + c ;
  mat[2][3] = SG_ZERO ;

  mat[3][0] = SG_ZERO ;
  mat[3][1] = SG_ZERO ;
  mat[3][2] = SG_ZERO ;
  mat[3][3] = SG_ONE ;
}


/*********************\
*    sgBox routines   *
\*********************/


void sgBox::extend ( const sgVec3 v )
{
  if ( isEmpty () )
  {
    sgCopyVec3 ( min, v ) ;
    sgCopyVec3 ( max, v ) ;
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


void sgBox::extend ( const sgBox *b )
{
  if ( b -> isEmpty () )
    return ;

  if ( isEmpty () )
  {
    sgCopyVec3 ( min, b->getMin() ) ;
    sgCopyVec3 ( max, b->getMax() ) ;
  }
  else
  {
    extend ( b->getMin() ) ;
    extend ( b->getMax() ) ;
  }
}


void sgBox::extend ( const sgSphere *s )
{
  if ( s -> isEmpty () ) 
    return ;

  /*
    In essence, this extends around a box around the sphere - which
    is still a perfect solution because both boxes are axially aligned.
  */

  sgVec3 x ;

  sgSetVec3 ( x, s->getCenter()[0]+s->getRadius(),
                 s->getCenter()[1]+s->getRadius(),
                 s->getCenter()[2]+s->getRadius() ) ;
  extend ( x ) ;

  sgSetVec3 ( x, s->getCenter()[0]-s->getRadius(),
                 s->getCenter()[1]-s->getRadius(),
                 s->getCenter()[2]-s->getRadius() ) ;
  extend ( x ) ;
}


int sgBox::intersects ( const sgVec4 plane ) const 
{
  /*
    Save multiplies by not redoing Ax+By+Cz+D for each point.
  */

  SGfloat Ax_min        = plane[0] * min[0] ;
  SGfloat By_min        = plane[1] * min[1] ;
  SGfloat Cz_min_plus_D = plane[2] * min[2] + plane[3] ;

  SGfloat Ax_max        = plane[0] * max[0] ;
  SGfloat By_max        = plane[1] * max[1] ;
  SGfloat Cz_max_plus_D = plane[2] * max[2] + plane[3] ;

  /*
    Count the number of vertices on the positive side of the plane.
  */

  int count = ( Ax_min + By_min + Cz_min_plus_D > SG_ZERO ) +
              ( Ax_min + By_min + Cz_max_plus_D > SG_ZERO ) +
              ( Ax_min + By_max + Cz_min_plus_D > SG_ZERO ) +
              ( Ax_min + By_max + Cz_max_plus_D > SG_ZERO ) +
              ( Ax_max + By_min + Cz_min_plus_D > SG_ZERO ) +
              ( Ax_max + By_min + Cz_max_plus_D > SG_ZERO ) +
              ( Ax_max + By_max + Cz_min_plus_D > SG_ZERO ) +
              ( Ax_max + By_max + Cz_max_plus_D > SG_ZERO ) ;

  /*
    The plane intersects the box unless all 8 are positive
    or none of them are positive.
  */
              
  return count != 0 && count != 8 ;
}



/**********************\
*  sgSphere routines   *
\**********************/

void sgSphere::extend ( const sgVec3 v )
{
  if ( isEmpty () )
  {
    sgCopyVec3 ( center, v ) ;
    radius = 0.0f ;
    return ;
  }

  SGfloat d = sgDistanceVec3 ( center, v ) ;

  if ( d <= radius )  /* Point is already inside sphere */
    return ;

  SGfloat new_radius = (radius + d) / 2.0f ;  /* Grow radius */

  SGfloat ratio = (new_radius - radius) / d ;

  center[0] += (v[0]-center[0]) * ratio ;    /* Move center */
  center[1] += (v[1]-center[1]) * ratio ;
  center[2] += (v[2]-center[2]) * ratio ;

  radius = new_radius ;
}


void sgSphere::extend ( const sgBox *b )
{
  if ( b -> isEmpty () )
    return ;

  if ( isEmpty() )
  {
    sgAddVec3   ( center, b->getMin(), b->getMax() ) ;
    sgScaleVec3 ( center, 0.5f ) ;
    radius = sgDistanceVec3 ( center, b->getMax() ) ;
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
  sgSphere s ;
  s.empty   ()    ;
  s.enclose ( b ) ;  /* Fast because s is empty */
    enclose ( s ) ;

#else

  /* TIGHTER/EXPENSIVE sphere-around-eight-points */
  sgVec3 x ;
                                                        extend ( b->getMin() ) ;
  sgSetVec3 ( x, b->getMin()[0],b->getMin()[1],b->getMax()[2] ) ; extend ( x ) ;
  sgSetVec3 ( x, b->getMin()[0],b->getMax()[1],b->getMin()[2] ) ; extend ( x ) ;
  sgSetVec3 ( x, b->getMin()[0],b->getMax()[1],b->getMax()[2] ) ; extend ( x ) ;
  sgSetVec3 ( x, b->getMax()[0],b->getMin()[1],b->getMin()[2] ) ; extend ( x ) ;
  sgSetVec3 ( x, b->getMax()[0],b->getMin()[1],b->getMax()[2] ) ; extend ( x ) ;
  sgSetVec3 ( x, b->getMax()[0],b->getMax()[1],b->getMin()[2] ) ; extend ( x ) ;
                                                        extend ( b->getMax() ) ;
#endif
}


void sgSphere::extend ( const sgSphere *s )
{
  if ( s->isEmpty () )
    return ;

  if ( isEmpty () )
  {
    sgCopyVec3 ( center, s->getCenter() ) ;
    radius = s->getRadius() ;
    return ;
  }

  /* 
    d == The distance between the sphere centers
  */

  SGfloat d = sgDistanceVec3 ( center, s->getCenter() ) ;

  if ( d + s->getRadius() <= radius )  /* New sphere is already inside this one */
    return ;

  if ( d + radius <= s->getRadius() )  /* New sphere completely contains this one */
  {
    sgCopyVec3 ( center, s->getCenter() ) ;
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

  SGfloat new_radius = (radius + d + s->getRadius() ) / 2.0f ;

  SGfloat ratio = ( new_radius - radius ) / d ;

  center[0] += ( s->getCenter()[0] - center[0] ) * ratio ;
  center[1] += ( s->getCenter()[1] - center[1] ) * ratio ;
  center[2] += ( s->getCenter()[2] - center[2] ) * ratio ;
  radius = new_radius ;
}


int sgSphere::intersects ( const sgBox *b ) const 
{
  sgVec3 closest ;

  if ( b->getMin()[0] > center[0] ) closest[0] = b->getMin()[0] ; else
  if ( b->getMax()[0] < center[0] ) closest[0] = b->getMax()[0] ; else
                                    closest[0] = center[0] ;

  if ( b->getMin()[1] > center[1] ) closest[1] = b->getMin()[1] ; else
  if ( b->getMax()[1] < center[1] ) closest[1] = b->getMax()[1] ; else
                                    closest[1] = center[1] ;

  if ( b->getMin()[2] > center[2] ) closest[2] = b->getMin()[2] ; else
  if ( b->getMax()[2] < center[2] ) closest[2] = b->getMax()[2] ; else
                                    closest[2] = center[2] ;

  return sgCompare3DSqdDist ( closest, center, sgSquare ( radius ) ) <= 0 ;
}


/************************\
*   sgFrustum routines   *
\************************/

void sgFrustum::update ()
{
  if ( fabs ( ffar - nnear ) < 0.1 )
  {
    fprintf ( stderr, "sgFrustum: Can't support depth of view <0.1 units.\n");
    return ;
  }

  if ( hfov != SG_ZERO && vfov != SG_ZERO )
  {
    if ( fabs ( hfov ) < 0.1 || fabs ( vfov ) < 0.1 )
    {
      fprintf ( stderr, "sgFrustum: Can't support fields of view narrower than 0.1 degrees.\n");
      return ;
    }

    /* Corners of screen relative to eye... */
  
    right = nnear * (SGfloat) tan ( hfov * SG_DEGREES_TO_RADIANS / SG_TWO ) ;
    top   = nnear * (SGfloat) tan ( vfov * SG_DEGREES_TO_RADIANS / SG_TWO ) ;
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

  sgVec3 v1, v2, v3, v4 ;

  sgSetVec3 ( v1, left , top, -nnear ) ;
  sgSetVec3 ( v2, right, top, -nnear ) ;
  sgSetVec3 ( v3, left , bot, -nnear ) ;
  sgSetVec3 ( v4, right, bot, -nnear ) ;

  sgNormaliseVec3 ( v1 ) ;
  sgNormaliseVec3 ( v2 ) ;
  sgNormaliseVec3 ( v3 ) ;
  sgNormaliseVec3 ( v4 ) ;

  /*
    Take care of the order of the parameters so that all the planes
    are oriented facing inwards...
  */

  sgVectorProductVec3 (   top_plane, v1, v2 ) ;
  sgVectorProductVec3 ( right_plane, v2, v4 ) ;
  sgVectorProductVec3 (   bot_plane, v4, v3 ) ;
  sgVectorProductVec3 (  left_plane, v3, v1 ) ;

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

  SGfloat w = right - left ;
  SGfloat h = top   - bot  ;
  SGfloat d = ffar  - nnear ;

  mat[0][0] =  SG_TWO * nnear / w ;
  mat[0][1] =  SG_ZERO ;
  mat[0][2] =  SG_ZERO ;
  mat[0][3] =  SG_ZERO ;

  mat[1][0] =  SG_ZERO ;
  mat[1][1] =  SG_TWO * nnear / h ;
  mat[1][2] =  SG_ZERO ;
  mat[1][3] =  SG_ZERO ;

  mat[2][0] =  ( right + left ) / w ;
  mat[2][1] =  ( top   + bot  ) / h ;
  mat[2][2] = -( ffar  + nnear ) / d ;
  mat[2][3] = -SG_ONE ;

  mat[3][0] =  SG_ZERO ;
  mat[3][1] =  SG_ZERO ;
  mat[3][2] = -SG_TWO * nnear * ffar/ d ;
  mat[3][3] =  SG_ZERO ;
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

int sgFrustum::getOutcode ( const sgVec3 pt ) const 
{
  /* Transform the point by the Frustum's transform. */

  sgVec4 tmp ;

  tmp [ 0 ] = pt [ 0 ] ;
  tmp [ 1 ] = pt [ 1 ] ;
  tmp [ 2 ] = pt [ 2 ] ;
  tmp [ 3 ] =  SG_ONE  ;

  sgXformPnt4 ( tmp, tmp, mat ) ;

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

int sgFrustum::contains ( const sgVec3 pt ) const 
{
  return getOutcode ( pt ) == OC_ALL_ON_SCREEN ;
}


int sgFrustum::contains ( const sgSphere *s ) const 
{
  /*
    Lop off half the database (roughly) with a quick near-plane test - and
    lop off a lot more with a quick far-plane test
  */

  if ( -s->getCenter() [ 2 ] + s->getRadius() < nnear ||
       -s->getCenter() [ 2 ] - s->getRadius() > ffar )
    return SG_OUTSIDE ;

  /*
    OK, so the sphere lies between near and far.

    Measure the distance of the center point from the four sides of the frustum,
    if it's outside by more than the radius then it's history.

    It's tempting to do a quick test to see if the center point is
    onscreen using sgFrustumContainsPt - but that takes a matrix transform
    which is 16 multiplies and 12 adds - versus this test which does the
    whole task using only 12 multiplies and 8 adds.
  */

  SGfloat sp1 = sgScalarProductVec3 (  left_plane, s->getCenter() ) ;
  SGfloat sp2 = sgScalarProductVec3 ( right_plane, s->getCenter() ) ;
  SGfloat sp3 = sgScalarProductVec3 (   bot_plane, s->getCenter() ) ;
  SGfloat sp4 = sgScalarProductVec3 (   top_plane, s->getCenter() ) ;

  if ( -sp1 >= s->getRadius() || -sp2 >= s->getRadius() ||
       -sp3 >= s->getRadius() || -sp4 >= s->getRadius() )
    return SG_OUTSIDE ;
  
  /*
    If it's inside by more than the radius then it's *completely* inside
    and we can save time elsewhere if we know that for sure.
  */

  if ( -s->getCenter() [ 2 ] - s->getRadius() > nnear &&
       -s->getCenter() [ 2 ] + s->getRadius() < ffar &&
       sp1 >= s->getRadius() && sp2 >= s->getRadius() &&
       sp3 >= s->getRadius() && sp4 >= s->getRadius() )
    return SG_INSIDE ;

  return SG_STRADDLE ;
}


void sgMakeCoordMat4 ( sgMat4 m, const SGfloat x, const SGfloat y, const SGfloat z, const SGfloat h, const SGfloat p, const SGfloat r )
{
  double ch, sh, cp, sp, cr, sr, srsp, crsp, srcp ;

  if ( h == SG_ZERO )
  {
    ch = SGD_ONE ;
    sh = SGD_ZERO ;
  }
  else
  {
    sh = (SGfloat) sin( (double)( h * SG_DEGREES_TO_RADIANS )) ;
    ch = (SGfloat) cos( (double)( h * SG_DEGREES_TO_RADIANS )) ;
  }

  if ( p == SG_ZERO )
  {
    cp = SGD_ONE ;
    sp = SGD_ZERO ;
  }
  else
  {
    sp = sin( (double)( p * SG_DEGREES_TO_RADIANS )) ;
    cp = cos( (double)( p * SG_DEGREES_TO_RADIANS )) ;
  }

  if ( r == SG_ZERO )
  {
    cr   = SGD_ONE ;
    sr   = SGD_ZERO ;
    srsp = SGD_ZERO ;
    srcp = SGD_ZERO ;
    crsp = sp ;
  }
  else
  {
    sr   = sin( (double)( r * SG_DEGREES_TO_RADIANS )) ;
    cr   = cos( (double)( r * SG_DEGREES_TO_RADIANS )) ;
    srsp = sr * sp ;
    crsp = cr * sp ;
    srcp = sr * cp ;
  }

  m[0][0] = (SGfloat)(  ch * cr - sh * srsp ) ;
  m[1][0] = (SGfloat)( -sh * cp ) ;
  m[2][0] = (SGfloat)(  sr * ch + sh * crsp ) ;
  m[3][0] =  x ;

  m[0][1] = (SGfloat)( cr * sh + srsp * ch ) ;
  m[1][1] = (SGfloat)( ch * cp ) ;
  m[2][1] = (SGfloat)( sr * sh - crsp * ch ) ;
  m[3][1] =  y ;

  m[0][2] = (SGfloat)( -srcp ) ;
  m[1][2] = (SGfloat)(  sp ) ;
  m[2][2] = (SGfloat)(  cr * cp ) ;
  m[3][2] =  z ;

  m[0][3] =  SG_ZERO ;
  m[1][3] =  SG_ZERO ;
  m[2][3] =  SG_ZERO ;
  m[3][3] =  SG_ONE ;
}


void sgMakeTransMat4 ( sgMat4 m, const sgVec3 xyz )
{
  m[0][1] = m[0][2] = m[0][3] =
  m[1][0] = m[1][2] = m[1][3] =
  m[2][0] = m[2][1] = m[2][3] = SG_ZERO ;
  m[0][0] = m[1][1] = m[2][2] = m[3][3] = SG_ONE ;
  sgCopyVec3 ( m[3], xyz ) ;
}


void sgMakeTransMat4 ( sgMat4 m, const SGfloat x, const SGfloat y, const SGfloat z )
{
  m[0][1] = m[0][2] = m[0][3] =
  m[1][0] = m[1][2] = m[1][3] =
  m[2][0] = m[2][1] = m[2][3] = SG_ZERO ;
  m[0][0] = m[1][1] = m[2][2] = m[3][3] = SG_ONE ;
  sgSetVec3 ( m[3], x, y, z ) ;
}


void sgSetCoord ( sgCoord *dst, const sgMat4 src )
{
  sgCopyVec3 ( dst->xyz, src[3] ) ;
    
  sgMat4 mat ;

  SGfloat s = sgLengthVec3 ( src[0] ) ;

  if ( s <= 0.00001 )
  {
    fprintf ( stderr, "sgMat4ToCoord: ERROR - Bad Matrix.\n" ) ;
    sgSetVec3 ( dst -> hpr, SG_ZERO, SG_ZERO, SG_ZERO ) ;
    return ;
  }

  sgScaleMat4 ( mat, src, SG_ONE / s ) ;
    
  dst->hpr[1] = (SGfloat) asin ( _sgClampToUnity ( mat[1][2] ) ) ;

  SGfloat cp = (SGfloat) cos ( dst->hpr[1] ) ;
    
  /* If pointing nearly vertically up - then heading is ill-defined */

  if ( cp > -0.00001 && cp < 0.00001 )
  {
    SGfloat cr = _sgClampToUnity ( mat[0][1] ) ; 
    SGfloat sr = _sgClampToUnity (-mat[2][1] ) ;

    dst->hpr[0] = SG_ZERO ;
    dst->hpr[2] = (SGfloat) atan2 ( sr, cr ) ;
  }
  else
  {
    SGfloat sr = _sgClampToUnity ( -mat[0][2] / cp ) ;
    SGfloat cr = _sgClampToUnity (  mat[2][2] / cp ) ;
    SGfloat sh = _sgClampToUnity ( -mat[1][0] / cp ) ;
    SGfloat ch = _sgClampToUnity (  mat[1][1] / cp ) ;
	
    if ( (sh == SG_ZERO && ch == SG_ZERO) || (sr == SG_ZERO && cr == SG_ZERO) )
    {
      cr = _sgClampToUnity ( mat[0][1] ) ;
      sr = _sgClampToUnity (-mat[2][1] ) ;

      dst->hpr[0] = SG_ZERO ;
    }
    else
      dst->hpr[0] = (SGfloat) atan2 ( sh, ch ) ;

    dst->hpr[2] = (SGfloat) atan2 ( sr, cr ) ;
  }

  sgScaleVec3 ( dst->hpr, SG_RADIANS_TO_DEGREES ) ;
}


void sgMakeNormal(sgVec3 dst, const sgVec3 a, const sgVec3 b, const sgVec3 c )
{
  sgVec3 ab ; sgSubVec3 ( ab, b, a ) ;
  sgVec3 ac ; sgSubVec3 ( ac, c, a ) ;
  sgVectorProductVec3 ( dst, ab,ac ) ; sgNormaliseVec3 ( dst ) ;
}


void sgPreMultMat4( sgMat4 dst, const sgMat4 src )
{
  sgMat4 mat ;
  sgMultMat4 ( mat, src, dst ) ;
  sgCopyMat4 ( dst, mat ) ;
}

void sgPostMultMat4( sgMat4 dst, const sgMat4 src )
{
  sgMat4 mat ;
  sgMultMat4 ( mat, dst, src ) ;
  sgCopyMat4 ( dst, mat ) ;
}

void sgMultMat4( sgMat4 dst, const sgMat4 m1, const sgMat4 m2 )
{
  if (((m1[0][3] != SG_ZERO) |
       (m1[1][3] != SG_ZERO) |
       (m1[2][3] != SG_ZERO) |
       (m2[0][3] != SG_ZERO) |
       (m2[1][3] != SG_ZERO) |
       (m2[2][3] != SG_ZERO) ) == 0x0 &&
        m1[3][3] == SG_ONE &&
        m2[3][3] == SG_ONE)
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

    dst[0][3] = SG_ZERO ;
    dst[1][3] = SG_ZERO ;
    dst[2][3] = SG_ZERO ;
    dst[3][3] = SG_ONE ;
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


void sgTransposeNegateMat4 ( sgMat4 dst, const sgMat4 src )
{
  /* Poor man's invert - can be used when matrix is a simple rotate-translate */

  dst[0][0] = src[0][0] ;
  dst[1][0] = src[0][1] ;
  dst[2][0] = src[0][2] ;
  dst[3][0] = - sgScalarProductVec3 ( src[3], src[0] ) ;

  dst[0][1] = src[1][0] ;
  dst[1][1] = src[1][1] ;
  dst[2][1] = src[1][2] ;
  dst[3][1] = - sgScalarProductVec3 ( src[3], src[1] ) ;
                                                                               
  dst[0][2] = src[2][0] ;                                                      
  dst[1][2] = src[2][1] ;                                                      
  dst[2][2] = src[2][2] ;                                                      
  dst[3][2] = - sgScalarProductVec3 ( src[3], src[2] ) ;
                                                                               
  dst[0][3] = SG_ZERO ;
  dst[1][3] = SG_ZERO ;                                                        
  dst[2][3] = SG_ZERO ;                                                        
  dst[3][3] = SG_ONE  ;
}


void sgTransposeNegateMat4 ( sgMat4 dst )
{
  sgMat4 src ;
  sgCopyMat4 ( src, dst ) ;
  sgTransposeNegateMat4 ( dst, src ) ;
}



void sgInvertMat4 ( sgMat4 dst, const sgMat4 src )
{
  sgMat4 tmp ;

  sgCopyMat4 ( tmp, src ) ;
  sgMakeIdentMat4 ( dst ) ;

  for ( int i = 0 ; i != 4 ; i++ )
  {
    SGfloat val = tmp[i][i] ;
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
        SGfloat t ;
        t = dst[j][i]; dst[j][i] = dst[j][ind]; dst[j][ind] = t ;
        t = tmp[j][i]; tmp[j][i] = tmp[j][ind]; tmp[j][ind] = t ;
      }
    }

#ifndef FLT_EPSILON
#define FLT_EPSILON 1.19209290e-07f
#endif
    // if ( val == SG_ZERO)
    if ( fabs(val) <= FLT_EPSILON )
    {
      fprintf ( stderr, "sg: ERROR - Singular matrix, no inverse!\n" ) ;
      sgMakeIdentMat4 ( dst ) ;  /* Do *something* */
      return;
    }

    SGfloat ival = SG_ONE / val ;

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



void sgXformVec3 ( sgVec3 dst, const sgVec3 src, const sgMat4 mat )
{
  SGfloat t0 = src[ 0 ] ;
  SGfloat t1 = src[ 1 ] ;
  SGfloat t2 = src[ 2 ] ;

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


void sgXformPnt3 ( sgVec3 dst, const sgVec3 src, const sgMat4 mat )
{
  SGfloat t0 = src[ 0 ] ;
  SGfloat t1 = src[ 1 ] ;
  SGfloat t2 = src[ 2 ] ;

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


void sgXformPnt4 ( sgVec4 dst, const sgVec4 src, const sgMat4 mat )
{
  SGfloat t0 = src[ 0 ] ;
  SGfloat t1 = src[ 1 ] ;
  SGfloat t2 = src[ 2 ] ;
  SGfloat t3 = src[ 3 ] ;

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


void sgFullXformPnt3 ( sgVec3 dst, const sgVec3 src, const sgMat4 mat )
{
  sgVec4 tmp ;

  tmp [ 0 ] = src [ 0 ] ;
  tmp [ 1 ] = src [ 1 ] ;
  tmp [ 2 ] = src [ 2 ] ;
  tmp [ 3 ] =   SG_ONE  ;

  sgXformPnt4 ( tmp, tmp, mat ) ;
  sgScaleVec3 ( dst, tmp, SG_ONE / tmp [ 3 ] ) ;
}

void sgHPRfromVec3 ( sgVec3 hpr, const sgVec3 src )
{
  sgVec3 tmp ;
  sgCopyVec3 ( tmp, src ) ;
  sgNormaliseVec3 ( tmp ) ;
  hpr[0] = -(SGfloat) atan2 ( tmp [ 0 ], tmp [ 1 ] ) * SG_RADIANS_TO_DEGREES ;
  hpr[1] = -(SGfloat) atan2 ( tmp [ 2 ], sqrt ( sgSquare ( tmp [ 0 ] ) +
                                                sgSquare ( tmp [ 1 ] ) ) ) *
                                                SG_RADIANS_TO_DEGREES ;
  hpr[2] = 0.0f ;
}



/*
  Quaternion routines are Copyright (C) 1999
  Kevin B. Thompson <kevinbthompson@yahoo.com>
  Modified by Sylvan W. Clebsch <sylvan@stanford.edu>
*/


void sgQuatToAngleAxis ( SGfloat *angle,
                         SGfloat *x, SGfloat *y, SGfloat *z,
                         const sgQuat *src )
{
  sgVec3 axis ;

  sgQuatToAngleAxis ( angle, axis, src ) ;

  *x = axis [ 0 ] ;
  *y = axis [ 1 ] ;
  *z = axis [ 2 ] ;
}


void sgQuatToAngleAxis ( SGfloat *angle, sgVec3 axis, const sgQuat *src )
{
  SGfloat a = (SGfloat) acos ( src->w ) ;
  SGfloat s = (SGfloat) sin  ( a ) ;

  *angle = a * SG_RADIANS_TO_DEGREES * SG_TWO ;

  if ( s == SG_ZERO )
    sgSetVec3 ( axis, SG_ZERO, SG_ZERO, SG_ONE );
  else
  {
    sgSetVec3   ( axis, src->x, src->y, src->z ) ;
    sgScaleVec3 ( axis, SG_ONE / s ) ;
  }
}


void sgMakeQuat ( sgQuat *dst, const sgVec3 hpr )
{
  sgVec3 temp_hpr;

  temp_hpr[0] = hpr[0] * SG_DEGREES_TO_RADIANS / SG_TWO ;
  temp_hpr[1] = hpr[1] * SG_DEGREES_TO_RADIANS / SG_TWO ;
  temp_hpr[2] = hpr[2] * SG_DEGREES_TO_RADIANS / SG_TWO ;

  SGfloat sh = -(SGfloat) sin ( temp_hpr[0] ) ; SGfloat ch = (SGfloat) cos ( temp_hpr[0] ) ;
  SGfloat sp = -(SGfloat) sin ( temp_hpr[1] ) ; SGfloat cp = (SGfloat) cos ( temp_hpr[1] ) ;
  SGfloat sr = -(SGfloat) sin ( temp_hpr[2] ) ; SGfloat cr = (SGfloat) cos ( temp_hpr[2] ) ;

  SGfloat cpch = cp * ch;
  SGfloat spsh = sp * sh;

  dst->w = cr * cpch + sr * spsh;
  dst->x = sr * cpch - cr * spsh;
  dst->y = cr * sp * ch + sr * cp * sh;
  dst->z = cr * cp * sh - sr * sp * ch;
}


void sgMakeQuat ( sgQuat *dst, const SGfloat angle, const SGfloat x, const SGfloat y, const SGfloat z )
{
  sgVec3 axis ; 
  sgSetVec3 ( axis, x, y, z ) ;
  sgMakeQuat ( dst, angle, axis ) ;
}


void sgMakeQuat ( sgQuat *dst, const SGfloat angle, const sgVec3 axis )
{
  SGfloat temp_angle = angle * SG_DEGREES_TO_RADIANS / SG_TWO ;

  sgVec3 ax ;
  sgNormaliseVec3 ( ax, axis ) ;

  SGfloat s = - (SGfloat) sin ( temp_angle ) ;

  dst->w = (SGfloat) cos ( temp_angle ) ;
  dst->x = s * ax[0] ;
  dst->y = s * ax[1] ;
  dst->z = s * ax[2] ;
}


void sgMultQuat ( sgQuat *dst, const sgQuat *a, const sgQuat *b )
{
  /* [ ww' - v.v', vxv' + wv' + v'w ] */

  SGfloat t[8];

  t[0] = (a->w + a->x) * (b->w + b->x);
  t[1] = (a->z - a->y) * (b->y - b->z);
  t[2] = (a->x - a->w) * (b->y + b->z);
  t[3] = (a->y + a->z) * (b->x - b->w);
  t[4] = (a->x + a->z) * (b->x + b->y);
  t[5] = (a->x - a->z) * (b->x - b->y);
  t[6] = (a->w + a->y) * (b->w - b->z);
  t[7] = (a->w - a->y) * (b->w + b->z);

  dst->w =  t[1] + ((-t[4] - t[5] + t[6] + t[7]) * 0.5f);
  dst->x =  t[0] - (( t[4] + t[5] + t[6] + t[7]) * 0.5f);
  dst->y = -t[2] + (( t[4] - t[5] + t[6] - t[7]) * 0.5f);
  dst->z = -t[3] + (( t[4] - t[5] - t[6] + t[7]) * 0.5f);
}


void sgMakeRotMat4 ( sgMat4 dst, const sgQuat *q )
{
  SGfloat two_xx = q->x * (q->x + q->x) ;
  SGfloat two_xy = q->x * (q->y + q->y) ;
  SGfloat two_xz = q->x * (q->z + q->z) ;

  SGfloat two_wx = q->w * (q->x + q->x) ;
  SGfloat two_wy = q->w * (q->y + q->y) ;
  SGfloat two_wz = q->w * (q->z + q->z) ;

  SGfloat two_yy = q->y * (q->y + q->y) ;
  SGfloat two_yz = q->y * (q->z + q->z) ;

  SGfloat two_zz = q->z * (q->z + q->z) ;

  sgSetVec4 ( dst[0], SG_ONE-(two_yy+two_zz), two_xy-two_wz, two_xz+two_wy, SG_ZERO ) ;
  sgSetVec4 ( dst[1], two_xy+two_wz, SG_ONE-(two_xx+two_zz), two_yz-two_wx, SG_ZERO ) ;
  sgSetVec4 ( dst[2], two_xz-two_wy, two_yz+two_wx, SG_ONE-(two_xx+two_yy), SG_ZERO ) ;
  sgSetVec4 ( dst[3], SG_ZERO, SG_ZERO, SG_ZERO, SG_ONE ) ;
}


void sgSlerpQuat( sgQuat *dst, const sgQuat *from, const sgQuat *to, const SGfloat t )
{
  SGfloat sign, co, scale0, scale1;

  /* SWC - Interpolate between to quaternions */

  co = from->x * to->x + from->y * to->y + from->z * to->z + from->w * to->w;

  if( co < SG_ZERO )
  {
    co = -co;
    sign = -SG_ONE;
  }
  else
    sign = SG_ONE;

  if( co < SG_ONE )
  {
    SGfloat o = (SGfloat)acos( co );
    SGfloat so = (SGfloat)sin( o );

    scale0 = (SGfloat)sin( (SG_ONE - t) * o ) / so;
    scale1 = (SGfloat)sin( t * o ) / so;
  }
  else
  {
    scale0 = SG_ONE - t;
    scale1 = t;
  }

  dst->x = scale0 * from->x + scale1 * ((sign > SG_ZERO) ? to->x : -to->x);
  dst->y = scale0 * from->y + scale1 * ((sign > SG_ZERO) ? to->y : -to->y);
  dst->z = scale0 * from->z + scale1 * ((sign > SG_ZERO) ? to->z : -to->z);
  dst->w = scale0 * from->w + scale1 * ((sign > SG_ZERO) ? to->w : -to->w);
}



