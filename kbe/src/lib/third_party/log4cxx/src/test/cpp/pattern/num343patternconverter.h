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

#include <log4cxx/pattern/loggingeventpatternconverter.h>
#include <vector>

namespace log4cxx
{
   namespace pattern {
     class Num343PatternConverter : public LoggingEventPatternConverter
     {
     public:
        DECLARE_LOG4CXX_OBJECT(Num343PatternConverter)

       Num343PatternConverter();
       static PatternConverterPtr newInstance(
          const std::vector<LogString>& options);

     protected:
          void format(
              const log4cxx::spi::LoggingEventPtr& event,
              LogString& toAppendTo,
              log4cxx::helpers::Pool& pool) const;
     };
   }
}
