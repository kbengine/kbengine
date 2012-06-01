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

#ifndef _LOG4CXX_MESSAGE_BUFFER_H
#define _LOG4CXX_MESSAGE_BUFFER_H

#include <log4cxx/log4cxx.h>
#include <log4cxx/logstring.h>
#include <sstream>

namespace log4cxx {


   namespace helpers {
   
   typedef std::ios_base& (*ios_base_manip)(std::ios_base&);

   /**
    *   This class is used by the LOG4CXX_INFO and similar
    *   macros to support insertion operators in the message parameter.
    *   The class is not intended for use outside of that context.
    */
   class LOG4CXX_EXPORT CharMessageBuffer {
   public:
        /**
         *  Creates a new instance.
         */
       CharMessageBuffer();
        /**
         *  Destructor.
         */
        ~CharMessageBuffer();

        
        /**
         *   Appends string to buffer.
         *   @param msg string append.
         *   @return this buffer.
         */
        CharMessageBuffer& operator<<(const std::basic_string<char>& msg);
        /**
         *   Appends string to buffer.
         *   @param msg string to append.
         *   @return this buffer.
         */
        CharMessageBuffer& operator<<(const char* msg);
        /**
         *   Appends string to buffer.
         *   @param msg string to append.
         *   @return this buffer.
         */
        CharMessageBuffer& operator<<(char* msg);

        /**
         *   Appends character to buffer.
         *   @param msg character to append.
         *   @return this buffer.
         */
        CharMessageBuffer& operator<<(const char msg);

        /**
         *   Insertion operator for STL manipulators such as std::fixed.
         *   @param manip manipulator.
         *   @return encapsulated STL stream.
         */
        std::ostream& operator<<(ios_base_manip manip);
        /**
         *   Insertion operator for built-in type.
         *   @param val build in type.
         *   @return encapsulated STL stream.
         */
        std::ostream& operator<<(bool val);

        /**
         *   Insertion operator for built-in type.
         *   @param val build in type.
         *   @return encapsulated STL stream.
         */
        std::ostream& operator<<(short val);
        /**
         *   Insertion operator for built-in type.
         *   @param val build in type.
         *   @return encapsulated STL stream.
         */
        std::ostream& operator<<(int val);
        /**
         *   Insertion operator for built-in type.
         *   @param val build in type.
         *   @return encapsulated STL stream.
         */
        std::ostream& operator<<(unsigned int val);
        /**
         *   Insertion operator for built-in type.
         *   @param val build in type.
         *   @return encapsulated STL stream.
         */
        std::ostream& operator<<(long val);
        /**
         *   Insertion operator for built-in type.
         *   @param val build in type.
         *   @return encapsulated STL stream.
         */
        std::ostream& operator<<(unsigned long val);
        /**
         *   Insertion operator for built-in type.
         *   @param val build in type.
         *   @return encapsulated STL stream.
         */
        std::ostream& operator<<(float val);
        /**
         *   Insertion operator for built-in type.
         *   @param val build in type.
         *   @return encapsulated STL stream.
         */
        std::ostream& operator<<(double val);
        /**
         *   Insertion operator for built-in type.
         *   @param val build in type.
         *   @return encapsulated STL stream.
         */
        std::ostream& operator<<(long double val);
        /**
         *   Insertion operator for built-in type.
         *   @param val build in type.
         *   @return encapsulated STL stream.
         */
        std::ostream& operator<<(void* val);

      /**
       *  Cast to ostream.
       */
      operator std::basic_ostream<char>&();

      /**
       *   Get content of buffer.
       *   @param os used only to signal that
       *       the embedded stream was used.
       */
      const std::basic_string<char>& str(std::basic_ostream<char>& os);

      /**
       *   Get content of buffer.
       *   @param buf used only to signal that
       *       the embedded stream was not used.
       */
      const std::basic_string<char>& str(CharMessageBuffer& buf);

        /**
         *  Returns true if buffer has an encapsulated STL stream.
         *  @return true if STL stream was created.
         */
        bool hasStream() const;

   private:
        /**
         * Prevent use of default copy constructor.
         */
      CharMessageBuffer(const CharMessageBuffer&);
        /**
         *   Prevent use of default assignment operator.  
         */
      CharMessageBuffer& operator=(const CharMessageBuffer&);

