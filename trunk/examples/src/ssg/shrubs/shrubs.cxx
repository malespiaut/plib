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

// $Id$

#include <plib/fnt.h>
#include <plib/pu.h>
#include <plib/ssg.h>
#include <plib/ssgaBillboards.h>

#include <time.h>
#include <math.h>

#ifdef UL_WIN32
#  include <windows.h>
#else
#  include <unistd.h>
#  include <sys/time.h>
#endif

#ifdef FREEGLUT_IS_PRESENT
#  include <GL/freeglut.h>
#else
#  ifdef UL_MAC_OSX
#    include <GLUT/glut.h>
#  else
#    include <GL/glut.h>
#  endif
#endif


#define TILE_RES   12
#define NUM_TILES  (TILE_RES * TILE_RES)

#define TILE_SIZE  300.0f
#define WORLD_SIZE (TILE_RES * TILE_SIZE)

#define FRAC_LEVEL 7  // --> 128*128


// framerate control
static double min_delta = 1.0 / 200.0;
static double cur_delta = 1.0 / 50.0;

// terrain grid (display list)
static GLuint grid;
static bool draw_grid = true;

// billboard config
static float near_dist = 500.0f;
static float fade_dist = 2000.0f;
static float clip_dist = 2500.0f;
static int draw_num = 1000;
static int fade_num = 100;

// state variables (GLUT enforced)
static int winsx = 800, winsy = 600;
static int mousex, mousey, bstate;

// motion model
static int model = 1;  // 0 - trackball, 1 - fly

// trackball
static sgQuat rot = { 0, 0, 0, 1 };
static sgVec3 tra = { 0, 1500, 0 };

// fly
static sgVec2 spin;
static float thrust;
static float speed;
static sgVec3 hpr;
static sgVec3 pos = { 0, -0.25f * WORLD_SIZE, 20.0f };
static sgVec3 dir = { 0, 1, 0 };

// scene
static ssgRoot *scene;
static ssgaBillboards *bb[NUM_TILES];

// gui
static fntRenderer *text;
static puSlider *near_dist_sl, *fade_dist_sl, *clip_dist_sl;
static puSlider *draw_num_sl, *fade_num_sl;


inline float clampf(float x, float lo, float hi)
{
    return x <= lo ? lo : x >= hi ? hi : x;
}


static double get_time()
{
#ifdef UL_WIN32
    static LONGLONG t0, f;
    LONGLONG t;
    QueryPerformanceCounter((LARGE_INTEGER *) &t);
    if (f == 0) {
	QueryPerformanceFrequency((LARGE_INTEGER *) &f);
	t0 = t;
    }
    return (double) (t - t0) / f;
#else
    static struct timeval t0;
    struct timeval t;
    gettimeofday(&t, 0);
    if (t0.tv_sec == 0)
	t0 = t;
    return (t.tv_sec - t0.tv_sec) + 1e-6 * (t.tv_usec - t0.tv_usec);
#endif
}


#ifdef UL_WIN32 
inline void   srand48(int seed) { srand(seed); }
inline double drand48() { return (double) rand() / RAND_MAX; }
#endif

#ifdef _MSC_VER  /* UL_MSVC sometimes defined for mingw (gcc) */
inline float hypotf(float x, float y) { return sqrtf(x*x + y*y); }
#define snprintf _snprintf
#endif


static void motion_fn(int x, int y)
{
    if (bstate || !puMouse(x, y)) {
	
	if (model == 1) {
	    if (bstate & 4) {
		spin[0] = 0;
		spin[1] = 0;
	    } else {
		spin[0] = 2.0f * x / winsx - 1.0f;
		spin[1] = 2.0f * y / winsy - 1.0f;
	    }
	} else {
	    float fx = 2.0f * x / winsx - 1.0f;
	    float fy = 1.0f - 2.0f * y / winsy;
	    float dx = 2.0f * (x - mousex) / winsx;
	    float dy = 2.0f * (mousey - y) / winsy;
	    
	    if (bstate & 1) {
		float a = 0.5f * hypotf(dx, dy);
		if (a > 1e-6f) {
		    sgQuat q;
		    q[0] = dy * (1.0f - fx * fx);
		    q[1] = dy * fx - dx * fy;
		    q[2] = dx * (fy * fy - 1.0f);
		    sgScaleVec3(q, sinf(a) / sgLengthVec3(q));
		    q[3] = cosf(a);
		    sgMultQuat(rot, rot, q);
		}
	    }

	    if (bstate & 4) {
		tra[1] = tra[1] / (1.0f - dy);
	    }
	}
    }    
    mousex = x;
    mousey = y;
}


