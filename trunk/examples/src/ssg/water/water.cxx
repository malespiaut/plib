/*
     PLIB - A Suite of Portable Game Libraries
     Copyright (C) 2001  Steve Baker

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published by
     the Free Software Foundation; either version 2 of the License, or
     (at your option) any later version.

     This program is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU General Public License for more details.

     You should have received a copy of the GNU General Public License
     along with this program; if not, write to the Free Software
     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA

     For further information visit http://plib.sourceforge.net

     $Id$
*/

#define PU_USE_PW

#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES
#endif

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#ifdef WIN32
#  include <windows.h>
#else
#  include <unistd.h>
#endif
#include <math.h>
#include <plib/pw.h>
#include <plib/ssg.h>
#include <plib/ssgAux.h>
#include <plib/pu.h>
#include <plib/puAux.h>

#ifndef GL_VERSION_1_3
// ARB_multitexture
#define GL_TEXTURE0      GL_TEXTURE0_ARB
#define GL_TEXTURE1      GL_TEXTURE1_ARB
#define glActiveTexture  glActiveTextureARB
#endif

#define GUI_BASE      90
#define VIEW_GUI_BASE 30
#define FONT_COLOUR   1,1,1,1

static puSlider    *trainLengthSlider  = (puSlider    *) NULL ;
static puSlider    *trainSpeedSlider   = (puSlider    *) NULL ;
static puSlider    *trainLambdaSlider  = (puSlider    *) NULL ;
static puSlider    *trainHeightSlider  = (puSlider    *) NULL ;
static puButton    *trainEnableButton  = (puButton    *) NULL ;
static puOneShot   *trainDisableButton = (puOneShot   *) NULL ;
static puDial      *trainHeadingDial   = (puDial      *) NULL ;
static puaSelectBox *trainSelectBox    = (puaSelectBox *) NULL ;
static puText      *timeText           = (puText      *) NULL ;

static puaSelectBox *depthSelectBox     = (puaSelectBox *) NULL ;

static puDial      *viewHeadingDial    = (puDial      *) NULL ;
static puDial      *viewPitchDial      = (puDial      *) NULL ;
static puSlider    *viewRangeSlider    = (puSlider    *) NULL ;
static puButton    *viewWireframeButton= (puButton    *) NULL ;
static puButton    *viewEnvMapButton   = (puButton    *) NULL ;

static puOneShot   *writeButton        = (puOneShot   *) NULL ;

static puSlider    *waveTextureSlider  = (puSlider    *) NULL ;
static puSlider    *waveSizeSlider     = (puSlider    *) NULL ;
static puSlider    *wavePolycountSlider= (puSlider    *) NULL ;

static ssgRoot            *scene        = NULL ;
static ssgaLensFlare      *sun_obj      = NULL ;
static ssgTransform       *sun          = NULL ;
static ssgTransform       *teapot       = NULL ;
static ssgTransform       *fire         = NULL ;
static ssgTransform       *pedestal     = NULL ;
static ssgaWaveSystem     *ocean        = NULL ;
static ssgaParticleSystem *fountain     = NULL ;
static ssgaCube           *ped_obj      = NULL ;
static ssgaTeapot         *tpt_obj      = NULL ;
static ssgaFire           *fire_obj     = NULL ;

static ssgSimpleState     *sea_state    = NULL ;
static ssgSimpleState     *splash_state = NULL ;
static ssgSimpleState     *teapot_state = NULL ;
static ssgSimpleState     *plinth_state = NULL ;

static ssgContext         *context      = NULL ;

static ssgaWaveTrain trains [ 16 ] ;

static char *trainNameList[] =
{
  "Train 0" , "Train 1" , "Train 2" , "Train 3" , "Train 4" ,
  "Train 5" , "Train 6" , "Train 7" , "Train 8" , "Train 9" ,
  "Train 10", "Train 11", "Train 12", "Train 13", "Train 14",
  "Train 15",
  NULL
} ;

static int   quit = FALSE;

static int   curr_train = 0 ;
static int   curr_depthfunc = 0 ;
static int   wireframe  = FALSE ;
static int   displayGUI = TRUE  ;
static float cam_range  = 25.0f ;

static sgCoord campos = { { 0, -20, 8 }, { 0, -30, 0 } } ;
static sgVec3  sunpos = { 400, 300, 50 } ;

static int enableTexGen ( ssgEntity * )
{
#ifdef GL_ARB_multitexture
  int tx ;
  glGetIntegerv ( GL_TEXTURE_BINDING_2D, &tx ) ;
  glActiveTexture ( GL_TEXTURE1 ) ;
#endif
  glTexGeni ( GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP ) ;
  glTexGeni ( GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP ) ;
  glEnable ( GL_TEXTURE_GEN_S ) ;
  glEnable ( GL_TEXTURE_GEN_T ) ;
#ifdef GL_ARB_multitexture
  glEnable ( GL_TEXTURE_2D ) ;  /* Enables the second texture map. */
  glBindTexture ( GL_TEXTURE_2D, tx ) ;
  glActiveTexture ( GL_TEXTURE0 ) ;
#endif    
  return TRUE ;
} 

