lib_LIBRARIES = libplibsl.a libplibsm.a

include_HEADERS = sl.h slPortability.h sm.h slMODPrivate.h slMODfile.h

libplibsl_a_SOURCES = \
	slDSP.cxx slSample.cxx slEnvelope.cxx \
	slPlayer.cxx slMODPlayer.cxx slSamplePlayer.cxx \
        slScheduler.cxx slMODdacio.cxx slMODfile.cxx \
        slMODinst.cxx slMODnote.cxx

libplibsm_a_SOURCES = slPortability.h smMixer.cxx

EXTRA_DIST = Makefile.msc README.freebsd README.linux \
	README.openbsd README.sgi README.unix README.win \
        sl.dsp libsm.bpr libsl.bpr


