
#include "ssgLocal.h"

#define HL_DELTA 0.04f

ssgVtxArray::ssgVtxArray () : ssgVtxTable()
{
  type |= SSG_TYPE_VTXARRAY ;
  indices = NULL;
}


ssgVtxArray::ssgVtxArray ( GLenum ty,
                ssgVertexArray   *vl,
                ssgNormalArray   *nl,
                ssgTexCoordArray *tl,
                ssgColourArray   *cl,
				ssgIndexArray    *il ) : ssgVtxTable( ty, vl, nl, tl, cl )
{
  type |= SSG_TYPE_VTXARRAY ;

  indices  = (il!=NULL) ? il : new ssgIndexArray () ;

  recalcBSphere () ;
}


ssgVtxArray::~ssgVtxArray ()
{
  ssgDeRefDelete ( indices ) ;
} 


void ssgVtxArray::transform ( sgMat4 m )
{
  int i ;

  for ( i = 0 ; i < getNumIndices() ; i++ )
  {
	int ii = *( indices->get(i) );
    sgXformPnt3 ( vertices->get(ii), vertices->get(ii), m ) ;
  }

  for ( i = 0 ; i < getNumIndices() ; i++ )
  {
	int ii = *( indices->get(i) );
    sgXformPnt3 ( normals->get(ii), normals->get(ii), m ) ;
  }

  recalcBSphere () ;
}


void ssgVtxArray::recalcBSphere ()
{
  emptyBSphere () ;
  bbox . empty () ;

  for ( int i = 0 ; i < getNumIndices() ; i++ )
  {
	int ii = *( indices->get(i) );
    bbox . extend ( vertices->get(ii) ) ;
  }

  extendBSphere ( & bbox ) ;
  dirtyBSphere () ;  /* Cause parents to redo their bspheres */
  bsphere_is_invalid = FALSE ;
}


void ssgVtxArray::drawHighlight ( sgVec4 colour )
{
  _ssgForceLineState () ;

  glPushClientAttrib ( GL_CLIENT_VERTEX_ARRAY_BIT ) ;

  glDisableClientState ( GL_COLOR_ARRAY ) ;
  glDisableClientState ( GL_NORMAL_ARRAY ) ;
  glDisableClientState ( GL_TEXTURE_COORD_ARRAY ) ;
  glEnableClientState ( GL_VERTEX_ARRAY ) ;

  glVertexPointer ( 3, GL_FLOAT, 0, vertices->get(0) ) ;

  glPushAttrib ( GL_POLYGON_BIT ) ;
  glPolygonMode ( GL_FRONT_AND_BACK, GL_LINE ) ;
  glColor4fv ( colour ) ;
  int i = getNumIndices ();
  int *ii = indices->get(0);
  glDrawElements ( gltype, i, GL_UNSIGNED_INT, ii ) ;
  glPopAttrib () ;
  glPopClientAttrib ( ) ;
  glEnable ( GL_DEPTH_TEST ) ;
}


void ssgVtxArray::drawHighlight ( sgVec4 colour, int i )
{
  _ssgForceLineState () ;

  if ( i < 0 || i >= getNumIndices () )
    return ;

  int ii = *( indices->get(i) );
  sgVec3 *vx = (sgVec3 *) vertices -> get(ii) ;

  float x = vx[0][0] ;
  float y = vx[0][1] ;
  float z = vx[0][2] ;

  sgVec3 t[6] ;
  sgSetVec3 ( t[0], x-HL_DELTA,y,z ) ;
  sgSetVec3 ( t[1], x+HL_DELTA,y,z ) ;
  sgSetVec3 ( t[2], x,y-HL_DELTA,z ) ;
  sgSetVec3 ( t[3], x,y+HL_DELTA,z ) ;
  sgSetVec3 ( t[4], x,y,z-HL_DELTA ) ;
  sgSetVec3 ( t[5], x,y,z+HL_DELTA ) ;

  glPushClientAttrib ( GL_CLIENT_VERTEX_ARRAY_BIT ) ;
  glDisableClientState ( GL_COLOR_ARRAY ) ;
  glDisableClientState ( GL_NORMAL_ARRAY ) ;
  glDisableClientState ( GL_TEXTURE_COORD_ARRAY ) ;
  glEnableClientState ( GL_VERTEX_ARRAY ) ;
  glVertexPointer ( 3, GL_FLOAT, 0, t ) ;
  glColor4fv ( colour ) ;
  glLineWidth ( 4.0f ) ;
  glDrawArrays ( GL_LINES, 0, 6 ) ;
  glLineWidth ( 1.0f ) ;
  glPopClientAttrib ( ) ;
  glEnable ( GL_DEPTH_TEST ) ;
}


