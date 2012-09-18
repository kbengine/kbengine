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

#ifndef _LOG4CXX_SPI_LOG_REPOSITORY_H
#define _LOG4CXX_SPI_LOG_REPOSITORY_H

#if defined(_MSC_VER)
#pragma warning ( push )
#pragma warning ( disable: 4231 4251 4275 4786 )
#endif


#include <log4cxx/appender.h>
#include <log4cxx/spi/loggerfactory.h>
#include <log4cxx/level.h>
#include <log4cxx/spi/hierarchyeventlistener.h>

namespace log4cxx
{
    namespace spi
        {
            
            /**
            A <code>LoggerRepository</code> is used to create and retrieve
            <code>Loggers</code>. The relation between loggers in a repository
            depends on the repository but typically loggers are arranged in a
            named hierarchy.

            <p>In addition to the creational methods, a
            <code>LoggerRepository</code> can be queried for existing loggers,
            can act as a point of registry for events related to loggers.
            */
            class LOG4CXX_EXPORT LoggerRepository : public virtual helpers::Object
            {
            public:
                DECLARE_ABSTRACT_LOG4CXX_OBJECT(LoggerRepository)
                    virtual ~LoggerRepository() {}

                /**
                Add a {@link spi::HierarchyEventListener HierarchyEventListener}
                            event to the repository.
                */
                virtual void addHierarchyEventListener(const HierarchyEventListenerPtr&
                                    listener) = 0;
                /**
                Is the repository disabled for a given level? The answer depends
                on the repository threshold and the <code>level</code>
                parameter. See also #setThreshold method.  */
                virtual bool isDisabled(int level) const = 0;

                /**
                Set the repository-wide threshold. All logging requests below the
                threshold are immediately dropped. By default, the threshold is
                set to <code>Level::getAll()</code> which has the lowest possible rank.  */
                virtual void setThreshold(const LevelPtr& level) = 0;

                /**
                Another form of {@link #setThreshold(const LevelPtr&)
                            setThreshold} accepting a string
                parameter instead of a <code>Level</code>. */
                virtual void setThreshold(const LogString& val) = 0;

                virtual void emitNoAppenderWarning(const LoggerPtr& logger) = 0;

                /**
                Get the repository-wide threshold. See {@link
                #setThreshold(const LevelPtr&) setThreshold}
                            for an explanation. */
                virtual const LevelPtr& getThreshold() const = 0;

                virtual LoggerPtr getLogger(const LogString& name) = 0;

                virtual LoggerPtr getLogger(const LogString& name,
                     const spi::LoggerFactoryPtr& factory) = 0;

                virtual LoggerPtr getRootLogger() const = 0;

                virtual LoggerPtr exists(const LogString& name) = 0;

                virtual void shutdown() = 0;

                virtual LoggerList getCurrentLoggers() const = 0;

                virtual void fireAddAppenderEvent(const LoggerPtr& logger,
                                    const AppenderPtr& appender) = 0;

                virtual void resetConfiguration() = 0;

            virtual bool isConfigured() = 0;
            virtual void setConfigured(bool configured) = 0;

            }; // class LoggerRepository

        }  // namespace spi
} // namespace log4cxx

#if defined(_MSC_VER)
#pragma warning ( pop )
#endif


#endif //_LOG4CXX_SPI_LOG_REPOSITORY_H
