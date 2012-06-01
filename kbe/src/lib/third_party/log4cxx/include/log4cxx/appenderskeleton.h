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

#ifndef _LOG4CXX_APPENDER_SKELETON_H
#define _LOG4CXX_APPENDER_SKELETON_H

#if defined(_MSC_VER)
#pragma warning ( push )
#pragma warning ( disable: 4231 4251 4275 4786 )
#endif


#include <log4cxx/appender.h>
#include <log4cxx/layout.h>
#include <log4cxx/spi/errorhandler.h>
#include <log4cxx/spi/filter.h>
#include <log4cxx/helpers/objectimpl.h>
#include <log4cxx/helpers/mutex.h>
#include <log4cxx/helpers/pool.h>
#include <log4cxx/level.h>


namespace log4cxx
{
        /**
        *  Implementation base class for all appenders.
        *
        *  This class provides the code for common functionality, such as
        *  support for threshold filtering and support for general filters.
        * */
        class LOG4CXX_EXPORT AppenderSkeleton :
                public virtual Appender,
                public virtual helpers::ObjectImpl
        {
        protected:
                /** The layout variable does not need to be set if the appender
                implementation has its own layout. */
                LayoutPtr layout;

                /** Appenders are named. */
                LogString name;

                /**
                There is no level threshold filtering by default.  */
                LevelPtr threshold;

                /**
                It is assumed and enforced that errorHandler is never null.
                */
                spi::ErrorHandlerPtr errorHandler;

                /** The first filter in the filter chain. Set to <code>null</code>
                initially. */
                spi::FilterPtr headFilter;

                /** The last filter in the filter chain. */
                spi::FilterPtr tailFilter;

                /**
                Is this appender closed?
                */
                bool closed;

                log4cxx::helpers::Pool pool;
                log4cxx::helpers::Mutex mutex;

        public:
                DECLARE_ABSTRACT_LOG4CXX_OBJECT(AppenderSkeleton)
                BEGIN_LOG4CXX_CAST_MAP()
                        LOG4CXX_CAST_ENTRY(Appender)
                        LOG4CXX_CAST_ENTRY(spi::OptionHandler)
                END_LOG4CXX_CAST_MAP()

                AppenderSkeleton();
                AppenderSkeleton(const LayoutPtr& layout);

                void addRef() const;
                void releaseRef() const;

                /**
                Finalize this appender by calling the derived class'
                <code>close</code> method.
                */
                void finalize();

                /**
                Derived appenders should override this method if option structure
                requires it.
                */
                virtual void activateOptions(log4cxx::helpers::Pool& /* pool */) {}
                virtual void setOption(const LogString& option, const LogString& value);

                /**
                Add a filter to end of the filter list.
                */
                void addFilter(const spi::FilterPtr& newFilter) ;

                /**
                Subclasses of <code>AppenderSkeleton</code> should implement this
                method to perform actual logging. See also AppenderSkeleton::doAppend
                method.
                */
        protected:
                virtual void append(const spi::LoggingEventPtr& event, log4cxx::helpers::Pool& p) = 0;

                /**
                Clear the filters chain.
                */
        public:
                void clearFilters();

                /**
                Return the currently set spi::ErrorHandler for this
                Appender.
                */
                const spi::ErrorHandlerPtr& getErrorHandler() const { return errorHandler; }

                /**
                Returns the head Filter.
                */
                spi::FilterPtr getFilter() const { return headFilter; }

                /**
                Return the first filter in the filter chain for this
                Appender. The return value may be <code>0</code> if no is
                filter is set.
                */
                const spi::FilterPtr& getFirstFilter() const { return headFilter; }

                /**
                Returns the layout of this appender. The value may be 0.
                */
                LayoutPtr getLayout() const { return layout; }


                /**
                Returns the name of this Appender.
                */
                LogString getName() const { return name; }

                /**
                Returns this appenders threshold level. See the #setThreshold
                method for the meaning of this option.
                */
                const LevelPtr& getThreshold() { return threshold; }

                /**
                Check whether the message level is below the appender's
                threshold. If there is no threshold set, then the return value is
                always <code>true</code>.
                */
                bool isAsSevereAsThreshold(const LevelPtr& level) const;


                /**
                * This method performs threshold checks and invokes filters before
                * delegating actual logging to the subclasses specific
                * AppenderSkeleton#append method.
                * */
                void doAppend(const spi::LoggingEventPtr& event, log4cxx::helpers::Pool& pool);

                /**
                Set the {@link spi::ErrorHandler ErrorHandler} for this Appender.
                */
                void setErrorHandler(const spi::ErrorHandlerPtr& eh);

                /**
                Set the layout for this appender. Note that some appenders have
                their own (fixed) layouts or do not use one. For example, the
                {@link net::SocketAppender SocketAppender} ignores the layout set
                here.
                */
                void setLayout(const LayoutPtr& layout1) { this->layout = layout1; }

                /**
                Set the name of this Appender.
                */
                void setName(const LogString& name1) { this->name.assign(name1); }


                /**
                Set the threshold level. All log events with lower level
                than the threshold level are ignored by the appender.

                <p>In configuration files this option is specified by setting the
                value of the <b>Threshold</b> option to a level
                string, such as "DEBUG", "INFO" and so on.
                */
                void setThreshold(const LevelPtr& threshold);

        }; // class AppenderSkeleton
}  // namespace log4cxx

#if defined(_MSC_VER)
#pragma warning ( pop )
#endif


#endif //_LOG4CXX_APPENDER_SKELETON_H
