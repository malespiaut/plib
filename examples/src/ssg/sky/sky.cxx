/*
     PLIB - A Suite of Portable Game Libraries
     Copyright (C) 1998,2002  Steve Baker
 
     This library is free software; you can redistribute it and/or
     modify it under the terms of the GNU Library General Public
     License as published by the Free Software Foundation; either
     version 2 of the License, or (at your option) any later version.
 
     This library is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
     Library General Public License for more details.
 
     You should have received a copy of the GNU Library General Public
     License along with this library; if not, write to the Free Software
     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 
     For further information visit http://plib.sourceforge.net

     $Id$
*/


#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#ifdef WIN32
#  include <windows.h>
#else
#  include <unistd.h>
#endif
#include <math.h>
#include <plib/ssg.h>
#include <plib/pu.h>
#include <plib/ssgAux.h>
#include <plib/ssgaSky.h>

#ifdef FREEGLUT_IS_PRESENT
#  include <GL/freeglut.h>
#else
#  ifdef __APPLE__
#    include <GLUT/glut.h>
#  else
#    include <GL/glut.h>
#  endif
#endif

#define ROT_SPEED   2.0f
#define TRANS_SPEED 5.0f

#define MAX_BODIES  2
#define MAX_CLOUDS  3

#define GUI_BASE      20
#define FONT_COLOUR   1,1,1,1

static int wireframe  = FALSE ;
static int displayGUI = TRUE  ;

static puSelectBox *bodySelectBox         = (puSelectBox *) NULL ;
static puDial      *bodyRADial            = (puDial      *) NULL ;
static puDial      *bodyDeclDial          = (puDial      *) NULL ;
static puSlider    *cloudElevationSlider  = (puSlider    *) NULL ;
static puSelectBox *cloudSelectBox        = (puSelectBox *) NULL ;
static puButton    *cloudEnableButton     = (puButton    *) NULL ;

static ssgRoot            *scene              = NULL ;
static ssgaSky            *sky                = NULL ;
static ssgTransform       *teapot             = NULL ;
static ssgTransform       *wave               = NULL ;

static ssgaTeapot         *tpt_obj            = NULL ;
static ssgaWaveSystem     *wave_obj           = NULL ;
static ssgaWaveTrain       wave_train;

static ssgaCelestialBody  *bodies[MAX_BODIES] = {NULL} ;
static ssgaCloudLayer     *clouds[MAX_CLOUDS] = {NULL} ;

static ssgSimpleState     *teapot_state       = NULL ;
static ssgSimpleState     *wave_state         = NULL ;

static int                 nstars             = 1000 ;
static sgdVec3            *star_data          = NULL ;
static int                 nplanets           = 0 ;
static sgdVec3            *planet_data        = NULL ;

static char *bodyNameList[]  = { "Sun", "Moon", NULL } ;
static char *cloudNameList[] = { "Cloud 0", "Cloud 1", "Cloud 2", NULL } ;

static int   curr_body  = 0 ;
static int   curr_cloud = 0 ;

static sgVec4 black             = { 0.0, 0.0, 0.0, 1.0 } ;
static sgVec4 white             = { 1.0, 1.0, 1.0, 1.0 } ;
static sgVec4 translucent_white = { 1.0, 1.0, 1.0, 0.8 } ;

static sgVec4 base_sky_color    = { 0.39, 0.5, 0.74, 1.0 } ;
static sgVec4 base_fog_color    = { 0.84, 0.87, 1.0, 1.0 } ;

static sgVec4 base_ambient      = { 0.2, 0.2, 0.2, 1.0 } ;
static sgVec4 base_diffuse      = { 1.0, 1.0, 1.0, 1.0 } ;
static sgVec4 base_specular     = { 1.0, 1.0, 1.0, 1.0 } ;

static sgVec4 sky_color ;
static sgVec4 fog_color ;
static sgVec4 cloud_color ;

static sgVec4 scene_ambient ;
static sgVec4 scene_diffuse ;
static sgVec4 scene_specular ;

