
#include "ssgAux.h"


void ssgaShape::copy_from ( ssgaShape *src, int clone_flags )
{
  ssgBranch::copy_from ( src, clone_flags ) ;
  if ( src -> isCorrupt () ) makeCorrupt () ;
  sgCopyVec3 ( center, src->center ) ;
  sgCopyVec3 ( size  , src->size   ) ;
  ntriangles = src -> ntriangles ;
}


void ssgaCube    ::copy_from ( ssgaCube     *src, int clone_flags ) { ssgaShape::copy_from ( src, clone_flags ) ;} 
void ssgaSphere  ::copy_from ( ssgaSphere   *src, int clone_flags ) { ssgaShape::copy_from ( src, clone_flags ) ;}
void ssgaCylinder::copy_from ( ssgaCylinder *src, int clone_flags ) { ssgaShape::copy_from ( src, clone_flags ) ;}


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
  regenerate () ;
} 

ssgaCube    ::ssgaCube     ( void ) : ssgaShape ()     { type |= SSGA_TYPE_CUBE     ; } 
ssgaCube    ::ssgaCube     (int nt) : ssgaShape ( nt ) { type |= SSGA_TYPE_CUBE     ; } 

ssgaSphere  ::ssgaSphere   ( void ) : ssgaShape ()     { type |= SSGA_TYPE_SPHERE   ; } 
ssgaSphere  ::ssgaSphere   (int nt) : ssgaShape ( nt ) { type |= SSGA_TYPE_SPHERE   ; } 

