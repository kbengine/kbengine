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

/* sockperf.c
 * This simple network client tries to connect to an echo daemon (echod)
 * listening on a port it supplies, then time how long it takes to
 * reply with packets of varying sizes.
 * It prints results once completed.
 *
 * To run,
 *
 *   ./echod &
 *   ./sockperf
 */

#include <stdio.h>
#include <stdlib.h>  /* for atexit() */

#include "apr.h"
#include "apr_network_io.h"
#include "apr_strings.h"

#define MAX_ITERS    10
#define TEST_SIZE  1024

struct testSet {
    char c;
    apr_size_t size;
    int iters;
} testRuns[] = {
    { 'a', 1, 3 },
    { 'b', 4, 3 },
    { 'c', 16, 5 },
    { 'd', 64, 5 },
    { 'e', 256, 10 },
};

struct testResult {
    int size;
    int iters;
    apr_time_t msecs[MAX_ITERS];
    apr_time_t avg;
};

static apr_int16_t testPort = 4747;
static apr_sockaddr_t *sockAddr = NULL;

static void reportError(const char *msg, apr_status_t rv, 
                        apr_pool_t *pool)
{
    fprintf(stderr, "%s\n", msg);
    if (rv != APR_SUCCESS)
        fprintf(stderr, "Error: %d\n'%s'\n", rv,
                apr_psprintf(pool, "%pm", &rv));
    
}

static void closeConnection(apr_socket_t *sock)
{
    apr_size_t len = 0;
    apr_socket_send(sock, NULL, &len);
}

static apr_status_t sendRecvBuffer(apr_time_t *t, const char *buf, 
                                   apr_size_t size, apr_pool_t *pool)
{
    apr_socket_t *sock;
    apr_status_t rv;
    apr_size_t len = size, thistime = size;
    char *recvBuf;
    apr_time_t testStart = apr_time_now(), testEnd;
    int i;

    if (! sockAddr) {
        rv = apr_sockaddr_info_get(&sockAddr, "127.0.0.1", APR_UNSPEC,
                                   testPort, 0, pool);
        if (rv != APR_SUCCESS) {
            reportError("Unable to get socket info", rv, pool);
            return rv;
        }

        /* make sure we can connect to daemon before we try tests */

        rv = apr_socket_create(&sock, APR_INET, SOCK_STREAM, APR_PROTO_TCP,
                           pool);
        if (rv != APR_SUCCESS) {
            reportError("Unable to create IPv4 stream socket", rv, pool);
            return rv;
        }

        rv = apr_socket_connect(sock, sockAddr);
        if (rv != APR_SUCCESS) {
            reportError("Unable to connect to echod!", rv, pool);
            apr_socket_close(sock);
            return rv;
        }
        apr_socket_close(sock);

    }

    recvBuf = apr_palloc(pool, size);
    if (! recvBuf) {
        reportError("Unable to allocate buffer", ENOMEM, pool);
        return ENOMEM;
    }

    *t = 0;

    /* START! */
    testStart = apr_time_now();
    rv = apr_socket_create(&sock, APR_INET, SOCK_STREAM, APR_PROTO_TCP,
                           pool);
    if (rv != APR_SUCCESS) {
        reportError("Unable to create IPv4 stream socket", rv, pool);
        return rv;
    }

    rv = apr_socket_connect(sock, sockAddr);
    if (rv != APR_SUCCESS) {
        reportError("Unable to connect to echod!", rv, pool);
        apr_socket_close(sock);
        return rv;
    }

    for (i = 0; i < 3; i++) {

        len = size;
        thistime = size;

        rv = apr_socket_send(sock, buf, &len);
        if (rv != APR_SUCCESS || len != size) {
            reportError(apr_psprintf(pool, 
                         "Unable to send data correctly (iteration %d of 3)",
                         i) , rv, pool);
            closeConnection(sock);
            apr_socket_close(sock);
            return rv;
        }
    
        do {
            len = thistime;
            rv = apr_socket_recv(sock, &recvBuf[size - thistime], &len);
            if (rv != APR_SUCCESS) {
                reportError("Error receiving from socket", rv, pool);
                break;
            }
            thistime -= len;
        } while (thistime);
    }

    closeConnection(sock);
    apr_socket_close(sock);
    testEnd = apr_time_now();
    /* STOP! */

    if (thistime) {
        reportError("Received less than we sent :-(", rv, pool);
        return rv;
    }        
    if (strncmp(recvBuf, buf, size) != 0) {
        reportError("Received corrupt data :-(", 0, pool);
        printf("We sent:\n%s\nWe received:\n%s\n", buf, recvBuf);
        return EINVAL;
    }
    *t = testEnd - testStart;
    return APR_SUCCESS;
}

static apr_status_t runTest(struct testSet *ts, struct testResult *res,
                            apr_pool_t *pool)
{
    char *buffer;
    apr_status_t rv;
    int i;
    apr_size_t sz = ts->size * TEST_SIZE;
    
    buffer = apr_palloc(pool, sz);
    if (!buffer) {
        reportError("Unable to allocate buffer", ENOMEM, pool);
        return ENOMEM;
    }
    memset(buffer, ts->c, sz);

    res->iters = ts->iters > MAX_ITERS ? MAX_ITERS : ts->iters;

    for (i = 0; i < res->iters; i++) {
        apr_time_t iterTime;
        rv = sendRecvBuffer(&iterTime, buffer, sz, pool);
        if (rv != APR_SUCCESS) {
            res->iters = i;
            break;
        }
        res->msecs[i] = iterTime;
    }

    return rv;
}

int main(int argc, char **argv)
{
    apr_pool_t *pool;
    apr_status_t rv;
    int i;
    int nTests = sizeof(testRuns) / sizeof(testRuns[0]);
    struct testResult *results;

    printf("APR Test Application: sockperf\n");

    apr_initialize();
    atexit(apr_terminate);

    apr_pool_create(&pool, NULL);

    results = (struct testResult *)apr_pcalloc(pool, 
                                        sizeof(*results) * nTests);

    for (i = 0; i < nTests; i++) {
        printf("Test -> %c\n", testRuns[i].c);
        results[i].size = testRuns[i].size * (apr_size_t)TEST_SIZE;
        rv = runTest(&testRuns[i], &results[i], pool);
        if (rv != APR_SUCCESS) {
            /* error already reported */
            exit(1);
        }
    }

    printf("Tests Complete!\n");
    for (i = 0; i < nTests; i++) {
        int j;
        apr_time_t totTime = 0;
        printf("%10d byte block:\n", results[i].size);
        printf("\t%2d iterations : ", results[i].iters);
        for (j = 0; j < results[i].iters; j++) {
            printf("%6" APR_TIME_T_FMT, results[i].msecs[j]);
            totTime += results[i].msecs[j];
        }
        printf("<\n");
        printf("\t  Average: %6" APR_TIME_T_FMT "\n",
               totTime / results[i].iters);
    }

    return 0;
}