static const double m_log01 = -log( 0.01 );
static const double sqrt_m_log01 = sqrt( m_log01 );

static int    mods = 0 ;
unsigned char keypress = 0 ;
static int    special_key = 0 ;

static sgCoord campos = { { -300, 0, 10 }, { -90, 0, 0 } } ;

static void bodySelectBox_cb ( puObject *ob )
{
  curr_body = ((puSelectBox *) ob) -> getCurrentItem () ;

  if ( curr_body < 0 )
    curr_body = 0 ;

  if ( curr_body >= MAX_BODIES )
    curr_body = MAX_BODIES - 1 ;

  bodyRADial   -> setValue ( float(bodies[curr_body]->getRightAscension () * SGD_RADIANS_TO_DEGREES) ) ;
  bodyDeclDial -> setValue ( float(bodies[curr_body]->getDeclination    () * SGD_RADIANS_TO_DEGREES) ) ;
}

static void bodyRADial_cb ( puObject *ob )
{
  bodies[curr_body] -> setRightAscension ( ob -> getFloatValue () * SGD_DEGREES_TO_RADIANS ) ;
  ob -> setLegend ( ob -> getStringValue () ) ;
}

static void bodyDeclDial_cb ( puObject *ob )
{
  bodies[curr_body] -> setDeclination ( ob -> getFloatValue () * SGD_DEGREES_TO_RADIANS ) ;
  ob -> setLegend ( ob -> getStringValue () ) ;
}

static void cloudSelectBox_cb ( puObject *ob )
{
  curr_cloud = ((puSelectBox *) ob) -> getCurrentItem () ;

  if ( curr_cloud < 0 )
    curr_cloud = 0 ;

  if ( curr_cloud >= MAX_CLOUDS )
    curr_cloud = MAX_CLOUDS - 1 ;

  cloudElevationSlider -> setValue ( clouds[curr_cloud]->getElevation() ) ;
  cloudEnableButton    -> setValue ( clouds[curr_cloud]->isEnabled() ) ;
}

static void cloudElevationSlider_cb ( puObject *ob )
{
  clouds[curr_cloud] -> setElevation ( ob -> getFloatValue () ) ;
  ob -> setLegend ( ob -> getStringValue () ) ;
}

static void cloudEnableButton_cb ( puObject *ob )
{
  if ( ob -> getIntegerValue () )
    clouds[curr_cloud] -> enable() ;
  else
    clouds[curr_cloud] -> disable() ;
}

