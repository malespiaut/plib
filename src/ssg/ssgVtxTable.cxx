
#include "ssgLocal.h"

#define HL_DELTA 0.04f

void ssgVtxTable::copy_from ( ssgVtxTable *src, int clone_flags )
{
  ssgLeaf::copy_from ( src, clone_flags ) ;

  gltype = src -> getPrimitiveType () ;

  if ( src->vertices != NULL && ( clone_flags & SSG_CLONE_GEOMETRY ) )
    vertices = (ssgVertexArray *)( src -> vertices -> clone ( clone_flags )) ;
  else
    vertices = src -> vertices ;

  if ( src->normals != NULL && ( clone_flags & SSG_CLONE_GEOMETRY ) )
    normals = (ssgNormalArray *)( src -> normals -> clone ( clone_flags )) ;
  else
    normals = src -> normals ;

  if ( src->texcoords != NULL && ( clone_flags & SSG_CLONE_GEOMETRY ) )
    texcoords = (ssgTexCoordArray *)( src -> texcoords -> clone ( clone_flags )) ;
  else
    texcoords = src -> texcoords ;

  if ( src->colours != NULL && ( clone_flags & SSG_CLONE_GEOMETRY ) )
    colours = (ssgColourArray *)( src -> colours -> clone ( clone_flags )) ;
  else
    colours = src -> colours ;

  if ( vertices  != NULL ) vertices  -> ref () ;
  if ( normals   != NULL ) normals   -> ref () ;
  if ( texcoords != NULL ) texcoords -> ref () ;
  if ( colours   != NULL ) colours   -> ref () ;

  recalcBSphere () ;
}

ssgBase *ssgVtxTable::clone ( int clone_flags )
{
  ssgVtxTable *b = new ssgVtxTable ;
  b -> copy_from ( this, clone_flags ) ;
  return b ;
}


ssgVtxTable::ssgVtxTable ()
{
  type |= SSG_TYPE_VTXTABLE ;
  gltype = GL_POINTS ;
  vertices  = NULL ;
  normals   = NULL ;
  texcoords = NULL ;
  colours   = NULL ;
}


ssgVtxTable::ssgVtxTable ( GLenum ty,
                ssgVertexArray   *vl,
                ssgNormalArray   *nl,
                ssgTexCoordArray *tl,
                ssgColourArray   *cl )
{
  gltype = ty ;
  type |= SSG_TYPE_VTXTABLE ;

  vertices  = (vl!=NULL) ? vl : new ssgVertexArray   () ;
  normals   = (nl!=NULL) ? nl : new ssgNormalArray   () ;
  texcoords = (tl!=NULL) ? tl : new ssgTexCoordArray () ;
  colours   = (cl!=NULL) ? cl : new ssgColourArray   () ;

  vertices  -> ref () ;
  normals   -> ref () ;
  texcoords -> ref () ;
  colours   -> ref () ;

  recalcBSphere () ;
}

void ssgVtxTable::setVertices ( ssgVertexArray *vl )
{
  ssgDeRefDelete ( vertices ) ;
  vertices = vl ;

  if ( vertices != NULL )
    vertices -> ref () ;

  recalcBSphere () ;
}

void ssgVtxTable::setNormals ( ssgNormalArray *nl )
{
  ssgDeRefDelete ( normals ) ;
  normals = nl ;

  if ( normals != NULL )
    normals -> ref () ;
}

void ssgVtxTable::setTexCoords ( ssgTexCoordArray *tl )
{
  ssgDeRefDelete ( texcoords ) ;
  texcoords = tl ;

  if ( texcoords != NULL )
    texcoords -> ref () ;
}

void ssgVtxTable::setColours ( ssgColourArray *cl )
{
  ssgDeRefDelete ( colours ) ;
  colours = cl ;

  if ( colours != NULL )
    colours -> ref () ;
}

ssgVtxTable::~ssgVtxTable ()
{
  ssgDeRefDelete ( vertices  ) ;
  ssgDeRefDelete ( normals   ) ;
  ssgDeRefDelete ( texcoords ) ;
  ssgDeRefDelete ( colours   ) ;
} 


void ssgVtxTable::getTriangle ( int n, short *v1, short *v2, short *v3 )
{
  switch ( getGLtype () )
  {
    case GL_POLYGON :
    case GL_TRIANGLE_FAN :
      *v1 =  0  ;
      *v2 = n+1 ;
      *v3 = n+2 ;
      return ;

    case GL_TRIANGLES :
      *v1 = n*3 ;
      *v2 = n*3+1 ;
      *v3 = n*3+2 ;
      return ;

    case GL_TRIANGLE_STRIP :
    case GL_QUAD_STRIP :
      if ( n & 1 )
      {
        *v3 =  n  ;
        *v2 = n+1 ;
        *v1 = n+2 ;
      }
      else
      {
        *v1 =  n  ;
        *v2 = n+1 ;
        *v3 = n+2 ;
      }
      return ;

    case GL_QUADS :
      *v1 = (n/2)*4 + (n&1) + 0 ;
      *v2 = (n/2)*4 + (n&1) + 1 ;
      *v3 = (n/2)*4 + (n&1) + 2 ;
      return ;

    default : return ;
  }
}


