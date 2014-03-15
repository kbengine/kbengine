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

#ifndef _LOG4CXX_HELPERS_DATAGRAM_SOCKET_H
#define _LOG4CXX_HELPERS_DATAGRAM_SOCKET_H

#include <log4cxx/helpers/objectimpl.h>
#include <log4cxx/helpers/objectptr.h>
#include <log4cxx/helpers/inetaddress.h>
#include <log4cxx/helpers/pool.h>
#include <log4cxx/helpers/datagrampacket.h>

extern "C" { struct apr_socket_t; }

namespace log4cxx
{
        namespace helpers
        {
                /** This class represents a socket for sending and receiving
                datagram packets.*/
                class LOG4CXX_EXPORT DatagramSocket : public helpers::ObjectImpl
                {
                public:
                        DECLARE_ABSTRACT_LOG4CXX_OBJECT(DatagramSocket)
                        BEGIN_LOG4CXX_CAST_MAP()
                                LOG4CXX_CAST_ENTRY(DatagramSocket)
                        END_LOG4CXX_CAST_MAP()

                        /** Constructs a datagram socket and binds it to any available port
                        on the local host machine.*/
                        DatagramSocket();

                        /** Constructs a datagram socket and binds it to the specified
                        port on the local host machine. */
                        DatagramSocket(int port);

                        /**  Creates a datagram socket, bound to the specified local
                        address. */
                        DatagramSocket(int port, InetAddressPtr laddr);

                        /** ensure the socket is closed. */
                        ~DatagramSocket();

                        /**  Binds a datagram socket to a local port and address.*/
                        void bind(int lport, InetAddressPtr laddress);

                        /** Creates a datagram socket.*/
                        void create();

                        /** Closes this datagram socket */
                        void close();

                        /** Connects the socket to a remote address for this socket. */
                        void connect(InetAddressPtr address, int port);

                        /** Returns the address to which this socket is connected. */
                        inline InetAddressPtr getInetAddress() const
                                { return address; }

                        /** Gets the local address to which the socket is bound. */
                        inline InetAddressPtr getLocalAddress() const
                                { return localAddress; }

                        /**  Returns the port number on the local host to which this
                        socket is bound. */
                        inline int getLocalPort() const
                                { return localPort; }

                        /** Returns the port for this socket */
                        inline int getPort() const
                                { return port; }

                        /** Returns the binding state of the socket. **/
                        inline bool isBound() const
                                { return localPort != 0; }

                        /** Returns wether the socket is closed or not. */
                        inline bool isClosed() const
                                { return socket != 0; }

                        /** Returns the connection state of the socket. */
                        inline bool isConnected() const
                                { return port != 0; }

                        /**  Receives a datagram packet from this socket. */
                        void receive(DatagramPacketPtr& p);

                        /** Sends a datagram packet from this socket. */
                        void  send(DatagramPacketPtr& p);

                private:
                        DatagramSocket(const DatagramSocket&);
                        DatagramSocket& operator=(const DatagramSocket&);
                        /** The APR socket */
                        apr_socket_t *socket;

                        /** The memory pool for the socket */
                        Pool socketPool;

                        InetAddressPtr address;

                        InetAddressPtr localAddress;

                        int port;

                        /** The local port number to which this socket is connected. */
                        int localPort;

                };
            LOG4CXX_PTR_DEF(DatagramSocket);
        }  // namespace helpers
} // namespace log4cxx

#endif //_LOG4CXX_HELPERS_DATAGRAM_SOCKET_H
