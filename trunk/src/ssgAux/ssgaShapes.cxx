
#include "ssgAux.h"


void ssgaShape::copy_from ( ssgaShape *src, int clone_flags )
{
  ssgBranch::copy_from ( src, clone_flags ) ;
  if ( src -> isCorrupt () ) makeCorrupt () ;
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


ssgBase *ssgaShape   ::clone ( int clone_flags = 0 )
{
/*
  ssgaShape *b = new ssgaShape ;
  b -> copy_from ( this, clone_flags ) ;
  return b ;
*/
  return NULL ;
}


ssgBase *ssgaCube    ::clone ( int clone_flags = 0 )
{
  ssgaCube *b = new ssgaCube ;
  b -> copy_from ( this, clone_flags ) ;
  return b ;
}


ssgBase *ssgaSphere  ::clone ( int clone_flags = 0 )
{
  ssgaSphere *b = new ssgaSphere ;
  b -> copy_from ( this, clone_flags ) ;
  return b ;
}


ssgBase *ssgaCylinder::clone ( int clone_flags = 0 )
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
  sgSetVec3 ( size, 1.0f, 1.0f, 1.0f ) ;
  kidState      = NULL ;
  kidPreDrawCB  = NULL ;
  kidPostDrawCB = NULL ;
} 

ssgaCube    ::ssgaCube     ( void ) : ssgaShape ()     { type |= SSGA_TYPE_CUBE     ; regenerate () ; } 
ssgaCube    ::ssgaCube     (int nt) : ssgaShape ( nt ) { type |= SSGA_TYPE_CUBE     ; regenerate () ; } 

ssgaSphere  ::ssgaSphere   ( void ) : ssgaShape ()     { type |= SSGA_TYPE_SPHERE   ; regenerate () ; } 
ssgaSphere  ::ssgaSphere   (int nt) : ssgaShape ( nt ) { type |= SSGA_TYPE_SPHERE   ; regenerate () ; } 