int ssgVtxTable::getNumTriangles ()
{
  switch ( getGLtype () )
  {
    case GL_POLYGON :
    case GL_TRIANGLE_FAN :
      return getNumVertices() - 2 ;

    case GL_TRIANGLES :
      return getNumVertices() / 3 ;

    case GL_TRIANGLE_STRIP :
      return getNumVertices() - 2 ;

    case GL_QUADS :
      return ( getNumVertices() / 4 ) * 2 ;

    case GL_QUAD_STRIP :
      return ( ( getNumVertices() - 2 ) / 2 ) * 2 ;

    default : break ;
  }

  return 0 ;   /* Should never get here...but you never know! */
}


void ssgVtxTable::transform ( sgMat4 m )
{
  int i ;

  for ( i = 0 ; i < getNumVertices() ; i++ )
    sgXformPnt3 ( vertices->get(i), vertices->get(i), m ) ;

  for ( i = 0 ; i < getNumNormals() ; i++ )
    sgXformVec3 ( normals->get(i), normals->get(i), m ) ;

  recalcBSphere () ;
}


void ssgVtxTable::recalcBSphere ()
{
  emptyBSphere () ;
  bbox . empty () ;

  for ( int i = 0 ; i < getNumVertices() ; i++ )
    bbox . extend ( vertices->get(i) ) ;

  extendBSphere ( & bbox ) ;
  dirtyBSphere () ;  /* Cause parents to redo their bspheres */
  bsphere_is_invalid = FALSE ;
}


void ssgVtxTable::drawHighlight ( sgVec4 colour )
{
  _ssgForceLineState () ;

  int i ;
  int num_vertices  = getNumVertices  () ;

  sgVec3 *vx = (sgVec3 *) vertices -> get(0) ;

  glPushAttrib ( GL_POLYGON_BIT ) ;
  glPolygonMode ( GL_FRONT_AND_BACK, GL_LINE ) ;
  glColor4fv ( colour ) ;
  glBegin ( gltype ) ;
  for ( i = 0 ; i < num_vertices ; i++ )
    glVertex3fv ( vx [ i ] ) ;
  glEnd () ;
  glPopAttrib () ;
  glEnable ( GL_DEPTH_TEST ) ;
}


void ssgVtxTable::drawHighlight ( sgVec4 colour, int v )
{
  _ssgForceLineState () ;

  int num_vertices  = getNumVertices  () ;

  if ( v < 0 || v >= num_vertices )
    return ;

  sgVec3 *vx = (sgVec3 *) vertices -> get(v) ;

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
  glColor4fv ( colour ) ;
  glLineWidth ( 4.0f ) ;
  glBegin ( GL_LINES ) ;
  glVertex3fv ( t[0] ) ;
  glVertex3fv ( t[1] ) ;
  glVertex3fv ( t[2] ) ;
  glVertex3fv ( t[3] ) ;
  glVertex3fv ( t[4] ) ;
  glVertex3fv ( t[5] ) ;
  glEnd () ;
  glLineWidth ( 1.0f ) ;
  glEnable ( GL_DEPTH_TEST ) ;
}


