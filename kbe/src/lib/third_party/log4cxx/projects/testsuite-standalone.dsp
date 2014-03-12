# Microsoft Developer Studio Project File - Name="testsuite_standalone" - Package Owner=<4>
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

CFG=testsuite_standalone - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "testsuite_standalone.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "testsuite_standalone.mak" CFG="testsuite_standalone - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "testsuite_standalone - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "testsuite_standalone - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "testsuite_standalone - Win32 Release"
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
# ADD BASE CPP /c /nologo /EHsc  /O2  /D "NDEBUG" /MD /D "LOG4CXX" /D "WIN32" /D "LOG4CXX"
# ADD CPP /I "..\src\main\include" /I "..\..\apr\include" /I "..\..\apr-util\include" /I "..\src\main\include" /I "..\..\apr\include" /c /nologo /EHsc  /O2  /D "NDEBUG" /MD /D "LOG4CXX" /D "WIN32" /D "LOG4CXX"
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

!ELSEIF  "$(CFG)" == "testsuite_standalone - Win32 Debug"
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
# ADD BASE CPP /c /nologo /EHsc /Zi /Od /GZ /D "_DEBUG" /MDd /D "LOG4CXX" /D "WIN32" /D "LOG4CXX"
# ADD CPP /I "..\src\main\include" /I "..\..\apr\include" /I "..\..\apr-util\include" /I "..\src\main\include" /I "..\..\apr\include" /c /nologo /EHsc /Zi /Od /GZ /D "_DEBUG" /MDd /D "LOG4CXX" /D "WIN32" /D "LOG4CXX"
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

# Name "testsuite_standalone - Win32 Release"
# Name "testsuite_standalone - Win32 Debug"
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

SOURCE=..\src\main\cpp\action.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\appenderattachableimpl.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\appenderskeleton.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\appenderskeletontestcase.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\aprinitializer.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\asyncappender.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\asyncappendertestcase.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\basicconfigurator.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\util\binarycompare.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\bufferedwriter.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\bytearrayinputstream.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\bytearrayoutputstream.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\bytebuffer.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\cacheddateformat.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\helpers\cacheddateformattestcase.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\charsetdecoder.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\helpers\charsetdecodertestcase.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\charsetencoder.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\helpers\charsetencodertestcase.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\class.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\classnamepatternconverter.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\classregistration.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\util\compare.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\condition.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\configurator.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\consoleappender.cpp
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

SOURCE=..\src\main\cpp\cyclicbuffer.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\helpers\cyclicbuffertestcase.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\dailyrollingfileappender.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\datagrampacket.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\datagramsocket.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\date.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\dateformat.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\datelayout.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\datepatternconverter.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\helpers\datetimedateformattestcase.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\defaultconfigurator.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\defaultloggerfactory.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\defaultrepositoryselector.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\domconfigurator.cpp
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

SOURCE=..\src\main\cpp\exception.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\fallbackerrorhandler.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\file.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\fileappender.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\fileappendertest.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\fileappendertestcase.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\filedatepatternconverter.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\fileinputstream.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\filelocationpatternconverter.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\util\filenamefilter.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\rolling\filenamepatterntestcase.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\fileoutputstream.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\filerenameaction.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\filetestcase.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\filewatchdog.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\filter.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\rolling\filterbasedrollingtest.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\filterbasedtriggeringpolicy.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\fixedwindowrollingpolicy.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\formattinginfo.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\fulllocationpatternconverter.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\gzcompressaction.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\hierarchy.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\hierarchytest.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\hierarchythresholdtestcase.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\htmllayout.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\inetaddress.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\helpers\inetaddresstestcase.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\inputstream.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\inputstreamreader.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\integer.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\integerpatternconverter.cpp
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

