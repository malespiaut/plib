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


int margin = 50;

char *long_text;
puObject *widget;



// samples

char text[] =
	"This program is meant to be used for widget testing. Insert your code\n"
	"between BEGIN and END marks. Use list, long_list, text and long_text\n"
	"as widget contents where appropriate. It's intended that there are no resizing\n"
	"limits. This allows to verify that the widget works even in insanely small\n"
	"sizes, or at least doesn't crash then.\n"
	"\n"
	"Some random parts from the pui documentation:\n"
	"\n"
	"Like the constructors of the previously described widgets, the puComboBox\n"
	"constructor takes a NULL-terminated array of strings specifying the items\n"
	"that should go into the widget as it's fourth parameter. If 'editable'\n"
	"is TRUE (default), the input box is made editable for allowing the user\n"
	"to enter a string in addition to selecting one of the entries from the\n"
	"popup menu.\n"
	"\n"
	"Depending on the position of the puComboBox inside the window, the widget\n"
	"automatically determines whether the popup menu should be shown above or\n"
	"below the widget when the user clicks on the arrow. The puArrowButton\n"
	"is either pointing in up or down direction to indicate where the menu will\n"
	"pop up.\n"
	"\n"
	"Regardless of whether you passed TRUE or FALSE to the 'editable' parameter\n"
	"when constructing the widget, the user can click inside the input box and\n"
	"use the Up and Down keys to browse through the items. The Home and End\n"
	"keys switch to the first or last item as you would expect, and the PgUp/PgDown\n"
	"can be used to toggle the popup menu.\n"
	"\n"
	"Just like all other classes which store a pointer to an array of strings\n"
	"that indicates the widget's elements, the puComboBox class provides a routine\n"
	"that allows you to change the list of elements in mid-run - and you can\n"
	"also retrieve the number of items.\n";

char *list[] = {
	"puArrowButton", "puButton", "puButtonBox", "puDial", "puFrame",
	"puGroup", "puInput", "puInterface", "puListBox", "puMenuBar",
	"puObject", "puOneShot", "puPopup", "puPopupMenu", "puSlider",
	"puText", "puValue", 0};

char *long_list[] = {
	"PUARROW_DOWN", "PUARROW_FASTDOWN", "PUARROW_FASTLEFT", "PUARROW_FASTRIGHT",
	"PUARROW_FASTUP", "PUARROW_LEFT", "PUARROW_RIGHT", "PUARROW_UP",
	"PUBUTTON_CIRCLE", "PUBUTTON_NORMAL", "PUBUTTON_RADIO", "PUBUTTON_VCHECK",
	"PUBUTTON_XCHECK", "PUCLASS_ARROW", "PUCLASS_BUTTON", "PUCLASS_BUTTONBOX",
	"PUCLASS_DIAL", "PUCLASS_DIALOGBOX", "PUCLASS_FRAME", "PUCLASS_GROUP",
	"PUCLASS_INPUT", "PUCLASS_INTERFACE", "PUCLASS_LISTBOX", "PUCLASS_MENUBAR",
	"PUCLASS_OBJECT", "PUCLASS_ONESHOT", "PUCLASS_POPUP", "PUCLASS_POPUPMENU",
	"PUCLASS_SLIDER", "PUCLASS_TEXT", "PUCLASS_VALUE", "PUCOL_BACKGROUND",
	"PUCOL_EDITFIELD", "PUCOL_FOREGROUND", "PUCOL_HIGHLIGHT", "PUCOL_LABEL",
	"PUCOL_LEGEND", "PUCOL_MAX", "PUCOL_MISC", "PUDEACTIVATE_ON_MOUSE_CLICK",
	"PUDEACTIVATE_ON_NEXT_WIDGET_ACTIVATION", "PUPLACE_ABOVE",
	"PUPLACE_ABOVE_LEFT", "PUPLACE_ABOVE_RIGHT", "PUPLACE_BELOW",
	"PUPLACE_BELOW_LEFT", "PUPLACE_BELOW_RIGHT", "PUPLACE_BOTTOM_CENTER",
	"PUPLACE_BOTTOM_CENTERED", "PUPLACE_BOTTOM_LEFT", "PUPLACE_BOTTOM_RIGHT",
	"PUPLACE_CENTERED", "PUPLACE_CENTERED_CENTERED", "PUPLACE_CENTERED_LEFT",
	"PUPLACE_CENTERED_RIGHT", "PUPLACE_DEFAULT", "PUPLACE_LABEL_DEFAULT",
	"PUPLACE_LEFT", "PUPLACE_LEFT_CENTER", "PUPLACE_LEGEND_DEFAULT",
	"PUPLACE_LOWER_LEFT", "PUPLACE_LOWER_RIGHT", "PUPLACE_RIGHT",
	"PUPLACE_RIGHT_CENTER", "PUPLACE_TOP_CENTER", "PUPLACE_TOP_CENTERED",
	"PUPLACE_TOP_LEFT", "PUPLACE_TOP_RIGHT", "PUPLACE_UPPER_LEFT",
	"PUPLACE_UPPER_RIGHT", "PUSLIDER_ALWAYS", "PUSLIDER_CLICK",
	"PUSLIDER_DELTA", "PUSTRING_INITIAL", "PUSTRING_MAX", "PUSTR_BGAP",
	"PUSTR_LGAP", "PUSTR_RGAP", "PUSTR_TGAP", "PUSTYLE_BEVELLED",
	"PUSTYLE_BOXED", "PUSTYLE_DEFAULT", "PUSTYLE_DROPSHADOW", "PUSTYLE_MAX",
	"PUSTYLE_NONE", "PUSTYLE_PLAIN", "PUSTYLE_RADIO", "PUSTYLE_SHADED",
	"PUSTYLE_SMALL_BEVELLED", "PUSTYLE_SMALL_SHADED", "PUSTYLE_SPECIAL_UNDERLINED",
	"PU_CONTINUAL", "PU_DOWN", "PU_DRAG", "PU_KEY_DOWN", "PU_KEY_END", "PU_KEY_F1",
	"PU_KEY_F10", "PU_KEY_F11", "PU_KEY_F12", "PU_KEY_F2", "PU_KEY_F3",
	"PU_KEY_F4", "PU_KEY_F5", "PU_KEY_F6", "PU_KEY_F7", "PU_KEY_F8", "PU_KEY_F9",
	"PU_KEY_GLUT_SPECIAL_OFFSET", "PU_KEY_HOME", "PU_KEY_INSERT", "PU_KEY_LEFT",
	"PU_KEY_PAGE_DOWN", "PU_KEY_PAGE_UP", "PU_KEY_RIGHT", "PU_KEY_UP",
	"PU_LEFT_BUTTON", "PU_MIDDLE_BUTTON", "PU_NOBUTTON", "PU_RADIO_BUTTON_SIZE",
	"PU_RIGHT_BUTTON", "PU_UP", "PU_UP_AND_DOWN", 0};