void ssgVtxTable::draw ()
{
  if ( ! preDraw () )
    return ;

  if ( hasState () ) getState () -> apply () ;

  stats_num_leaves++ ;
  stats_num_vertices += getNumVertices() ;

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
void ssgVtxTable::pick ( int baseName )
{
  int i ;
  int num_vertices  = getNumVertices  () ;

  sgVec3 *vx = (sgVec3 *) vertices -> get(0) ;

  /* Test the entire primitive. */

  glPushName ( baseName ) ;
  glBegin  ( gltype ) ;

  for ( i = 0 ; i < num_vertices ; i++ )
    glVertex3fv ( vx [ i ] ) ;
 
  glEnd     () ;

  /* Then test each vertex in turn */

  for ( i = 0 ; i < num_vertices ; i++ )
  {
    glLoadName  ( baseName + i + 1 ) ;
    glBegin  ( GL_POINTS ) ;
    glVertex3fv ( vx [ i ] ) ;
    glEnd     () ;
  }

  glPopName () ;
}
#endif // #ifdef _SSG_USE_PICK


void ssgVtxTable::draw_geometry ()
{
  int num_colours   = getNumColours   () ;
  int num_normals   = getNumNormals   () ;
  int num_vertices  = getNumVertices  () ;
  int num_texcoords = getNumTexCoords () ;

  sgVec3 *vx = (sgVec3 *) vertices  -> get(0) ;
  sgVec3 *nm = (sgVec3 *) normals   -> get(0) ;
  sgVec2 *tx = (sgVec2 *) texcoords -> get(0) ;
  sgVec4 *cl = (sgVec4 *) colours   -> get(0) ;

  glBegin ( gltype ) ;

  if ( num_colours == 0 ) glColor4f   ( 1.0f, 1.0f, 1.0f, 1.0f ) ;
  if ( num_colours == 1 ) glColor4fv  ( cl [ 0 ] ) ;
  if ( num_normals == 1 ) glNormal3fv ( nm [ 0 ] ) ;
  
  for ( int i = 0 ; i < num_vertices ; i++ )
  {
    if ( num_colours   > 1 ) glColor4fv    ( cl [ i ] ) ;
    if ( num_normals   > 1 ) glNormal3fv   ( nm [ i ] ) ;
    if ( num_texcoords > 1 ) glTexCoord2fv ( tx [ i ] ) ;

    glVertex3fv ( vx [ i ] ) ;
  }
 
  glEnd () ;
}



void ssgVtxTable::hot_triangles ( sgVec3 s, sgMat4 m, int /* test_needed */ )
{
  int nt = getNumTriangles () ;

  stats_hot_triangles += nt ;

  for ( int i = 0 ; i < nt ; i++ )
  {
    short   v1,  v2,  v3 ;
    sgVec3 vv1, vv2, vv3 ;
    sgVec4 plane ;

    getTriangle ( i, &v1, &v2, &v3 ) ;

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

    float ap = (float) fabs ( ep1 + ep2 + ep3 ) ;
    float ai = (float) ( fabs ( e1 + ep1 - e2 ) +
                         fabs ( e2 + ep2 - e3 ) +
                         fabs ( e3 + ep3 - e1 ) ) ;

    if ( ai > ap * 1.01 )
      continue ;

    _ssgAddHit ( this, i, m, plane ) ;
  }
}


void ssgVtxTable::isect_triangles ( sgSphere *s, sgMat4 m, int test_needed )
{
  int nt = getNumTriangles () ;

  stats_isect_triangles += nt ;

  for ( int i = 0 ; i < nt ; i++ )
  {
    short   v1,  v2,  v3 ;
    sgVec3 vv1, vv2, vv3 ;
    sgVec4 plane ;

    getTriangle ( i, &v1, &v2, &v3 ) ;

    sgXformPnt3 ( vv1, getVertex(v1), m ) ;
    sgXformPnt3 ( vv2, getVertex(v2), m ) ;
    sgXformPnt3 ( vv3, getVertex(v3), m ) ;

    sgMakePlane ( plane, vv1, vv2, vv3 ) ;

    if ( ! test_needed )
    {
      _ssgAddHit ( this, i, m, plane ) ;
      continue ;
    }

    float dp = (float) fabs ( sgDistToPlaneVec3 ( plane, s->getCenter() ) ) ;

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


void ssgVtxTable::print ( FILE *fd, char *indent )
{
  ssgLeaf  ::print ( fd, indent ) ;
  vertices ->print ( fd, indent ) ;
  normals  ->print ( fd, indent ) ;
  texcoords->print ( fd, indent ) ;
  colours  ->print ( fd, indent ) ;
}



int ssgVtxTable::load ( FILE *fd )
{
  sgVec3 temp;

  _ssgReadVec3  ( fd, temp ); bbox.setMin( temp ) ;
  _ssgReadVec3  ( fd, temp ); bbox.setMax( temp ) ;
  _ssgReadInt   ( fd, (int *)(&gltype) ) ;

  if ( ! ssgLeaf::load(fd) )
    return FALSE ;

  vertices  = new ssgVertexArray   () ; vertices  -> ref () ;
  normals   = new ssgNormalArray   () ; normals   -> ref () ;
  texcoords = new ssgTexCoordArray () ; texcoords -> ref () ;
  colours   = new ssgColourArray   () ; colours   -> ref () ;

  if ( ! vertices  -> load ( fd ) ||
       ! normals   -> load ( fd ) ||
       ! texcoords -> load ( fd ) ||
       ! colours   -> load ( fd ) )
  {
    ulSetError ( UL_WARNING, "loadSSG: Failed to read vertex array." ) ;
    return FALSE ;
  }

  return TRUE ;
}


int ssgVtxTable::save ( FILE *fd )
{
  _ssgWriteVec3  ( fd, bbox.getMin() ) ;
  _ssgWriteVec3  ( fd, bbox.getMax() ) ;
  _ssgWriteInt   ( fd, (int) gltype ) ;

  ssgLeaf::save(fd) ;

  if ( ! vertices  -> save ( fd ) ||
       ! normals   -> save ( fd ) ||
       ! texcoords -> save ( fd ) ||
       ! colours   -> save ( fd ) )
  {
    ulSetError ( UL_WARNING, "saveSSG: Failed to write vertex array" ) ;
    return FALSE ;
  }

  return TRUE ;
}



