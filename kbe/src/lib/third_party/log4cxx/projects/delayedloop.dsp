# Microsoft Developer Studio Project File - Name="delayedloop" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

#
# Licensed to the Apache Software Foundation (ASF) under one or more
# contributor license agreements.  See the NOTICE file distributed with
# this work for additional information regarding copyright ownership.
# The ASF licenses this file to You under the Apache License, Version 2.0
# (the "License"); you may not use this file except in compliance with
# the License.  You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=delayedloop - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "delayedloop.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "delayedloop.mak" CFG="delayedloop - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "delayedloop - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "delayedloop - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "delayedloop - Win32 Release"
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
# ADD BASE CPP /c /nologo /EHsc  /O2  /D "NDEBUG" /MD /D "WIN32" /D "APR_DECLARE_STATIC"
# ADD CPP /I "..\src\main\include" /I "..\src\examples\cpp" /I "..\..\apr\include" /c /nologo /EHsc  /O2  /D "NDEBUG" /MD /D "WIN32" /D "APR_DECLARE_STATIC"
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /NOLOGO /SUBSYSTEM:CONSOLE /INCREMENTAL:NO
# ADD LINK32 ADVAPI32.LIB WS2_32.LIB MSWSOCK.LIB SHELL32.LIB ODBC32.LIB /NOLOGO /SUBSYSTEM:CONSOLE /INCREMENTAL:NO

!ELSEIF  "$(CFG)" == "delayedloop - Win32 Debug"
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
# ADD BASE CPP /c /nologo /EHsc /Zi /Od /GZ /D "_DEBUG" /MDd /D "WIN32" /D "APR_DECLARE_STATIC"
# ADD CPP /I "..\src\main\include" /I "..\src\examples\cpp" /I "..\..\apr\include" /c /nologo /EHsc /Zi /Od /GZ /D "_DEBUG" /MDd /D "WIN32" /D "APR_DECLARE_STATIC"
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /NOLOGO /DEBUG /SUBSYSTEM:CONSOLE /INCREMENTAL:NO
# ADD LINK32 ADVAPI32.LIB WS2_32.LIB MSWSOCK.LIB SHELL32.LIB ODBC32.LIB /NOLOGO /DEBUG /SUBSYSTEM:CONSOLE /INCREMENTAL:NO

!ENDIF
# Begin Target

# Name "delayedloop - Win32 Release"
# Name "delayedloop - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\src\examples\cpp\delayedloop.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
