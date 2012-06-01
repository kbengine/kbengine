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

#ifndef _LOG4CXX_LOGGER_H
#define _LOG4CXX_LOGGER_H

#if defined(_MSC_VER)
#pragma warning ( push )
#pragma warning ( disable: 4231 4251 4275 4786 )
#endif

#include <log4cxx/helpers/appenderattachableimpl.h>
#include <log4cxx/level.h>
#include <log4cxx/helpers/pool.h>
#include <log4cxx/helpers/mutex.h>
#include <log4cxx/spi/location/locationinfo.h>
#include <log4cxx/helpers/resourcebundle.h>
#include <log4cxx/helpers/messagebuffer.h>


namespace log4cxx
{

    namespace helpers {
            class synchronized;
    }
    
    namespace spi {
        class LoggerRepository;
        LOG4CXX_PTR_DEF(LoggerRepository);
        class LoggerFactory;
        LOG4CXX_PTR_DEF(LoggerFactory);
    }

    class Logger;
    /** smart pointer to a Logger class */
    LOG4CXX_PTR_DEF(Logger);
    LOG4CXX_LIST_DEF(LoggerList, LoggerPtr);


    /**
    This is the central class in the log4cxx package. Most logging
    operations, except configuration, are done through this class.
    */
    class LOG4CXX_EXPORT Logger :
                public virtual log4cxx::spi::AppenderAttachable,
                public virtual helpers::ObjectImpl
    {
    public:
        DECLARE_ABSTRACT_LOG4CXX_OBJECT(Logger)
        BEGIN_LOG4CXX_CAST_MAP()
                LOG4CXX_CAST_ENTRY(Logger)
                LOG4CXX_CAST_ENTRY(spi::AppenderAttachable)
        END_LOG4CXX_CAST_MAP()
        
    private:
        /**
         *   Reference to memory pool.
         */
        helpers::Pool* pool;

    protected:
        /**
        The name of this logger.
        */
        LogString name;

        /**
        The assigned level of this logger.  The
        <code>level</code> variable need not be assigned a value in
        which case it is inherited form the hierarchy.  */
        LevelPtr level;

        /**
        The parent of this logger. All loggers have at least one
        ancestor which is the root logger. */
        LoggerPtr parent;

        /** The resourceBundle for localized messages.

        @see setResourceBundle, getResourceBundle
        */
        helpers::ResourceBundlePtr resourceBundle;


        // Loggers need to know what Hierarchy they are in
        log4cxx::spi::LoggerRepository * repository;

        helpers::AppenderAttachableImplPtr aai;

                /** Additivity is set to true by default, that is children inherit
                        the appenders of their ancestors by default. If this variable is
                        set to <code>false</code> then the appenders found in the
                        ancestors of this logger are not used. However, the children
                        of this logger will inherit its appenders, unless the children
                        have their additivity flag set to <code>false</code> too. See
                        the user manual for more details. */
                bool additive;

    protected:
        friend class DefaultLoggerFactory;

        /**
        This constructor created a new <code>logger</code> instance and
        sets its name.

        <p>It is intended to be used by sub-classes only. You should not
        create categories directly.

        @param pool lifetime of pool must be longer than logger.
        @param name The name of the logger.
        */
        Logger(log4cxx::helpers::Pool& pool, const LogString& name);

    public:
        ~Logger();


        void addRef() const;
        void releaseRef() const;

        /**
        Add <code>newAppender</code> to the list of appenders of this
        Logger instance.

        <p>If <code>newAppender</code> is already in the list of
        appenders, then it won't be added again.
        */
        virtual void addAppender(const AppenderPtr& newAppender);


         /**
        Call the appenders in the hierrachy starting at
        <code>this</code>.  If no appenders could be found, emit a
        warning.

        <p>This method calls all the appenders inherited from the
        hierarchy circumventing any evaluation of whether to log or not
        to log the particular log request.

        @param event the event to log.  
        @param p memory pool for any allocations needed to process request.
        */
        void callAppenders(const log4cxx::spi::LoggingEventPtr& event, log4cxx::helpers::Pool& p) const;

        /**
        Close all attached appenders implementing the AppenderAttachable
        interface.
        */
        void closeNestedAppenders();

        /**
        Log a message string with the DEBUG level.

        <p>This method first checks if this logger is <code>DEBUG</code>
        enabled by comparing the level of this logger with the
        DEBUG level. If this logger is
        <code>DEBUG</code> enabled, it proceeds to call all the
        registered appenders in this logger and also higher in the
        hierarchy depending on the value of the additivity flag.

        @param msg the message string to log.
        @param location location of source of logging request.
        */
        void debug(const std::string& msg, const log4cxx::spi::LocationInfo& location) const;
        /**
        Log a message string with the DEBUG level.

        <p>This method first checks if this logger is <code>DEBUG</code>
        enabled by comparing the level of this logger with the
        DEBUG level. If this logger is
        <code>DEBUG</code> enabled, it proceeds to call all the
        registered appenders in this logger and also higher in the
        hierarchy depending on the value of the additivity flag.

        @param msg the message string to log.
        */
        void debug(const std::string& msg) const;
#if LOG4CXX_WCHAR_T_API
        /**
        Log a message string with the DEBUG level.

        <p>This method first checks if this logger is <code>DEBUG</code>
        enabled by comparing the level of this logger with the
        DEBUG level. If this logger is
        <code>DEBUG</code> enabled, it proceeds to call all the
        registered appenders in this logger and also higher in the
        hierarchy depending on the value of the additivity flag.

        @param msg the message string to log.
        @param location location of source of logging request.
        */
        void debug(const std::wstring& msg, const log4cxx::spi::LocationInfo& location) const;
        /**
        Log a message string with the DEBUG level.

        <p>This method first checks if this logger is <code>DEBUG</code>
        enabled by comparing the level of this logger with the
        DEBUG level. If this logger is
        <code>DEBUG</code> enabled, it proceeds to call all the
        registered appenders in this logger and also higher in the
        hierarchy depending on the value of the additivity flag.

        @param msg the message string to log.
        */
        void debug(const std::wstring& msg) const;
#endif
#if LOG4CXX_UNICHAR_API
        /**
        Log a message string with the DEBUG level.

        <p>This method first checks if this logger is <code>DEBUG</code>
        enabled by comparing the level of this logger with the
        DEBUG level. If this logger is
        <code>DEBUG</code> enabled, it proceeds to call all the
        registered appenders in this logger and also higher in the
        hierarchy depending on the value of the additivity flag.

        @param msg the message string to log.
        @param location location of source of logging request.
        */
        void debug(const std::basic_string<UniChar>& msg, const log4cxx::spi::LocationInfo& location) const;
        /**
        Log a message string with the DEBUG level.

        <p>This method first checks if this logger is <code>DEBUG</code>
        enabled by comparing the level of this logger with the
        DEBUG level. If this logger is
        <code>DEBUG</code> enabled, it proceeds to call all the
        registered appenders in this logger and also higher in the
        hierarchy depending on the value of the additivity flag.

        @param msg the message string to log.
        */
        void debug(const std::basic_string<UniChar>& msg) const;
#endif
#if LOG4CXX_CFSTRING_API
        /**
        Log a message string with the DEBUG level.

        <p>This method first checks if this logger is <code>DEBUG</code>
        enabled by comparing the level of this logger with the
        DEBUG level. If this logger is
        <code>DEBUG</code> enabled, it proceeds to call all the
        registered appenders in this logger and also higher in the
        hierarchy depending on the value of the additivity flag.

        @param msg the message string to log.
        @param location location of source of logging request.
        */
        void debug(const CFStringRef& msg, const log4cxx::spi::LocationInfo& location) const;
        /**
        Log a message string with the DEBUG level.

        <p>This method first checks if this logger is <code>DEBUG</code>
        enabled by comparing the level of this logger with the
        DEBUG level. If this logger is
        <code>DEBUG</code> enabled, it proceeds to call all the
        registered appenders in this logger and also higher in the
        hierarchy depending on the value of the additivity flag.

        @param msg the message string to log.
        */
        void debug(const CFStringRef& msg) const;
#endif

        /**
        Log a message string with the ERROR level.

        <p>This method first checks if this logger is <code>ERROR</code>
        enabled by comparing the level of this logger with the
        ERROR level. If this logger is
        <code>ERROR</code> enabled, it proceeds to call all the
        registered appenders in this logger and also higher in the
        hierarchy depending on the value of the additivity flag.

        @param msg the message string to log.
        @param location location of source of logging request.
        */
        void error(const std::string& msg, const log4cxx::spi::LocationInfo& location) const;
        /**
        Log a message string with the ERROR level.

        <p>This method first checks if this logger is <code>ERROR</code>
        enabled by comparing the level of this logger with the
        ERROR level. If this logger is
        <code>ERROR</code> enabled, it proceeds to call all the
        registered appenders in this logger and also higher in the
        hierarchy depending on the value of the additivity flag.

        @param msg the message string to log.
        */
        void error(const std::string& msg) const;
#if LOG4CXX_WCHAR_T_API
        /**
        Log a message string with the ERROR level.

        <p>This method first checks if this logger is <code>ERROR</code>
        enabled by comparing the level of this logger with the
        ERROR level. If this logger is
        <code>ERROR</code> enabled, it proceeds to call all the
        registered appenders in this logger and also higher in the
        hierarchy depending on the value of the additivity flag.

        @param msg the message string to log.
        */
        void error(const std::wstring& msg) const;
        /**
        Log a message string with the ERROR level.

        <p>This method first checks if this logger is <code>ERROR</code>
        enabled by comparing the level of this logger with the
        ERROR level. If this logger is
        <code>ERROR</code> enabled, it proceeds to call all the
        registered appenders in this logger and also higher in the
        hierarchy depending on the value of the additivity flag.

        @param msg the message string to log.
        @param location location of source of logging request.
        */
        void error(const std::wstring& msg, const log4cxx::spi::LocationInfo& location) const;
#endif
#if LOG4CXX_UNICHAR_API
        /**
        Log a message string with the ERROR level.

        <p>This method first checks if this logger is <code>ERROR</code>
        enabled by comparing the level of this logger with the
        ERROR level. If this logger is
        <code>ERROR</code> enabled, it proceeds to call all the
        registered appenders in this logger and also higher in the
        hierarchy depending on the value of the additivity flag.

        @param msg the message string to log.
        @param location location of source of logging request.
        */
        void error(const std::basic_string<UniChar>& msg, const log4cxx::spi::LocationInfo& location) const;
        /**
        Log a message string with the ERROR level.

        <p>This method first checks if this logger is <code>ERROR</code>
        enabled by comparing the level of this logger with the
        ERROR level. If this logger is
        <code>ERROR</code> enabled, it proceeds to call all the
        registered appenders in this logger and also higher in the
        hierarchy depending on the value of the additivity flag.

        @param msg the message string to log.
        */
        void error(const std::basic_string<UniChar>& msg) const;
#endif
#if LOG4CXX_CFSTRING_API
        /**
        Log a message string with the ERROR level.

        <p>This method first checks if this logger is <code>ERROR</code>
        enabled by comparing the level of this logger with the
        ERROR level. If this logger is
        <code>ERROR</code> enabled, it proceeds to call all the
        registered appenders in this logger and also higher in the
        hierarchy depending on the value of the additivity flag.

        @param msg the message string to log.
        @param location location of source of logging request.
        */
        void error(const CFStringRef& msg, const log4cxx::spi::LocationInfo& location) const;
        /**
        Log a message string with the ERROR level.

        <p>This method first checks if this logger is <code>ERROR</code>
        enabled by comparing the level of this logger with the
        ERROR level. If this logger is
        <code>ERROR</code> enabled, it proceeds to call all the
        registered appenders in this logger and also higher in the
        hierarchy depending on the value of the additivity flag.

        @param msg the message string to log.
        */
        void error(const CFStringRef& msg) const;
#endif

        /**
        Log a message string with the FATAL level.

        <p>This method first checks if this logger is <code>FATAL</code>
        enabled by comparing the level of this logger with the
        FATAL level. If this logger is
        <code>FATAL</code> enabled, it proceeds to call all the
        registered appenders in this logger and also higher in the
        hierarchy depending on the value of the additivity flag.

        @param msg the message string to log.
        @param location location of source of logging request.
        */
        void fatal(const std::string& msg, const log4cxx::spi::LocationInfo& location) const;
        /**
        Log a message string with the ERROR level.

        <p>This method first checks if this logger is <code>ERROR</code>
        enabled by comparing the level of this logger with the
        ERROR level. If this logger is
        <code>ERROR</code> enabled, it proceeds to call all the
        registered appenders in this logger and also higher in the
        hierarchy depending on the value of the additivity flag.

        @param msg the message string to log.
        */
        void fatal(const std::string& msg) const;
#if LOG4CXX_WCHAR_T_API
        /**
        Log a message string with the ERROR level.

        <p>This method first checks if this logger is <code>ERROR</code>
        enabled by comparing the level of this logger with the
        ERROR level. If this logger is
        <code>ERROR</code> enabled, it proceeds to call all the
        registered appenders in this logger and also higher in the
        hierarchy depending on the value of the additivity flag.

        @param msg the message string to log.
        @param location location of source of logging request.
        */
        void fatal(const std::wstring& msg, const log4cxx::spi::LocationInfo& location) const;
        /**
        Log a message string with the ERROR level.

        <p>This method first checks if this logger is <code>ERROR</code>
        enabled by comparing the level of this logger with the
        ERROR level. If this logger is
        <code>ERROR</code> enabled, it proceeds to call all the
        registered appenders in this logger and also higher in the
        hierarchy depending on the value of the additivity flag.

        @param msg the message string to log.
        */
        void fatal(const std::wstring& msg) const;
#endif
#if LOG4CXX_UNICHAR_API
        /**
        Log a message string with the ERROR level.

        <p>This method first checks if this logger is <code>ERROR</code>
        enabled by comparing the level of this logger with the
        ERROR level. If this logger is
        <code>ERROR</code> enabled, it proceeds to call all the
        registered appenders in this logger and also higher in the
        hierarchy depending on the value of the additivity flag.

        @param msg the message string to log.
        @param location location of source of logging request.
        */
        void fatal(const std::basic_string<UniChar>& msg, const log4cxx::spi::LocationInfo& location) const;
        /**
        Log a message string with the ERROR level.

        <p>This method first checks if this logger is <code>ERROR</code>
        enabled by comparing the level of this logger with the
        ERROR level. If this logger is
        <code>ERROR</code> enabled, it proceeds to call all the
        registered appenders in this logger and also higher in the
        hierarchy depending on the value of the additivity flag.

        @param msg the message string to log.
        */
        void fatal(const std::basic_string<UniChar>& msg) const;
#endif
#if LOG4CXX_CFSTRING_API
        /**
        Log a message string with the ERROR level.

        <p>This method first checks if this logger is <code>ERROR</code>
        enabled by comparing the level of this logger with the
        ERROR level. If this logger is
        <code>ERROR</code> enabled, it proceeds to call all the
        registered appenders in this logger and also higher in the
        hierarchy depending on the value of the additivity flag.

        @param msg the message string to log.
        @param location location of source of logging request.
        */
        void fatal(const CFStringRef& msg, const log4cxx::spi::LocationInfo& location) const;
        /**
        Log a message string with the ERROR level.

        <p>This method first checks if this logger is <code>ERROR</code>
        enabled by comparing the level of this logger with the
        ERROR level. If this logger is
        <code>ERROR</code> enabled, it proceeds to call all the
        registered appenders in this logger and also higher in the
        hierarchy depending on the value of the additivity flag.

        @param msg the message string to log.
        */
        void fatal(const CFStringRef& msg) const;
#endif

        /**
        This method creates a new logging event and logs the event
        without further checks.
        @param level the level to log.
        @param message message.
        @param location location of source of logging request.
        */
        void forcedLog(const LevelPtr& level, const std::string& message,
                        const log4cxx::spi::LocationInfo& location) const;
        /**
        This method creates a new logging event and logs the event
        without further checks.
        @param level the level to log.
        @param message message.
        */
        void forcedLog(const LevelPtr& level, const std::string& message) const;

#if LOG4CXX_WCHAR_T_API
        /**
        This method creates a new logging event and logs the event
        without further checks.
        @param level the level to log.
        @param message message.
        @param location location of source of logging request.
        */
        void forcedLog(const LevelPtr& level, const std::wstring& message,
                        const log4cxx::spi::LocationInfo& location) const;
        /**
        This method creates a new logging event and logs the event
        without further checks.
        @param level the level to log.
        @param message message.
        */
        void forcedLog(const LevelPtr& level, const std::wstring& message) const;
#endif
#if LOG4CXX_UNICHAR_API || LOG4CXX_CFSTRING_API
        /**
        This method creates a new logging event and logs the event
        without further checks.
        @param level the level to log.
        @param message message.
        @param location location of source of logging request.
        */
        void forcedLog(const LevelPtr& level, const std::basic_string<UniChar>& message,
                        const log4cxx::spi::LocationInfo& location) const;
        /**
        This method creates a new logging event and logs the event
        without further checks.
        @param level the level to log.
        @param message message.
        */
        void forcedLog(const LevelPtr& level, const std::basic_string<UniChar>& message) const;
#endif
#if LOG4CXX_CFSTRING_API
        /**
        This method creates a new logging event and logs the event
        without further checks.
        @param level the level to log.
        @param message message.
        @param location location of source of logging request.
        */
        void forcedLog(const LevelPtr& level, const CFStringRef& message,
                        const log4cxx::spi::LocationInfo& location) const;
        /**
        This method creates a new logging event and logs the event
        without further checks.
        @param level the level to log.
        @param message message.
        */
        void forcedLog(const LevelPtr& level, const CFStringRef& message) const;
#endif
        /**
        This method creates a new logging event and logs the event
        without further checks.
        @param level the level to log.
        @param message the message string to log.
        @param location location of the logging statement.
        */
        void forcedLogLS(const LevelPtr& level, const LogString& message,
                        const log4cxx::spi::LocationInfo& location) const;

        /**
        Get the additivity flag for this Logger instance.
        */
        bool getAdditivity() const;

        /**
        Get the appenders contained in this logger as an AppenderList.
        If no appenders can be found, then an empty AppenderList
        is returned.
        @return AppenderList An collection of the appenders in this logger.*/
        AppenderList getAllAppenders() const;

        /**
        Look for the appender named as <code>name</code>.
        <p>Return the appender with that name if in the list. Return
        <code>NULL</code> otherwise.  */
        AppenderPtr getAppender(const LogString& name) const;

        /**
        Starting from this logger, search the logger hierarchy for a
        non-null level and return it.

        <p>The Logger class is designed so that this method executes as
        quickly as possible.

        @throws RuntimeException if all levels are null in the hierarchy
        */
        virtual const LevelPtr& getEffectiveLevel() const;

        /**
        Return the the LoggerRepository where this
        <code>Logger</code> is attached.
        */
        log4cxx::spi::LoggerRepositoryPtr getLoggerRepository() const;


        /**
        * Get the logger name.
        * @return logger name as LogString.  
        */
        const LogString getName() const { return name; }
        /**
        * Get logger name in current encoding.
        * @param name buffer to which name is appended.  
        */
        void getName(std::string& name) const;
#if LOG4CXX_WCHAR_T_API
        /**
        * Get logger name.
        * @param name buffer to which name is appended.  
        */
        void getName(std::wstring& name) const;
#endif
#if LOG4CXX_UNICHAR_API
        /**
        * Get logger name.
        * @param name buffer to which name is appended.  
        */
        void getName(std::basic_string<UniChar>& name) const;
#endif
#if LOG4CXX_CFSTRING_API
        /**
        * Get logger name.
        * @param name buffer to which name is appended.  
        */
        void getName(CFStringRef& name) const;
#endif

        /**
        Returns the parent of this logger. Note that the parent of a
        given logger may change during the lifetime of the logger.

        <p>The root logger will return <code>0</code>.
        */
        LoggerPtr getParent() const;


        /**
        Returns the assigned Level, if any, for this Logger.

        @return Level - the assigned Level, can be null.
        */
        LevelPtr getLevel() const;

        /**
        * Retrieve a logger by name in current encoding.
        * @param name logger name. 
        */
        static LoggerPtr getLogger(const std::string& name);
        /**
        * Retrieve a logger by name in current encoding.
        * @param name logger name. 
        */
        static LoggerPtr getLogger(const char* const name);
#if LOG4CXX_WCHAR_T_API
        /**
        * Retrieve a logger by name.
        * @param name logger name. 
        */
        static LoggerPtr getLogger(const std::wstring& name);
        /**
        * Retrieve a logger by name.
        * @param name logger name. 
        */
        static LoggerPtr getLogger(const wchar_t* const name);
#endif
#if LOG4CXX_UNICHAR_API
        /**
        * Retrieve a logger by name.
        * @param name logger name. 
        */
        static LoggerPtr getLogger(const std::basic_string<UniChar>& name);
#endif
#if LOG4CXX_CFSTRING_API
        /**
        * Retrieve a logger by name.
        * @param name logger name. 
        */
        static LoggerPtr getLogger(const CFStringRef& name);
#endif
        /**
        * Retrieve a logger by name in Unicode.
        * @param name logger name. 
        */
        static LoggerPtr getLoggerLS(const LogString& name);

        /**
        Retrieve the root logger.
        */
        static LoggerPtr getRootLogger();

        /**
        Like #getLogger except that the type of logger
        instantiated depends on the type returned by the
        LoggerFactory#makeNewLoggerInstance method of the
        <code>factory</code> parameter.

        <p>This method is intended to be used by sub-classes.

        @param name The name of the logger to retrieve.

        @param factory A LoggerFactory implementation that will
        actually create a new Instance.
        */
        static LoggerPtr getLoggerLS(const LogString& name,
                        const log4cxx::spi::LoggerFactoryPtr& factory);
        /**
        Like #getLogger except that the type of logger
        instantiated depends on the type returned by the
        LoggerFactory#makeNewLoggerInstance method of the
        <code>factory</code> parameter.

        <p>This method is intended to be used by sub-classes.

        @param name The name of the logger to retrieve.

        @param factory A LoggerFactory implementation that will
        actually create a new Instance.
        */
        static LoggerPtr getLogger(const std::string& name,
                        const log4cxx::spi::LoggerFactoryPtr& factory);
#if LOG4CXX_WCHAR_T_API
        /**
        Like #getLogger except that the type of logger
        instantiated depends on the type returned by the
        LoggerFactory#makeNewLoggerInstance method of the
        <code>factory</code> parameter.

        <p>This method is intended to be used by sub-classes.

        @param name The name of the logger to retrieve.

        @param factory A LoggerFactory implementation that will
        actually create a new Instance.
        */
        static LoggerPtr getLogger(const std::wstring& name,
                        const log4cxx::spi::LoggerFactoryPtr& factory);
#endif
#if LOG4CXX_UNICHAR_API
        /**
        Like #getLogger except that the type of logger
        instantiated depends on the type returned by the
        LoggerFactory#makeNewLoggerInstance method of the
        <code>factory</code> parameter.

        <p>This method is intended to be used by sub-classes.

        @param name The name of the logger to retrieve.

        @param factory A LoggerFactory implementation that will
        actually create a new Instance.
        */
        static LoggerPtr getLogger(const std::basic_string<UniChar>& name,
                        const log4cxx::spi::LoggerFactoryPtr& factory);
#endif
#if LOG4CXX_CFSTRING_API
        /**
        Like #getLogger except that the type of logger
        instantiated depends on the type returned by the
        LoggerFactory#makeNewLoggerInstance method of the
        <code>factory</code> parameter.

        <p>This method is intended to be used by sub-classes.

        @param name The name of the logger to retrieve.

        @param factory A LoggerFactory implementation that will
        actually create a new Instance.
        */
        static LoggerPtr getLogger(const CFStringRef& name,
                        const log4cxx::spi::LoggerFactoryPtr& factory);
#endif

        /**
        Return the <em>inherited</em> ResourceBundle for this logger.


        This method walks the hierarchy to find the appropriate resource bundle.
        It will return the resource bundle attached to the closest ancestor of
        this logger, much like the way priorities are searched. In case there
        is no bundle in the hierarchy then <code>NULL</code> is returned.
        */
        helpers::ResourceBundlePtr getResourceBundle() const;

        protected:
        /**
        Returns the string resource coresponding to <code>key</code> in this
        logger's inherited resource bundle.

        If the resource cannot be found, then an {@link #error error} message
        will be logged complaining about the missing resource.

        @see #getResourceBundle.
        */
        LogString getResourceBundleString(const LogString& key) const;

        public:
        /**
        Log a message string with the INFO level.

        <p>This method first checks if this logger is <code>INFO</code>
        enabled by comparing the level of this logger with the 
        INFO level. If this logger is
        <code>INFO</code> enabled, it proceeds to call all the
        registered appenders in this logger and also higher in the
        hierarchy depending on the value of the additivity flag.

        @param msg the message string to log.
        @param location location of source of logging request.
                */
       void info(const std::string& msg, const log4cxx::spi::LocationInfo& location) const;
       void info(const std::string& msg) const;
#if LOG4CXX_WCHAR_T_API
        /**
        Log a message string with the INFO level.

        <p>This method first checks if this logger is <code>INFO</code>
        enabled by comparing the level of this logger with the 
        INFO level. If this logger is
        <code>INFO</code> enabled, it proceeds to call all the
        registered appenders in this logger and also higher in the
        hierarchy depending on the value of the additivity flag.

        @param msg the message string to log.
        @param location location of source of logging request.
                */
       void info(const std::wstring& msg, const log4cxx::spi::LocationInfo& location) const;
        /**
        Log a message string with the INFO level.

        <p>This method first checks if this logger is <code>INFO</code>
        enabled by comparing the level of this logger with the 
        INFO level. If this logger is
        <code>INFO</code> enabled, it proceeds to call all the
        registered appenders in this logger and also higher in the
        hierarchy depending on the value of the additivity flag.

        @param msg the message string to log.
                */
       void info(const std::wstring& msg) const;
#endif
#if LOG4CXX_UNICHAR_API
        /**
        Log a message string with the INFO level.

        <p>This method first checks if this logger is <code>INFO</code>
        enabled by comparing the level of this logger with the 
        INFO level. If this logger is
        <code>INFO</code> enabled, it proceeds to call all the
        registered appenders in this logger and also higher in the
        hierarchy depending on the value of the additivity flag.

        @param msg the message string to log.
        @param location location of source of logging request.
                */
        void info(const std::basic_string<UniChar>& msg, const log4cxx::spi::LocationInfo& location) const;
        /**
        Log a message string with the INFO level.

        <p>This method first checks if this logger is <code>INFO</code>
        enabled by comparing the level of this logger with the 
        INFO level. If this logger is
        <code>INFO</code> enabled, it proceeds to call all the
        registered appenders in this logger and also higher in the
        hierarchy depending on the value of the additivity flag.

        @param msg the message string to log.
                */
        void info(const std::basic_string<UniChar>& msg) const;
#endif
#if LOG4CXX_CFSTRING_API
        /**
        Log a message string with the INFO level.

        <p>This method first checks if this logger is <code>INFO</code>
        enabled by comparing the level of this logger with the 
        INFO level. If this logger is
        <code>INFO</code> enabled, it proceeds to call all the
        registered appenders in this logger and also higher in the
        hierarchy depending on the value of the additivity flag.

        @param msg the message string to log.
        @param location location of source of logging request.
                */
        void info(const CFStringRef& msg, const log4cxx::spi::LocationInfo& location) const;
        /**
        Log a message string with the INFO level.

        <p>This method first checks if this logger is <code>INFO</code>
        enabled by comparing the level of this logger with the 
        INFO level. If this logger is
        <code>INFO</code> enabled, it proceeds to call all the
        registered appenders in this logger and also higher in the
        hierarchy depending on the value of the additivity flag.

        @param msg the message string to log.
                */
        void info(const CFStringRef& msg) const;
#endif

        /**
        Is the appender passed as parameter attached to this logger?
        */
        bool isAttached(const AppenderPtr& appender) const;

       /**
        *  Check whether this logger is enabled for the <code>DEBUG</code>
        *  Level.
        *
        *  <p> This function is intended to lessen the computational cost of
        *  disabled log debug statements.
        *
        *  <p> For some <code>logger</code> Logger object, when you write,
        *  <pre>
        *      logger->debug("debug message");
        *  </pre>
        *
        *  <p>You incur the cost constructing the message, concatenation in
        *  this case, regardless of whether the message is logged or not.
        *
        *  <p>If you are worried about speed, then you should write
        *  <pre>
        *    if(logger->isDebugEnabled()) {
        *      logger->debug("debug message");
        *    }
        *  </pre>
        *
        *  <p>This way you will not incur the cost of parameter
        *  construction if debugging is disabled for <code>logger</code>. On
        *  the other hand, if the <code>logger</code> is debug enabled, you
        *  will incur the cost of evaluating whether the logger is debug
        *  enabled twice. Once in <code>isDebugEnabled</code> and once in
        *  the <code>debug</code>.  This is an insignificant overhead
        *  since evaluating a logger takes about 1%% of the time it
        *  takes to actually log.
        *
        *  @return bool - <code>true</code> if this logger is debug
        *  enabled, <code>false</code> otherwise.
        *   */
        bool isDebugEnabled() const;

        /**
        Check whether this logger is enabled for a given
        Level passed as parameter.

        See also #isDebugEnabled.

        @return bool True if this logger is enabled for <code>level</code>.
        */
        bool isEnabledFor(const LevelPtr& level) const;


        /**
        Check whether this logger is enabled for the info Level.
        See also #isDebugEnabled.

        @return bool - <code>true</code> if this logger is enabled
        for level info, <code>false</code> otherwise.
        */
        bool isInfoEnabled() const;

         /**
        Check whether this logger is enabled for the warn Level.
        See also #isDebugEnabled.

        @return bool - <code>true</code> if this logger is enabled
        for level warn, <code>false</code> otherwise.
        */
        bool isWarnEnabled() const;

         /**
        Check whether this logger is enabled for the error Level.
        See also #isDebugEnabled.

        @return bool - <code>true</code> if this logger is enabled
        for level error, <code>false</code> otherwise.
        */
        bool isErrorEnabled() const;

         /**
        Check whether this logger is enabled for the fatal Level.
        See also #isDebugEnabled.

        @return bool - <code>true</code> if this logger is enabled
        for level fatal, <code>false</code> otherwise.
        */
        bool isFatalEnabled() const;

        /**
        Check whether this logger is enabled for the trace level.
        See also #isDebugEnabled.

        @return bool - <code>true</code> if this logger is enabled
        for level trace, <code>false</code> otherwise.
        */
        bool isTraceEnabled() const;

        /**
        Log a localized and parameterized message.

        First, the user supplied
        <code>key</code> is searched in the resource bundle. Next, the resulting
        pattern is formatted using helpers::StringHelper::format method with the user
        supplied string array <code>params</code>.

        @param level The level of the logging request.
        @param key The key to be searched in the ResourceBundle.
        @param locationInfo The location info of the logging request.
        @param values The values for the placeholders <code>{0}</code>,
                      <code>{1}</code> etc. within the pattern.

        @see #setResourceBundle
        */
        void l7dlog(const LevelPtr& level, const LogString& key,
                    const log4cxx::spi::LocationInfo& locationInfo,
                    const std::vector<LogString>& values) const;
        /**
        Log a localized and parameterized message.

        First, the user supplied
        <code>key</code> is searched in the resource bundle. Next, the resulting
        pattern is formatted using helpers::StringHelper::format method with the user
        supplied string array <code>params</code>.

        @param level The level of the logging request.
        @param key The key to be searched in the ResourceBundle.
        @param locationInfo The location info of the logging request.

        @see #setResourceBundle
        */
        void l7dlog(const LevelPtr& level, const std::string& key,
                    const log4cxx::spi::LocationInfo& locationInfo) const;
        /**
        Log a localized and parameterized message.

        First, the user supplied
        <code>key</code> is searched in the resource bundle. Next, the resulting
        pattern is formatted using helpers::StringHelper::format method with the user
        supplied string array <code>params</code>.

        @param level The level of the logging request.
        @param key The key to be searched in the ResourceBundle.
        @param locationInfo The location info of the logging request.
        @param val1 The first value for the placeholders within the pattern.

        @see #setResourceBundle
        */
        void l7dlog(const LevelPtr& level, const std::string& key,
                    const log4cxx::spi::LocationInfo& locationInfo,
                    const std::string& val1) const;
        /**
        Log a localized and parameterized message.

        First, the user supplied
        <code>key</code> is searched in the resource bundle. Next, the resulting
        pattern is formatted using helpers::StringHelper::format method with the user
        supplied string array <code>params</code>.

        @param level The level of the logging request.
        @param key The key to be searched in the ResourceBundle.
        @param locationInfo The location info of the logging request.
        @param val1 The first value for the placeholders within the pattern.
        @param val2 The second value for the placeholders within the pattern.

        @see #setResourceBundle
        */
        void l7dlog(const LevelPtr& level, const std::string& key,
                    const log4cxx::spi::LocationInfo& locationInfo,
                    const std::string& val1, const std::string& val2) const;
        /**
        Log a localized and parameterized message.

        First, the user supplied
        <code>key</code> is searched in the resource bundle. Next, the resulting
        pattern is formatted using helpers::StringHelper::format method with the user
        supplied string array <code>params</code>.

        @param level The level of the logging request.
        @param key The key to be searched in the ResourceBundle.
        @param locationInfo The location info of the logging request.
        @param val1 The value for the first placeholder within the pattern.
        @param val2 The value for the second placeholder within the pattern.
        @param val3 The value for the third placeholder within the pattern.

        @see #setResourceBundle
        */
        void l7dlog(const LevelPtr& level, const std::string& key,
                    const log4cxx::spi::LocationInfo& locationInfo,
                    const std::string& val1, const std::string& val2, const std::string& val3) const;

#if LOG4CXX_WCHAR_T_API
        /**
        Log a localized and parameterized message.

        First, the user supplied
        <code>key</code> is searched in the resource bundle. Next, the resulting
        pattern is formatted using helpers::StringHelper::format method with the user
        supplied string array <code>params</code>.

        @param level The level of the logging request.
        @param key The key to be searched in the ResourceBundle.
        @param locationInfo The location info of the logging request.

        @see #setResourceBundle
        */
        void l7dlog(const LevelPtr& level, const std::wstring& key,
                    const log4cxx::spi::LocationInfo& locationInfo) const;
        /**
        Log a localized and parameterized message.

        First, the user supplied
        <code>key</code> is searched in the resource bundle. Next, the resulting
        pattern is formatted using helpers::StringHelper::format method with the user
        supplied string array <code>params</code>.

        @param level The level of the logging request.
        @param key The key to be searched in the ResourceBundle.
        @param locationInfo The location info of the logging request.
        @param val1 The value for the first placeholder within the pattern.

        @see #setResourceBundle
        */
        void l7dlog(const LevelPtr& level, const std::wstring& key,
                    const log4cxx::spi::LocationInfo& locationInfo,
                    const std::wstring& val1) const;
        /**
        Log a localized and parameterized message.

        First, the user supplied
        <code>key</code> is searched in the resource bundle. Next, the resulting
        pattern is formatted using helpers::StringHelper::format method with the user
        supplied string array <code>params</code>.

        @param level The level of the logging request.
        @param key The key to be searched in the ResourceBundle.
        @param locationInfo The location info of the logging request.
        @param val1 The value for the first placeholder within the pattern.
        @param val2 The value for the second placeholder within the pattern.

        @see #setResourceBundle
        */
        void l7dlog(const LevelPtr& level, const std::wstring& key,
                    const log4cxx::spi::LocationInfo& locationInfo,
                    const std::wstring& val1, const std::wstring& val2) const;
        /**
        Log a localized and parameterized message.

        First, the user supplied
        <code>key</code> is searched in the resource bundle. Next, the resulting
        pattern is formatted using helpers::StringHelper::format method with the user
        supplied string array <code>params</code>.

        @param level The level of the logging request.
        @param key The key to be searched in the ResourceBundle.
        @param locationInfo The location info of the logging request.
        @param val1 The value for the first placeholder within the pattern.
        @param val2 The value for the second placeholder within the pattern.
        @param val3 The value for the third placeholder within the pattern.

        @see #setResourceBundle
        */
        void l7dlog(const LevelPtr& level, const std::wstring& key,
                    const log4cxx::spi::LocationInfo& locationInfo,
                    const std::wstring& val1, const std::wstring& val2, const std::wstring& val3) const;
#endif
#if LOG4CXX_UNICHAR_API
        /**
        Log a localized and parameterized message.

        First, the user supplied
        <code>key</code> is searched in the resource bundle. Next, the resulting
        pattern is formatted using helpers::StringHelper::format method with the user
        supplied string array <code>params</code>.

        @param level The level of the logging request.
        @param key The key to be searched in the ResourceBundle.
        @param locationInfo The location info of the logging request.

        @see #setResourceBundle
        */
        void l7dlog(const LevelPtr& level, const std::basic_string<UniChar>& key,
                    const log4cxx::spi::LocationInfo& locationInfo) const;
        /**
        Log a localized and parameterized message.

        First, the user supplied
        <code>key</code> is searched in the resource bundle. Next, the resulting
        pattern is formatted using helpers::StringHelper::format method with the user
        supplied string array <code>params</code>.

        @param level The level of the logging request.
        @param key The key to be searched in the ResourceBundle.
        @param locationInfo The location info of the logging request.
        @param val1 The value for the first placeholder within the pattern.

        @see #setResourceBundle
        */
        void l7dlog(const LevelPtr& level, const std::basic_string<UniChar>& key,
                    const log4cxx::spi::LocationInfo& locationInfo,
                    const std::basic_string<UniChar>& val1) const;
        /**
        Log a localized and parameterized message.

        First, the user supplied
        <code>key</code> is searched in the resource bundle. Next, the resulting
        pattern is formatted using helpers::StringHelper::format method with the user
        supplied string array <code>params</code>.

        @param level The level of the logging request.
        @param key The key to be searched in the ResourceBundle.
        @param locationInfo The location info of the logging request.
        @param val1 The value for the first placeholder within the pattern.
        @param val2 The value for the second placeholder within the pattern.

        @see #setResourceBundle
        */
        void l7dlog(const LevelPtr& level, const std::basic_string<UniChar>& key,
                    const log4cxx::spi::LocationInfo& locationInfo,
                    const std::basic_string<UniChar>& val1, const std::basic_string<UniChar>& val2) const;
        /**
        Log a localized and parameterized message.

        First, the user supplied
        <code>key</code> is searched in the resource bundle. Next, the resulting
        pattern is formatted using helpers::StringHelper::format method with the user
        supplied string array <code>params</code>.

        @param level The level of the logging request.
        @param key The key to be searched in the ResourceBundle.
        @param locationInfo The location info of the logging request.
        @param val1 The value for the first placeholder within the pattern.
        @param val2 The value for the second placeholder within the pattern.
        @param val3 The value for the third placeholder within the pattern.

        @see #setResourceBundle
        */
        void l7dlog(const LevelPtr& level, const std::basic_string<UniChar>& key,
                    const log4cxx::spi::LocationInfo& locationInfo,
                    const std::basic_string<UniChar>& val1, const std::basic_string<UniChar>& val2, 
                    const std::basic_string<UniChar>& val3) const;
#endif
#if LOG4CXX_CFSTRING_API
        /**
        Log a localized and parameterized message.

        First, the user supplied
        <code>key</code> is searched in the resource bundle. Next, the resulting
        pattern is formatted using helpers::StringHelper::format method with the user
        supplied string array <code>params</code>.

        @param level The level of the logging request.
        @param key The key to be searched in the ResourceBundle.
        @param locationInfo The location info of the logging request.

        @see #setResourceBundle
        */
        void l7dlog(const LevelPtr& level, const CFStringRef& key,
                    const log4cxx::spi::LocationInfo& locationInfo) const;
        /**
        Log a localized and parameterized message.

        First, the user supplied
        <code>key</code> is searched in the resource bundle. Next, the resulting
        pattern is formatted using helpers::StringHelper::format method with the user
        supplied string array <code>params</code>.

        @param level The level of the logging request.
        @param key The key to be searched in the ResourceBundle.
        @param locationInfo The location info of the logging request.
        @param val1 The value for the first placeholder within the pattern.

        @see #setResourceBundle
        */
        void l7dlog(const LevelPtr& level, const CFStringRef& key,
                    const log4cxx::spi::LocationInfo& locationInfo,
                    const CFStringRef& val1) const;
        /**
        Log a localized and parameterized message.

        First, the user supplied
        <code>key</code> is searched in the resource bundle. Next, the resulting
        pattern is formatted using helpers::StringHelper::format method with the user
        supplied string array <code>params</code>.

        @param level The level of the logging request.
        @param key The key to be searched in the ResourceBundle.
        @param locationInfo The location info of the logging request.
        @param val1 The value for the first placeholder within the pattern.
        @param val2 The value for the second placeholder within the pattern.

        @see #setResourceBundle
        */
        void l7dlog(const LevelPtr& level, const CFStringRef& key,
                    const log4cxx::spi::LocationInfo& locationInfo,
                    const CFStringRef& val1, const CFStringRef& val2) const;
        /**
        Log a localized and parameterized message.

        First, the user supplied
        <code>key</code> is searched in the resource bundle. Next, the resulting
        pattern is formatted using helpers::StringHelper::format method with the user
        supplied string array <code>params</code>.

        @param level The level of the logging request.
        @param key The key to be searched in the ResourceBundle.
        @param locationInfo The location info of the logging request.
        @param val1 The value for the first placeholder within the pattern.
        @param val2 The value for the second placeholder within the pattern.
        @param val3 The value for the third placeholder within the pattern.

        @see #setResourceBundle
        */
        void l7dlog(const LevelPtr& level, const CFStringRef& key,
                    const log4cxx::spi::LocationInfo& locationInfo,
                    const CFStringRef& val1, const CFStringRef& val2, 
                    const CFStringRef& val3) const;
#endif

          /**
        This is the most generic printing method. It is intended to be
        invoked by <b>wrapper</b> classes.

        @param level The level of the logging request.
        @param message The message of the logging request.
        @param location The source file of the logging request, may be null. */
        void log(const LevelPtr& level, const std::string& message,
            const log4cxx::spi::LocationInfo& location) const;
          /**
        This is the most generic printing method. It is intended to be
        invoked by <b>wrapper</b> classes.

        @param level The level of the logging request.
        @param message The message of the logging request.
        */
        void log(const LevelPtr& level, const std::string& message) const;
#if LOG4CXX_WCHAR_T_API
          /**
        This is the most generic printing method. It is intended to be
        invoked by <b>wrapper</b> classes.

        @param level The level of the logging request.
        @param message The message of the logging request.
        @param location The source file of the logging request, may be null. */
        void log(const LevelPtr& level, const std::wstring& message,
            const log4cxx::spi::LocationInfo& location) const;
          /**
        This is the most generic printing method. It is intended to be
        invoked by <b>wrapper</b> classes.

        @param level The level of the logging request.
        @param message The message of the logging request.
        */
        void log(const LevelPtr& level, const std::wstring& message) const;
#endif
#if LOG4CXX_UNICHAR_API
          /**
        This is the most generic printing method. It is intended to be
        invoked by <b>wrapper</b> classes.

        @param level The level of the logging request.
        @param message The message of the logging request.
        @param location The source file of the logging request, may be null. */
        void log(const LevelPtr& level, const std::basic_string<UniChar>& message,
            const log4cxx::spi::LocationInfo& location) const;
          /**
        This is the most generic printing method. It is intended to be
        invoked by <b>wrapper</b> classes.

        @param level The level of the logging request.
        @param message The message of the logging request.
        */
        void log(const LevelPtr& level, const std::basic_string<UniChar>& message) const;
#endif
#if LOG4CXX_CFSTRING_API
          /**
        This is the most generic printing method. It is intended to be
        invoked by <b>wrapper</b> classes.

        @param level The level of the logging request.
        @param message The message of the logging request.
        @param location The source file of the logging request, may be null. */
        void log(const LevelPtr& level, const CFStringRef& message,
            const log4cxx::spi::LocationInfo& location) const;
          /**
        This is the most generic printing method. It is intended to be
        invoked by <b>wrapper</b> classes.

        @param level The level of the logging request.
        @param message The message of the logging request.
        */
        void log(const LevelPtr& level, const CFStringRef& message) const;
#endif
          /**
        This is the most generic printing method. It is intended to be
        invoked by <b>wrapper</b> classes.

        @param level The level of the logging request.
        @param message The message of the logging request.
        @param location The source file of the logging request, may be null. */
        void logLS(const LevelPtr& level, const LogString& message,
            const log4cxx::spi::LocationInfo& location) const;



        /**
        Remove all previously added appenders from this logger
        instance.
        <p>This is useful when re-reading configuration information.
        */
        void removeAllAppenders();

        /**
        Remove the appender passed as parameter form the list of appenders.
        */
        void removeAppender(const AppenderPtr& appender);

        /**
        Remove the appender with the name passed as parameter form the
        list of appenders.
         */
        void removeAppender(const LogString& name);

       /**
        Set the additivity flag for this Logger instance.
         */
        void setAdditivity(bool additive);

    protected:
        friend class Hierarchy;
        /**
        Only the Hierarchy class can set the hierarchy of a logger.*/
        void setHierarchy(spi::LoggerRepository * repository);

        public:
        /**
        Set the level of this Logger.

        <p>As in <pre> &nbsp;&nbsp;&nbsp;logger->setLevel(Level::getDebug()); </pre>

        <p>Null values are admitted.  */
        virtual void setLevel(const LevelPtr& level);

        /**
        Set the resource bundle to be used with localized logging methods.
        */
        inline void setResourceBundle(const helpers::ResourceBundlePtr& bundle)
                { resourceBundle = bundle; }

#if LOG4CXX_WCHAR_T_API
        /**
        Log a message string with the WARN level.

        <p>This method first checks if this logger is <code>WARN</code>
        enabled by comparing the level of this logger with the
        WARN level. If this logger is
        <code>WARN</code> enabled, it proceeds to call all the
        registered appenders in this logger and also higher in the
        hierarchy depending on the value of the additivity flag.

        @param msg the message string to log.
        @param location location of source of logging request.
        */
        void warn(const std::wstring& msg, const log4cxx::spi::LocationInfo& location) const;
        /**
        Log a message string with the WARN level.

        <p>This method first checks if this logger is <code>WARN</code>
        enabled by comparing the level of this logger with the
        WARN level. If this logger is
        <code>WARN</code> enabled, it proceeds to call all the
        registered appenders in this logger and also higher in the
        hierarchy depending on the value of the additivity flag.

        @param msg the message string to log.
        */
        void warn(const std::wstring& msg) const;
#endif
#if LOG4CXX_UNICHAR_API
        /**
        Log a message string with the WARN level.

        <p>This method first checks if this logger is <code>WARN</code>
        enabled by comparing the level of this logger with the
        WARN level. If this logger is
        <code>WARN</code> enabled, it proceeds to call all the
        registered appenders in this logger and also higher in the
        hierarchy depending on the value of the additivity flag.

        @param msg the message string to log.
        @param location location of source of logging request.
        */
        void warn(const std::basic_string<UniChar>& msg, const log4cxx::spi::LocationInfo& location) const;
        /**
        Log a message string with the WARN level.

        <p>This method first checks if this logger is <code>WARN</code>
        enabled by comparing the level of this logger with the
        WARN level. If this logger is
        <code>WARN</code> enabled, it proceeds to call all the
        registered appenders in this logger and also higher in the
        hierarchy depending on the value of the additivity flag.

        @param msg the message string to log.
        */
        void warn(const std::basic_string<UniChar>& msg) const;
#endif
#if LOG4CXX_CFSTRING_API
        /**
        Log a message string with the WARN level.

        <p>This method first checks if this logger is <code>WARN</code>
        enabled by comparing the level of this logger with the
        WARN level. If this logger is
        <code>WARN</code> enabled, it proceeds to call all the
        registered appenders in this logger and also higher in the
        hierarchy depending on the value of the additivity flag.

        @param msg the message string to log.
        @param location location of source of logging request.
        */
        void warn(const CFStringRef& msg, const log4cxx::spi::LocationInfo& location) const;
        /**
        Log a message string with the WARN level.

        <p>This method first checks if this logger is <code>WARN</code>
        enabled by comparing the level of this logger with the
        WARN level. If this logger is
        <code>WARN</code> enabled, it proceeds to call all the
        registered appenders in this logger and also higher in the
        hierarchy depending on the value of the additivity flag.

        @param msg the message string to log.
        */
        void warn(const CFStringRef& msg) const;
#endif
        /**
        Log a message string with the WARN level.

        <p>This method first checks if this logger is <code>WARN</code>
        enabled by comparing the level of this logger with the
        WARN level. If this logger is
        <code>WARN</code> enabled, it proceeds to call all the
        registered appenders in this logger and also higher in the
        hierarchy depending on the value of the additivity flag.

        @param msg the message string to log.
        @param location location of source of logging request.
        */
        void warn(const std::string& msg, const log4cxx::spi::LocationInfo& location) const;
        /**
        Log a message string with the WARN level.

        <p>This method first checks if this logger is <code>WARN</code>
        enabled by comparing the level of this logger with the
        WARN level. If this logger is
        <code>WARN</code> enabled, it proceeds to call all the
        registered appenders in this logger and also higher in the
        hierarchy depending on the value of the additivity flag.

        @param msg the message string to log.
        */
        void warn(const std::string& msg) const;

#if LOG4CXX_WCHAR_T_API
        /**
        Log a message string with the TRACE level.

        <p>This method first checks if this logger is <code>TRACE</code>
        enabled by comparing the level of this logger with the
        TRACE level. If this logger is
        <code>TRACE</code> enabled, it proceeds to call all the
        registered appenders in this logger and also higher in the
        hierarchy depending on the value of the additivity flag.

        @param msg the message string to log.
        @param location location of source of logging request.
        */
        void trace(const std::wstring& msg, const log4cxx::spi::LocationInfo& location) const;
        /**
        Log a message string with the TRACE level.

        <p>This method first checks if this logger is <code>TRACE</code>
        enabled by comparing the level of this logger with the
        TRACE level. If this logger is
        <code>TRACE</code> enabled, it proceeds to call all the
        registered appenders in this logger and also higher in the
        hierarchy depending on the value of the additivity flag.

        @param msg the message string to log.
        */
        void trace(const std::wstring& msg) const;
#endif
#if LOG4CXX_UNICHAR_API
        /**
        Log a message string with the TRACE level.

        <p>This method first checks if this logger is <code>TRACE</code>
        enabled by comparing the level of this logger with the
        TRACE level. If this logger is
        <code>TRACE</code> enabled, it proceeds to call all the
        registered appenders in this logger and also higher in the
        hierarchy depending on the value of the additivity flag.

        @param msg the message string to log.
        @param location location of source of logging request.
        */
        void trace(const std::basic_string<UniChar>& msg, const log4cxx::spi::LocationInfo& location) const;
        /**
        Log a message string with the TRACE level.

        <p>This method first checks if this logger is <code>TRACE</code>
        enabled by comparing the level of this logger with the
        TRACE level. If this logger is
        <code>TRACE</code> enabled, it proceeds to call all the
        registered appenders in this logger and also higher in the
        hierarchy depending on the value of the additivity flag.

        @param msg the message string to log.
        */
        void trace(const std::basic_string<UniChar>& msg) const;
#endif
#if LOG4CXX_CFSTRING_API
        /**
        Log a message string with the TRACE level.

        <p>This method first checks if this logger is <code>TRACE</code>
        enabled by comparing the level of this logger with the
        TRACE level. If this logger is
        <code>TRACE</code> enabled, it proceeds to call all the
        registered appenders in this logger and also higher in the
        hierarchy depending on the value of the additivity flag.

        @param msg the message string to log.
        @param location location of source of logging request.
        */
        void trace(const CFStringRef& msg, const log4cxx::spi::LocationInfo& location) const;
        /**
        Log a message string with the TRACE level.

        <p>This method first checks if this logger is <code>TRACE</code>
        enabled by comparing the level of this logger with the
        TRACE level. If this logger is
        <code>TRACE</code> enabled, it proceeds to call all the
        registered appenders in this logger and also higher in the
        hierarchy depending on the value of the additivity flag.

        @param msg the message string to log.
        */
        void trace(const CFStringRef& msg) const;
#endif
        /**
        Log a message string with the TRACE level.

        <p>This method first checks if this logger is <code>TRACE</code>
        enabled by comparing the level of this logger with the
        TRACE level. If this logger is
        <code>TRACE</code> enabled, it proceeds to call all the
        registered appenders in this logger and also higher in the
        hierarchy depending on the value of the additivity flag.

        @param msg the message string to log.
        @param location location of source of logging request.
        */
        void trace(const std::string& msg, const log4cxx::spi::LocationInfo& location) const;
        /**
        Log a message string with the TRACE level.

        <p>This method first checks if this logger is <code>TRACE</code>
        enabled by comparing the level of this logger with the
        TRACE level. If this logger is
        <code>TRACE</code> enabled, it proceeds to call all the
        registered appenders in this logger and also higher in the
        hierarchy depending on the value of the additivity flag.

        @param msg the message string to log.
        */
        void trace(const std::string& msg) const;

        inline const log4cxx::helpers::Mutex& getMutex() const { return mutex; }

        private:
                //
        //  prevent copy and assignment
        Logger(const Logger&);
        Logger& operator=(const Logger&);
        log4cxx::helpers::Mutex mutex;
        friend class log4cxx::helpers::synchronized;
   };
   LOG4CXX_LIST_DEF(LoggerList, LoggerPtr);
   
}