ssgaCylinder::ssgaCylinder ( void ) : ssgaShape ()     { type |= SSGA_TYPE_CYLINDER ; regenerate () ; }
ssgaCylinder::ssgaCylinder (int nt) : ssgaShape ( nt ) { type |= SSGA_TYPE_CYLINDER ; regenerate () ; }

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

  sgVec3 v ; sgVec3 n ; sgVec2 t ; sgVec4 c ;

  sgSetVec4 ( c, 1, 1, 1, 1 ) ;

  sgSetVec3 ( v, center[0]+size[0]/2.0f, center[1]-size[1]/2.0f, center[2]+size[2]/2.0f ) ; v0->add ( v ) ;
  sgSetVec3 ( n,          1            ,          0            ,          0             ) ; n0->add ( n ) ;
  c0->add ( c ) ;
  sgSetVec2 ( t,          0                       ,                1                    ) ; t0->add ( t ) ;

  sgSetVec3 ( v, center[0]+size[0]/2.0f, center[1]-size[1]/2.0f, center[2]-size[2]/2.0f ) ; v0->add ( v ) ;
  sgSetVec3 ( n,          1            ,          0            ,          0             ) ; n0->add ( n ) ;
  c0->add ( c ) ;
  sgSetVec2 ( t,          0                       ,                0                    ) ; t0->add ( t ) ;

  sgSetVec3 ( v, center[0]+size[0]/2.0f, center[1]+size[1]/2.0f, center[2]+size[2]/2.0f ) ; v0->add ( v ) ;
  sgSetVec3 ( n,          1            ,          0            ,          0             ) ; n0->add ( n ) ;
  c0->add ( c ) ;
  sgSetVec2 ( t,          1                       ,                1                    ) ; t0->add ( t ) ;

  sgSetVec3 ( v, center[0]+size[0]/2.0f, center[1]+size[1]/2.0f, center[2]-size[2]/2.0f ) ; v0->add ( v ) ;
  sgSetVec3 ( n,          1            ,          0            ,          0             ) ; n0->add ( n ) ;
  c0->add ( c ) ;
  sgSetVec2 ( t,          1                       ,                0                    ) ; t0->add ( t ) ;

  sgSetVec3 ( v, center[0]-size[0]/2.0f, center[1]+size[1]/2.0f, center[2]+size[2]/2.0f ) ; v0->add ( v ) ;
  sgSetVec3 ( n,          0            ,          1            ,          0             ) ; n0->add ( n ) ;
  c0->add ( c ) ;
  sgSetVec2 ( t,          2                       ,                1                    ) ; t0->add ( t ) ;

  sgSetVec3 ( v, center[0]-size[0]/2.0f, center[1]+size[1]/2.0f, center[2]-size[2]/2.0f ) ; v0->add ( v ) ;
  sgSetVec3 ( n,          0            ,          1            ,          0             ) ; n0->add ( n ) ;
  c0->add ( c ) ;
  sgSetVec2 ( t,          2                       ,                0                    ) ; t0->add ( t ) ;

  sgSetVec3 ( v, center[0]-size[0]/2.0f, center[1]-size[1]/2.0f, center[2]+size[2]/2.0f ) ; v0->add ( v ) ;
  sgSetVec3 ( n,         -1            ,          0            ,          0             ) ; n0->add ( n ) ;
  c0->add ( c ) ;
  sgSetVec2 ( t,          3                       ,                1                    ) ; t0->add ( t ) ;

  sgSetVec3 ( v, center[0]-size[0]/2.0f, center[1]-size[1]/2.0f, center[2]-size[2]/2.0f ) ; v0->add ( v ) ;
  sgSetVec3 ( n,         -1            ,          0            ,          0             ) ; n0->add ( n ) ;
  c0->add ( c ) ;
  sgSetVec2 ( t,          3                       ,                0                    ) ; t0->add ( t ) ;

  vt0 -> setVertices  ( v0 ) ;
  vt0 -> setNormals   ( n0 ) ;
  vt0 -> setColours   ( c0 ) ;
  vt0 -> setTexCoords ( t0 ) ;

  vt0 -> recalcBSphere () ;

  sgSetVec3 ( v, center[0]-size[0]/2.0f, center[1]+size[1]/2.0f, center[2]-size[2]/2.0f ) ; v1->add ( v ) ;
  sgSetVec3 ( n,          0            ,          0            ,         -1             ) ; n1->add ( n ) ;
  c1->add ( c ) ;
  sgSetVec2 ( t,          0                       ,                0                    ) ; t1->add ( t ) ;

  sgSetVec3 ( v, center[0]+size[0]/2.0f, center[1]+size[1]/2.0f, center[2]-size[2]/2.0f ) ; v1->add ( v ) ;
  sgSetVec3 ( n,          0            ,          0            ,         -1             ) ; n1->add ( n ) ;
  c1->add ( c ) ;
  sgSetVec2 ( t,          1                       ,                0                    ) ; t1->add ( t ) ;

  sgSetVec3 ( v, center[0]-size[0]/2.0f, center[1]-size[1]/2.0f, center[2]-size[2]/2.0f ) ; v1->add ( v ) ;
  sgSetVec3 ( n,          0            ,          0            ,         -1             ) ; n1->add ( n ) ;
  c1->add ( c ) ;
  sgSetVec2 ( t,          0                       ,                1                    ) ; t1->add ( t ) ;

  sgSetVec3 ( v, center[0]+size[0]/2.0f, center[1]-size[1]/2.0f, center[2]-size[2]/2.0f ) ; v1->add ( v ) ;
  sgSetVec3 ( n,          0            ,          0            ,         -1             ) ; n1->add ( n ) ;
  c1->add ( c ) ;
  sgSetVec2 ( t,          1                       ,                1                    ) ; t1->add ( t ) ;

  sgSetVec3 ( v, center[0]-size[0]/2.0f, center[1]-size[1]/2.0f, center[2]+size[2]/2.0f ) ; v1->add ( v ) ;
  sgSetVec3 ( n,          0            ,         -1            ,          0             ) ; n1->add ( n ) ;
  c1->add ( c ) ;
  sgSetVec2 ( t,          0                       ,                2                    ) ; t1->add ( t ) ;

  sgSetVec3 ( v, center[0]+size[0]/2.0f, center[1]-size[1]/2.0f, center[2]+size[2]/2.0f ) ; v1->add ( v ) ;
  sgSetVec3 ( n,          0            ,         -1            ,          0             ) ; n1->add ( n ) ;
  c1->add ( c ) ;
  sgSetVec2 ( t,          1                       ,                2                    ) ; t1->add ( t ) ;

  sgSetVec3 ( v, center[0]-size[0]/2.0f, center[1]+size[1]/2.0f, center[2]+size[2]/2.0f ) ; v1->add ( v ) ;
  sgSetVec3 ( n,          0            ,          0            ,          1             ) ; n1->add ( n ) ;
  c1->add ( c ) ;
  sgSetVec2 ( t,          0                       ,                3                    ) ; t1->add ( t ) ;

  sgSetVec3 ( v, center[0]+size[0]/2.0f, center[1]+size[1]/2.0f, center[2]+size[2]/2.0f ) ; v1->add ( v ) ;
  sgSetVec3 ( n,          0            ,          0            ,          1             ) ; n1->add ( n ) ;
  c1->add ( c ) ;
  sgSetVec2 ( t,          1                       ,                3                    ) ; t1->add ( t ) ;

  vt1 -> setVertices  ( v1 ) ;
  vt1 -> setNormals   ( n1 ) ;
  vt1 -> setColours   ( c1 ) ;
  vt1 -> setTexCoords ( t1 ) ;

  vt1 -> recalcBSphere () ;

  recalcBSphere () ;
}




