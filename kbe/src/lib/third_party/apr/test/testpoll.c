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

#include "testutil.h"
#include "apr_strings.h"
#include "apr_errno.h"
#include "apr_general.h"
#include "apr_lib.h"
#include "apr_network_io.h"
#include "apr_poll.h"

#define SMALL_NUM_SOCKETS 3
/* We can't use 64 here, because some platforms *ahem* Solaris *ahem* have
 * a default limit of 64 open file descriptors per process.  If we use
 * 64, the test will fail even though the code is correct.
 */
#define LARGE_NUM_SOCKETS 50

static apr_socket_t *s[LARGE_NUM_SOCKETS];
static apr_sockaddr_t *sa[LARGE_NUM_SOCKETS];
static apr_pollset_t *pollset;
static apr_pollcb_t *pollcb;

/* ###: tests surrounded by ifdef OLD_POLL_INTERFACE either need to be
 * converted to use the pollset interface or removed. */

#ifdef OLD_POLL_INTERFACE
static apr_pollfd_t *pollarray;
static apr_pollfd_t *pollarray_large;
#endif

static void make_socket(apr_socket_t **sock, apr_sockaddr_t **sa, 
                        apr_port_t port, apr_pool_t *p, abts_case *tc)
{
    apr_status_t rv;

    rv = apr_sockaddr_info_get(sa, "127.0.0.1", APR_UNSPEC, port, 0, p);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);

    rv = apr_socket_create(sock, (*sa)->family, SOCK_DGRAM, 0, p);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);

    rv = apr_socket_bind((*sock), (*sa));
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
}

#ifdef OLD_POLL_INTERFACE
static void check_sockets(const apr_pollfd_t *pollarray, 
                          apr_socket_t **sockarray, int which, int pollin, 
                          abts_case *tc)
{
    apr_status_t rv;
    apr_int16_t event;
    char *str;

    rv = apr_poll_revents_get(&event, sockarray[which], 
                              (apr_pollfd_t *)pollarray);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    if (pollin) {
        str = apr_psprintf(p, "Socket %d not signalled when it should be",
                           which);
        ABTS_ASSERT(tc, str, event & APR_POLLIN);
    } else {
        str = apr_psprintf(p, "Socket %d signalled when it should not be",
                           which);
        ABTS_ASSERT(tc, str, !(event & APR_POLLIN));
    }
}
#endif

static void send_msg(apr_socket_t **sockarray, apr_sockaddr_t **sas, int which,
                     abts_case *tc)
{
    apr_size_t len = 5;
    apr_status_t rv;

    ABTS_PTR_NOTNULL(tc, sockarray[which]);

    rv = apr_socket_sendto(sockarray[which], sas[which], 0, "hello", &len);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    ABTS_SIZE_EQUAL(tc, strlen("hello"), len);
}

static void recv_msg(apr_socket_t **sockarray, int which, apr_pool_t *p, 
                     abts_case *tc)
{
    apr_size_t buflen = 5;
    char *buffer = apr_pcalloc(p, sizeof(char) * (buflen + 1));
    apr_sockaddr_t *recsa;
    apr_status_t rv;

    ABTS_PTR_NOTNULL(tc, sockarray[which]);

    apr_sockaddr_info_get(&recsa, "127.0.0.1", APR_UNSPEC, 7770, 0, p);

    rv = apr_socket_recvfrom(recsa, sockarray[which], 0, buffer, &buflen);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    ABTS_SIZE_EQUAL(tc, strlen("hello"), buflen);
    ABTS_STR_EQUAL(tc, "hello", buffer);
}

    
static void create_all_sockets(abts_case *tc, void *data)
{
    int i;

    for (i = 0; i < LARGE_NUM_SOCKETS; i++){
        make_socket(&s[i], &sa[i], 7777 + i, p, tc);
    }
}
       
#ifdef OLD_POLL_INTERFACE
static void setup_small_poll(abts_case *tc, void *data)
{
    apr_status_t rv;
    int i;

    rv = apr_poll_setup(&pollarray, SMALL_NUM_SOCKETS, p);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    
    for (i = 0; i < SMALL_NUM_SOCKETS;i++){
        ABTS_INT_EQUAL(tc, 0, pollarray[i].reqevents);
        ABTS_INT_EQUAL(tc, 0, pollarray[i].rtnevents);

        rv = apr_poll_socket_add(pollarray, s[i], APR_POLLIN);
        ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
        ABTS_PTR_EQUAL(tc, s[i], pollarray[i].desc.s);
    }
}

