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

#include <log4cxx/spi/loggingevent.h>
#include <log4cxx/ndc.h>

#include <log4cxx/level.h>
#include <log4cxx/helpers/loglog.h>
#include <log4cxx/helpers/system.h>
#include <log4cxx/helpers/socket.h>
#if !defined(LOG4CXX)
#define LOG4CXX 1
#endif
#include <log4cxx/helpers/aprinitializer.h>
#include <log4cxx/helpers/threadspecificdata.h>
#include <log4cxx/helpers/transcoder.h>

#include <apr_time.h>
#include <apr_portable.h>
#include <apr_strings.h>
#include <log4cxx/helpers/stringhelper.h>
#include <log4cxx/helpers/objectoutputstream.h>
#include <log4cxx/helpers/bytebuffer.h>
#include <log4cxx/logger.h>
#include <log4cxx/private/log4cxx_private.h>

using namespace log4cxx;
using namespace log4cxx::spi;
using namespace log4cxx::helpers;

IMPLEMENT_LOG4CXX_OBJECT(LoggingEvent)


//
//   Accessor for start time.
//
log4cxx_time_t LoggingEvent::getStartTime() {
  return log4cxx::helpers::APRInitializer::initialize();
}

LoggingEvent::LoggingEvent() :
   ndc(0),
   mdcCopy(0),
   properties(0),
   ndcLookupRequired(true),
   mdcCopyLookupRequired(true),
   timeStamp(0),
   locationInfo() {
}

LoggingEvent::LoggingEvent(
        const LogString& logger1, const LevelPtr& level1,
        const LogString& message1, const LocationInfo& locationInfo1) :
   logger(logger1),
   level(level1),
   ndc(0),
   mdcCopy(0),
   properties(0),
   ndcLookupRequired(true),
   mdcCopyLookupRequired(true),
   message(message1),
   timeStamp(apr_time_now()),
   locationInfo(locationInfo1),
   threadName(getCurrentThreadName()) {
}

LoggingEvent::~LoggingEvent()
{
        delete ndc;
        delete mdcCopy;
        delete properties;
}

bool LoggingEvent::getNDC(LogString& dest) const
{
        if(ndcLookupRequired)
        {
                ndcLookupRequired = false;
                LogString val;
                if(NDC::get(val)) {
                     ndc = new LogString(val);
                }
        }
        if (ndc) {
            dest.append(*ndc);
            return true;
        }
        return false;
}

bool LoggingEvent::getMDC(const LogString& key, LogString& dest) const
{
   // Note the mdcCopy is used if it exists. Otherwise we use the MDC
    // that is associated with the thread.
    if (mdcCopy != 0 && !mdcCopy->empty())
        {
                MDC::Map::const_iterator it = mdcCopy->find(key);

                if (it != mdcCopy->end())
                {
                        if (!it->second.empty())
                        {
                                dest.append(it->second);
                                return true;
                        }
                }
    }

    return MDC::get(key, dest);

}

LoggingEvent::KeySet LoggingEvent::getMDCKeySet() const
{
        LoggingEvent::KeySet set;

        if (mdcCopy != 0 && !mdcCopy->empty())
        {
                MDC::Map::const_iterator it;
                for (it = mdcCopy->begin(); it != mdcCopy->end(); it++)
                {
                        set.push_back(it->first);

                }
        }
        else
        {
                ThreadSpecificData* data = ThreadSpecificData::getCurrentData();
                if (data != 0) {
                    MDC::Map& m = data->getMap();

                    for(MDC::Map::const_iterator it = m.begin(); it != m.end(); it++) {
                        set.push_back(it->first);
                    }
                }
        }

        return set;
}

void LoggingEvent::getMDCCopy() const
{
        if(mdcCopyLookupRequired)
        {
                mdcCopyLookupRequired = false;
                // the clone call is required for asynchronous logging.
                ThreadSpecificData* data = ThreadSpecificData::getCurrentData();
                if (data != 0) {
                    mdcCopy = new MDC::Map(data->getMap());
                } else {
                    mdcCopy = new MDC::Map();
                }
       }
}

