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

#ifndef _LOG4CXX_HELPERS_TRANSFORM_H
#define _LOG4CXX_HELPERS_TRANSFORM_H

#include <log4cxx/logstring.h>

namespace log4cxx
{
        namespace helpers
        {
                /**
                Utility class for transforming strings.
                */
                class LOG4CXX_EXPORT Transform
                {
                public:
                        /**
                        * This method takes a string which may contain HTML tags (ie,
                        * &lt;b&gt;, &lt;table&gt;, etc) and replaces any '<' and '>'
                        * characters with respective predefined entity references.
                        *
                        * @param buf output stream where to write the modified string.
                        * @param input The text to be converted.
                        * @return The input string with the characters '<' and '>' replaced with
                        *  &amp;lt; and &amp;gt; respectively.
                        * */
                        static void appendEscapingTags(
                                LogString& buf, const LogString& input);

                        /**
                        * Ensures that embeded CDEnd strings (]]>) are handled properly
                        * within message, NDC and throwable tag text.
                        *
                        * @param buf output stream holding the XML data to this point.  The
                        * initial CDStart (<![CDATA[) and final CDEnd (]]>) of the CDATA
                        * section are the responsibility of the calling method.
                        * @param input The String that is inserted into an existing CDATA
                        * Section within buf.
                        */
                        static void appendEscapingCDATA(
                                LogString& buf, const LogString& input);
                }; // class Transform
        }  // namespace helpers
} //namespace log4cxx

#endif // _LOG4CXX_HELPERS_TRANSFORM_H
