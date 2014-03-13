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
#
noinst_HEADERS = \
	$(top_srcdir)/src/test/cpp/customlogger/*.h \
	$(top_srcdir)/src/test/cpp/helpers/*.h \
	$(top_srcdir)/src/test/cpp/net/*.h \
	$(top_srcdir)/src/test/cpp/pattern/*.h \
	$(top_srcdir)/src/test/cpp/util/*.h \
	$(top_srcdir)/src/test/cpp/xml/*.h \
	$(top_srcdir)/src/test/cpp/*.h

INCLUDES = -I$(top_srcdir)/src/main/include -I$(top_builddir)/src/main/include

noinst_PROGRAMS = testsuite

customlogger_tests = \
	customlogger/xlogger.cpp\
	customlogger/xloggertestcase.cpp

defaultinit_tests = \
	defaultinit/testcase1.cpp\
	defaultinit/testcase2.cpp\
	defaultinit/testcase3.cpp\
	defaultinit/testcase4.cpp

helpers = \
        helpers/absolutetimedateformattestcase.cpp \
        helpers/cacheddateformattestcase.cpp \
        helpers/charsetdecodertestcase.cpp \
        helpers/charsetencodertestcase.cpp \
        helpers/cyclicbuffertestcase.cpp\
        helpers/datetimedateformattestcase.cpp \
        helpers/inetaddresstestcase.cpp \
        helpers/iso8601dateformattestcase.cpp \
        helpers/localechanger.cpp\
        helpers/messagebuffertest.cpp \
        helpers/optionconvertertestcase.cpp       \
        helpers/propertiestestcase.cpp \
        helpers/relativetimedateformattestcase.cpp \
        helpers/stringtokenizertestcase.cpp \
        helpers/stringhelpertestcase.cpp \
        helpers/syslogwritertest.cpp \
        helpers/timezonetestcase.cpp \
        helpers/transcodertestcase.cpp

net_tests = \
	net/smtpappendertestcase.cpp \
	net/socketappendertestcase.cpp \
	net/sockethubappendertestcase.cpp \
	net/socketservertestcase.cpp \
	net/syslogappendertestcase.cpp \
	net/telnetappendertestcase.cpp \
	net/xmlsocketappendertestcase.cpp

pattern_tests = \
	pattern/num343patternconverter.cpp \
	pattern/patternparsertestcase.cpp

rolling_tests = \
        rolling/filenamepatterntestcase.cpp \
        rolling/filterbasedrollingtest.cpp \
        rolling/manualrollingtest.cpp \
        rolling/obsoletedailyrollingfileappendertest.cpp \
        rolling/obsoleterollingfileappendertest.cpp \
        rolling/sizebasedrollingtest.cpp \
        rolling/timebasedrollingtest.cpp

util = \
	util/absolutetimefilter.cpp\
	util/absolutedateandtimefilter.cpp\
	util/binarycompare.cpp\
	util/compare.cpp\
	util/controlfilter.cpp\
	util/filenamefilter.cpp \
	util/utilfilter.cpp\
	util/iso8601filter.cpp\
	util/linenumberfilter.cpp\
	util/relativetimefilter.cpp\
    util/serializationtesthelper.cpp \
	util/threadfilter.cpp\
	util/transformer.cpp\
	util/xmlfilenamefilter.cpp \
	util/xmllineattributefilter.cpp\
	util/xmltimestampfilter.cpp \
	util/xmlthreadfilter.cpp

varia_tests = \
	varia/errorhandlertestcase.cpp \
	varia/levelmatchfiltertestcase.cpp \
	varia/levelrangefiltertestcase.cpp

db_tests = \
	db/odbcappendertestcase.cpp

xml_tests = \
	xml/customleveltestcase.cpp \
	xml/domtestcase.cpp \
	xml/xlevel.cpp \
	xml/xmllayouttestcase.cpp \
    xml/xmllayouttest.cpp

nt_tests = \
	nt/nteventlogappendertestcase.cpp

testsuite_SOURCES = \
        $(customlogger_tests) \
        $(defaultinit_tests) \
        $(helpers) \
        $(net_tests) \
        $(pattern_tests) \
        $(rolling_tests) \
        $(util) \
        $(varia_tests) \
        $(db_tests) \
        $(xml_tests) \
        $(nt_tests) \
        abts.cpp \
        asyncappendertestcase.cpp\
        encodingtest.cpp\
        filetestcase.cpp \
        hierarchytest.cpp\
        hierarchythresholdtestcase.cpp\
        l7dtestcase.cpp\
        leveltestcase.cpp \
        logunit.cpp \
        loggertestcase.cpp\
        minimumtestcase.cpp\
        patternlayouttest.cpp\
        vectorappender.cpp\
        appenderskeletontestcase.cpp\
        consoleappendertestcase.cpp\
        fileappendertestcase.cpp\
        rollingfileappendertestcase.cpp\
        streamtestcase.cpp\
        writerappendertestcase.cpp \
        ndctestcase.cpp \
        propertyconfiguratortest.cpp 

testsuite_LDADD = \
        $(top_builddir)/src/main/cpp/liblog4cxx.la


testsuite_DEPENDENCIES = \
        $(top_builddir)/src/main/cpp/liblog4cxx.la

check: testsuite

