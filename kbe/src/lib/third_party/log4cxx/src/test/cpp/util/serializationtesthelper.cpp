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

#include "serializationtesthelper.h"
#include <log4cxx/helpers/bytearrayoutputstream.h>
#include <log4cxx/helpers/objectoutputstream.h>
#include <log4cxx/helpers/fileinputstream.h>
#include <log4cxx/helpers/bytebuffer.h>
#include <log4cxx/file.h>
#include "apr_pools.h"

using namespace log4cxx;
using namespace log4cxx::util;
using namespace log4cxx::helpers;
using namespace log4cxx::spi;



bool SerializationTestHelper::compare(
    const char* witness, const LoggingEventPtr& event, size_t endCompare)  {
    ByteArrayOutputStreamPtr memOut = new ByteArrayOutputStream();
    Pool p;
    ObjectOutputStream objOut(memOut, p);
    event->write(objOut, p);
    objOut.close(p);
    return compare(witness, memOut->toByteArray(), endCompare, p);
  }

  /**
   * Asserts the serialized form of an object.
   * @param witness file name of expected serialization.
   * @param actual byte array of actual serialization.
   * @param skip positions to skip comparison.
   * @param endCompare position to stop comparison.
   * @throws IOException thrown on IO or serialization exception.
   */
bool SerializationTestHelper::compare(
    const char* witness, const std::vector<unsigned char>& actual, 
    size_t endCompare, Pool& p) {
    File witnessFile(witness);

      char* expected = p.pstralloc(actual.size());
      FileInputStreamPtr is(new FileInputStream(witnessFile));
      ByteBuffer readBuffer(expected, actual.size());
      int bytesRead = is->read(readBuffer);
      is->close();

      if(bytesRead < endCompare) {
          puts("Witness file is shorter than expected");
          return false;
      }

      int endScan = actual.size();

      if (endScan > endCompare) {
        endScan = endCompare;
      }

      for (int i = 0; i < endScan; i++) {
          if (((unsigned char) expected[i]) != actual[i]) {
            printf("Difference at offset %d, expected %x, actual %x\n", i, expected[i], actual[i]);
            return false;
          }
        }
    return true;
  
}
