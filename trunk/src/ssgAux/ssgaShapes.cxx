
#include "ssgAux.h"


void ssgaShape::copy_from ( ssgaShape *src, int clone_flags )
{
  ssgBranch::copy_from ( src, clone_flags ) ;
  if ( src -> isCorrupt () ) makeCorrupt () ;
  sgCopyVec4 ( colour, src->colour ) ;
  sgCopyVec3 ( center, src->center ) ;
  sgCopyVec3 ( size  , src->size   ) ;
  ntriangles    = src -> ntriangles ;
  kidState      = src -> getKidState      () ;
  kidPreDrawCB  = src -> getKidPreDrawCB  () ;
  kidPostDrawCB = src -> getKidPostDrawCB () ;
}


void ssgaCube    ::copy_from ( ssgaCube     *src, int clone_flags ) { ssgaShape::copy_from ( src, clone_flags ) ;} 
void ssgaSphere  ::copy_from ( ssgaSphere   *src, int clone_flags ) { ssgaShape::copy_from ( src, clone_flags ) ;}
void ssgaCylinder::copy_from ( ssgaCylinder *src, int clone_flags ) { ssgaShape::copy_from ( src, clone_flags ) ;}


ssgBase *ssgaShape   ::clone ( int clone_flags )
{
/*
  ssgaShape *b = new ssgaShape ;
  b -> copy_from ( this, clone_flags ) ;
  return b ;
*/
  return NULL ;
}


ssgBase *ssgaCube    ::clone ( int clone_flags )
{
  ssgaCube *b = new ssgaCube ;
  b -> copy_from ( this, clone_flags ) ;
  return b ;
}


ssgBase *ssgaSphere  ::clone ( int clone_flags )
{
  ssgaSphere *b = new ssgaSphere ;
  b -> copy_from ( this, clone_flags ) ;
  return b ;
}


ssgBase *ssgaCylinder::clone ( int clone_flags )
{
  ssgaCylinder *b = new ssgaCylinder ;
  b -> copy_from ( this, clone_flags ) ;
  return b ;
}


ssgaShape::ssgaShape (void)
{
  ntriangles = 50 ;
  init () ;
}

ssgaShape::ssgaShape ( int np )
{
  ntriangles = np ;
  init () ;
}

void ssgaShape::init ()
{
  type |= SSGA_TYPE_SHAPE ;
  corrupted = FALSE ;
  sgZeroVec3 ( center ) ;
  sgSetVec4 ( colour, 1.0f, 1.0f, 1.0f, 1.0f ) ;
  sgSetVec3 ( size, 1.0f, 1.0f, 1.0f ) ;
  kidState      = NULL ;
  kidPreDrawCB  = NULL ;
  kidPostDrawCB = NULL ;
} 

ssgaCube    ::ssgaCube     ( void ) : ssgaShape ()     { type |= SSGA_TYPE_CUBE     ; regenerate () ; } 
ssgaCube    ::ssgaCube     (int nt) : ssgaShape ( nt ) { type |= SSGA_TYPE_CUBE     ; regenerate () ; } 


ssgaSphere  ::ssgaSphere   ( void ) : ssgaShape ()
{
 type |= SSGA_TYPE_SPHERE   ;
 latlong_style = TRUE ;
 regenerate () ;
} 


ssgaSphere  ::ssgaSphere   (int nt) : ssgaShape ( nt )
{
 type |= SSGA_TYPE_SPHERE   ;
 latlong_style = TRUE ;
 regenerate () ;
} 


ssgaCylinder::ssgaCylinder ( void ) : ssgaShape ()
{
 type |= SSGA_TYPE_CYLINDER ;
 capped = TRUE ;
 regenerate () ;
}


ssgaCylinder::ssgaCylinder (int nt) : ssgaShape ( nt )
{
 type |= SSGA_TYPE_CYLINDER ;
 capped = TRUE ;
 regenerate () ;
}


ssgaShape   ::~ssgaShape    (void) {}
ssgaCube    ::~ssgaCube     (void) {}
ssgaSphere  ::~ssgaSphere   (void) {}
ssgaCylinder::~ssgaCylinder (void) {}

char *ssgaShape   ::getTypeName(void) { return "ssgaShape"    ; }
char *ssgaCube    ::getTypeName(void) { return "ssgaCube"     ; }
char *ssgaSphere  ::getTypeName(void) { return "ssgaSphere"   ; }
char *ssgaCylinder::getTypeName(void) { return "ssgaCylinder" ; }


