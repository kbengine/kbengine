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
#include <log4cxx/logstring.h>
#include <log4cxx/helpers/datagramsocket.h>
#include <log4cxx/helpers/datagrampacket.h>
#include <log4cxx/helpers/loglog.h>
#include <log4cxx/helpers/transcoder.h>

#include "apr_network_io.h"
#include "apr_lib.h"

using namespace log4cxx::helpers;

IMPLEMENT_LOG4CXX_OBJECT(DatagramSocket)

DatagramSocket::DatagramSocket()
 : socket(0), address(), localAddress(), port(0), localPort(0)
{
   create();
}

DatagramSocket::DatagramSocket(int localPort1)
 : socket(0), address(), localAddress(), port(0), localPort(0)
{
   InetAddressPtr bindAddr = InetAddress::anyAddress();

   create();
   bind(localPort1, bindAddr);
}

DatagramSocket::DatagramSocket(int localPort1, InetAddressPtr localAddress1)
 : socket(0), address(), localAddress(), port(0), localPort(0)
{
   create();
   bind(localPort1, localAddress1);
}

DatagramSocket::~DatagramSocket()
{
   try
   {
      close();
   }
   catch(SocketException&)
   {
   }
}

/**  Binds a datagram socket to a local port and address.*/
void DatagramSocket::bind(int localPort1, InetAddressPtr localAddress1)
{
   Pool addrPool;

   // Create server socket address (including port number)
   LOG4CXX_ENCODE_CHAR(hostAddr, localAddress1->getHostAddress());
   apr_sockaddr_t *server_addr;
   apr_status_t status =
       apr_sockaddr_info_get(&server_addr, hostAddr.c_str(), APR_INET,
                             localPort1, 0, addrPool.getAPRPool());
   if (status != APR_SUCCESS) {
     throw BindException(status);
   }

   // bind the socket to the address
   status = apr_socket_bind(socket, server_addr);
   if (status != APR_SUCCESS) {
     throw BindException(status);
   }

   this->localPort = localPort1;
   this->localAddress = localAddress1;
}

/** Close the socket.*/
void DatagramSocket::close()
{
   if (socket != 0) {
      apr_status_t status = apr_socket_close(socket);
      if (status != APR_SUCCESS) {
        throw SocketException(status);
      }

      socket = 0;
      localPort = 0;
   }
}

void DatagramSocket::connect(InetAddressPtr address1, int port1)
{

   this->address = address1;
   this->port = port1;

   Pool addrPool;

   // create socket address
   LOG4CXX_ENCODE_CHAR(hostAddr, address1->getHostAddress());
   apr_sockaddr_t *client_addr;
   apr_status_t status =
       apr_sockaddr_info_get(&client_addr, hostAddr.c_str(), APR_INET,
                             port, 0, addrPool.getAPRPool());
   if (status != APR_SUCCESS) {
     throw ConnectException(status);
   }

   // connect the socket
   status = apr_socket_connect(socket, client_addr);
   if (status != APR_SUCCESS) {
     throw ConnectException(status);
   }
}

/** Creates a datagram socket.*/
void DatagramSocket::create()
{
  apr_socket_t* newSocket;
  apr_status_t status =
    apr_socket_create(&newSocket, APR_INET, SOCK_DGRAM,
                      APR_PROTO_UDP, socketPool.getAPRPool());
  socket = newSocket;
  if (status != APR_SUCCESS) {
    throw SocketException(status);
  }
}

/** Receive the datagram packet.*/
void DatagramSocket::receive(DatagramPacketPtr& p)
{
   Pool addrPool;

   // Create the address from which to receive the datagram packet
   LOG4CXX_ENCODE_CHAR(hostAddr, p->getAddress()->getHostAddress());
   apr_sockaddr_t *addr;
   apr_status_t status =
       apr_sockaddr_info_get(&addr, hostAddr.c_str(), APR_INET,
                             p->getPort(), 0, addrPool.getAPRPool());
   if (status != APR_SUCCESS) {
     throw SocketException(status);
   }

   // receive the datagram packet
   apr_size_t len = p->getLength();
   status = apr_socket_recvfrom(addr, socket, 0,
                                (char *)p->getData(), &len);
   if (status != APR_SUCCESS) {
     throw IOException(status);
   }
}

/**  Sends a datagram packet.*/
void DatagramSocket::send(DatagramPacketPtr& p)
{
   Pool addrPool;

   // create the adress to which to send the datagram packet
   LOG4CXX_ENCODE_CHAR(hostAddr, p->getAddress()->getHostAddress());
   apr_sockaddr_t *addr;
   apr_status_t status =
       apr_sockaddr_info_get(&addr, hostAddr.c_str(), APR_INET, p->getPort(),
                             0, addrPool.getAPRPool());
   if (status != APR_SUCCESS) {
     throw SocketException(status);
   }

   // send the datagram packet
   apr_size_t len = p->getLength();
   status = apr_socket_sendto(socket, addr, 0,
                              (char *)p->getData(), &len);
   if (status != APR_SUCCESS) {
     throw IOException(status);
   }
}
