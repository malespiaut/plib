if BUILD_SL

noinst_PROGRAMS = example mod_demo

example_SOURCES  = example.cxx
mod_demo_SOURCES = mod_demo.cxx

example_LDADD  = -lplibsm -lplibsl -lplibul $(SND_LIBS)
mod_demo_LDADD = -lplibsm -lplibsl -lplibul $(SND_LIBS)

endif

EXTRA_DIST = cuckoo.au scream.ub wheeee.ub zzap.wav tuxr.mod \
             mod_demo.dsp example.dsp

