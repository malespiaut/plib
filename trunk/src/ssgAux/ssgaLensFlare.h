
#include "ssgaShapes.h"

class ssgaLensFlare : public ssgaShape
{
  ssgVtxTable      *vt ;
  ssgVertexArray   *v0 ;
  ssgNormalArray   *n0 ;
  ssgColourArray   *c0 ;
  ssgTexCoordArray *t0 ;

  void update ( sgMat4 m ) ;

protected:
  virtual void copy_from ( ssgaLensFlare *src, int clone_flags ) ;
public:

  ssgaLensFlare () ;
  ssgaLensFlare ( int nt ) ;

  virtual ~ssgaLensFlare () ;

  virtual ssgBase    *clone       ( int clone_flags = 0 ) ;
  virtual void        regenerate  () ;
  virtual const char *getTypeName ( void ) ;
  virtual void        cull  ( sgFrustum *f, sgMat4 m, int test_needed ) ;
} ;

unsigned char *_ssgaGetLensFlareTexture () ;