void ssgVtxArray::draw ()
{
  if ( ! preDraw () )
    return ;

  if ( hasState () ) getState () -> apply () ;

  stats_num_leaves++ ;
  stats_num_vertices += getNumIndices() ;

#ifdef _SSG_USE_DLIST
  if ( dlist )
    glCallList ( dlist ) ;
  else
#endif
    draw_geometry () ;

  if ( postDrawCB != NULL )
    (*postDrawCB)(this) ;
}


#ifdef _SSG_USE_PICK
void ssgVtxArray::pick ( int baseName )
{
  int i ;
  int num_vertices  = getNumIndices () ;

  glPushClientAttrib ( GL_CLIENT_VERTEX_ARRAY_BIT ) ;
  glEnableClientState ( GL_VERTEX_ARRAY ) ;
  glVertexPointer ( 3, GL_FLOAT, 0, vertices->get(0) ) ;

  /* Test the entire primitive. */

  glPushName ( baseName ) ;
  int *ii = indices->get(0);
  glDrawElements ( gltype, num_vertices, GL_UNSIGNED_INT, ii ) ;

  /* Then test each vertex in turn */

  for ( i = 0 ; i < num_vertices ; i++ )
  {
	int ii = *( indices->get(i) );
    glLoadName ( baseName + i + 1 ) ;
    glBegin ( GL_POINTS ) ;
    glArrayElement ( ii ) ;
    glEnd () ;
  }

  glPopName () ;

  glPopClientAttrib ( ) ;
}
#endif // #ifdef _SSG_USE_PICK


void ssgVtxArray::draw_geometry ()
{
  int num_colours   = getNumColours   () ;
  int num_normals   = getNumNormals   () ;
  int num_texcoords = getNumTexCoords () ;

  sgVec3 *nm = (sgVec3 *) normals   -> get(0) ;
  sgVec4 *cl = (sgVec4 *) colours   -> get(0) ;

  if ( num_colours == 0 ) glColor4f   ( 1.0f, 1.0f, 1.0f, 1.0f ) ;
  if ( num_colours == 1 ) glColor4fv  ( cl [ 0 ] ) ;
  if ( num_normals == 1 ) glNormal3fv ( nm [ 0 ] ) ;
  
  glPushClientAttrib ( GL_CLIENT_VERTEX_ARRAY_BIT ) ;

  if ( num_colours > 1 )
  {
    glEnableClientState ( GL_COLOR_ARRAY ) ;
    glColorPointer ( 4, GL_FLOAT, 0, colours->get(0) ) ;
  }
  if ( num_normals > 1 )
  {
    glEnableClientState ( GL_NORMAL_ARRAY ) ;
    glNormalPointer ( GL_FLOAT, 0, normals->get(0) ) ;
  }
  if ( num_texcoords > 1 )
  {
    glEnableClientState ( GL_TEXTURE_COORD_ARRAY ) ;
    glTexCoordPointer ( 2, GL_FLOAT, 0, texcoords->get(0) ) ;
  }
  glEnableClientState ( GL_VERTEX_ARRAY ) ;
  glVertexPointer ( 3, GL_FLOAT, 0, vertices->get(0) ) ;

  int i = getNumIndices ();
  int *ii = indices->get(0);
  glDrawElements ( gltype, i, GL_UNSIGNED_INT, ii ) ;

  glPopClientAttrib ( ) ;
}