void ssgaSphere::regenerate ()
{
  int stacks = (int) sqrt ( (double) ntriangles / 2.0f ) ;
  int slices = ntriangles / stacks ;

  if ( stacks < 2 ) stacks = 2 ;
  if ( slices < 3 ) slices = 3 ;

  removeAllKids () ;

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

    sgVec3 v ; sgVec3 n ; sgVec2 t ; sgVec4 c ;

    sgSetVec4 ( c, 1, 1, 1, 1 ) ;

    if ( i == stacks-1 )   /* North Pole */
    {
      vt -> setPrimitiveType ( GL_TRIANGLE_FAN ) ;

      sgSetVec3 ( v, center[0], center[1], center[2]+size[2] ) ;
      sgSetVec3 ( n, 0, 0, 1 ) ;
      sgSetVec2 ( t, 0.5f, 1 ) ;
      vv->add(v) ; nn->add(n) ; cc->add(c) ; tt->add(t) ;

      for ( int j = slices ; j >= 0 ; j-- )
      {
        float a = (j==0 || j==slices) ? 0.0f : (float) j * SG_PI * 2.0f / (float) slices ;
        float b = (float) i * SG_PI / (float) stacks ;

	sgSetVec3 ( v, center[0] + size[0]*sin(a)*sin(b),
                       center[1] + size[1]*cos(a)*sin(b),
                       center[2] - size[2]*       cos(b) ) ;
	sgSetVec3 ( n, -sin(a)*sin(b), -cos(a)*sin(b), -cos(b) ) ;
	sgSetVec2 ( t, (float)j/(float)slices, (float) i /(float)stacks ) ;
	vv->add(v) ; nn->add(n) ; cc->add(c) ; tt->add(t) ;
        
      }
    }
    else
    if ( i == 0 )   /* South Pole */
    {
      vt -> setPrimitiveType ( GL_TRIANGLE_FAN ) ;

      sgSetVec3 ( v, center[0], center[1], center[2]-size[2] ) ;
      sgSetVec3 ( n, 0, 0, -1 ) ;
      sgSetVec2 ( t, 0.5, 0 ) ;
      vv->add(v) ; nn->add(n) ; cc->add(c) ; tt->add(t) ;

      for ( int j = 0 ; j < slices+1 ; j++ )
      {
        float a = (j==0 || j==slices) ? 0.0f : (float) j * SG_PI * 2.0f / (float) slices ;
        float b = (float)(i+1) * SG_PI / (float) stacks ;

	sgSetVec3 ( v, center[0] + size[0]*sin(a)*sin(b),
                       center[1] + size[1]*cos(a)*sin(b),
                       center[2] - size[2]*       cos(b) ) ;
	sgSetVec3 ( n, -sin(a)*sin(b), -cos(a)*sin(b), -cos(b) ) ;
	sgSetVec2 ( t, (float)j/(float)slices, (float)(i+1)/(float)stacks ) ;
	vv->add(v) ; nn->add(n) ; cc->add(c) ; tt->add(t) ;
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

	sgSetVec3 ( v, center[0] + size[0]*sin(a)*sin(b0),
                       center[1] + size[1]*cos(a)*sin(b0),
                       center[2] - size[2]*       cos(b0) ) ;
	sgSetVec3 ( n, -sin(a)*sin(b0), -cos(a)*sin(b0), -cos(b0) ) ;
	sgSetVec2 ( t, (float)j/(float)slices, (float)i/(float)stacks ) ;
	vv->add(v) ; nn->add(n) ; cc->add(c) ; tt->add(t) ;

	sgSetVec3 ( v, center[0] + size[0]*sin(a)*sin(b1),
                       center[1] + size[1]*cos(a)*sin(b1),
                       center[2] - size[2]*       cos(b1) ) ;
	sgSetVec3 ( n, -sin(a)*sin(b1), -cos(a)*sin(b1), -cos(b1) ) ;
	sgSetVec2 ( t, (float)j/(float)slices, (float)(i+1)/(float)stacks ) ;
	vv->add(v) ; nn->add(n) ; cc->add(c) ; tt->add(t) ;
        
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




void ssgaCylinder::regenerate ()
{
}



