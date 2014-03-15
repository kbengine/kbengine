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
lib_LTLIBRARIES = liblog4cxx.la

INCLUDES = -I$(top_srcdir)/src/main/include -I$(top_builddir)/src/main/include


liblog4cxx_la_SOURCES = \
        action.cpp \
        appenderattachableimpl.cpp \
        appenderskeleton.cpp \
        aprinitializer.cpp \
        asyncappender.cpp \
        basicconfigurator.cpp \
        bufferedwriter.cpp \
        bytearrayinputstream.cpp \
        bytearrayoutputstream.cpp \
        bytebuffer.cpp \
        cacheddateformat.cpp \
        charsetdecoder.cpp \
        charsetencoder.cpp \
        class.cpp \
        classnamepatternconverter.cpp \
        classregistration.cpp \
        condition.cpp \
        configurator.cpp \
        consoleappender.cpp \
        cyclicbuffer.cpp \
        dailyrollingfileappender.cpp \
        datagrampacket.cpp \
        datagramsocket.cpp \
        date.cpp \
        dateformat.cpp \
        datelayout.cpp \
        datepatternconverter.cpp \
        defaultloggerfactory.cpp \
        defaultconfigurator.cpp \
        defaultrepositoryselector.cpp \
        domconfigurator.cpp \
        exception.cpp \
        fallbackerrorhandler.cpp \
        file.cpp \
        fileappender.cpp \
        filedatepatternconverter.cpp \
        fileinputstream.cpp \
        filelocationpatternconverter.cpp \
        fileoutputstream.cpp \
        filerenameaction.cpp \
        filewatchdog.cpp \
        filter.cpp \
        filterbasedtriggeringpolicy.cpp \
        fixedwindowrollingpolicy.cpp \
        formattinginfo.cpp \
        fulllocationpatternconverter.cpp \
        gzcompressaction.cpp \
        hierarchy.cpp \
        htmllayout.cpp \
        inetaddress.cpp \
        inputstream.cpp \
        inputstreamreader.cpp \
        integer.cpp \
        integerpatternconverter.cpp \
        layout.cpp\
        level.cpp \
        levelmatchfilter.cpp \
        levelrangefilter.cpp \
        levelpatternconverter.cpp \
        linelocationpatternconverter.cpp \
        lineseparatorpatternconverter.cpp \
        literalpatternconverter.cpp \
        loggerpatternconverter.cpp \
        loggingeventpatternconverter.cpp \
        loader.cpp\
        locale.cpp\
        locationinfo.cpp\
        logger.cpp \
        loggingevent.cpp \
        loglog.cpp \
        logmanager.cpp \
        logstream.cpp \
        manualtriggeringpolicy.cpp \
        messagebuffer.cpp \
        messagepatternconverter.cpp \
        methodlocationpatternconverter.cpp \
        mdc.cpp \
        mutex.cpp \
        nameabbreviator.cpp \
        namepatternconverter.cpp \
        ndcpatternconverter.cpp \
        ndc.cpp \
        nteventlogappender.cpp \
        objectimpl.cpp \
        objectptr.cpp \
        objectoutputstream.cpp \
        obsoleterollingfileappender.cpp \
        odbcappender.cpp \
        onlyonceerrorhandler.cpp \
        optionconverter.cpp \
        outputdebugstringappender.cpp \
        outputstream.cpp \
        outputstreamwriter.cpp \
        patternconverter.cpp \
        patternlayout.cpp \
        patternparser.cpp \
        pool.cpp \
        properties.cpp \
        propertiespatternconverter.cpp \
        propertyconfigurator.cpp \
        propertyresourcebundle.cpp \
        propertysetter.cpp \
        reader.cpp \
        relativetimedateformat.cpp \
        relativetimepatternconverter.cpp \
        resourcebundle.cpp \
        rollingfileappender.cpp \
        rollingpolicy.cpp \
        rollingpolicybase.cpp \
        rolloverdescription.cpp \
        rootlogger.cpp \
        serversocket.cpp \
        simpledateformat.cpp \
        simplelayout.cpp \
        sizebasedtriggeringpolicy.cpp \
        smtpappender.cpp \
        socket.cpp \
        socketappender.cpp \
        socketappenderskeleton.cpp \
        sockethubappender.cpp \
        socketoutputstream.cpp \
        strftimedateformat.cpp \
        stringhelper.cpp \
        stringmatchfilter.cpp \
        stringtokenizer.cpp \
        synchronized.cpp \
        syslogappender.cpp \
        syslogwriter.cpp \
        system.cpp \
        systemerrwriter.cpp \
        systemoutwriter.cpp \
        telnetappender.cpp \
        threadcxx.cpp \
        threadlocal.cpp \
        threadspecificdata.cpp \
        threadpatternconverter.cpp \
        throwableinformationpatternconverter.cpp \
        timezone.cpp \
        timebasedrollingpolicy.cpp \
        transform.cpp \
        triggeringpolicy.cpp \
        transcoder.cpp \
        ttcclayout.cpp \
        writer.cpp \
        writerappender.cpp \
        xmllayout.cpp\
        xmlsocketappender.cpp \
        zipcompressaction.cpp

AM_CPPFLAGS = @CPPFLAGS_ODBC@
liblog4cxx_la_LDFLAGS = -version-info @LT_VERSION@ @LIBS_ODBC@ -@APR_LIBS@

