#include <stdlib.h>
#include <plib/pu.h>
#include "SDL.h"

#include "Demo.h"
#include "EventLoopSDL.h"


// default window size
static const int WIDTH  = 640;
static const int HEIGHT = 480;


static void initSDL(int w, int h, const char *title)
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		fprintf(stderr, "Unable to initialize SDL: %s\n", SDL_GetError());
		exit(1);
	}
	atexit(SDL_Quit);

	if (SDL_SetVideoMode(w, h, 0, SDL_OPENGL | SDL_ANYFORMAT/* | SDL_FULLSCREEN*/) == NULL) {
		fprintf(stderr, "Unable to create OpenGL screen: %s\n", SDL_GetError());
		exit(1);
	}

	if (title)
		SDL_WM_SetCaption(title, NULL);

	SDL_EnableKeyRepeat(150, 75);
	SDL_EnableUNICODE(true);
}


void initOGL(int w, int h)
{
	glViewport(0, 0, w, h);
	glClearColor(0.1f, 0.4f, 0.1f, 1.0f);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glAlphaFunc(GL_GREATER, 0.1f);
}


int main(int argc, char *argv[])
{
	initSDL(WIDTH, HEIGHT, "PUI with SDL sample");
	initOGL(WIDTH, HEIGHT);
	initPUI();

	createInterface();
	
	EventLoopSDL::run();
	
	return 0;
}

