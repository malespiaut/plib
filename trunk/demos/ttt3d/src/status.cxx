
#include "p3d.h"
#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
#include <stdarg.h>

#define MAX_STRING          30
#define MAX_STRING_LENGTH  256

static int displayed_score = 0 ;

static fntRenderer *text = NULL ;

char debug_strings [ MAX_STRING ][ MAX_STRING_LENGTH ] ;
/*int next_string   = 0 ;
int stats_enabled = FALSE ;*/

static int    about_timer = 99999 ;
static int versions_timer = 99999 ;
static int  credits_timer = 99999 ;
static int    intro_timer =    0  ;
static int     help_timer = 99999 ;

void hide_status () { versions_timer = credits_timer =
                         intro_timer = help_timer = about_timer = 99999 ; }
void about       () {    about_timer = 0 ; }
void credits     () {  credits_timer = 0 ; }
void versions    () { versions_timer = 0 ; }
void help        () {     help_timer = 0 ; }

static void drawText ( const char *str, int sz, int x, int y )
{
  text -> setFont      ( font ) ;
  text -> setPointSize ( sz ) ;

  text -> begin () ;
    text -> start2f ( x, y ) ;
    text -> puts ( str ) ;
  text -> end () ;
}


static void drawInverseDropShadowText ( const char *str, int sz, int x, int y )
{
  glColor4f ( 1.0f, 1.0f, 1.0f, 1.0f ) ;
  drawText ( str, sz, x, y ) ;
  glColor4f ( 0.0f, 0.0f, 0.0f, 1.0f ) ;
  drawText ( str, sz, x+1, y+1 ) ;
}


static void drawDropShadowText ( const char *str, int sz, int x, int y )
{
  glColor4f ( 0.0f, 0.0f, 0.0f, 1.0f ) ;

  drawText ( str, sz, x, y ) ;

  glColor4f ( 1.0f, 1.0f, 1.0f, 1.0f ) ;

  drawText ( str, sz, x+1, y+1 ) ;
}


static void drawHelpText ()
{
  drawDropShadowText ( "Press SPACE to toggle the menu.", 18, 70, 400 ) ;
  drawDropShadowText ( "Press ESCAPE to exit the game.", 18, 70, 370 ) ;
  drawDropShadowText ( "Use arrow keys and PgUp,PgDown,Home,End,Ins", 18, 70, 340 ) ;
  drawDropShadowText ( "          or Del to move the grid around.", 18, 70, 310 ) ;
  drawDropShadowText ( "Move the current pipe with A,S,Z,W,E,X", 18, 70, 280 ) ;
}


static void drawTitleText ()
{
  drawDropShadowText ( "TTT3D", 20, 70, 400 ) ;
  drawDropShadowText ( "By Steve Baker", 12, 170, 385 ) ;
}


static void drawIntroText ()
{
  drawTitleText () ;

  if ( intro_timer & 8 )
    drawDropShadowText ( "Press SPACE bar for menu.",
                       15, 10, 430 ) ;
}


static const char *aboutText [] =
{
  "  Yada, yada, yada.",
  NULL
} ;


static void drawVersionsText ()
{
  char str [ 256 ] ;

#ifdef VERSION
  sprintf ( str, "TTT3D: Version: %s", VERSION ) ;
#else
  sprintf ( str, "TTT3D: Unknown Version." ) ;
#endif
  drawDropShadowText ( str, 15, 20, 250 ) ;

  sprintf ( str, "PLIB Version: %s", ssgGetVersion() ) ;
  drawDropShadowText ( str, 15, 20, 225 ) ;

  sprintf ( str, "OpenGL Version: %s", glGetString ( GL_VERSION ) ) ;
  drawDropShadowText ( str, 15, 20, 200 ) ;

  sprintf ( str, "OpenGL Vendor: %s", glGetString ( GL_VENDOR ) ) ;
  drawDropShadowText ( str, 15, 20, 175 ) ;

  sprintf ( str, "OpenGL Renderer: %s", glGetString ( GL_RENDERER ) ) ;

  if ( strlen ( str ) > 50 )
  {
    int l = strlen ( str ) ;
    int ll = 0 ;

    for ( int i = 0 ; i < l ; i++, ll++ )
    {
      if ( ll > 40 && str[i] == ' ' )
      {
        str[i] = '\n' ;
        ll = 0 ;
      }
    }
  }

  drawDropShadowText ( str, 15, 20, 150 ) ;

  if ( versions_timer & 8 )
    drawDropShadowText ( "Press SPACE to continue",
                       15, 10, 430 ) ;
}


static void drawAboutText ()
{
  drawTitleText () ;

  for ( int i = 0 ; aboutText [ i ] != NULL ; i++ )
    drawDropShadowText ( aboutText [ i ],
                       10, 40, 230 - 10 * i ) ;

  if ( about_timer & 8 )
    drawDropShadowText ( "Press SPACE to continue",
                       15, 10, 430 ) ;
}


static const char *creditsText [] =
{
  "  Steve  Baker    - Coding, design, bug insertion.",
  NULL
} ;



