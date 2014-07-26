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

#if !defined(_LOG4CXX_ROLLING_ROLLING_FILE_APPENDER_H)
#define _LOG4CXX_ROLLING_ROLLING_FILE_APPENDER_H

#include <log4cxx/rolling/rollingfileappenderskeleton.h>


namespace log4cxx {
    namespace rolling {

        
        /**
         * <code>RollingFileAppender</code> extends {@link log4cxx::FileAppender} to backup the log files
         * depending on {@link log4cxx::rolling::RollingPolicy RollingPolicy} and {@link log4cxx::rolling::TriggeringPolicy TriggeringPolicy}.
         * <p>
         * To be of any use, a <code>RollingFileAppender</code> instance must have both
         * a <code>RollingPolicy</code> and a <code>TriggeringPolicy</code> set up.
         * However, if its <code>RollingPolicy</code> also implements the
         * <code>TriggeringPolicy</code> interface, then only the former needs to be
         * set up. For example, {@link log4cxx::rolling::TimeBasedRollingPolicy TimeBasedRollingPolicy} acts both as a
         * <code>RollingPolicy</code> and a <code>TriggeringPolicy</code>.
         *
         * <p><code>RollingFileAppender</code> can be configured programattically or
         * using {@link log4cxx::xml::DOMConfigurator}. Here is a sample
         * configration file:

        <pre>&lt;?xml version="1.0" encoding="UTF-8" ?>
        &lt;!DOCTYPE log4j:configuration>

        &lt;log4j:configuration debug="true">

          &lt;appender name="ROLL" class="org.apache.log4j.rolling.RollingFileAppender">
            <b>&lt;rollingPolicy class="org.apache.log4j.rolling.TimeBasedRollingPolicy">
              &lt;param name="FileNamePattern" value="/wombat/foo.%d{yyyy-MM}.gz"/>
            &lt;/rollingPolicy></b>

            &lt;layout class="org.apache.log4j.PatternLayout">
              &lt;param name="ConversionPattern" value="%c{1} - %m%n"/>
            &lt;/layout>
          &lt;/appender>

          &lt;root">
            &lt;appender-ref ref="ROLL"/>
          &lt;/root>

        &lt;/log4j:configuration>
        </pre>

         *<p>This configuration file specifies a monthly rollover schedule including
         * automatic compression of the archived files. See
         * {@link TimeBasedRollingPolicy} for more details.
         *
         * 
         * 
         * 
         * */
        class LOG4CXX_EXPORT RollingFileAppender : public RollingFileAppenderSkeleton {
          DECLARE_LOG4CXX_OBJECT(RollingFileAppender)
          BEGIN_LOG4CXX_CAST_MAP()
                  LOG4CXX_CAST_ENTRY(RollingFileAppender)
                  LOG4CXX_CAST_ENTRY_CHAIN(RollingFileAppenderSkeleton)
          END_LOG4CXX_CAST_MAP()

        public:
          RollingFileAppender();

          using RollingFileAppenderSkeleton::getRollingPolicy;

          using RollingFileAppenderSkeleton::getTriggeringPolicy;

          /**
           * Sets the rolling policy. In case the 'policy' argument also implements
           * {@link TriggeringPolicy}, then the triggering policy for this appender
           * is automatically set to be the policy argument.
           * @param policy
           */
          using RollingFileAppenderSkeleton::setRollingPolicy;

          using RollingFileAppenderSkeleton::setTriggeringPolicy;

        };

        LOG4CXX_PTR_DEF(RollingFileAppender);

    }
}

#endif

