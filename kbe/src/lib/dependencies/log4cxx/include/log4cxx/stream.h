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

#ifndef _LOG4CXX_STREAM_H
#define _LOG4CXX_STREAM_H

#include <log4cxx/logger.h>
#include <sstream>
#include <log4cxx/spi/location/locationinfo.h>

namespace log4cxx
{

       /**
         *   Base class for the basic_logstream template which attempts
         *   to emulate std::basic_ostream but attempts to short-circuit
         *   unnecessary operations.
         *
         *   The logstream has a logger and level that are used for logging
         *   requests.  The level of the stream is compared against the 
         *   current level of the logger to determine if the request should be processed.
         */
        class LOG4CXX_EXPORT logstream_base {
        public:
             /**
              *  Create new instance.
              *  @param logger logger logger used in log requests.
              *  @param level indicates level that will be used in log requests.  Can
              *      be modified later by inserting a level or calling setLevel.
              */
             logstream_base(const log4cxx::LoggerPtr& logger,
                 const log4cxx::LevelPtr& level);
             /**
              *  Destructor.
              */
             virtual ~logstream_base();
             /**
              *  Insertion operator for std::fixed and similar manipulators.
              */
             void insert(std::ios_base& (*manip)(std::ios_base&));

             /**
              *   get precision.
              */
             int precision();
             /**
              *   get width.
              */
             int width();
             /**
              *   set precision.  This should be used in preference to inserting an std::setprecision(n)
              *   since the other requires construction of an STL stream which may be expensive.
              */
             int precision(int newval);
             /**
              *   set width.  This should be used in preference to inserting an std::setw(n)
              *   since the other requires construction of an STL stream which may be expensive.
              */
             int width(int newval);
             /**
              *   Get fill character.
              */
             int fill();
             /**
              *  Set fill character.
              */
             int fill(int newval);
             
             /**
              *   Set flags. see std::ios_base.
              */
             std::ios_base::fmtflags flags(std::ios_base::fmtflags newflags);
             /**
              *   Set flags. see std::ios_base.
              */
             std::ios_base::fmtflags setf(std::ios_base::fmtflags newflags, std::ios_base::fmtflags mask);
             /**
              *   Set flags. see std::ios_base.
              */
             std::ios_base::fmtflags setf(std::ios_base::fmtflags newflags);
             
             
             /**
              *  end of message manipulator, triggers logging.
              */
             static logstream_base& endmsg(logstream_base&);

             /**
              *  no-operation manipulator,  Used to avoid ambiguity with VC6.
              */
             static logstream_base& nop(logstream_base&);
             
             /**
              *   end of message action.
              */
             void end_message();


            
             /**
              * Set the level.
              * @param level level
              */
              void setLevel(const LevelPtr& level);
              /**
               *  Returns true if the current level is the same or high as the 
               *  level of logger at time of construction or last setLevel.
               */
              inline bool isEnabled() const {
                 return enabled;
              }

              /**
               *  Returns if logger is currently enabled for the specified level.
               */
              bool isEnabledFor(const LevelPtr& level) const;

              /**
               *  Sets the location for subsequent log requests.
               */
              void setLocation(const log4cxx::spi::LocationInfo& location);

              /**
               *  Sets the state of the embedded stream (if any)
               *     to the state of the formatting info.
               *   @param os stream to receive formatting info.
               *   @param fillchar receives fill charater.
               *   @return true if fill character was specified.     
               */
              bool set_stream_state(std::ios_base& os, int& fillchar);

        protected:
              /**
               *   Dispatches the pending log request.
               */
              virtual void log(LoggerPtr& logger,
                               const LevelPtr& level,
                               const log4cxx::spi::LocationInfo& location) = 0;
              /**
               *   Erase any content in the message construction buffer.
               */
              virtual void erase() = 0;
              /**
               *   Copy state of embedded stream (if any)
               *      to value and mask instances of std::ios_base
               *      and return fill character value.
               */
              virtual void get_stream_state(std::ios_base& base,
                                            std::ios_base& mask,
                                            int& fill,
                                            bool& fillSet) const = 0;
              virtual void refresh_stream_state() = 0;
             
