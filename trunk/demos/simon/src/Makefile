
SIMON = libsimon.a

all: ${SIMON}

simon.o : simon.cxx simon.h
	g++ -o simon.o -c simon.cxx

libsimon.a : simon.o
	ar cru libsimon.a simon.o
 
install: libsimon.a simon.h
	mkdir -p /usr/local/simon
	cp libsimon.a simon.h /usr/local/simon

uninstall:
	rm -r /usr/local/simon

clean:
	rm -f libsimon.a *.o

