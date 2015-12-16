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
#include <log4cxx/ttcclayout.h>
#include <log4cxx/spi/loggingevent.h>
#include <log4cxx/level.h>
#include <log4cxx/helpers/stringhelper.h>
#include <log4cxx/ndc.h>

using namespace log4cxx;
using namespace log4cxx::spi;
using namespace log4cxx::helpers;

IMPLEMENT_LOG4CXX_OBJECT(TTCCLayout)

TTCCLayout::TTCCLayout()
: DateLayout(LOG4CXX_STR("RELATIVE")), threadPrinting(true), categoryPrefixing(true),
contextPrinting(true), filePrinting(false)
{
        Pool pool;
        activateOptions(pool);
}

TTCCLayout::TTCCLayout(const LogString& dateFormatType)
: DateLayout(dateFormatType), threadPrinting(true), categoryPrefixing(true),
contextPrinting(true), filePrinting(false)
{
        Pool pool;
        activateOptions(pool);
}

void TTCCLayout::format(LogString& output,
      const spi::LoggingEventPtr& event,
      Pool& p) const
{
        formatDate(output, event, p);

        if(threadPrinting)
        {
                output.append(1, (logchar) 0x5B /* '[' */);
                output.append(event->getThreadName());
                output.append(1, (logchar) 0x5D /* ']' */);
                output.append(1, (logchar) 0x20 /* ' ' */);
        }

        output.append(event->getLevel()->toString());
        output.append(1, (logchar) 0x20 /* ' ' */);
        if(categoryPrefixing)
        {
                output.append(event->getLoggerName());
                output.append(1, (logchar) 0x20 /* ' ' */);
        }

        if(contextPrinting && event->getNDC(output)) {
                output.append(1, (logchar) 0x20 /* ' ' */);
        }

        output.append(1, (logchar) 0x2D /* '-' */);
        output.append(1, (logchar) 0x20 /* ' ' */);
        output.append(event->getRenderedMessage());
        output.append(LOG4CXX_EOL);
}
