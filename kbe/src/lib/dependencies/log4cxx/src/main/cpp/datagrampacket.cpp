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
#include <log4cxx/helpers/datagrampacket.h>

using namespace log4cxx::helpers;

IMPLEMENT_LOG4CXX_OBJECT(DatagramPacket)

/** Constructs a DatagramPacket for receiving packets of length
<code>length</code>. */
DatagramPacket::DatagramPacket(void * buf1, int length1)
: buf(buf1), offset(0), length(length1), address(), port(0)
{
}

/** Constructs a datagram packet for sending packets of length
<code>length/<code> to the specified port number on the specified
host. */
DatagramPacket::DatagramPacket(void * buf1, int length1, InetAddressPtr address1,
int port1)
: buf(buf1), offset(0), length(length1), address(address1), port(port1)
{
}

/** Constructs a DatagramPacket for receiving packets of length
<code>length</code>, specifying an offset into the buffer. */
DatagramPacket::DatagramPacket(void * buf1, int offset1, int length1)
: buf(buf1), offset(offset1), length(length1), address(), port(0)
{
}
/** Constructs a datagram packet for sending packets of length
<code>length</code> with offset <code>offset</code> to the
specified port number on the specified host. */
DatagramPacket::DatagramPacket(void * buf1, int offset1, int length1,
InetAddressPtr address1, int port1)
: buf(buf1), offset(offset1), length(length1), address(address1), port(port1)
{
}

DatagramPacket::~DatagramPacket()
{
}
