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
#include "apr_general.h"
#include "apr_network_io.h"
#include "apr_errno.h"

static void test_bad_input(abts_case *tc, void *data)
{
    struct {
        const char *ipstr;
        const char *mask;
        apr_status_t expected_rv;
    } testcases[] =
    {
        /* so we have a few good inputs in here; sue me */
        {"my.host.name",       NULL,               APR_EINVAL}
        ,{"127.0.0.256",       NULL,               APR_EBADIP}
        ,{"127.0.0.1",         NULL,               APR_SUCCESS}
        ,{"127.0.0.1",         "32",               APR_SUCCESS}
        ,{"127.0.0.1",         "1",                APR_SUCCESS}
        ,{"127.0.0.1",         "15",               APR_SUCCESS}
        ,{"127.0.0.1",         "-1",               APR_EBADMASK}
        ,{"127.0.0.1",         "0",                APR_EBADMASK}
        ,{"127.0.0.1",         "33",               APR_EBADMASK}
        ,{"127.0.0.1",         "255.0.0.0",        APR_SUCCESS}
        ,{"127.0.0.1",         "255.0",            APR_EBADMASK}
        ,{"127.0.0.1",         "255.255.256.0",    APR_EBADMASK}
        ,{"127.0.0.1",         "abc",              APR_EBADMASK}
        ,{"127",               NULL,               APR_SUCCESS}
        ,{"127.0.0.1.2",       NULL,               APR_EBADIP}
        ,{"127.0.0.1.2",       "8",                APR_EBADIP}
        ,{"127",               "255.0.0.0",        APR_EBADIP} /* either EBADIP or EBADMASK seems fine */
#if APR_HAVE_IPV6
        ,{"::1",               NULL,               APR_SUCCESS}
        ,{"::1",               "20",               APR_SUCCESS}
        ,{"::ffff:9.67.113.15", NULL,              APR_EBADIP} /* yes, this is goodness */
        ,{"fe80::",            "16",               APR_SUCCESS}
        ,{"fe80::",            "255.0.0.0",        APR_EBADMASK}
        ,{"fe80::1",           "0",                APR_EBADMASK}
        ,{"fe80::1",           "-1",               APR_EBADMASK}
        ,{"fe80::1",           "1",                APR_SUCCESS}
        ,{"fe80::1",           "33",               APR_SUCCESS}
        ,{"fe80::1",           "128",              APR_SUCCESS}
        ,{"fe80::1",           "129",              APR_EBADMASK}
#else
        /* do some IPv6 stuff and verify that it fails with APR_EBADIP */
        ,{"::ffff:9.67.113.15", NULL,              APR_EBADIP}
#endif
    };
    int i;
    apr_ipsubnet_t *ipsub;
    apr_status_t rv;

    for (i = 0; i < (sizeof testcases / sizeof testcases[0]); i++) {
        rv = apr_ipsubnet_create(&ipsub, testcases[i].ipstr, testcases[i].mask, p);
        ABTS_INT_EQUAL(tc, testcases[i].expected_rv, rv);
    }
}

static void test_singleton_subnets(abts_case *tc, void *data)
{
    const char *v4addrs[] = {
        "127.0.0.1", "129.42.18.99", "63.161.155.20", "207.46.230.229", "64.208.42.36",
        "198.144.203.195", "192.18.97.241", "198.137.240.91", "62.156.179.119", 
        "204.177.92.181"
    };
    apr_ipsubnet_t *ipsub;
    apr_sockaddr_t *sa;
    apr_status_t rv;
    int i, j, rc;

    for (i = 0; i < sizeof v4addrs / sizeof v4addrs[0]; i++) {
        rv = apr_ipsubnet_create(&ipsub, v4addrs[i], NULL, p);
        ABTS_TRUE(tc, rv == APR_SUCCESS);
        for (j = 0; j < sizeof v4addrs / sizeof v4addrs[0]; j++) {
            rv = apr_sockaddr_info_get(&sa, v4addrs[j], APR_INET, 0, 0, p);
            ABTS_TRUE(tc, rv == APR_SUCCESS);
            rc = apr_ipsubnet_test(ipsub, sa);
            if (!strcmp(v4addrs[i], v4addrs[j])) {
                ABTS_TRUE(tc, rc != 0);
            }
            else {
                ABTS_TRUE(tc, rc == 0);
            }
        }
    }

    /* same for v6? */
}