bool LoggingEvent::getProperty(const LogString& key, LogString& dest) const
{
        if (properties == 0)
        {
                return false;
        }

        std::map<LogString, LogString>::const_iterator  it = properties->find(key);

        if (it != properties->end())
        {
                dest.append(it->second);
                return true;
        }

        return false;
}

LoggingEvent::KeySet LoggingEvent::getPropertyKeySet() const
{
        LoggingEvent::KeySet set;

        if (properties != 0)
        {
                std::map<LogString, LogString>::const_iterator it;
                for (it = properties->begin(); it != properties->end(); it++)
                {
                        set.push_back(it->first);
                }
        }

        return set;
}


const LogString LoggingEvent::getCurrentThreadName() {
#if APR_HAS_THREADS
#if defined(_WIN32)
   char result[20];
   DWORD threadId = GetCurrentThreadId();
   apr_snprintf(result, sizeof(result), LOG4CXX_WIN32_THREAD_FMTSPEC, threadId);
#else
   // apr_os_thread_t encoded in HEX takes needs as many characters
   // as two times the size of the type, plus an additional null byte.
   char result[sizeof(apr_os_thread_t) * 3 + 10];
   apr_os_thread_t threadId = apr_os_thread_current();
   apr_snprintf(result, sizeof(result), LOG4CXX_APR_THREAD_FMTSPEC, (void*) &threadId);
#endif
   LOG4CXX_DECODE_CHAR(str, (const char*) result);
   return str;
#else
   return LOG4CXX_STR("0x00000000");
#endif
}


void LoggingEvent::setProperty(const LogString& key, const LogString& value)
{
        if (properties == 0)
        {
                properties = new std::map<LogString, LogString>;
        }

        (*properties)[key] = value;
}



