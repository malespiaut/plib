# Microsoft Developer Studio Project File - Name="ode_demo" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 60000
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=ode_demo - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ode_demo.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ode_demo.mak" CFG="ode_demo - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ode_demo - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "ode_demo - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=snCl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ode_demo - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /O2 /I "." /I "..\.." /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0xc09 /d "NDEBUG"
# ADD RSC /l 0xc09 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=snLink.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "ode_demo - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /ZI /Od /I "." /I "..\..\.." /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD BASE RSC /l 0xc09 /d "_DEBUG"
# ADD RSC /l 0xc09 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=snLink.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ssgAux_d.lib ssg_d.lib sg_d.lib ul_d.lib pui_d.lib fnt_d.lib /nologo /subsystem:console /debug /machine:I386 /out:"ode_demo.exe" /pdbtype:sept /libpath:"..\.."

!ENDIF 

# Begin Target

# Name "ode_demo - Win32 Release"
# Name "ode_demo - Win32 Debug"
# Begin Group "ode"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ode\array.cpp
# End Source File
# Begin Source File

SOURCE=.\ode\array.h
# End Source File
# Begin Source File

SOURCE=.\ode\common.h
# End Source File
# Begin Source File

SOURCE=.\ode\config.h
# End Source File
# Begin Source File

SOURCE=.\ode\contact.h
# End Source File
# Begin Source File

SOURCE=.\ode\dRay.cpp
# End Source File
# Begin Source File

SOURCE=.\ode\dRay.h
# End Source File
# Begin Source File

SOURCE=.\ode\dRay_Box.cpp
# End Source File
# Begin Source File

SOURCE=.\ode\dRay_CCylinder.cpp
# End Source File
# Begin Source File

SOURCE=.\ode\dRay_Plane.cpp
# End Source File
# Begin Source File

SOURCE=.\ode\dRay_Sphere.cpp
# End Source File
# Begin Source File

SOURCE=.\ode\dTriList.cpp
# End Source File
# Begin Source File

SOURCE=.\ode\dTriList.h
# End Source File
# Begin Source File

SOURCE=.\ode\dTriList_Box.cpp
# End Source File
# Begin Source File

SOURCE=.\ode\dTriList_CCylinder.cpp
# End Source File
# Begin Source File

SOURCE=.\ode\dTriList_Plane.cpp
# End Source File
# Begin Source File

SOURCE=.\ode\dTriList_Ray.cpp
# End Source File
# Begin Source File

SOURCE=.\ode\dTriList_Sphere.cpp
# End Source File
# Begin Source File

SOURCE=.\ode\dTriList_TriList.cpp
# End Source File
# Begin Source File

SOURCE=.\ode\dxRay.h
# End Source File
# Begin Source File

SOURCE=.\ode\dxTriList.h
# End Source File
# Begin Source File

SOURCE=.\ode\error.cpp
# End Source File
# Begin Source File

SOURCE=.\ode\error.h
# End Source File
# Begin Source File

SOURCE=.\ode\fastdot.c
# End Source File
# Begin Source File

SOURCE=.\ode\fastldlt.c
# End Source File
# Begin Source File

SOURCE=.\ode\fastlsolve.c
# End Source File
# Begin Source File

SOURCE=.\ode\fastltsolve.c
# End Source File
# Begin Source File

SOURCE=.\ode\geom.cpp
# End Source File
# Begin Source File

SOURCE=.\ode\geom.h
# End Source File
# Begin Source File

SOURCE=.\ode\geom_internal.h
# End Source File
# Begin Source File

SOURCE=.\ode\joint.cpp
# End Source File
# Begin Source File

SOURCE=.\ode\joint.h
# End Source File
# Begin Source File

SOURCE=.\ode\lcp.cpp
# End Source File
# Begin Source File

SOURCE=.\ode\lcp.h
# End Source File
# Begin Source File

SOURCE=.\ode\mass.cpp
# End Source File
# Begin Source File

SOURCE=.\ode\mass.h
# End Source File
# Begin Source File

SOURCE=.\ode\mat.cpp
# End Source File
# Begin Source File

SOURCE=.\ode\mat.h
# End Source File
# Begin Source File

