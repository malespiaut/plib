#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#ifdef WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#else
#  include <unistd.h>
#endif
#include <math.h>
#include <plib/ssg.h>
#include <plib/pu.h>
#include <plib/ssgAux.h>

#ifdef FREEGLUT_IS_PRESENT
#  include <GL/freeglut.h>
#else
#  ifdef __APPLE__
#    include <GLUT/glut.h>
#  else
#    include <GL/glut.h>
#  endif
#endif

#include <ode/ode.h>


// some constants

#define NUM 20			// max number of objects
#define DENSITY (5.0)		// density of all objects


// dynamics and collision objects

struct MyObject {
  dBodyID body;			// the body
  dGeomID geom;     // geometry representing this body
  ssgTransform* transform;
};

static int num=0;		// number of objects in simulation
static int nextobj=0;		// next object to recycle if num==NUM
static dWorldID world;
static dSpaceID space;
static MyObject obj[NUM];
static dJointGroupID contactgroup;


static ssgRoot  *scene    = NULL ;

#define GUI_BASE      80
#define VIEW_GUI_BASE 20
#define FONT_COLOUR   1,1,1,1

static puDial      *viewHeadingDial    = (puDial      *) NULL ;
static puDial      *viewPitchDial      = (puDial      *) NULL ;
static puSlider    *viewRangeSlider    = (puSlider    *) NULL ;
static puButton    *viewWireframeButton= (puButton    *) NULL ;

static int   wireframe  = FALSE ;
static float cam_range  = 5.0f ;
static sgCoord campos = { { 0, -5, 1 }, { 0, 0, 0 } } ;


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
  viewRangeSlider->setValue      ( 5.0f ) ;
  viewRangeSlider->setMaxValue   ( 20.0f ) ;
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

  window_group->close () ;
}

ssgaShape* create_box (float lx, float ly, float lz, float r, float g, float b)
{
  sgVec3 size = {0.0f, 0.0f, 0.0f};
  sgVec4 colour = {0.0f, 0.0f, 0.0f, 1.f};
  sgVec4 spec = {0.1f, 0.1f, 0.1f, 0.f};

  size[0] = lx;
  size[1] = ly;
  size[2] = lz;

  colour[0] = r;
  colour[1] = g;
  colour[2] = b;

  ssgSimpleState *st = new ssgSimpleState ;
  st -> disable ( GL_TEXTURE_2D ) ;
  st -> disable ( GL_ALPHA_TEST ) ;
  st -> disable ( GL_BLEND ) ;
  st -> setOpaque () ;
  st -> setMaterial ( GL_AMBIENT, colour ) ;
  st -> setMaterial ( GL_DIFFUSE, colour ) ;
  st -> setMaterial ( GL_SPECULAR, spec ) ;
  st -> setShininess ( 0.1f );
  st -> disable ( GL_COLOR_MATERIAL ) ;
  st -> enable  ( GL_LIGHTING ) ;
  st -> setShadeModel ( GL_SMOOTH ) ;
  
  ssgaShape* shape = new ssgaCube (100) ;
  shape -> setColour (colour) ;
  shape -> setSize (size) ;
  shape -> setKidState (st) ;
  return shape ;
}

ssgaShape* create_sphere (float radius, float r, float g, float b)
{
  sgVec4 colour = {0.0f, 0.0f, 0.0f, 1.f};
  sgVec4 spec = {0.1f, 0.1f, 0.1f, 0.f};

  colour[0] = r;
  colour[1] = g;
  colour[2] = b;

  ssgSimpleState *st = new ssgSimpleState ;
  st -> disable ( GL_TEXTURE_2D ) ;
  st -> disable ( GL_ALPHA_TEST ) ;
  st -> disable ( GL_BLEND ) ;
  st -> setOpaque () ;
  st -> setMaterial ( GL_AMBIENT, colour ) ;
  st -> setMaterial ( GL_DIFFUSE, colour ) ;
  st -> setMaterial ( GL_SPECULAR, spec ) ;
  st -> setShininess ( 0.1f );
  st -> disable ( GL_COLOR_MATERIAL ) ;
  st -> enable  ( GL_LIGHTING ) ;
  st -> setShadeModel ( GL_SMOOTH ) ;
  
  ssgaShape* shape = new ssgaSphere (100) ;
  shape -> setColour (colour) ;
  shape -> setSize (radius * 2.f) ;
  shape -> setKidState (st) ;
  return shape ;
}