void LoggingEvent::writeProlog(ObjectOutputStream& os, Pool& p)  {
     char classDesc[] = {
        0x72, 0x00, 0x21, 
        0x6F, 0x72, 0x67, 0x2E, 0x61, 0x70, 0x61, 0x63, 
        0x68, 0x65, 0x2E, 0x6C, 0x6F, 0x67, 0x34, 0x6A, 
        0x2E, 0x73, 0x70, 0x69, 0x2E, 0x4C, 0x6F, 0x67, 
        0x67, 0x69, 0x6E, 0x67, 0x45, 0x76, 0x65, 0x6E, 
        0x74, 0xF3, 0xF2, 0xB9, 0x23, 0x74, 0x0B, 0xB5, 
        0x3F, 0x03, 0x00, 0x0A, 0x5A, 0x00, 0x15, 0x6D, 
        0x64, 0x63, 0x43, 0x6F, 0x70, 0x79, 0x4C, 0x6F, 
        0x6F, 0x6B, 0x75, 0x70, 0x52, 0x65, 0x71, 0x75, 
        0x69, 0x72, 0x65, 0x64, 0x5A, 0x00, 0x11, 0x6E, 
        0x64, 0x63, 0x4C, 0x6F, 0x6F, 0x6B, 0x75, 0x70, 
        0x52, 0x65, 0x71, 0x75, 0x69, 0x72, 0x65, 0x64, 
        0x4A, 0x00, 0x09, 0x74, 0x69, 0x6D, 0x65, 0x53, 
        0x74, 0x61, 0x6D, 0x70, 0x4C, 0x00, 0x0C, 0x63, 
        0x61, 0x74, 0x65, 0x67, 0x6F, 0x72, 0x79, 0x4E, 
        0x61, 0x6D, 0x65, 0x74, 0x00, 0x12, 0x4C, 0x6A, 
        0x61, 0x76, 0x61, 0x2F, 0x6C, 0x61, 0x6E, 0x67, 
        0x2F, 0x53, 0x74, 0x72, 0x69, 0x6E, 0x67, 0x3B, 
        0x4C, 0x00, 0x0C, 0x6C, 0x6F, 0x63, 0x61, 0x74, 
        0x69, 0x6F, 0x6E, 0x49, 0x6E, 0x66, 0x6F, 0x74, 
        0x00, 0x23, 0x4C, 0x6F, 0x72, 0x67, 0x2F, 0x61, 
        0x70, 0x61, 0x63, 0x68, 0x65, 0x2F, 0x6C, 0x6F, 
        0x67, 0x34, 0x6A, 0x2F, 0x73, 0x70, 0x69, 0x2F, 
        0x4C, 0x6F, 0x63, 0x61, 0x74, 0x69, 0x6F, 0x6E, 
        0x49, 0x6E, 0x66, 0x6F, 0x3B, 0x4C, 0x00, 0x07, 
        0x6D, 0x64, 0x63, 0x43, 0x6F, 0x70, 0x79, 0x74, 
        0x00, 0x15, 0x4C, 0x6A, 0x61, 0x76, 0x61, 0x2F, 
        0x75, 0x74, 0x69, 0x6C, 0x2F, 0x48, 0x61, 0x73, 
        0x68, 0x74, 0x61, 0x62, 0x6C, 0x65, 0x3B, 0x4C, 
        0x00, 0x03, 0x6E, 0x64, 0x63, 
        0x74, 0x00, 0x12, 0x4C, 0x6A, 
        0x61, 0x76, 0x61, 0x2F, 0x6C, 0x61, 0x6E, 0x67, 
        0x2F, 0x53, 0x74, 0x72, 0x69, 0x6E, 0x67, 0x3B,
        0x4C, 0x00, 0x0F, 0x72, 0x65, 0x6E, 
        0x64, 0x65, 0x72, 0x65, 0x64, 0x4D, 0x65, 0x73, 
        0x73, 0x61, 0x67, 0x65, 
        0x74, 0x00, 0x12, 0x4C, 0x6A, 
        0x61, 0x76, 0x61, 0x2F, 0x6C, 0x61, 0x6E, 0x67, 
        0x2F, 0x53, 0x74, 0x72, 0x69, 0x6E, 0x67, 0x3B,
        0x4C, 0x00, 0x0A, 0x74, 0x68, 0x72, 0x65, 
        0x61, 0x64, 0x4E, 0x61, 0x6D, 0x65, 
        0x74, 0x00, 0x12, 0x4C, 0x6A, 
        0x61, 0x76, 0x61, 0x2F, 0x6C, 0x61, 0x6E, 0x67, 
        0x2F, 0x53, 0x74, 0x72, 0x69, 0x6E, 0x67, 0x3B,
        0x4C, 0x00, 0x0D, 0x74, 0x68, 
        0x72, 0x6F, 0x77, 0x61, 0x62, 0x6C, 0x65, 0x49, 
        0x6E, 0x66, 0x6F, 0x74, 0x00, 0x2B, 0x4C, 0x6F, 
        0x72, 0x67, 0x2F, 0x61, 0x70, 0x61, 0x63, 0x68, 
        0x65, 0x2F, 0x6C, 0x6F, 0x67, 0x34, 0x6A, 0x2F, 
        0x73, 0x70, 0x69, 0x2F, 0x54, 0x68, 0x72, 0x6F, 
        0x77, 0x61, 0x62, 0x6C, 0x65, 0x49, 0x6E, 0x66, 
        0x6F, 0x72, 0x6D, 0x61, 0x74, 0x69, 0x6F, 0x6E, 
        0x3B, 0x78, 0x70 }; 

     os.writeProlog("org.apache.log4j.spi.LoggingEvent", 
        8, classDesc, sizeof(classDesc), p);
}

void LoggingEvent::write(helpers::ObjectOutputStream& os, Pool& p) const {
      writeProlog(os, p);
      // mdc and ndc lookup required should always be false
      char lookupsRequired[] = { 0, 0 };
      os.writeBytes(lookupsRequired, sizeof(lookupsRequired), p);
      os.writeLong(timeStamp/1000, p);
      os.writeObject(logger, p);
      locationInfo.write(os, p);
      if (mdcCopy == 0 || mdcCopy->size() == 0) {
          os.writeNull(p);
      } else {
          os.writeObject(*mdcCopy, p);
      }
      if (ndc == 0) {
          os.writeNull(p);
      } else {
          os.writeObject(*ndc, p);
      }
      os.writeObject(message, p);
      os.writeObject(threadName, p);
      //  throwable
      os.writeNull(p);
      os.writeByte(ObjectOutputStream::TC_BLOCKDATA, p);
      os.writeByte(0x04, p);
      os.writeInt(level->toInt(), p);
      os.writeNull(p);
      os.writeByte(ObjectOutputStream::TC_ENDBLOCKDATA, p);
}

