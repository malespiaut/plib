# -*-Makefile-*- $Id$


CXXFLAGS = $(shell fltk-config --use-gl --cxxflags) -Wall -O2 -DPU_USE_FLTK
LDFLAGS  = $(shell fltk-config --use-gl --ldflags) -lplibpu -lplibfnt -lplibul

OBJS = main.o gui.o puf.o


fltk_demo: $(OBJS)
	g++ $(OBJS) $(LDFLAGS) -o $@

clean:
	rm -f gui.{h,cxx} *.o *~


%.o: %.cxx
	g++ $(CXXFLAGS) -c $<

%.h %.cxx: %.fl	
	fluid -c $<


main.o gui.o: gui.h puf.h
puf.o: puf.h