static void setup_large_poll(abts_case *tc, void *data)
{
    apr_status_t rv;
    int i;

    rv = apr_poll_setup(&pollarray_large, LARGE_NUM_SOCKETS, p);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    
    for (i = 0; i < LARGE_NUM_SOCKETS;i++){
        ABTS_INT_EQUAL(tc, 0, pollarray_large[i].reqevents);
        ABTS_INT_EQUAL(tc, 0, pollarray_large[i].rtnevents);

        rv = apr_poll_socket_add(pollarray_large, s[i], APR_POLLIN);
        ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
        ABTS_PTR_EQUAL(tc, s[i], pollarray_large[i].desc.s);
    }
}

static void nomessage(abts_case *tc, void *data)
{
    apr_status_t rv;
    int srv = SMALL_NUM_SOCKETS;

    rv = apr_poll(pollarray, SMALL_NUM_SOCKETS, &srv, 2 * APR_USEC_PER_SEC);
    ABTS_INT_EQUAL(tc, 1, APR_STATUS_IS_TIMEUP(rv));
    check_sockets(pollarray, s, 0, 0, tc);
    check_sockets(pollarray, s, 1, 0, tc);
    check_sockets(pollarray, s, 2, 0, tc);
}

static void send_2(abts_case *tc, void *data)
{
    apr_status_t rv;
    int srv = SMALL_NUM_SOCKETS;

    send_msg(s, sa, 2, tc);

    rv = apr_poll(pollarray, SMALL_NUM_SOCKETS, &srv, 2 * APR_USEC_PER_SEC);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    check_sockets(pollarray, s, 0, 0, tc);
    check_sockets(pollarray, s, 1, 0, tc);
    check_sockets(pollarray, s, 2, 1, tc);
}

static void recv_2_send_1(abts_case *tc, void *data)
{
    apr_status_t rv;
    int srv = SMALL_NUM_SOCKETS;

    recv_msg(s, 2, p, tc);
    send_msg(s, sa, 1, tc);

    rv = apr_poll(pollarray, SMALL_NUM_SOCKETS, &srv, 2 * APR_USEC_PER_SEC);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    check_sockets(pollarray, s, 0, 0, tc);
    check_sockets(pollarray, s, 1, 1, tc);
    check_sockets(pollarray, s, 2, 0, tc);
}

static void send_2_signaled_1(abts_case *tc, void *data)
{
    apr_status_t rv;
    int srv = SMALL_NUM_SOCKETS;

    send_msg(s, sa, 2, tc);

    rv = apr_poll(pollarray, SMALL_NUM_SOCKETS, &srv, 2 * APR_USEC_PER_SEC);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    check_sockets(pollarray, s, 0, 0, tc);
    check_sockets(pollarray, s, 1, 1, tc);
    check_sockets(pollarray, s, 2, 1, tc);
}

static void recv_1_send_0(abts_case *tc, void *data)
{
    apr_status_t rv;
    int srv = SMALL_NUM_SOCKETS;

    recv_msg(s, 1, p, tc);
    send_msg(s, sa, 0, tc);

    rv = apr_poll(pollarray, SMALL_NUM_SOCKETS, &srv, 2 * APR_USEC_PER_SEC);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    check_sockets(pollarray, s, 0, 1, tc);
    check_sockets(pollarray, s, 1, 0, tc);
    check_sockets(pollarray, s, 2, 1, tc);
}

static void clear_all_signalled(abts_case *tc, void *data)
{
    apr_status_t rv;
    int srv = SMALL_NUM_SOCKETS;

    recv_msg(s, 0, p, tc);
    recv_msg(s, 2, p, tc);

    rv = apr_poll(pollarray, SMALL_NUM_SOCKETS, &srv, 2 * APR_USEC_PER_SEC);
    ABTS_INT_EQUAL(tc, 1, APR_STATUS_IS_TIMEUP(rv));
    check_sockets(pollarray, s, 0, 0, tc);
    check_sockets(pollarray, s, 1, 0, tc);
    check_sockets(pollarray, s, 2, 0, tc);
}

