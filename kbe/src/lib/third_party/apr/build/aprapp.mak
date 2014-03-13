# Microsoft Developer Studio Generated NMAKE File, Based on aprapp.dsp
!IF "$(CFG)" == ""
CFG=aprapp - Win32 Release
!MESSAGE No configuration specified. Defaulting to aprapp - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "aprapp - Win32 Release" && "$(CFG)" != "aprapp - Win32 Debug" && "$(CFG)" != "aprapp - Win32 Release9x" && "$(CFG)" != "aprapp - Win32 Debug9x" && "$(CFG)" != "aprapp - x64 Release" && "$(CFG)" != "aprapp - x64 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
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
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "aprapp - Win32 Release"

OUTDIR=.\..\LibR
INTDIR=.\LibR
# Begin Custom Macros
OutDir=.\..\LibR
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\aprapp-1.lib"

!ELSE 

ALL : "preaprapp - Win32 Release" "$(OUTDIR)\aprapp-1.lib"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"preaprapp - Win32 ReleaseCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\apr_app.obj"
	-@erase "$(OUTDIR)\aprapp-1.lib"
	-@erase "..\LibR\aprapp-1.idb"
	-@erase "..\LibR\aprapp-1.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /Zi /O2 /Oy- /I "../include" /I "../include/arch" /I "../include/arch/win32" /I "../include/arch/unix" /D "NDEBUG" /D "WINNT" /D "WIN32" /D "_WINDOWS" /D "APR_APP" /D "APR_DECLARE_STATIC" /Fo"$(INTDIR)\\" /Fd"$(OUTDIR)\aprapp-1" /FD /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\aprapp.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\aprapp-1.lib" 
LIB32_OBJS= \
	"$(INTDIR)\apr_app.obj"

"$(OUTDIR)\aprapp-1.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "aprapp - Win32 Debug"

OUTDIR=.\..\LibD
INTDIR=.\LibD
# Begin Custom Macros
OutDir=.\..\LibD
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\aprapp-1.lib"

!ELSE 

ALL : "preaprapp - Win32 Debug" "$(OUTDIR)\aprapp-1.lib"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"preaprapp - Win32 DebugCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\apr_app.obj"
	-@erase "$(OUTDIR)\aprapp-1.lib"
	-@erase "..\LibD\aprapp-1.idb"
	-@erase "..\LibD\aprapp-1.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MDd /W3 /Zi /Od /I "../include" /I "../include/arch" /I "../include/arch/win32" /I "../include/arch/unix" /D "_DEBUG" /D "WINNT" /D "WIN32" /D "_WINDOWS" /D "APR_APP" /D "APR_DECLARE_STATIC" /Fo"$(INTDIR)\\" /Fd"$(OUTDIR)\aprapp-1" /FD /EHsc /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\aprapp.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\aprapp-1.lib" 
LIB32_OBJS= \
	"$(INTDIR)\apr_app.obj"

"$(OUTDIR)\aprapp-1.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "aprapp - Win32 Release9x"

OUTDIR=.\..\9x\LibR
INTDIR=.\9x\LibR
# Begin Custom Macros
OutDir=.\..\9x\LibR
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\aprapp-1.lib"

!ELSE 

ALL : "preaprapp - Win32 Release9x" "$(OUTDIR)\aprapp-1.lib"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"preaprapp - Win32 Release9xCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\apr_app.obj"
	-@erase "$(OUTDIR)\aprapp-1.lib"
	-@erase "..\9x\LibR\aprapp-1.idb"
	-@erase "..\9x\LibR\aprapp-1.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /Zi /O2 /Oy- /I "../include" /I "../include/arch" /I "../include/arch/win32" /I "../include/arch/unix" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "APR_APP" /D "APR_DECLARE_STATIC" /Fo"$(INTDIR)\\" /Fd"$(OUTDIR)\aprapp-1" /FD /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\aprapp.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\aprapp-1.lib" 
LIB32_OBJS= \
	"$(INTDIR)\apr_app.obj"

"$(OUTDIR)\aprapp-1.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "aprapp - Win32 Debug9x"

OUTDIR=.\..\9x\LibD
INTDIR=.\9x\LibD
# Begin Custom Macros
OutDir=.\..\9x\LibD
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\aprapp-1.lib"

!ELSE 

ALL : "preaprapp - Win32 Debug9x" "$(OUTDIR)\aprapp-1.lib"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"preaprapp - Win32 Debug9xCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\apr_app.obj"
	-@erase "$(OUTDIR)\aprapp-1.lib"
	-@erase "..\9x\LibD\aprapp-1.idb"
	-@erase "..\9x\LibD\aprapp-1.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MDd /W3 /Zi /Od /I "../include" /I "../include/arch" /I "../include/arch/win32" /I "../include/arch/unix" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "APR_APP" /D "APR_DECLARE_STATIC" /Fo"$(INTDIR)\\" /Fd"$(OUTDIR)\aprapp-1" /FD /EHsc /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\aprapp.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\aprapp-1.lib" 