static void mouse_fn(int button, int updown, int x, int y)
{
    if (bstate || !puMouse(button, updown, x, y)) {
	int bit = 0;
	switch (button) {
	case GLUT_LEFT_BUTTON:    bit = 1; break;	
	case GLUT_MIDDLE_BUTTON:  bit = 2; break;
	case GLUT_RIGHT_BUTTON:   bit = 4; break;
	}
	switch (updown) {
	case GLUT_DOWN:  bstate |= bit;  break;
	case GLUT_UP:    bstate &= ~bit; break;
	}
	thrust = (bstate & 1) - (bstate & 4) / 2;
	if (bstate & 4) {
	    spin[0] = 0;
	    spin[1] = 0;
	}
    }
    mousex = x;
    mousey = y;
}


static void special_fn(int key, int x, int y)
{
    if (!puKeyboard(key + PU_KEY_GLUT_SPECIAL_OFFSET, PU_DOWN)) {
	switch (key) {
	case GLUT_KEY_UP:    min_delta *= 1.1; break;
	case GLUT_KEY_DOWN:  min_delta /= 1.1; break;
	}
    }
}


static void keyboard_fn(unsigned char key, int, int)
{
    if (!puKeyboard(key, PU_DOWN)) {
	switch (key) {
	case 0x1b: 
	    exit(0);
	case '1':
	    model = 0; 
	    break;
	case '2':
	    model = 1;
	    break;
	case 'g':
	    draw_grid = !draw_grid;
	    break;
	}
    }
}



static void update_bboards()
{
    for (int i = 0; i < NUM_TILES; i++) {
	bb[i]->setNearRange(near_dist);
	bb[i]->setFadeRange(fade_dist);
	bb[i]->setClipRange(clip_dist);
	bb[i]->setDrawNum(draw_num);
	bb[i]->setFadeNum(fade_num);
    }
}

static void near_dist_cb(puObject *ob)
{
    near_dist = ob->getFloatValue();
    update_bboards();
}

static void fade_dist_cb(puObject *ob)
{
    fade_dist = ob->getFloatValue();
    update_bboards();
}

static void clip_dist_cb(puObject *ob)
{
    clip_dist = ob->getFloatValue();
    update_bboards();
}

static void draw_num_cb(puObject *ob)
{
    draw_num = ob->getIntegerValue();
    update_bboards();
}

static void fade_num_cb(puObject *ob)
{
    fade_num = ob->getIntegerValue();
    update_bboards();
}



