# Microsoft Developer Studio Project File - Name="aprapp" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=aprapp - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "aprapp.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "aprapp.mak" CFG="aprapp - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "aprapp - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "aprapp - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "aprapp - Win32 Release9x" (based on "Win32 (x86) Static Library")
!MESSAGE "aprapp - Win32 Debug9x" (based on "Win32 (x86) Static Library")
!MESSAGE "aprapp - x64 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "aprapp - x64 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "aprapp - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "..\LibR"
# PROP BASE Intermediate_Dir "LibR"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\LibR"
# PROP Intermediate_Dir "LibR"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FD /c
# ADD CPP /nologo /MD /W3 /O2 /Oy- /Zi /I "../include" /I "../include/arch" /I "../include/arch/win32" /I "../include/arch/unix" /D "NDEBUG" /D "WINNT" /D "WIN32" /D "_WINDOWS" /D APR_APP /D "APR_DECLARE_STATIC" /Fo"$(INTDIR)\" /Fd"$(OUTDIR)\aprapp-1" /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\LibR\aprapp-1.lib"

!ELSEIF  "$(CFG)" == "aprapp - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "..\LibD"
# PROP BASE Intermediate_Dir "LibD"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\LibD"
# PROP Intermediate_Dir "LibD"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /EHsc /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FD /c
# ADD CPP /nologo /MDd /W3 /EHsc /Zi /Od /I "../include" /I "../include/arch" /I "../include/arch/win32" /I "../include/arch/unix" /D "_DEBUG" /D "WINNT" /D "WIN32" /D "_WINDOWS" /D APR_APP /D "APR_DECLARE_STATIC" /Fo"$(INTDIR)\" /Fd"$(OUTDIR)\aprapp-1" /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\LibD\aprapp-1.lib"

!ELSEIF  "$(CFG)" == "aprapp - Win32 Release9x"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "..\9x\LibR"
# PROP BASE Intermediate_Dir "9x\LibR"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\9x\LibR"
# PROP Intermediate_Dir "9x\LibR"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FD /c
# ADD CPP /nologo /MD /W3 /O2 /Oy- /Zi /I "../include" /I "../include/arch" /I "../include/arch/win32" /I "../include/arch/unix" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D APR_APP /D "APR_DECLARE_STATIC" /Fo"$(INTDIR)\" /Fd"$(OUTDIR)\aprapp-1" /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\9x\LibR\aprapp-1.lib"

!ELSEIF  "$(CFG)" == "aprapp - Win32 Debug9x"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "..\9x\LibD"
# PROP BASE Intermediate_Dir "9x\LibD"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\9x\LibD"
# PROP Intermediate_Dir "9x\LibD"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /EHsc /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FD /c
# ADD CPP /nologo /MDd /W3 /EHsc /Zi /Od /I "../include" /I "../include/arch" /I "../include/arch/win32" /I "../include/arch/unix" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D APR_APP /D "APR_DECLARE_STATIC" /Fo"$(INTDIR)\" /Fd"$(OUTDIR)\aprapp-1" /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\9x\LibD\aprapp-1.lib"

!ELSEIF  "$(CFG)" == "aprapp - x64 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "..\x64\LibR"
# PROP BASE Intermediate_Dir "x64\LibR"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\x64\LibR"
# PROP Intermediate_Dir "x64\LibR"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FD /c
# ADD CPP /nologo /MD /W3 /O2 /Oy- /Zi /I "../include" /I "../include/arch" /I "../include/arch/win32" /I "../include/arch/unix" /D "NDEBUG" /D "WINNT" /D "WIN32" /D "_WINDOWS" /D APR_APP /D "APR_DECLARE_STATIC" /Fo"$(INTDIR)\" /Fd"$(OUTDIR)\aprapp-1" /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\x64\LibR\aprapp-1.lib"

!ELSEIF  "$(CFG)" == "aprapp - x64 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "..\x64\LibD"
# PROP BASE Intermediate_Dir "x64\LibD"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\x64\LibD"
# PROP Intermediate_Dir "x64\LibD"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /EHsc /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FD /c
# ADD CPP /nologo /MDd /W3 /EHsc /Zi /Od /I "../include" /I "../include/arch" /I "../include/arch/win32" /I "../include/arch/unix" /D "_DEBUG" /D "WINNT" /D "WIN32" /D "_WINDOWS" /D APR_APP /D "APR_DECLARE_STATIC" /Fo"$(INTDIR)\" /Fd"$(OUTDIR)\aprapp-1" /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\x64\LibD\aprapp-1.lib"

!ENDIF 

# Begin Target

# Name "aprapp - Win32 Release"
# Name "aprapp - Win32 Debug"
# Name "aprapp - Win32 Release9x"
# Name "aprapp - Win32 Debug9x"
# Name "aprapp - x64 Release"
# Name "aprapp - x64 Debug"
# Begin Source File

SOURCE=..\misc\win32\apr_app.c
# End Source File
# End Target
# End Project