      /**
         * Encapsulated std::string.
         */
        std::basic_string<char> buf;
        /**
         *  Encapsulated stream, created on demand.
         */
        std::basic_ostringstream<char>* stream;
   };

template<class V>
std::basic_ostream<char>& operator<<(CharMessageBuffer& os, const V& val) {
   return ((std::basic_ostream<char>&) os) << val;
}

#if LOG4CXX_UNICHAR_API || LOG4CXX_CFSTRING_API || LOG4CXX_LOGCHAR_IS_UNICHAR
   /**
    *   This class is designed to support insertion operations
   *   in the message argument to the LOG4CXX_INFO and similar
   *   macros and is not designed for general purpose use.
   */
   class LOG4CXX_EXPORT UniCharMessageBuffer {
   public:
        /**
         *  Creates a new instance.
         */
       UniCharMessageBuffer();
        /**
         *  Destructor.
         */
        ~UniCharMessageBuffer();
        
        typedef std::basic_ostream<UniChar> uostream;

        
        /**
         *   Appends string to buffer.
         *   @param msg string append.
         *   @return this buffer.
         */
        UniCharMessageBuffer& operator<<(const std::basic_string<UniChar>& msg);
        /**
         *   Appends string to buffer.
         *   @param msg string to append.
         *   @return this buffer.
         */
        UniCharMessageBuffer& operator<<(const UniChar* msg);
        /**
         *   Appends string to buffer.
         *   @param msg string to append.
         *   @return this buffer.
         */
        UniCharMessageBuffer& operator<<(UniChar* msg);

        /**
         *   Appends character to buffer.
         *   @param msg character to append.
         *   @return this buffer.
         */
        UniCharMessageBuffer& operator<<(const UniChar msg);
        
#if LOG4CXX_CFSTRING_API
      /**
         *   Appends a string into the buffer and
         *   fixes the buffer to use char characters.
         *   @param msg message to append.
         *   @return encapsulated CharMessageBuffer.
         */
        UniCharMessageBuffer& operator<<(const CFStringRef& msg);
#endif        

        /**
         *   Insertion operator for STL manipulators such as std::fixed.
         *   @param manip manipulator.
         *   @return encapsulated STL stream.
         */
        uostream& operator<<(ios_base_manip manip);
        /**
         *   Insertion operator for built-in type.
         *   @param val build in type.
         *   @return encapsulated STL stream.
         */
        uostream& operator<<(bool val);

        /**
         *   Insertion operator for built-in type.
         *   @param val build in type.
         *   @return encapsulated STL stream.
         */
        uostream& operator<<(short val);
        /**
         *   Insertion operator for built-in type.
         *   @param val build in type.
         *   @return encapsulated STL stream.
         */
        uostream& operator<<(int val);
        /**
         *   Insertion operator for built-in type.
         *   @param val build in type.
         *   @return encapsulated STL stream.
         */
        uostream& operator<<(unsigned int val);
        /**
         *   Insertion operator for built-in type.
         *   @param val build in type.
         *   @return encapsulated STL stream.
         */
        uostream& operator<<(long val);
        /**
         *   Insertion operator for built-in type.
         *   @param val build in type.
         *   @return encapsulated STL stream.
         */
        uostream& operator<<(unsigned long val);
        /**
         *   Insertion operator for built-in type.
         *   @param val build in type.
         *   @return encapsulated STL stream.
         */
        uostream& operator<<(float val);
        /**
         *   Insertion operator for built-in type.
         *   @param val build in type.
         *   @return encapsulated STL stream.
         */
        uostream& operator<<(double val);
        /**
         *   Insertion operator for built-in type.
         *   @param val build in type.
         *   @return encapsulated STL stream.
         */
        uostream& operator<<(long double val);
        /**
         *   Insertion operator for built-in type.
         *   @param val build in type.
         *   @return encapsulated STL stream.
         */
        uostream& operator<<(void* val);


        /**
       *  Cast to ostream.
       */
      operator uostream&();

      /**
       *   Get content of buffer.
       *   @param os used only to signal that
       *       the embedded stream was used.
       */
      const std::basic_string<UniChar>& str(uostream& os);

      /**
       *   Get content of buffer.
       *   @param buf used only to signal that
       *       the embedded stream was not used.
       */
      const std::basic_string<UniChar>& str(UniCharMessageBuffer& buf);

        /**
         *  Returns true if buffer has an encapsulated STL stream.
         *  @return true if STL stream was created.
         */
        bool hasStream() const;

   private:
        /**
         * Prevent use of default copy constructor.
         */
      UniCharMessageBuffer(const UniCharMessageBuffer&);
        /**
         *   Prevent use of default assignment operator.  
         */
      UniCharMessageBuffer& operator=(const UniCharMessageBuffer&);

