
SIMON = libsimon.a   simon
LIBS  = -L/usr/X11R6/lib -lplibpsl -lplibssg -lplibsg -lplibul -lglut -lGLU -lGL -lpthread -lX11 -lm

all: ${SIMON}

simon : libsimon.a simon.o
	g++ -o simon simon.o libsimon.a ${LIBS}

simon.o : simon.cxx simon.h
	g++ -I/usr/X11R6/include -I. -o simon.o -c simon.cxx

simon_api.o : simon_api.cxx simon.h
	g++ -I/usr/X11R6/include -o simon_api.o -c simon_api.cxx

libsimon.a : simon_api.o
	ar cru libsimon.a simon_api.o
 
install: libsimon.a simon.h simon
	mkdir -p /usr/local/simon
	cp simon libsimon.a simon.h /usr/local/simon

uninstall:
	rm -r /usr/local/simon

clean:
	rm -f simon libsimon.a *.o