int ssgVtxArray::getNumTriangles ()
{
  switch ( getGLtype () )
  {
    case GL_POLYGON :
    case GL_TRIANGLE_FAN :
      return getNumIndices() - 2 ;

    case GL_TRIANGLES :
      return getNumIndices() / 3 ;

    case GL_TRIANGLE_STRIP :
      return getNumIndices() - 2 ;

    case GL_QUADS :
      return ( getNumIndices() / 4 ) * 2 ;

    case GL_QUAD_STRIP :
      return ( ( getNumIndices() - 2 ) / 2 ) * 2 ;

    default : break ;
  }

  return 0 ;   /* Should never get here...but you never know! */
}


// It would be nice to have this only in the ssgVtxTable.
void ssgVtxArray::hot_triangles ( sgVec3 s, sgMat4 m, int /* test_needed */ )
{
  int nt = getNumTriangles () ;

  stats_hot_triangles += nt ;

  for ( int i = 0 ; i < nt ; i++ )
  {
    short   v1,  v2,  v3 ;
    sgVec3 vv1, vv2, vv3 ;
    sgVec4 plane ;

    getTriangle ( i, &v1, &v2, &v3 ) ;

	v1 = *( indices->get(v1) ) ;
	v2 = *( indices->get(v2) ) ;
	v3 = *( indices->get(v3) ) ;

    sgXformPnt3 ( vv1, getVertex(v1), m ) ;
    sgXformPnt3 ( vv2, getVertex(v2), m ) ;
    sgXformPnt3 ( vv3, getVertex(v3), m ) ;

    /*
      Does the X/Y coordinate lie outside the triangle's bbox, or
      does the Z coordinate lie beneath the bbox ?
    */

    if ( ( s[0] < vv1[0] && s[0] < vv2[0] && s[0] < vv3[0] ) ||
         ( s[1] < vv1[1] && s[1] < vv2[1] && s[1] < vv3[1] ) ||
         ( s[0] > vv1[0] && s[0] > vv2[0] && s[0] > vv3[0] ) ||
         ( s[1] > vv1[1] && s[1] > vv2[1] && s[1] > vv3[1] ) ||
         ( s[2] < vv1[2] && s[2] < vv2[2] && s[2] < vv3[2] ) )
      continue ;

    sgMakePlane ( plane, vv1, vv2, vv3 ) ;

    if ( _ssgIsHotTest )
    {
      /* No HOT from upside-down or vertical triangles */

      if ( getCullFace() && plane [ 2 ] <= 0 )
        continue ;

      /* Find the point vertically below the text point
	as it crosses the plane of the polygon */

      float z = sgHeightOfPlaneVec2 ( plane, s ) ;

      /* No HOT from below the triangle */

      if ( z > s[2] )
	continue ;

      /* Outside the vertical extent of the triangle? */

      if ( ( z < vv1[2] && z < vv2[2] && z < vv3[2] ) ||
	   ( z > vv1[2] && z > vv2[2] && z > vv3[2] ) )
	continue ;
    }

    /*
      Now it gets messy - the isect point is inside
      the bbox of the triangle - but that's not enough.
      Is it inside the triangle itself?
    */

    float  e1 =  s [0] * vv1[1] -  s [1] * vv1[0] ;
    float  e2 =  s [0] * vv2[1] -  s [1] * vv2[0] ;
    float  e3 =  s [0] * vv3[1] -  s [1] * vv3[0] ;
    float ep1 = vv1[0] * vv2[1] - vv1[1] * vv2[0] ;
    float ep2 = vv2[0] * vv3[1] - vv2[1] * vv3[0] ;
    float ep3 = vv3[0] * vv1[1] - vv3[1] * vv1[0] ;

    float ap = fabs ( ep1 + ep2 + ep3 ) ;
    float ai = fabs ( e1 + ep1 - e2 ) +
               fabs ( e2 + ep2 - e3 ) +
               fabs ( e3 + ep3 - e1 ) ;

    if ( ai > ap * 1.01 )
      continue ;

    _ssgAddHit ( this, i, m, plane ) ;
  }
}


