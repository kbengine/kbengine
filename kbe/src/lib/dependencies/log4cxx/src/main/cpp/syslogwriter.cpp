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
#if defined(_MSC_VER)
#pragma warning ( disable: 4231 4251 4275 4786 )
#endif

#include <log4cxx/logstring.h>
#include <log4cxx/helpers/syslogwriter.h>
#include <log4cxx/helpers/loglog.h>
#include <log4cxx/helpers/inetaddress.h>
#include <log4cxx/helpers/datagramsocket.h>
#include <log4cxx/helpers/datagrampacket.h>
#include <log4cxx/helpers/transcoder.h>

#define SYSLOG_PORT 514

using namespace log4cxx;
using namespace log4cxx::helpers;

SyslogWriter::SyslogWriter(const LogString& syslogHost1)
: syslogHost(syslogHost1)
{
   try
   {
      this->address = InetAddress::getByName(syslogHost1);
   }
   catch(UnknownHostException& e)
   {
      LogLog::error(((LogString) LOG4CXX_STR("Could not find ")) + syslogHost1 +
         LOG4CXX_STR(". All logging will FAIL."), e);
   }

   try
   {
      this->ds = new DatagramSocket();
   }
   catch (SocketException& e)
   {
      LogLog::error(((LogString) LOG4CXX_STR("Could not instantiate DatagramSocket to ")) + syslogHost1 +
            LOG4CXX_STR(". All logging will FAIL."), e);
   }
}

void SyslogWriter::write(const LogString& source) {
  if (this->ds != 0 && this->address != 0) {
      LOG4CXX_ENCODE_CHAR(data, source);

      DatagramPacketPtr packet( 
          new DatagramPacket((void*) data.data(), data.length(),
                             address, SYSLOG_PORT));

      ds->send(packet);
   }
}
