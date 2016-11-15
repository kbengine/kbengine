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
#include <log4cxx/helpers/objectoutputstream.h>
#include <log4cxx/helpers/bytebuffer.h>
#include <log4cxx/helpers/outputstream.h>
#include <log4cxx/helpers/charsetencoder.h>
#include "apr_pools.h"

using namespace log4cxx;
using namespace log4cxx::helpers;

IMPLEMENT_LOG4CXX_OBJECT(ObjectOutputStream)

ObjectOutputStream::ObjectOutputStream(OutputStreamPtr outputStream, Pool& p)
     : os(outputStream) , 
       utf8Encoder(CharsetEncoder::getUTF8Encoder()), 
       objectHandle(0x7E0000),
       classDescriptions(new ClassDescriptionMap())
{
   char start[] = { (char)0xAC, (char)0xED, (char)0x00, (char)0x05 };
   ByteBuffer buf(start, sizeof(start));
   os->write(buf, p);
}

ObjectOutputStream::~ObjectOutputStream() {
    delete classDescriptions;
}

void ObjectOutputStream::close(Pool& p) {
    os->close(p);
}

void ObjectOutputStream::flush(Pool& p) {
    os->flush(p);
}

void ObjectOutputStream::writeObject(const LogString& val, Pool& p) {
   objectHandle++;
   writeByte(TC_STRING, p);
   char bytes[2];
#if LOG4CXX_LOGCHAR_IS_UTF8
    size_t len = val.size();
    ByteBuffer dataBuf(const_cast<char*>(val.data()), val.size()); 
#else
    size_t maxSize = 6 * val.size();
    char* data = p.pstralloc(maxSize);
    ByteBuffer dataBuf(data, maxSize);
    LogString::const_iterator iter(val.begin());
    utf8Encoder->encode(val, iter, dataBuf); 
    dataBuf.flip();
    size_t len = dataBuf.limit();
#endif
   bytes[1] = (char) (len & 0xFF);
   bytes[0] = (char) ((len >> 8) & 0xFF);
   ByteBuffer lenBuf(bytes, sizeof(bytes));
   os->write(lenBuf, p);
   os->write(dataBuf, p);
}


void ObjectOutputStream::writeObject(const MDC::Map& val, Pool& p) {
    //
    //  TC_OBJECT and the classDesc for java.util.Hashtable
    //
    char prolog[] = {
        (char)0x72, (char)0x00, (char)0x13, (char)0x6A, (char)0x61, (char)0x76, (char)0x61, 
        (char)0x2E, (char)0x75, (char)0x74, (char)0x69, (char)0x6C, (char)0x2E, (char)0x48, (char)0x61, 
        (char)0x73, (char)0x68, (char)0x74, (char)0x61, (char)0x62, (char)0x6C, (char)0x65, (char)0x13, 
        (char)0xBB, (char)0x0F, (char)0x25, (char)0x21, (char)0x4A, (char)0xE4, (char)0xB8, (char)0x03, 
        (char)0x00, (char)0x02, (char)0x46, (char)0x00, (char)0x0A, (char)0x6C, (char)0x6F, (char)0x61, 
        (char)0x64, (char)0x46, (char)0x61, (char)0x63, (char)0x74, (char)0x6F, (char)0x72, (char)0x49, 
        (char)0x00, (char)0x09, (char)0x74, (char)0x68, (char)0x72, (char)0x65, (char)0x73, (char)0x68, 
        (char)0x6F, (char)0x6C, (char)0x64, (char)0x78, (char)0x70  };
    writeProlog("java.util.Hashtable", 1, prolog, sizeof(prolog), p);
    //
    //   loadFactor = 0.75, threshold = 5, blockdata start, buckets.size = 7
    char data[] = { (char)0x3F, (char)0x40, (char)0x00, (char)0x00, (char)0x00, (char)0x00, (char)0x00, (char)0x05, 
        TC_BLOCKDATA, (char)0x08, (char)0x00, (char)0x00, (char)0x00, (char)0x07 };
    ByteBuffer dataBuf(data, sizeof(data));
    os->write(dataBuf, p);
    char size[4];
    size_t sz = val.size();
    size[3] = (char) (sz & 0xFF);
    size[2] = (char) ((sz >> 8) & 0xFF);
    size[1] = (char) ((sz >> 16) & 0xFF);
    size[0] = (char) ((sz >> 24) & 0xFF);
    ByteBuffer sizeBuf(size, sizeof(size));
    os->write(sizeBuf, p);
    for(MDC::Map::const_iterator iter = val.begin();
        iter != val.end();
        iter++) {
        writeObject(iter->first, p);
        writeObject(iter->second, p);
    }
    writeByte(TC_ENDBLOCKDATA, p);
}