static void drawCreditsText ()
{
  drawTitleText () ;

  drawDropShadowText ( "Credits:",
                       20, 70, 250 ) ;

  for ( int i = 0 ; creditsText [ i ] != NULL ; i++ )
    drawDropShadowText ( creditsText [ i ],
                       12, 100, 230 - 12 * i ) ;

  if ( credits_timer & 8 )
    drawDropShadowText ( "Press SPACE to continue",
                       15, 10, 430 ) ;
}



static void drawScore ()
{
  if ( displayed_score > 0 )
  {
    char str [ 20 ] ;
    sprintf ( str, "%9d", displayed_score ) ;
    drawDropShadowText ( str, 18, 500, 410 ) ;
  }
}



static void drawInGameScore ()
{
  char str [ 50 ] ;

  sprintf ( str, "TicTacToe 3D" ) ;
  drawInverseDropShadowText ( str, 18, 360, 460 ) ;

  sprintf ( str, "A,Z : Move UP or DOWN" ) ;
  drawDropShadowText ( str, 18, 360, 420 ) ;
  sprintf ( str, "S,D,X,C : Move Sideways" ) ;
  drawDropShadowText ( str, 18, 360, 400 ) ;
  sprintf ( str, "Enter : Play at cursor" ) ;
  drawDropShadowText ( str, 18, 360, 380 ) ;
  sprintf ( str, "Arrow Keys : Spin Board" ) ;
  drawDropShadowText ( str, 18, 360, 360 ) ;

  sprintf ( str, "Four in a row to win" ) ;
  drawInverseDropShadowText ( str, 18, 60, 420 ) ;
}



static void drawGameOverText ( int score )
{
  static int timer = 0 ;

  displayed_score = score ;

  drawTitleText () ;
  drawScore () ;

  glColor4f ( sin ( (float)timer/5.1f ) / 2.0f + 0.5f,
              sin ( (float)timer/6.3f ) / 2.0f + 0.5f,
              sin ( (float)timer/7.2f ) / 2.0f + 0.5f, 0.5 ) ;


  switch ( puzzle -> getGameState () )
  {
    case STILL_PLAYING : break ;

    case HUMAN_WIN     : drawText ( "YOU" , 50, 220, 230 ) ;
                         drawText ( "WON!", 50, 150, 180 ) ;
                         break ;

    case COMPUTER_WIN  : drawText ( "YOU"  , 50, 220, 230 ) ;
                         drawText ( "LOSE!", 50, 150, 180 ) ;
                         break ;

    case DRAWN_GAME    : drawText ( "ITS A", 50, 220, 230 ) ;
                         drawText ( "DRAW!", 50, 150, 180 ) ;
                         break ;
  }

  if ( timer++ & 8 )
  {
    drawDropShadowText ( "Press R to reset and play again", 15, 10, 30 ) ;
  }
}


static void drawGameIntroText ()
{
  static int timer = 0 ;

  if ( timer++ & 8 )
    drawDropShadowText ( "Press S to start", 15, 10, 430 ) ;

  if ( help_timer++ < 400 )
    drawHelpText () ;
  else
  if ( intro_timer++ < 400 )
    drawIntroText () ;
  else
  if ( credits_timer++ < 1600 )
    drawCreditsText () ;
  else
  if ( about_timer++ < 1600 )
    drawAboutText () ;
  else
  if ( versions_timer++ < 1600 )
    drawVersionsText () ;
}


static void drawGameRunningText ( int score )
{
  displayed_score = score ;
  drawInGameScore () ;

  glColor4f ( 0.6f, 0.0f, 0.6f, 1.0f ) ;

  if ( help_timer++ < 400 )
    drawHelpText () ;
  else
  if ( credits_timer++ < 1600 )
    drawCreditsText () ;
  else
  if ( about_timer++ < 1600 )
    drawAboutText () ;
  else
  if ( versions_timer++ < 1600 )
    drawVersionsText () ;
}



void drawStatusText ( int score )
{
  if ( text == NULL )
    text = new fntRenderer () ;

  glMatrixMode   ( GL_MODELVIEW ) ;
  glPushMatrix   () ;
  glLoadIdentity () ;

  glMatrixMode   ( GL_PROJECTION ) ;
  glPushMatrix   () ;
  glLoadIdentity () ;

  glPushAttrib   ( GL_ENABLE_BIT | GL_LIGHTING_BIT ) ;
  glDisable      ( GL_DEPTH_TEST ) ;
  glDisable      ( GL_LIGHTING   ) ;
  glDisable      ( GL_FOG        ) ;
  glDisable      ( GL_CULL_FACE  ) ;
  glDisable      ( GL_ALPHA_TEST ) ;
  glEnable       ( GL_BLEND      ) ;

  glOrtho        ( 0, 640, 0, 480, 0, 100 ) ;

  switch ( game_state )
  {
    case GAME_INTRO   : drawGameIntroText   ()        ; break ; 
    case GAME_DEBRIEF : drawGameOverText    ( score ) ; break ; 
    case GAME_RUNNING : drawGameRunningText ( score ) ; break ;
  }

  glPopAttrib  () ;
  glPopMatrix  () ;
  glMatrixMode ( GL_MODELVIEW ) ;
  glPopMatrix  () ;
}


