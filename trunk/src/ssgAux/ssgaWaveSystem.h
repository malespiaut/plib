
#include "ssgaShapes.h"

typedef float (* ssgaWSDepthCallback ) ( float x, float y ) ;

class ssgaWaveSystem : public ssgaShape
{
  ssgaWSDepthCallback gridGetter ;

  sgVec3 *normals   ;
  sgVec4 *colours   ;
  sgVec2 *texcoords ;
  sgVec3 *vertices  ;
  sgVec3 *orig_vertices  ;

  float windSpeed   ;
  float windHeading ;
  float waveHeight  ;
  float kappa  ;
  float lambda ;
  float omega  ;
  float edgeFlatten ;

  float tu, tv ;

  int nstrips ;
  int nstacks ;

protected:
  virtual void copy_from ( ssgaWaveSystem *src, int clone_flags ) ;
public:

  ssgaWaveSystem ( int ntri ) ;

  virtual ~ssgaWaveSystem () ;

  virtual ssgBase    *clone       ( int clone_flags = 0 ) ;
  virtual void        regenerate  () ;
  virtual const char *getTypeName ( void ) ;
 
  virtual int load ( FILE * ) ;
  virtual int save ( FILE * ) ;

  ssgaWSDepthCallback getDepthCallback () { return gridGetter ; } 

  float getWindSpeed     () { return windSpeed   ; }
  float getWindDirn      () { return windHeading ; }
  float getWaveHeight    () { return waveHeight  ; }
  float getEdgeFlatten   () { return edgeFlatten ; }

  void  setDepthCallback ( ssgaWSDepthCallback cb ) { gridGetter  = cb ; } 
  void  setWindSpeed     ( float speed            ) { windSpeed   = speed ; }
  void  setWindDirn      ( float dirn             ) { windHeading = dirn  ; }
  void  setWaveHeight    ( float height           ) { waveHeight  = height ; }
  void  setEdgeFlatten   ( float dist             ) { edgeFlatten = dist ; }
  void  setTexScale      ( float u, float v       ) { tu = u ; tv = v ; }

  void updateAnimation ( float t ) ;
} ;


