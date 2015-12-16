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

#if !defined(_LOG4CXX_ROLLING_ROLLING_POLICY_H)
#define _LOG4CXX_ROLLING_ROLLING_POLICY_H

#include <log4cxx/portability.h>
#include <log4cxx/spi/optionhandler.h>
#include <log4cxx/rolling/rolloverdescription.h>
#include <log4cxx/file.h>

namespace log4cxx {
    namespace rolling {


        /**
         * A <code>RollingPolicy</code> is responsible for performing the
         * rolling over of the active log file. The <code>RollingPolicy</code>
         * is also responsible for providing the <em>active log file</em>,
         * that is the live file where logging output will be directed.
         *
         * 
         * 
         *
        */
        class LOG4CXX_EXPORT RollingPolicy :
        public virtual spi::OptionHandler {
            DECLARE_ABSTRACT_LOG4CXX_OBJECT(RollingPolicy)

        public:
        virtual ~RollingPolicy() {}
        /**
       * Initialize the policy and return any initial actions for rolling file appender.
       *
       * @param file current value of RollingFileAppender.getFile().
       * @param append current value of RollingFileAppender.getAppend().
       * @param p pool for memory allocations during call.
       * @return Description of the initialization, may be null to indicate
       * no initialization needed.
       * @throws SecurityException if denied access to log files.
       */
       virtual RolloverDescriptionPtr initialize(
        const LogString& file,
        const bool append,
        log4cxx::helpers::Pool& p) = 0;

      /**
       * Prepare for a rollover.  This method is called prior to
       * closing the active log file, performs any necessary
       * preliminary actions and describes actions needed
       * after close of current log file.
       *
       * @param activeFile file name for current active log file.
       * @param p pool for memory allocations during call.
       * @return Description of pending rollover, may be null to indicate no rollover
       * at this time.
       * @throws SecurityException if denied access to log files.
       */
      virtual RolloverDescriptionPtr rollover(const LogString& activeFile,
          log4cxx::helpers::Pool& p) = 0;
        };

      LOG4CXX_PTR_DEF(RollingPolicy);

    }
}
#endif

