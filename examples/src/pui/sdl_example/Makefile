SDL_LDFLAGS := $(shell sdl-config --libs)
OGL_LDFLAGS := -lGL -lGLU
PUI_LDFLAGS := -lplibpu -lplibfnt -lplibsg -lplibul
LDFLAGS := $(OGL_LDFLAGS) $(SDL_LDFLAGS) $(PUI_LDFLAGS) -lm

SDL_CFLAGS := $(shell sdl-config --cflags)
PUI_CFLAGS := -DPU_USE_SDL
CXXFLAGS := -Wall -g $(SDL_CFLAGS) $(PUI_CFLAGS)

CXX := g++


OBJECT := mainSDL.o EventLoopSDL.o Demo.o


%.o : %.cxx
	$(CXX) $(CXXFLAGS) -c $<


sdl_example: $(OBJECT)
	$(CXX) -o $@ $+ $(LDFLAGS)


.PHONY: clean depend

clean:
	rm -f *.o sdl_example

depend:
	makedepend -Y *.cxx >/dev/null 2>&1


# Do 'make depend' to regenerate dependencies.
# DO NOT DELETE

Demo.o: Demo.h
EventLoopSDL.o: EventLoopSDL.h
mainSDL.o: Demo.h EventLoopSDL.h