void ssgaCube    ::regenerate ()
{
  removeAllKids () ;

  if ( ntriangles == 0 )
    return ;

  ssgVtxTable     *vt0 = new ssgVtxTable () ;         ssgVtxTable     *vt1 = new ssgVtxTable () ;

  ssgVertexArray   *v0 = new ssgVertexArray   ( 8 ) ; ssgVertexArray   *v1 = new ssgVertexArray   ( 8 ) ;
  ssgNormalArray   *n0 = new ssgNormalArray   ( 8 ) ; ssgNormalArray   *n1 = new ssgNormalArray   ( 8 ) ;
  ssgColourArray   *c0 = new ssgColourArray   ( 8 ) ; ssgColourArray   *c1 = new ssgColourArray   ( 8 ) ;
  ssgTexCoordArray *t0 = new ssgTexCoordArray ( 8 ) ; ssgTexCoordArray *t1 = new ssgTexCoordArray ( 8 ) ;

  vt0 -> setPrimitiveType ( GL_TRIANGLE_STRIP ) ;
  vt1 -> setPrimitiveType ( GL_TRIANGLE_STRIP ) ;

  addKid ( vt0 ) ; addKid ( vt1 ) ;

  vt0 -> setState    ( getKidState () ) ;
  vt1 -> setState    ( getKidState () ) ;
  vt0 -> setCallback ( SSG_CALLBACK_PREDRAW , getKidPreDrawCB  () ) ;
  vt1 -> setCallback ( SSG_CALLBACK_PREDRAW , getKidPreDrawCB  () ) ;
  vt0 -> setCallback ( SSG_CALLBACK_POSTDRAW, getKidPostDrawCB () ) ;
  vt1 -> setCallback ( SSG_CALLBACK_POSTDRAW, getKidPostDrawCB () ) ;

  sgVec3 v ; sgVec3 n ; sgVec2 t ;

  sgSetVec3 ( v, center[0]+size[0]/2.0f, center[1]-size[1]/2.0f, center[2]+size[2]/2.0f ) ; v0->add ( v ) ;
  sgSetVec3 ( n,          1            ,          0            ,          0             ) ; n0->add ( n ) ;
  c0->add ( colour ) ;
  sgSetVec2 ( t,          0                       ,                1                    ) ; t0->add ( t ) ;

  sgSetVec3 ( v, center[0]+size[0]/2.0f, center[1]-size[1]/2.0f, center[2]-size[2]/2.0f ) ; v0->add ( v ) ;
  sgSetVec3 ( n,          1            ,          0            ,          0             ) ; n0->add ( n ) ;
  c0->add ( colour ) ;
  sgSetVec2 ( t,          0                       ,                0                    ) ; t0->add ( t ) ;

  sgSetVec3 ( v, center[0]+size[0]/2.0f, center[1]+size[1]/2.0f, center[2]+size[2]/2.0f ) ; v0->add ( v ) ;
  sgSetVec3 ( n,          1            ,          0            ,          0             ) ; n0->add ( n ) ;
  c0->add ( colour ) ;
  sgSetVec2 ( t,          1                       ,                1                    ) ; t0->add ( t ) ;

  sgSetVec3 ( v, center[0]+size[0]/2.0f, center[1]+size[1]/2.0f, center[2]-size[2]/2.0f ) ; v0->add ( v ) ;
  sgSetVec3 ( n,          1            ,          0            ,          0             ) ; n0->add ( n ) ;
  c0->add ( colour ) ;
  sgSetVec2 ( t,          1                       ,                0                    ) ; t0->add ( t ) ;

  sgSetVec3 ( v, center[0]-size[0]/2.0f, center[1]+size[1]/2.0f, center[2]+size[2]/2.0f ) ; v0->add ( v ) ;
  sgSetVec3 ( n,          0            ,          1            ,          0             ) ; n0->add ( n ) ;
  c0->add ( colour ) ;
  sgSetVec2 ( t,          2                       ,                1                    ) ; t0->add ( t ) ;

  sgSetVec3 ( v, center[0]-size[0]/2.0f, center[1]+size[1]/2.0f, center[2]-size[2]/2.0f ) ; v0->add ( v ) ;
  sgSetVec3 ( n,          0            ,          1            ,          0             ) ; n0->add ( n ) ;
  c0->add ( colour ) ;
  sgSetVec2 ( t,          2                       ,                0                    ) ; t0->add ( t ) ;

  sgSetVec3 ( v, center[0]-size[0]/2.0f, center[1]-size[1]/2.0f, center[2]+size[2]/2.0f ) ; v0->add ( v ) ;
  sgSetVec3 ( n,         -1            ,          0            ,          0             ) ; n0->add ( n ) ;
  c0->add ( colour ) ;
  sgSetVec2 ( t,          3                       ,                1                    ) ; t0->add ( t ) ;

  sgSetVec3 ( v, center[0]-size[0]/2.0f, center[1]-size[1]/2.0f, center[2]-size[2]/2.0f ) ; v0->add ( v ) ;
  sgSetVec3 ( n,         -1            ,          0            ,          0             ) ; n0->add ( n ) ;
  c0->add ( colour ) ;
  sgSetVec2 ( t,          3                       ,                0                    ) ; t0->add ( t ) ;

  vt0 -> setVertices  ( v0 ) ;
  vt0 -> setNormals   ( n0 ) ;
  vt0 -> setColours   ( c0 ) ;
  vt0 -> setTexCoords ( t0 ) ;

  vt0 -> recalcBSphere () ;

  sgSetVec3 ( v, center[0]-size[0]/2.0f, center[1]+size[1]/2.0f, center[2]-size[2]/2.0f ) ; v1->add ( v ) ;
  sgSetVec3 ( n,          0            ,          0            ,         -1             ) ; n1->add ( n ) ;
  c1->add ( colour ) ;
  sgSetVec2 ( t,          0                       ,                0                    ) ; t1->add ( t ) ;

  sgSetVec3 ( v, center[0]+size[0]/2.0f, center[1]+size[1]/2.0f, center[2]-size[2]/2.0f ) ; v1->add ( v ) ;
  sgSetVec3 ( n,          0            ,          0            ,         -1             ) ; n1->add ( n ) ;
  c1->add ( colour ) ;
  sgSetVec2 ( t,          1                       ,                0                    ) ; t1->add ( t ) ;

  sgSetVec3 ( v, center[0]-size[0]/2.0f, center[1]-size[1]/2.0f, center[2]-size[2]/2.0f ) ; v1->add ( v ) ;
  sgSetVec3 ( n,          0            ,          0            ,         -1             ) ; n1->add ( n ) ;
  c1->add ( colour ) ;
  sgSetVec2 ( t,          0                       ,                1                    ) ; t1->add ( t ) ;

  sgSetVec3 ( v, center[0]+size[0]/2.0f, center[1]-size[1]/2.0f, center[2]-size[2]/2.0f ) ; v1->add ( v ) ;
  sgSetVec3 ( n,          0            ,          0            ,         -1             ) ; n1->add ( n ) ;
  c1->add ( colour ) ;
  sgSetVec2 ( t,          1                       ,                1                    ) ; t1->add ( t ) ;

  sgSetVec3 ( v, center[0]-size[0]/2.0f, center[1]-size[1]/2.0f, center[2]+size[2]/2.0f ) ; v1->add ( v ) ;
  sgSetVec3 ( n,          0            ,         -1            ,          0             ) ; n1->add ( n ) ;
  c1->add ( colour ) ;
  sgSetVec2 ( t,          0                       ,                2                    ) ; t1->add ( t ) ;

  sgSetVec3 ( v, center[0]+size[0]/2.0f, center[1]-size[1]/2.0f, center[2]+size[2]/2.0f ) ; v1->add ( v ) ;
  sgSetVec3 ( n,          0            ,         -1            ,          0             ) ; n1->add ( n ) ;
  c1->add ( colour ) ;
  sgSetVec2 ( t,          1                       ,                2                    ) ; t1->add ( t ) ;

  sgSetVec3 ( v, center[0]-size[0]/2.0f, center[1]+size[1]/2.0f, center[2]+size[2]/2.0f ) ; v1->add ( v ) ;
  sgSetVec3 ( n,          0            ,          0            ,          1             ) ; n1->add ( n ) ;
  c1->add ( colour ) ;
  sgSetVec2 ( t,          0                       ,                3                    ) ; t1->add ( t ) ;

  sgSetVec3 ( v, center[0]+size[0]/2.0f, center[1]+size[1]/2.0f, center[2]+size[2]/2.0f ) ; v1->add ( v ) ;
  sgSetVec3 ( n,          0            ,          0            ,          1             ) ; n1->add ( n ) ;
  c1->add ( colour ) ;
  sgSetVec2 ( t,          1                       ,                3                    ) ; t1->add ( t ) ;

  vt1 -> setVertices  ( v1 ) ;
  vt1 -> setNormals   ( n1 ) ;
  vt1 -> setColours   ( c1 ) ;
  vt1 -> setTexCoords ( t1 ) ;

  vt1 -> recalcBSphere () ;

  recalcBSphere () ;
}



