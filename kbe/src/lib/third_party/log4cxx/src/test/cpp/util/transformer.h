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

#ifndef _LOG4CXX_TESTS_UTIL_TRANSFORMER_H
#define _LOG4CXX_TESTS_UTIL_TRANSFORMER_H

#include "filter.h"
#include <vector>

extern "C" {
struct apr_pool_t;
}

namespace log4cxx
{
       class File;

        class Transformer
        {
        public:
                static void transform(const File& in,
                        const File& out,
                        const std::vector<Filter *>& filters);

                static void transform(const File& in,
                        const File& out,
                        const Filter& filter);
                        
                static void transform(const File& in,
                                      const File& out,
                                      const std::vector< log4cxx::Filter::PatternReplacement >& patterns);
        private:
                static void copyFile(const File& in,
                                      const File& out);
                static void createSedCommandFile(const std::string& regexName, 
                    const log4cxx::Filter::PatternList& patterns,
                    apr_pool_t* pool);
        
        };
}

#endif //_LOG4CXX_TESTS_UTIL_TRANSFORMER_H