SOURCE=.\ode\matrix.cpp
# End Source File
# Begin Source File

SOURCE=.\ode\matrix.h
# End Source File
# Begin Source File

SOURCE=.\ode\memory.cpp
# End Source File
# Begin Source File

SOURCE=.\ode\memory.h
# End Source File
# Begin Source File

SOURCE=.\ode\misc.cpp
# End Source File
# Begin Source File

SOURCE=.\ode\misc.h
# End Source File
# Begin Source File

SOURCE=.\ode\objects.h
# End Source File
# Begin Source File

SOURCE=.\ode\objects_internal.h
# End Source File
# Begin Source File

SOURCE=.\ode\obstack.cpp
# End Source File
# Begin Source File

SOURCE=.\ode\obstack.h
# End Source File
# Begin Source File

SOURCE=.\ode\ode.cpp
# End Source File
# Begin Source File

SOURCE=.\ode\ode.h
# End Source File
# Begin Source File

SOURCE=.\ode\odecpp.h
# End Source File
# Begin Source File

SOURCE=.\ode\odemath.cpp
# End Source File
# Begin Source File

SOURCE=.\ode\odemath.h
# End Source File
# Begin Source File

SOURCE=.\ode\rotation.cpp
# End Source File
# Begin Source File

SOURCE=.\ode\rotation.h
# End Source File
# Begin Source File

SOURCE=.\ode\space.cpp
# End Source File
# Begin Source File

SOURCE=.\ode\space.h
# End Source File
# Begin Source File

SOURCE=.\ode\step.cpp
# End Source File
# Begin Source File

SOURCE=.\ode\step.h
# End Source File
# Begin Source File

SOURCE=.\ode\testing.cpp
# End Source File
# Begin Source File

SOURCE=.\ode\testing.h
# End Source File
# Begin Source File

SOURCE=.\ode\timer.cpp
# End Source File
# Begin Source File

SOURCE=.\ode\timer.h
# End Source File
# End Group
# Begin Group "opcode"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\opcode\OPC_AABB.cpp
# End Source File
# Begin Source File

SOURCE=.\opcode\OPC_AABB.h
# End Source File
# Begin Source File

SOURCE=.\opcode\OPC_AABBCollider.cpp
# End Source File
# Begin Source File

SOURCE=.\opcode\OPC_AABBCollider.h
# End Source File
# Begin Source File

SOURCE=.\opcode\OPC_AABBTree.cpp
# End Source File
# Begin Source File

SOURCE=.\opcode\OPC_AABBTree.h
# End Source File
# Begin Source File

SOURCE=.\opcode\OPC_BoundingSphere.h
# End Source File
# Begin Source File

SOURCE=.\opcode\OPC_BoxBoxOverlap.h
# End Source File
# Begin Source File

SOURCE=.\opcode\OPC_BVTCache.h
# End Source File
# Begin Source File

SOURCE=.\opcode\OPC_Collider.cpp
# End Source File
# Begin Source File

SOURCE=.\opcode\OPC_Collider.h
# End Source File
# Begin Source File

SOURCE=.\opcode\OPC_Common.cpp
# End Source File
# Begin Source File

SOURCE=.\opcode\OPC_Common.h
# End Source File
# Begin Source File

SOURCE=.\opcode\OPC_Container.cpp
# End Source File
# Begin Source File

SOURCE=.\opcode\OPC_Container.h
# End Source File
# Begin Source File

SOURCE=.\opcode\OPC_FPU.h
# End Source File
# Begin Source File

SOURCE=.\opcode\OPC_HPoint.h
# End Source File
# Begin Source File

SOURCE=.\opcode\OPC_Matrix3x3.cpp
# End Source File
# Begin Source File

SOURCE=.\opcode\OPC_Matrix3x3.h
# End Source File
# Begin Source File

SOURCE=.\opcode\OPC_Matrix4x4.cpp
# End Source File
# Begin Source File

SOURCE=.\opcode\OPC_Matrix4x4.h
# End Source File
# Begin Source File

SOURCE=.\opcode\OPC_MemoryMacros.h
# End Source File
# Begin Source File

SOURCE=.\opcode\OPC_Model.cpp
# End Source File
# Begin Source File

