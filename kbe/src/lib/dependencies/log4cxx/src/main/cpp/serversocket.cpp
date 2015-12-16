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
 
#include <log4cxx/helpers/serversocket.h>
#include <log4cxx/helpers/synchronized.h>
#include "apr_network_io.h"
#include "apr_pools.h"
#include "apr_poll.h"

using namespace log4cxx::helpers;

/**  Creates a server socket on a specified port.
*/
ServerSocket::ServerSocket(int port) : pool(), mutex(pool), socket(0), timeout(0)
{
  apr_status_t status =
    apr_socket_create(&socket, APR_INET, SOCK_STREAM, 
                      APR_PROTO_TCP, pool.getAPRPool());
  if (status != APR_SUCCESS) {
    throw SocketException(status);
  }
  
  status = apr_socket_opt_set(socket, APR_SO_NONBLOCK, 1);
  if (status != APR_SUCCESS) {
    throw SocketException(status);
  }
  
        // Create server socket address (including port number)
  apr_sockaddr_t *server_addr;
  status =
      apr_sockaddr_info_get(&server_addr, NULL, APR_INET,
                                  port, 0, pool.getAPRPool());
  if (status != APR_SUCCESS) {
      throw ConnectException(status);
  }

  // bind the socket to the address
  status = apr_socket_bind(socket, server_addr);
  if (status != APR_SUCCESS) {
      throw BindException(status);
  }
  
  
  status = apr_socket_listen(socket, 50);
  if (status != APR_SUCCESS) {
    throw SocketException(status);
  }
}


ServerSocket::~ServerSocket()
{
}

void ServerSocket::close() {
    synchronized sync(mutex);
    if (socket != 0) {
        apr_status_t status = apr_socket_close(socket);
        if (status != APR_SUCCESS) {
            throw SocketException(status);
        }
        socket = 0;
    }
}

/** Listens for a connection to be made to this socket and
accepts it
*/
SocketPtr ServerSocket::accept() {
    synchronized sync(mutex);
    if (socket == 0) {
        throw IOException();
    }
    
    apr_pollfd_t poll;
    poll.p = pool.getAPRPool();
    poll.desc_type = APR_POLL_SOCKET;
    poll.reqevents = APR_POLLIN;
    poll.rtnevents = 0;
    poll.desc.s = socket;
    poll.client_data = NULL;
  
    apr_int32_t signaled;
    apr_interval_time_t to = timeout * 1000;
    apr_status_t status = apr_poll(&poll, 1, &signaled, to);  
  
    if (APR_STATUS_IS_TIMEUP(status)) {
        throw SocketTimeoutException();
    } else if (status != APR_SUCCESS) {
        throw SocketException(status);
    }
    
    apr_pool_t* newPool;
    status = apr_pool_create(&newPool, 0);
    if (status != APR_SUCCESS) {
        throw PoolException(status);
    }
    apr_socket_t* newSocket;
    status = apr_socket_accept(&newSocket, socket, newPool);
    if (status != APR_SUCCESS) {
        apr_pool_destroy(newPool);
        throw SocketException(status);
    }
    
    status = apr_socket_opt_set(newSocket, APR_SO_NONBLOCK, 0);
    if (status != APR_SUCCESS) {
        apr_pool_destroy(newPool);
        throw SocketException(status);
    }
    
    return new Socket(newSocket, newPool);
}

/** Retrive setting for SO_TIMEOUT.
*/
int ServerSocket::getSoTimeout() const
{
   return timeout;
}

/** Enable/disable SO_TIMEOUT with the specified timeout, in milliseconds.
*/
void ServerSocket::setSoTimeout(int newVal)
{
   timeout = newVal;
}