// callback functions

void motionfn(int x, int y)
{
	puMouse(x, y);
	glutPostRedisplay();
}


void mousefn(int button, int updown, int x, int y)
{
	puMouse(button, updown, x, y);
	glutPostRedisplay();
}


void specialfn(int key, int, int)
{
	printf("SPECIAL=%d\n", key);
	puKeyboard(key + PU_KEY_GLUT_SPECIAL_OFFSET, PU_DOWN);
	glutPostRedisplay();
}


void keyfn(unsigned char key, int, int)
{
	printf("KEY=%d\n", key);
	puKeyboard(key, PU_DOWN);
	glutPostRedisplay();
	if (key == 27)
		exit(0);
}


void displayfn(void)
{
	glClearColor(0.4f, 0.4f, 0.4f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	puDisplay();
	glutSwapBuffers();
	glutPostRedisplay();
}


void reshapefn(int w, int h)
{
	widget->setSize(w - 2 * margin, h - 2 * margin);
}


void callback(puObject *object)
{
	printf("callback value: [%s]\n", object->getStringValue());
}


void atexitfn(void)
{
	delete [] long_text;
}




int main(int argc, char **argv)
{
	printf("PLIB Version %d\n", PLIB_VERSION);

	atexit(atexitfn);
	const int REPEAT = 10;
	long_text = new char[REPEAT * strlen(text) + REPEAT + 1];
	long_text[0] = '\0';
	for (int i = 0; i < REPEAT; i++) {
		strcat(long_text, text);
		strcat(long_text, "\n");
	}

	glutInitWindowSize(640, 480);
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB|GLUT_DOUBLE|GLUT_DEPTH);
	glutCreateWindow("PUI Application");
	glutDisplayFunc(displayfn);
	glutMouseFunc(mousefn);
	glutKeyboardFunc(keyfn);
	glutSpecialFunc(specialfn);
	glutMotionFunc(motionfn);
	glutReshapeFunc(reshapefn);

	puSetDefaultStyle(PUSTYLE_SMALL_SHADED);
	puInit();
	//
	//-- BEGIN ----------------------------------------------------------------------


	margin = 50;

	widget = new puaLargeInput(margin, margin, 420, 400, 11, 20, false);
	widget->setValue(text);
	widget->setCallback(callback);


	//-- END ------------------------------------------------------------------------
	//
	int w, h;
	widget->getSize(&w, &h);
	glutReshapeWindow(w + 2 * margin, h + 2 * margin);
	glutMainLoop();
	return 0;
}

