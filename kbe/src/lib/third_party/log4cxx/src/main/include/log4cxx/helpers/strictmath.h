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

#ifndef _LOG4CXX_HELPERS_STRICTMATH_H
#define _LOG4CXX_HELPERS_STRICTMATH_H

#include <log4cxx/log4cxx.h>

namespace log4cxx
{
        namespace helpers
        {
                /**
                The class StrictMath contains methods for performing basic numeric
                operations
                */
                class StrictMath
                {
                public:
                        template<typename _type> static inline const _type&
                                minimum(const _type& a, const _type& b)
                        {
                                return (a < b) ? a : b;
                        }

                        template<typename _type> static inline const _type&
                                maximum(const _type& a, const _type& b)
                        {
                                return (a > b) ? a : b;
                        }
                }; // class StrictMath
        }  // namespace helpers
} // namespace log4cx

#endif //_LOG4CXX_HELPERS_STRICTMATH_H