/** @addtogroup LoggingMacros Logging macros
@{
*/

#if !defined(LOG4CXX_UNLIKELY)
#if __GNUC__ >= 3
/**
Provides optimization hint to the compiler
to optimize for the expression being false.
@param expr boolean expression.
@returns value of expression.
*/
#define LOG4CXX_UNLIKELY(expr) __builtin_expect(expr, 0)
#else
/**
Provides optimization hint to the compiler
to optimize for the expression being false.
@param expr boolean expression.
@returns value of expression.
**/
#define LOG4CXX_UNLIKELY(expr) expr
#endif
#endif


/**
Logs a message to a specified logger with a specified level.

@param logger the logger to be used.
@param level the level to log.
@param message the message string to log.
*/
#define LOG4CXX_LOG(logger, level, message) { \
        if (logger->isEnabledFor(level)) {\
           ::log4cxx::helpers::MessageBuffer oss_; \
           logger->forcedLog(level, oss_.str(oss_ << message), LOG4CXX_LOCATION); } }

/**
Logs a message to a specified logger with a specified level.

@param logger the logger to be used.
@param level the level to log.
@param message the message string to log in the internal encoding.
*/
#define LOG4CXX_LOGLS(logger, level, message) { \
        if (logger->isEnabledFor(level)) {\
           ::log4cxx::helpers::LogCharMessageBuffer oss_; \
           logger->forcedLog(level, oss_.str(oss_ << message), LOG4CXX_LOCATION); } }