static void update_motion ()
{
  static int frameno = 0 ;
  static ulClock ck ;

  frameno++ ;

  ck . update () ;

  double t = ck . getAbsTime   () ;
  float dt = ck . getDeltaTime () ;

  /* update camera

     keyboard controls:
	 - left/right             : turn left/right
	 - up/down arrow          : look up/down
	 - up/down arrow + shift  : increase/decrease altitude
	 - a/z                    : move forward/backward

     todo: add mouse or joystick support
  */

  if ( special_key == GLUT_KEY_UP )
  {
	if ( mods & GLUT_ACTIVE_SHIFT )
      campos.xyz[SG_Z] += TRANS_SPEED ;
	else
      campos.hpr[1] += ROT_SPEED ;
  }
  else if ( special_key == GLUT_KEY_DOWN )
  {
	if ( mods & GLUT_ACTIVE_SHIFT )
      campos.xyz[SG_Z] -= TRANS_SPEED ;
	else
      campos.hpr[1] -= ROT_SPEED ;
  }
  else if ( special_key == GLUT_KEY_LEFT )
    campos.hpr[0] += ROT_SPEED ;
  else if ( special_key == GLUT_KEY_RIGHT )
    campos.hpr[0] -= ROT_SPEED ;

  if ( keypress == 'a' || keypress == 'A' )
  {
    campos.xyz[SG_X] -= sin ( campos.hpr[0] * SG_DEGREES_TO_RADIANS ) * TRANS_SPEED ;
    campos.xyz[SG_Y] += cos ( campos.hpr[0] * SG_DEGREES_TO_RADIANS ) * TRANS_SPEED ;
  }
  else if ( keypress == 'z' || keypress == 'Z' )
  {
    campos.xyz[SG_X] += sin ( campos.hpr[0] * SG_DEGREES_TO_RADIANS ) * TRANS_SPEED ;
    campos.xyz[SG_Y] -= cos ( campos.hpr[0] * SG_DEGREES_TO_RADIANS ) * TRANS_SPEED ;
  }

  ssgSetCamera ( & campos ) ;

  /* update teapot */

  sgCoord teapotpos ;
  sgSetCoord ( & teapotpos, -280.0, -8.0, 3.0, frameno/5.0, 0.0, 0.0 ) ;
  teapot -> setTransform ( & teapotpos ) ;

  /* update waves */

  sgCoord wavepos ;
  sgSetCoord ( & wavepos, 0, 0, 0, 0, 0, 0 ) ;
  wave     -> setTransform ( & wavepos ) ;
  wave_obj -> updateAnimation ( t ) ;

  /* move heaven & earth ...
     if you wish to place sun, moon, planets & stars correctly then
     you will need to use SimGear Ephemeris code */
  //bodies[0] -> setDeclination ( bodies[0] -> getDeclination() - 0.01*SGD_DEGREES_TO_RADIANS );
  //bodies[1] -> setDeclination ( bodies[1] -> getDeclination() - 0.01*SGD_DEGREES_TO_RADIANS );

  /* update sky */

  sky -> repositionFlat ( campos.xyz, 0, dt );
  sky -> modifyVisibility ( campos.xyz[SG_Z], dt );

  double sol_angle = bodies[0]->getAngle();
  double sky_brightness = (1.0 + cos(sol_angle))/2.0; // 0.0 - 1.0
  double scene_brightness = pow(sky_brightness,0.5);

  /* set sky color */
  sky_color[0] = base_sky_color[0] * sky_brightness;
  sky_color[1] = base_sky_color[1] * sky_brightness;
  sky_color[2] = base_sky_color[2] * sky_brightness;
  sky_color[3] = base_sky_color[3];

  /* set cloud and fog color */
  cloud_color[0] = fog_color[0] = base_fog_color[0] * sky_brightness;
  cloud_color[1] = fog_color[1] = base_fog_color[1] * sky_brightness;
  cloud_color[2] = fog_color[2] = base_fog_color[2] * sky_brightness;
  cloud_color[3] = fog_color[3] = base_fog_color[3];

  /* repaint the sky */
  sky -> repaint ( sky_color, fog_color, cloud_color, sol_angle, nplanets, planet_data, nstars, star_data );

  /* set light source */
  sgCoord solpos;
  bodies[0] -> getPosition ( & solpos );
  ssgGetLight ( 0 ) -> setPosition ( solpos.xyz ) ;

  scene_ambient[0] = base_ambient[0] * scene_brightness;
  scene_ambient[1] = base_ambient[1] * scene_brightness;
  scene_ambient[2] = base_ambient[2] * scene_brightness;
  scene_ambient[3] = 1.0;

  scene_diffuse[0] = base_diffuse[0] * scene_brightness;
  scene_diffuse[1] = base_diffuse[1] * scene_brightness;
  scene_diffuse[2] = base_diffuse[2] * scene_brightness;
  scene_diffuse[3] = 1.0;

  scene_specular[0] = base_specular[0] * scene_brightness;
  scene_specular[1] = base_specular[1] * scene_brightness;
  scene_specular[2] = base_specular[2] * scene_brightness;
  scene_specular[3] = 1.0;
  // GL_LIGHT_MODEL_AMBIENT has a default non-zero value so if
  // we only update GL_AMBIENT for our lights we will never get
  // a completely dark scene.  So, we set GL_LIGHT_MODEL_AMBIENT
  // explicitely to black.
  glLightModelfv( GL_LIGHT_MODEL_AMBIENT, black );
  ssgGetLight( 0 ) -> setColour( GL_AMBIENT, scene_ambient );
  ssgGetLight( 0 ) -> setColour( GL_DIFFUSE, scene_diffuse );
  ssgGetLight( 0 ) -> setColour( GL_SPECULAR, scene_specular );
}



