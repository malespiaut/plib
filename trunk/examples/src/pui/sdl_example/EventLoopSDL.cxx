#include <GL/gl.h>
#include <SDL/SDL_keysym.h>
#include <plib/pu.h>

#include "EventLoopSDL.h"


static bool stop_requested;



void EventLoopSDL::run()
{
	SDL_Event event;

	stop_requested = false;
	while (!stop_requested) {

		// process events
		SDL_WaitEvent(NULL);
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_QUIT:
				stop_requested = true;
				break;
			case SDL_MOUSEMOTION:
				onMouseMove(event.motion.state, event.motion.x, event.motion.y);
				break;
			case SDL_MOUSEBUTTONDOWN:
				onMouseDown(event.button.button, event.button.x, event.button.y);
				break;
			case SDL_MOUSEBUTTONUP:
				onMouseUp(event.button.button, event.button.x, event.button.y);
				break;
			case SDL_KEYDOWN:
				onKeyDown(event.key.keysym);
				break;
			case SDL_KEYUP:
				onKeyUp(event.key.keysym);
				break;
			}
		}

		// redraw
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		draw();
		SDL_GL_SwapBuffers();
	}
}


void EventLoopSDL::stop()
{
	stop_requested = true;
}


void EventLoopSDL::draw()
{
	puDisplay();
}


int EventLoopSDL::translateKey(const SDL_keysym& keysym)
{
	// Printable characters
	if (keysym.unicode > 0)
		return keysym.unicode;

	// Numpad key, translate no non-numpad equivalent
	if (keysym.sym >= SDLK_KP0 && keysym.sym <= SDLK_KP_EQUALS) {
		switch (keysym.sym) {
		case SDLK_KP0:
			return PU_KEY_INSERT;
		case SDLK_KP1:
			return PU_KEY_END;
		case SDLK_KP2:
			return PU_KEY_DOWN;
		case SDLK_KP3:
			return PU_KEY_PAGE_DOWN;
		case SDLK_KP4:
			return PU_KEY_LEFT;
		case SDLK_KP6:
			return PU_KEY_RIGHT;
		case SDLK_KP7:
			return PU_KEY_HOME;
		case SDLK_KP8:
			return PU_KEY_UP;
		case SDLK_KP9:
			return PU_KEY_PAGE_UP;
		default:
			return -1;
		}
	}

	// Everything else
	switch (keysym.sym) {
	case SDLK_UP:
		return PU_KEY_UP;
	case SDLK_DOWN:
		return PU_KEY_DOWN;
	case SDLK_LEFT:
		return PU_KEY_LEFT;
	case SDLK_RIGHT:
		return PU_KEY_RIGHT;

	case SDLK_PAGEUP:
		return PU_KEY_PAGE_UP;
	case SDLK_PAGEDOWN:
		return PU_KEY_PAGE_DOWN;
	case SDLK_HOME:
		return PU_KEY_HOME;
	case SDLK_END:
		return PU_KEY_END;
	case SDLK_INSERT:
		return PU_KEY_INSERT;
	case SDLK_DELETE:
		return -1;

	case SDLK_F1:
		return PU_KEY_F1;
	case SDLK_F2:
		return PU_KEY_F2;
	case SDLK_F3:
		return PU_KEY_F3;
	case SDLK_F4:
		return PU_KEY_F4;
	case SDLK_F5:
		return PU_KEY_F5;
	case SDLK_F6:
		return PU_KEY_F6;
	case SDLK_F7:
		return PU_KEY_F7;
	case SDLK_F8:
		return PU_KEY_F8;
	case SDLK_F9:
		return PU_KEY_F9;
	case SDLK_F10:
		return PU_KEY_F10;
	case SDLK_F11:
		return PU_KEY_F11;
	case SDLK_F12:
		return PU_KEY_F12;

	default:
		return -1;
	}
}

int EventLoopSDL::translateMouse(int btn)
{
	// SDL counts buttons from 1, PUI from 0
	return btn-1;
}


int EventLoopSDL::onMouseMove(char state, int x, int y)
{
	return false;
}

int EventLoopSDL::onMouseDown(int btn, int x, int y)
{
	return puMouse(translateMouse(btn), PU_DOWN, x, y);
}

int EventLoopSDL::onMouseUp(int btn, int x, int y)
{
	return puMouse(translateMouse(btn), PU_UP, x, y);
}

int EventLoopSDL::onKeyDown(const SDL_keysym& keysym)
{
	return puKeyboard(translateKey(keysym), PU_DOWN);
}

int EventLoopSDL::onKeyUp(const SDL_keysym& keysym)
{
	return puKeyboard(translateKey(keysym), PU_UP);
}
