# Microsoft Developer Studio Project File - Name="libaprapp" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=libaprapp - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "libaprapp.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "libaprapp.mak" CFG="libaprapp - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "libaprapp - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "libaprapp - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "libaprapp - Win32 Release9x" (based on "Win32 (x86) Static Library")
!MESSAGE "libaprapp - Win32 Debug9x" (based on "Win32 (x86) Static Library")
!MESSAGE "libaprapp - x64 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "libaprapp - x64 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "libaprapp - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "..\Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FD /c
# ADD CPP /nologo /MD /W3 /O2 /Oy- /Zi /I "../include" /I "../include/arch" /I "../include/arch/win32" /I "../include/arch/unix" /D "NDEBUG" /D "WINNT" /D "WIN32" /D "_WINDOWS" /D "APR_APP" /Fo"$(INTDIR)\" /Fd"$(OUTDIR)\libaprapp-1" /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\Release\libaprapp-1.lib"

!ELSEIF  "$(CFG)" == "libaprapp - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "..\Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /EHsc /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FD /c
# ADD CPP /nologo /MDd /W3 /EHsc /Zi /Od /I "../include" /I "../include/arch" /I "../include/arch/win32" /I "../include/arch/unix" /D "_DEBUG" /D "WINNT" /D "WIN32" /D "_WINDOWS" /D "APR_APP" /Fo"$(INTDIR)\" /Fd"$(OUTDIR)\libaprapp-1" /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\Debug\libaprapp-1.lib"

!ELSEIF  "$(CFG)" == "libaprapp - Win32 Release9x"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "..\9x\Release"
# PROP BASE Intermediate_Dir "9x\Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\9x\Release"
# PROP Intermediate_Dir "9x\Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FD /c
# ADD CPP /nologo /MD /W3 /O2 /Oy- /Zi /I "../include" /I "../include/arch" /I "../include/arch/win32" /I "../include/arch/unix" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "APR_APP" /Fo"$(INTDIR)\" /Fd"$(OUTDIR)\libaprapp-1" /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\9x\Release\libaprapp-1.lib"

!ELSEIF  "$(CFG)" == "libaprapp - Win32 Debug9x"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "..\9x\Debug"
# PROP BASE Intermediate_Dir "9x\Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\9x\Debug"
# PROP Intermediate_Dir "9x\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /EHsc /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FD /c
# ADD CPP /nologo /MDd /W3 /EHsc /Zi /Od /I "../include" /I "../include/arch" /I "../include/arch/win32" /I "../include/arch/unix" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "APR_APP" /Fo"$(INTDIR)\" /Fd"$(OUTDIR)\libaprapp-1" /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\9x\Debug\libaprapp-1.lib"

!ELSEIF  "$(CFG)" == "libaprapp - x64 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "..\x64\Release"
# PROP BASE Intermediate_Dir "x64\Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\x64\Release"
# PROP Intermediate_Dir "x64\Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FD /c
# ADD CPP /nologo /MD /W3 /O2 /Oy- /Zi /I "../include" /I "../include/arch" /I "../include/arch/win32" /I "../include/arch/unix" /D "NDEBUG" /D "WINNT" /D "WIN32" /D "_WINDOWS" /D "APR_APP" /Fo"$(INTDIR)\" /Fd"$(OUTDIR)\libaprapp-1" /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\x64\Release\libaprapp-1.lib"

!ELSEIF  "$(CFG)" == "libaprapp - x64 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "..\x64\Debug"
# PROP BASE Intermediate_Dir "x64\Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\x64\Debug"
# PROP Intermediate_Dir "x64\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /EHsc /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FD /c
# ADD CPP /nologo /MDd /W3 /EHsc /Zi /Od /I "../include" /I "../include/arch" /I "../include/arch/win32" /I "../include/arch/unix" /D "_DEBUG" /D "WINNT" /D "WIN32" /D "_WINDOWS" /D "APR_APP" /Fo"$(INTDIR)\" /Fd"$(OUTDIR)\libaprapp-1" /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\x64\Debug\libaprapp-1.lib"

!ENDIF 

# Begin Target

# Name "libaprapp - Win32 Release"
# Name "libaprapp - Win32 Debug"
# Name "libaprapp - Win32 Release9x"
# Name "libaprapp - Win32 Debug9x"
# Name "libaprapp - x64 Release"
# Name "libaprapp - x64 Debug"
# Begin Source File

SOURCE=..\misc\win32\apr_app.c
# End Source File
# Begin Source File

SOURCE=..\misc\win32\internal.c
# End Source File
# End Target
# End Project