/*
  The GLUT window reshape event
*/

static void reshape ( int w, int h )
{
  glViewport ( 0, 0, w, h ) ;
}


/*
  The GLUT keyboard/mouse events
*/

static void keyboard ( unsigned char key, int x, int y )
{
  keypress = key;

  if ( key == 'f' || key == 'F' )
    /* fullscreen */
    glutFullScreen();
  else if ( key == 'w' || key == 'W' )
    /* wireframe */
    wireframe = TRUE ;
  else if ( key == 's' || key == 'S' )
    /* fill */
    wireframe = FALSE ;
  else if ( key == ' ' )
    /* show/hide gui */
    displayGUI = ! displayGUI ;
  else if ( key == 'q' || key == 'Q' || key == 27 /* esc */ )
    /* quit */
    exit ( 0 ) ;
}

static void keyboard_up ( unsigned char key, int x, int y )
{
  keypress = 0;
}

static void special ( int key, int x, int y )
{
  mods = glutGetModifiers();
  special_key = key;
}

static void special_up ( int key, int x, int y )
{
  mods = glutGetModifiers();
  if ( special_key == key )
    special_key = 0;
}

static void motionfn ( int x, int y )
{
  if ( displayGUI )
    puMouse ( x, y ) ;
}

static void mousefn ( int button, int updown, int x, int y )
{
  if ( displayGUI )
    puMouse ( button, updown, x, y ) ;
}

/*
  The GLUT redraw event
*/

static void redraw ()
{
  update_motion () ;

  glClearColor ( fog_color[0], fog_color[1], fog_color[2], fog_color[3] ) ;

  glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ) ;

  if ( wireframe )
    glPolygonMode ( GL_FRONT_AND_BACK, GL_LINE ) ;
  else
    glPolygonMode ( GL_FRONT_AND_BACK, GL_FILL ) ;

  /* Adjust fog based on visibility (when in clouds) */
  GLfloat fog_exp2_density = sqrt_m_log01 / sky->getVisibility();
  glEnable( GL_FOG );
  glFogf  ( GL_FOG_DENSITY, fog_exp2_density);
  glFogi  ( GL_FOG_MODE,    GL_EXP2 );
  glFogfv ( GL_FOG_COLOR,   fog_color );

  /* Draw scene */

  // we need a white diffuse light for the phase of the moon
  ssgGetLight( 0 ) -> setColour( GL_DIFFUSE, white );

  sky -> preDraw  ( );

  // return to the desired diffuse color
  ssgGetLight( 0 ) -> setColour( GL_DIFFUSE, scene_diffuse );

  ssgCullAndDraw  ( scene ) ;
  sky -> postDraw ( campos.xyz[SG_Z] ); /* altitude */

  /* Draw gui */

  glPolygonMode ( GL_FRONT_AND_BACK, GL_FILL ) ;
  if ( displayGUI )
    puDisplay () ;

  glutPostRedisplay () ;
  glutSwapBuffers () ;
}



