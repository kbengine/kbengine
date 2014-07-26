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
#define __STDC_CONSTANT_MACROS
#include <log4cxx/logstring.h>
#include <log4cxx/helpers/filewatchdog.h>
#include <log4cxx/helpers/loglog.h>
#include <apr_time.h>
#include <apr_thread_proc.h>
#include <apr_atomic.h>
#include <log4cxx/helpers/transcoder.h>


using namespace log4cxx;
using namespace log4cxx::helpers;

long FileWatchdog::DEFAULT_DELAY = 60000;

#if APR_HAS_THREADS

FileWatchdog::FileWatchdog(const File& file1)
 : file(file1), delay(DEFAULT_DELAY), lastModif(0),
warnedAlready(false), interrupted(0), thread()
{
}

FileWatchdog::~FileWatchdog() {
   apr_atomic_set32(&interrupted, 0xFFFF);
   thread.join();
}

void FileWatchdog::checkAndConfigure()
{
    Pool pool1;
   if (!file.exists(pool1))
   {
              if(!warnedAlready)
              {
                      LogLog::debug(((LogString) LOG4CXX_STR("["))
                         + file.getPath()
                         + LOG4CXX_STR("] does not exist."));
                      warnedAlready = true;
              }
   }
   else
   {
        apr_time_t thisMod = file.lastModified(pool1);
      if (thisMod > lastModif)
      {
         lastModif = thisMod;
         doOnChange();
         warnedAlready = false;
      }
   }
}

void* APR_THREAD_FUNC FileWatchdog::run(apr_thread_t* /* thread */, void* data) {
   FileWatchdog* pThis = (FileWatchdog*) data;

   unsigned int interrupted = apr_atomic_read32(&pThis->interrupted);
    while(!interrupted)
   {
      apr_sleep(APR_INT64_C(1000) * pThis->delay);
      interrupted = apr_atomic_read32(&pThis->interrupted);
      if (!interrupted) {
        pThis->checkAndConfigure();
        interrupted = apr_atomic_read32(&pThis->interrupted);
      }
    }
   return NULL;
}

void FileWatchdog::start()
{
   checkAndConfigure();

   thread.run(run, this);
}

#endif
