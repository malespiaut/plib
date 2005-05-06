# Microsoft Developer Studio Project File - Name="puAux" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=puAux - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "puAux.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "puAux.mak" CFG="puAux - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "puAux - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "puAux - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "puAux - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
LINK32=link.exe -lib
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /Zi /O2 /I "..\fnt" /I "..\sg" /I "..\util" /I ".." /I "..\pui" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "PU_USE_NONE" /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Desc=Copy Library to plib directory
PostBuild_Cmds=copy  release\*.lib  ..\..\*.* 	copy  puAux.h  ..\..\puAux.h
# End Special Build Tool

!ELSEIF  "$(CFG)" == "puAux - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
LINK32=link.exe -lib
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /GX /Zi /Od /I "..\fnt" /I "..\sg" /I "..\util" /I ".." /I "..\pui" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "PU_USE_NONE" /FR /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"Debug\puAux_d.lib"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Desc=Copy Library to plib directory
PostBuild_Cmds=copy  debug\*.lib  ..\..\*.* 	copy  puAux.h  ..\..\puAux.h
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "puAux - Win32 Release"
# Name "puAux - Win32 Debug"
# Begin Source File

SOURCE=.\puAux.cxx
# End Source File
# Begin Source File

SOURCE=.\puAux.h
# End Source File
# Begin Source File

SOURCE=.\puAuxBiSlider.cxx
# End Source File
# Begin Source File

SOURCE=.\puAuxBiSliderWithEnds.cxx
# End Source File
# Begin Source File

SOURCE=.\puAuxComboBox.cxx
# End Source File
# Begin Source File

SOURCE=.\puAuxCompass.cxx
# End Source File
# Begin Source File

SOURCE=.\puAuxFileSelector.cxx
# End Source File
# Begin Source File

SOURCE=.\puAuxLargeInput.cxx
# End Source File
# Begin Source File

SOURCE=.\puAuxList.cxx
# End Source File
# Begin Source File

SOURCE=.\puAuxLocal.h
# End Source File
# Begin Source File

SOURCE=.\puAuxScrollBar.cxx
# End Source File
# Begin Source File

SOURCE=.\puAuxSelectBox.cxx
# End Source File
# Begin Source File

SOURCE=.\puAuxSliderWithInput.cxx
# End Source File
# Begin Source File

SOURCE=.\puAuxSpinBox.cxx
# End Source File
# Begin Source File

SOURCE=.\puAuxTriSlider.cxx
# End Source File
# Begin Source File

SOURCE=.\puAuxVerticalMenu.cxx
# End Source File
# End Target
# End Project
