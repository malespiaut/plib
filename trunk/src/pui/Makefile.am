if BUILD_PUI

lib_LIBRARIES = libplibpu.a

include_HEADERS = pu.h puGLUT.h puFLTK.h puSDL.h puNative.h puPW.h

libplibpu_a_SOURCES = \
        pu.cxx            puBox.cxx       puButton.cxx puButtonBox.cxx    \
        puArrowButton.cxx puDialogBox.cxx puFrame.cxx  puGroup.cxx        \
        puInput.cxx       puInterface.cxx puLocal.h    puMenuBar.cxx      \
        puObject.cxx      puOneShot.cxx   puPopup.cxx  puPopupMenu.cxx    \
        puListBox.cxx   puSlider.cxx puText.cxx         \
        puValue.cxx       puFont.cxx      \
        puDial.cxx        \
        puRange.cxx	  \
        puInputBase.cxx

AM_CPPFLAGS = -I$(top_srcdir)/src/sg -I$(top_srcdir)/src/fnt
AM_CPPFLAGS += -I$(top_srcdir)/src/util

endif

EXTRA_DIST = pui.dsp