static void init_scene()
{
    int i, x, y;

    scene = new ssgRoot;

    // produce a simple diamond-square fractal
    const int size = 1 << FRAC_LEVEL;
    const int mask = size - 1;
    float *elev = new float[size * size]; // height map
    memset(elev, 0, sizeof(elev));
    for (i = FRAC_LEVEL; i > 0; i--) {
	int s = 1 << i, t = s >> 1;
	float r = 750.0f * powf(s / size, 0.9f);
	for (y = 0; y < size; y += s) {
	    int y1 = (y + s) & mask;
	    for (x = 0; x < size; x += s) {
		int x1 = (x + s) & mask;
		elev[(y + t) * size + (x + t)] = (0.25f * (elev[y  * size + x ] +
							   elev[y  * size + x1] +
							   elev[y1 * size + x ] +
							   elev[y1 * size + x1])
						  + r * (0.5f - (float) drand48()));
	    }
	}
	r = 750.0f * powf(0.7071f * s / size, 0.9f);
	for (y = 0; y < size; y += s) {
	    int y0 = (y - t) & mask;
	    int y1 = (y + s) & mask;
	    for (x = 0; x < size; x += s) {
		int x0 = (x - t) & mask;
		int x1 = (x + s) & mask;
		elev[y * size + (x + t)] = (0.25f * (elev[y0 * size + (x + t)] +
						     elev[y * size + x ] +
						     elev[y * size + x1] +
						     elev[(y + t) * size + (x + t)]) 
					    + r * (0.5f - (float) drand48()));
		elev[(y + t) * size + x] = (0.25f * (elev[(y + t) * size + x0] +
						     elev[y  * size + x] +
						     elev[y1 * size + x] +
						     elev[(y + t) * size + (x + t)]) 
					    + r * (0.5f - (float) drand48()));
	    }
	}	
    }

    // generate display list
    grid = glGenLists(1);
    glNewList(grid, GL_COMPILE);
    for (y = 0; y <= size; y += 4) {
	glBegin(GL_LINE_STRIP);
	for (x = 0; x <= size; x++)
	    glVertex3f(((float) x / size - 0.5f) * WORLD_SIZE,
		       ((float) y / size - 0.5f) * WORLD_SIZE,
		       elev[(y & mask) * size + (x & mask)]);
	glEnd();
    }
    for (x = 0; x <= size; x += 4) {
	glBegin(GL_LINE_STRIP);
	for (y = 0; y <= size; y++)
	    glVertex3f(((float) x / size - 0.5f) * WORLD_SIZE,
		       ((float) y / size - 0.5f) * WORLD_SIZE,
		       elev[(y & mask) * size + (x & mask)]);
	glEnd();
    }
    glEndList();


    // now load the shrubs

    ssgTexture *tex = new ssgTexture("data/deciduous-tree.rgb");

    ssgaBillboards::initTexAlpha(tex);

    for (y = 0; y < TILE_RES; y++) {
	for (x = 0; x < TILE_RES; x++) {
	    ssgaBillboards *b = new ssgaBillboards;
	  
	    b->setTexture(tex);
	    b->setWidth(3.0f);
	    b->setHeight(8.0f);

	    float tx = (x - 0.5f * TILE_RES) * TILE_SIZE;
	    float ty = (y - 0.5f * TILE_RES) * TILE_SIZE;

	    for (int i = 0; i < 3000; i++) {

		float x = tx + drand48() * TILE_SIZE;
		float y = ty + drand48() * TILE_SIZE;
		float z;

		// bilinear lookup
		float xf = (0.5f + x / WORLD_SIZE) * size;
		float yf = (0.5f + y / WORLD_SIZE) * size;
		int x0 = (int) xf;
		int y0 = (int) yf;
		int x1 = x0 + 1;
		int y1 = y0 + 1;
		float xw1 = xf - (float) x0;
		float yw1 = yf - (float) y0;
		float xw0 = 1.0f - xw1;
		float yw0 = 1.0f - yw1;
		x0 &= mask;
		y0 &= mask;
		x1 &= mask;
		y1 &= mask;
		z = (elev[y0 * size + x0] * xw0 * yw0 +
		     elev[y0 * size + x1] * xw1 * yw0 +
		     elev[y1 * size + x0] * xw0 * yw1 +
		     elev[y1 * size + x1] * xw1 * yw1);
		
		b->add(x, y, z, 0.75f + 0.5f * drand48());
	    }
	    
	    bb[y * TILE_RES + x] = b;
	    
	    scene->addKid(b);
	}
    }

    x = (int)((0.5f + pos[0] / WORLD_SIZE) * size);
    y = (int)((0.5f + pos[1] / WORLD_SIZE) * size);
    pos[2] += elev[(y & mask) * size + (x & mask)];

    delete [] elev;

    update_bboards();
}