void ObjectOutputStream::writeUTFString(const std::string& val, Pool& p) {
    char bytes[3];
    size_t len = val.size();
    ByteBuffer dataBuf(const_cast<char*>(val.data()), val.size()); 
   objectHandle++;
   bytes[0] = 0x74;
   bytes[1] = (char) ((len >> 8) & 0xFF);
   bytes[2] = (char) (len & 0xFF);
   ByteBuffer lenBuf(bytes, sizeof(bytes));
   os->write(lenBuf, p);
   os->write(dataBuf, p);
}



void ObjectOutputStream::writeByte(char val, Pool& p) {
   ByteBuffer buf(&val, 1);
   os->write(buf, p);
}

void ObjectOutputStream::writeInt(int val, Pool& p) {
   char bytes[4];
   bytes[3] = (char) (val & 0xFF);
   bytes[2] = (char) ((val >> 8) & 0xFF);
   bytes[1] = (char) ((val >> 16) & 0xFF);
   bytes[0] = (char) ((val >> 24) & 0xFF);
   ByteBuffer buf(bytes, sizeof(bytes));
   os->write(buf, p);
}

void ObjectOutputStream::writeLong(log4cxx_time_t val, Pool& p) {
   char bytes[8];
   bytes[7] = (char) (val & 0xFF);
   bytes[6] = (char) ((val >> 8) & 0xFF);
   bytes[5] = (char) ((val >> 16) & 0xFF);
   bytes[4] = (char) ((val >> 24) & 0xFF);
   bytes[3] = (char) ((val >> 32) & 0xFF);
   bytes[2] = (char) ((val >> 40) & 0xFF);
   bytes[1] = (char) ((val >> 48) & 0xFF);
   bytes[0] = (char) ((val >> 56) & 0xFF);
   ByteBuffer buf(bytes, sizeof(bytes));
   os->write(buf, p);
}

void ObjectOutputStream::writeBytes(const char* bytes, size_t len, Pool& p) {
   ByteBuffer buf(const_cast<char*>(bytes), len);
   os->write(buf, p);
}

void ObjectOutputStream::writeNull(Pool& p) {
   writeByte(TC_NULL, p);
}

void ObjectOutputStream::writeProlog(const char* className,
                        int classDescIncrement,
                        char* classDesc,
                        size_t len,
                        Pool& p) {
    ClassDescriptionMap::const_iterator match = classDescriptions->find(className);
    if (match != classDescriptions->end()) {
        char bytes[6];
        bytes[0] = TC_OBJECT;
        bytes[1] = TC_REFERENCE;
        bytes[2] = (char) ((match->second >> 24) & 0xFF);
        bytes[3] = (char) ((match->second >> 16) & 0xFF);
        bytes[4] = (char) ((match->second >> 8) & 0xFF);
        bytes[5] = (char) (match->second & 0xFF);
        ByteBuffer buf(bytes, sizeof(bytes));
        os->write(buf, p);
        objectHandle++;
    } else {
        classDescriptions->insert(ClassDescriptionMap::value_type(className, objectHandle));
        writeByte(TC_OBJECT, p);
        ByteBuffer buf(classDesc, len);
        os->write(buf, p);
        objectHandle += (classDescIncrement + 1);
    }
} 