/**
Logs a message to a specified logger with the DEBUG level.

@param logger the logger to be used.
@param message the message string to log.
*/
#define LOG4CXX_DEBUG(logger, message) { \
        if (LOG4CXX_UNLIKELY(logger->isDebugEnabled())) {\
           ::log4cxx::helpers::MessageBuffer oss_; \
           logger->forcedLog(::log4cxx::Level::getDebug(), oss_.str(oss_ << message), LOG4CXX_LOCATION); }}

/**
Logs a message to a specified logger with the TRACE level.

@param logger the logger to be used.
@param message the message string to log.
*/
#define LOG4CXX_TRACE(logger, message) { \
        if (LOG4CXX_UNLIKELY(logger->isTraceEnabled())) {\
           ::log4cxx::helpers::MessageBuffer oss_; \
           logger->forcedLog(::log4cxx::Level::getTrace(), oss_.str(oss_ << message), LOG4CXX_LOCATION); }}


/**
Logs a message to a specified logger with the INFO level.

@param logger the logger to be used.
@param message the message string to log.
*/
#define LOG4CXX_INFO(logger, message) { \
        if (logger->isInfoEnabled()) {\
           ::log4cxx::helpers::MessageBuffer oss_; \
           logger->forcedLog(::log4cxx::Level::getInfo(), oss_.str(oss_ << message), LOG4CXX_LOCATION); }}

