# Microsoft Developer Studio Project File - Name="testlib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) External Target" 0x0106

CFG=testlib - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "testlib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "testlib.mak" CFG="testlib - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "testlib - Win32 Release" (based on "Win32 (x86) External Target")
!MESSAGE "testlib - Win32 Debug" (based on "Win32 (x86) External Target")
!MESSAGE "testlib - Win32 Release9x" (based on "Win32 (x86) External Target")
!MESSAGE "testlib - Win32 Debug9x" (based on "Win32 (x86) External Target")
!MESSAGE "testlib - x64 Release" (based on "Win32 (x86) External Target")
!MESSAGE "testlib - x64 Debug" (based on "Win32 (x86) External Target")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""

!IF  "$(CFG)" == "testlib - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ""
# PROP BASE Intermediate_Dir ""
# PROP BASE Cmd_Line "NMAKE /f Makefile.win INTDIR=LibR OUTDIR=LibR MODEL=static all check"
# PROP BASE Rebuild_Opt "/a"
# PROP BASE Target_File "LibR\testall.exe"
# PROP BASE Bsc_Name ""
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ""
# PROP Intermediate_Dir ""
# PROP Cmd_Line "NMAKE /f Makefile.win INTDIR=LibR OUTDIR=LibR MODEL=static all check"
# PROP Rebuild_Opt "/a"
# PROP Target_File "LibR\testall.exe"
# PROP Bsc_Name ""
# PROP Target_Dir ""

!ELSEIF  "$(CFG)" == "testlib - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ""
# PROP BASE Intermediate_Dir ""
# PROP BASE Cmd_Line "NMAKE /f Makefile.win INTDIR=LibD OUTDIR=LibD MODEL=static _DEBUG=1 all check"
# PROP BASE Rebuild_Opt "/a"
# PROP BASE Target_File "LibD\testall.exe"
# PROP BASE Bsc_Name ""
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ""
# PROP Intermediate_Dir ""
# PROP Cmd_Line "NMAKE /f Makefile.win INTDIR=LibD OUTDIR=LibD MODEL=static _DEBUG=1 all check"
# PROP Rebuild_Opt "/a"
# PROP Target_File "LibD\testall.exe"
# PROP Bsc_Name ""
# PROP Target_Dir ""

!ELSEIF  "$(CFG)" == "testlib - Win32 Release9x"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ""
# PROP BASE Intermediate_Dir ""
# PROP BASE Cmd_Line "NMAKE /f Makefile.win INTDIR=9x\LibR OUTDIR=9x\LibR MODEL=static all check"
# PROP BASE Rebuild_Opt "/a"
# PROP BASE Target_File "9x\LibR\testall.exe"
# PROP BASE Bsc_Name ""
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ""
# PROP Intermediate_Dir ""
# PROP Cmd_Line "NMAKE /f Makefile.win INTDIR=9x\LibR OUTDIR=9x\LibR MODEL=static all check"
# PROP Rebuild_Opt "/a"
# PROP Target_File "9x\LibR\testall.exe"
# PROP Bsc_Name ""
# PROP Target_Dir ""

!ELSEIF  "$(CFG)" == "testlib - Win32 Debug9x"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ""
# PROP BASE Intermediate_Dir ""
# PROP BASE Cmd_Line "NMAKE /f Makefile.win INTDIR=9x\LibD OUTDIR=9x\LibD MODEL=static _DEBUG=1 all check"
# PROP BASE Rebuild_Opt "/a"
# PROP BASE Target_File "9x\LibD\testall.exe"
# PROP BASE Bsc_Name ""
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ""
# PROP Intermediate_Dir ""
# PROP Cmd_Line "NMAKE /f Makefile.win INTDIR=9x\LibD OUTDIR=9x\LibD MODEL=static _DEBUG=1 all check"
# PROP Rebuild_Opt "/a"
# PROP Target_File "9x\LibD\testall.exe"
# PROP Bsc_Name ""
# PROP Target_Dir ""

!ELSEIF  "$(CFG)" == "testlib - x64 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ""
# PROP BASE Intermediate_Dir ""
# PROP BASE Cmd_Line "NMAKE /f Makefile.win INTDIR=x64\LibR OUTDIR=x64\LibR MODEL=static all check"
# PROP BASE Rebuild_Opt "/a"
# PROP BASE Target_File "x64\LibR\testall.exe"
# PROP BASE Bsc_Name ""
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ""
# PROP Intermediate_Dir ""
# PROP Cmd_Line "NMAKE /f Makefile.win INTDIR=x64\LibR OUTDIR=x64\LibR MODEL=static all check"
# PROP Rebuild_Opt "/a"
# PROP Target_File "x64\LibR\testall.exe"
# PROP Bsc_Name ""
# PROP Target_Dir ""

!ELSEIF  "$(CFG)" == "testlib - x64 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ""
# PROP BASE Intermediate_Dir ""
# PROP BASE Cmd_Line "NMAKE /f Makefile.win INTDIR=x64\LibD OUTDIR=x64\LibD MODEL=static _DEBUG=1 all check"
# PROP BASE Rebuild_Opt "/a"
# PROP BASE Target_File "x64\LibD\testall.exe"
# PROP BASE Bsc_Name ""
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ""
# PROP Intermediate_Dir ""
# PROP Cmd_Line "NMAKE /f Makefile.win INTDIR=x64\LibD OUTDIR=x64\LibD MODEL=static _DEBUG=1 all check"
# PROP Rebuild_Opt "/a"
# PROP Target_File "x64\LibD\testall.exe"
# PROP Bsc_Name ""
# PROP Target_Dir ""

