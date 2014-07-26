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
#include <log4cxx/helpers/system.h>

#include <log4cxx/helpers/transcoder.h>
#include <log4cxx/helpers/pool.h>
#include <apr_file_io.h>
#include <apr_user.h>
#include <apr_env.h>


using namespace log4cxx;
using namespace log4cxx::helpers;


LogString System::getProperty(const LogString& lkey)
{
        if (lkey.empty())
        {
                throw IllegalArgumentException(LOG4CXX_STR("key is empty"));
        }

        LogString rv;
        if (lkey == LOG4CXX_STR("java.io.tmpdir")) {
          Pool p;
          const char* dir = NULL;
          apr_status_t stat = apr_temp_dir_get(&dir, p.getAPRPool());
          if (stat == APR_SUCCESS) {
            Transcoder::decode(dir, rv);
          }
          return rv;
        }

        if (lkey == LOG4CXX_STR("user.dir")) {
          Pool p;
          char* dir = NULL;
          apr_status_t stat = apr_filepath_get(&dir, APR_FILEPATH_NATIVE,
              p.getAPRPool());
          if (stat == APR_SUCCESS) {
            Transcoder::decode(dir, rv);
          }
          return rv;
        }
#if APR_HAS_USER
        if (lkey == LOG4CXX_STR("user.home") || lkey == LOG4CXX_STR("user.name")) {
          Pool pool;
          apr_uid_t userid;
          apr_gid_t groupid;
          apr_pool_t* p = pool.getAPRPool();
          apr_status_t stat = apr_uid_current(&userid, &groupid, p);
          if (stat == APR_SUCCESS) {
            char* username = NULL;
            stat = apr_uid_name_get(&username, userid, p);
            if (stat == APR_SUCCESS) {
              if (lkey == LOG4CXX_STR("user.name")) {
                Transcoder::decode(username, rv);
              } else {
                char* dirname = NULL;
                stat = apr_uid_homepath_get(&dirname, username, p);
                if (stat == APR_SUCCESS) {
                  Transcoder::decode(dirname, rv);
                }
              }
            }
          }
          return rv;
        }
#endif

        LOG4CXX_ENCODE_CHAR(key, lkey);
        Pool p;
        char* value = NULL;
        apr_status_t stat = apr_env_get(&value, key.c_str(), 
            p.getAPRPool());
        if (stat == APR_SUCCESS) {
             Transcoder::decode((const char*) value, rv);
        }
        return rv;
}