static void send_large_pollarray(abts_case *tc, void *data)
{
    apr_status_t rv;
    int lrv = LARGE_NUM_SOCKETS;
    int i;

    send_msg(s, sa, LARGE_NUM_SOCKETS - 1, tc);

    rv = apr_poll(pollarray_large, LARGE_NUM_SOCKETS, &lrv, 
                  2 * APR_USEC_PER_SEC);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);

    for (i = 0; i < LARGE_NUM_SOCKETS; i++) {
        if (i == (LARGE_NUM_SOCKETS - 1)) {
            check_sockets(pollarray_large, s, i, 1, tc);
        }
        else {
            check_sockets(pollarray_large, s, i, 0, tc);
        }
    }
}

static void recv_large_pollarray(abts_case *tc, void *data)
{
    apr_status_t rv;
    int lrv = LARGE_NUM_SOCKETS;
    int i;

    recv_msg(s, LARGE_NUM_SOCKETS - 1, p, tc);

    rv = apr_poll(pollarray_large, LARGE_NUM_SOCKETS, &lrv, 
                  2 * APR_USEC_PER_SEC);
    ABTS_INT_EQUAL(tc, 1, APR_STATUS_IS_TIMEUP(rv));

    for (i = 0; i < LARGE_NUM_SOCKETS; i++) {
        check_sockets(pollarray_large, s, i, 0, tc);
    }
}
#endif

static void setup_pollset(abts_case *tc, void *data)
{
    apr_status_t rv;
    rv = apr_pollset_create(&pollset, LARGE_NUM_SOCKETS, p, 0);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
}

static void multi_event_pollset(abts_case *tc, void *data)
{
    apr_status_t rv;
    apr_pollfd_t socket_pollfd;
    int lrv;
    const apr_pollfd_t *descs = NULL;

    ABTS_PTR_NOTNULL(tc, s[0]);
    socket_pollfd.desc_type = APR_POLL_SOCKET;
    socket_pollfd.reqevents = APR_POLLIN | APR_POLLOUT;
    socket_pollfd.desc.s = s[0];
    socket_pollfd.client_data = s[0];
    rv = apr_pollset_add(pollset, &socket_pollfd);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);

    send_msg(s, sa, 0, tc);

    rv = apr_pollset_poll(pollset, -1, &lrv, &descs);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    if (lrv == 1) {
        int ev = descs[0].rtnevents;
        ABTS_PTR_EQUAL(tc, s[0], descs[0].desc.s);
        ABTS_PTR_EQUAL(tc, s[0],  descs[0].client_data);
        ABTS_ASSERT(tc, "either or both of APR_POLLIN, APR_POLLOUT returned",
                    ((ev & APR_POLLIN) != 0) || ((ev & APR_POLLOUT) != 0));
    }
    else if (lrv == 2) {
        ABTS_PTR_EQUAL(tc, s[0], descs[0].desc.s);
        ABTS_PTR_EQUAL(tc, s[0], descs[0].client_data);
        ABTS_PTR_EQUAL(tc, s[0], descs[1].desc.s);
        ABTS_PTR_EQUAL(tc, s[0], descs[1].client_data);
        ABTS_ASSERT(tc, "returned events incorrect",
                    ((descs[0].rtnevents | descs[1].rtnevents)
                     == (APR_POLLIN | APR_POLLOUT))
                    && descs[0].rtnevents != descs[1].rtnevents);
    }
    else {
        ABTS_ASSERT(tc, "either one or two events returned",
                    lrv == 1 || lrv == 2);
    }

    recv_msg(s, 0, p, tc);

    rv = apr_pollset_poll(pollset, 0, &lrv, &descs);
    ABTS_INT_EQUAL(tc, 0, APR_STATUS_IS_TIMEUP(rv));
    ABTS_INT_EQUAL(tc, 1, lrv);
    ABTS_PTR_EQUAL(tc, s[0], descs[0].desc.s);
    ABTS_INT_EQUAL(tc, APR_POLLOUT, descs[0].rtnevents);
    ABTS_PTR_EQUAL(tc, s[0],  descs[0].client_data);

    rv = apr_pollset_remove(pollset, &socket_pollfd);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
}
                         