LIB32_OBJS= \
	"$(INTDIR)\apr_app.obj"

"$(OUTDIR)\aprapp-1.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "aprapp - x64 Release"

OUTDIR=.\..\x64\LibR
INTDIR=.\x64\LibR
# Begin Custom Macros
OutDir=.\..\x64\LibR
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\aprapp-1.lib"

!ELSE 

ALL : "preaprapp - x64 Release" "$(OUTDIR)\aprapp-1.lib"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"preaprapp - x64 ReleaseCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\apr_app.obj"
	-@erase "$(OUTDIR)\aprapp-1.lib"
	-@erase "..\x64\LibR\aprapp-1.idb"
	-@erase "..\x64\LibR\aprapp-1.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /Zi /O2 /Oy- /I "../include" /I "../include/arch" /I "../include/arch/win32" /I "../include/arch/unix" /D "NDEBUG" /D "WINNT" /D "WIN32" /D "_WINDOWS" /D "APR_APP" /D "APR_DECLARE_STATIC" /Fo"$(INTDIR)\\" /Fd"$(OUTDIR)\aprapp-1" /FD /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\aprapp.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\aprapp-1.lib" 
LIB32_OBJS= \
	"$(INTDIR)\apr_app.obj"

"$(OUTDIR)\aprapp-1.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "aprapp - x64 Debug"

OUTDIR=.\..\x64\LibD
INTDIR=.\x64\LibD
# Begin Custom Macros
OutDir=.\..\x64\LibD
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\aprapp-1.lib"

!ELSE 

ALL : "preaprapp - x64 Debug" "$(OUTDIR)\aprapp-1.lib"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"preaprapp - x64 DebugCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\apr_app.obj"
	-@erase "$(OUTDIR)\aprapp-1.lib"
	-@erase "..\x64\LibD\aprapp-1.idb"
	-@erase "..\x64\LibD\aprapp-1.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MDd /W3 /Zi /Od /I "../include" /I "../include/arch" /I "../include/arch/win32" /I "../include/arch/unix" /D "_DEBUG" /D "WINNT" /D "WIN32" /D "_WINDOWS" /D "APR_APP" /D "APR_DECLARE_STATIC" /Fo"$(INTDIR)\\" /Fd"$(OUTDIR)\aprapp-1" /FD /EHsc /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\aprapp.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\aprapp-1.lib" 
LIB32_OBJS= \
	"$(INTDIR)\apr_app.obj"

"$(OUTDIR)\aprapp-1.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("aprapp.dep")
!INCLUDE "aprapp.dep"
!ELSE 
!MESSAGE Warning: cannot find "aprapp.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "aprapp - Win32 Release" || "$(CFG)" == "aprapp - Win32 Debug" || "$(CFG)" == "aprapp - Win32 Release9x" || "$(CFG)" == "aprapp - Win32 Debug9x" || "$(CFG)" == "aprapp - x64 Release" || "$(CFG)" == "aprapp - x64 Debug"

!IF  "$(CFG)" == "aprapp - Win32 Release"

"preaprapp - Win32 Release" : 
   cd "."
   NMAKE /nologo /f NUL
   cd "."

"preaprapp - Win32 ReleaseCLEAN" : 
   cd "."
   cd "."

!ELSEIF  "$(CFG)" == "aprapp - Win32 Debug"

"preaprapp - Win32 Debug" : 
   cd "."
   NMAKE /nologo /f NUL
   cd "."

"preaprapp - Win32 DebugCLEAN" : 
   cd "."
   cd "."

!ELSEIF  "$(CFG)" == "aprapp - Win32 Release9x"

"preaprapp - Win32 Release9x" : 
   cd "."
   NMAKE /nologo /f NUL
   cd "."

"preaprapp - Win32 Release9xCLEAN" : 
   cd "."
   cd "."

!ELSEIF  "$(CFG)" == "aprapp - Win32 Debug9x"

"preaprapp - Win32 Debug9x" : 
   cd "."
   NMAKE /nologo /f NUL
   cd "."

"preaprapp - Win32 Debug9xCLEAN" : 
   cd "."
   cd "."

!ELSEIF  "$(CFG)" == "aprapp - x64 Release"

"preaprapp - x64 Release" : 
   cd "."
   NMAKE /nologo /f NUL
   cd "."

"preaprapp - x64 ReleaseCLEAN" : 
   cd "."
   cd "."

!ELSEIF  "$(CFG)" == "aprapp - x64 Debug"

"preaprapp - x64 Debug" : 
   cd "."
   NMAKE /nologo /f NUL
   cd "."

"preaprapp - x64 DebugCLEAN" : 
   cd "."
   cd "."

!ENDIF 

SOURCE=..\misc\win32\apr_app.c

"$(INTDIR)\apr_app.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)



!ENDIF 

