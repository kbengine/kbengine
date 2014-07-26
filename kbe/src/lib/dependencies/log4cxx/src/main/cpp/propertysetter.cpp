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
#include <log4cxx/config/propertysetter.h>
#include <log4cxx/helpers/object.h>
#include <log4cxx/helpers/loglog.h>
#include <log4cxx/helpers/optionconverter.h>
#include <log4cxx/spi/optionhandler.h>
#include <log4cxx/helpers/properties.h>
#include <log4cxx/appender.h>
#include <log4cxx/layout.h>
#include <log4cxx/helpers/pool.h>

using namespace log4cxx;
using namespace log4cxx::helpers;
using namespace log4cxx::spi;
using namespace log4cxx::config;

PropertySetter::PropertySetter(const helpers::ObjectPtr& obj1) : obj(obj1)
{
}

void PropertySetter::setProperties(const helpers::ObjectPtr& obj,
     helpers::Properties& properties,
     const LogString& prefix,
     Pool& p)
{
        PropertySetter(obj).setProperties(properties, prefix, p);
}


void PropertySetter::setProperties(helpers::Properties& properties,
        const LogString& prefix,
        Pool& p)
{
        int len = prefix.length();

        std::vector<LogString> names = properties.propertyNames();
        std::vector<LogString>::iterator it;

        for (it = names.begin(); it != names.end(); it++)
        {
                LogString key = *it;

                // handle only properties that start with the desired frefix.
                if (key.find(prefix) == 0)
                {
                        // ignore key if it contains dots after the prefix
                        if (key.find(0x2E /* '.' */, len + 1) != LogString::npos)
                        {
                                continue;
                        }

                        LogString value = OptionConverter::findAndSubst(key, properties);
                        key = key.substr(len);
                        if (key == LOG4CXX_STR("layout")
                                && obj != 0
                                && obj->instanceof(Appender::getStaticClass()))
                        {
                                continue;
                        }
                        setProperty(key, value, p);
                }
        }
        activate(p);
}

void PropertySetter::setProperty(const LogString& option,
                                 const LogString& value,
                                 Pool&)
{
        if (value.empty())
                return;

        if (obj != 0 && obj->instanceof(OptionHandler::getStaticClass()))
        {
                LogLog::debug(LOG4CXX_STR("Setting option name=[") +
                        option + LOG4CXX_STR("], value=[") + value + LOG4CXX_STR("]"));
                OptionHandlerPtr(obj)->setOption(option, value);
        }
}

void PropertySetter::activate(Pool& p)
{
        if (obj != 0 && obj->instanceof(OptionHandler::getStaticClass()))
        {
                OptionHandlerPtr(obj)->activateOptions(p);
        }
}