      /**
         * Encapsulated std::string.
         */
        std::basic_string<UniChar> buf;
        /**
         *  Encapsulated stream, created on demand.
         */
        std::basic_ostringstream<UniChar>* stream;
   };

template<class V>
UniCharMessageBuffer::uostream& operator<<(UniCharMessageBuffer& os, const V& val) {
   return ((UniCharMessageBuffer::uostream&) os) << val;
}
#endif

#if LOG4CXX_WCHAR_T_API
   /**
    *   This class is designed to support insertion operations
   *   in the message argument to the LOG4CXX_INFO and similar
   *   macros and is not designed for general purpose use.
   */
   class LOG4CXX_EXPORT WideMessageBuffer {
   public:
        /**
         *  Creates a new instance.
         */
       WideMessageBuffer();
        /**
         *  Destructor.
         */
        ~WideMessageBuffer();

        
        /**
         *   Appends string to buffer.
         *   @param msg string append.
         *   @return this buffer.
         */
        WideMessageBuffer& operator<<(const std::basic_string<wchar_t>& msg);
        /**
         *   Appends string to buffer.
         *   @param msg string to append.
         *   @return this buffer.
         */
        WideMessageBuffer& operator<<(const wchar_t* msg);
        /**
         *   Appends string to buffer.
         *   @param msg string to append.
         *   @return this buffer.
         */
        WideMessageBuffer& operator<<(wchar_t* msg);

        /**
         *   Appends character to buffer.
         *   @param msg character to append.
         *   @return this buffer.
         */
        WideMessageBuffer& operator<<(const wchar_t msg);

        /**
         *   Insertion operator for STL manipulators such as std::fixed.
         *   @param manip manipulator.
         *   @return encapsulated STL stream.
         */
        std::basic_ostream<wchar_t>& operator<<(ios_base_manip manip);
        /**
         *   Insertion operator for built-in type.
         *   @param val build in type.
         *   @return encapsulated STL stream.
         */
        std::basic_ostream<wchar_t>& operator<<(bool val);

        /**
         *   Insertion operator for built-in type.
         *   @param val build in type.
         *   @return encapsulated STL stream.
         */
        std::basic_ostream<wchar_t>& operator<<(short val);
        /**
         *   Insertion operator for built-in type.
         *   @param val build in type.
         *   @return encapsulated STL stream.
         */
        std::basic_ostream<wchar_t>& operator<<(int val);
        /**
         *   Insertion operator for built-in type.
         *   @param val build in type.
         *   @return encapsulated STL stream.
         */
        std::basic_ostream<wchar_t>& operator<<(unsigned int val);
        /**
         *   Insertion operator for built-in type.
         *   @param val build in type.
         *   @return encapsulated STL stream.
         */
        std::basic_ostream<wchar_t>& operator<<(long val);
        /**
         *   Insertion operator for built-in type.
         *   @param val build in type.
         *   @return encapsulated STL stream.
         */
        std::basic_ostream<wchar_t>& operator<<(unsigned long val);
        /**
         *   Insertion operator for built-in type.
         *   @param val build in type.
         *   @return encapsulated STL stream.
         */
        std::basic_ostream<wchar_t>& operator<<(float val);
        /**
         *   Insertion operator for built-in type.
         *   @param val build in type.
         *   @return encapsulated STL stream.
         */
        std::basic_ostream<wchar_t>& operator<<(double val);
        /**
         *   Insertion operator for built-in type.
         *   @param val build in type.
         *   @return encapsulated STL stream.
         */
        std::basic_ostream<wchar_t>& operator<<(long double val);
        /**
         *   Insertion operator for built-in type.
         *   @param val build in type.
         *   @return encapsulated STL stream.
         */
        std::basic_ostream<wchar_t>& operator<<(void* val);


        /**
       *  Cast to ostream.
       */
      operator std::basic_ostream<wchar_t>&();

      /**
       *   Get content of buffer.
       *   @param os used only to signal that
       *       the embedded stream was used.
       */
      const std::basic_string<wchar_t>& str(std::basic_ostream<wchar_t>& os);

      /**
       *   Get content of buffer.
       *   @param buf used only to signal that
       *       the embedded stream was not used.
       */
      const std::basic_string<wchar_t>& str(WideMessageBuffer& buf);

        /**
         *  Returns true if buffer has an encapsulated STL stream.
         *  @return true if STL stream was created.
         */
        bool hasStream() const;

   private:
        /**
         * Prevent use of default copy constructor.
         */
      WideMessageBuffer(const WideMessageBuffer&);
        /**
         *   Prevent use of default assignment operator.  
         */
      WideMessageBuffer& operator=(const WideMessageBuffer&);

