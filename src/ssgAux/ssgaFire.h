
#include "ssgAux.h"


class ssgaFire : public ssgaParticleSystem
{
public:

  ssgaFire ( int num,
             int initial_num,
             float _create_rate,
             float bsphere_size ) ;

  virtual ~ssgaFire () ;

  virtual void update ( float t ) ;
} ;

unsigned char *_ssgaGetFireTexture () ;