SOURCE=.\opcode\OPC_Model.h
# End Source File
# Begin Source File

SOURCE=.\opcode\OPC_OBB.cpp
# End Source File
# Begin Source File

SOURCE=.\opcode\OPC_OBB.h
# End Source File
# Begin Source File

SOURCE=.\opcode\OPC_OBBCollider.cpp
# End Source File
# Begin Source File

SOURCE=.\opcode\OPC_OBBCollider.h
# End Source File
# Begin Source File

SOURCE=.\opcode\OPC_OptimizedTree.cpp
# End Source File
# Begin Source File

SOURCE=.\opcode\OPC_OptimizedTree.h
# End Source File
# Begin Source File

SOURCE=.\opcode\OPC_Plane.cpp
# End Source File
# Begin Source File

SOURCE=.\opcode\OPC_Plane.h
# End Source File
# Begin Source File

SOURCE=.\opcode\OPC_PlanesAABBOverlap.h
# End Source File
# Begin Source File

SOURCE=.\opcode\OPC_PlanesCollider.cpp
# End Source File
# Begin Source File

SOURCE=.\opcode\OPC_PlanesCollider.h
# End Source File
# Begin Source File

SOURCE=.\opcode\OPC_PlanesTriOverlap.h
# End Source File
# Begin Source File

SOURCE=.\opcode\OPC_Point.cpp
# End Source File
# Begin Source File

SOURCE=.\opcode\OPC_Point.h
# End Source File
# Begin Source File

SOURCE=.\opcode\OPC_Preprocessor.h
# End Source File
# Begin Source File

SOURCE=.\opcode\OPC_Ray.cpp
# End Source File
# Begin Source File

SOURCE=.\opcode\OPC_Ray.h
# End Source File
# Begin Source File

SOURCE=.\opcode\OPC_RayAABBOverlap.h
# End Source File
# Begin Source File

SOURCE=.\opcode\OPC_RayCollider.cpp
# End Source File
# Begin Source File

SOURCE=.\opcode\OPC_RayCollider.h
# End Source File
# Begin Source File

SOURCE=.\opcode\OPC_RayTriOverlap.h
# End Source File
# Begin Source File

SOURCE=.\opcode\OPC_Settings.h
# End Source File
# Begin Source File

SOURCE=.\opcode\OPC_SphereAABBOverlap.h
# End Source File
# Begin Source File

SOURCE=.\opcode\OPC_SphereCollider.cpp
# End Source File
# Begin Source File

SOURCE=.\opcode\OPC_SphereCollider.h
# End Source File
# Begin Source File

SOURCE=.\opcode\OPC_SphereTriOverlap.h
# End Source File
# Begin Source File

SOURCE=.\opcode\OPC_TreeBuilders.cpp
# End Source File
# Begin Source File

SOURCE=.\opcode\OPC_TreeBuilders.h
# End Source File
# Begin Source File

SOURCE=.\opcode\OPC_TreeCollider.cpp
# End Source File
# Begin Source File

SOURCE=.\opcode\OPC_TreeCollider.h
# End Source File
# Begin Source File

SOURCE=.\opcode\OPC_Triangle.cpp
# End Source File
# Begin Source File

SOURCE=.\opcode\OPC_Triangle.h
# End Source File
# Begin Source File

SOURCE=.\opcode\OPC_TriBoxOverlap.h
# End Source File
# Begin Source File

SOURCE=.\opcode\OPC_TriTriOverlap.h
# End Source File
# Begin Source File

SOURCE=.\opcode\OPC_Types.h
# End Source File
# Begin Source File

SOURCE=.\opcode\OPC_VolumeCollider.cpp
# End Source File
# Begin Source File

SOURCE=.\opcode\OPC_VolumeCollider.h
# End Source File
# Begin Source File

SOURCE=.\opcode\Opcode.cpp
# End Source File
# Begin Source File

SOURCE=.\opcode\Opcode.h
# End Source File
# Begin Source File

SOURCE=.\opcode\StdAfx.cpp
# End Source File
# Begin Source File

SOURCE=.\opcode\StdAfx.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\simple.cpp
# End Source File
# End Target
# End Project
