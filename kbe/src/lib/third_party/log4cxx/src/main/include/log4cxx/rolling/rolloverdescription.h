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

#if !defined(_LOG4CXX_ROLLING_ROLLOVER_DESCRIPTION_H)
#define _LOG4CXX_ROLLING_ROLLOVER_DESCRIPTION_H

#include <log4cxx/portability.h>
#include <log4cxx/rolling/action.h>

namespace log4cxx {
    namespace rolling {


        class RolloverDescription : public log4cxx::helpers::ObjectImpl {
          DECLARE_LOG4CXX_OBJECT(RolloverDescription)
          BEGIN_LOG4CXX_CAST_MAP()
                  LOG4CXX_CAST_ENTRY(RolloverDescription)
          END_LOG4CXX_CAST_MAP()
          /**
           * Active log file name after rollover.
           */
          LogString activeFileName;

          /**
           * Should active file be opened for appending.
           */
          bool append;

          /**
           * Action to be completed after close of current active log file
           * before returning control to caller.
           */
          ActionPtr synchronous;

          /**
           * Action to be completed after close of current active log file
           * and before next rollover attempt, may be executed asynchronously.
           */
          ActionPtr asynchronous;

          public:
          RolloverDescription();
          /**
           * Create new instance.
           * @param activeFileName active log file name after rollover, may not be null.
           * @param append true if active log file after rollover should be opened for appending.
           * @param synchronous action to be completed after close of current active log file, may be null.
           * @param asynchronous action to be completed after close of current active log file and
           * before next rollover attempt.
           */
          RolloverDescription(
            const LogString& activeFileName,
            const bool append,
            const ActionPtr& synchronous,
            const ActionPtr& asynchronous);

          /**
           * Active log file name after rollover.
           * @return active log file name after rollover.
           */
          LogString getActiveFileName() const;

          bool getAppend() const;

          /**
           * Action to be completed after close of current active log file
           * before returning control to caller.
           *
           * @return action, may be null.
           */
          ActionPtr getSynchronous() const;

          /**
           * Action to be completed after close of current active log file
           * and before next rollover attempt, may be executed asynchronously.
           *
           * @return action, may be null.
           */
          ActionPtr getAsynchronous() const;
        };

        LOG4CXX_PTR_DEF(RolloverDescription);

    }
}
#endif

