# Microsoft Developer Studio Project File - Name="expat" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=expat - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "expat.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "expat.mak" CFG="expat - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "expat - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "expat - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "expat - Win32 Release"

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
# ADD BASE CPP /nologo /MT /W3 /EHsc /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "EXPAT_EXPORTS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MT /W3 /EHsc /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "EXPAT_EXPORTS" /D "COMPILED_FROM_DSP" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll
# Begin Special Build Tool
TargetPath=.\Release\expat.dll
SOURCE="$(InputPath)"
PostBuild_Desc=Embed .manifest
PostBuild_Cmds=if exist $(TargetPath).manifest mt.exe -manifest $(TargetPath).manifest -outputresource:$(TargetPath);2
# End Special Build Tool

!ELSEIF  "$(CFG)" == "expat - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /EHsc /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "EXPAT_EXPORTS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /EHsc /Zi /Od /D "_DEBUG" /D "COMPILED_FROM_DSP" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "EXPAT_EXPORTS" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /pdbtype:sept
# Begin Special Build Tool
TargetPath=.\Debug\expat.dll
SOURCE="$(InputPath)"
PostBuild_Desc=Embed .manifest
PostBuild_Cmds=if exist $(TargetPath).manifest mt.exe -manifest $(TargetPath).manifest -outputresource:$(TargetPath);2
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "expat - Win32 Release"
# Name "expat - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\xmlparse.c

!IF  "$(CFG)" == "expat - Win32 Release"

# ADD CPP /D VERSION=\"expat_1.95.7\"

!ELSEIF  "$(CFG)" == "expat - Win32 Debug"

# ADD CPP /EHsc- /Od /D VERSION=\"expat_1.95.7\"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\xmlrole.c

!IF  "$(CFG)" == "expat - Win32 Release"

# ADD CPP /D VERSION=\"expat_1.95.7\"

!ELSEIF  "$(CFG)" == "expat - Win32 Debug"

# ADD CPP /D VERSION=\"expat_1.95.7\"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\xmltok.c

!IF  "$(CFG)" == "expat - Win32 Release"

# ADD CPP /D VERSION=\"expat_1.95.7\"

!ELSEIF  "$(CFG)" == "expat - Win32 Debug"

# ADD CPP /D VERSION=\"expat_1.95.7\"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=xmltok_impl.c

!IF  "$(CFG)" == "expat - Win32 Release"

# ADD CPP /D VERSION=\"expat_1.95.7\"

!ELSEIF  "$(CFG)" == "expat - Win32 Debug"

# PROP Exclude_From_Build 1
# ADD CPP /D VERSION=\"expat_1.95.7\"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=xmltok_ns.c

!IF  "$(CFG)" == "expat - Win32 Release"

# ADD CPP /D VERSION=\"expat_1.95.7\"

!ELSEIF  "$(CFG)" == "expat - Win32 Debug"

# PROP Exclude_From_Build 1
# ADD CPP /D VERSION=\"expat_1.95.7\"

!ENDIF 

# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\ascii.h
# End Source File
# Begin Source File

SOURCE=.\asciitab.h
# End Source File
# Begin Source File

SOURCE=.\config.h
# End Source File
# Begin Source File

SOURCE=.\expat.h
# End Source File
# Begin Source File

SOURCE=.\iasciitab.h
# End Source File
# Begin Source File

SOURCE=.\latin1tab.h
# End Source File
# Begin Source File

SOURCE=.\nametab.h
# End Source File
# Begin Source File

SOURCE=.\utf8tab.h
# End Source File
# Begin Source File

SOURCE=.\xmlrole.h
# End Source File
# Begin Source File

SOURCE=.\xmltok.h
# End Source File
# Begin Source File

SOURCE=.\xmltok_impl.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
