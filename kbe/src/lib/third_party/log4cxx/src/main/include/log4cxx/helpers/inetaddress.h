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

#ifndef _LOG4CXX_HELPER_INETADDRESS_H
#define _LOG4CXX_HELPER_INETADDRESS_H

#if defined(_MSC_VER)
#pragma warning ( push )
#pragma warning ( disable: 4231 4251 4275 4786 )
#endif



#include <log4cxx/helpers/objectimpl.h>
#include <log4cxx/helpers/objectptr.h>
#include <log4cxx/logstring.h>
#include <vector>
#include <log4cxx/helpers/exception.h>

namespace log4cxx
{
        namespace helpers
        {
                class LOG4CXX_EXPORT UnknownHostException : public Exception
                {
                public:
                      UnknownHostException(const LogString& msg);
                      UnknownHostException(const UnknownHostException& src);
                      UnknownHostException& operator=(const UnknownHostException& src);
                };


                class InetAddress;
                LOG4CXX_PTR_DEF(InetAddress);
                LOG4CXX_LIST_DEF(InetAddressList, InetAddressPtr);

                class LOG4CXX_EXPORT InetAddress : public ObjectImpl
                {
                public:
                        DECLARE_ABSTRACT_LOG4CXX_OBJECT(InetAddress)
                        BEGIN_LOG4CXX_CAST_MAP()
                                LOG4CXX_CAST_ENTRY(InetAddress)
                        END_LOG4CXX_CAST_MAP()

                        InetAddress(const LogString& hostName, const LogString& hostAddr);

                        /** Determines all the IP addresses of a host, given the host's name.
                        */
                        static InetAddressList getAllByName(const LogString& host);

                        /** Determines the IP address of a host, given the host's name.
                        */
                        static InetAddressPtr getByName(const LogString& host);

                        /** Returns the IP address string "%d.%d.%d.%d".
                        */
                        LogString getHostAddress() const;

                        /** Gets the host name for this IP address.
                        */
                        LogString getHostName() const;

                        /** Returns the local host.
                        */
                        static InetAddressPtr  getLocalHost();

                        /** Returns an InetAddress which can be used as any
                         *  address, for example when listening on a port from any
                         *  remote addresss.
                         */
                        static InetAddressPtr anyAddress();

                        /** Converts this IP address to a String.
                        */
                        LogString toString() const;

                private:
                        LogString ipAddrString;

                        LogString hostNameString;

                }; // class InetAddress
        }  // namespace helpers
} // namespace log4cxx

#if defined(_MSC_VER)
#pragma warning ( pop )
#endif


#endif // _LOG4CXX_HELPER_INETADDRESS_H