static void init_graphics ()
{
  int   fake_argc = 1 ;
  char *fake_argv[3] ;
  fake_argv[0] = "ssgExample" ;
  fake_argv[1] = "Simple Scene Graph : Example Program." ;
  fake_argv[2] = NULL ;

  /* Initialise GLUT */

  glutInitWindowPosition ( 0, 0 ) ;
  glutInitWindowSize     ( 640, 480 ) ;
  glutInit               ( &fake_argc, fake_argv ) ;
  glutInitDisplayMode    ( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH ) ;
  glutCreateWindow       ( fake_argv[1] ) ;
  glutDisplayFunc        ( redraw       ) ;
  glutReshapeFunc        ( reshape      ) ;
  glutKeyboardFunc       ( keyboard     ) ;
  glutKeyboardUpFunc     ( keyboard_up  ) ;
  glutSpecialFunc        ( special      ) ;
  glutSpecialUpFunc      ( special_up   ) ;
  glutMouseFunc          ( mousefn      ) ;
  glutMotionFunc         ( motionfn     ) ;
 
  /* Initialise SSG */

  puInit  () ;
  ssgInit () ;

  /* Some basic OpenGL setup */

  glEnable ( GL_DEPTH_TEST ) ;

  /* Set up the viewing parameters */

  ssgSetFOV     ( 60, 45 ) ;
  ssgSetNearFar ( 1, 200000 ) ; /* nb. skydome is scalable ... so far can be reduced */

  /* Set up the Sun */

  sgVec3 solposn ;
  sgSetVec3 ( solposn, 0, 0, 0 ) ;
  ssgGetLight ( 0 ) -> setPosition ( solposn ) ;
}


/*
  Load a simple database
*/

static void load_database ()
{
  /* Set up the path to the data files */

  ssgModelPath   ( "data" ) ;
  ssgTexturePath ( "data" ) ;

  /* Setup teapot state */

  teapot_state = new ssgSimpleState () ;
  teapot_state -> setTexture        ( "data/candy.rgb" ) ;
  teapot_state -> enable            ( GL_TEXTURE_2D ) ;
  teapot_state -> setShadeModel     ( GL_SMOOTH ) ;
  teapot_state -> enable            ( GL_CULL_FACE ) ;
  teapot_state -> enable            ( GL_BLEND ) ;
  teapot_state -> enable            ( GL_LIGHTING ) ;
  teapot_state -> setColourMaterial ( GL_AMBIENT_AND_DIFFUSE ) ;
  teapot_state -> setMaterial       ( GL_EMISSION, 0, 0, 0, 1 ) ;
  teapot_state -> setMaterial       ( GL_SPECULAR, 1, 1, 1, 1 ) ;
  teapot_state -> setShininess      ( 2 ) ;

  /* Setup wave state */

  wave_state = new ssgSimpleState () ;
  wave_state -> setTexture        ( "data/ocean.rgb" ) ;
  wave_state -> enable            ( GL_TEXTURE_2D ) ;
  wave_state -> setShadeModel     ( GL_SMOOTH ) ;
  wave_state -> enable            ( GL_CULL_FACE ) ;
  wave_state -> enable            ( GL_BLEND ) ;
  wave_state -> enable            ( GL_LIGHTING ) ;
  wave_state -> setColourMaterial ( GL_AMBIENT_AND_DIFFUSE ) ;
  wave_state -> setMaterial       ( GL_EMISSION, 0, 0, 0, 1 ) ;
  wave_state -> setMaterial       ( GL_SPECULAR, 1, 1, 1, 1 ) ;
  wave_state -> setShininess      ( 5 ) ;

  wave_train.setSpeed      ( 1.0f  ) ;
  wave_train.setLength     ( 3.0f  ) ;
  wave_train.setLambda     ( 0.5f  ) ;
  wave_train.setHeading    ( 30.0f ) ;
  wave_train.setWaveHeight ( 1.5f  ) ;

  /* Create object */

  tpt_obj =  new ssgaTeapot ( 1000 ) ;
  tpt_obj -> setSize        ( 2 ) ;
  tpt_obj -> setKidState    ( teapot_state ) ;
  tpt_obj -> regenerate     () ;

  sgVec3 pos = {0, 0, 0};

  wave_obj = new ssgaWaveSystem ( 4000 ) ;
  wave_obj->setColour           ( white ) ;
  wave_obj->setSize             ( 80000 ) ;  /* todo: fix this ... prefer size 2000,
                                                but wave system has recently changed and
                                                no longer stretches on for 'infinity' :( */
  wave_obj->setTexScale         ( 32, 32 ) ;
  wave_obj->setCenter           ( pos ) ;
  wave_obj->setKidState         ( wave_state ) ;
  wave_obj->setWindSpeed        ( 10.0f ) ;
  wave_obj->setWaveTrain        ( 0, &wave_train ) ;

  /* Build the scene graph */

  star_data = new sgdVec3[nstars] ;
  for ( int i = 0; i < nstars; i++ )
  {
    star_data[i][0] = ssgaRandom() * SGD_PI ;
	star_data[i][1] = ssgaRandom() * SGD_PI ;
	star_data[i][2] = ssgaRandom() ;
  }

  /* sky */
  sky       =  new ssgaSky ;
  sky       -> build ( 80000, 80000, nplanets, planet_data, nstars, star_data );
  /* sun */
  bodies[0] =  sky -> addBody (
                  NULL,                 // body texture
				  "data/halo.rgba",     // halo texture
				  5000,                 // size
				  80000,                // distance
				  true );               // is sun - dome painted based on this
  bodies[0] -> setDeclination  ( 20*SGD_DEGREES_TO_RADIANS );
  /* moon */
  bodies[1] =  sky -> addBody (
                  "data/moon.rgba",     // body texture
				  NULL,                 // halo texture
				  5000,                 // size
				  80000 );              // distance
  bodies[1] -> setDeclination  ( 65*SGD_DEGREES_TO_RADIANS );
  /* clouds */
  clouds[0] =  sky -> addCloud (
				 "data/scattered.rgba", // texture
                 80000,                 // span
				 2000,                  // elevation,
				 100,                   // thickness
				 100 );                 // transition
  clouds[0] -> setSpeed ( 50 ) ;
  clouds[0] -> setDirection ( 45 ) ;
  clouds[1] =  sky -> addCloud (
				 "data/scattered.rgba", // texture
                 80000,                 // span
				 3000,                  // elevation,
				 100,                   // thickness
				 100 );                 // transition
  clouds[1] -> setSpeed ( 20 ) ;
  clouds[1] -> setDirection ( 30 ) ;
  clouds[2] =  sky -> addCloud (
				 "data/scattered.rgba", // texture
                 80000,                 // span
				 1000,                  // elevation,
				 100,                   // thickness
				 100 );                 // transition
  clouds[2] -> setSpeed ( 5 ) ;
  clouds[2] -> setDirection ( 20 ) ;

  teapot    =  new ssgTransform ;
  teapot    -> addKid ( tpt_obj ) ;

  wave      =  new ssgTransform ;
  wave      -> addKid ( wave_obj ) ;

  scene     =  new ssgRoot ;
  scene     -> addKid ( teapot ) ;
  scene     -> addKid ( wave   ) ;
}


