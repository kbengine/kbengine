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

#include "apr.h"
#include "apr_poll.h"
#include "apr_arch_networkio.h"

APR_DECLARE(apr_status_t) apr_poll(apr_pollfd_t *aprset, apr_int32_t num,
                      apr_int32_t *nsds, apr_interval_time_t timeout)
{
    int *pollset;
    int i;
    int num_read = 0, num_write = 0, num_except = 0, num_total;
    int pos_read, pos_write, pos_except;

    for (i = 0; i < num; i++) {
        if (aprset[i].desc_type == APR_POLL_SOCKET) {
            num_read += (aprset[i].reqevents & APR_POLLIN) != 0;
            num_write += (aprset[i].reqevents & APR_POLLOUT) != 0;
            num_except += (aprset[i].reqevents & APR_POLLPRI) != 0;
        }
    }

    num_total = num_read + num_write + num_except;
    pollset = alloca(sizeof(int) * num_total);
    memset(pollset, 0, sizeof(int) * num_total);

    pos_read = 0;
    pos_write = num_read;
    pos_except = pos_write + num_write;

    for (i = 0; i < num; i++) {
        if (aprset[i].desc_type == APR_POLL_SOCKET) {
            if (aprset[i].reqevents & APR_POLLIN) {
                pollset[pos_read++] = aprset[i].desc.s->socketdes;
            }

            if (aprset[i].reqevents & APR_POLLOUT) {
                pollset[pos_write++] = aprset[i].desc.s->socketdes;
            }

            if (aprset[i].reqevents & APR_POLLPRI) {
                pollset[pos_except++] = aprset[i].desc.s->socketdes;
            }

            aprset[i].rtnevents = 0;
        }
    }

    if (timeout > 0) {
        timeout /= 1000; /* convert microseconds to milliseconds */
    }

    i = select(pollset, num_read, num_write, num_except, timeout);
    (*nsds) = i;

    if ((*nsds) < 0) {
        return APR_FROM_OS_ERROR(sock_errno());
    }

    if ((*nsds) == 0) {
        return APR_TIMEUP;
    }

    pos_read = 0;
    pos_write = num_read;
    pos_except = pos_write + num_write;

    for (i = 0; i < num; i++) {
        if (aprset[i].desc_type == APR_POLL_SOCKET) {
            if (aprset[i].reqevents & APR_POLLIN) {
                if (pollset[pos_read++] > 0) {
                    aprset[i].rtnevents |= APR_POLLIN;
                }
            }

            if (aprset[i].reqevents & APR_POLLOUT) {
                if (pollset[pos_write++] > 0) {
                    aprset[i].rtnevents |= APR_POLLOUT;
                }
            }

            if (aprset[i].reqevents & APR_POLLPRI) {
                if (pollset[pos_except++] > 0) {
                    aprset[i].rtnevents |= APR_POLLPRI;
                }
            }
        }
    }

    return APR_SUCCESS;
}
