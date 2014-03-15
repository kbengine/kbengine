/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership. * The ASF licenses this file to You under the Apache License, Version 2.0
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

#include "binarycompare.h"
#include <apr_file_io.h>
#include <log4cxx/helpers/pool.h>
#include "../logunit.h"
#include <apr_pools.h>
#include <apr_strings.h>

using namespace log4cxx;
using namespace log4cxx::util;
using namespace log4cxx::helpers;

void BinaryCompare::compare(const char* filename1,
                            const char* filename2) {
    Pool p;
    apr_pool_t* pool = p.getAPRPool();
    apr_file_t* file1;
    apr_int32_t flags = APR_FOPEN_READ;
    apr_fileperms_t perm = APR_OS_DEFAULT;
    apr_status_t stat1 = apr_file_open(&file1,
        filename1, flags, perm, pool);
    if (stat1 != APR_SUCCESS) {
      LOGUNIT_FAIL(std::string("Unable to open ") + filename1);
    }

    apr_file_t* file2;
    apr_status_t stat2 = apr_file_open(&file2,
        filename2, flags, perm, pool);
    if (stat2 != APR_SUCCESS) {
      LOGUNIT_FAIL(std::string("Unable to open ") + filename2);
    }

    enum { BUFSIZE = 1024 };
    char* contents1 = (char*) apr_palloc(pool, BUFSIZE);
    char* contents2 = (char*) apr_palloc(pool, BUFSIZE);
    memset(contents1, 0, BUFSIZE);
    memset(contents2, 0, BUFSIZE);
    apr_size_t bytesRead1 = BUFSIZE;
    apr_size_t bytesRead2 = BUFSIZE;

    stat1 = apr_file_read(file1, contents1, &bytesRead1);
    if (stat1 != APR_SUCCESS) {
      LOGUNIT_FAIL(std::string("Unable to read ") + filename1);
    }

    stat2 = apr_file_read(file2, contents2, &bytesRead2);
    if (stat2 != APR_SUCCESS) {
      LOGUNIT_FAIL(std::string("Unable to read ") + filename2);
    }

    for (int i = 0; i < BUFSIZE; i++) {
       if (contents1[i] != contents2[i]) {
         std::string msg("Contents differ at position ");
         msg += apr_itoa(pool, i);
         msg += ": [";
         msg += filename1;
         msg += "] has ";
         msg += apr_itoa(pool, contents1[i]);
         msg += ", [";
         msg += filename2;
         msg += "] has ";
         msg += apr_itoa(pool, contents2[i]);
         msg += ".";
         LOGUNIT_FAIL(msg);
       }
    }
}
