<!--
 Licensed to the Apache Software Foundation (ASF) under one or more
 contributor license agreements.  See the NOTICE file distributed with
 this work for additional information regarding copyright ownership.
 The ASF licenses this file to You under the Apache License, Version 2.0
 (the "License"); you may not use this file except in compliance with
 the License.  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.

-->
<xsl:transform xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xsl:version="1.0">

   <xsl:output method="xml" indent="yes"/>

   <xsl:apply-templates select="/"/>

   <xsl:template match="/">
  <xsl:comment>

 Licensed to the Apache Software Foundation (ASF) under one or more
 contributor license agreements.  See the NOTICE file distributed with
 this work for additional information regarding copyright ownership.
 The ASF licenses this file to You under the Apache License, Version 2.0
 (the "License"); you may not use this file except in compliance with
 the License.  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.

  </xsl:comment>
  <document>
  <properties>
    <title>Apache log4cxx</title>
  </properties>
  <body>
  
    <release version="0.10.0" date="2008-02-29" description="First Apache release">
       <xsl:apply-templates select='/rss/channel/item'>
           <xsl:sort select="substring-after(key, '-')" data-type="number"/>
       </xsl:apply-templates>
     </release>

<release version="0.9.7" date="2004-05-10">
<action type="fix">Fixed examples source code in the "Short introduction to log4cxx".</action>
<action type="fix">Fixed, in the renaming algorithm of RollingFileAppender and
  DailyRollingFileAppender, a problem specific to Unicode.</action>
<action type="fix">Fixed conflict with Windows macros "min" and "max", by renaming
  StrictMath::min and StrictMath::max to StrictMath::minimum and
  StrictMath::maximum.</action>
<action type="add">Port to HPUX 11.0.</action>
<action type="fix">Fixed segmentation fault in PropertyConfigurator.</action>
<action type="add">Port to Solaris.</action>
<action type="fix">Fixed MutexException thrown while destroying RollingFileAppender.</action>
<action type="fix">Logging macros can be used without explicity declaring the use of log4cxx namespace.</action>
<action type="fix">Fixed static library unresolved externals for msvc 6 and 7.1</action>
</release>
<release version="0.9.6" date="2004-04-11">
<action>Timezone management has been optimized through the class TimeZone</action>
<action>Inter-thread synchronization and reference counting has been optimized</action>
<action>Reference counting now uses gcc atomic functions (bug 929078)</action>
<action>Use of StringBuffer has been optimized.</action>
<action>Support of localisation throug resourceBundles</action>
<action>SyslogAppender now uses the system function 'syslog' to log on the local host.
 (only for POSIX systems)</action>
<action>Added TimeZone configuration to PatternLayout (bug 912563)</action>
<action>Support of the DailyRollingFileAppender (feature request 842765)</action>
</release>
<release version="0.9.5" date="2004-02-04">
<action>Port of log4j Jnuit tests with Cppunit and Boost Regex.</action>
<action>Added explicit exports for MSDEV 6 and MSDEV 7 (no further need of .def files)</action>
<action>Custom levels can be configured through the DOMConfigurator and
  PropertyConfigurator classes (Level inherites from Object)</action>
<action>Added a reference counter to LoggingEvent to avoid useless copies
  (LoggingEvent inherites from Object)</action>
<action>The file log4j.xml as well as the file log4j.properties are now search
  for, in log4cxx initialization.</action>
<action>The root logger can be assigned the "OFF" level.</action>
<action>Added MSVC6 project missing files mutext.cpp and condition.cpp (bug 847397)</action>
<action>condition.cpp now compiles with MSVC6 (bug 847417)</action>
<action>fixed pure virtual function call in PropertyConfigurator::configureAndWatch
  (bug 848521)</action>
<action>XMLAppender now displays correct timestamp with MSVC 6 (bug 852836)</action>
<action>SRLPORT 4.6 support.</action>
<action>Fixed an infinite loop in class Properties.</action>
<action>Fixed compilations problems with unicode.</action>
<action>Fixed SocketAppender bug concerning MDC and NDC.</action>
</release>
<release version="0.9.4" date="2003-10-25">
<action>StringBuffer has been optimized.</action>
<action>Fixed miscellaneous threading problems.</action>
<action>Added TimeZone support in PatternLayout (bug 796894)</action>
<action>Fixed threading configuration problems (bug 809125)</action>
<action>Fixed miscellaneous MSVC and cygwin compilation problems.</action>
</release>
<release version="0.9.3" date="2003-09-19">
<action>Changed tstring to log4cxx::String and tostringstream to
  log4cxx::StringBuffer.
