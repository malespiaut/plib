#ifndef EVENTLOOPSDL_H
#define EVENTLOOPSDL_H


class EventLoopSDL
{
public:
	// starts the main loop
	static void run();

	// requests main loop termination
	static void stop();

	// draw one frame
	static void draw();

	// translates an SDL keysym to the PUI equivalent
	static int translateKey(const SDL_keysym& keysym);

	// translates an SDL mouse button to the PUI equivalent
	static int translateMouse(int btn);

	// event handlers
	static int onMouseMove(char state, int x, int y);
	static int onMouseDown(int btn, int x, int y);
	static int onMouseUp(int btn, int x, int y);
	static int onKeyDown(const SDL_keysym& keysym);
	static int onKeyUp(const SDL_keysym& keysym);
};


#endif
