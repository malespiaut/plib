
SIMON = libsimon.a   simon
LIBS  = -lplibpsl -lplibssg -lplibsg -lplibjs -lplibul -lglut -lGLU -lGL -lpthread -L/usr/X11/lib -lX11 -lm

all: ${SIMON}

simon : libsimon.a simon.o /usr/lib/libplibpsl.a
	g++ -o simon simon.o libsimon.a ${LIBS}

simon.o : simon.cxx simon.h
	g++ -I. -o simon.o -c simon.cxx

simon_api.o : simon_api.cxx simon.h
	g++ -o simon_api.o -c simon_api.cxx

libsimon.a : simon_api.o
	ar cru libsimon.a simon_api.o
 
install: libsimon.a simon.h simon
	mkdir -p /usr/local/simon
	cp libsimon.a simon.h /usr/local/simon
	cp simon /usr/local/bin

uninstall:
	rm -r /usr/local/simon
	rm /usr/local/bin/simon

clean:
	rm -f simon libsimon.a *.o

