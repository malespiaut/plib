#include <stdio.h>
#include <GL/gl.h>
#include <plib/pu.h>

#include "Demo.h"


static puInput* input = NULL;



static void on_quit(puObject*)
{
	exit(0);
}

static void on_say_hello(puObject*)
{
	printf("Hello World!\n");
}

static void on_print_text(puObject*)
{
	printf("Input text: %s\n", input->getStringValue());
}



void initPUI()
{
	puInit();

	puSetDefaultStyle(PUSTYLE_SMALL_BEVELLED);
	puSetDefaultColourScheme(0.75f, 0.75f, 0.75f, 1.0f);
}


void createInterface()
{
	// menu bar
	puMenuBar* menu = new puMenuBar();
	char* menu_text[] = {
		"Quit", 
		"----", 
		"Say Hello", 
		"Print text", 
		NULL 
	};
	puCallback menu_cb[] = {
		on_quit,
		NULL,
		on_say_hello,
		on_print_text,
		NULL 
	};
	menu->add_submenu("Demo", menu_text, menu_cb);
	menu->close();

	// input and buttons
	puButton* button;
	button = new puOneShot(100, 200, 220, 225);
	button->setLegend("Quit");
	button->setCallback(on_quit);

	button = new puOneShot(100, 250, 220, 275);
	button->setLegend("Say Hello");
	button->setCallback(on_say_hello);

	button = new puOneShot(100, 300, 220, 325);
	button->setLegend("Print text");
	button->setCallback(on_print_text);

	input = new puInput(250, 300, 450, 325);
}