SOURCE=..\src\main\cpp\layout.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\level.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\levelmatchfilter.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\varia\levelmatchfiltertestcase.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\levelpatternconverter.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\levelrangefilter.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\varia\levelrangefiltertestcase.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\leveltestcase.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\linelocationpatternconverter.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\util\linenumberfilter.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\lineseparatorpatternconverter.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\literalpatternconverter.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\loader.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\locale.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\helpers\localechanger.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\locationinfo.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\resources\log4cxx.rc
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\logger.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\loggerpatternconverter.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\loggertestcase.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\loggingevent.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\loggingeventpatternconverter.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\spi\loggingeventtest.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\loglog.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\logmanager.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\logstream.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\logunit.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\rolling\manualrollingtest.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\manualtriggeringpolicy.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\mdc.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\messagebuffer.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\helpers\messagebuffertest.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\messagepatternconverter.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\methodlocationpatternconverter.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\minimumtestcase.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\mutex.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\nameabbreviator.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\namepatternconverter.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\ndc.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\ndcpatternconverter.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\ndctestcase.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\nteventlogappender.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\nt\nteventlogappendertestcase.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\pattern\num343patternconverter.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\objectimpl.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\objectoutputstream.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\objectptr.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\rolling\obsoletedailyrollingfileappendertest.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\obsoleterollingfileappender.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\rolling\obsoleterollingfileappendertest.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\odbcappender.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\db\odbcappendertestcase.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\onlyonceerrorhandler.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\optionconverter.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\helpers\optionconvertertestcase.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\outputdebugstringappender.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\outputstream.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\outputstreamwriter.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\patternconverter.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\patternlayout.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\patternlayouttest.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\patternparser.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\pattern\patternparsertestcase.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\pool.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\properties.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\propertiespatternconverter.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\helpers\propertiestestcase.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\propertyconfigurator.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\propertyconfiguratortest.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\propertyresourcebundle.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\propertysetter.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\reader.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\relativetimedateformat.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\helpers\relativetimedateformattestcase.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\util\relativetimefilter.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\relativetimepatternconverter.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\resourcebundle.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\rollingfileappender.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\rollingfileappendertestcase.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\rollingpolicy.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\rollingpolicybase.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\rolloverdescription.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\rootlogger.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\util\serializationtesthelper.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\serversocket.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\simpledateformat.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\simplelayout.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\rolling\sizebasedrollingtest.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\sizebasedtriggeringpolicy.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\smtpappender.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\net\smtpappendertestcase.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\socket.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\socketappender.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\socketappenderskeleton.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\net\socketappendertestcase.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\sockethubappender.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\net\sockethubappendertestcase.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\socketoutputstream.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\net\socketservertestcase.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\streamtestcase.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\strftimedateformat.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\stringhelper.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\helpers\stringhelpertestcase.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\stringmatchfilter.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\stringtokenizer.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\helpers\stringtokenizertestcase.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\synchronized.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\syslogappender.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\net\syslogappendertestcase.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\syslogwriter.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\helpers\syslogwritertest.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\system.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\systemerrwriter.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\systemoutwriter.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\telnetappender.cpp
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

SOURCE=..\src\main\cpp\threadcxx.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\util\threadfilter.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\threadlocal.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\threadpatternconverter.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\threadspecificdata.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\throwableinformationpatternconverter.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\timebasedrollingpolicy.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\rolling\timebasedrollingtest.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\timezone.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\helpers\timezonetestcase.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\transcoder.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\helpers\transcodertestcase.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\transform.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\util\transformer.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\triggeringpolicy.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\ttcclayout.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\util\utilfilter.cpp
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\vectorappender.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\writer.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main\cpp\writerappender.cpp
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

SOURCE=..\src\main\cpp\xmllayout.cpp
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

SOURCE=..\src\main\cpp\xmlsocketappender.cpp
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
# Begin Source File

SOURCE=..\src\main\cpp\zipcompressaction.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\src\test\cpp\util\absolutedateandtimefilter.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\helpers\absolutetimedateformat.h
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

