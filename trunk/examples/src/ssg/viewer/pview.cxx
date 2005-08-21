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

/*
 * pview.cxx
 *
 * By Bram Stolk.
 * Minimalistic plib viewer with a proper 'virtual ball' interface.
 * LMB for orbiting model, MMB for translation, RMB for approaching model.
 */

#include <assert.h>
#include <GL/glut.h>

#include <plib/ssg.h>

static float aspectratio=1.0;
static int winw, winh;
static ssgRoot *scene=0;
static ssgContext *context=0;
static sgMat4 camtrf;


static void idle(void)
{
  context->setCamera(camtrf);
  glutPostRedisplay () ;
}


static void redraw(void)
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ) ;
  glMatrixMode(GL_MODELVIEW);
  if (scene) ssgCullAndDraw(scene);
  glutSwapBuffers () ;
}


static int prevx=0;
static int prevy=0;
static int prevbutton=-1;

static void mouse(int button, int state, int x, int y)
{
  prevx=x;
  prevy=y;
  prevbutton=button;
}

static void motion(int x, int y)
{
  const float vel=220.0;
  float dx = vel * (x-prevx) / winw;
  float dy = vel * (y-prevy) / winh;
  prevx = x; prevy = y;

  if (prevbutton==0)
  {
    sgQuat hq, vq;
    sgAngleAxisToQuat(hq, -dx, camtrf[2]);
    sgAngleAxisToQuat(vq, -dy, camtrf[0]);
    sgQuat totalq;
    sgMultQuat(totalq, vq, hq);
    sgNormalizeQuat(totalq);
    sgMat4 m;
    sgQuatToMatrix(m, totalq);
    sgPostMultMat4(camtrf, m);
    return;
  }
  if (prevbutton==1)
  {
    float curdist = sgLengthVec3(camtrf[3]);
    sgAddScaledVec3(camtrf[3], camtrf[2],  dy*curdist/200.0);
    sgAddScaledVec3(camtrf[3], camtrf[0], -dx*curdist/200.0);
    return;
  }
  if (prevbutton==2)
  {
    float curdist = sgLengthVec3(camtrf[3]);
    sgAddScaledVec3(camtrf[3], camtrf[1], -dy*curdist/80.0);
    return;
  }
}


static void reshape(int w, int h)
{
  glViewport ( 0, 0, w, h ) ;

  aspectratio = w / (float) h;
  winw=w; winh=h;
  float fovy = 60.0 * M_PI / 180.0;
  float nearPlaneDistance = 0.4;
  float farPlaneDistance  = 2500.0;
  float y = tan(0.5 * fovy) * nearPlaneDistance;
  float x = aspectratio * y;
  context->setFrustum(-x,x,-y,y,nearPlaneDistance,farPlaneDistance);
}


static void keyboard(unsigned char k, int, int)
{
  if (k==27)
  {
    exit(0);
  }
}


int main(int argc, char *argv[]) 
{
  if (argc<2)
  {
    fprintf(stderr,"Usage: %s model0 [model1] .. [modeln]\n", argv[0]);
    return -1;
  }
  glutInitWindowSize(512,512);
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH | GLUT_MULTISAMPLE);
  glutCreateWindow(argv[1]);
  glutDisplayFunc(redraw);
  glutReshapeFunc(reshape);
  glutKeyboardFunc(keyboard);
  glutMouseFunc(mouse);
  glutIdleFunc(idle);
  glutMotionFunc(motion);

  ssgInit();

  glClearColor(0.3,0.3,0.55,1);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  scene = new ssgRoot();

  float amb[4]={0.2, 0.2, 0.2, 1};
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, amb);

  ssgTransform *trf = new ssgTransform();
  scene->addKid(trf);

  for (int i=1; i<argc; i++)
    trf->addKid(ssgLoad(argv[i]));

  context = new ssgContext();
  context->makeCurrent();

  sgVec3 off;
  sgZeroVec3(off);
  sgSubVec3(off, scene->getBSphere()->getCenter());

  trf->setTransform(off);
  SGfloat radius = scene->getBSphere()->getRadius();
  float d = 1.8*radius;
  if (d<0.5) d=0.5; // avoid placing near-plane beyond model
  sgMakeTransMat4(camtrf, 0, -d, 0);

  ssgLight *light=ssgGetLight(0);
  light->setColour(GL_AMBIENT,0,0,0);
  light->setPosition(0,0,radius);
  light->on();

  glutMainLoop();
}

