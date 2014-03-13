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
#include <log4cxx/filter/levelrangefilter.h>
#include <log4cxx/spi/loggingevent.h>
#include <log4cxx/helpers/stringhelper.h>
#include <log4cxx/helpers/optionconverter.h>
#include <log4cxx/level.h>

using namespace log4cxx;
using namespace log4cxx::filter;
using namespace log4cxx::spi;
using namespace log4cxx::helpers;

IMPLEMENT_LOG4CXX_OBJECT(LevelRangeFilter)


LevelRangeFilter::LevelRangeFilter()
: acceptOnMatch(true), levelMin(Level::getAll()), levelMax(Level::getOff())
{
}

void LevelRangeFilter::setOption(const LogString& option,
   const LogString& value)
{

   if (StringHelper::equalsIgnoreCase(option,
                 LOG4CXX_STR("LEVELMIN"), LOG4CXX_STR("levelmin")))
   {
      levelMin = OptionConverter::toLevel(value, levelMin);
   }
        else if (StringHelper::equalsIgnoreCase(option,
                LOG4CXX_STR("LEVELMAX"), LOG4CXX_STR("levelmax")))
   {
      levelMax = OptionConverter::toLevel(value, levelMax);
   }
   else if (StringHelper::equalsIgnoreCase(option,
                LOG4CXX_STR("ACCEPTONMATCH"), LOG4CXX_STR("acceptonmatch")))
   {
      acceptOnMatch = OptionConverter::toBoolean(value, acceptOnMatch);
   }
}

Filter::FilterDecision LevelRangeFilter::decide(
   const spi::LoggingEventPtr& event) const
{
   if (levelMin != 0 && !event->getLevel()->isGreaterOrEqual(levelMin))
   {
      // level of event is less than minimum
      return Filter::DENY;
   }

   if (levelMax != 0 && event->getLevel()->toInt() > levelMax->toInt())
   {
      // level of event is greater than maximum
      // Alas, there is no Level.isGreater method. and using
      // a combo of isGreaterOrEqual && !Equal seems worse than
      // checking the int values of the level objects..
      return Filter::DENY;
   }

   if (acceptOnMatch)
   {
      // this filter set up to bypass later filters and always return
      // accept if level in range
      return Filter::ACCEPT;
   }
   else
   {
      // event is ok for this filter; allow later filters to have a look..
      return Filter::NEUTRAL;
   }
}

