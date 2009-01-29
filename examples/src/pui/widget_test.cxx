#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef WIN32
#  include <windows.h>
#else
#  include <unistd.h>
#endif
#include <math.h>

#ifdef FREEGLUT_IS_PRESENT
#  include <GL/freeglut.h>
#else
#  ifdef __APPLE__
#    include <GLUT/glut.h>
#  else
#    include <GL/glut.h>
#  endif
#endif

#include <plib/puAux.h>


char *entries[] = {
	"Unfortunately,",
	"the",
	"input",
	"box",
	"isn't",
	"updated",
	"on",
	"popup",
	"selection",
	0,
};



void motionfn (int x, int y)
{
	puMouse(x, y);
	glutPostRedisplay();
}


void mousefn (int button, int updown, int x, int y)
{
	puMouse(button, updown, x, y);
	glutPostRedisplay();
}


void displayfn(void)
{
	glClearColor(0.4f, 0.4f, 0.4f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	puDisplay();
	glutSwapBuffers();
	glutPostRedisplay();
}


int main(int argc, char **argv)
{
	glutInitWindowSize(640, 480);
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB|GLUT_DOUBLE|GLUT_DEPTH);
	glutCreateWindow("PUI Application");
	glutDisplayFunc(displayfn);
	glutMouseFunc(mousefn);
	glutMotionFunc(motionfn);
	puInit();

	puaComboBox *b = new puaComboBox(220, 180, 420, 220, entries,true);
	b->setLegend("Say Hello");

	printf("%d\n", PLIB_VERSION);
	glutMainLoop();
	return 0;
}

