# Microsoft Developer Studio Project File - Name="lib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=lib - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "lib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "lib.mak" CFG="lib - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "lib - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "lib - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe

!IF  "$(CFG)" == "lib - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "../../../../src/include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "__MSC__" /YX /FD /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"p3dc.lib"

!ELSEIF  "$(CFG)" == "lib - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /Z7 /Od /I "../../../../src/include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "__MSC__" /D "PARANOID_LIST" /FR /YX /FD /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"p3dc.lib"

!ENDIF 

# Begin Target

# Name "lib - Win32 Release"
# Name "lib - Win32 Debug"
# Begin Source File

SOURCE=..\..\..\..\src\lib\camera.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\lib\capture.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\lib\clip.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\lib\display.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\lib\draw.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\lib\gal_glide2.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\lib\light.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\lib\list.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\lib\log.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\lib\memory.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\lib\model.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\lib\model_parse.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\include\p3dc.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\lib\primitives.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\lib\rbtree.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\lib\texture.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\lib\texture_md.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\lib\transform.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\lib\util.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\lib\vertex.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\lib\writepng.c
# End Source File
# End Target
# End Project
