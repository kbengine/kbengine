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

#include "transformer.h"
#include <log4cxx/file.h>
#include <log4cxx/helpers/transcoder.h>
#include <apr_thread_proc.h>
#include <apr_pools.h>
#include <apr_file_io.h>
#include <apr_strings.h>
#include <assert.h>
#include <iostream>

using namespace log4cxx;
using namespace log4cxx::helpers;

#if !defined(APR_FOPEN_READ)
#define APR_FOPEN_READ APR_READ
#define APR_FOPEN_CREATE APR_CREATE
#define APR_FOPEN_WRITE APR_WRITE
#define APR_FOPEN_TRUNCATE APR_TRUNCATE
#define APR_FOPEN_APPEND APR_APPEND
#endif

void Transformer::transform(const File& in, const File& out,
        const std::vector<Filter *>& filters)
{
     log4cxx::Filter::PatternList patterns;
     for(std::vector<Filter*>::const_iterator iter = filters.begin();
         iter != filters.end();
         iter++) {

         const log4cxx::Filter::PatternList& thesePatterns = (*iter)->getPatterns();
         for (log4cxx::Filter::PatternList::const_iterator pattern = thesePatterns.begin();
              pattern != thesePatterns.end();
              pattern++) {
              patterns.push_back(*pattern);
         }
     }
     transform(in, out, patterns);
}

void Transformer::transform(const File& in, const File& out,
        const Filter& filter)
{
    transform(in, out, filter.getPatterns());
}


void Transformer::copyFile(const File& in, const File& out) {
       Pool p;
       apr_pool_t* pool = p.getAPRPool();


        //
        //    fairly naive file copy code
        //
        //
        apr_file_t* child_out;
        apr_int32_t flags = APR_FOPEN_WRITE | APR_FOPEN_CREATE | APR_FOPEN_TRUNCATE;
        apr_status_t stat = out.open(&child_out, flags, APR_OS_DEFAULT, p);
        assert(stat == APR_SUCCESS);

        apr_file_t* in_file;
        stat = in.open(&in_file, APR_FOPEN_READ, APR_OS_DEFAULT, p);
        assert(stat == APR_SUCCESS);
        apr_size_t bufsize = 32000;
        void* buf = apr_palloc(pool, bufsize);
        apr_size_t bytesRead = bufsize;

        while(stat == 0 && bytesRead == bufsize) {
            stat = apr_file_read(in_file, buf, &bytesRead);
            if (stat == 0 && bytesRead > 0) {
                stat = apr_file_write(child_out, buf, &bytesRead);
                assert(stat == APR_SUCCESS);
            }
        }
        stat = apr_file_close(child_out);
        assert(stat == APR_SUCCESS);
}

void Transformer::createSedCommandFile(const std::string& regexName,
        const log4cxx::Filter::PatternList& patterns,
        apr_pool_t* pool) {
        apr_file_t* regexFile;
        apr_status_t stat = apr_file_open(&regexFile,
               regexName.c_str(),
               APR_FOPEN_WRITE | APR_FOPEN_CREATE | APR_FOPEN_TRUNCATE, APR_OS_DEFAULT,
               pool);
        assert(stat == APR_SUCCESS);

        std::string tmp;
        for (log4cxx::Filter::PatternList::const_iterator iter = patterns.begin();
          iter != patterns.end();
          iter++) {
          tmp = "sQ";
          tmp.append(iter->first);
          tmp.append(1, 'Q');
          tmp.append(iter->second);
          tmp.append("Qg\n");
          apr_file_puts(tmp.c_str(), regexFile);
        }
        apr_file_close(regexFile);
}

void Transformer::transform(const File& in, const File& out,
        const log4cxx::Filter::PatternList& patterns)
{
    //
    //   if no patterns just copy the file
    //
    if (patterns.size() == 0) {
        copyFile(in, out);
    } else {
   Pool p;
        apr_pool_t* pool = p.getAPRPool();
 
        //
        //   write the regex's to a temporary file since they
        //      may get mangled if passed as parameters
        //
        std::string regexName;
        Transcoder::encode(in.getPath(), regexName);
        regexName.append(".sed");
        createSedCommandFile(regexName, patterns, pool);


        //
        //  prepare to launch sed
        //
        //
        apr_procattr_t* attr = NULL;
        apr_status_t stat = apr_procattr_create(&attr, pool);
        assert(stat == APR_SUCCESS);

        stat = apr_procattr_io_set(attr, APR_NO_PIPE, APR_FULL_BLOCK,
                             APR_FULL_BLOCK);
        assert(stat == APR_SUCCESS);

        //
        //   find the program on the path
        //
        stat = apr_procattr_cmdtype_set(attr, APR_PROGRAM_PATH);
        assert(stat == APR_SUCCESS);

        //
        //   build the argument list
        //      using Q as regex separator on s command
        //
        const char** args = (const char**)
          apr_palloc(pool, 5 * sizeof(*args));
        int i = 0;

        //
        //   not well documented
        //     but the first arg is a duplicate of the executable name
        //
        args[i++] = "sed";


        std::string regexArg("-f");
        regexArg.append(regexName);
        args[i++] = apr_pstrdup(pool, regexArg.c_str());

        //
        //    specify the input file
        args[i++] = Transcoder::encode(in.getPath(), p);
        args[i] = NULL;



        //
        //    set the output stream to the filtered file
        //
        apr_file_t* child_out;
        apr_int32_t flags = APR_FOPEN_READ | APR_FOPEN_WRITE |
            APR_FOPEN_CREATE | APR_FOPEN_TRUNCATE;
        stat = out.open(&child_out, flags, APR_OS_DEFAULT, p);
        assert(stat == APR_SUCCESS);

        stat =  apr_procattr_child_out_set(attr, child_out, NULL);
        assert(stat == APR_SUCCESS);

        //
        //   redirect the child's error stream to this processes' error stream
        //
        apr_file_t* child_err;
        stat = apr_file_open_stderr(&child_err, pool);
        assert(stat == 0);
        stat =  apr_procattr_child_err_set(attr, child_err, NULL);
        assert(stat == APR_SUCCESS);



        apr_proc_t pid;
        stat = apr_proc_create(&pid,"sed", args, NULL, attr, pool);
        if (stat != APR_SUCCESS) {
            puts("Error invoking sed, sed must be on the path in order to run unit tests");
        }
        assert(stat == APR_SUCCESS);

        apr_proc_wait(&pid, NULL, NULL, APR_WAIT);
        stat = apr_file_close(child_out);
        assert(stat == APR_SUCCESS);
     }


}