SOURCE=..\src\main\include\log4cxx\rolling\action.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\filter\andfilter.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\appender.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\spi\appenderattachable.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\helpers\appenderattachableimpl.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\appenderskeleton.h
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\appenderskeletontestcase.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\helpers\aprinitializer.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\asyncappender.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\basicconfigurator.h
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\util\binarycompare.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\helpers\bufferedoutputstream.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\helpers\bufferedwriter.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\helpers\bytearrayinputstream.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\helpers\bytearrayoutputstream.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\helpers\bytebuffer.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\helpers\cacheddateformat.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\helpers\charsetdecoder.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\helpers\charsetencoder.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\helpers\class.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\pattern\classnamepatternconverter.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\helpers\classregistration.h
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\util\compare.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\helpers\condition.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\spi\configurator.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\consoleappender.h
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\util\controlfilter.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\helpers\cyclicbuffer.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\dailyrollingfileappender.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\helpers\datagrampacket.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\helpers\datagramsocket.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\helpers\date.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\helpers\dateformat.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\helpers\datelayout.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\pattern\datepatternconverter.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\helpers\datetimedateformat.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\defaultconfigurator.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\defaultloggerfactory.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\spi\defaultrepositoryselector.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\filter\denyallfilter.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\xml\domconfigurator.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\spi\errorhandler.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\helpers\exception.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\filter\expressionfilter.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\varia\fallbackerrorhandler.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\file.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\fileappender.h
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\fileappendertestcase.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\pattern\filedatepatternconverter.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\helpers\fileinputstream.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\pattern\filelocationpatternconverter.h
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\util\filenamefilter.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\helpers\fileoutputstream.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\rolling\filerenameaction.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\helpers\filewatchdog.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\spi\filter.h
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\util\filter.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\rolling\filterbasedtriggeringpolicy.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\rolling\fixedwindowrollingpolicy.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\pattern\formattinginfo.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\pattern\fulllocationpatternconverter.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\rolling\gzcompressaction.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\hierarchy.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\spi\hierarchyeventlistener.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\htmllayout.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\helpers\inetaddress.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\helpers\inputstream.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\helpers\inputstreamreader.h
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\insertwide.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\helpers\integer.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\pattern\integerpatternconverter.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\helpers\iso8601dateformat.h
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\util\iso8601filter.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\layout.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\level.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\filter\levelmatchfilter.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\pattern\levelpatternconverter.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\filter\levelrangefilter.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\pattern\linelocationpatternconverter.h
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\util\linenumberfilter.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\pattern\lineseparatorpatternconverter.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\pattern\literalpatternconverter.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\helpers\loader.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\helpers\locale.h
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\helpers\localechanger.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\spi\location\locationinfo.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\filter\locationinfofilter.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\log4cxx.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\private\log4cxx_private.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\logger.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\spi\loggerfactory.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\pattern\loggerpatternconverter.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\spi\loggerrepository.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\spi\loggingevent.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\pattern\loggingeventpatternconverter.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\helpers\loglog.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\logmanager.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\logstring.h
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\logunit.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\rolling\manualtriggeringpolicy.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\filter\mapfilter.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\mdc.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\helpers\messagebuffer.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\pattern\messagepatternconverter.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\pattern\methodlocationpatternconverter.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\helpers\mutex.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\pattern\nameabbreviator.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\pattern\namepatternconverter.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\ndc.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\pattern\ndcpatternconverter.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\nt\nteventlogappender.h
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\pattern\num343patternconverter.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\helpers\object.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\helpers\objectimpl.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\helpers\objectoutputstream.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\helpers\objectptr.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\db\odbcappender.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\helpers\onlyonceerrorhandler.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\helpers\optionconverter.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\spi\optionhandler.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\nt\outputdebugstringappender.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\helpers\outputstream.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\helpers\outputstreamwriter.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\pattern\patternconverter.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\patternlayout.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\pattern\patternparser.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\helpers\pool.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\portability.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\helpers\properties.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\pattern\propertiespatternconverter.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\propertyconfigurator.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\filter\propertyfilter.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\helpers\propertyresourcebundle.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\config\propertysetter.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\provisionnode.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\helpers\reader.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\helpers\relativetimedateformat.h
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\util\relativetimefilter.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\pattern\relativetimepatternconverter.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\spi\repositoryselector.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\helpers\resourcebundle.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\rolling\rollingfileappender.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\rollingfileappender.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\rolling\rollingfileappenderskeleton.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\rolling\rollingpolicy.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\rolling\rollingpolicybase.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\rolling\rolloverdescription.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\spi\rootlogger.h
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\util\serializationtesthelper.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\helpers\serversocket.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\helpers\simpledateformat.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\simplelayout.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\rolling\sizebasedtriggeringpolicy.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\net\smtpappender.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\helpers\socket.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\net\socketappender.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\net\socketappenderskeleton.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\net\sockethubappender.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\helpers\socketoutputstream.h
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\net\socketservertestcase.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\stream.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\helpers\strftimedateformat.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\helpers\strictmath.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\helpers\stringhelper.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\filter\stringmatchfilter.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\helpers\stringtokenizer.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\helpers\synchronized.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\net\syslogappender.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\helpers\syslogwriter.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\helpers\system.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\helpers\systemerrwriter.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\helpers\systemoutwriter.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\helpers\tchar.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\net\telnetappender.h
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\testchar.h
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\testutil.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\helpers\thread.h
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\util\threadfilter.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\helpers\threadlocal.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\pattern\threadpatternconverter.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\helpers\threadspecificdata.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\pattern\throwableinformationpatternconverter.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\rolling\timebasedrollingpolicy.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\helpers\timezone.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\helpers\transcoder.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\helpers\transform.h
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\util\transformer.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\spi\triggeringeventevaluator.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\rolling\triggeringpolicy.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\ttcclayout.h
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\vectorappender.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\helpers\writer.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\writerappender.h
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

SOURCE=..\src\main\include\log4cxx\helpers\xml.h
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\util\xmlfilenamefilter.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\xml\xmllayout.h
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\util\xmllineattributefilter.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\net\xmlsocketappender.h
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\util\xmlthreadfilter.h
# End Source File
# Begin Source File

SOURCE=..\src\test\cpp\util\xmltimestampfilter.h
# End Source File
# Begin Source File

SOURCE=..\src\main\include\log4cxx\rolling\zipcompressaction.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