static void test_interesting_subnets(abts_case *tc, void *data)
{
    struct {
        const char *ipstr, *mask;
        int family;
        char *in_subnet, *not_in_subnet;
    } testcases[] =
    {
         {"9.67",             NULL,            APR_INET,  "9.67.113.15",         "10.1.2.3"}
        ,{"9.67.0.0",         "16",            APR_INET,  "9.67.113.15",         "10.1.2.3"}
        ,{"9.67.0.0",         "255.255.0.0",   APR_INET,  "9.67.113.15",         "10.1.2.3"}
        ,{"9.67.113.99",      "16",            APR_INET,  "9.67.113.15",         "10.1.2.3"}
        ,{"9.67.113.99",      "255.255.255.0", APR_INET,  "9.67.113.15",         "10.1.2.3"}
        ,{"127",              NULL,            APR_INET,  "127.0.0.1",           "10.1.2.3"}
        ,{"127.0.0.1",        "8",             APR_INET,  "127.0.0.1",           "10.1.2.3"}
#if APR_HAVE_IPV6
        ,{"38.0.0.0",         "8",             APR_INET6, "::ffff:38.1.1.1",     "2600::1"} /* PR 54047 */
        ,{"fe80::",           "8",             APR_INET6, "fe80::1",             "ff01::1"}
        ,{"ff01::",           "8",             APR_INET6, "ff01::1",             "fe80::1"}
        ,{"3FFE:8160::",      "28",            APR_INET6, "3ffE:816e:abcd:1234::1", "3ffe:8170::1"}
        ,{"127.0.0.1",        NULL,            APR_INET6, "::ffff:127.0.0.1",    "fe80::1"}
        ,{"127.0.0.1",        "8",             APR_INET6, "::ffff:127.0.0.1",    "fe80::1"}
#endif
    };
    apr_ipsubnet_t *ipsub;
    apr_sockaddr_t *sa;
    apr_status_t rv;
    int i, rc;

    for (i = 0; i < sizeof testcases / sizeof testcases[0]; i++) {
        rv = apr_ipsubnet_create(&ipsub, testcases[i].ipstr, testcases[i].mask, p);
        ABTS_TRUE(tc, rv == APR_SUCCESS);
        rv = apr_sockaddr_info_get(&sa, testcases[i].in_subnet, testcases[i].family, 0, 0, p);
        ABTS_TRUE(tc, rv == APR_SUCCESS);
        ABTS_TRUE(tc, sa != NULL);
        if (!sa) continue;
        rc = apr_ipsubnet_test(ipsub, sa);
        ABTS_TRUE(tc, rc != 0);
        rv = apr_sockaddr_info_get(&sa, testcases[i].not_in_subnet, testcases[i].family, 0, 0, p);
        ABTS_TRUE(tc, rv == APR_SUCCESS);
        rc = apr_ipsubnet_test(ipsub, sa);
        ABTS_TRUE(tc, rc == 0);
    }
}

static void test_badmask_str(abts_case *tc, void *data)
{
    char buf[128];

    ABTS_STR_EQUAL(tc, apr_strerror(APR_EBADMASK, buf, sizeof buf),
                      "The specified network mask is invalid.");
}

static void test_badip_str(abts_case *tc, void *data)
{
    char buf[128];

    ABTS_STR_EQUAL(tc, apr_strerror(APR_EBADIP, buf, sizeof buf),
                      "The specified IP address is invalid.");
}

abts_suite *testipsub(abts_suite *suite)
{
    suite = ADD_SUITE(suite)

    abts_run_test(suite, test_bad_input, NULL);
    abts_run_test(suite, test_singleton_subnets, NULL);
    abts_run_test(suite, test_interesting_subnets, NULL);
    abts_run_test(suite, test_badmask_str, NULL);
    abts_run_test(suite, test_badip_str, NULL);
    return suite;
}