      /**
         * Encapsulated std::string.
         */
        std::basic_string<wchar_t> buf;
        /**
         *  Encapsulated stream, created on demand.
         */
        std::basic_ostringstream<wchar_t>* stream;
   };

template<class V>
std::basic_ostream<wchar_t>& operator<<(WideMessageBuffer& os, const V& val) {
   return ((std::basic_ostream<wchar_t>&) os) << val;
}

   /**
    *   This class is used by the LOG4CXX_INFO and similar
    *   macros to support insertion operators in the message parameter.
    *   The class is not intended for use outside of that context.
    */
   class LOG4CXX_EXPORT MessageBuffer {
   public:
        /**
         *  Creates a new instance.
         */
      MessageBuffer();
      /**
         * Destructor.
         */
      ~MessageBuffer();

      /**
       *  Cast to ostream.
       */
      operator std::ostream&();

      /**
         *   Appends a string into the buffer and
         *   fixes the buffer to use char characters.
         *   @param msg message to append.
         *   @return encapsulated CharMessageBuffer.
         */
        CharMessageBuffer& operator<<(const std::string& msg);
        /**
         *   Appends a string into the buffer and
         *   fixes the buffer to use char characters.
         *   @param msg message to append.
         *   @return encapsulated CharMessageBuffer.
         */
        CharMessageBuffer& operator<<(const char* msg);
        /**
         *   Appends a string into the buffer and
         *   fixes the buffer to use char characters.
         *   @param msg message to append.
         *   @return encapsulated CharMessageBuffer.
         */
        CharMessageBuffer& operator<<(char* msg);

        /**
         *   Appends a string into the buffer and
         *   fixes the buffer to use char characters.
         *   @param msg message to append.
         *   @return encapsulated CharMessageBuffer.
         */
        CharMessageBuffer& operator<<(const char msg);

      /**
       *   Get content of buffer.
       *   @param buf used only to signal
       *       the character type and that
       *       the embedded stream was not used.
       */
      const std::string& str(CharMessageBuffer& buf);

      /**
       *   Get content of buffer.
       *   @param os used only to signal 
       *       the character type and that
       *       the embedded stream was used.
       */
      const std::string& str(std::ostream& os);

      /**
         *   Appends a string into the buffer and
         *   fixes the buffer to use char characters.
         *   @param msg message to append.
         *   @return encapsulated CharMessageBuffer.
         */
        WideMessageBuffer& operator<<(const std::wstring& msg);
        /**
         *   Appends a string into the buffer and
         *   fixes the buffer to use char characters.
         *   @param msg message to append.
         *   @return encapsulated CharMessageBuffer.
         */
        WideMessageBuffer& operator<<(const wchar_t* msg);
        /**
         *   Appends a string into the buffer and
         *   fixes the buffer to use char characters.
         *   @param msg message to append.
         *   @return encapsulated CharMessageBuffer.
         */
        WideMessageBuffer& operator<<(wchar_t* msg);
        /**
         *   Appends a string into the buffer and
         *   fixes the buffer to use char characters.
         *   @param msg message to append.
         *   @return encapsulated CharMessageBuffer.
         */
        WideMessageBuffer& operator<<(const wchar_t msg);

#if LOG4CXX_UNICHAR_API || LOG4CXX_CFSTRING_API
      /**
         *   Appends a string into the buffer and
         *   fixes the buffer to use char characters.
         *   @param msg message to append.
         *   @return encapsulated CharMessageBuffer.
         */
        UniCharMessageBuffer& operator<<(const std::basic_string<UniChar>& msg);
        /**
         *   Appends a string into the buffer and
         *   fixes the buffer to use char characters.
         *   @param msg message to append.
         *   @return encapsulated CharMessageBuffer.
         */
        UniCharMessageBuffer& operator<<(const UniChar* msg);
        /**
         *   Appends a string into the buffer and
         *   fixes the buffer to use char characters.
         *   @param msg message to append.
         *   @return encapsulated CharMessageBuffer.
         */
        UniCharMessageBuffer& operator<<(UniChar* msg);
        /**
         *   Appends a string into the buffer and
         *   fixes the buffer to use char characters.
         *   @param msg message to append.
         *   @return encapsulated CharMessageBuffer.
         */
        UniCharMessageBuffer& operator<<(const UniChar msg);
#endif

#if LOG4CXX_CFSTRING_API
      /**
         *   Appends a string into the buffer and
         *   fixes the buffer to use char characters.
         *   @param msg message to append.
         *   @return encapsulated CharMessageBuffer.
         */
        UniCharMessageBuffer& operator<<(const CFStringRef& msg);
#endif