        private:
            /**
             *   prevent copy constructor.
             */
            logstream_base(logstream_base&);
            /**
             *   prevent copy operatpr.
             */
            logstream_base& operator=(logstream_base&);
            /**
             *   Minimal extension of std::ios_base to allow creation
             *     of embedded IO states.
             */
            class LOG4CXX_EXPORT logstream_ios_base : public std::ios_base {
            public:
                logstream_ios_base(std::ios_base::fmtflags initval, 
                    int initsize);
            } initset, initclear;
            /**
             *   fill character.
             */
            int fillchar;
            /**
             *   true if fill character is set.
             */
            bool fillset;
            /**
             *   true if assigned level was same or higher than level of associated logger.
             */
            bool enabled;
            /**
             *   associated logger.
             */
            log4cxx::LoggerPtr logger;
            /**
             *   associated level.
             */
            log4cxx::LevelPtr level;
            /**
             *   associated level.
             */
            log4cxx::spi::LocationInfo location;
        };
        
      typedef logstream_base& (*logstream_manipulator)(logstream_base&);
       
        /**
         *  An STL-like stream API for log4cxx using char as the character type.
       *. Instances of log4cxx::logstream
         *  are not  designedfor use by multiple threads and in general should be short-lived
         *  function scoped objects.  Using log4cxx::basic_logstream as a class member or 
         *  static instance should be avoided in the same manner as you would avoid placing a std::ostringstream
         *  in those locations.  Insertion operations are generally short-circuited if the 
         *  level for the stream is not the same of higher that the level of the associated logger.
         */
        class LOG4CXX_EXPORT logstream : public logstream_base {
          typedef char Ch;
        public:
            /**
             *   Constructor.
             */
             logstream(const log4cxx::LoggerPtr& logger,
                 const log4cxx::LevelPtr& level);
             
            /**
             *   Constructor.
             */
             logstream(const Ch* loggerName, 
                const log4cxx::LevelPtr& level);

            /**
             *   Constructor.
             */
             logstream(const std::basic_string<Ch>& loggerName, 
                const log4cxx::LevelPtr& level);
             
             ~logstream();
             
             /**
              *   Insertion operator for std::fixed and similar manipulators.
              */
             logstream& operator<<(std::ios_base& (*manip)(std::ios_base&));
            
             /**
              *   Insertion operator for logstream_base::endmsg.
              */
              logstream& operator<<(logstream_manipulator manip);
            
              /**
               *   Insertion operator for level.
               */
              logstream& operator<<(const log4cxx::LevelPtr& level);
            /**
             *   Insertion operator for location.
             */
             logstream& operator<<(const log4cxx::spi::LocationInfo& location);
            
            /**
             *   Alias for insertion operator for location.  Kludge to avoid
          *      inappropriate compiler ambiguity.
             */
             logstream& operator>>(const log4cxx::spi::LocationInfo& location);

            /**
             *   Cast operator to provide access to embedded std::basic_ostream.
             */
             operator std::basic_ostream<Ch>&();

#if !(LOG4CXX_USE_GLOBAL_SCOPE_TEMPLATE)             
            /**
              *  Template to allow any class with an std::basic_ostream inserter
              *    to be applied to this class.
             */
             template <class V>
             inline log4cxx::logstream& operator<<(const V& val) {
                 if (LOG4CXX_UNLIKELY(isEnabled())) {
                      ((std::basic_ostream<char>&) *this) << val;
                 }
                 return *this;
              }
#endif              
             
            
        protected:
              virtual void log(LoggerPtr& logger,
                               const LevelPtr& level,
                               const log4cxx::spi::LocationInfo& location);
              
              virtual void erase();
              
