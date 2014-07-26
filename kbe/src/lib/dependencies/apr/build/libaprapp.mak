# Microsoft Developer Studio Generated NMAKE File, Based on libaprapp.dsp
!IF "$(CFG)" == ""
CFG=libaprapp - Win32 Release
!MESSAGE No configuration specified. Defaulting to libaprapp - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "libaprapp - Win32 Release" && "$(CFG)" != "libaprapp - Win32 Debug" && "$(CFG)" != "libaprapp - Win32 Release9x" && "$(CFG)" != "libaprapp - Win32 Debug9x" && "$(CFG)" != "libaprapp - x64 Release" && "$(CFG)" != "libaprapp - x64 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
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
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "libaprapp - Win32 Release"

OUTDIR=.\..\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\..\Release
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\libaprapp-1.lib"

!ELSE 

ALL : "prelibaprapp - Win32 Release" "$(OUTDIR)\libaprapp-1.lib"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"prelibaprapp - Win32 ReleaseCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\apr_app.obj"
	-@erase "$(INTDIR)\internal.obj"
	-@erase "$(OUTDIR)\libaprapp-1.lib"
	-@erase "..\Release\libaprapp-1.idb"
	-@erase "..\Release\libaprapp-1.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /Zi /O2 /Oy- /I "../include" /I "../include/arch" /I "../include/arch/win32" /I "../include/arch/unix" /D "NDEBUG" /D "WINNT" /D "WIN32" /D "_WINDOWS" /D "APR_APP" /Fo"$(INTDIR)\\" /Fd"$(OUTDIR)\libaprapp-1" /FD /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\libaprapp.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\libaprapp-1.lib" 
LIB32_OBJS= \
	"$(INTDIR)\apr_app.obj" \
	"$(INTDIR)\internal.obj"

"$(OUTDIR)\libaprapp-1.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "libaprapp - Win32 Debug"

OUTDIR=.\..\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\..\Debug
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\libaprapp-1.lib"

!ELSE 

ALL : "prelibaprapp - Win32 Debug" "$(OUTDIR)\libaprapp-1.lib"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"prelibaprapp - Win32 DebugCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\apr_app.obj"
	-@erase "$(INTDIR)\internal.obj"
	-@erase "$(OUTDIR)\libaprapp-1.lib"
	-@erase "..\Debug\libaprapp-1.idb"
	-@erase "..\Debug\libaprapp-1.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MDd /W3 /Zi /Od /I "../include" /I "../include/arch" /I "../include/arch/win32" /I "../include/arch/unix" /D "_DEBUG" /D "WINNT" /D "WIN32" /D "_WINDOWS" /D "APR_APP" /Fo"$(INTDIR)\\" /Fd"$(OUTDIR)\libaprapp-1" /FD /EHsc /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\libaprapp.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\libaprapp-1.lib" 
LIB32_OBJS= \
	"$(INTDIR)\apr_app.obj" \
	"$(INTDIR)\internal.obj"

"$(OUTDIR)\libaprapp-1.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "libaprapp - Win32 Release9x"

OUTDIR=.\..\9x\Release
INTDIR=.\9x\Release
# Begin Custom Macros
OutDir=.\..\9x\Release
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\libaprapp-1.lib"

!ELSE 

ALL : "prelibaprapp - Win32 Release9x" "$(OUTDIR)\libaprapp-1.lib"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"prelibaprapp - Win32 Release9xCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\apr_app.obj"
	-@erase "$(INTDIR)\internal.obj"
	-@erase "$(OUTDIR)\libaprapp-1.lib"
	-@erase "..\9x\Release\libaprapp-1.idb"
	-@erase "..\9x\Release\libaprapp-1.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /Zi /O2 /Oy- /I "../include" /I "../include/arch" /I "../include/arch/win32" /I "../include/arch/unix" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "APR_APP" /Fo"$(INTDIR)\\" /Fd"$(OUTDIR)\libaprapp-1" /FD /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\libaprapp.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\libaprapp-1.lib" 
LIB32_OBJS= \
	"$(INTDIR)\apr_app.obj" \
	"$(INTDIR)\internal.obj"

"$(OUTDIR)\libaprapp-1.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "libaprapp - Win32 Debug9x"

OUTDIR=.\..\9x\Debug
INTDIR=.\9x\Debug
# Begin Custom Macros
OutDir=.\..\9x\Debug
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\libaprapp-1.lib"

!ELSE 

ALL : "prelibaprapp - Win32 Debug9x" "$(OUTDIR)\libaprapp-1.lib"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"prelibaprapp - Win32 Debug9xCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\apr_app.obj"
	-@erase "$(INTDIR)\internal.obj"
	-@erase "$(OUTDIR)\libaprapp-1.lib"
	-@erase "..\9x\Debug\libaprapp-1.idb"
	-@erase "..\9x\Debug\libaprapp-1.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MDd /W3 /Zi /Od /I "../include" /I "../include/arch" /I "../include/arch/win32" /I "../include/arch/unix" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "APR_APP" /Fo"$(INTDIR)\\" /Fd"$(OUTDIR)\libaprapp-1" /FD /EHsc /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\libaprapp.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\libaprapp-1.lib" 
