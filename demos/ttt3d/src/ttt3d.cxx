
#include "p3d.h"
#include <signal.h>

#ifdef _WIN32
#  include <io.h>
#  include <direct.h>

#  define access _access
#  define chdir _chdir
#  define F_OK 04
#endif

char   *ttt3d_datadir = NULL ;
GFX              *gfx = NULL ;
GUI              *gui = NULL ;
SoundSystem    *sound = NULL ;
ssgRoot        *scene = NULL ;
ssgTransform    *spin = NULL ;
Puzzle        *puzzle = NULL ;
float        spin_rot = 0.0f ;
float        spin_alt = 0.0f ;
float        spin_pit = 0.0f ;
float        zoom = 60.0f ;
int        game_state = GAME_START_OF_DAY ;

Level level ;

static void banner ()
{
  printf ( "\n\n" ) ;
  printf ( "             TTT3D.\n" ) ;
  printf ( "\n" ) ;
  printf ( "               /    /   \n" ) ;
  printf ( "          ____/____/____\n" ) ;
  printf ( "             /    /     \n" ) ;
  printf ( "        ____/____/____  \n" ) ;
  printf ( "           /    /       \n" ) ;
  printf ( "          /    /        \n" ) ;
  printf ( "\n" ) ;
  printf ( "               by Steve Baker\n" ) ;
  printf ( "                 <sjbaker1@airmail.net>\n" ) ;
  printf ( "\n\n" ) ;
}

static void cmdline_help ()
{
  banner () ;

  printf ( "Usage:\n\n" ) ;
  printf ( "    ttt3d [OPTIONS]...\n\n" ) ;
  printf ( "Options:\n" ) ;
  printf ( "  -h, --help            Display this help message.\n" ) ;
  printf ( "  -D, --datadir DIR     Load the game data from DIR defaults\n" ) ;
  printf ( "                        to /usr/local/share/games/ttt3d\n" ) ;
  printf ( "\n" ) ;
}


int main ( int argc, char **argv )
{
  for ( int i = 1 ; i < argc ; i++ )
  {
    if ( argv[i][0] == '-' )
      switch ( argv[i][1] )
      {
        case '-' :
          {
            if ( strcmp ( & argv[i][2], "help" ) == 0 )
            {
              cmdline_help () ;
              exit ( 0 ) ;
            }
          }
          break ;

        case 'h' : case 'H' : cmdline_help () ; exit ( 0 ) ;
        default  : break ;
      }
  }

  /* Set tux_aqfh_datadir to the correct directory */

  if ( ttt3d_datadir == NULL )
  {
    if ( getenv ( "TTT3D_DATADIR" ) != NULL )
      ttt3d_datadir = getenv ( "TTT3D_DATADIR" ) ;
    else
    if ( access ( "data/levels.dat", F_OK ) == 0 )
      ttt3d_datadir = "." ;
    else
    if ( access ( "../data/levels.dat", F_OK ) == 0 )
      ttt3d_datadir = ".." ;
    else
#ifdef TTT3D_DATADIR
      ttt3d_datadir = TTT3D_DATADIR ;
#else
      ttt3d_datadir = "/usr/local/share/games/ttt3d" ;
#endif
  }

  fprintf ( stderr, "Data files will be fetched from: '%s'\n",
                                                    ttt3d_datadir ) ;

  if ( chdir ( ttt3d_datadir ) == -1 )
  {
    fprintf ( stderr, "Couldn't chdir() to '%s'.\n", ttt3d_datadir ) ;
    exit ( 1 ) ;
  }

  banner () ;

  gfx   = new GFX ;

  sound = new SoundSystem ;
  sound -> change_track ( "mods/mfarmer.mod" ) ;

  gui   = new GUI ;

  scene = new ssgRoot ;
  spin  = new ssgTransform ;
  scene -> addKid ( spin ) ;

  puzzle = new Puzzle () ;

  spin -> addKid ( puzzle -> getSSG () ) ;
  spinLeft () ;

  signal ( 11, SIG_DFL ) ;
  glutMainLoop () ;
  return 0 ;
}



void spinPuzzle ()
{
  sgMat4 rot, pit, final ;

  sgMakeRotMat4 ( rot, spin_rot, 0, 0 ) ;
  sgMakeRotMat4 ( pit, 0, spin_pit, 0 ) ;
  sgMultMat4    ( final, pit, rot ) ;
  sgSetVec3     ( final[3], 0, 8, 0 /* spin_alt */ ) ;

  spin -> setTransform ( final ) ;
}

void spinUp ()
{
  spin_alt -= 0.4f ;
  spin_pit += 3.5f ;
  spinPuzzle () ;
}

void spinDown ()
{
  spin_alt += 0.4f ;
  spin_pit -= 3.5f ;
  spinPuzzle () ;
}

void spinRight ()
{
  spin_rot += 4.4f ;
  spinPuzzle () ;
}

void spinLeft ()
{
  spin_rot -= 4.4f ;
  spinPuzzle () ;
}

void zoomIn ()
{
  zoom *= 1.1 ;

  if ( zoom > 160 ) zoom = 160 ;

  ssgSetFOV ( zoom, zoom * 3.0f/4.0f ) ;
}

void zoomOut ()
{
  if ( zoom < 6 ) zoom = 6 ;

  zoom /= 1.1 ;

  ssgSetFOV ( zoom, zoom * 3.0f/4.0f ) ;
}


void startLevel ()
{
  game_state = GAME_RUNNING ;
  puzzle -> reset () ;
  puzzle -> update () ;
  sound  -> playSfx ( SOUND_CLAP ) ;
}


void makeMove ()
{
  if ( get_board_entry ( puzzle->getCx(),
                         puzzle->getCy(),
                         puzzle->getCz() ) != ' ' )
    sound->playSfx ( SOUND_FROG  ) ;
  else
  {
    int res = game_update ( puzzle->getCx(),
                            puzzle->getCy(),
                            puzzle->getCz() ) ;

    for ( int x = 0 ; x < 4 ; x++ )
      for ( int y = 0 ; y < 4 ; y++ )
        for ( int z = 0 ; z < 4 ; z++ )
          switch ( get_board_entry ( x, y, z ) )
          {
            case ' ' : puzzle->put_empty ( x, y, z ) ; break ;
            case 'X' : puzzle->put_X     ( x, y, z ) ; break ;
            case 'O' : puzzle->put_O     ( x, y, z ) ; break ;
            default  : puzzle->put_empty ( x, y, z ) ; break ;
          }

    puzzle->setGameState ( res ) ;

    sound->playSfx ( SOUND_POP  ) ;

    if ( res == HUMAN_WIN )
      sound->playSfx ( SOUND_CLAP   ) ;
    else
    if ( res == COMPUTER_WIN )
      sound->playSfx ( SOUND_GLASBK ) ;
    else
    if ( res == DRAWN_GAME )
      sound->playSfx ( SOUND_AHOOGA ) ;
  }
}


void ttt3dMainLoop ()
{
  switch ( game_state )
  {
    case GAME_START_OF_DAY : startLevel () ; break ;
    case GAME_INTRO        : startLevel () ; break ;
    case GAME_DEBRIEF      : break ;

    case GAME_RUNNING :
      if ( puzzle->gameOver () )
        game_state = GAME_DEBRIEF ;
      break ;
  }

  glLineWidth ( 2 ) ;
  puzzle -> update () ;
  gfx    -> update () ;
  gui    -> update () ;
  sound  -> update () ;
  gfx    -> done   () ;  /* Swap buffers! */
}


