
#include "p3d.h"
#include "plib/pu.h"

static int mouse_x ;
static int mouse_y ;
static int mouse_dx = 0 ;
static int mouse_dy = 0 ;
static int mouse_buttons = 0 ;

fntTexFont *font ;

static void motionfn ( int x, int y )
{
  mouse_x = x ;
  mouse_y = y ;
  mouse_dx += mouse_x - 320 ;
  mouse_dy += mouse_y - 240 ;
  puMouse ( x, y ) ;
}

static void mousefn ( int button, int updown, int x, int y )
{
  mouse_x = x ;
  mouse_y = y ;

  if ( updown == GLUT_DOWN )
    mouse_buttons |= (1<<button) ;
  else
    mouse_buttons &= ~(1<<button) ;

  mouse_dx += mouse_x - 320 ;
  mouse_dy += mouse_y - 240 ;

  puMouse ( button, updown, x, y ) ;

  if ( updown == GLUT_DOWN )
    hide_status () ;
}

static void credits_cb ( puObject * )
{
  hide_status () ;
  credits () ;
}

static void versions_cb ( puObject * )
{
  hide_status () ;
  versions () ;
}

static void about_cb ( puObject * )
{
  hide_status () ;
  about () ;
}

static void help_cb ( puObject * )
{
  hide_status () ;
  help () ;
}


static void music_off_cb     ( puObject * ) { sound->disable_music () ; } 
static void music_on_cb      ( puObject * ) { sound->enable_music  () ; } 
static void sfx_off_cb       ( puObject * ) { sound->disable_sfx   () ; } 
static void sfx_on_cb        ( puObject * ) { sound->enable_sfx    () ; } 

static void exit_cb ( puObject * )
{
  fprintf ( stderr, "Exiting TTT3D.\n" ) ;
  exit ( 1 ) ;
}

/* Menu bar entries: */

static char      *exit_submenu    [] = {  "Exit", NULL } ;
static puCallback exit_submenu_cb [] = { exit_cb, NULL } ;

static char      *sound_submenu    [] = { "Turn off Music", "Turn off Sounds", "Turn on Music", "Turn on Sounds", NULL } ;
static puCallback sound_submenu_cb [] = {  music_off_cb,        sfx_off_cb,     music_on_cb,        sfx_on_cb, NULL } ;

static char      *help_submenu    [] = { "Versions...", "Credits...", "About...",  "Help", NULL } ;
static puCallback help_submenu_cb [] = {   versions_cb,   credits_cb,   about_cb, help_cb, NULL } ;



GUI::GUI ()
{
  hidden = TRUE ;
  mouse_x = 320 ;
  mouse_y = 240 ;

  glutMouseFunc         ( mousefn   ) ;
  glutMotionFunc        ( motionfn  ) ;
  glutPassiveMotionFunc ( motionfn  ) ;
 
  ssgInit () ;
  puInit () ;

  font = new fntTexFont ;
  font -> load ( "fonts/sorority.txf" ) ;
  puFont ff ( font, 15 ) ;
  puSetDefaultFonts        ( ff, ff ) ;
  puSetDefaultStyle        ( PUSTYLE_SMALL_SHADED ) ;
  puSetDefaultColourScheme ( 0.8, 0.8, 0.8, 0.6 ) ;

  /* Make the menu bar */

  main_menu_bar = new puMenuBar () ;

    main_menu_bar -> add_submenu ( "Exit" , exit_submenu ,  exit_submenu_cb ) ;
    main_menu_bar -> add_submenu ( "Sound", sound_submenu, sound_submenu_cb ) ;
    main_menu_bar -> add_submenu ( "Help" , help_submenu ,  help_submenu_cb ) ;

  main_menu_bar -> close () ;
  main_menu_bar -> hide  () ;
}


void GUI::show ()
{
  hide_status () ;
  hidden = FALSE ;
  main_menu_bar -> reveal () ;
}

void GUI::hide ()
{
  hidden = TRUE ;
  hide_status () ;
  main_menu_bar -> hide () ;
}

void GUI::update ()
{
  int score = 0 ; /* INSERT SCORE-GETTER HERE */

  keyboardInput  () ;
  drawStatusText ( score ) ;

  glBlendFunc ( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA ) ;
  glAlphaFunc ( GL_GREATER, 0.1f ) ;
  glEnable    ( GL_BLEND ) ;

  puDisplay () ;
}


void GUI::keyboardInput ()
{
  int c = getGLUTKeystroke () ;

  if ( c <= 0 )
    return ;

  switch ( c )
  {
    case 0x1B /* Escape */      :
    case 0x03 /* Control-C */   : exit ( 0 ) ;

    case 'h' : hide_status () ; help  () ; return ;

    case ' ' : if ( isHidden () )
	       {
		 sound->playSfx ( SOUND_WHO_ELSE ) ;
		 show () ;
	       }
	       else
	       {
		 sound->playSfx ( SOUND_UGH ) ;
		 hide () ;
	       }
	       return ;

    case 'r' :
    case 'R' : startLevel () ; return ;

    case (256+GLUT_KEY_LEFT      ) : spinLeft  () ; return ;
    case (256+GLUT_KEY_RIGHT     ) : spinRight () ; return ;
    case (256+GLUT_KEY_UP        ) : spinUp    () ; return ;
    case (256+GLUT_KEY_DOWN      ) : spinDown  () ; return ;
    case (256+GLUT_KEY_PAGE_UP   ) : zoomIn    () ; return ;
    case (256+GLUT_KEY_PAGE_DOWN ) : zoomOut   () ; return ;

    default : /* DO NOTHING */ break ;
  }

  if ( game_state == GAME_DEBRIEF )
  {
    switch ( c )
    {
      case 'r' :
      case 'R' : startLevel () ; return ;
      default : /* DO NOTHING */ break ;
    }
  }

  if ( game_state == GAME_RUNNING )
  {
    switch ( c )
    {
      case 'r' :
      case 'R' : startLevel () ; return ;

      case '\n':
      case '\r': makeMove () ; return ;

      case 'a' : puzzle->cursor_up    () ; return ;
      case 'z' : puzzle->cursor_down  () ; return ;
      case 's' : puzzle->cursor_in    () ; return ;
      case 'd' : puzzle->cursor_out   () ; return ;
      case 'x' : puzzle->cursor_left  () ; return ;
      case 'c' : puzzle->cursor_right () ; return ;

      default : /* DO NOTHING */ break ;
    }
  }
}