static void init_gui()
{
    fntInit();

    fntTexFont *font = new fntTexFont("data/Courier-Bold.txf");

    text = new fntRenderer();
    text->setFont(font);
    text->setPointSize(18); 

    puInit();

    puSetDefaultFonts(puFont(font, 14), puFont(font, 18));
    puSetDefaultStyle(PUSTYLE_SMALL_SHADED);
    puSetDefaultColourScheme(0.7f, 0.7f, 0.7f, 1.0f);

    puGroup *group = new puGroup(0, 0);
    puSlider *sl;
    int h = winsy;

    near_dist_sl = sl = new puSlider(250, h - 30, 150);
    sl->setValue(near_dist);
    sl->setMinValue(100.0f);
    sl->setMaxValue(5000.0f);
    sl->setCallback(near_dist_cb);

    fade_dist_sl = sl = new puSlider(250, h - 55, 150);
    sl->setValue(fade_dist);
    sl->setMinValue(100.0f);
    sl->setMaxValue(5000.0f);
    sl->setCallback(fade_dist_cb);

    clip_dist_sl = sl = new puSlider(250, h - 80, 150);
    sl->setValue(clip_dist);
    sl->setMinValue(100.0f);
    sl->setMaxValue(5000.0f);
    sl->setCallback(clip_dist_cb);
   
    draw_num_sl = sl = new puSlider(250, h - 110, 150);
    sl->setValue(draw_num);
    sl->setMinValue(0);
    sl->setMaxValue(3000);    
    sl->setCallback(draw_num_cb);
   
    fade_num_sl = sl = new puSlider(250, h - 135, 150);
    sl->setValue(fade_num);
    sl->setMinValue(0);
    sl->setMaxValue(3000);
    sl->setCallback(fade_num_cb);
       
    group->close();
} 


static void reshape_fn(int w, int h)
{
    winsx = w;
    winsy = h;

    glViewport(0, 0, w, h);

    if (near_dist_sl) {
	near_dist_sl->setPosition(250, h - 30);
	fade_dist_sl->setPosition(250, h - 55);
	clip_dist_sl->setPosition(250, h - 80);
	draw_num_sl->setPosition(250, h - 110);
	fade_num_sl->setPosition(250, h - 135);
    }
}