void sync_transform (ssgTransform* transform, const dGeomID geom)
{
  const dReal *pos = dGeomGetPosition (geom) ;
  const dReal *R = dGeomGetRotation (geom) ;

  sgMat4 matrix;
  matrix[0][0]=R[0];
  matrix[0][1]=R[4];
  matrix[0][2]=R[8];
  matrix[0][3]=0.f;

  matrix[1][0]=R[1];
  matrix[1][1]=R[5];
  matrix[1][2]=R[9];
  matrix[1][3]=0.f;

  matrix[2][0]=R[2];
  matrix[2][1]=R[6];
  matrix[2][2]=R[10];
  matrix[2][3]=0.f;

  matrix[3][0]=pos[0];
  matrix[3][1]=pos[1];
  matrix[3][2]=pos[2];
  matrix[3][3]=1.f;

  transform -> setTransform (matrix) ;
}

/*
Do the physics movement and collision
*/

// this is called by dSpaceCollide when two objects in space are
// potentially colliding.

static void nearCallback (void *data, dGeomID o1, dGeomID o2)
{
  // if (o1->body && o2->body) return;
  
  // exit without doing anything if the two bodies are connected by a joint
  dBodyID b1 = dGeomGetBody(o1);
  dBodyID b2 = dGeomGetBody(o2);
  if (b1 && b2 && dAreConnected (b1,b2)) return;
  
  enum { MAX_CONTACTS = 64 };
  static dContact contact[MAX_CONTACTS];			// up to MAX_CONTACTS contacts per box
  for (int i=0; i<MAX_CONTACTS; i++) {
    contact[i].surface.mode = dContactBounce; //dContactMu2;
    contact[i].surface.mu = dInfinity;
    contact[i].surface.mu2 = 0;
    contact[i].surface.bounce = 0.5f;
    contact[i].surface.bounce_vel = 0.1f;
  }
  if (int numc = dCollide (o1,o2,MAX_CONTACTS,&contact[0].geom,sizeof(dContact))) {
    for (int i=0; i<numc; i++) {
      dJointID c = dJointCreateContact (world,contactgroup,contact+i);
      dJointAttach (c,b1,b2);
    }
  }
}

static void keyboard ( unsigned char cmd, int, int );

static void update_motion ()
{
  static ulClock ck ;
  ck . update () ;

  double t = ck . getAbsTime   () ;

  static double drop_time = 0.f;
  if (t >= drop_time)
  {
    float dt = 0.25f + dRandReal() * 1.f;
    drop_time = t + dt;

    int cmd = 'b';
    if (dRandReal() > 0.5f)
      cmd = 's';
    keyboard(cmd, 0, 0);
  }

  static double step_time = 0.f;
  if (t >= step_time)
  {
    // note: dWorldStep() is very sensitive to step values
    // just keep it a constant
    const float dt = 1.f/60;
    
    step_time += dt;
    if (t >= step_time)  // too slow?
      step_time = t + dt;
    
    dSpaceCollide (space,0,&nearCallback);
    dWorldStep (world,dt);
    
    // remove all contact joints
    dJointGroupEmpty (contactgroup);
    
    for (int i=0; i<num; ++i)
    {
      if (obj[i].geom && obj[i].transform)
        sync_transform (obj[i].transform, obj[i].geom);
    }
  }
}


/*
The GLUT window reshape event
*/

static void reshape ( int w, int h )
{
  glViewport ( 0, 0, w, h ) ;
}


/*
The GLUT keyboard event
*/

char locase (char c)
{
  if (c >= 'A' && c <= 'Z') return c - ('a'-'A');
  else return c;
}