static void add_sockets_pollset(abts_case *tc, void *data)
{
    apr_status_t rv;
    int i;

    for (i = 0; i < LARGE_NUM_SOCKETS;i++){
        apr_pollfd_t socket_pollfd;

        ABTS_PTR_NOTNULL(tc, s[i]);

        socket_pollfd.desc_type = APR_POLL_SOCKET;
        socket_pollfd.reqevents = APR_POLLIN;
        socket_pollfd.desc.s = s[i];
        socket_pollfd.client_data = s[i];
        rv = apr_pollset_add(pollset, &socket_pollfd);
        ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    }
}

static void nomessage_pollset(abts_case *tc, void *data)
{
    apr_status_t rv;
    int lrv;
    const apr_pollfd_t *descs = NULL;

    rv = apr_pollset_poll(pollset, 0, &lrv, &descs);
    ABTS_INT_EQUAL(tc, 1, APR_STATUS_IS_TIMEUP(rv));
    ABTS_INT_EQUAL(tc, 0, lrv);
    ABTS_PTR_EQUAL(tc, NULL, descs);
}

static void send0_pollset(abts_case *tc, void *data)
{
    apr_status_t rv;
    const apr_pollfd_t *descs = NULL;
    int num;

    send_msg(s, sa, 0, tc);
    rv = apr_pollset_poll(pollset, -1, &num, &descs);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    ABTS_INT_EQUAL(tc, 1, num);
    ABTS_PTR_NOTNULL(tc, descs);

    ABTS_PTR_EQUAL(tc, s[0], descs[0].desc.s);
    ABTS_PTR_EQUAL(tc, s[0], descs[0].client_data);
}

static void recv0_pollset(abts_case *tc, void *data)
{
    apr_status_t rv;
    int lrv;
    const apr_pollfd_t *descs = NULL;

    recv_msg(s, 0, p, tc);
    rv = apr_pollset_poll(pollset, 0, &lrv, &descs);
    ABTS_INT_EQUAL(tc, 1, APR_STATUS_IS_TIMEUP(rv));
    ABTS_INT_EQUAL(tc, 0, lrv);
    ABTS_PTR_EQUAL(tc, NULL, descs);
}

static void send_middle_pollset(abts_case *tc, void *data)
{
    apr_status_t rv;
    const apr_pollfd_t *descs = NULL;
    int num;
    
    send_msg(s, sa, 2, tc);
    send_msg(s, sa, 5, tc);
    rv = apr_pollset_poll(pollset, -1, &num, &descs);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    ABTS_PTR_NOTNULL(tc, descs);
    ABTS_ASSERT(tc, "either one or two events returned",
                num == 1 || num == 2);

    /* The poll might only see the first sent message, in which
     * case we just don't bother checking this assertion */
    if (num == 2) {
        ABTS_ASSERT(tc, "Incorrect socket in result set",
                    ((descs[0].desc.s == s[2]) && (descs[1].desc.s == s[5])) ||
                    ((descs[0].desc.s == s[5]) && (descs[1].desc.s == s[2])));
    }
}

static void clear_middle_pollset(abts_case *tc, void *data)
{
    apr_status_t rv;
    int lrv;
    const apr_pollfd_t *descs = NULL;

    recv_msg(s, 2, p, tc);
    recv_msg(s, 5, p, tc);

    rv = apr_pollset_poll(pollset, 0, &lrv, &descs);
    ABTS_INT_EQUAL(tc, 1, APR_STATUS_IS_TIMEUP(rv));
    ABTS_INT_EQUAL(tc, 0, lrv);
    ABTS_PTR_EQUAL(tc, NULL, descs);
}

static void send_last_pollset(abts_case *tc, void *data)
{
    apr_status_t rv;
    const apr_pollfd_t *descs = NULL;
    int num;
    
    send_msg(s, sa, LARGE_NUM_SOCKETS - 1, tc);
    rv = apr_pollset_poll(pollset, -1, &num, &descs);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    ABTS_INT_EQUAL(tc, 1, num);
    ABTS_PTR_NOTNULL(tc, descs);

    ABTS_PTR_EQUAL(tc, s[LARGE_NUM_SOCKETS - 1], descs[0].desc.s);
    ABTS_PTR_EQUAL(tc, s[LARGE_NUM_SOCKETS - 1], descs[0].client_data);
}

