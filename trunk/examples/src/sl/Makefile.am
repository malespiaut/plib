if BUILD_SL

noinst_PROGRAMS = example mod_demo playsound

example_SOURCES  = example.cxx
mod_demo_SOURCES = mod_demo.cxx
playsound_SOURCES = playsound.cxx

example_LDADD   = -lplibsm -lplibsl -lplibul $(SND_LIBS) -lm
mod_demo_LDADD  = -lplibsm -lplibsl -lplibul $(SND_LIBS) -lm
playsound_LDADD = -lplibsm -lplibsl -lplibul $(SND_LIBS) -lm

endif

EXTRA_DIST = cuckoo.au scream.ub wheeee.ub zzap.wav tuxr.mod \
             mod_demo.dsp example.dsp