static void keyboard ( unsigned char cmd, int, int )
{
  if ( puKeyboard ( cmd, PU_DOWN ) )
    return;

  if (cmd == 0x03)
    exit ( 0 ) ;

  int i,k;
  dReal sides[3];
  dMass m;
  
  cmd = locase (cmd);
  if (cmd == 'b' || cmd == 's') {
    if (num < NUM) {
      i = num;
      num++;
    }
    else {
      i = nextobj;
      nextobj++;
      if (nextobj >= num) nextobj = 0;
      
      // destroy the body and geoms for slot i
      dBodyDestroy (obj[i].body);
      if (obj[i].geom) dGeomDestroy (obj[i].geom);
      if (obj[i].transform) scene -> removeKid ( obj[i].transform ) ;
      memset (&obj[i],0,sizeof(obj[i]));
    }
    
    obj[i].body = dBodyCreate (world);
    for (k=0; k<3; k++) sides[k] = dRandReal()*0.5+0.1;
    
    dBodySetPosition (obj[i].body,
      dRandReal()*2-1,dRandReal()*2-1,dRandReal()+1);

    dMatrix3 R;
    dRFromAxisAndAngle (R,dRandReal()*2.0-1.0,dRandReal()*2.0-1.0,
      dRandReal()*2.0-1.0,dRandReal()*10.0-5.0);
    dBodySetRotation (obj[i].body,R);

    dBodySetData (obj[i].body,(void*) i);

    ssgEntity* shape = NULL;

    if (cmd == 'b') {
      dMassSetBox (&m,DENSITY,sides[0],sides[1],sides[2]);
      obj[i].geom = dCreateBox (space,sides[0],sides[1],sides[2]);
      shape = create_box(sides[0],sides[1],sides[2],1.f,0.f,0.f);
    }
    else if (cmd == 's') {
      sides[0] *= 0.5;
      dMassSetSphere (&m,DENSITY,sides[0]);
      obj[i].geom = dCreateSphere (space,sides[0]);
      shape = create_sphere(sides[0],0.f,0.f,1.f);
    }
    
    if (obj[i].geom)
      dGeomSetBody (obj[i].geom,obj[i].body);
    
    dBodySetMass (obj[i].body,&m);

    obj[i].transform = new ssgTransform ;
    obj[i].transform -> addKid ( shape ) ;
    scene -> addKid ( obj[i].transform ) ;

    sync_transform (obj[i].transform, obj[i].geom);
  }
}

static void specialfn ( int key, int x, int y )
{
  puKeyboard ( key + PU_KEY_GLUT_SPECIAL_OFFSET, PU_DOWN ) ;
}

static void motionfn ( int x, int y )
{
  puMouse ( x, y ) ;
}

static void mousefn ( int button, int updown, int x, int y )
{
  puMouse ( button, updown, x, y ) ;
}


/*
The GLUT redraw event
*/

static void redraw ()
{
  update_motion () ;
  
  glClear  ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ) ;
  
  ssgSetCamera ( & campos ) ;

  if ( wireframe )
    glPolygonMode ( GL_FRONT_AND_BACK, GL_LINE ) ;
  else
    glPolygonMode ( GL_FRONT_AND_BACK, GL_FILL ) ;

  ssgCullAndDraw ( scene ) ;
  
  glPolygonMode ( GL_FRONT_AND_BACK, GL_FILL ) ;

  puDisplay () ;

  glutPostRedisplay () ;
  glutSwapBuffers () ;
}


static void init_graphics ()
{
  int   fake_argc = 1 ;
  char *fake_argv[3] ;
  fake_argv[0] = "ode_demo" ;
  fake_argv[1] = "Open Dynamics Engine (ODE) Demo" ;
  fake_argv[2] = NULL ;
  
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
  glutSpecialFunc        ( specialfn ) ;
  glutMouseFunc          ( mousefn   ) ;
  glutMotionFunc         ( motionfn  ) ;
  glutPassiveMotionFunc  ( motionfn  ) ;
  
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
  
  ssgSetFOV     ( 60.0f, 0.0f ) ;
  ssgSetNearFar ( 1.0f, 700.0f ) ;

  /*
  Set up the Sun.
  */
  
  sgVec3 sunposn ;
  sgSetVec3 ( sunposn, 20.f, -50.f, 50.f ) ;
  ssgGetLight ( 0 ) -> setPosition ( sunposn ) ;
}


/*
Initialise the world
*/

static void init_world ()
{
  printf ("To drop another object, press:\n");
  printf ("   b for box.\n");
  printf ("   s for sphere.\n");
  
  // create world
  
  world = dWorldCreate();
  space = dHashSpaceCreate();
  contactgroup = dJointGroupCreate (0);
  dWorldSetGravity (world,0,0,-9.81f);
  dWorldSetCFM (world,1e-5f);
  memset (obj,0,sizeof(obj));

  // create scene root
  
  ssgModelPath   ( "data" ) ;
  ssgTexturePath ( "data" ) ;
  
  scene = new ssgRoot ;

  // create ground

  sgVec3 pos = {0.f, 0.f, -1.f};
  ssgTransform* transform = new ssgTransform;
  transform -> setTransform ( pos ) ;
  ssgaShape* shape = create_box (100.f, 100.f, 1.f, 0.0f, 0.4f, 0.0f) ;
  transform -> addKid ( shape ) ;
  scene -> addKid ( transform ) ;
  dCreatePlane (space,0,0,1,0);
}



/*
The works.
*/

int main ( int, char ** )
{
  init_graphics ();
  init_world ();
  init_gui ();

  glutMainLoop  () ;

  dJointGroupDestroy (contactgroup);
  dSpaceDestroy (space);
  dWorldDestroy (world);

  return 0 ;
}
