#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
#include <math.h>
#include <plib/ssg.h>
#include <plib/js.h>
#include <GL/glut.h>

#define TILE_SIZE                 2000.0f      /* cubits */
#define LAMBDA                   (TILE_SIZE/48.0f)
#define TILE_GRID_SIZE            20          /* Even number please! */
#define TRIANGLE_GRID_SIZE        12          /* Num vertices */
#define ELEVATION_SCALE           4.0f
#define ONLINE_TERRAIN_RANGE ((float)(TILE_GRID_SIZE/2  )* TILE_SIZE ) /* cubits */
#define VISUAL_RANGE         ((float)(TILE_GRID_SIZE/2-1)* TILE_SIZE ) /* cubits */

/******* expose the private ssgSGIHeader interface from ssgImageLoader ******/

/* Some magic constants in the file header. */

#define SGI_IMG_MAGIC           0x01DA
#define SGI_IMG_SWABBED_MAGIC   0xDA01   /* This is how it appears on a PC */
#define SGI_IMG_VERBATIM        0
#define SGI_IMG_RLE             1

class ssgSGIHeader
{
public:    /* Yuk!  Need to hide some of this public stuff! */
  unsigned short magic ;
  int            max ;
  int            min ;
  int            colormap ;
  char           type ;
  char           bpp ;
  unsigned int  *start ;
  int           *leng ;
  unsigned short dim ;
  unsigned short xsize ;
  unsigned short ysize ;
  unsigned short zsize ;
  int           tablen ;

  ssgSGIHeader () ;
  void makeConsistant () ;
  void getRow   ( unsigned char *buf, int y, int z ) ;
  void getPlane ( unsigned char *buf, int z ) ;
  void getImage ( unsigned char *buf ) ;
  void readHeader () ;
} ;

/***** *****/

ssgSimpleState *state    = NULL ;
ssgRoot        *scene    = NULL ;
ssgTransform   *penguin  = NULL ;
ssgTransform   *terrain  = NULL ;
ssgTransform   *tilegrid [ TILE_GRID_SIZE ][ TILE_GRID_SIZE ] ;

ssgSGIHeader *material  ;
ssgSGIHeader *elevation ;

#include "elevation_map.h"
#include "image_map.h"

jsJoystick *js ;
float      *ax ;
sgCoord tuxpos = { { 0.0f,0.0f,0.0f }, { 0.0f,0.0f,0.0f } } ;

#define ROT_SPEED 5.0f
#define TRANS_SPEED 10.0f

void update_motion ()
{
  static int frameno = 0 ;

  frameno++ ;

  sgCoord campos ;

  int buttons ;

  js -> read ( &buttons, ax ) ;

  tuxpos.hpr[0] -= ax [ 0 ] * ROT_SPEED ;
  tuxpos.xyz[0] -= sin ( tuxpos.hpr[0] * SG_DEGREES_TO_RADIANS ) * ax[1] * TRANS_SPEED ;
  tuxpos.xyz[1] += cos ( tuxpos.hpr[0] * SG_DEGREES_TO_RADIANS ) * ax[1] * TRANS_SPEED ;

  sgVec3 test_vec ;
  sgMat4 invmat ;
  sgMakeIdentMat4 ( invmat ) ;

  invmat[3][0] = -tuxpos.xyz[0] ;
  invmat[3][1] = -tuxpos.xyz[1] ;
  invmat[3][2] =  0.0f          ;

  test_vec [0] = 0.0f ;
  test_vec [1] = 0.0f ;
  test_vec [2] = 100000.0f ;

  ssgHit *results ;
  int num_hits = ssgHOT ( scene, test_vec, invmat, &results ) ;

  float hot = -1000000.0f ;

  for ( int i = 0 ; i < num_hits ; i++ )
  {
    ssgHit *h = &results [ i ] ;

    float hgt = - h->plane[3] / h->plane[2] ;

    if ( hgt >= hot )
      hot = hgt ;
  }

  tuxpos . xyz [ 2 ] = hot + 0.1f ;
  tuxpos . hpr [ 1 ] = 0.0f ;
  tuxpos . hpr [ 2 ] = 0.0f ;

  sgCopyVec3 ( campos.xyz, tuxpos.xyz ) ;

  campos . hpr [ 0 ] = 0.0f ;
  campos . hpr [ 1 ] = -20.0f ;
  campos . hpr [ 2 ] = 0.0f ;
  campos . xyz [ 0 ] +=  1.0f ;
  campos . xyz [ 1 ] -= 20.0f ;
  campos . xyz [ 2 ] += 12.0f ;

  ssgSetCamera ( & campos ) ;
  penguin -> setTransform ( & tuxpos ) ;
}



