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

#ifndef _LOG4CXX_HELPERS_FILEWATCHDOG_H
#define _LOG4CXX_HELPERS_FILEWATCHDOG_H

#include <log4cxx/logstring.h>
#include <time.h>
#include <log4cxx/helpers/pool.h>
#include <log4cxx/helpers/thread.h>
#include <log4cxx/file.h>

namespace log4cxx
{
        namespace helpers
        {

                /**
                Check every now and then that a certain file has not changed. If it
                has, then call the #doOnChange method.
                */
                class LOG4CXX_EXPORT FileWatchdog
                {
                public:
                        virtual ~FileWatchdog();
                        /**
                        The default delay between every file modification check, set to 60
                        seconds.  */
                        static long DEFAULT_DELAY /*= 60000*/;

                protected:
                        /**
                        The name of the file to observe  for changes.
                        */
                        File file;

                        /**
                        The delay to observe between every check.
                        By default set DEFAULT_DELAY.*/
                        long delay;
                        log4cxx_time_t lastModif;
                        bool warnedAlready;
                        volatile unsigned int interrupted;

                protected:
                        FileWatchdog(const File& filename);
                        virtual void doOnChange() = 0;
                        void checkAndConfigure();

                public:
                        /**
                        Set the delay to observe between each check of the file changes.
                        */
                        void setDelay(long delay1)
                                { this->delay = delay1; }

                        void start();

                private:
                    static void* LOG4CXX_THREAD_FUNC run(apr_thread_t* thread, void* data);
                        Pool pool;
                        Thread thread;

                        FileWatchdog(const FileWatchdog&);
                        FileWatchdog& operator=(const FileWatchdog&);

                };
        }  // namespace helpers
} // namespace log4cxx


#endif // _LOG4CXX_HELPERS_FILEWATCHDOG_H