static void clear_last_pollset(abts_case *tc, void *data)
{
    apr_status_t rv;
    int lrv;
    const apr_pollfd_t *descs = NULL;

    recv_msg(s, LARGE_NUM_SOCKETS - 1, p, tc);

    rv = apr_pollset_poll(pollset, 0, &lrv, &descs);
    ABTS_INT_EQUAL(tc, 1, APR_STATUS_IS_TIMEUP(rv));
    ABTS_INT_EQUAL(tc, 0, lrv);
    ABTS_PTR_EQUAL(tc, NULL, descs);
}

static void close_all_sockets(abts_case *tc, void *data)
{
    apr_status_t rv;
    int i;

    for (i = 0; i < LARGE_NUM_SOCKETS; i++){
        rv = apr_socket_close(s[i]);
        ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    }
}

static void pollset_remove(abts_case *tc, void *data)
{
    apr_status_t rv;
    apr_pollset_t *pollset;
    const apr_pollfd_t *hot_files;
    apr_pollfd_t pfd;
    apr_int32_t num;

    rv = apr_pollset_create(&pollset, 5, p, 0);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);

    pfd.p = p;
    pfd.desc_type = APR_POLL_SOCKET;
    pfd.reqevents = APR_POLLOUT;

    pfd.desc.s = s[0];
    pfd.client_data = (void *)1;
    rv = apr_pollset_add(pollset, &pfd);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);

    pfd.desc.s = s[1];
    pfd.client_data = (void *)2;
    rv = apr_pollset_add(pollset, &pfd);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);

    pfd.desc.s = s[2];
    pfd.client_data = (void *)3;
    rv = apr_pollset_add(pollset, &pfd);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);

    pfd.desc.s = s[3];
    pfd.client_data = (void *)4;
    rv = apr_pollset_add(pollset, &pfd);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);

    rv = apr_pollset_poll(pollset, 1000, &num, &hot_files);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    ABTS_INT_EQUAL(tc, 4, num);

    /* now remove the pollset element referring to desc s[1] */
    pfd.desc.s = s[1];
    pfd.client_data = (void *)999; /* not used on this call */
    rv = apr_pollset_remove(pollset, &pfd);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);

    /* this time only three should match */
    rv = apr_pollset_poll(pollset, 1000, &num, &hot_files);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    ABTS_INT_EQUAL(tc, 3, num);
    ABTS_PTR_EQUAL(tc, (void *)1, hot_files[0].client_data);
    ABTS_PTR_EQUAL(tc, s[0], hot_files[0].desc.s);
    ABTS_PTR_EQUAL(tc, (void *)3, hot_files[1].client_data);
    ABTS_PTR_EQUAL(tc, s[2], hot_files[1].desc.s);
    ABTS_PTR_EQUAL(tc, (void *)4, hot_files[2].client_data);
    ABTS_PTR_EQUAL(tc, s[3], hot_files[2].desc.s);
    
    /* now remove the pollset elements referring to desc s[2] */
    pfd.desc.s = s[2];
    pfd.client_data = (void *)999; /* not used on this call */
    rv = apr_pollset_remove(pollset, &pfd);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);

    /* this time only two should match */
    rv = apr_pollset_poll(pollset, 1000, &num, &hot_files);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    ABTS_INT_EQUAL(tc, 2, num);
    ABTS_ASSERT(tc, "Incorrect socket in result set",
            ((hot_files[0].desc.s == s[0]) && (hot_files[1].desc.s == s[3]))  ||
            ((hot_files[0].desc.s == s[3]) && (hot_files[1].desc.s == s[0])));
    ABTS_ASSERT(tc, "Incorrect client data in result set",
            ((hot_files[0].client_data == (void *)1) &&
             (hot_files[1].client_data == (void *)4)) ||
            ((hot_files[0].client_data == (void *)4) &&
             (hot_files[1].client_data == (void *)1)));
}