static int disableTexGen ( ssgEntity * )
{
#ifdef GL_ARB_multitexture
  glActiveTexture ( GL_TEXTURE1 ) ;
#endif
  glDisable ( GL_TEXTURE_GEN_S ) ;
  glDisable ( GL_TEXTURE_GEN_T ) ;
#ifdef GL_ARB_multitexture
  glDisable ( GL_TEXTURE_2D ) ; /* Disables the second texture map */
  glActiveTexture ( GL_TEXTURE0 ) ;
#endif
  return TRUE ;
}
 
static void writeCplusplusCode ()
{
  if ( viewEnvMapButton -> getIntegerValue () )
  {
    printf ( "\n" ) ;
    printf ( "static int enableTexGen ( ssgEntity * )\n" ) ;
    printf ( "{\n" ) ;
    printf ( "#ifdef GL_ARB_multitexture\n" ) ;
    printf ( "  int tx ;\n" ) ;
    printf ( "  glGetIntegerv ( GL_TEXTURE_BINDING_2D, &tx ) ;\n" ) ;
    printf ( "  glActiveTextureARB ( GL_TEXTURE1_ARB ) ;\n" ) ;
    printf ( "#endif\n" ) ;
    printf ( "  glTexGeni ( GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP ) ;\n" ) ;
    printf ( "  glTexGeni ( GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP ) ;\n" ) ;
    printf ( "  glEnable ( GL_TEXTURE_GEN_S ) ;\n" ) ;
    printf ( "  glEnable ( GL_TEXTURE_GEN_T ) ;\n" ) ;
    printf ( "  glEnable ( GL_TEXTURE_2D ) ;\n" ) ;
    printf ( "#ifdef GL_ARB_multitexture\n" ) ;
    printf ( "  glBindTexture ( GL_TEXTURE_2D, tx ) ;\n" ) ;
    printf ( "  glActiveTextureARB ( GL_TEXTURE0_ARB ) ;\n" ) ;
    printf ( "#endif\n" ) ;
    printf ( "  return TRUE ;\n" ) ;
    printf ( "} \n" ) ;
    printf ( "\n" ) ;
    printf ( "static int disableTexGen ( ssgEntity * )\n" ) ;
    printf ( "{\n" ) ;
    printf ( "#ifdef GL_ARB_multitexture\n" ) ;
    printf ( "  glActiveTextureARB ( GL_TEXTURE1_ARB ) ;\n" ) ;
    printf ( "#endif\n" ) ;
    printf ( "  glDisable ( GL_TEXTURE_GEN_S ) ;\n" ) ;
    printf ( "  glDisable ( GL_TEXTURE_GEN_T ) ;\n" ) ;
    printf ( "  glDisable ( GL_TEXTURE_2D ) ;\n" ) ;
    printf ( "#ifdef GL_ARB_multitexture\n" ) ;
    printf ( "  glActiveTextureARB ( GL_TEXTURE0_ARB ) ;\n" ) ;
    printf ( "#endif\n" ) ;
    printf ( "  return TRUE ;\n" ) ;
    printf ( "}\n" ) ;
    printf ( "\n" ) ;
  }
 
  printf ( "\n" ) ;
  printf ( "ssgaWaveSystem *makeWaveSystem ( ssgState *state )\n" ) ;
  printf ( "{\n" ) ;
  printf ( "  ssgaWaveTrain  *wavetrain ;\n" ) ;
  printf ( "  ssgaWaveSystem *ocean ;\n" ) ;
  printf ( "  ocean =  new ssgaWaveSystem ( %d ) ;\n", ocean->getNumTris() ) ;
  printf ( "  ocean -> setSize            ( %g ) ;\n", ocean->getSize()[0] ) ;
  printf ( "  ocean -> setKidState        ( state ) ;\n" ) ;
  printf ( "  ocean -> setTexScale      ( %g, %g ) ;\n",
                                                     ocean->getTexScaleU (),
                                                     ocean->getTexScaleV () ) ;
  printf ( "  ocean -> setWindSpeed     ( 10.0f ) ;\n\n" ) ;

  for ( int i = 0 ; i < SSGA_MAX_WAVETRAIN ; i++ )
  {
    if ( ocean -> getWaveTrain ( i ) != NULL )
    {
      printf ( "  wavetrain = new ssgaWaveTrain () ;\n" ) ;

      printf ( "  wavetrain->setSpeed      ( %g ) ;\n",trains[i].getSpeed ());
      printf ( "  wavetrain->setLength     ( %g ) ;\n",trains[i].getLength ());
      printf ( "  wavetrain->setLambda     ( %g ) ;\n",trains[i].getLambda ());
      printf ( "  wavetrain->setHeading    ( %g ) ;\n",trains[i].getHeading ());
      printf ( "  wavetrain->setWaveHeight ( %g ) ;\n",trains[i].getWaveHeight());

      printf ( "  ocean -> setWaveTrain ( %d, wavetrain ) ;\n\n", i ) ;
    }
  }

  if ( viewEnvMapButton -> getIntegerValue () )
  {
    printf ( "  ocean   -> setKidCallback ( SSG_CALLBACK_PREDRAW ,  enableTexGen ) ;\n" ) ;
    printf ( "  ocean   -> setKidCallback ( SSG_CALLBACK_POSTDRAW, disableTexGen ) ;\n" ) ;
  }
  printf ( "  ocean -> regenerate () ;\n" ) ;
  printf ( "  return ocean ;\n" ) ;
  printf ( "}\n" ) ;
  printf ( "\n" ) ;
}