/**
Logs a message to a specified logger with the WARN level.

@param logger the logger to be used.
@param message the message string to log.
*/
#define LOG4CXX_WARN(logger, message) { \
        if (logger->isWarnEnabled()) {\
           ::log4cxx::helpers::MessageBuffer oss_; \
           logger->forcedLog(::log4cxx::Level::getWarn(), oss_.str(oss_ << message), LOG4CXX_LOCATION); }}

/**
Logs a message to a specified logger with the ERROR level.

@param logger the logger to be used.
@param message the message string to log.
*/
#define LOG4CXX_ERROR(logger, message) { \
        if (logger->isErrorEnabled()) {\
           ::log4cxx::helpers::MessageBuffer oss_; \
           logger->forcedLog(::log4cxx::Level::getError(), oss_.str(oss_ << message), LOG4CXX_LOCATION); }}

/**
Logs a error if the condition is not true.

@param logger the logger to be used.
@param condition condition
@param message the message string to log.
*/
#define LOG4CXX_ASSERT(logger, condition, message) { \
        if (!(condition) && logger->isErrorEnabled()) {\
           ::log4cxx::helpers::MessageBuffer oss_; \
           logger->forcedLog(::log4cxx::Level::getError(), oss_.str(oss_ << message), LOG4CXX_LOCATION); }}