</action>
<action>Fixed MSVC 2003 compilation erros and warnings.
</action>
<action>Added helpers for NDC and MDC.
</action>
<action>Added TimeZone support in TTCCLayout.
</action>
<action>Fixed compilation problems with logger macros (LOG4CXX_...)
</action>
<action>Fixed milliseconds formatting problem with MSVC 6.0 and 2003
</action>
<action>Fixed AsyncAppender crash
</action>
<action>Added new tests
</action>
<action>Added benchmarks
</action>
</release>
<release version="0.9.2" date="2003-08-10">
<action>Fixed FreeBSD compilation problem with pthread mutex (class CriticalSection).
</action>
<action>Fixed milliseconds formatting problem (class DateFormat).
</action>
<action>Long events (&gt; 1024 chars) are now supported in the class XMLSocketAppender.
</action>
<action>Carriage returns have been normalized in the class XMLLayout.
</action>
</release>
<release version="0.9.1" date="2003-08-06">
<action>Fixed deadlock problems in classes Logger and AsyncAppender.
</action>
<action>Fixed MSVC 6.0 compilation problems.
</action>
<action>Added MSVC 6.0 static libraty project.
</action>
<action>Default configuration for the SMTP options is "no".
</action>
</release>
<release version="0.9.0" date="2003-08-06">
<action>Added ODBCAppender (matching log4j JDBCAppender)
</action>
<action>Added SyslogAppender
</action>
<action>Added SMTPAppender (only for Linux/FreeBSD)
</action>
<action>Added BasicConfigurator
</action>
<action>Added a FileWatchDog in PropertyConfigurator and DOMConfigurator
</action>
<action>Possibility to load a custom LoggerFactory through the DOMConfigurator
</action>
<action>Changed time precision from seconds to milliseconds
</action>
<action>Added MSVC 6.0 'Unicode Debug' and 'Unicode Release' targets
</action>
<action>Added Java like System class.
</action>
</release>
<release version="0.1.1" date="2003-07-09">
<action>Fixed MSVC 6.0 compilation problems concerning the 'Release' target
</action>
<action>Added MSVC 6.0 tests projects
</action>
</release>
<release version="0.1.0" date="2003-07-08">
<action>FreeBSD Autotools/Compilation support
</action>
<action>Fixed TelnetAppender crash when a socket bind exception occured.
</action>
<action>Added log4j DTD support to XMLLayout and DOMConfigurator
</action>
<action>Can now send events in XML format over TCP (class XMLSocketAppender) for the
  log4j Chainsaw UI
</action>
<action>Now compiles with 'configure --enable-unicode' (UTF16 Unicode support)
</action>
<action>Added Java like Properties class. It's a helper for the PropertyConfigurator
</action>
<action>Added Java like objects with dynamic cast and instanciation. Custom objects
  can be configured through the DOMConfigurator and PropertyConfigurator classes
</action>
<action>Port of the PropertyConfigurator class
</action>
<action>Port of the "Map Diagnostic Context" (MDC) class
</action>
<action>Added 13 tests (try make check)
</action>
</release>
<release version="0.0.1" date="2003-05-31">
<action type="add">Loggers, Hierarchy, Filters, Appenders, Layouts, NDC
</action>
<action type="add">Appenders:
  AsyncAppender, ConsoleAppender, FileAppender, NTEventLogAppender,
  RollingFileAppender, SocketAppender, SocketHubAappender,
  TelnetAppender
</action>
<action type="add">Layouts:
  HTMLLayout, PatternLayout, SimpleLayout, TTCCLayout, XMLLayout
</action>
<action type="add">Filters:
  DenyAllFilter, LevelMatchFilter, LevelRangeFilter, StringMatchFilter

</action>
<action type="add">Configurators:
  DOMConfigurator
</action>
</release>
  </body>
</document>
</xsl:template>

<xsl:template match="item">
      <action issue="{key}"><xsl:value-of select="summary"/></action>
</xsl:template>

</xsl:transform>