static void waveTextureSlider_cb ( puObject *ob )
{
  ocean   -> setTexScale ( ob -> getFloatValue (),
                           ob -> getFloatValue () ) ;
}


static void waveSizeSlider_cb ( puObject *ob )
{
  cam_range = ob -> getFloatValue () ;
  ocean   -> setSize ( ob -> getFloatValue () ) ;
}


static void wavePolycountSlider_cb ( puObject *ob )
{
  ocean -> setNumTris ( ob -> getIntegerValue () ) ;
}


static void viewHeadingDial_cb ( puObject *ob )
{
  campos.hpr[0] = ob -> getFloatValue () ;

  sgVec3 r = { 0, -cam_range, 0 } ;
  sgMat4 mat ;

  sgMakeRotMat4 ( mat, campos.hpr ) ;
  sgXformVec3 ( campos.xyz, r, mat ) ;
}


static void viewPitchDial_cb ( puObject *ob )
{
  campos . hpr [ 1 ] = ob -> getFloatValue () ;

  sgVec3 r = { 0, -cam_range, 0 } ;
  sgMat4 mat ;

  sgMakeRotMat4 ( mat, campos.hpr ) ;
  sgXformVec3 ( campos.xyz, r, mat ) ;
}


static void viewRangeSlider_cb ( puObject *ob )
{
  cam_range = ob -> getFloatValue () ;

  sgVec3 r = { 0, -cam_range, 0 } ;
  sgMat4 mat ;

  sgMakeRotMat4 ( mat, campos.hpr ) ;
  sgXformVec3 ( campos.xyz, r, mat ) ;
}


static void viewWireframeButton_cb ( puObject *ob )
{
  wireframe = ob -> getIntegerValue () ;
}


static void viewEnvMapButton_cb ( puObject *ob )
{
  if ( ob -> getIntegerValue () )
  {
    ocean   -> setKidCallback   ( SSG_CALLBACK_PREDRAW , enableTexGen ) ;
    ocean   -> setKidCallback   ( SSG_CALLBACK_POSTDRAW, disableTexGen ) ;
    ocean   -> regenerate       () ;
  }
  else
  {
    ocean   -> setKidCallback   ( SSG_CALLBACK_PREDRAW , NULL ) ;
    ocean   -> setKidCallback   ( SSG_CALLBACK_POSTDRAW, NULL ) ;
    ocean   -> regenerate       () ;
  }
}


static void trainLengthSlider_cb ( puObject *ob )
{
  trains[curr_train].setLength ( ob -> getFloatValue () ) ;
  ob -> setLegend ( ob -> getStringValue () ) ;
}


static void trainSpeedSlider_cb ( puObject *ob )
{
  trains[curr_train].setSpeed ( ob -> getFloatValue () ) ;
  ob -> setLegend ( ob -> getStringValue () ) ;
}


static void trainLambdaSlider_cb ( puObject *ob )
{
  trains[curr_train].setLambda ( ob -> getFloatValue () ) ;
  ob -> setLegend ( ob -> getStringValue () ) ;
}


static void trainHeightSlider_cb ( puObject *ob )
{
  trains[curr_train].setWaveHeight ( ob -> getFloatValue () ) ;
  ob -> setLegend ( ob -> getStringValue () ) ;
}

static void trainDisableButton_cb ( puObject *ob )
{
  for ( int i = 0 ; i < SSGA_MAX_WAVETRAIN ; i++ )
    ocean -> setWaveTrain ( i, NULL ) ;

  trainEnableButton -> setValue ( 0 ) ;
}

static void writeButton_cb ( puObject *ob )
{
  if ( ob -> getIntegerValue () )
    writeCplusplusCode () ;
}

static void trainEnableButton_cb ( puObject *ob )
{
  if ( ob -> getIntegerValue () )
    ocean -> setWaveTrain ( curr_train, & trains [ curr_train ] ) ;
  else
    ocean -> setWaveTrain ( curr_train, NULL ) ;
}

static void trainHeadingDial_cb ( puObject *ob )
{
  trains[curr_train].setHeading ( ob -> getFloatValue () ) ;
  ob -> setLegend ( ob -> getStringValue () ) ;
}

static void trainSelectBox_cb ( puObject *ob )
{
  curr_train = ((puaSelectBox *) ob) -> getCurrentItem () ;

  if ( curr_train < 0 )
    curr_train = 0 ;

  if ( curr_train >= SSGA_MAX_WAVETRAIN )
    curr_train = SSGA_MAX_WAVETRAIN - 1 ;

  trainEnableButton -> setValue (
                              ocean -> getWaveTrain ( curr_train ) != NULL ) ;

  trainLengthSlider -> setValue ( trains[curr_train].getLength     () ) ;
  trainSpeedSlider  -> setValue ( trains[curr_train].getSpeed      () ) ;
  trainLambdaSlider -> setValue ( trains[curr_train].getLambda     () ) ;
  trainHeightSlider -> setValue ( trains[curr_train].getWaveHeight () ) ;
  trainHeadingDial  -> setValue ( trains[curr_train].getHeading    () ) ;
}