#define POLLCB_PREREQ \
    do { \
        if (pollcb == NULL) { \
            ABTS_NOT_IMPL(tc, "pollcb interface not supported"); \
            return; \
        } \
    } while (0)

static void setup_pollcb(abts_case *tc, void *data)
{
    apr_status_t rv;
    rv = apr_pollcb_create(&pollcb, LARGE_NUM_SOCKETS, p, 0);
    if (rv == APR_ENOTIMPL) {
        pollcb = NULL;
        ABTS_NOT_IMPL(tc, "pollcb interface not supported");
    }
    else {
        ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    }
}

typedef struct pollcb_baton_t {
    abts_case *tc;
    int count;
} pollcb_baton_t;

static apr_status_t trigger_pollcb_cb(void* baton, apr_pollfd_t *descriptor)
{
    pollcb_baton_t* pcb = (pollcb_baton_t*) baton;
    ABTS_PTR_EQUAL(pcb->tc, s[0], descriptor->desc.s);
    ABTS_PTR_EQUAL(pcb->tc, s[0], descriptor->client_data);
    pcb->count++;
    return APR_SUCCESS;
}

static void trigger_pollcb(abts_case *tc, void *data)
{
    apr_status_t rv;
    apr_pollfd_t socket_pollfd;
    pollcb_baton_t pcb;
    
    POLLCB_PREREQ;

    ABTS_PTR_NOTNULL(tc, s[0]);
    socket_pollfd.desc_type = APR_POLL_SOCKET;
    socket_pollfd.reqevents = APR_POLLIN;
    socket_pollfd.desc.s = s[0];
    socket_pollfd.client_data = s[0];
    rv = apr_pollcb_add(pollcb, &socket_pollfd);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    
    send_msg(s, sa, 0, tc);
    pcb.tc = tc;
    pcb.count = 0;
    rv = apr_pollcb_poll(pollcb, -1, trigger_pollcb_cb, &pcb);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    ABTS_INT_EQUAL(tc, 1, pcb.count);

    rv = apr_pollcb_remove(pollcb, &socket_pollfd);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
}

static void timeout_pollcb(abts_case *tc, void *data)
{
    apr_status_t rv;
    pollcb_baton_t pcb;

    POLLCB_PREREQ;

    pcb.count = 0;
    pcb.tc = tc;

    rv = apr_pollcb_poll(pollcb, 1, trigger_pollcb_cb, &pcb);    
    ABTS_INT_EQUAL(tc, 1, APR_STATUS_IS_TIMEUP(rv));
    ABTS_INT_EQUAL(tc, 0, pcb.count);
}

static void timeout_pollin_pollcb(abts_case *tc, void *data)
{
    apr_status_t rv;
    pollcb_baton_t pcb;
    apr_pollfd_t socket_pollfd;

    POLLCB_PREREQ;

    recv_msg(s, 0, p, tc);
    
    ABTS_PTR_NOTNULL(tc, s[0]);
    socket_pollfd.desc_type = APR_POLL_SOCKET;
    socket_pollfd.reqevents = APR_POLLIN;
    socket_pollfd.desc.s = s[0];
    socket_pollfd.client_data = s[0];
    rv = apr_pollcb_add(pollcb, &socket_pollfd);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    
    pcb.count = 0;
    pcb.tc = tc;
    
    rv = apr_pollcb_poll(pollcb, 1, trigger_pollcb_cb, &pcb);    
    ABTS_INT_EQUAL(tc, 1, APR_STATUS_IS_TIMEUP(rv));
    ABTS_INT_EQUAL(tc, 0, pcb.count);

    rv = apr_pollcb_remove(pollcb, &socket_pollfd);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
}