/*
  The GLUT window reshape event
*/

void reshape ( int w, int h )
{
  glViewport ( 0, 0, w, h ) ;
}



/*
  The GLUT keyboard event
*/

void keyboard ( unsigned char k, int, int )
{
  static int wireframe = FALSE ;

  if ( k == 'w' )
  {
    wireframe = ! wireframe ;

    glPolygonMode ( GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL ) ;
  }

  if ( k == 0x03 || k == 'x' )
    exit ( 0 ) ;
}



/*
  The GLUT redraw event
*/

void redraw ()
{
  glutWarpPointer ( 320, 240 ) ;
  update_motion () ;

  glClear  ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ) ;

  ssgCullAndDraw ( scene ) ;

  glutPostRedisplay () ;
  glutSwapBuffers () ;
}



void init_graphics ()
{
  int   fake_argc = 1 ;
  char *fake_argv[3] ;
  fake_argv[0] = "MajikTerrain" ;
  fake_argv[1] = "Majik Terrain Demo - by Steve Baker" ;
  fake_argv[2] = NULL ;

  js = new jsJoystick ( 0 ) ;

  if ( js -> notWorking () || js -> getNumAxes () < 2 )
  {
    fprintf ( stderr,
       "Majik_Demo: Joystick with at least two axes required.\n" ) ;
    exit ( 0 ) ;
  }

  ax = new float [ js->getNumAxes () ] ;

  /*
    Initialise GLUT
  */

  glutInitWindowPosition ( 0, 0 ) ;
  glutInitWindowSize     ( 640, 480 ) ;
  glutInit               ( &fake_argc, fake_argv ) ;
  glutInitDisplayMode    ( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH ) ;
  glutCreateWindow       ( fake_argv[1] ) ;
  glutDisplayFunc        ( redraw   ) ;
  glutReshapeFunc        ( reshape  ) ;
  glutKeyboardFunc       ( keyboard ) ;
 
  /*
    Initialise SSG
  */

  ssgInit () ;

  /*
    Some basic OpenGL setup
  */

  sgVec4 skycol ; sgSetVec4 ( skycol, 0.4f, 0.7f, 1.0f, 1.0f ) ;
  sgVec4 fogcol ; sgSetVec4 ( fogcol, 0.4f, 0.7f, 1.0f, 1.0f ) ;

  glClearColor ( skycol[0], skycol[1], skycol[2], skycol[3] ) ;

  glEnable ( GL_DEPTH_TEST ) ;

  /*
    Set up the viewing parameters
  */

  ssgSetFOV     ( 60.0f, 0.0f ) ;
  ssgSetNearFar ( 1.0f, 30000.0f ) ;

  /*
    Initial Position 
  */

  sgCoord startpos ;
  sgSetCoord ( &startpos, 2350.0f, -920.0f, 0.0f,   0.0f, 0.0f, 0.0f ) ;

  /*
    Set up some fog
  */

  glFogf ( GL_FOG_DENSITY, 0.015 / 100.0f ) ;
  glFogfv( GL_FOG_COLOR  , fogcol    ) ;
  glFogf ( GL_FOG_START  , 0.0       ) ;
  glFogi ( GL_FOG_MODE   , GL_EXP2   ) ;
  glHint ( GL_FOG_HINT   , GL_NICEST ) ;
  glEnable ( GL_FOG ) ;
 
  /*
    Set up the Sun.
  */

  sgVec3 sunposn ;
  sgVec4 sunamb  ;
  sgSetVec3 ( sunposn, 0.2f, -0.5f, 0.5f ) ;
  sgSetVec4 ( sunamb , 0.4f, 0.4f, 0.4f, 1.0f ) ;
  ssgGetLight ( 0 ) -> setPosition ( sunposn ) ;
  ssgGetLight ( 0 ) -> setColour ( GL_AMBIENT, sunamb  ) ;
}


