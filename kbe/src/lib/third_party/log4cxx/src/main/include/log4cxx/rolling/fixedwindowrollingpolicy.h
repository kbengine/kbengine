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

#if !defined(_LOG4CXX_ROLLING_FIXED_WINDOW_ROLLING_POLICY_H)
#define _LOG4CXX_ROLLING_FIXED_WINDOW_ROLLING_POLICY_H

#include <log4cxx/rolling/rollingpolicybase.h>




namespace log4cxx {

    namespace helpers {
      class Pool;
    }

    namespace rolling {


/**
 * When rolling over, <code>FixedWindowRollingPolicy</code> renames files
 * according to a fixed window algorithm as described below.
 *
 * <p>The <b>ActiveFileName</b> property, which is required, represents the name
 * of the file where current logging output will be written.
 * The <b>FileNamePattern</b>  option represents the file name pattern for the
 * archived (rolled over) log files. If present, the <b>FileNamePattern</b>
 * option must include an integer token, that is the string "%i" somwhere
 * within the pattern.
 *
 * <p>Let <em>max</em> and <em>min</em> represent the values of respectively
 * the <b>MaxIndex</b> and <b>MinIndex</b> options. Let "foo.log" be the value
 * of the <b>ActiveFile</b> option and "foo.%i.log" the value of
 * <b>FileNamePattern</b>. Then, when rolling over, the file
 * <code>foo.<em>max</em>.log</code> will be deleted, the file
 * <code>foo.<em>max-1</em>.log</code> will be renamed as
 * <code>foo.<em>max</em>.log</code>, the file <code>foo.<em>max-2</em>.log</code>
 * renamed as <code>foo.<em>max-1</em>.log</code>, and so on,
 * the file <code>foo.<em>min+1</em>.log</code> renamed as
 * <code>foo.<em>min+2</em>.log</code>. Lastly, the active file <code>foo.log</code>
 * will be renamed as <code>foo.<em>min</em>.log</code> and a new active file name
 * <code>foo.log</code> will be created.
 *
 * <p>Given that this rollover algorithm requires as many file renaming
 * operations as the window size, large window sizes are discouraged. The
 * current implementation will automatically reduce the window size to 12 when
 * larger values are specified by the user.
 *
 *
 * 
 * 
 * */
        class LOG4CXX_EXPORT FixedWindowRollingPolicy : public RollingPolicyBase {
          DECLARE_LOG4CXX_OBJECT(FixedWindowRollingPolicy)
          BEGIN_LOG4CXX_CAST_MAP()
                  LOG4CXX_CAST_ENTRY(FixedWindowRollingPolicy)
                  LOG4CXX_CAST_ENTRY_CHAIN(RollingPolicyBase)
          END_LOG4CXX_CAST_MAP()

          int minIndex;
          int maxIndex;
          bool explicitActiveFile;

          /**
           * It's almost always a bad idea to have a large window size, say over 12.
           */
          enum { MAX_WINDOW_SIZE = 12 };

          bool purge(int purgeStart, int maxIndex, log4cxx::helpers::Pool& p) const;

        public:

          FixedWindowRollingPolicy();

          void activateOptions(log4cxx::helpers::Pool& p);
          void setOption(const LogString& option,
             const LogString& value);

          void rollover();

          int getMaxIndex() const;

          int getMinIndex() const;

          void setMaxIndex(int newVal);
          void setMinIndex(int newVal);


/**
* Initialize the policy and return any initial actions for rolling file appender.
*
* @param file current value of RollingFileAppender::getFile().
* @param append current value of RollingFileAppender::getAppend().
* @param p pool used for any required memory allocations.
* @return Description of the initialization, may be null to indicate
* no initialization needed.
* @throws SecurityException if denied access to log files.
*/
virtual RolloverDescriptionPtr initialize(
const LogString& file, const bool append, log4cxx::helpers::Pool& p);

/**
* Prepare for a rollover.  This method is called prior to
* closing the active log file, performs any necessary
* preliminary actions and describes actions needed
* after close of current log file.
*
* @param activeFile file name for current active log file.
* @param p pool used for any required memory allocations.
* @return Description of pending rollover, may be null to indicate no rollover
* at this time.
* @throws SecurityException if denied access to log files.
*/
virtual RolloverDescriptionPtr rollover(const LogString& activeFile, log4cxx::helpers::Pool& p);

protected:
             log4cxx::pattern::PatternMap getFormatSpecifiers() const;

        };

        LOG4CXX_PTR_DEF(FixedWindowRollingPolicy);

     }
}

#endif