              virtual void get_stream_state(std::ios_base& base,
                                            std::ios_base& mask,
                                            int& fill,
                                            bool& fillSet) const;
              virtual void refresh_stream_state();
              
            
        private:
            logstream(const logstream&);
            logstream& operator=(const logstream&);        
            std::basic_stringstream<Ch>* stream;
             
        };
        
#if LOG4CXX_WCHAR_T_API        
        /**
         *  An STL-like stream API for log4cxx using wchar_t as the character type.
       *. Instances of log4cxx::logstream
         *  are not  designedfor use by multiple threads and in general should be short-lived
         *  function scoped objects.  Using log4cxx::basic_logstream as a class member or 
         *  static instance should be avoided in the same manner as you would avoid placing a std::ostringstream
         *  in those locations.  Insertion operations are generally short-circuited if the 
         *  level for the stream is not the same of higher that the level of the associated logger.
         */
        class LOG4CXX_EXPORT wlogstream : public logstream_base {
          typedef wchar_t Ch;
        public:
            /**
             *   Constructor.
             */
             wlogstream(const log4cxx::LoggerPtr& logger,
                 const log4cxx::LevelPtr& level);
             
            /**
             *   Constructor.
             */
             wlogstream(const Ch* loggerName, 
                const log4cxx::LevelPtr& level);

            /**
             *   Constructor.
             */
             wlogstream(const std::basic_string<Ch>& loggerName, 
                const log4cxx::LevelPtr& level);
             
             ~wlogstream();
             
             /**
              *   Insertion operator for std::fixed and similar manipulators.
              */
             wlogstream& operator<<(std::ios_base& (*manip)(std::ios_base&));
            
             /**
              *   Insertion operator for logstream_base::endmsg.
              */
              wlogstream& operator<<(logstream_manipulator manip);
            
              /**
               *   Insertion operator for level.
               */
              wlogstream& operator<<(const log4cxx::LevelPtr& level);
            /**
             *   Insertion operator for location.
             */
            wlogstream& operator<<(const log4cxx::spi::LocationInfo& location);
            
            /**
             *   Alias for insertion operator for location.  Kludge to avoid
          *      inappropriate compiler ambiguity.
             */
             wlogstream& operator>>(const log4cxx::spi::LocationInfo& location);
            

            /**
             *   Cast operator to provide access to embedded std::basic_ostream.
             */
             operator std::basic_ostream<Ch>&();
            
#if !(LOG4CXX_USE_GLOBAL_SCOPE_TEMPLATE)             
            /**
              *  Template to allow any class with an std::basic_ostream inserter
              *    to be applied to this class.
             */
             template <class V>
             inline log4cxx::wlogstream& operator<<(const V& val) {
                 if (LOG4CXX_UNLIKELY(isEnabled())) {
                      ((std::basic_ostream<wchar_t>&) *this) << val;
                 }
                 return *this;
              }
#endif              
            
        protected:
              virtual void log(LoggerPtr& logger,
                               const LevelPtr& level,
                               const log4cxx::spi::LocationInfo& location);
              
              virtual void erase();
              
              virtual void get_stream_state(std::ios_base& base,
                                            std::ios_base& mask,
                                            int& fill,
                                            bool& fillSet) const;
              virtual void refresh_stream_state();
              
            
        private:
            wlogstream(const wlogstream&);
            wlogstream& operator=(const wlogstream&);
            std::basic_stringstream<Ch>* stream;
             
        };
#endif

#if LOG4CXX_UNICHAR_API || LOG4CXX_CFSTRING_API        
        /**
         *  An STL-like stream API for log4cxx using UniChar as the character type.
       *. Instances of log4cxx::logstream
         *  are not  designedfor use by multiple threads and in general should be short-lived
         *  function scoped objects.  Using log4cxx::basic_logstream as a class member or 
         *  static instance should be avoided in the same manner as you would avoid placing a std::ostringstream
         *  in those locations.  Insertion operations are generally short-circuited if the 
         *  level for the stream is not the same of higher that the level of the associated logger.
         */
        class LOG4CXX_EXPORT ulogstream : public logstream_base {
          typedef UniChar Ch;
        public:
            /**
             *   Constructor.
             */
             ulogstream(const log4cxx::LoggerPtr& logger,
                 const log4cxx::LevelPtr& level);
             
#if LOG4CXX_UNICHAR_API             
            /**
             *   Constructor.
             */
             ulogstream(const Ch* loggerName, 
                const log4cxx::LevelPtr& level);

