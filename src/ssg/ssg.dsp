# Microsoft Developer Studio Project File - Name="ssg" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=ssg - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ssg.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ssg.mak" CFG="ssg - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ssg - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "ssg - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "ssg"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ssg - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "..\sg" /I "..\util" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "ssg - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /GX /ZI /Od /I "..\sg" /I "..\util" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "ssg - Win32 Release"
# Name "ssg - Win32 Debug"
# Begin Source File

SOURCE=.\ssg.cxx
# End Source File
# Begin Source File

SOURCE=.\ssg.h
# End Source File
# Begin Source File

SOURCE=.\ssgAnimation.cxx
# End Source File
# Begin Source File

SOURCE=.\ssgBase.cxx
# End Source File
# Begin Source File

SOURCE=.\ssgBaseTransform.cxx
# End Source File
# Begin Source File

SOURCE=.\ssgBranch.cxx
# End Source File
# Begin Source File

SOURCE=.\ssgContext.cxx
# End Source File
# Begin Source File

SOURCE=.\ssgCutout.cxx
# End Source File
# Begin Source File

SOURCE=.\ssgDList.cxx
# End Source File
# Begin Source File

SOURCE=.\ssgEntity.cxx
# End Source File
# Begin Source File

SOURCE=.\ssgImageLoader.cxx
# End Source File
# Begin Source File

SOURCE=.\ssgInvisible.cxx
# End Source File
# Begin Source File

SOURCE=.\ssgIO.cxx
# End Source File
# Begin Source File

SOURCE=.\ssgIsect.cxx
# End Source File
# Begin Source File

SOURCE=.\ssgKeyFlier.h
# End Source File
# Begin Source File

SOURCE=.\ssgLeaf.cxx
# End Source File
# Begin Source File

SOURCE=.\ssgList.cxx
# End Source File
# Begin Source File

SOURCE=.\ssgLoad3ds.cxx
# End Source File
# Begin Source File

SOURCE=.\ssgLoadAC.cxx
# End Source File
# Begin Source File

SOURCE=.\ssgLoadASE.cxx
# End Source File
# Begin Source File

SOURCE=.\ssgLoadDXF.cxx
# End Source File
# Begin Source File

SOURCE=.\ssgLoaderWriterStuff.cxx
# End Source File
# Begin Source File

SOURCE=.\ssgLoaderWriterStuff.h
# End Source File
# Begin Source File

SOURCE=.\ssgLoadFLT.cxx
# End Source File
# Begin Source File

SOURCE=.\ssgLoadmd2.cxx
# End Source File
# Begin Source File

SOURCE=.\ssgLoadMDL.cxx
# End Source File
# Begin Source File

SOURCE=.\ssgLoadMDL.h
# End Source File
# Begin Source File

SOURCE=.\ssgLoadOBJ.cxx
# End Source File
# Begin Source File

SOURCE=.\ssgLoadSSG.cxx
# End Source File
# Begin Source File

SOURCE=.\ssgLoadTRI.cxx
# End Source File
# Begin Source File

SOURCE=.\ssgLoadVRML.cxx
# End Source File
# Begin Source File

SOURCE=.\ssgLoadX.cxx
# End Source File
# Begin Source File

SOURCE=.\ssgLocal.h
# End Source File
# Begin Source File

SOURCE=.\ssgMSFSPalette.h
# End Source File
# Begin Source File

SOURCE=.\ssgOptimiser.cxx
# End Source File
# Begin Source File

SOURCE=.\ssgParser.cxx
# End Source File
# Begin Source File

SOURCE=.\ssgParser.h
# End Source File
# Begin Source File

SOURCE=.\ssgRangeSelector.cxx
# End Source File
# Begin Source File

SOURCE=.\ssgRoot.cxx
# End Source File
# Begin Source File

SOURCE=.\ssgSaveAC.cxx
# End Source File
# Begin Source File

SOURCE=.\ssgSaveASE.cxx
# End Source File
# Begin Source File

SOURCE=.\ssgSaveDXF.cxx
# End Source File
# Begin Source File

SOURCE=.\ssgSaveOBJ.cxx
# End Source File
# Begin Source File

SOURCE=.\ssgSaveTRI.cxx
# End Source File
# Begin Source File

SOURCE=.\ssgSaveX.cxx
# End Source File
# Begin Source File

SOURCE=.\ssgSelector.cxx
# End Source File
# Begin Source File

SOURCE=.\ssgSimpleList.cxx
# End Source File
# Begin Source File

SOURCE=.\ssgSimpleState.cxx
# End Source File
# Begin Source File

SOURCE=.\ssgState.cxx
# End Source File
# Begin Source File

SOURCE=.\ssgStateSelector.cxx
# End Source File
# Begin Source File

SOURCE=.\ssgStateTables.cxx
# End Source File
# Begin Source File

SOURCE=.\ssgStats.cxx
# End Source File
# Begin Source File

SOURCE=.\ssgTexTrans.cxx
# End Source File
# Begin Source File

SOURCE=.\ssgTexture.cxx
# End Source File
# Begin Source File

SOURCE=.\ssgTransform.cxx
# End Source File
# Begin Source File

SOURCE=.\ssgVTable.cxx
# End Source File
# Begin Source File

SOURCE=.\ssgVtxArray.cxx
# End Source File
# Begin Source File

SOURCE=.\ssgVtxTable.cxx
# End Source File
# End Target
# End Project