static void display_fn()
{
    static float backlog1[256];
    static float backlog2[256];

    static double last_time;
    static double sync_time;
    static int frame_count;

    double t0 = get_time();
    int i;

    frame_count++;

    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);   
    glEnable(GL_DEPTH_TEST);    

    sgMat4 mat;

    if (model == 0) {
	// trackball
	sgQuatToMatrix(mat, rot);
	sgCopyVec3(mat[3], tra);
	sgTransposeNegateMat4(mat);
    } else {
	// fly
	float dt = cur_delta;
	float s = powf(0.2f, dt);
	hpr[2] = s * hpr[2] + (1.0f - s) * 40.0f * spin[0];
	hpr[1] = s * hpr[1] + (1.0f - s) * 40.0f * spin[1];
	hpr[0] -= dt * hpr[2];
	float a = 10.0f * (thrust - dir[2]);
	speed = clampf(speed + a * dt, 1.0f, 25.0f);	
	sgAddScaledVec3(pos, dir, dt * speed);
	sgMakeCoordMat4(mat, pos, hpr);
	sgCopyVec3(dir, mat[1]);
    }

    ssgaBillboards::total_drawn = 0;
    
    ssgSetCamera(mat);
    ssgCullAndDraw(scene);
    ssgLoadModelviewMatrix();

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_COLOR_MATERIAL);
    glDisable(GL_LIGHTING);
    glDisable(GL_BLEND);
    glDisable(GL_LINE_SMOOTH);

    if (draw_grid) {
	glColor3f(0, 0, 0);
	glCallList(grid);
    }

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glAlphaFunc(GL_GREATER, 0.3f);
    glEnable(GL_ALPHA_TEST);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);


    // timings

    const float y0 = 0.45f, y1 = 1.0f;
    const float d0 = 0, d1 = 0.05f;

    glBegin(GL_LINE_STRIP);
    glColor3f(0.4f, 0.7f, 0.4f);
    for (i = 1; i <= 256; i++) {
	float d = backlog2[(frame_count - i) & 0xff];
	glVertex2f(2.0f * i / 257 - 1.0f, y0 + (y1 - y0) * (d - d0) / (d1 - d0));
    }
    glEnd();

    glBegin(GL_LINE_STRIP);
    glColor3f(0.7f, 0.7f, 0.7f);       
    for (i = 1; i <= 256; i++) {
	float d = backlog1[(frame_count - i) & 0xff];
	glVertex2f(2.0f * i / 257 - 1.0f, y0 + (y1 - y0) * (d - d0) / (d1 - d0));
    }
    glEnd();

    glBegin(GL_LINES);
    glColor3f(0.3f, 0.4f, 0.4f);
    float y = y0 + (y1 - y0) * ((float) min_delta - d0) / (d1 - d0);
    glVertex2f(-1.0f, y);
    glVertex2f( 1.0f, y);
    glEnd();
        

    // gui

    int w = winsx;
    int h = winsy;
    char buf[64];
    
    glOrtho(0, w, 0, h, 0, 1);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glColor3f(0, 0, 0);
  
    text->begin();

    text->start2f(w - 200, h - 30);
    text->puts("fps:");
    snprintf(buf, sizeof(buf), "%.0f Hz", 1.0 / cur_delta);
    text->start2f(w - 100, h - 30);
    text->puts(buf);

    text->start2f(w - 200, h - 55);
    text->puts("shrubs:");
    snprintf(buf, sizeof(buf), "%d\n", ssgaBillboards::total_drawn); 
    text->start2f(w - 100, h - 55);
    text->puts(buf);

    text->start2f(20, h - 30);
    text->puts("NearRange:");
    snprintf(buf, sizeof(buf), "%5.0f", near_dist);
    text->start2f(150, h - 30);
    text->puts(buf);

    text->start2f(20, h - 55);
    text->puts("FadeRange:");
    snprintf(buf, sizeof(buf), "%5.0f", fade_dist);
    text->start2f(150, h - 55);
    text->puts(buf);

    text->start2f(20, h - 80);
    text->puts("ClipRange:");
    snprintf(buf, sizeof(buf), "%5.0f", clip_dist);
    text->start2f(150, h - 80);
    text->puts(buf);

    text->start2f(20, h - 110);
    text->puts("DrawNum:");
    snprintf(buf, sizeof(buf), "%5d", draw_num);
    text->start2f(150, h - 110);
    text->puts(buf);

    text->start2f(20, h - 135);
    text->puts("FadeNum:");
    snprintf(buf, sizeof(buf), "%5d", fade_num);
    text->start2f(150, h - 135);
    text->puts(buf);

    text->end();

    puDisplay();

    glFinish();


    // sync

    double t1 = get_time();

    backlog1[frame_count & 0xff] = t1 - t0;
    backlog2[frame_count & 0xff] = t1 - last_time;

    last_time = t1;

    sync_time += min_delta;
  
    if (t1 < sync_time) {
	double dt = sync_time - t1 - 0.005;
	if (dt > 0) {
#ifdef UL_WIN32
	    Sleep((int)(1e+3 * dt));
#else
	    usleep((int)(1e+6 * dt));
#endif
	}
    } else if (t1 > sync_time + 0.05)
	sync_time = t1;

    cur_delta = 0;
    for (i = 0; i < 256; i++)
	cur_delta += backlog2[i];
    cur_delta *= 1.0 / 256;

    glutSwapBuffers();
    glutPostRedisplay();
}



int main(int argc, char** argv)
{
    srand48(time(0));
    
    glutInit(&argc, argv);
    glutInitWindowSize(winsx, winsy);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutCreateWindow("shrubs");

    glutDisplayFunc(display_fn);
    glutReshapeFunc(reshape_fn);
    glutMouseFunc(mouse_fn);
    glutMotionFunc(motion_fn);
    glutKeyboardFunc(keyboard_fn);
    glutSpecialFunc(special_fn);

    ssgInit();

    ssgSetFOV(60.0f, 0.0f);
    ssgSetNearFar(10.0f, 10000.0f);
    
    //ssgModelPath(".");
    //ssgTexturePath(".");

    init_scene();
    
    init_gui();

    glutMainLoop();

    return 0;
}


/*
  Local Variables:
  mode: C++
  c-basic-offset: 4
  c-file-offsets: ((substatement-open 0) (case-label 0))
  End:
*/