ssgaCylinder::ssgaCylinder ( void ) : ssgaShape ()     { type |= SSGA_TYPE_CYLINDER ; }
ssgaCylinder::ssgaCylinder (int nt) : ssgaShape ( nt ) { type |= SSGA_TYPE_CYLINDER ; }

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
  ntriangles = 12 ;

  removeAllKids () ;

  ssgVtxTable     *vt0 = new ssgVtxTable () ;         ssgVtxTable     *vt1 = new ssgVtxTable () ;

  ssgVertexArray   *v0 = new ssgVertexArray   ( 8 ) ; ssgVertexArray   *v1 = new ssgVertexArray   ( 8 ) ;
  ssgNormalArray   *n0 = new ssgNormalArray   ( 8 ) ; ssgNormalArray   *n1 = new ssgNormalArray   ( 8 ) ;
  ssgColourArray   *c0 = new ssgColourArray   ( 8 ) ; ssgColourArray   *c1 = new ssgColourArray   ( 8 ) ;
  ssgTexCoordArray *t0 = new ssgTexCoordArray ( 8 ) ; ssgTexCoordArray *t1 = new ssgTexCoordArray ( 8 ) ;

  addKid ( vt0 ) ; addKid ( vt1 ) ;

  sgVec3 v ; sgVec3 n ; sgVec2 t ; sgVec4 c ;

  sgSetVec3 ( v, center[0]+size[0]/2.0f, center[1]-size[1]/2.0f, center[2]-size[2]/2.0f ) ; v0->add ( v ) ;
  sgSetVec3 ( n,          1            ,          0            ,          0             ) ; n0->add ( n ) ;
  sgSetVec4 ( c,          1            ,    1     ,     1      ,          1             ) ; c0->add ( c ) ;
  sgSetVec2 ( t,          0                       ,                0                    ) ; t0->add ( t ) ;

  sgSetVec3 ( v, center[0]+size[0]/2.0f, center[1]-size[1]/2.0f, center[2]+size[2]/2.0f ) ; v0->add ( v ) ;
  sgSetVec3 ( n,          1            ,          0            ,          0             ) ; n0->add ( n ) ;
  sgSetVec4 ( c,          1            ,    1     ,     1      ,          1             ) ; c0->add ( c ) ;
  sgSetVec2 ( t,          0                       ,                1                    ) ; t0->add ( t ) ;

  sgSetVec3 ( v, center[0]+size[0]/2.0f, center[1]+size[1]/2.0f, center[2]-size[2]/2.0f ) ; v0->add ( v ) ;
  sgSetVec3 ( n,          1            ,          0            ,          0             ) ; n0->add ( n ) ;
  sgSetVec4 ( c,          1            ,    1     ,     1      ,          1             ) ; c0->add ( c ) ;
  sgSetVec2 ( t,          1                       ,                0                    ) ; t0->add ( t ) ;

  sgSetVec3 ( v, center[0]+size[0]/2.0f, center[1]+size[1]/2.0f, center[2]+size[2]/2.0f ) ; v0->add ( v ) ;
  sgSetVec3 ( n,          1            ,          0            ,          0             ) ; n0->add ( n ) ;
  sgSetVec4 ( c,          1            ,    1     ,     1      ,          1             ) ; c0->add ( c ) ;
  sgSetVec2 ( t,          1                       ,                1                    ) ; t0->add ( t ) ;

  sgSetVec3 ( v, center[0]-size[0]/2.0f, center[1]+size[1]/2.0f, center[2]-size[2]/2.0f ) ; v0->add ( v ) ;
  sgSetVec3 ( n,          0            ,          1            ,          0             ) ; n0->add ( n ) ;
  sgSetVec4 ( c,          1            ,    1     ,     1      ,          1             ) ; c0->add ( c ) ;
  sgSetVec2 ( t,          2                       ,                0                    ) ; t0->add ( t ) ;

  sgSetVec3 ( v, center[0]-size[0]/2.0f, center[1]+size[1]/2.0f, center[2]+size[2]/2.0f ) ; v0->add ( v ) ;
  sgSetVec3 ( n,          0            ,          1            ,          0             ) ; n0->add ( n ) ;
  sgSetVec4 ( c,          1            ,    1     ,     1      ,          1             ) ; c0->add ( c ) ;
  sgSetVec2 ( t,          2                       ,                1                    ) ; t0->add ( t ) ;

  sgSetVec3 ( v, center[0]-size[0]/2.0f, center[1]-size[1]/2.0f, center[2]-size[2]/2.0f ) ; v0->add ( v ) ;
  sgSetVec3 ( n,         -1            ,          0            ,          0             ) ; n0->add ( n ) ;
  sgSetVec4 ( c,          1            ,    1     ,     1      ,          1             ) ; c0->add ( c ) ;
  sgSetVec2 ( t,          3                       ,                0                    ) ; t0->add ( t ) ;

  sgSetVec3 ( v, center[0]-size[0]/2.0f, center[1]-size[1]/2.0f, center[2]+size[2]/2.0f ) ; v0->add ( v ) ;
  sgSetVec3 ( n,         -1            ,          0            ,          0             ) ; n0->add ( n ) ;
  sgSetVec4 ( c,          1            ,    1     ,     1      ,          1             ) ; c0->add ( c ) ;
  sgSetVec2 ( t,          3                       ,                1                    ) ; t0->add ( t ) ;

  vt0 -> setVertices  ( v0 ) ;
  vt0 -> setNormals   ( n0 ) ;
  vt0 -> setColours   ( c0 ) ;
  vt0 -> setTexCoords ( t0 ) ;

  vt0 -> recalcBSphere () ;

  sgSetVec3 ( v, center[0]+size[0]/2.0f, center[1]+size[1]/2.0f, center[2]-size[2]/2.0f ) ; v1->add ( v ) ;
  sgSetVec3 ( n,          0            ,          0            ,         -1             ) ; n1->add ( n ) ;
  sgSetVec4 ( c,          1            ,    1     ,     1      ,          1             ) ; c1->add ( c ) ;
  sgSetVec2 ( t,          1                       ,                0                    ) ; t1->add ( t ) ;

  sgSetVec3 ( v, center[0]-size[0]/2.0f, center[1]+size[1]/2.0f, center[2]-size[2]/2.0f ) ; v1->add ( v ) ;
  sgSetVec3 ( n,          0            ,          0            ,         -1             ) ; n1->add ( n ) ;
  sgSetVec4 ( c,          1            ,    1     ,     1      ,          1             ) ; c1->add ( c ) ;
  sgSetVec2 ( t,          0                       ,                0                    ) ; t1->add ( t ) ;

  sgSetVec3 ( v, center[0]+size[0]/2.0f, center[1]-size[1]/2.0f, center[2]-size[2]/2.0f ) ; v1->add ( v ) ;
  sgSetVec3 ( n,          0            ,          0            ,         -1             ) ; n1->add ( n ) ;
  sgSetVec4 ( c,          1            ,    1     ,     1      ,          1             ) ; c1->add ( c ) ;
  sgSetVec2 ( t,          1                       ,                1                    ) ; t1->add ( t ) ;

  sgSetVec3 ( v, center[0]-size[0]/2.0f, center[1]-size[1]/2.0f, center[2]-size[2]/2.0f ) ; v1->add ( v ) ;
  sgSetVec3 ( n,          0            ,          0            ,         -1             ) ; n1->add ( n ) ;
  sgSetVec4 ( c,          1            ,    1     ,     1      ,          1             ) ; c1->add ( c ) ;
  sgSetVec2 ( t,          0                       ,                1                    ) ; t1->add ( t ) ;

  sgSetVec3 ( v, center[0]+size[0]/2.0f, center[1]-size[1]/2.0f, center[2]+size[2]/2.0f ) ; v1->add ( v ) ;
  sgSetVec3 ( n,          0            ,         -1            ,          0             ) ; n1->add ( n ) ;
  sgSetVec4 ( c,          1            ,    1     ,     1      ,          1             ) ; c1->add ( c ) ;
  sgSetVec2 ( t,          1                       ,                2                    ) ; t1->add ( t ) ;

  sgSetVec3 ( v, center[0]-size[0]/2.0f, center[1]-size[1]/2.0f, center[2]+size[2]/2.0f ) ; v1->add ( v ) ;
  sgSetVec3 ( n,          0            ,         -1            ,          0             ) ; n1->add ( n ) ;
  sgSetVec4 ( c,          1            ,    1     ,     1      ,          1             ) ; c1->add ( c ) ;
  sgSetVec2 ( t,          0                       ,                2                    ) ; t1->add ( t ) ;

  sgSetVec3 ( v, center[0]+size[0]/2.0f, center[1]+size[1]/2.0f, center[2]+size[2]/2.0f ) ; v1->add ( v ) ;
  sgSetVec3 ( n,          0            ,          0            ,          1             ) ; n1->add ( n ) ;
  sgSetVec4 ( c,          1            ,    1     ,     1      ,          1             ) ; c1->add ( c ) ;
  sgSetVec2 ( t,          1                       ,                3                    ) ; t1->add ( t ) ;

  sgSetVec3 ( v, center[0]-size[0]/2.0f, center[1]+size[1]/2.0f, center[2]+size[2]/2.0f ) ; v1->add ( v ) ;
  sgSetVec3 ( n,          0            ,          0            ,          1             ) ; n1->add ( n ) ;
  sgSetVec4 ( c,          1            ,    1     ,     1      ,          1             ) ; c1->add ( c ) ;
  sgSetVec2 ( t,          0                       ,                3                    ) ; t1->add ( t ) ;

  vt1 -> setVertices  ( v1 ) ;
  vt1 -> setNormals   ( n1 ) ;
  vt1 -> setColours   ( c1 ) ;
  vt1 -> setTexCoords ( t1 ) ;

  vt1 -> recalcBSphere () ;

  recalcBSphere () ;
}

void ssgaSphere  ::regenerate ()
{
}


void ssgaCylinder::regenerate ()
{
}