static void pollset_default(abts_case *tc, void *data)
{
    apr_status_t rv1, rv2;
    apr_pollset_t *pollset;

    /* verify that APR will successfully create a pollset if an invalid method
     * is specified as long as APR_POLLSET_NODEFAULT isn't specified
     * (no platform has both APR_POLLSET_PORT and APR_POLLSET_KQUEUE, so at
     * least one create call will succeed after having to switch to the default
     * type)
     */
    rv1 = apr_pollset_create_ex(&pollset, 1, p, 0, APR_POLLSET_PORT);

    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv1);
    ABTS_PTR_NOTNULL(tc, pollset);
    
    rv1 = apr_pollset_create_ex(&pollset, 1, p, 0, APR_POLLSET_KQUEUE);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv1);
    ABTS_PTR_NOTNULL(tc, pollset);

    /* verify that APR will fail to create a pollset if an invalid method is
     * specified along with APR_POLLSET_NODEFAULT
     * (no platform has both APR_POLLSET_PORT and APR_POLLSET_KQUEUE, so at
     * least one create call will fail since it can't switch to the default
     * type)
     */
    rv1 = apr_pollset_create_ex(&pollset, 1, p, APR_POLLSET_NODEFAULT,
                               APR_POLLSET_PORT);

    if (rv1 == APR_SUCCESS) {
        ABTS_PTR_NOTNULL(tc, pollset);
    }
    
    rv2 = apr_pollset_create_ex(&pollset, 1, p, APR_POLLSET_NODEFAULT,
                               APR_POLLSET_KQUEUE);
    if (rv2 == APR_SUCCESS) {
        ABTS_PTR_NOTNULL(tc, pollset);
    }
    
    ABTS_ASSERT(tc,
                "failure using APR_POLLSET_NODEFAULT with unsupported method",
                rv1 != APR_SUCCESS || rv2 != APR_SUCCESS);
}

static void pollcb_default(abts_case *tc, void *data)
{
    apr_status_t rv1, rv2;
    apr_pollcb_t *pollcb;

    /* verify that APR will successfully create a pollcb if an invalid method
     * is specified as long as APR_POLLSET_NODEFAULT isn't specified
     * (no platform has both APR_POLLSET_PORT and APR_POLLSET_KQUEUE, so at
     * least one create call will succeed after having to switch to the default
     * type)
     */
    rv1 = apr_pollcb_create_ex(&pollcb, 1, p, 0, APR_POLLSET_PORT);
    if (rv1 == APR_ENOTIMPL) {
        ABTS_NOT_IMPL(tc, "pollcb interface not supported");
        return;
    }

    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv1);
    ABTS_PTR_NOTNULL(tc, pollcb);
    
    rv1 = apr_pollcb_create_ex(&pollcb, 1, p, 0, APR_POLLSET_KQUEUE);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv1);
    ABTS_PTR_NOTNULL(tc, pollcb);

    /* verify that APR will fail to create a pollcb if an invalid method is
     * specified along with APR_POLLSET_NODEFAULT
     * (no platform has both APR_POLLSET_PORT and APR_POLLSET_KQUEUE, so at
     * least one create call will fail since it can't switch to the default
     * type)
     */
    rv1 = apr_pollcb_create_ex(&pollcb, 1, p, APR_POLLSET_NODEFAULT,
                               APR_POLLSET_PORT);

    if (rv1 == APR_SUCCESS) {
        ABTS_PTR_NOTNULL(tc, pollcb);
    }
    
    rv2 = apr_pollcb_create_ex(&pollcb, 1, p, APR_POLLSET_NODEFAULT,
                               APR_POLLSET_KQUEUE);
    if (rv2 == APR_SUCCESS) {
        ABTS_PTR_NOTNULL(tc, pollcb);
    }
    
    ABTS_ASSERT(tc,
                "failure using APR_POLLSET_NODEFAULT with unsupported method",
                rv1 != APR_SUCCESS || rv2 != APR_SUCCESS);


    /* verify basic behavior for another method fallback case (this caused
     * APR to crash before r834029)
     */

    rv1 = apr_pollcb_create_ex(&pollcb, 1, p, 0, APR_POLLSET_POLL);
    if (rv1 != APR_ENOTIMPL) {
        ABTS_INT_EQUAL(tc, rv1, APR_SUCCESS);
        ABTS_PTR_NOTNULL(tc, pollcb);
    }
}