        /**
         *   Insertion operator for STL manipulators such as std::fixed.
         *   @param manip manipulator.
         *   @return encapsulated STL stream.
         */
        std::ostream& operator<<(ios_base_manip manip);

        /**
         *   Insertion operator for built-in type.
         *   @param val build in type.
         *   @return encapsulated STL stream.
         */
        std::ostream& operator<<(bool val);

        /**
         *   Insertion operator for built-in type.
         *   @param val build in type.
         *   @return encapsulated STL stream.
         */
        std::ostream& operator<<(short val);
        /**
         *   Insertion operator for built-in type.
         *   @param val build in type.
         *   @return encapsulated STL stream.
         */
        std::ostream& operator<<(int val);
        /**
         *   Insertion operator for built-in type.
         *   @param val build in type.
         *   @return encapsulated STL stream.
         */
        std::ostream& operator<<(unsigned int val);
        /**
         *   Insertion operator for built-in type.
         *   @param val build in type.
         *   @return encapsulated STL stream.
         */
        std::ostream& operator<<(long val);
        /**
         *   Insertion operator for built-in type.
         *   @param val build in type.
         *   @return encapsulated STL stream.
         */
        std::ostream& operator<<(unsigned long val);
        /**
         *   Insertion operator for built-in type.
         *   @param val build in type.
         *   @return encapsulated STL stream.
         */
        std::ostream& operator<<(float val);
        /**
         *   Insertion operator for built-in type.
         *   @param val build in type.
         *   @return encapsulated STL stream.
         */
        std::ostream& operator<<(double val);
        /**
         *   Insertion operator for built-in type.
         *   @param val build in type.
         *   @return encapsulated STL stream.
         */
        std::ostream& operator<<(long double val);
        /**
         *   Insertion operator for built-in type.
         *   @param val build in type.
         *   @return encapsulated STL stream.
         */
        std::ostream& operator<<(void* val);
      /**
       *   Get content of buffer.
       *   @param buf used only to signal
       *       the character type and that
       *       the embedded stream was not used.
       */
      const std::wstring& str(WideMessageBuffer& buf);

      /**
       *   Get content of buffer.
       *   @param os used only to signal 
       *       the character type and that
       *       the embedded stream was used.
       */
      const std::wstring& str(std::basic_ostream<wchar_t>& os);
        
#if LOG4CXX_UNICHAR_API || LOG4CXX_CFSTRING_API
      /**
       *   Get content of buffer.
       *   @param buf used only to signal
       *       the character type and that
       *       the embedded stream was not used.
       */
      const std::basic_string<UniChar>& str(UniCharMessageBuffer& buf);

      /**
       *   Get content of buffer.
       *   @param os used only to signal 
       *       the character type and that
       *       the embedded stream was used.
       */
      const std::basic_string<UniChar>& str(UniCharMessageBuffer::uostream& os);
#endif        

        /**
         *  Returns true if buffer has an encapsulated STL stream.
         *  @return true if STL stream was created.
         */
        bool hasStream() const;

   private:
        /**
         * Prevent use of default copy constructor.
         */
        MessageBuffer(const MessageBuffer&);
        /**
         *   Prevent use of default assignment operator.  
         */
        MessageBuffer& operator=(const MessageBuffer&);

        /**
         *  Character message buffer.
         */
        CharMessageBuffer cbuf;

        /**
         * Encapsulated wide message buffer, created on demand.
         */
        WideMessageBuffer* wbuf;        
#if LOG4CXX_UNICHAR_API || LOG4CXX_CFSTRING_API
        /**
         * Encapsulated wide message buffer, created on demand.
         */
        UniCharMessageBuffer* ubuf;        
#endif        
   };

template<class V>
std::ostream& operator<<(MessageBuffer& os, const V& val) {
   return ((std::ostream&) os) << val;
}

#if LOG4CXX_LOGCHAR_IS_UTF8
typedef CharMessageBuffer LogCharMessageBuffer;
#endif

#if LOG4CXX_LOGCHAR_IS_WCHAR
typedef WideMessageBuffer LogCharMessageBuffer;
#endif

#if LOG4CXX_LOGCHAR_IS_UNICHAR
typedef UniCharMessageBuffer LogCharMessageBuffer;
#endif

#else
typedef CharMessageBuffer MessageBuffer;
typedef CharMessageBuffer LogCharMessageBuffer;
#endif

}}
#endif