/*
  This code is 'inspired' by the function 'sphere' written by
  David Blythe for GLUT 3.5 and uses his values for the coordinates
  of the initial Icosahedron.
*/
 
#define CZ 0.89442719099991f   /* 2/sqrt(5) */
#define SZ 0.44721359549995f   /* 1/sqrt(5) */
#define C1 0.951056516f        /* cos(18) */
#define S1 0.309016994f        /* sin(18) */
#define C2 0.587785252f        /* cos(54) */
#define S2 0.809016994f        /* sin(54) */
#define X1 (C1*CZ)
#define Y1 (S1*CZ)
#define X2 (C2*CZ)
#define Y2 (S2*CZ)
 
#define P0 {   0,   0,   1 }
#define P1 { -X2, -Y2,  SZ }
#define P2 {  X2, -Y2,  SZ }
#define P3 {  X1,  Y1,  SZ }
#define P4 {   0,  CZ,  SZ }
#define P5 { -X1,  Y1,  SZ }
#define P6 { -X1, -Y1, -SZ }
#define P7 {   0, -CZ, -SZ }
#define P8 {  X1, -Y1, -SZ }
#define P9 {  X2,  Y2, -SZ }
#define PA { -X2,  Y2, -SZ }
#define PB {   0,   0,  -1 }