static void justsleep(abts_case *tc, void *data)
{
    apr_int32_t nsds;
    const apr_pollfd_t *hot_files;
    apr_pollset_t *pollset;
    apr_status_t rv;
    apr_time_t t1, t2;
    int i;
    apr_pollset_method_e methods[] = {
        APR_POLLSET_DEFAULT,
        APR_POLLSET_SELECT,
        APR_POLLSET_KQUEUE,
        APR_POLLSET_PORT,
        APR_POLLSET_EPOLL,
        APR_POLLSET_POLL};

    nsds = 1;
    t1 = apr_time_now();
    rv = apr_poll(NULL, 0, &nsds, apr_time_from_msec(200));
    t2 = apr_time_now();
    ABTS_INT_EQUAL(tc, 1, APR_STATUS_IS_TIMEUP(rv));
    ABTS_INT_EQUAL(tc, 0, nsds);
    ABTS_ASSERT(tc,
                "apr_poll() didn't sleep",
                (t2 - t1) > apr_time_from_msec(100));

    for (i = 0; i < sizeof methods / sizeof methods[0]; i++) {
        rv = apr_pollset_create_ex(&pollset, 5, p, 0, methods[i]);
        if (rv != APR_ENOTIMPL) {
            ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);

            nsds = 1;
            t1 = apr_time_now();
            rv = apr_pollset_poll(pollset, apr_time_from_msec(200), &nsds,
                                  &hot_files);
            t2 = apr_time_now();
            ABTS_INT_EQUAL(tc, 1, APR_STATUS_IS_TIMEUP(rv));
            ABTS_INT_EQUAL(tc, 0, nsds);
            ABTS_ASSERT(tc,
                        "apr_pollset_poll() didn't sleep",
                        (t2 - t1) > apr_time_from_msec(100));

            rv = apr_pollset_destroy(pollset);
            ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
        }

        rv = apr_pollcb_create_ex(&pollcb, 5, p, 0, methods[0]);
        if (rv != APR_ENOTIMPL) {
            ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);

            t1 = apr_time_now();
            rv = apr_pollcb_poll(pollcb, apr_time_from_msec(200), NULL, NULL);
            t2 = apr_time_now();
            ABTS_INT_EQUAL(tc, 1, APR_STATUS_IS_TIMEUP(rv));
            ABTS_ASSERT(tc,
                        "apr_pollcb_poll() didn't sleep",
                        (t2 - t1) > apr_time_from_msec(100));

            /* no apr_pollcb_destroy() */
        }
    }
}

abts_suite *testpoll(abts_suite *suite)
{
    suite = ADD_SUITE(suite)

    abts_run_test(suite, create_all_sockets, NULL);

#ifdef OLD_POLL_INTERFACE
    abts_run_test(suite, setup_small_poll, NULL);
    abts_run_test(suite, setup_large_poll, NULL);
    abts_run_test(suite, nomessage, NULL);
    abts_run_test(suite, send_2, NULL);
    abts_run_test(suite, recv_2_send_1, NULL);
    abts_run_test(suite, send_2_signaled_1, NULL);
    abts_run_test(suite, recv_1_send_0, NULL);
    abts_run_test(suite, clear_all_signalled, NULL);
    abts_run_test(suite, send_large_pollarray, NULL);
    abts_run_test(suite, recv_large_pollarray, NULL);
#endif

    abts_run_test(suite, setup_pollset, NULL);
    abts_run_test(suite, multi_event_pollset, NULL);
    abts_run_test(suite, add_sockets_pollset, NULL);
    abts_run_test(suite, nomessage_pollset, NULL);
    abts_run_test(suite, send0_pollset, NULL);
    abts_run_test(suite, recv0_pollset, NULL);
    abts_run_test(suite, send_middle_pollset, NULL);
    abts_run_test(suite, clear_middle_pollset, NULL);
    abts_run_test(suite, send_last_pollset, NULL);
    abts_run_test(suite, clear_last_pollset, NULL);
    abts_run_test(suite, pollset_remove, NULL);
    abts_run_test(suite, close_all_sockets, NULL);
    abts_run_test(suite, create_all_sockets, NULL);
    abts_run_test(suite, setup_pollcb, NULL);
    abts_run_test(suite, trigger_pollcb, NULL);
    abts_run_test(suite, timeout_pollcb, NULL);
    abts_run_test(suite, timeout_pollin_pollcb, NULL);
    abts_run_test(suite, close_all_sockets, NULL);
    abts_run_test(suite, pollset_default, NULL);
    abts_run_test(suite, pollcb_default, NULL);
    abts_run_test(suite, justsleep, NULL);
    return suite;
}

