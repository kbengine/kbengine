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

#ifndef _LOG4CXX_HELPERS_EXCEPTION_H
#define _LOG4CXX_HELPERS_EXCEPTION_H

#include <exception>
#include <log4cxx/log4cxx.h>
#include <log4cxx/logstring.h>

namespace log4cxx
{
        namespace helpers
        {
                /** The class Exception and its subclasses indicate conditions that a
                reasonable application might want to catch.
                */
                class LOG4CXX_EXPORT Exception : public ::std::exception
                {
                public:
                        Exception(const char* msg);
                        Exception(const LogString& msg);
                        Exception(const Exception& src);
                        Exception& operator=(const Exception& src);
                        const char* what() const throw();
                private:
                        enum { MSG_SIZE = 128 };
                        char msg[MSG_SIZE + 1];
                }; // class Exception

                /** RuntimeException is the parent class of those exceptions that can be
                thrown during the normal operation of the process.
                */
                class LOG4CXX_EXPORT RuntimeException : public Exception
                {
                public:
                        RuntimeException(log4cxx_status_t stat);
                        RuntimeException(const LogString& msg);
                        RuntimeException(const RuntimeException& msg);
                        RuntimeException& operator=(const RuntimeException& src);
                private:
                        static LogString formatMessage(log4cxx_status_t stat);
                }; // class RuntimeException

                /** Thrown when an application attempts to use null in a case where an
                object is required.
                */
                class LOG4CXX_EXPORT  NullPointerException : public RuntimeException
                {
                public:
                        NullPointerException(const LogString& msg);
                        NullPointerException(const NullPointerException& msg);
                        NullPointerException& operator=(const NullPointerException& src);
                }; // class NullPointerException

                /** Thrown to indicate that a method has been passed
                an illegal or inappropriate argument.*/
                class LOG4CXX_EXPORT IllegalArgumentException : public RuntimeException
                {
                public:
                   IllegalArgumentException(const LogString& msg);
                   IllegalArgumentException(const IllegalArgumentException&);
                   IllegalArgumentException& operator=(const IllegalArgumentException&);
                }; // class IllegalArgumentException

                /** Signals that an I/O exception of some sort has occurred. This class
                is the general class of exceptions produced by failed or interrupted
                I/O operations.
                */
                class LOG4CXX_EXPORT IOException : public Exception
                {
                public:
                    IOException();
                    IOException(log4cxx_status_t stat);
                    IOException(const LogString& msg);
                    IOException(const IOException &src);
                    IOException& operator=(const IOException&);
                private:
                    static LogString formatMessage(log4cxx_status_t stat);
                };

                class LOG4CXX_EXPORT MissingResourceException : public Exception
                {
                    public:
                    MissingResourceException(const LogString& key);
                    MissingResourceException(const MissingResourceException &src);
                    MissingResourceException& operator=(const MissingResourceException&);
                private:
                    static LogString formatMessage(const LogString& key);
                };

                class LOG4CXX_EXPORT PoolException : public Exception
                {
                public:
                      PoolException(log4cxx_status_t stat);
                      PoolException(const PoolException &src);
                      PoolException& operator=(const PoolException&);
                private:
                      static LogString formatMessage(log4cxx_status_t stat);
                };


                class LOG4CXX_EXPORT MutexException : public Exception
                {
                public:
                      MutexException(log4cxx_status_t stat);
                      MutexException(const MutexException &src);
                      MutexException& operator=(const MutexException&);
                private:
                      static LogString formatMessage(log4cxx_status_t stat);
                };

                class LOG4CXX_EXPORT InterruptedException : public Exception
                {
                public:
                      InterruptedException();
                      InterruptedException(log4cxx_status_t stat);
                      InterruptedException(const InterruptedException &src);
                      InterruptedException& operator=(const InterruptedException&);
                private:
                      static LogString formatMessage(log4cxx_status_t stat);
                };

                class LOG4CXX_EXPORT ThreadException
                      : public Exception {
                public:
                      ThreadException(log4cxx_status_t stat);
                      ThreadException(const LogString& msg);
                      ThreadException(const ThreadException &src);
                      ThreadException& operator=(const ThreadException&);
                private:
                      static LogString formatMessage(log4cxx_status_t stat);
                };

                class LOG4CXX_EXPORT TranscoderException : public Exception
                {
                public:
                      TranscoderException(log4cxx_status_t stat);
                      TranscoderException(const TranscoderException &src);
                      TranscoderException& operator=(const TranscoderException&);
                private:
                      static LogString formatMessage(log4cxx_status_t stat);
                };