struct Triangle
{
  sgVec3 v0, v1, v2 ;
} ;
 
 
static Triangle icosahedron [ 20 ] =
{
  { P0, P1, P2 }, { P0, P5, P1 }, { P0, P4, P5 }, { P0, P3, P4 }, { P0, P2, P3 },
  { P1, P6, P7 }, { P6, P1, P5 }, { P5, PA, P6 }, { PA, P5, P4 }, { P4, P9, PA },
  { P9, P4, P3 }, { P3, P8, P9 }, { P8, P3, P2 }, { P2, P7, P8 }, { P7, P2, P1 },
  { P9, P8, PB }, { PA, P9, PB }, { P6, PA, PB }, { P7, P6, PB }, { P8, P7, PB }
} ;



void ssgaSphere::regenerateTessellatedIcosahedron ()
{
  int tris_per_strip = 1 ;
  int nstrips = 1 ;
  int nt = 20 ;
 
  while ( nt < ntriangles )
  {
    nstrips++ ;
    tris_per_strip += 2 ;
    nt += tris_per_strip * 20 ;
  }
 
  /* 20 sides of an Icosahedron */
 
  for ( int s = 0 ; s < 20 ; s++ )
  {
    Triangle *tri = & icosahedron [ s ] ;
 
    for ( int i = 0 ; i < nstrips ; i++ )
    {
      /*
        Create a tri-strip for each row
        The number of triangles in each strip is two greater
        than the last one.
      */
 
      sgVec3 v0, v1, v2, v3, va, vb ;
 
      /*
                          t->v[0]
                             /\
                            /  \
                           /____\v1
                          /\    /\
                         /  \  /  \
                        /____\/vb__\v0
                       /\    /\    /\
                      /  \  /  \  /  \
                     /____\/____\/va__\
                    /\    /\    /\    /\
                   /  \  /  \  /  \  /  \
          t->v[2] /____\/____\/____\/____\ t->v[1]
                       v3     v2
      */
 
      /*
        This should be a spherical interpolation - but that's slow and
        nobody can tell the difference anyway
      */

      ssgVtxTable      *vt = new ssgVtxTable ;
      ssgVertexArray   *vv = new ssgVertexArray   ( i + 3 ) ;
      ssgNormalArray   *nn = new ssgNormalArray   ( i + 3 ) ;
      ssgColourArray   *cc = new ssgColourArray   ( i + 3 ) ;
      ssgTexCoordArray *tt = new ssgTexCoordArray ( i + 3 ) ;

      addKid ( vt ) ;

      vt -> setState    ( getKidState () ) ;
      vt -> setCallback ( SSG_CALLBACK_PREDRAW , getKidPreDrawCB  () ) ;
      vt -> setCallback ( SSG_CALLBACK_POSTDRAW, getKidPostDrawCB () ) ;

      sgVec3 v ; sgVec3 n ; sgVec2 t ;

      vt -> setPrimitiveType ( GL_TRIANGLE_STRIP ) ;

      sgLerpVec3 ( v0, tri->v1, tri->v0, (float)(i+1) / (float) nstrips ) ;
      sgLerpVec3 ( v1, tri->v1, tri->v0, (float)  i   / (float) nstrips ) ;
      sgLerpVec3 ( v2, tri->v1, tri->v2, (float)(i+1) / (float) nstrips ) ;
      sgLerpVec3 ( v3, tri->v1, tri->v2, (float)  i   / (float) nstrips ) ;

      sgNormalizeVec3 ( v0 ) ;
      sgNormalizeVec3 ( v1 ) ;
      sgNormalizeVec3 ( v2 ) ;
      sgNormalizeVec3 ( v3 ) ;

      sgSetVec3 ( v, center[0]+size[0]*v0[0],
                     center[1]+size[1]*v0[1],
                     center[2]+size[2]*v0[2] ) ;
      sgSetVec3 ( n, size[0]*v0[0],
                     size[1]*v0[1],
                     size[2]*v0[2] ) ;
      sgNormalizeVec3 ( n ) ;
      sgSetVec2 ( t, atan2(n[0],n[1])/(SG_PI*2.0f)+0.5f, 0.5f+v0[2]/2.0f ) ;
      vv->add(v) ; nn->add(n) ; cc->add(colour) ; tt->add(t) ;
    
      sgSetVec3 ( v, center[0]+size[0]*v1[0],
                     center[1]+size[1]*v1[1],
                     center[2]+size[2]*v1[2] ) ;
      sgSetVec3 ( n, size[0]*v1[0],
                     size[1]*v1[1],
                     size[2]*v1[2] ) ;
      sgNormalizeVec3 ( n ) ;
      sgSetVec2 ( t, atan2(n[0],n[1])/(SG_PI*2.0f)+0.5f, 0.5f+v1[2]/2.0f ) ;
      vv->add(v) ; nn->add(n) ; cc->add(colour) ; tt->add(t) ;

      for ( int j = 0 ; j < i ; j++ )
      {
        sgLerpVec3 ( va, v0, v2, (float)(j+1) / (float) (i+1) ) ;
        sgLerpVec3 ( vb, v1, v3, (float)(j+1) / (float)   i   ) ;
        sgNormalizeVec3 ( va ) ;
        sgNormalizeVec3 ( vb ) ;
 
	sgSetVec3 ( v, center[0]+size[0]*va[0],
		       center[1]+size[1]*va[1],
		       center[2]+size[2]*va[2] ) ;
	sgSetVec3 ( n, size[0]*va[0],
		       size[1]*va[1],
		       size[2]*va[2] ) ;
	sgNormalizeVec3 ( n ) ;
        sgSetVec2 ( t, atan2(n[0],n[1])/(SG_PI*2.0f)+0.5f, 0.5f+va[2]/2.0f ) ;
	vv->add(v) ; nn->add(n) ; cc->add(colour) ; tt->add(t) ;

	sgSetVec3 ( v, center[0]+size[0]*vb[0],
		       center[1]+size[1]*vb[1],
		       center[2]+size[2]*vb[2] ) ;
	sgSetVec3 ( n, size[0]*vb[0],
		       size[1]*vb[1],
		       size[2]*vb[2] ) ;
	sgNormalizeVec3 ( n ) ;
        sgSetVec2 ( t, atan2(n[0],n[1])/(SG_PI*2.0f)+0.5f, 0.5f+vb[2]/2.0f ) ;
	vv->add(v) ; nn->add(n) ; cc->add(colour) ; tt->add(t) ;
      }
 
      sgSetVec3 ( v, center[0]+size[0]*v2[0],
                     center[1]+size[1]*v2[1],
                     center[2]+size[2]*v2[2] ) ;
      sgSetVec3 ( n, size[0]*v2[0],
                     size[1]*v2[1],
                     size[2]*v2[2] ) ;
      sgNormalizeVec3 ( n ) ;
      sgSetVec2 ( t, atan2(n[0],n[1])/(SG_PI*2.0f)+0.5f, 0.5f+v2[2]/2.0f ) ;
      vv->add(v) ; nn->add(n) ; cc->add(colour) ; tt->add(t) ;

      vt -> setVertices  ( vv ) ;
      vt -> setNormals   ( nn ) ;
      vt -> setColours   ( cc ) ;
      vt -> setTexCoords ( tt ) ;

      vt -> recalcBSphere () ;
    }
  }

  recalcBSphere () ;
}