!ENDIF 

# Begin Target

# Name "testlib - Win32 Release"
# Name "testlib - Win32 Debug"
# Name "testlib - Win32 Release9x"
# Name "testlib - Win32 Debug9x"
# Name "testlib - x64 Release"
# Name "testlib - x64 Debug"
# Begin Group "testall Source Files"

# PROP Default_Filter ".c"
# Begin Source File

SOURCE=.\abts.c
# End Source File
# Begin Source File

SOURCE=.\abts.h
# End Source File
# Begin Source File

SOURCE=.\abts_tests.h
# End Source File
# Begin Source File

SOURCE=.\testapp.c
# End Source File
# Begin Source File

SOURCE=.\testargs.c
# End Source File
# Begin Source File

SOURCE=.\testatomic.c
# End Source File
# Begin Source File

SOURCE=.\testcond.c
# End Source File
# Begin Source File

SOURCE=.\testdir.c
# End Source File
# Begin Source File

SOURCE=.\testdso.c
# End Source File
# Begin Source File

SOURCE=.\testdup.c
# End Source File
# Begin Source File

SOURCE=.\testenv.c
# End Source File
# Begin Source File

SOURCE=.\testfile.c
# End Source File
# Begin Source File

SOURCE=.\testfilecopy.c
# End Source File
# Begin Source File

SOURCE=.\testfileinfo.c
# End Source File
# Begin Source File

SOURCE=.\testflock.c
# End Source File
# Begin Source File

SOURCE=.\testflock.h
# End Source File
# Begin Source File

SOURCE=.\testfmt.c
# End Source File
# Begin Source File

SOURCE=.\testfnmatch.c
# End Source File
# Begin Source File

SOURCE=.\testglobalmutex.c
# End Source File
# Begin Source File

SOURCE=.\testglobalmutex.h
# End Source File
# Begin Source File

SOURCE=.\testhash.c
# End Source File
# Begin Source File

SOURCE=.\testipsub.c
# End Source File
# Begin Source File

SOURCE=.\testlfs.c
# End Source File
# Begin Source File

SOURCE=.\testlock.c
# End Source File
# Begin Source File

SOURCE=.\testmmap.c
# End Source File
# Begin Source File

SOURCE=.\testnames.c
# End Source File
# Begin Source File

SOURCE=.\testoc.c
# End Source File
# Begin Source File

SOURCE=.\testpath.c
# End Source File
# Begin Source File

SOURCE=.\testpipe.c
# End Source File
# Begin Source File

SOURCE=.\testpoll.c
# End Source File
# Begin Source File

SOURCE=.\testpools.c
# End Source File
# Begin Source File

SOURCE=.\testproc.c
# End Source File
# Begin Source File

SOURCE=.\testrand.c
# End Source File
# Begin Source File

SOURCE=.\testshm.c
# End Source File
# Begin Source File

SOURCE=.\testshm.h
# End Source File
# Begin Source File

SOURCE=.\testsleep.c
# End Source File
# Begin Source File

SOURCE=.\testsock.c
# End Source File
# Begin Source File

SOURCE=.\testsock.h
# End Source File
# Begin Source File

SOURCE=.\testsockets.c
# End Source File
# Begin Source File

SOURCE=.\testsockopt.c
# End Source File
# Begin Source File

SOURCE=.\teststr.c
# End Source File
# Begin Source File

SOURCE=.\teststrnatcmp.c
# End Source File
# Begin Source File

SOURCE=.\testtable.c
# End Source File
# Begin Source File

SOURCE=.\testtemp.c
# End Source File
# Begin Source File

SOURCE=.\testthread.c
# End Source File
# Begin Source File

SOURCE=.\testtime.c
# End Source File
# Begin Source File

SOURCE=.\testud.c
# End Source File
# Begin Source File

SOURCE=.\testuser.c
# End Source File
# Begin Source File

SOURCE=.\testutil.c
# End Source File
# Begin Source File

SOURCE=.\testutil.h
# End Source File
# Begin Source File

SOURCE=.\testvsn.c
# End Source File
# End Group
# Begin Group "Other Source Files"

# PROP Default_Filter ".c"
# Begin Source File

SOURCE=.\globalmutexchild.c
# End Source File
# Begin Source File

SOURCE=.\mod_test.c
# End Source File
# Begin Source File

SOURCE=.\nw_misc.c
# End Source File
# Begin Source File

SOURCE=.\occhild.c
# End Source File
# Begin Source File

SOURCE=.\proc_child.c
# End Source File
# Begin Source File

SOURCE=.\readchild.c
# End Source File
# Begin Source File

SOURCE=.\sendfile.c
# End Source File
# Begin Source File

SOURCE=.\sockchild.c
# End Source File
# Begin Source File

SOURCE=.\testlockperf.c
# End Source File
# Begin Source File

SOURCE=.\testmutexscope.c
# End Source File
# Begin Source File

SOURCE=.\testprocmutex.c
# End Source File
# Begin Source File

SOURCE=.\testshmconsumer.c
# End Source File
# Begin Source File

SOURCE=.\testshmproducer.c
# End Source File
# Begin Source File

SOURCE=.\tryread.c
# End Source File
# End Group
# Begin Source File

SOURCE=.\Makefile.win
# End Source File
# End Target
# End Project
