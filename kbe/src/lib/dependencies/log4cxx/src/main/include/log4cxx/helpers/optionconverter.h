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

#ifndef _LOG4CXX_HELPER_OPTION_CONVERTER_H
#define _LOG4CXX_HELPER_OPTION_CONVERTER_H

#include <log4cxx/logstring.h>
#include <log4cxx/helpers/objectptr.h>

namespace log4cxx
{
        class Level;
        class File;
        typedef helpers::ObjectPtrT<Level> LevelPtr;

        namespace spi
        {
                class LoggerRepository;
                typedef helpers::ObjectPtrT<LoggerRepository> LoggerRepositoryPtr;
        }

        namespace helpers
        {
                class Properties;

                class Object;
                typedef ObjectPtrT<Object> ObjectPtr;

                class Class;

                /** A convenience class to convert property values to specific types.*/
                class LOG4CXX_EXPORT OptionConverter
                {
                /** OptionConverter is a static class. */
                private:
                        OptionConverter() {}

                public:
                        static LogString convertSpecialChars(const LogString& s);

                        /**
                        If <code>value</code> is "true", then <code>true</code> is
                        returned. If <code>value</code> is "false", then
                        <code>true</code> is returned. Otherwise, <code>default</code> is
                        returned.

                        <p>Case of value is unimportant.
                        */
                        static bool toBoolean(const LogString& value, bool dEfault);
                        static int toInt(const LogString& value, int dEfault);
                        static long toFileSize(const LogString& value, long dEfault);
                        static LevelPtr toLevel(const LogString& value,
                                const LevelPtr& defaultValue);

                        /**
                Find the value corresponding to <code>key</code> in
                <code>props</code>. Then perform variable substitution on the
                found value.
                        */
                        static LogString findAndSubst(const LogString& key, Properties& props);

/**
Perform variable substitution in string <code>val</code> from the
values of keys found in the system propeties.

<p>The variable substitution delimeters are <b>${</b> and <b>}</b>.

<p>For example, if the System properties contains "key=value", then
the call
<pre>
String s = OptionConverter.substituteVars("Value of key is ${key}.");
</pre>

will set the variable <code>s</code> to "Value of key is value.".

<p>If no value could be found for the specified key, then the
<code>props</code> parameter is searched, if the value could not
be found there, then substitution defaults to the empty string.

<p>For example, if system propeties contains no value for the key
"inexistentKey", then the call

<pre>
String s = OptionConverter.subsVars("Value of inexistentKey is [${inexistentKey}]");
</pre>
will set <code>s</code> to "Value of inexistentKey is []"

<p>An IllegalArgumentException is thrown if
<code>val</code> contains a start delimeter "${" which is not
balanced by a stop delimeter "}". </p>

@param val The string on which variable substitution is performed.
@param props The properties from which variable substitution is performed.
@throws IllegalArgumentException if <code>val</code> is malformed.
*/
                        static LogString substVars(const LogString& val, Properties& props);

                        /**
                         *  Gets the specified system property.  
                        @param key The key to search for.
                        @param def The default value to return.
                        @return the string value of the system property, or the default
                        value if there is no property with that key.
                        */
                        static LogString getSystemProperty(const LogString& key, const LogString& def);

                        /**
                        Instantiate an object given a class name. Check that the
                        <code>className</code> is a subclass of
                        <code>superClass</code>. If that test fails or the object could
                        not be instantiated, then <code>defaultValue</code> is returned.

                        @param className The fully qualified class name of the object to instantiate.
                        @param superClass The class to which the new object should belong.
                        @param defaultValue The object to return in case of non-fulfillment
                        */
                        static ObjectPtr instantiateByClassName(const LogString& className,
                                const Class& superClass, const ObjectPtr& defaultValue);

                        static ObjectPtr instantiateByKey(Properties& props,
                                const LogString& key, const Class& superClass,
                                const ObjectPtr& defaultValue);

                        /**
                        Configure log4cxx given a configFileName.

                        <p>The configFileName must point to a file which will be
                        interpreted by a new instance of a log4cxx configurator.

                        <p>All configurations steps are taken on the
                        <code>hierarchy</code> passed as a parameter.

                        <p>
                        @param configFileName The location of the configuration file.
                        @param clazz The classname, of the log4cxx configurator which
                        will parse the file <code>configFileName</code>. This must be
                        a subclass of Configurator, or null. If this value is null then
                        a default configurator of PropertyConfigurator is used, unless the
                        filename pointed to by <code>configFileName</code> ends in '.xml',
                        in which case DOMConfigurator is used.
                        @param hierarchy The Hierarchy to act on.
                        */
                        static void selectAndConfigure(const File& configFileName,
                                const LogString& clazz, spi::LoggerRepositoryPtr& hierarchy);
                };
        }  // namespace helpers
} // namespace log4cxx

#endif //_LOG4CXX_HELPER_OPTION_CONVERTER_H