ssgBranch *createTileLOD ( int x, int y, ssgSimpleState *state, int ntris, float z_off ) 
{
  sgVec4 *scolors = new sgVec4 [ (ntris+1) * (ntris+1) ] ;
  sgVec2 *tcoords = new sgVec2 [ (ntris+1) * (ntris+1) ] ;
  sgVec3 *snorms  = new sgVec3 [ (ntris+1) * (ntris+1) ] ;
  sgVec3 *scoords = new sgVec3 [ (ntris+1) * (ntris+1) ] ;

  for ( int j = 0 ; j < (ntris+1) ; j++ )
    for ( int i = 0 ; i < (ntris+1) ; i++ )
    {
      int address  =  (int)floor( 12.0f * ((float)x + (float)i / (float)ntris)      ) % 256  +
                     ((int)floor( 12.0f * ((float)y + (float)j / (float)ntris)      ) % 256 ) * 256 ;
      int addressN =  (int)floor( 12.0f * ((float)x + (float)i / (float)ntris)      ) % 256  +
                     ((int)floor( 12.0f * ((float)y + (float)j / (float)ntris)+1.0f ) % 256 ) * 256 ;
      int addressE =  (int)floor( 12.0f * ((float)x + (float)i / (float)ntris)+1.0f ) % 256  +
                     ((int)floor( 12.0f * ((float)y + (float)j / (float)ntris)      ) % 256 ) * 256 ;

      float zz  = (float) elevation_map [ address  ] * ELEVATION_SCALE + z_off ;
      float zzN = (float) elevation_map [ addressN ] * ELEVATION_SCALE + z_off ;
      float zzE = (float) elevation_map [ addressE ] * ELEVATION_SCALE + z_off ;

      float rr = (float) image_map [ address * 3 + 0 ] / 255.0f ;
      float gg = (float) image_map [ address * 3 + 1 ] / 255.0f ;
      float bb = (float) image_map [ address * 3 + 2 ] / 255.0f ;

      float xx = (float) i * (TILE_SIZE/(float)ntris) ;
      float yy = (float) j * (TILE_SIZE/(float)ntris) ;

      sgVec3 nrm ;

      nrm [ 0 ] = (zz - zzE) / (TILE_SIZE/12.0f) ;
      nrm [ 1 ] = (zz - zzN) / (TILE_SIZE/12.0f) ;
      nrm [ 2 ] = sqrt ( nrm[0] * nrm[0] + nrm[1] * nrm[1] ) ;

      sgCopyVec3( snorms  [i+j*(ntris+1)], nrm ) ;
      sgSetVec2 ( tcoords [i+j*(ntris+1)], xx/LAMBDA, yy/LAMBDA ) ;
      sgSetVec4 ( scolors [i+j*(ntris+1)], rr, gg, bb, 1.0f ) ;
      sgSetVec3 ( scoords [i+j*(ntris+1)], xx, yy, zz ) ;
    }

  ssgBranch *branch = new ssgBranch () ;

  for ( int i = 0 ; i < ntris ; i++ )
  {
    unsigned short *vlist = new unsigned short [ 2 * (ntris+1) ] ;

    for ( int j = 0 ; j < (ntris+1) ; j++ )
    {
      vlist [   j + j   ] = j + (i+1) * (ntris+1) ;
      vlist [ j + j + 1 ] = j +   i   * (ntris+1) ;
    }

    ssgLeaf *gset = new ssgVTable ( GL_TRIANGLE_STRIP,
                     2 * (ntris+1), vlist, scoords,
                     2 * (ntris+1), vlist, snorms ,
                     2 * (ntris+1), vlist, tcoords,
                     2 * (ntris+1), vlist, scolors ) ;
    gset -> setState ( state ) ;
    branch -> addKid ( gset  ) ;
  }

  return branch ;
}


