/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _LOG4CXX_XML_DOM_CONFIGURATOR_H
#define _LOG4CXX_XML_DOM_CONFIGURATOR_H

#if defined(_MSC_VER)
#pragma warning (push)
#pragma warning ( disable: 4231 4251 4275 4786 )
#endif



#include <log4cxx/logstring.h>
#include <map>
#include <log4cxx/appender.h>
#include <log4cxx/layout.h>
#include <log4cxx/logger.h>
#include <log4cxx/helpers/properties.h>
#include <log4cxx/spi/configurator.h>
#include <log4cxx/helpers/charsetdecoder.h>
#include <log4cxx/spi/filter.h>
#include <log4cxx/rolling/triggeringpolicy.h>
#include <log4cxx/rolling/rollingpolicy.h>
#include <log4cxx/file.h>
#include <log4cxx/config/propertysetter.h>

extern "C" {
   struct apr_xml_doc;
   struct apr_xml_elem;
}

namespace log4cxx
{

        namespace xml
        {

              /**
              Use this class to initialize the log4cxx environment using a DOM tree.

              <p>Sometimes it is useful to see how log4cxx is reading configuration
              files. You can enable log4cxx internal logging by setting the
              <code>debug</code> attribute in the
              <code>log4cxx</code> element. As in
              <pre>
                      &lt;log4j:configuration <b>debug="true"</b> xmlns:log4j="http://jakarta.apache.org/log4j/">
                      ...
                      &lt;/log4j:configuration>
              </pre>

              <p>There are sample XML files included in the package.
              */
                class LOG4CXX_EXPORT DOMConfigurator :
                        virtual public spi::Configurator,
                        virtual public helpers::ObjectImpl
                {
                protected:
                        typedef std::map<LogString, AppenderPtr> AppenderMap;
                        /**
                        Used internally to parse appenders by IDREF name.
                        */
                        AppenderPtr findAppenderByName(
                                log4cxx::helpers::Pool& p,
                                log4cxx::helpers::CharsetDecoderPtr& utf8Decoder,
                                apr_xml_elem* elem,
                                apr_xml_doc* doc,
                                const LogString& appenderName,
                                AppenderMap& appenders);

                        /**
                        Used internally to parse appenders by IDREF element.
                        */
                        AppenderPtr findAppenderByReference(
                                log4cxx::helpers::Pool& p,
                                log4cxx::helpers::CharsetDecoderPtr& utf8Decoder,
                                apr_xml_elem* appenderRef,
                                apr_xml_doc* doc,
                                AppenderMap& appenders);

                        /**
                        Used internally to parse an appender element.
                        */
                        AppenderPtr parseAppender(
                            log4cxx::helpers::Pool& p,
                            log4cxx::helpers::CharsetDecoderPtr& utf8Decoder,
                            apr_xml_elem* appenderElement,
                            apr_xml_doc* doc,
                            AppenderMap& appenders);

                        /**
                        Used internally to parse an {@link spi::ErrorHandler ErrorHandler } element.
                        */
                        void parseErrorHandler(
                            log4cxx::helpers::Pool& p,
                            log4cxx::helpers::CharsetDecoderPtr& utf8Decoder,
                            apr_xml_elem* element, 
                            AppenderPtr& appender,
                            apr_xml_doc* doc,
                            AppenderMap& appenders);

                        /**
                         Used internally to parse a filter element.
                        */
                        void parseFilters(
                           log4cxx::helpers::Pool& p,
                           log4cxx::helpers::CharsetDecoderPtr& utf8Decoder,
                           apr_xml_elem* element,
                           std::vector<log4cxx::spi::FilterPtr>& filters);

                        /**
                        Used internally to parse a logger element.
                        */
                        void parseLogger(
                            log4cxx::helpers::Pool& p,
                            log4cxx::helpers::CharsetDecoderPtr& utf8Decoder,
                            apr_xml_elem* loggerElement,
                            apr_xml_doc* doc,
                            AppenderMap& appenders);

                        /**
                         Used internally to parse the logger factory element.
                        */
                        void parseLoggerFactory(
                            log4cxx::helpers::Pool& p,
                            log4cxx::helpers::CharsetDecoderPtr& utf8Decoder,
                            apr_xml_elem* factoryElement);

                        /**
                         Used internally to parse the logger factory element.
                        */
                        log4cxx::helpers::ObjectPtr parseTriggeringPolicy(
                            log4cxx::helpers::Pool& p,
                            log4cxx::helpers::CharsetDecoderPtr& utf8Decoder,
                            apr_xml_elem* factoryElement);

                        /**
                         Used internally to parse the logger factory element.
                        */
                        log4cxx::rolling::RollingPolicyPtr parseRollingPolicy(
                            log4cxx::helpers::Pool& p,
                            log4cxx::helpers::CharsetDecoderPtr& utf8Decoder,
                            apr_xml_elem* factoryElement);

                        /**
                         Used internally to parse the root logger element.
                        */
                        void parseRoot(log4cxx::helpers::Pool& p, 
                            log4cxx::helpers::CharsetDecoderPtr& utf8Decoder,
                            apr_xml_elem* rootElement, apr_xml_doc* doc, AppenderMap& appenders);

                        /**
                         Used internally to parse the children of a logger element.
                        */
                        void parseChildrenOfLoggerElement(
                                log4cxx::helpers::Pool& p,
                                log4cxx::helpers::CharsetDecoderPtr& utf8Decoder,
                                apr_xml_elem* catElement,
                                LoggerPtr logger, bool isRoot,
                                apr_xml_doc* doc,
                                AppenderMap& appenders);

                        /**
                         Used internally to parse a layout element.
                        */
                        LayoutPtr parseLayout(
                            log4cxx::helpers::Pool& p,
                            log4cxx::helpers::CharsetDecoderPtr& utf8Decoder,
                            apr_xml_elem* layout_element);

                        /**
                         Used internally to parse a level  element.
                        */
                        void parseLevel(
                                log4cxx::helpers::Pool& p,
                                log4cxx::helpers::CharsetDecoderPtr& utf8Decoder,
                                apr_xml_elem* element,
                                LoggerPtr logger, bool isRoot);

                        void setParameter(
                                log4cxx::helpers::Pool& p,
                                log4cxx::helpers::CharsetDecoderPtr& utf8Decoder,
                                apr_xml_elem* elem,
                        log4cxx::config::PropertySetter& propSetter);

                        /**
                         Used internally to configure the log4cxx framework from
                         an in-memory representation of an XML document.
                        */
                        void parse(
                            log4cxx::helpers::Pool& p,
                            log4cxx::helpers::CharsetDecoderPtr& utf8Decoder,
                            apr_xml_elem* element, 
                            apr_xml_doc* doc,
                            AppenderMap& appenders);

                public:
                        DOMConfigurator();

                        DECLARE_LOG4CXX_OBJECT(DOMConfigurator)
                        BEGIN_LOG4CXX_CAST_MAP()
                                LOG4CXX_CAST_ENTRY(spi::Configurator)
                        END_LOG4CXX_CAST_MAP()

                        DOMConfigurator(log4cxx::helpers::Pool& p);

                        void addRef() const;
                        void releaseRef() const;

                        /**
                        A static version of #doConfigure.
                        */
                        static void configure(const std::string& filename);
#if LOG4CXX_WCHAR_T_API
                        static void configure(const std::wstring& filename);
#endif
#if LOG4CXX_UNICHAR_API
                        static void configure(const std::basic_string<UniChar>& filename);
#endif
#if LOG4CXX_CFSTRING_API
                        static void configure(const CFStringRef& filename);
#endif
                        /**
                        Like #configureAndWatch(const std::string& configFilename, long delay)
                        except that the default delay as defined by
                        log4cxx::helpers::FileWatchdog#DEFAULT_DELAY is used.
                        @param configFilename A log4j configuration file in XML format.
                        */
                        static void configureAndWatch(const std::string& configFilename);
#if LOG4CXX_WCHAR_T_API
                        static void configureAndWatch(const std::wstring& configFilename);
#endif
#if LOG4CXX_UNICHAR_API
                        static void configureAndWatch(const std::basic_string<UniChar>& configFilename);
#endif
#if LOG4CXX_CFSTRING_API
                        static void configureAndWatch(const CFStringRef& configFilename);
#endif
                        /**
                        Read the configuration file <code>configFilename</code> if it
                        exists. Moreover, a thread will be created that will periodically
                        check if <code>configFilename</code> has been created or
                        modified. The period is determined by the <code>delay</code>
                        argument. If a change or file creation is detected, then
                        <code>configFilename</code> is read to configure log4cxx.

                        @param configFilename A log4j configuration file in XML format.
                        @param delay The delay in milliseconds to wait between each check.
                        */
                        static void configureAndWatch(const std::string& configFilename,
                                long delay);
#if LOG4CXX_WCHAR_T_API
                        static void configureAndWatch(const std::wstring& configFilename,
                                long delay);
#endif
#if LOG4CXX_UNICHAR_API
                        static void configureAndWatch(const std::basic_string<UniChar>& configFilename,
                                long delay);
#endif
#if LOG4CXX_CFSTRING_API
                        static void configureAndWatch(const CFStringRef& configFilename,
                                long delay);
#endif

                        /**
                        Interpret the XML file pointed by <code>filename</code> and set up
                        log4cxx accordingly.
                        <p>The configuration is done relative to the hierarchy parameter.
                        @param filename The file to parse.
                        @param repository The hierarchy to operation upon.
                        */
                        void doConfigure(const File& filename,
                                spi::LoggerRepositoryPtr& repository);

                protected:
                        static LogString getAttribute(
                            log4cxx::helpers::CharsetDecoderPtr& utf8Decoder,
                            apr_xml_elem*, 
                            const std::string& attrName);

                        LogString subst(const LogString& value);

                protected:
                        helpers::Properties props;
                        spi::LoggerRepositoryPtr repository;
                        spi::LoggerFactoryPtr loggerFactory;

                 private:
                        //   prevent assignment or copy statements
                        DOMConfigurator(const DOMConfigurator&);
                        DOMConfigurator& operator=(const DOMConfigurator&);

                };
            LOG4CXX_PTR_DEF(DOMConfigurator);
        }  // namespace xml
} // namespace log4cxx

#if defined(_MSC_VER)
#pragma warning (pop)
#endif

#endif // _LOG4CXX_XML_DOM_CONFIGURATOR_H
