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

 #if !defined(_LOG4CXX_ROLLING_FILTER_BASED_TRIGGERING_POLICY_H)
 #define _LOG4CXX_ROLLING_FILTER_BASED_TRIGGERING_POLICY_H

#include <log4cxx/rolling/triggeringpolicy.h>
#include <log4cxx/spi/filter.h>

 namespace log4cxx {

     class File;

     namespace helpers {
        class Pool;
     }


     namespace rolling {


/**
 * FilterBasedTriggeringPolicy determines if rolling should be triggered
 * by evaluating the current message against a set of filters.  Unless a
 * filter rejects a message, a rolling event will be triggered.
 *
 * 
 * 
 *
 */
class LOG4CXX_EXPORT FilterBasedTriggeringPolicy : public TriggeringPolicy {

  DECLARE_LOG4CXX_OBJECT(FilterBasedTriggeringPolicy)
  BEGIN_LOG4CXX_CAST_MAP()
          LOG4CXX_CAST_ENTRY(FilterBasedTriggeringPolicy)
          LOG4CXX_CAST_ENTRY_CHAIN(TriggeringPolicy)
  END_LOG4CXX_CAST_MAP()

  /**
   * The first filter in the filter chain. Set to <code>null</code> initially.
   */
  log4cxx::spi::FilterPtr headFilter;

  /**
   * The last filter in the filter chain.
   */
  log4cxx::spi::FilterPtr tailFilter;

public:
  /**
   *  Creates a new FilterBasedTriggeringPolicy.
   */
  FilterBasedTriggeringPolicy();
  virtual ~FilterBasedTriggeringPolicy();

/**
 * Determines if a rollover may be appropriate at this time.  If
 * true is returned, RolloverPolicy.rollover will be called but it
 * can determine that a rollover is not warranted.
 *
 * @param appender A reference to the appender.
 * @param event A reference to the currently event.
 * @param filename The filename for the currently active log file.
 * @param fileLength Length of the file in bytes.
 * @return true if a rollover should occur.
 */
virtual bool isTriggeringEvent(
  Appender* appender,
  const log4cxx::spi::LoggingEventPtr& event,
  const LogString& filename,
  size_t fileLength);

  /**
   * Add a filter to end of the filter list.
   * @param newFilter filter to add to end of list.
   */
  void addFilter(const log4cxx::spi::FilterPtr& newFilter);

  /**
   * Clear the filters chain.
   *
   */
  void clearFilters();

  /**
   * Returns the head Filter.
   *
   */
  log4cxx::spi::FilterPtr& getFilter();

  /**
   *  Prepares the instance for use.
   */
  void activateOptions(log4cxx::helpers::Pool&);

  void setOption(const LogString& option, const LogString& value);
  };

  LOG4CXX_PTR_DEF(FilterBasedTriggeringPolicy);

}
 }

#endif