            /**
             *   Constructor.
             */
             ulogstream(const std::basic_string<Ch>& loggerName, 
                const log4cxx::LevelPtr& level);
#endif

#if LOG4CXX_CFSTRING_API
             ulogstream(const CFStringRef& loggerName,
                   const log4cxx::LevelPtr& level);
#endif             
             
             ~ulogstream();
             
             /**
              *   Insertion operator for std::fixed and similar manipulators.
              */
             ulogstream& operator<<(std::ios_base& (*manip)(std::ios_base&));
            
             /**
              *   Insertion operator for logstream_base::endmsg.
              */
              ulogstream& operator<<(logstream_manipulator manip);
            
              /**
               *   Insertion operator for level.
               */
              ulogstream& operator<<(const log4cxx::LevelPtr& level);
            /**
             *   Insertion operator for location.
             */
            ulogstream& operator<<(const log4cxx::spi::LocationInfo& location);
            
            /**
             *   Alias for insertion operator for location.  Kludge to avoid
          *      inappropriate compiler ambiguity.
             */
             ulogstream& operator>>(const log4cxx::spi::LocationInfo& location);
            

            /**
             *   Cast operator to provide access to embedded std::basic_ostream.
             */
             operator std::basic_ostream<Ch>&();
            
#if !(LOG4CXX_USE_GLOBAL_SCOPE_TEMPLATE)             
            /**
              *  Template to allow any class with an std::basic_ostream inserter
              *    to be applied to this class.
             */
             template <class V>
             inline ulogstream& operator<<(const V& val) {
                 if (LOG4CXX_UNLIKELY(isEnabled())) {
                      ((std::basic_ostream<Ch>&) *this) << val;
                 }
                 return *this;
              }
#endif              
            
        protected:
              virtual void log(LoggerPtr& logger,
                               const LevelPtr& level,
                               const log4cxx::spi::LocationInfo& location);
              
              virtual void erase();
              
              virtual void get_stream_state(std::ios_base& base,
                                            std::ios_base& mask,
                                            int& fill,
                                            bool& fillSet) const;
              virtual void refresh_stream_state();
              
            
        private:
            ulogstream(const ulogstream&);
            ulogstream& operator=(const ulogstream&);
            std::basic_stringstream<Ch>* stream;
             
        };
#endif


}  // namespace log4cxx


#if LOG4CXX_USE_GLOBAL_SCOPE_TEMPLATE
//
//  VC6 will fail to compile if class-scope templates
//     are used to handle arbitrary insertion operations.
//     However, using global namespace insertion operations 
//     run into LOGCXX-150.

/**
 *  Template to allow any class with an std::basic_ostream inserter
 *    to be applied to this class.
 */
template <class V>
inline log4cxx::logstream& operator<<(log4cxx::logstream& os, const V& val) {
     if (LOG4CXX_UNLIKELY(os.isEnabled())) {
         ((std::basic_ostream<char>&) os) << val;
     }
     return os;
}

#if LOG4CXX_WCHAR_T_API            
/**
 *  Template to allow any class with an std::basic_ostream inserter
 *    to be applied to this class.
 */
template <class V>
inline log4cxx::wlogstream& operator<<(log4cxx::wlogstream& os, const V& val) {
     if (LOG4CXX_UNLIKELY(os.isEnabled())) {
         ((std::basic_ostream<wchar_t>&) os) << val;
     }
     return os;
}
#endif
#endif

#if !defined(LOG4CXX_ENDMSG)
#if LOG4CXX_LOGSTREAM_ADD_NOP
#define LOG4CXX_ENDMSG (log4cxx::logstream_manipulator) log4cxx::logstream_base::nop >> LOG4CXX_LOCATION << (log4cxx::logstream_manipulator) log4cxx::logstream_base::endmsg
#else
#define LOG4CXX_ENDMSG LOG4CXX_LOCATION << (log4cxx::logstream_manipulator) log4cxx::logstream_base::endmsg
#endif
#endif


#endif //_LOG4CXX_STREAM_H
