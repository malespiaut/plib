
struct ssgaParticle
{
  sgVec4 col ;
  sgVec3 pos ;
  sgVec3 vel ;
  sgVec3 acc ;

  float time_to_live ;

  void init () ;
  void update ( float dt ) ;
} ;


typedef void (* ssgaParticleFunc) ( ssgaParticle *p ) ;


class ssgaParticleSystem : public ssgVtxArray
{
  int num_particles  ;
  int num_verts      ;
  int turn_to_face   ;
  ssgaParticle *particle ;

  float create_error ;
  float create_rate ;

  float size ;

  ssgaParticleFunc particle_create ;
  ssgaParticleFunc particle_update ;

public:

  ssgaParticleSystem ( int num, int initial_num,
                       float _create_rate, int _turn_to_face,
                       float sz, float bsphere_size,
                       ssgaParticleFunc _particle_create,
                       ssgaParticleFunc _particle_update ) ;

  void update ( float t ) ;
  void draw_geometry () ;
} ;