static float halfMeterEverywhere ( float x, float y )
{
  return 0.5f ;
}

 
static float oneMeterEverywhere ( float x, float y )
{
  return 1.0f ;
}

 
static float twoMeterEverywhere ( float x, float y )
{
  return 1.5f ;
}

 
static float gentleSlope ( float x, float y )
{
  return (1.0f + x / (ocean -> getSize ()[0] / 2.0f)) * 2.0f ;
}


static float steepSlope ( float x, float y )
{
  return (1.0f + x / (ocean -> getSize ()[0] / 2.0f)) * 10.0f ;
}


static float stepFunction ( float x, float y )
{
  return (x < 0.0f ) ? 0.5f : 20000.0f ;
}


static float twoBeaches ( float x, float y )
{
  return (float) fabs ( sin(       x / ocean->getSize()[0]) *
                        sin(2.0f * y / ocean->getSize()[1]) * 1.5 + 0.5 ) ;
}

static const ssgaWSDepthCallback depthFuncs [] =
{
  NULL,   /* Infinite depth */
  halfMeterEverywhere,
  oneMeterEverywhere,
  twoMeterEverywhere,
  gentleSlope,
  steepSlope,
  stepFunction,
  twoBeaches,
  NULL,
} ;

static char *depthNames [] =
{
  "Infinite Depth",
  "Half Meter Deep",
  "One Meter Deep",
  "Two Meters Deep",
  "Gentle Slope",
  "Steep Slope",
  "Step Function",
  "Two Curved Beaches",
  NULL,
} ;

static void depthSelectBox_cb ( puObject *ob )
{
  curr_depthfunc = ((puaSelectBox *) ob) -> getCurrentItem () ;

  if ( curr_depthfunc < 0 )
    curr_depthfunc = 0 ;

  if ( curr_depthfunc >= (int)(sizeof(depthFuncs)/sizeof(ssgaWSDepthCallback)))
    curr_depthfunc = (int)(sizeof(depthFuncs)/sizeof(ssgaWSDepthCallback)) - 1 ;

  ocean -> setDepthCallback ( depthFuncs [ curr_depthfunc ] ) ;
}



static void update_motion ()
{
  static ulClock ck ;
  static char s [ 128 ] ;

  ck . update () ;

  double t = ck . getAbsTime   () ;
  float dt = (float) ck . getDeltaTime () ;

  ocean -> setWindDirn ( (float)( 25.0 * sin ( t / 100.0 ) ) ) ;
  ocean -> updateAnimation ( (float) t ) ;

  fountain -> update ( dt ) ;
  fire_obj -> update ( dt ) ;

  dt = (float) ck . getDeltaTime () ;

  sprintf ( s, "CalcTime=%1.1fms", dt * 1000.0 ) ;
  timeText->setLabel ( s ) ;
  sgCoord tptpos ;

  context -> setCamera ( & campos ) ;

  sgSetCoord ( & tptpos, 0.0f,  0.0f, 0.6f, (float)(t * 60), 0.0f, 0.0f ) ;
  teapot  -> setTransform ( & tptpos ) ;
  sgSetCoord ( & tptpos, 0.0f,  0.0f, 3.0f, 0.0f, 0.0f, 0.0f ) ;
  fire    -> setTransform ( & tptpos ) ;
}



/*
  The PW window reshape event
*/

static void reshape ( int w, int h )
{
  glViewport ( 0, 0, w, h ) ;
}



/*
  The PW keyboard/mouse events
*/

static void keyboard ( int key, int updown, int, int )
{
  if ( ! puKeyboard ( key, updown ) && ( updown == PU_DOWN ) )
  {
    switch ( key )
    {
      case 0x03 :
      case 0x1B :
      case 'q'  :
        exit ( 0 ) ;

      case ' ' :
      default  : 
        displayGUI = ! displayGUI ; 
        break ;
    }
  }
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
  The redraw function
*/

static void main_loop ()
{
  while ( !quit )
  {
    update_motion () ;
  
    glClear  ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ) ;
  
    if ( wireframe )
      glPolygonMode ( GL_FRONT_AND_BACK, GL_LINE ) ;
    else
      glPolygonMode ( GL_FRONT_AND_BACK, GL_FILL ) ;
  
    ssgCullAndDraw ( scene ) ;
  
    glPolygonMode ( GL_FRONT_AND_BACK, GL_FILL ) ;
  
    if ( displayGUI )
      puDisplay () ;
    
    pwSwapBuffers () ;
  }
  
  pwCleanup () ;
}



static void init_graphics ()
{
  /*
    Initialise PW
  */
  pwInit ( 50, 50, 640, 480, FALSE, "Simple Scene Graph : Water Example Program.", TRUE, 0) ;
  pwSetCallbacks ( keyboard, mousefn, motionfn, reshape ) ;

  puInit  () ;
  ssgInit () ;

  /*
    Some basic OpenGL setup
  */

  glClearColor ( 0.2f, 0.7f, 1.0f, 1.0f ) ;
  glEnable ( GL_DEPTH_TEST ) ;

  /*
    Set up the viewing parameters
  */
  context = new ssgContext () ;
  context -> setFOV     ( 60.0f, 0.0f   ) ;
  context -> setNearFar (  1.0f, 700.0f ) ;
  context -> makeCurrent () ;

  /*
    Set up the Sun.
  */

  ssgGetLight ( 0 ) -> setPosition ( sunpos ) ;
}


