/* Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#include "apr_strings.h"
#include "apr_pools.h"
#include "apr_general.h"
#include "apr_hash.h"
#include "apr_lib.h"
#include "apr_time.h"
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>

int main( int argc, char** argv) {
    apr_pool_t *context;
    regex_t regex;
    int rc;
    int i;
    int iters;
    apr_time_t now;
    apr_time_t end;
    apr_hash_t *h;
    

    if (argc !=4 ) {
            fprintf(stderr, "Usage %s match string #iterations\n",argv[0]);
            return -1;
    }
    iters = atoi( argv[3]);
    
    apr_initialize() ;
    atexit(apr_terminate);
    if (apr_pool_create(&context, NULL) != APR_SUCCESS) {
        fprintf(stderr, "Something went wrong\n");
        exit(-1);
    }
    rc = regcomp( &regex, argv[1], REG_EXTENDED|REG_NOSUB);


    if (rc) {
        char errbuf[2000];
        regerror(rc, &regex,errbuf,2000);
        fprintf(stderr,"Couldn't compile regex ;(\n%s\n ",errbuf);
        return -1;
    }
    if ( regexec( &regex, argv[2], 0, NULL,0) == 0 ) {
        fprintf(stderr,"Match\n");
    }
    else {
        fprintf(stderr,"No Match\n");
    }
    now = apr_time_now();
    for (i=0;i<iters;i++) {
        regexec( &regex, argv[2], 0, NULL,0) ;
    }
    end=apr_time_now();
    puts(apr_psprintf( context, "Time to run %d regex's          %8lld\n",iters,end-now));
    h = apr_hash_make( context);
    for (i=0;i<70;i++) {
            apr_hash_set(h,apr_psprintf(context, "%dkey",i),APR_HASH_KEY_STRING,"1");
    }
    now = apr_time_now();
    for (i=0;i<iters;i++) {
        apr_hash_get( h, argv[2], APR_HASH_KEY_STRING);
    }
    end=apr_time_now();
    puts(apr_psprintf( context, "Time to run %d hash (no find)'s %8lld\n",iters,end-now));
    apr_hash_set(h, argv[2],APR_HASH_KEY_STRING,"1");
    now = apr_time_now();
    for (i=0;i<iters;i++) {
        apr_hash_get( h, argv[2], APR_HASH_KEY_STRING);
    }
    end=apr_time_now();
    puts(apr_psprintf( context, "Time to run %d hash (find)'s    %8lld\n",iters,end-now));
 
    return 0;
}