void ssgaSphere::regenerateLatLong ()
{
  int stacks = (int) sqrt ( (double) ntriangles / 2.0f ) ;
  int slices = ntriangles / stacks ;

  if ( stacks < 2 ) stacks = 2 ;
  if ( slices < 3 ) slices = 3 ;

  for ( int i = 0 ; i < stacks ; i++ )
  {
    ssgVtxTable      *vt = new ssgVtxTable ;
    ssgVertexArray   *vv = new ssgVertexArray   ( (slices+1)*2 ) ;
    ssgNormalArray   *nn = new ssgNormalArray   ( (slices+1)*2 ) ;
    ssgColourArray   *cc = new ssgColourArray   ( (slices+1)*2 ) ;
    ssgTexCoordArray *tt = new ssgTexCoordArray ( (slices+1)*2 ) ;

    addKid ( vt ) ;

    vt -> setState    ( getKidState () ) ;
    vt -> setCallback ( SSG_CALLBACK_PREDRAW , getKidPreDrawCB  () ) ;
    vt -> setCallback ( SSG_CALLBACK_POSTDRAW, getKidPostDrawCB () ) ;

    sgVec3 v ; sgVec3 n ; sgVec2 t ;

    if ( i == stacks-1 )   /* North Pole */
    {
      vt -> setPrimitiveType ( GL_TRIANGLE_FAN ) ;

      sgSetVec3 ( v, center[0], center[1], center[2]+size[2]/2.0f ) ;
      sgSetVec3 ( n, 0, 0, 1 ) ;
      sgSetVec2 ( t, 0.5f, 1 ) ;
      vv->add(v) ; nn->add(n) ; cc->add(colour) ; tt->add(t) ;

      for ( int j = slices ; j >= 0 ; j-- )
      {
        float a = (j==0 || j==slices) ? 0.0f : (float) j * SG_PI * 2.0f / (float) slices ;
        float b = (float) i * SG_PI / (float) stacks ;

	sgSetVec3 ( v, center[0] + size[0]*sin(a)*sin(b)/2.0f,
                       center[1] + size[1]*cos(a)*sin(b)/2.0f,
                       center[2] - size[2]*       cos(b)/2.0f ) ;
	sgSetVec3 ( n, sin(a)*sin(b)*size[0],
                       cos(a)*sin(b)*size[1],
                       -cos(b)*size[2] ) ;
        sgNormalizeVec3 ( n ) ;
	sgSetVec2 ( t, (float)j/(float)slices, (float) i /(float)stacks ) ;
	vv->add(v) ; nn->add(n) ; cc->add(colour) ; tt->add(t) ;
        
      }
    }
    else
    if ( i == 0 )   /* South Pole */
    {
      vt -> setPrimitiveType ( GL_TRIANGLE_FAN ) ;

      sgSetVec3 ( v, center[0], center[1], center[2]-size[2]/2.0f ) ;
      sgSetVec3 ( n, 0, 0, -1 ) ;
      sgSetVec2 ( t, 0.5, 0 ) ;
      vv->add(v) ; nn->add(n) ; cc->add(colour) ; tt->add(t) ;

      for ( int j = 0 ; j < slices+1 ; j++ )
      {
        float a = (j==0 || j==slices) ? 0.0f : (float) j * SG_PI * 2.0f / (float) slices ;
        float b = (float)(i+1) * SG_PI / (float) stacks ;

	sgSetVec3 ( v, center[0] + size[0]*sin(a)*sin(b)/2.0f,
                       center[1] + size[1]*cos(a)*sin(b)/2.0f,
                       center[2] - size[2]*       cos(b)/2.0f ) ;
	sgSetVec3 ( n, sin(a)*sin(b)*size[0],
                       cos(a)*sin(b)*size[1],
                       -cos(b)*size[2] ) ;
        sgNormalizeVec3 ( n ) ;
	sgSetVec2 ( t, (float)j/(float)slices,
                       (float)(i+1)/(float)stacks ) ;
	vv->add(v) ; nn->add(n) ; cc->add(colour) ; tt->add(t) ;
      }
    }
    else
    {
      vt -> setPrimitiveType ( GL_TRIANGLE_STRIP ) ;

      for ( int j = 0 ; j < slices+1 ; j++ )
      {
        float a = (j==0 || j==slices) ? 0.0f : (float) j * SG_PI * 2.0f / (float) slices ;
        float b0 = (float) i * SG_PI / (float) stacks ;
        float b1 = (float)(i+1) * SG_PI / (float) stacks ;

	sgSetVec3 ( v, center[0] + size[0]*sin(a)*sin(b0)/2.0f,
                       center[1] + size[1]*cos(a)*sin(b0)/2.0f,
                       center[2] - size[2]*       cos(b0)/2.0f ) ;
	sgSetVec3 ( n, sin(a)*sin(b0)*size[0],
                       cos(a)*sin(b0)*size[1],
                       -cos(b0)*size[2] ) ;
        sgNormalizeVec3 ( n ) ;
	sgSetVec2 ( t, (float)j/(float)slices,
                       (float)i/(float)stacks ) ;
	vv->add(v) ; nn->add(n) ; cc->add(colour) ; tt->add(t) ;

	sgSetVec3 ( v, center[0] + size[0]*sin(a)*sin(b1)/2.0f,
                       center[1] + size[1]*cos(a)*sin(b1)/2.0f,
                       center[2] - size[2]*       cos(b1)/2.0f ) ;
	sgSetVec3 ( n, sin(a)*sin(b1)*size[0],
                       cos(a)*sin(b1)*size[1],
                       -cos(b1)*size[2] ) ;
        sgNormalizeVec3 ( n ) ;
	sgSetVec2 ( t, (float)j/(float)slices,
                       (float)(i+1)/(float)stacks ) ;
	vv->add(v) ; nn->add(n) ; cc->add(colour) ; tt->add(t) ;
        
      }
    }

    vt -> setVertices  ( vv ) ;
    vt -> setNormals   ( nn ) ;
    vt -> setColours   ( cc ) ;
    vt -> setTexCoords ( tt ) ;

    vt -> recalcBSphere () ;
  }

  recalcBSphere () ;
}


