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

#ifndef _LOG4CXX_CONFIG_PROPERTYSETTER_H
#define _LOG4CXX_CONFIG_PROPERTYSETTER_H

#include <log4cxx/logstring.h>
#include <log4cxx/helpers/objectptr.h>

namespace log4cxx
{
        namespace helpers
        {
                class Object;
                typedef ObjectPtrT<Object> ObjectPtr;

                class Properties;
                class Pool;
        }

        namespace config
        {
                /**
                General purpose Object property setter. Clients repeatedly invokes
                {@link #setProperty setProperty(name,value)} in order to invoke setters
                on the Object specified in the constructor.

                <p>Usage:
                <pre>
                PropertySetter ps(anObject);
                ps.set("name", "Joe");
                ps.set("age", "32");
                ps.set("isMale", "true");
                </pre>
                will cause the invocations anObject->setOption("name", "Joe"),
                anObject->setOption("age", "32") and anObject->setOption("isMale", "true")
                if the spi::OptionHandler interface is supported by anObject.
                */
                class LOG4CXX_EXPORT PropertySetter
                {
                protected:
                        helpers::ObjectPtr obj;

                public:
                        /**
                        Create a new PropertySetter for the specified Object. This is done
                        in prepartion for invoking #setProperty one or more times.

                        @param obj  the object for which to set properties
                        */
                        PropertySetter(const helpers::ObjectPtr& obj);

                        /**
                        Set the properties of an object passed as a parameter in one
                        go. The <code>properties</code> are parsed relative to a
                        <code>prefix</code>.

                        @param obj The object to configure.
                        @param properties A java.util.Properties containing keys and values.
                        @param prefix Only keys having the specified prefix will be set.
                        @param p pool to use for any allocations required during call.
                        */
                        static void setProperties(const helpers::ObjectPtr& obj,
                                helpers::Properties& properties,
                                const LogString& prefix,
                                log4cxx::helpers::Pool& p);

                        /**
                        Set the properites for the object that match the
                        <code>prefix</code> passed as parameter.
                        */
                        void setProperties(helpers::Properties& properties,
                            const LogString& prefix,
                            log4cxx::helpers::Pool& p);

                        /**
                        Set a property on this PropertySetter's Object. If the underlying
                        Object supports the spi::OptionHandler interface, the
                        {@link spi::OptionHandler#setOption setOption} method is called.

                        @param option   name of the property
                        @param value   String value of the property
                        @param p pool to use for any allocations required during call.
                        */
                        void setProperty(const LogString& option,
                            const LogString& value,
                            log4cxx::helpers::Pool& p);

                        void activate(log4cxx::helpers::Pool& p);
                }; // class PropertySetter
        }  // namespace config;
} // namespace log4cxx

#endif //_LOG4CXX_CONFIG_PROPERTYSETTER_H
