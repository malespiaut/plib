
#define SOUND_AHOOGA		0
#define SOUND_CLAP		1
#define SOUND_FROG		2
#define SOUND_GLASBK		3
#define SOUND_POP		4
#define SOUND_UGH		5
#define SOUND_WHO_ELSE		6
 
class SoundSystem
{
  char current_track [ 256 ] ;
  slScheduler *sched ;

public:
  SoundSystem () ;

  void update () ;
  void playSfx ( int sound ) ;

  void setSafetyMargin ( float t = 0.25 )
  {
    sched -> setSafetyMargin ( t ) ;
  }

  void  change_track ( const char *fname ) ;
  void disable_music () ;
  void  enable_music () ;

  void disable_sfx   () ;
  void  enable_sfx   () ;
} ;