LIB32_OBJS= \
	"$(INTDIR)\apr_app.obj" \
	"$(INTDIR)\internal.obj"

"$(OUTDIR)\libaprapp-1.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "libaprapp - x64 Release"

OUTDIR=.\..\x64\Release
INTDIR=.\x64\Release
# Begin Custom Macros
OutDir=.\..\x64\Release
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\libaprapp-1.lib"

!ELSE 

ALL : "prelibaprapp - x64 Release" "$(OUTDIR)\libaprapp-1.lib"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"prelibaprapp - x64 ReleaseCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\apr_app.obj"
	-@erase "$(INTDIR)\internal.obj"
	-@erase "$(OUTDIR)\libaprapp-1.lib"
	-@erase "..\x64\Release\libaprapp-1.idb"
	-@erase "..\x64\Release\libaprapp-1.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /Zi /O2 /Oy- /I "../include" /I "../include/arch" /I "../include/arch/win32" /I "../include/arch/unix" /D "NDEBUG" /D "WINNT" /D "WIN32" /D "_WINDOWS" /D "APR_APP" /Fo"$(INTDIR)\\" /Fd"$(OUTDIR)\libaprapp-1" /FD /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\libaprapp.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\libaprapp-1.lib" 
LIB32_OBJS= \
	"$(INTDIR)\apr_app.obj" \
	"$(INTDIR)\internal.obj"

"$(OUTDIR)\libaprapp-1.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "libaprapp - x64 Debug"

OUTDIR=.\..\x64\Debug
INTDIR=.\x64\Debug
# Begin Custom Macros
OutDir=.\..\x64\Debug
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\libaprapp-1.lib"

!ELSE 

ALL : "prelibaprapp - x64 Debug" "$(OUTDIR)\libaprapp-1.lib"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"prelibaprapp - x64 DebugCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\apr_app.obj"
	-@erase "$(INTDIR)\internal.obj"
	-@erase "$(OUTDIR)\libaprapp-1.lib"
	-@erase "..\x64\Debug\libaprapp-1.idb"
	-@erase "..\x64\Debug\libaprapp-1.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MDd /W3 /Zi /Od /I "../include" /I "../include/arch" /I "../include/arch/win32" /I "../include/arch/unix" /D "_DEBUG" /D "WINNT" /D "WIN32" /D "_WINDOWS" /D "APR_APP" /Fo"$(INTDIR)\\" /Fd"$(OUTDIR)\libaprapp-1" /FD /EHsc /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\libaprapp.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\libaprapp-1.lib" 
LIB32_OBJS= \
	"$(INTDIR)\apr_app.obj" \
	"$(INTDIR)\internal.obj"

"$(OUTDIR)\libaprapp-1.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("libaprapp.dep")
!INCLUDE "libaprapp.dep"
!ELSE 
!MESSAGE Warning: cannot find "libaprapp.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "libaprapp - Win32 Release" || "$(CFG)" == "libaprapp - Win32 Debug" || "$(CFG)" == "libaprapp - Win32 Release9x" || "$(CFG)" == "libaprapp - Win32 Debug9x" || "$(CFG)" == "libaprapp - x64 Release" || "$(CFG)" == "libaprapp - x64 Debug"

!IF  "$(CFG)" == "libaprapp - Win32 Release"

"prelibaprapp - Win32 Release" : 
   cd "."
   NMAKE /nologo /f NUL
   cd "."

"prelibaprapp - Win32 ReleaseCLEAN" : 
   cd "."
   cd "."

!ELSEIF  "$(CFG)" == "libaprapp - Win32 Debug"

"prelibaprapp - Win32 Debug" : 
   cd "."
   NMAKE /nologo /f NUL
   cd "."

"prelibaprapp - Win32 DebugCLEAN" : 
   cd "."
   cd "."

!ELSEIF  "$(CFG)" == "libaprapp - Win32 Release9x"

"prelibaprapp - Win32 Release9x" : 
   cd "."
   NMAKE /nologo /f NUL
   cd "."

"prelibaprapp - Win32 Release9xCLEAN" : 
   cd "."
   cd "."

!ELSEIF  "$(CFG)" == "libaprapp - Win32 Debug9x"

"prelibaprapp - Win32 Debug9x" : 
   cd "."
   NMAKE /nologo /f NUL
   cd "."

"prelibaprapp - Win32 Debug9xCLEAN" : 
   cd "."
   cd "."

!ELSEIF  "$(CFG)" == "libaprapp - x64 Release"

"prelibaprapp - x64 Release" : 
   cd "."
   NMAKE /nologo /f NUL
   cd "."

"prelibaprapp - x64 ReleaseCLEAN" : 
   cd "."
   cd "."

!ELSEIF  "$(CFG)" == "libaprapp - x64 Debug"

"prelibaprapp - x64 Debug" : 
   cd "."
   NMAKE /nologo /f NUL
   cd "."

"prelibaprapp - x64 DebugCLEAN" : 
   cd "."
   cd "."

!ENDIF 

SOURCE=..\misc\win32\apr_app.c

"$(INTDIR)\apr_app.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\misc\win32\internal.c

"$(INTDIR)\internal.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)



!ENDIF 