static void init_gui ()
{
  static puFont     *sorority ;
  static fntTexFont *fnt      ;

  fnt      = new fntTexFont () ;
  fnt     -> load ( "data/sorority.txf" ) ;
  sorority = new puFont ( fnt, 12 ) ;

  puSetDefaultFonts        ( *sorority, *sorority ) ;
  puSetDefaultStyle        ( PUSTYLE_SMALL_SHADED ) ;
  puSetDefaultColourScheme ( 0.2f, 0.5f, 0.2f, 0.5f ) ;

  puGroup *window_group = new puGroup ( 0, 0 ) ;

  bodyRADial = new puDial    ( 150, GUI_BASE+90, 50 ) ;
  bodyRADial->setWrap        ( 1 ) ;
  bodyRADial->setMaxValue    ( 360 ) ;
  bodyRADial->setMinValue    ( 0 ) ;
  bodyRADial->setStepSize    ( 0.1 ) ;
  bodyRADial->setCBMode      ( PUSLIDER_ALWAYS ) ;
  bodyRADial->setCallback    ( bodyRADial_cb ) ;
  bodyRADial->setLabel       ( "Right Ascension" ) ;
  bodyRADial->setLabelPlace  ( PUPLACE_CENTERED_RIGHT ) ;
  bodyRADial->setColour( PUCOL_LABEL, FONT_COLOUR ) ;
  bodyRADial->setLegendPlace ( PUPLACE_BOTTOM_CENTERED ) ;

  bodyDeclDial = new puDial    ( 400, GUI_BASE+90, 50 ) ;
  bodyDeclDial->setWrap        ( 1 ) ;
  bodyDeclDial->setMaxValue    ( 360 ) ;
  bodyDeclDial->setMinValue    ( 0 ) ;
  bodyDeclDial->setStepSize    ( 0.1 ) ;
  bodyDeclDial->setCBMode      ( PUSLIDER_ALWAYS ) ;
  bodyDeclDial->setCallback    ( bodyDeclDial_cb ) ;
  bodyDeclDial->setLabel       ( "Declination" ) ;
  bodyDeclDial->setLabelPlace  ( PUPLACE_CENTERED_RIGHT ) ;
  bodyDeclDial->setColour( PUCOL_LABEL, FONT_COLOUR ) ;
  bodyDeclDial->setLegendPlace ( PUPLACE_BOTTOM_CENTERED ) ;
  
  bodySelectBox = new puSelectBox ( 150, GUI_BASE+150, 300, GUI_BASE+180, bodyNameList ) ;
  bodySelectBox->setCallback      ( bodySelectBox_cb ) ;
  bodySelectBox->setCurrentItem   ( 0 ) ;
  bodySelectBox->setLabel         ( "Edit Body" ) ;
  bodySelectBox->setLabelPlace    ( PUPLACE_CENTERED_LEFT ) ;
  bodySelectBox->setColour( PUCOL_LABEL, FONT_COLOUR ) ;
  
  /* Set everything up on the first time around */
  bodySelectBox_cb ( bodySelectBox ) ;

  cloudElevationSlider = new puSlider ( 150, GUI_BASE+20, 90, false, 20 ) ;
  cloudElevationSlider->setMaxValue   ( 10000.0f ) ;
  cloudElevationSlider->setMinValue   (   100.0f ) ;
  cloudElevationSlider->setStepSize   (    10.0f ) ;
  cloudElevationSlider->setCBMode     ( PUSLIDER_ALWAYS ) ;
  cloudElevationSlider->setCallback   ( cloudElevationSlider_cb ) ;
  cloudElevationSlider->setLabel      ( "Cloud Elevation" ) ;
  cloudElevationSlider->setLabelPlace ( PUPLACE_CENTERED_LEFT ) ;
  cloudElevationSlider->setColour( PUCOL_LABEL, FONT_COLOUR ) ;

  cloudEnableButton = new puButton ( 150, GUI_BASE, " " ) ;
  cloudEnableButton->setStyle      ( PUSTYLE_RADIO ) ;
  cloudEnableButton->setCallback   ( cloudEnableButton_cb ) ;
  cloudEnableButton->setLabel      ( "Enable Cloud" ) ;
  cloudEnableButton->setLabelPlace ( PUPLACE_CENTERED_LEFT ) ;
  cloudEnableButton->setColour( PUCOL_LABEL, FONT_COLOUR ) ;
  
  cloudSelectBox = new puSelectBox ( 150, GUI_BASE+50, 300, GUI_BASE+80, cloudNameList ) ;
  cloudSelectBox->setCallback      ( cloudSelectBox_cb ) ;
  cloudSelectBox->setCurrentItem   ( 0 ) ;
  cloudSelectBox->setLabel         ( "Edit Cloud" ) ;
  cloudSelectBox->setLabelPlace    ( PUPLACE_CENTERED_LEFT ) ;
  cloudSelectBox->setColour( PUCOL_LABEL, FONT_COLOUR ) ;
 
  /* Set everything up on the first time around */
  cloudSelectBox_cb ( cloudSelectBox ) ;

  window_group->close () ;
}


/*
  The works.
*/

int main ( int, char ** )
{
  init_graphics () ;
  load_database () ;
  init_gui      () ;
  glutMainLoop  () ;
  return 0 ;
}
