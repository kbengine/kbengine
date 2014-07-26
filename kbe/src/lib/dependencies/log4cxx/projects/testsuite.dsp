# Microsoft Developer Studio Project File - Name="testsuite" - Package Owner=<4>
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

CFG=testsuite - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "testsuite.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "testsuite.mak" CFG="testsuite - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "testsuite - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "testsuite - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "testsuite - Win32 Release"
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
# ADD BASE CPP /c /nologo /EHsc  /O2  /D "NDEBUG" /MD /D "APR_DECLARE_STATIC" /D "APU_DECLARE_STATIC" /D "WIN32"
# ADD CPP /I "..\src\main\include" /I "..\..\apr\include" /I "..\..\apr-util\include" /c /nologo /EHsc  /O2  /D "NDEBUG" /MD /D "APR_DECLARE_STATIC" /D "APU_DECLARE_STATIC" /D "WIN32"
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

!ELSEIF  "$(CFG)" == "testsuite - Win32 Debug"
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
# ADD BASE CPP /c /nologo /EHsc /Zi /Od /GZ /D "_DEBUG" /MDd /D "APR_DECLARE_STATIC" /D "APU_DECLARE_STATIC" /D "WIN32"
# ADD CPP /I "..\src\main\include" /I "..\..\apr\include" /I "..\..\apr-util\include" /c /nologo /EHsc /Zi /Od /GZ /D "_DEBUG" /MDd /D "APR_DECLARE_STATIC" /D "APU_DECLARE_STATIC" /D "WIN32"
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

# Name "testsuite - Win32 Release"
# Name "testsuite - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\src\test\cpp\util\absolutedateandtimefilter.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\helpers\absolutetimedateformattestcase.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\util\absolutetimefilter.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\abts.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\appenderskeletontestcase.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\asyncappendertestcase.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\util\binarycompare.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\helpers\cacheddateformattestcase.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\helpers\charsetdecodertestcase.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\helpers\charsetencodertestcase.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\util\compare.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\consoleappendertestcase.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\util\controlfilter.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\xml\customleveltestcase.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\helpers\cyclicbuffertestcase.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\helpers\datetimedateformattestcase.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\xml\domtestcase.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\encodingtest.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\varia\errorhandlertestcase.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\fileappendertest.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\fileappendertestcase.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\util\filenamefilter.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\rolling\filenamepatterntestcase.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\filetestcase.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\rolling\filterbasedrollingtest.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\hierarchytest.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\hierarchythresholdtestcase.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\helpers\inetaddresstestcase.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\helpers\iso8601dateformattestcase.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\util\iso8601filter.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\l7dtestcase.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\varia\levelmatchfiltertestcase.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\varia\levelrangefiltertestcase.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\leveltestcase.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\util\linenumberfilter.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\helpers\localechanger.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\loggertestcase.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\spi\loggingeventtest.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\logunit.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\rolling\manualrollingtest.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\helpers\messagebuffertest.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\minimumtestcase.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\ndctestcase.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\nt\nteventlogappendertestcase.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\pattern\num343patternconverter.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\rolling\obsoletedailyrollingfileappendertest.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\rolling\obsoleterollingfileappendertest.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\db\odbcappendertestcase.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\helpers\optionconvertertestcase.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\patternlayouttest.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\pattern\patternparsertestcase.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\helpers\propertiestestcase.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\propertyconfiguratortest.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\helpers\relativetimedateformattestcase.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\util\relativetimefilter.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\rollingfileappendertestcase.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\util\serializationtesthelper.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\rolling\sizebasedrollingtest.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\net\smtpappendertestcase.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\net\socketappendertestcase.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\net\sockethubappendertestcase.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\net\socketservertestcase.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\streamtestcase.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\helpers\stringhelpertestcase.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\helpers\stringtokenizertestcase.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\net\syslogappendertestcase.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\helpers\syslogwritertest.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\net\telnetappendertestcase.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\defaultinit\testcase1.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\defaultinit\testcase2.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\defaultinit\testcase3.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\defaultinit\testcase4.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\util\threadfilter.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\rolling\timebasedrollingtest.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\helpers\timezonetestcase.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\helpers\transcodertestcase.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\util\transformer.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\util\utilfilter.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\vectorappender.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\writerappendertestcase.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\xml\xlevel.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\customlogger\xlogger.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\customlogger\xloggertestcase.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\util\xmlfilenamefilter.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\xml\xmllayouttest.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\xml\xmllayouttestcase.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\util\xmllineattributefilter.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\net\xmlsocketappendertestcase.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\util\xmlthreadfilter.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\util\xmltimestampfilter.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\src\test\cpp\util\absolutedateandtimefilter.h
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\util\absolutetimefilter.h
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\abts.h
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\abts_tests.h
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\appenderskeletontestcase.h
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\util\binarycompare.h
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\util\compare.h
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\util\controlfilter.h
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\fileappendertestcase.h
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\util\filenamefilter.h
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\util\filter.h
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\insertwide.h
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\util\iso8601filter.h
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\util\linenumberfilter.h
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\helpers\localechanger.h
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\logunit.h
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\pattern\num343patternconverter.h
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\util\relativetimefilter.h
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\util\serializationtesthelper.h
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\net\socketservertestcase.h
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\testchar.h
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\testutil.h
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\util\threadfilter.h
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\util\transformer.h
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\vectorappender.h
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\writerappendertestcase.h
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\xml\xlevel.h
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\customlogger\xlogger.h
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\util\xmlfilenamefilter.h
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\util\xmllineattributefilter.h
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\util\xmlthreadfilter.h
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\util\xmltimestampfilter.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