void createTile ( ssgTransform *tile, int x, int y, ssgSimpleState *state ) 
{
  float rr[] = { 0.0f, 6000.0f, 7000.0f, 12000.0f } ;
  ssgRangeSelector *lod = new ssgRangeSelector () ;

  lod  -> addKid ( createTileLOD ( x, y, state, TRIANGLE_GRID_SIZE   - 1,    0.0f ) ) ;
  lod  -> addKid ( createTileLOD ( x, y, state, TRIANGLE_GRID_SIZE/2 - 1,  -30.0f ) ) ;
  lod  -> addKid ( createTileLOD ( x, y, state, TRIANGLE_GRID_SIZE/4 - 1, -250.0f ) ) ;
  lod  -> setRanges ( rr, 4 ) ;

  tile -> addKid ( lod ) ;
}


/*
  Load a simple database
*/

void load_database ()
{
  /*
    Set up the path to the data files
  */

  ssgModelPath   ( "data" ) ;
  ssgTexturePath ( "data" ) ;

  /*
    Create a root node - and a transform to position
    the terrain - and another to position the player.
  */

  scene    = new ssgRoot      ;
  penguin  = new ssgTransform ;
  terrain  = new ssgTransform ;
  state    = new ssgSimpleState ;
  state -> setTexture ( "data/bumpnoise.rgb" ) ;
  state -> enable     ( GL_TEXTURE_2D ) ;
  state -> enable     ( GL_LIGHTING ) ;
  state -> setShadeModel ( GL_SMOOTH ) ;
  state -> enable ( GL_COLOR_MATERIAL ) ;
  state -> enable ( GL_CULL_FACE      ) ;
  state -> setColourMaterial ( GL_AMBIENT_AND_DIFFUSE ) ;
  state -> setMaterial ( GL_EMISSION, 0, 0, 0, 1 ) ;
  state -> setMaterial ( GL_SPECULAR, 0, 0, 0, 1 ) ;
  state -> setShininess ( 0 ) ;
  state -> setOpaque () ;
  state -> disable ( GL_BLEND ) ;

  /*
    Load the models - optimise them a bit
    and then add them into the scene.
  */

  ssgEntity *tux_obj = ssgLoadAC ( "tuxedo.ac"   ) ;

  penguin  -> addKid ( tux_obj  ) ;
  ssgFlatten         ( tux_obj  ) ;
  ssgStripify        ( penguin  ) ;
  penguin  -> clrTraversalMaskBits ( SSGTRAV_HOT ) ;

  for ( int i = 0 ; i < TILE_GRID_SIZE ; i++ )
    for ( int j = 0 ; j < TILE_GRID_SIZE ; j++ )
    {
      tilegrid [ i ][ j ] = new ssgTransform ;

      terrain -> addKid ( tilegrid [ i ][ j ] ) ;

      sgVec3 tilepos ;
      sgSetVec3 ( tilepos, (float)i * TILE_SIZE - ONLINE_TERRAIN_RANGE,
                           (float)j * TILE_SIZE - ONLINE_TERRAIN_RANGE, 0.0f ) ;

      tilegrid [ i ][ j ] -> setTransform ( tilepos ) ;
      createTile ( tilegrid [ i ][ j ], i, j, state ) ;
    }

  scene -> addKid ( terrain ) ;
  scene -> addKid ( penguin ) ;
}



/*
  The works.
*/

int main ( int, char ** )
{
  init_graphics () ;
  load_database () ;
  glutMainLoop  () ;
  return 0 ;
}

