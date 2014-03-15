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

#include <log4cxx/logstring.h>
#include <log4cxx/xml/xmllayout.h>
#include <log4cxx/spi/loggingevent.h>
#include <log4cxx/helpers/optionconverter.h>
#include <log4cxx/level.h>
#include <log4cxx/helpers/transform.h>
#include <log4cxx/helpers/iso8601dateformat.h>
#include <log4cxx/helpers/stringhelper.h>
#include <log4cxx/helpers/transcoder.h>
#include <log4cxx/ndc.h>


using namespace log4cxx;
using namespace log4cxx::helpers;
using namespace log4cxx::spi;
using namespace log4cxx::xml;

IMPLEMENT_LOG4CXX_OBJECT(XMLLayout)

XMLLayout::XMLLayout()
: locationInfo(false), properties(false)
{
}

void XMLLayout::setOption(const LogString& option,
        const LogString& value)
{
        if (StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("LOCATIONINFO"), LOG4CXX_STR("locationinfo")))
        {
                setLocationInfo(OptionConverter::toBoolean(value, false));
        }
        if (StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("PROPERTIES"), LOG4CXX_STR("properties")))
        {
                setProperties(OptionConverter::toBoolean(value, false));
        }
}

void XMLLayout::format(LogString& output,
     const spi::LoggingEventPtr& event,
     Pool& p) const
{
        output.append(LOG4CXX_STR("<log4j:event logger=\""));
        Transform::appendEscapingTags(output, event->getLoggerName());
        output.append(LOG4CXX_STR("\" timestamp=\""));
        StringHelper::toString(event->getTimeStamp()/1000L, p, output);
        output.append(LOG4CXX_STR("\" level=\""));
        Transform::appendEscapingTags(output, event->getLevel()->toString());
        output.append(LOG4CXX_STR("\" thread=\""));
        Transform::appendEscapingTags(output, event->getThreadName());
        output.append(LOG4CXX_STR("\">"));
        output.append(LOG4CXX_EOL);

        output.append(LOG4CXX_STR("<log4j:message><![CDATA["));
        // Append the rendered message. Also make sure to escape any
        // existing CDATA sections.
        Transform::appendEscapingCDATA(output, event->getRenderedMessage());
        output.append(LOG4CXX_STR("]]></log4j:message>"));
        output.append(LOG4CXX_EOL);

        LogString ndc;
        if(event->getNDC(ndc)) {
                output.append(LOG4CXX_STR("<log4j:NDC><![CDATA["));
                Transform::appendEscapingCDATA(output, ndc);
                output.append(LOG4CXX_STR("]]></log4j:NDC>"));
                output.append(LOG4CXX_EOL);
        }

        if(locationInfo)
        {
                output.append(LOG4CXX_STR("<log4j:locationInfo class=\""));
                const LocationInfo& locInfo = event->getLocationInformation();
                LOG4CXX_DECODE_CHAR(className, locInfo.getClassName());
                Transform::appendEscapingTags(output, className);
                output.append(LOG4CXX_STR("\" method=\""));
                LOG4CXX_DECODE_CHAR(method, locInfo.getMethodName());
                Transform::appendEscapingTags(output, method);
                output.append(LOG4CXX_STR("\" file=\""));
                LOG4CXX_DECODE_CHAR(fileName, locInfo.getFileName());
                Transform::appendEscapingTags(output, fileName);
                output.append(LOG4CXX_STR("\" line=\""));
                StringHelper::toString(locInfo.getLineNumber(), p, output);
                output.append(LOG4CXX_STR("\"/>"));
                output.append(LOG4CXX_EOL);
        }
        
        if (properties) {
            LoggingEvent::KeySet propertySet(event->getPropertyKeySet());
            LoggingEvent::KeySet keySet(event->getMDCKeySet());
            if (!(keySet.empty() && propertySet.empty())) {
                output.append(LOG4CXX_STR("<log4j:properties>"));
                output.append(LOG4CXX_EOL);
                for (LoggingEvent::KeySet::const_iterator i = keySet.begin();
                        i != keySet.end(); 
                        i++) {
                        LogString key(*i);
                        LogString value;
                        if(event->getMDC(key, value)) {
                            output.append(LOG4CXX_STR("<log4j:data name=\""));
                            Transform::appendEscapingTags(output, key);
                            output.append(LOG4CXX_STR("\" value=\""));
                            Transform::appendEscapingTags(output, value);
                            output.append(LOG4CXX_STR("\"/>"));
                            output.append(LOG4CXX_EOL);
                        }
                }
            for (LoggingEvent::KeySet::const_iterator i2 = propertySet.begin();
                        i2 != propertySet.end(); 
                        i2++) {
                        LogString key(*i2);
                        LogString value;
                        if(event->getProperty(key, value)) {
                            output.append(LOG4CXX_STR("<log4j:data name=\""));
                            Transform::appendEscapingTags(output, key);
                            output.append(LOG4CXX_STR("\" value=\""));
                            Transform::appendEscapingTags(output, value);
                            output.append(LOG4CXX_STR("\"/>"));
                            output.append(LOG4CXX_EOL);
                        }
                }
                output.append(LOG4CXX_STR("</log4j:properties>"));
                output.append(LOG4CXX_EOL);
            }
        }

        output.append(LOG4CXX_STR("</log4j:event>"));
        output.append(LOG4CXX_EOL);
        output.append(LOG4CXX_EOL);        
}

