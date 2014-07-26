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

#ifndef _LOG4CXX_HELPERS_CYCLICBUFFER_H
#define _LOG4CXX_HELPERS_CYCLICBUFFER_H

#include <log4cxx/spi/loggingevent.h>

namespace log4cxx
{
        namespace helpers
        {
                /**
                CyclicBuffer is used by other appenders to hold instances of
                {@link log4cxx::spi::LoggingEvent LoggingEvent} for immediate 
                or deferred display.
                <p>This buffer gives read access to any element in the buffer not
                just the first or last element.
                */
                class LOG4CXX_EXPORT CyclicBuffer
                {
                        log4cxx::spi::LoggingEventList ea;
                        int first;
                        int last;
                        int numElems;
                        int maxSize;

                public:
                        /**
                        Instantiate a new CyclicBuffer of at most <code>maxSize</code>
                        events.
                        The <code>maxSize</code> argument must a positive integer.
                        @param maxSize The maximum number of elements in the buffer.
                        @throws IllegalArgumentException if <code>maxSize</code>
                        is negative.
                        */
                        CyclicBuffer(int maxSize);
                        ~CyclicBuffer();

                        /**
                        Add an <code>event</code> as the last event in the buffer.
                        */
                        void add(const spi::LoggingEventPtr& event);

                        /**
                        Get the <i>i</i>th oldest event currently in the buffer. If
                        <em>i</em> is outside the range 0 to the number of elements
                        currently in the buffer, then <code>null</code> is returned.
                        */
                        spi::LoggingEventPtr get(int i);

                        int getMaxSize() const
                                { return maxSize; }

                        /**
                        Get the oldest (first) element in the buffer. The oldest element
                        is removed from the buffer.
                        */
                        spi::LoggingEventPtr get();

                        /**
                        Get the number of elements in the buffer. This number is
                        guaranteed to be in the range 0 to <code>maxSize</code>
                        (inclusive).
                        */
                        int length() const
                                { return numElems; }

                        /**
                        Resize the cyclic buffer to <code>newSize</code>.
                        @throws IllegalArgumentException if <code>newSize</code> is negative.
                        */
                        void resize(int newSize);
                }; // class CyclicBuffer
        }  //namespace helpers
} //namespace log4cxx

#endif //_LOG4CXX_HELPERS_CYCLICBUFFER_H