void ssgaSphere::regenerate ()
{
  removeAllKids () ;

  if ( ntriangles == 0 )
    return ;

  if ( latlong_style )
    regenerateLatLong () ;
  else
    regenerateTessellatedIcosahedron () ;
}



void ssgaCylinder::regenerate ()
{
  removeAllKids () ;

  if ( ntriangles == 0 )
    return ;

  int slices = 1 + ntriangles / 4 ;

  if ( slices < 3 ) slices = 3 ;

  ssgVtxTable      *vt = new ssgVtxTable ;
  ssgVertexArray   *vv = new ssgVertexArray   ( (slices+1)*2 ) ;
  ssgNormalArray   *nn = new ssgNormalArray   ( (slices+1)*2 ) ;
  ssgColourArray   *cc = new ssgColourArray   ( (slices+1)*2 ) ;
  ssgTexCoordArray *tt = new ssgTexCoordArray ( (slices+1)*2 ) ;

  addKid ( vt ) ;

  vt -> setState    ( getKidState () ) ;
  vt -> setCallback ( SSG_CALLBACK_PREDRAW , getKidPreDrawCB  () ) ;
  vt -> setCallback ( SSG_CALLBACK_POSTDRAW, getKidPostDrawCB () ) ;

  sgVec3 v ; sgVec3 n ; sgVec2 t ;

  vt -> setPrimitiveType ( GL_TRIANGLE_STRIP ) ;

  for ( int j = 0 ; j < slices+1 ; j++ )
  {
    float a = (j==0 || j==slices) ? 0.0f : (float) j * SG_PI * 2.0f / (float) slices ;

    sgSetVec3 ( v, center[0] + size[0]*sin(a)/2.0f,
		   center[1] + size[1]*cos(a)/2.0f,
		   center[2] - size[2] / 2.0f ) ;
    sgSetVec3 ( n, -sin(a) * size[0], -cos(a) * size[1], 0 ) ;
    sgNormalizeVec3 ( n ) ;
    sgSetVec2 ( t, (float)j/(float)slices, 0 ) ;
    vv->add(v) ; nn->add(n) ; cc->add(colour) ; tt->add(t) ;

    sgSetVec3 ( v, center[0] + size[0]*sin(a)/2.0f,
		   center[1] + size[1]*cos(a)/2.0f,
		   center[2] + size[2] / 2.0f ) ;
    sgSetVec3 ( n, -sin(a) * size[0], -cos(a) * size[1], 0 ) ;
    sgNormalizeVec3 ( n ) ;
    sgSetVec2 ( t, (float)j/(float)slices, 1 ) ;
    vv->add(v) ; nn->add(n) ; cc->add(colour) ; tt->add(t) ;
    
  }

  vt -> setVertices  ( vv ) ;
  vt -> setNormals   ( nn ) ;
  vt -> setColours   ( cc ) ;
  vt -> setTexCoords ( tt ) ;
  vt -> recalcBSphere () ;

  if ( capped )
  {
    ssgVtxTable      *vt0 = new ssgVtxTable ;
    ssgVtxTable      *vt1 = new ssgVtxTable ;

    ssgVertexArray   *vv0 = new ssgVertexArray   ( slices ) ;
    ssgNormalArray   *nn0 = new ssgNormalArray   ( slices ) ;
    ssgColourArray   *cc0 = new ssgColourArray   ( slices ) ;
    ssgTexCoordArray *tt0 = new ssgTexCoordArray ( slices ) ;

    ssgVertexArray   *vv1 = new ssgVertexArray   ( slices ) ;
    ssgNormalArray   *nn1 = new ssgNormalArray   ( slices ) ;
    ssgColourArray   *cc1 = new ssgColourArray   ( slices ) ;
    ssgTexCoordArray *tt1 = new ssgTexCoordArray ( slices ) ;

    addKid ( vt0 ) ;
    addKid ( vt1 ) ;

    vt0 -> setState    ( getKidState () ) ;
    vt0 -> setCallback ( SSG_CALLBACK_PREDRAW , getKidPreDrawCB  () ) ;
    vt0 -> setCallback ( SSG_CALLBACK_POSTDRAW, getKidPostDrawCB () ) ;

    vt1 -> setState    ( getKidState () ) ;
    vt1 -> setCallback ( SSG_CALLBACK_PREDRAW , getKidPreDrawCB  () ) ;
    vt1 -> setCallback ( SSG_CALLBACK_POSTDRAW, getKidPostDrawCB () ) ;

    sgVec3 v ; sgVec3 n ; sgVec2 t ;

    vt0 -> setPrimitiveType ( GL_TRIANGLE_FAN ) ;
    vt1 -> setPrimitiveType ( GL_TRIANGLE_FAN ) ;

    for ( int j = 0 ; j < slices ; j++ )
    {
      float a0 = (float)(1+slices-j) * SG_PI * 2.0f / (float) slices ;
      float a1 = (float)      j      * SG_PI * 2.0f / (float) slices ;

      /* Top */

      sgSetVec3 ( v, center[0] + size[0]*sin(a0)/2.0f,
		     center[1] + size[1]*cos(a0)/2.0f,
		     center[2] + size[2] / 2.0f ) ;
      sgSetVec3 ( n, 0, 0, 1 ) ;
      sgSetVec2 ( t, 0.5 + sin(a0)/2.0f, 0.5 + cos(a0)/2.0f ) ;
      vv0->add(v) ; nn0->add(n) ; cc0->add(colour) ; tt0->add(t) ;

      /* Bottom */

      sgSetVec3 ( v, center[0] + size[0]*sin(a1)/2.0f,
		     center[1] + size[1]*cos(a1)/2.0f,
		     center[2] - size[2] / 2.0f ) ;
      sgSetVec3 ( n, 0, 0, -1 ) ;
      sgSetVec2 ( t, 0.5 + sin(a1)/2.0f, 0.5 + cos(a1)/2.0f ) ;
      vv1->add(v) ; nn1->add(n) ; cc1->add(colour) ; tt1->add(t) ;
    }

    vt0 -> setVertices  ( vv0 ) ; vt1 -> setVertices  ( vv1 ) ;
    vt0 -> setNormals   ( nn0 ) ; vt1 -> setNormals   ( nn1 ) ;
    vt0 -> setColours   ( cc0 ) ; vt1 -> setColours   ( cc1 ) ;
    vt0 -> setTexCoords ( tt0 ) ; vt1 -> setTexCoords ( tt1 ) ;
    vt0 -> recalcBSphere  ()    ; vt1 -> recalcBSphere  () ;
  }

  recalcBSphere () ;
}



