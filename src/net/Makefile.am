lib_LIBRARIES = libplibnet.a

include_HEADERS = netBuffer.h netChannel.h netChat.h netMessage.h \
	netMonitor.h netRoom.h netSocket.h netList.h

libplibnet_a_SOURCES = netBuffer.cxx netChannel.cxx netChat.cxx \
	netMessage.cxx netMonitor.cxx netRoom.cxx netRoomClient.cxx \
	netRoomMaster.cxx netRoomServer.cxx netSocket.cxx

INCLUDES += -I$(top_srcdir)/src/util

EXTRA_DIST = net.dsp