/*
  Particle system definitions for the fountain.
*/

#define SPS ssgaParticleSystem  /* Too much typing! */                          

static void droplet_create ( SPS *, int, ssgaParticle *p )
{
  float c = ((float)(rand()%100)/100.0f) * (256.0f-163.0f)/255.0f ;

  sgSetVec4 ( p -> col, 96.0f/255.0f+c, 147.0f/255.0f+c, 163.0f/255.0f+c, 0.5);
  sgSetVec3 ( p -> pos, -2.4f, -0.1f, 1.9f ) ;
  sgSetVec3 ( p -> vel, 
             -(float)(rand()%1000)/200.0f,
              (float)(rand()%1000 - 500)/400.0f,
              (float)(rand()%1000)/1000.0f + 3.0f ) ;
  sgAddScaledVec3 ( p -> pos, p -> vel, (float)(rand()%1000)/20000.0f ) ;
  sgSetVec3 ( p -> acc, 0, 0, -9.8f ) ;
  p -> time_to_live = 1 ;
}


static void init_states ()
{
  plinth_state = new ssgSimpleState () ;
  plinth_state -> setTexture        ( "data/pavement.rgb" ) ;
  plinth_state -> enable            ( GL_TEXTURE_2D ) ;
  plinth_state -> setShadeModel     ( GL_SMOOTH ) ;
  plinth_state -> enable            ( GL_CULL_FACE ) ;
  plinth_state -> enable            ( GL_BLEND ) ;
  plinth_state -> enable            ( GL_LIGHTING ) ;
  plinth_state -> setColourMaterial ( GL_AMBIENT_AND_DIFFUSE ) ;
  plinth_state -> setMaterial       ( GL_EMISSION, 0, 0, 0, 1 ) ;
  plinth_state -> setMaterial       ( GL_SPECULAR, 1, 1, 1, 1 ) ;
  plinth_state -> setShininess      ( 2 ) ;

  teapot_state = new ssgSimpleState () ;
  teapot_state -> setTexture        ( "data/pattern.rgb" ) ;
  teapot_state -> enable            ( GL_TEXTURE_2D ) ;
  teapot_state -> setShadeModel     ( GL_SMOOTH ) ;
  teapot_state -> enable            ( GL_CULL_FACE ) ;
  teapot_state -> enable            ( GL_BLEND ) ;
  teapot_state -> enable            ( GL_LIGHTING ) ;
  teapot_state -> setColourMaterial ( GL_AMBIENT_AND_DIFFUSE ) ;
  teapot_state -> setMaterial       ( GL_EMISSION, 0, 0, 0, 1 ) ;
  teapot_state -> setMaterial       ( GL_SPECULAR, 1, 1, 1, 1 ) ;
  teapot_state -> setShininess      ( 2 ) ;

  sea_state = new ssgSimpleState () ;
  sea_state -> setTexture        ( "data/ocean.rgb" ) ;
  sea_state -> setTranslucent    () ;
  sea_state -> enable            ( GL_TEXTURE_2D ) ;
  sea_state -> setShadeModel     ( GL_SMOOTH ) ;
  sea_state -> enable            ( GL_CULL_FACE ) ;
  sea_state -> enable            ( GL_BLEND ) ;
  sea_state -> enable            ( GL_LIGHTING ) ;
  sea_state -> setColourMaterial ( GL_AMBIENT_AND_DIFFUSE ) ;
  sea_state -> setMaterial       ( GL_EMISSION, 0, 0, 0, 1 ) ;
  sea_state -> setMaterial       ( GL_SPECULAR, 1, 1, 1, 1 ) ;
  sea_state -> setShininess      (  5 ) ;

  splash_state = new ssgSimpleState () ;
  splash_state -> setTexture        ( "data/droplet.rgb" ) ;
  splash_state -> setTranslucent    () ;
  splash_state -> enable            ( GL_TEXTURE_2D ) ;
  splash_state -> setShadeModel     ( GL_SMOOTH ) ;
  splash_state -> enable            ( GL_CULL_FACE ) ;
  splash_state -> enable            ( GL_BLEND ) ;
  splash_state -> enable            ( GL_LIGHTING ) ;
  splash_state -> setColourMaterial ( GL_EMISSION ) ;
  splash_state -> setMaterial       ( GL_AMBIENT, 0, 0, 0, 1 ) ;
  splash_state -> setMaterial       ( GL_DIFFUSE, 0, 0, 0, 1 ) ;
  splash_state -> setMaterial       ( GL_SPECULAR, 0, 0, 0, 1 ) ;
  splash_state -> setShininess      (  0 ) ;
}




