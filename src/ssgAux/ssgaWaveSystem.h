
#include "ssgaShapes.h"

typedef float (* ssgaWSDepthCallback ) ( float x, float y ) ;

#define SSGA_MAX_WAVETRAIN  16

class ssgaWaveTrain
{
  float height ;
  float length ;
  float lambda ;
  float speed  ;
  float heading;

public:

  ssgaWaveTrain ()
  {
    height  = 0.5f ;
    length  = 0.8f ;
    lambda  = 1.0f ;
    speed   = (float) sqrt ( 2.0f/3.0f ) ;
    heading = 0.0f ;
  }

  float getSpeed () { return speed  ; }
  void  setSpeed ( float h ) { speed  = h ; }

  float getLength () { return length  ; }
  void  setLength ( float h ) { length  = h ; }

  float getLambda () { return lambda  ; }
  void  setLambda ( float h ) { lambda  = h ; }

  float getHeading () { return heading  ; }
  void  setHeading ( float h ) { heading  = h ; }

  float getWaveHeight () { return height  ; }
  void  setWaveHeight ( float h ) { height  = h ; }
} ;

class ssgaWaveSystem : public ssgaShape
{
  ssgaWSDepthCallback gridGetter ;

  sgVec3 *normals   ;
  sgVec4 *colours   ;
  sgVec2 *texcoords ;
  sgVec3 *vertices  ;
  sgVec3 *orig_vertices  ;

  ssgaWaveTrain *train [ SSGA_MAX_WAVETRAIN ] ;

  float windSpeed   ;
  float windHeading ;
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

  ssgaWaveTrain *getWaveTrain ( int i )
  {
    return ( i < 0 || i >= SSGA_MAX_WAVETRAIN ) ? NULL : train [ i ] ;
  }

  void setWaveTrain ( int i, ssgaWaveTrain *t )
  {
    assert ( i >= 0 && i < SSGA_MAX_WAVETRAIN ) ;
    train [ i ] = t ;
  }

  float getWindSpeed     () { return windSpeed   ; }
  float getWindDirn      () { return windHeading ; }
  float getEdgeFlatten   () { return edgeFlatten ; }

  void  setDepthCallback ( ssgaWSDepthCallback cb ) { gridGetter  = cb ; } 
  void  setWindSpeed     ( float speed            ) { windSpeed   = speed ; }
  void  setWindDirn      ( float dirn             ) { windHeading = dirn  ; }
  void  setEdgeFlatten   ( float dist             ) { edgeFlatten = dist ; }
  void  setTexScale      ( float u, float v       ) { tu = u ; tv = v ; }

  void updateAnimation ( float t ) ;
} ;