                class LOG4CXX_EXPORT IllegalMonitorStateException
                      : public Exception {
                public:
                      IllegalMonitorStateException(const LogString& msg);
                      IllegalMonitorStateException(const IllegalMonitorStateException& msg);
                      IllegalMonitorStateException& operator=(const IllegalMonitorStateException& msg);
                };

                /**
                Thrown when an application tries to create an instance of a class using
                the newInstance method in class Class, but the specified class object
                cannot be instantiated because it is an interface or is an abstract class.
                */
                class LOG4CXX_EXPORT InstantiationException : public Exception
                {
                public:
                        InstantiationException(const LogString& msg);
                        InstantiationException(const InstantiationException& msg);
                        InstantiationException& operator=(const InstantiationException& msg);
                };

                /**
                Thrown when an application tries to load in a class through its
                string name but no definition for the class with the specified name
                could be found.
                */
                class LOG4CXX_EXPORT ClassNotFoundException : public Exception
                {
                public:
                    ClassNotFoundException(const LogString& className);
                    ClassNotFoundException(const ClassNotFoundException& msg);
                    ClassNotFoundException& operator=(const ClassNotFoundException& msg);
                private:
                    static LogString formatMessage(const LogString& className);
                };


                class NoSuchElementException : public Exception {
                public:
                      NoSuchElementException();
                      NoSuchElementException(const NoSuchElementException&);
                      NoSuchElementException& operator=(const NoSuchElementException&);
                };

                class IllegalStateException : public Exception {
                public:
                      IllegalStateException();
                      IllegalStateException(const IllegalStateException&);
                      IllegalStateException& operator=(const IllegalStateException&);
                };

                /** Thrown to indicate that there is an error in the underlying
                protocol, such as a TCP error.
                */
                class LOG4CXX_EXPORT SocketException : public IOException
                {
                public:
                        SocketException(const LogString& msg);
                        SocketException(log4cxx_status_t status);
                        SocketException(const SocketException&);
                        SocketException& operator=(const SocketException&);
                };

                /** Signals that an error occurred while attempting to connect a socket
                to a remote address and port. Typically, the connection was refused
                remotely (e.g., no process is listening on the remote address/port).
                */
                class LOG4CXX_EXPORT ConnectException : public SocketException
                {
                public:
                    ConnectException(log4cxx_status_t status);
                    ConnectException(const ConnectException& src);
                   ConnectException& operator=(const ConnectException&);
                };

                class LOG4CXX_EXPORT ClosedChannelException : public SocketException
                {
                public:
                    ClosedChannelException();
                    ClosedChannelException(const ClosedChannelException& src);
                    ClosedChannelException& operator=(const ClosedChannelException&);
                };

                /** Signals that an error occurred while attempting to bind a socket to
                a local address and port. Typically, the port is in use, or the
                requested local address could not be assigned.
                */
                class LOG4CXX_EXPORT BindException : public SocketException
                {
                public:
                      BindException(log4cxx_status_t status);
                      BindException(const BindException&);
                      BindException& operator=(const BindException&);
                };
               
                /** Signals that an I/O operation has been interrupted. An
                InterruptedIOException is thrown to indicate that an input or output
                transfer has been terminated because the thread performing it was
                interrupted. The field bytesTransferred  indicates how many bytes were
                successfully transferred before the interruption occurred.
                */
                class LOG4CXX_EXPORT InterruptedIOException : public IOException
                {
                public:
                     InterruptedIOException(const LogString& msg);
                     InterruptedIOException(const InterruptedIOException&);
                     InterruptedIOException& operator=(const InterruptedIOException&);
                };

               
                /** Signals that an I/O operation has been interrupted. An
                InterruptedIOException is thrown to indicate that an input or output
                transfer has been terminated because the thread performing it was
                interrupted. The field bytesTransferred  indicates how many bytes were
                successfully transferred before the interruption occurred.
                */
                class LOG4CXX_EXPORT SocketTimeoutException : public InterruptedIOException
                {
                public:
                     SocketTimeoutException();
                     SocketTimeoutException(const SocketTimeoutException&);
                     SocketTimeoutException& operator=(const SocketTimeoutException&);
                };
                 

        }  // namespace helpers
} // namespace log4cxx

#endif // _LOG4CXX_HELPERS_EXCEPTION_H