static void load_database ()
{
  /* Set up the path to the data files */

  ssgModelPath   ( "data" ) ;
  ssgTexturePath ( "data" ) ;

  /* Load the states */

  init_states () ;

  sgVec4  TRANSLUCENT_WHITE  = { 1.0f, 1.0f, 1.0f, 0.8f } ;
  sgVec3  pos    = { 0, 0, 0 } ;
  sgCoord pedpos = { { 0, 0, -1.5 }, { 0, 0, 0 } } ;

  /* Create a the scene content.  */

  fountain = new ssgaParticleSystem ( 1000, 100, 500, TRUE,
                                      0.2f, 1000,
                                      droplet_create ) ;
  fountain -> setState ( splash_state ) ;

  /* Set up some interesting defaults. */

  trains[0] . setSpeed      (  3.1f ) ;
  trains[0] . setLength     (  0.8f ) ;
  trains[0] . setLambda     (  0.6f ) ;
  trains[0] . setHeading    ( 47.0f ) ;
  trains[0] . setWaveHeight (  0.2f ) ;

  trains[1] . setSpeed      (  4.6f ) ;
  trains[1] . setLength     (  0.8f ) ;
  trains[1] . setLambda     (  1.0f ) ;
  trains[1] . setHeading    ( 36.0f ) ;
  trains[1] . setWaveHeight (  0.1f ) ;

  trains[2] . setSpeed      (  8.5f ) ;
  trains[2] . setLength     (  0.6f ) ;
  trains[2] . setLambda     (  1.0f ) ;
  trains[2] . setHeading    ( 65.0f ) ;
  trains[2] . setWaveHeight (  0.1f ) ;

  ocean   =  new ssgaWaveSystem ( 10000 ) ;
  ocean   -> setColour        ( TRANSLUCENT_WHITE ) ;
  ocean   -> setSize          ( 100 ) ;
  ocean   -> setTexScale      ( 3, 3 ) ;
  ocean   -> setCenter        ( pos ) ;
  ocean   -> setDepthCallback ( depthFuncs [ curr_depthfunc ] ) ;
  ocean   -> setKidState      ( sea_state ) ;
  ocean   -> setWindSpeed     ( 10.0f ) ;
  ocean   -> setWaveTrain     ( 0, & trains[0] ) ;
  ocean   -> setWaveTrain     ( 1, & trains[1] ) ;
  ocean   -> setWaveTrain     ( 2, & trains[2] ) ;

  ped_obj =  new ssgaCube     () ;
  ped_obj -> setSize          ( 4 ) ;
  ped_obj -> setKidState      ( plinth_state ) ;
  ped_obj -> regenerate       () ;

  tpt_obj =  new ssgaTeapot   ( 1000 ) ;
  tpt_obj -> setSize          ( 2 ) ;
  tpt_obj -> setKidState      ( teapot_state ) ;
  tpt_obj -> regenerate       () ;

  fire_obj =  new ssgaFire ( 200 ) ;

  /* Build the scene graph */

  teapot   =  new ssgTransform ;
  teapot   -> addKid          ( tpt_obj  ) ;
  teapot   -> addKid          ( fountain ) ;

  pedestal =  new ssgTransform ;
  pedestal -> setTransform    ( & pedpos ) ;
  pedestal -> addKid          ( ped_obj  ) ;

  sun_obj  = new ssgaLensFlare () ;

  sun      = new ssgTransform ;
  sun      -> setTransform    ( sunpos ) ;
  sun      -> addKid          ( sun_obj  ) ;

  fire     =  new ssgTransform ;
  fire     -> addKid          ( fire_obj  ) ;

  scene    =  new ssgRoot ;
  scene    -> addKid          ( ocean    ) ;
  scene    -> addKid          ( fire     ) ;
  scene    -> addKid          ( pedestal ) ;
  scene    -> addKid          ( teapot   ) ;
  scene    -> addKid          ( sun      ) ;
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

  trainLengthSlider = new puSlider ( 200, GUI_BASE+28, 90, false, 20 ) ;
  trainLengthSlider->setMaxValue   ( 20.0f ) ;
  trainLengthSlider->setMinValue   (  0.1f ) ;
  trainLengthSlider->setStepSize   (  0.1f ) ;
  trainLengthSlider->setCBMode     ( PUSLIDER_ALWAYS ) ;
  trainLengthSlider->setCallback   ( trainLengthSlider_cb ) ;
  trainLengthSlider->setLabel      ( "Wave Length" ) ;
  trainLengthSlider->setLabelPlace ( PUPLACE_CENTERED_LEFT ) ;
  trainLengthSlider->setColour( PUCOL_LABEL, FONT_COLOUR ) ;
  
  trainSpeedSlider = new puSlider  ( 200, GUI_BASE+56, 90, false, 20 ) ;
  trainSpeedSlider->setMaxValue    ( 20.0f ) ;
  trainSpeedSlider->setMinValue    ( 0.0f ) ;
  trainSpeedSlider->setStepSize    ( 0.1f ) ;
  trainSpeedSlider->setCBMode      ( PUSLIDER_ALWAYS ) ;
  trainSpeedSlider->setCallback    ( trainSpeedSlider_cb ) ;
  trainSpeedSlider->setLabel       ( "Wave Speed" ) ;
  trainSpeedSlider->setLabelPlace  ( PUPLACE_CENTERED_LEFT ) ;
  trainSpeedSlider->setColour( PUCOL_LABEL, FONT_COLOUR ) ;
  
  trainLambdaSlider = new puSlider ( 300, GUI_BASE+28, 90, false, 20 ) ;
  trainLambdaSlider->setMaxValue   ( 2.0f ) ;
  trainLambdaSlider->setMinValue   ( 0.0f ) ;
  trainLambdaSlider->setStepSize   ( 0.1f ) ;
  trainLambdaSlider->setCBMode     ( PUSLIDER_ALWAYS ) ;
  trainLambdaSlider->setCallback   ( trainLambdaSlider_cb ) ;
  trainLambdaSlider->setLabel      ( "Wave Curl" ) ;
  trainLambdaSlider->setLabelPlace ( PUPLACE_CENTERED_RIGHT ) ;
  trainLambdaSlider->setColour( PUCOL_LABEL, FONT_COLOUR ) ;
  
  trainHeightSlider = new puSlider ( 300, GUI_BASE+56, 90, false, 20 ) ;
  trainHeightSlider->setMaxValue   ( 5.0f ) ;
  trainHeightSlider->setMinValue   ( 0.0f ) ;
  trainHeightSlider->setStepSize   ( 0.1f ) ;
  trainHeightSlider->setCBMode     ( PUSLIDER_ALWAYS ) ;
  trainHeightSlider->setCallback   ( trainHeightSlider_cb ) ;
  trainHeightSlider->setLabel      ( "Wave Height (meters)" ) ;
  trainHeightSlider->setLabelPlace ( PUPLACE_CENTERED_RIGHT ) ;
  trainHeightSlider->setColour( PUCOL_LABEL, FONT_COLOUR ) ;
  
  trainEnableButton = new puButton ( 200, GUI_BASE+84, " " ) ;
  trainEnableButton->setStyle      ( PUSTYLE_RADIO ) ;
  trainEnableButton->setCallback   ( trainEnableButton_cb ) ;
  trainEnableButton->setLabel      ( "Enable this Wave Train" ) ;
  trainEnableButton->setLabelPlace ( PUPLACE_CENTERED_LEFT ) ;
  trainEnableButton->setColour( PUCOL_LABEL, FONT_COLOUR ) ;
  
  trainDisableButton = new puOneShot( 450, GUI_BASE+120,
                                                "Disable All WaveTrains" ) ;
  trainDisableButton->setCallback  ( trainDisableButton_cb ) ;
  
  trainHeadingDial = new puDial    ( 300, GUI_BASE+82, 50 ) ;
  trainHeadingDial->setWrap        ( 1 ) ;
  trainHeadingDial->setMaxValue    ( 360 ) ;
  trainHeadingDial->setMinValue    ( 0 ) ;
  trainHeadingDial->setStepSize    ( 1 ) ;
  trainHeadingDial->setCBMode      ( PUSLIDER_ALWAYS ) ;
  trainHeadingDial->setCallback    ( trainHeadingDial_cb ) ;
  trainHeadingDial->setLabel       ( "Wave Direction" ) ;
  trainHeadingDial->setLabelPlace  ( PUPLACE_CENTERED_RIGHT ) ;
  trainHeadingDial->setColour( PUCOL_LABEL, FONT_COLOUR ) ;
  trainHeadingDial->setLegendPlace ( PUPLACE_BOTTOM_CENTERED ) ;
  
  trainSelectBox = new puaSelectBox ( 200, GUI_BASE+109, 300, GUI_BASE+139,
                                     trainNameList ) ;
  trainSelectBox->setCallback      ( trainSelectBox_cb ) ;
  trainSelectBox->setCurrentItem   ( 0 ) ;
  trainSelectBox->setLabel         ( "Edit Wave Train Number" ) ;
  trainSelectBox->setLabelPlace    ( PUPLACE_CENTERED_LEFT ) ;
  trainSelectBox->setColour( PUCOL_LABEL, FONT_COLOUR ) ;
  
  /* Set everything up on the first time around */
  trainSelectBox_cb ( trainSelectBox ) ;

  depthSelectBox = new puaSelectBox ( 200, GUI_BASE, 400, GUI_BASE+20,
                                     depthNames ) ;
  depthSelectBox->setCallback      ( depthSelectBox_cb ) ;
  depthSelectBox->setCurrentItem   ( 0 ) ;
  depthSelectBox->setLabel         ( "Water Depth" ) ;
  depthSelectBox->setLabelPlace    ( PUPLACE_CENTERED_LEFT ) ;
  depthSelectBox->setColour( PUCOL_LABEL, FONT_COLOUR ) ;
  
  /* Set everything up on the first time around */
  depthSelectBox_cb ( depthSelectBox ) ;

  viewHeadingDial = new puDial (  50, VIEW_GUI_BASE, 50 ) ;
  viewHeadingDial->setValue       ( 0.0f ) ;
  viewHeadingDial->setWrap        ( 1 ) ;
  viewHeadingDial->setMaxValue    ( 360 ) ;
  viewHeadingDial->setMinValue    ( 0 ) ;
  viewHeadingDial->setStepSize    ( 0 ) ;
  viewHeadingDial->setCBMode      ( PUSLIDER_ALWAYS ) ;
  viewHeadingDial->setCallback    ( viewHeadingDial_cb ) ;
  viewHeadingDial->setLabel       ( "Pan" ) ;
  viewHeadingDial->setLabelPlace  ( PUPLACE_BOTTOM_CENTERED ) ;
  viewHeadingDial->setColour( PUCOL_LABEL, FONT_COLOUR ) ;

  viewPitchDial  = new puDial ( 100, VIEW_GUI_BASE, 50 ) ;
  viewPitchDial  ->setValue       ( -45.0f ) ;
  viewPitchDial  ->setWrap        ( 1 ) ;
  viewPitchDial  ->setMaxValue    ( 360 ) ;
  viewPitchDial  ->setMinValue    ( 0 ) ;
  viewPitchDial  ->setStepSize    ( 0 ) ;
  viewPitchDial  ->setCBMode      ( PUSLIDER_ALWAYS ) ;
  viewPitchDial  ->setCallback    ( viewPitchDial_cb ) ;
  viewPitchDial  ->setLabel       ( "Tilt" ) ;
  viewPitchDial  ->setLabelPlace  ( PUPLACE_BOTTOM_CENTERED ) ;
  viewPitchDial  ->setColour( PUCOL_LABEL, FONT_COLOUR ) ;

  viewRangeSlider = new puSlider ( 150, VIEW_GUI_BASE, 90, false, 20 ) ;
  viewRangeSlider->setValue      ( 25.0f ) ;
  viewRangeSlider->setMaxValue   ( 150.0f ) ;
  viewRangeSlider->setMinValue   ( 0.0f ) ;
  viewRangeSlider->setStepSize   ( 0 ) ;
  viewRangeSlider->setCBMode     ( PUSLIDER_ALWAYS ) ;
  viewRangeSlider->setCallback   ( viewRangeSlider_cb ) ;
  viewRangeSlider->setLabel      ( "Range" ) ;
  viewRangeSlider->setLabelPlace ( PUPLACE_BOTTOM_CENTERED ) ;
  viewRangeSlider->setColour( PUCOL_LABEL, FONT_COLOUR ) ;
  
  viewWireframeButton= new puButton ( 400, VIEW_GUI_BASE, "Wireframe" ) ;
  viewWireframeButton->setCallback  ( viewWireframeButton_cb ) ;
  viewWireframeButton->setValue     ( FALSE ) ;

  viewEnvMapButton= new puButton ( 300, VIEW_GUI_BASE, "Env.Map" ) ;
  viewEnvMapButton->setCallback  ( viewEnvMapButton_cb ) ;
  viewEnvMapButton->setValue     ( FALSE ) ;

  writeButton = new puOneShot ( 400, VIEW_GUI_BASE + 30, "Write C++" ) ;
  writeButton->setCallback  ( writeButton_cb ) ;
  writeButton->setValue     ( FALSE ) ;

  waveTextureSlider = new puSlider ( 500, VIEW_GUI_BASE   , 90, false, 20 ) ;
  waveTextureSlider->setValue      ( 1.0f ) ;
  waveTextureSlider->setMaxValue   ( 50.0f ) ;
  waveTextureSlider->setMinValue   ( 0.01f ) ;
  waveTextureSlider->setStepSize   ( 0 ) ;
  waveTextureSlider->setCBMode     ( PUSLIDER_ALWAYS ) ;
  waveTextureSlider->setCallback   ( waveTextureSlider_cb ) ;
  waveTextureSlider->setLabel      ( "Texture" ) ;
  waveTextureSlider->setLabelPlace ( PUPLACE_CENTERED_RIGHT ) ;
  waveTextureSlider->setColour( PUCOL_LABEL, FONT_COLOUR ) ;
  
  waveSizeSlider = new puSlider ( 500, VIEW_GUI_BASE+30, 90, false, 20 ) ;
  waveSizeSlider->setValue      ( 25.0f ) ;
  waveSizeSlider->setMaxValue   ( 500.0f ) ;
  waveSizeSlider->setMinValue   (  10.0f ) ;
  waveSizeSlider->setStepSize   ( 0 ) ;
  waveSizeSlider->setCBMode     ( PUSLIDER_ALWAYS ) ;
  waveSizeSlider->setCallback   ( waveSizeSlider_cb ) ;
  waveSizeSlider->setLabel      ( "Size" ) ;
  waveSizeSlider->setLabelPlace ( PUPLACE_CENTERED_RIGHT ) ;
  waveSizeSlider->setColour( PUCOL_LABEL, FONT_COLOUR ) ;

  wavePolycountSlider= new puSlider ( 500, VIEW_GUI_BASE+60, 90, false, 20 ) ;
  wavePolycountSlider->setValue      ( 10000 ) ;
  wavePolycountSlider->setMaxValue   ( 50000 ) ;
  wavePolycountSlider->setMinValue   (  4000 ) ;
  wavePolycountSlider->setStepSize   ( 0 ) ;
  wavePolycountSlider->setCBMode     ( PUSLIDER_ALWAYS ) ;
  wavePolycountSlider->setCallback   ( wavePolycountSlider_cb ) ;
  wavePolycountSlider->setLabel      ( "Polygons" ) ;
  wavePolycountSlider->setLabelPlace ( PUPLACE_CENTERED_RIGHT ) ;
  wavePolycountSlider->setColour( PUCOL_LABEL, FONT_COLOUR ) ;
  
  timeText = new puText ( 500, VIEW_GUI_BASE+80 ) ;
  timeText->setColour( PUCOL_LABEL, FONT_COLOUR ) ;
  
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
  main_loop     () ;
  return 0 ;
}