// XXX really need these (and ssgLocal.h is not accessible):
extern int _ssgLoadObject ( FILE *, ssgBase **, int ) ;
extern int _ssgSaveObject ( FILE *, ssgBase * ) ;


#define load_field(fp, name) (fread(&(name), 1, sizeof(name), fp) == sizeof(name))
#define save_field(fp, name) (fwrite(&(name), 1, sizeof(name), fp) == sizeof(name))


int ssgaShape::load ( FILE *fp )
{
   return ( load_field ( fp, corrupted ) && 
	    load_field ( fp, colour ) &&
	    load_field ( fp, center ) &&
	    load_field ( fp, size ) &&
	    load_field ( fp, ntriangles ) &&
	    _ssgLoadObject ( fp, (ssgBase **) &kidState, ssgTypeState () ) &&
	    ssgBranch::load ( fp ) ) ;
}

int ssgaShape::save ( FILE *fp )
{
   return ( save_field ( fp, corrupted ) &&
	    save_field ( fp, colour ) &&
	    save_field ( fp, center ) &&
	    save_field ( fp, size ) &&
	    save_field ( fp, ntriangles ) &&
	    _ssgSaveObject ( fp, kidState ) &&
	    ssgBranch::save ( fp ) ) ;
}


int ssgaSphere::load ( FILE *fp )
{
   return ( load_field ( fp, latlong_style ) &&
	    ssgaShape::load ( fp ) ) ;
}

int ssgaSphere::save ( FILE *fp )
{
   return ( save_field ( fp, latlong_style ) &&
	    ssgaShape::save ( fp ) ) ;
}


int ssgaCylinder::load ( FILE *fp )
{
   return ( load_field ( fp, capped ) &&
	    ssgaShape::load ( fp ) ) ;
}

int ssgaCylinder::save ( FILE *fp )
{
   return ( save_field ( fp, capped ) &&
	    ssgaShape::save ( fp ) ) ;
}