// It would be nice to have this only in the ssgVtxTable.
void ssgVtxArray::isect_triangles ( sgSphere *s, sgMat4 m, int test_needed )
{
  int nt = getNumTriangles () ;

  stats_isect_triangles += nt ;

  for ( int i = 0 ; i < nt ; i++ )
  {
    short   v1,  v2,  v3 ;
    sgVec3 vv1, vv2, vv3 ;
    sgVec4 plane ;

    getTriangle ( i, &v1, &v2, &v3 ) ;

	v1 = *( indices->get(v1) ) ;
	v2 = *( indices->get(v2) ) ;
	v3 = *( indices->get(v3) ) ;

    sgXformPnt3 ( vv1, getVertex(v1), m ) ;
    sgXformPnt3 ( vv2, getVertex(v2), m ) ;
    sgXformPnt3 ( vv3, getVertex(v3), m ) ;

    sgMakePlane ( plane, vv1, vv2, vv3 ) ;

    if ( ! test_needed )
    {
      _ssgAddHit ( this, i, m, plane ) ;
      continue ;
    }

    float dp = fabs ( sgDistToPlaneVec3 ( plane, s->getCenter() ) ) ;

    if ( dp > s->getRadius() )
      continue ;

    /*
      The BSphere touches the plane containing
      the triangle - but does it actually touch
      the triangle itself?  Let's erect some
      vertical walls around the triangle.
    */

    /*
      Construct a 'wall' as a plane through
      two vertices and a third vertex made
      by adding the surface normal to the
      first of those two vertices.
    */

    sgVec3 vvX ;
    sgVec4 planeX ;

    sgAddVec3 ( vvX, plane, vv1 ) ;
    sgMakePlane ( planeX, vv1, vv2, vvX ) ;
    float dp1 = sgDistToPlaneVec3 ( planeX, s->getCenter() ) ;
    
    if ( dp1 > s->getRadius() )
      continue ;

    sgAddVec3 ( vvX, plane, vv2 ) ;
    sgMakePlane ( planeX, vv2, vv3, vvX ) ;
    float dp2 = sgDistToPlaneVec3 ( planeX, s->getCenter() ) ;
    
    if ( dp2 > s->getRadius() )
      continue ;

    sgAddVec3 ( vvX, plane, vv3 ) ;
    sgMakePlane ( planeX, vv3, vv1, vvX ) ;
    float dp3 = sgDistToPlaneVec3 ( planeX, s->getCenter() ) ;
    
    if ( dp3 > s->getRadius() )
      continue ;

    /*
      OK, so we now know that the sphere
      intersects the plane of the triangle
      and is not more than one radius outside
      the walls. However, you can still get
      close enough to the wall and to the
      triangle itself and *still* not
      intersect the triangle itself.

      However, if the center is inside the
      triangle then we don't need that
      costly test.
    */
 
    if ( dp1 <= 0 && dp2 <= 0 && dp3 <= 0 )
    {
      _ssgAddHit ( this, i, m, plane ) ;
      continue ;
    }

    /*
      <sigh> ...now we really need that costly set of tests...

      If the sphere penetrates the plane of the triangle
      and the plane of the wall, then we can use pythagoras
      to determine if the sphere actually intersects that
      edge between the wall and the triangle.

        if ( dp_sqd + dp1_sqd > radius_sqd ) ...in! else ...out!
    */

    float r2 = s->getRadius() * s->getRadius() - dp * dp ;

    if ( dp1 * dp1 <= r2 ||
         dp2 * dp2 <= r2 ||
         dp3 * dp3 <= r2 )
    {
      _ssgAddHit ( this, i, m, plane ) ;
      continue ;
    }
  }
}


void ssgVtxArray::print ( FILE *fd, char *indent )
{
  ssgVtxTable::print ( fd, indent ) ;
  indices   -> print ( fd, indent ) ;
}


int ssgVtxArray::load ( FILE *fd )
{
  if ( ! ssgVtxTable::load(fd) )
    return FALSE ;

  indices = new ssgIndexArray () ; indices -> ref () ;

  if ( ! indices -> load ( fd ) )
  {
    fprintf ( stderr, "loadSSG: Failed to read index array.\n" ) ;
    return FALSE ;
  }

  return TRUE ;
}


int ssgVtxArray::save ( FILE *fd )
{
  ssgVtxTable::save(fd) ;

  if ( ! indices -> save ( fd ) )
  {
    fprintf ( stderr, "saveSSG: Failed to write index array\n" ) ;
    return FALSE ;
  }

  return TRUE ;
}