/**
Logs a message to a specified logger with the FATAL level.

@param logger the logger to be used.
@param message the message string to log.
*/
#define LOG4CXX_FATAL(logger, message) { \
        if (logger->isFatalEnabled()) {\
           ::log4cxx::helpers::MessageBuffer oss_; \
           logger->forcedLog(::log4cxx::Level::getFatal(), oss_.str(oss_ << message), LOG4CXX_LOCATION); }}

/**
Logs a localized message with no parameter.

@param logger the logger to be used.
@param level the level to log.
@param key the key to be searched in the resourceBundle of the logger.
*/
#define LOG4CXX_L7DLOG(logger, level, key) { \
        if (logger->isEnabledFor(level)) {\
        logger->l7dlog(level, key, LOG4CXX_LOCATION); }}

/**
Logs a localized message with one parameter.

@param logger the logger to be used.
@param level the level to log.
@param key the key to be searched in the resourceBundle of the logger.
@param p1 the unique parameter.
*/
#define LOG4CXX_L7DLOG1(logger, level, key, p1) { \
        if (logger->isEnabledFor(level)) {\
        logger->l7dlog(level, key, LOG4CXX_LOCATION, p1); }}

/**
Logs a localized message with two parameters.

@param logger the logger to be used.
@param level the level to log.
@param key the key to be searched in the resourceBundle of the logger.
@param p1 the first parameter.
@param p2 the second parameter.
*/
#define LOG4CXX_L7DLOG2(logger, level, key, p1, p2) { \
        if (logger->isEnabledFor(level)) {\
        logger->l7dlog(level, key, LOG4CXX_LOCATION, p1, p2); }}

/**
Logs a localized message with three parameters.

@param logger the logger to be used.
@param level the level to log.
@param key the key to be searched in the resourceBundle of the logger.
@param p1 the first parameter.
@param p2 the second parameter.
@param p3 the third parameter.
*/
#define LOG4CXX_L7DLOG3(logger, level, key, p1, p2, p3) { \
        if (logger->isEnabledFor(level)) {\
        logger->l7dlog(level, key, LOG4CXX_LOCATION, p1, p2, p3); }}

/**@}*/

#if defined(_MSC_VER)
#pragma warning ( pop )
#endif

#include <log4cxx/spi/loggerrepository.h>

#endif //_LOG4CXX_LOGGER_H
