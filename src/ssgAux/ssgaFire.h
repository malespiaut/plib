
#include "ssgAux.h"


class ssgaFire : public ssgaParticleSystem
{
  int    tableSize   ;
  float *colourTable ;
  float *sizeTable   ;

  sgVec4 hot_colour  ;
  float max_ttl      ;
  float start_size   ;
  float upward_speed ;
  float radius       ;

  void reInit () ;

public:

  void createParticle ( int idx, ssgaParticle *p ) ;
  void updateParticle ( int idx, ssgaParticle *p ) ;

  ssgaFire ( int num_tris, float _radius = 1.0f, 
                           float height  = 5.0f,
                           float speed   = 2.0f ) ;

  virtual ~ssgaFire () ;

  virtual void update ( float t ) ;

  void setUpwardSpeed ( float spd )
  {
    upward_speed = spd ;
  }

  void setHeight    ( float hgt )
  {
    max_ttl = hgt / upward_speed ;
    getBSphere () -> setRadius ( hgt * 2.0f ) ;
    getBSphere () -> setCenter ( 0, 0, 0 ) ;
    reInit () ;
  }

  void setHotColour ( sgVec4 col )
  {
    sgCopyVec4 ( hot_colour, col ) ;
    reInit () ;
  }

  void getHotColour ( sgVec4 col ) { sgCopyVec4 ( col, hot_colour ) ; }

} ;

unsigned char *_ssgaGetFireTexture () ;

