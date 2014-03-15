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
#include "apr.h"
#include "apr_strings.h"
#include "apr_general.h"
#include "apr_pools.h"
#include "apr_tables.h"
#if APR_HAVE_STDIO_H
#include <stdio.h>
#endif
#if APR_HAVE_STDLIB_H
#include <stdlib.h>
#endif
#if APR_HAVE_STRING_H
#include <string.h>
#endif

static apr_array_header_t *a1 = NULL;
static apr_table_t *t1 = NULL;

static void array_clear(abts_case *tc, void *data)
{
    a1 = apr_array_make(p, 2, sizeof(const char *));
    APR_ARRAY_PUSH(a1, const char *) = "foo";
    APR_ARRAY_PUSH(a1, const char *) = "bar";
    apr_array_clear(a1);
    ABTS_INT_EQUAL(tc, 0, a1->nelts);
}

static void table_make(abts_case *tc, void *data)
{
    t1 = apr_table_make(p, 5);
    ABTS_PTR_NOTNULL(tc, t1);
}

static void table_get(abts_case *tc, void *data)
{
    const char *val;

    apr_table_set(t1, "foo", "bar");
    val = apr_table_get(t1, "foo");
    ABTS_STR_EQUAL(tc, "bar", val);
}

static void table_getm(abts_case *tc, void *data)
{
    const char *orig, *val;
    apr_pool_t *subp;

    apr_pool_create(&subp, p);

    orig = "bar";
    apr_table_setn(t1, "foo", orig);
    val = apr_table_getm(subp, t1, "foo");
    ABTS_PTR_EQUAL(tc, orig, val);
    ABTS_STR_EQUAL(tc, "bar", val);
    apr_table_add(t1, "foo", "baz");
    val = apr_table_getm(subp, t1, "foo");
    ABTS_STR_EQUAL(tc, "bar,baz", val);

    apr_pool_destroy(subp);
}

static void table_set(abts_case *tc, void *data)
{
    const char *val;

    apr_table_set(t1, "setkey", "bar");
    apr_table_set(t1, "setkey", "2ndtry");
    val = apr_table_get(t1, "setkey");
    ABTS_STR_EQUAL(tc, "2ndtry", val);
}

static void table_getnotthere(abts_case *tc, void *data)
{
    const char *val;

    val = apr_table_get(t1, "keynotthere");
    ABTS_PTR_EQUAL(tc, NULL, (void *)val);
}

static void table_add(abts_case *tc, void *data)
{
    const char *val;

    apr_table_add(t1, "addkey", "bar");
    apr_table_add(t1, "addkey", "foo");
    val = apr_table_get(t1, "addkey");
    ABTS_STR_EQUAL(tc, "bar", val);

}

static void table_nelts(abts_case *tc, void *data)
{
    const char *val;
    apr_table_t *t = apr_table_make(p, 1);

    apr_table_set(t, "abc", "def");
    apr_table_set(t, "def", "abc");
    apr_table_set(t, "foo", "zzz");
    val = apr_table_get(t, "foo");
    ABTS_STR_EQUAL(tc, "zzz", val);
    val = apr_table_get(t, "abc");
    ABTS_STR_EQUAL(tc, "def", val);
    val = apr_table_get(t, "def");
    ABTS_STR_EQUAL(tc, "abc", val);
    ABTS_INT_EQUAL(tc, 3, apr_table_elts(t)->nelts);
}

static void table_clear(abts_case *tc, void *data)
{
    apr_table_clear(t1);
    ABTS_INT_EQUAL(tc, 0, apr_table_elts(t1)->nelts);
}

static void table_unset(abts_case *tc, void *data)
{
    const char *val;
    apr_table_t *t = apr_table_make(p, 1);

    apr_table_set(t, "a", "1");
    apr_table_set(t, "b", "2");
    apr_table_unset(t, "b");
    ABTS_INT_EQUAL(tc, 1, apr_table_elts(t)->nelts);
    val = apr_table_get(t, "a");
    ABTS_STR_EQUAL(tc, "1", val);
    val = apr_table_get(t, "b");
    ABTS_PTR_EQUAL(tc, (void *)NULL, (void *)val);
}

static void table_overlap(abts_case *tc, void *data)
{
    const char *val;
    apr_table_t *t1 = apr_table_make(p, 1);
    apr_table_t *t2 = apr_table_make(p, 1);

    apr_table_addn(t1, "a", "0");
    apr_table_addn(t1, "g", "7");
    apr_table_addn(t2, "a", "1");
    apr_table_addn(t2, "b", "2");
    apr_table_addn(t2, "c", "3");
    apr_table_addn(t2, "b", "2.0");
    apr_table_addn(t2, "d", "4");
    apr_table_addn(t2, "e", "5");
    apr_table_addn(t2, "b", "2.");
    apr_table_addn(t2, "f", "6");
    apr_table_overlap(t1, t2, APR_OVERLAP_TABLES_SET);
    
    ABTS_INT_EQUAL(tc, 7, apr_table_elts(t1)->nelts);
    val = apr_table_get(t1, "a");
    ABTS_STR_EQUAL(tc, "1", val);
    val = apr_table_get(t1, "b");
    ABTS_STR_EQUAL(tc, "2.", val);
    val = apr_table_get(t1, "c");
    ABTS_STR_EQUAL(tc, "3", val);
    val = apr_table_get(t1, "d");
    ABTS_STR_EQUAL(tc, "4", val);
    val = apr_table_get(t1, "e");
    ABTS_STR_EQUAL(tc, "5", val);
    val = apr_table_get(t1, "f");
    ABTS_STR_EQUAL(tc, "6", val);
    val = apr_table_get(t1, "g");
    ABTS_STR_EQUAL(tc, "7", val);
}

static void table_overlap2(abts_case *tc, void *data)
{
    apr_pool_t *subp;
    apr_table_t *t1, *t2;

    apr_pool_create(&subp, p);

    t1 = apr_table_make(subp, 1);
    t2 = apr_table_make(p, 1);
    apr_table_addn(t1, "t1", "one");
    apr_table_addn(t2, "t2", "two");
    
    apr_table_overlap(t1, t2, APR_OVERLAP_TABLES_SET);
    
    ABTS_INT_EQUAL(tc, 2, apr_table_elts(t1)->nelts);
    
    ABTS_STR_EQUAL(tc, "one", apr_table_get(t1, "t1"));
    ABTS_STR_EQUAL(tc, "two", apr_table_get(t1, "t2"));

}

abts_suite *testtable(abts_suite *suite)
{
    suite = ADD_SUITE(suite)

    abts_run_test(suite, array_clear, NULL);
    abts_run_test(suite, table_make, NULL);
    abts_run_test(suite, table_get, NULL);
    abts_run_test(suite, table_getm, NULL);
    abts_run_test(suite, table_set, NULL);
    abts_run_test(suite, table_getnotthere, NULL);
    abts_run_test(suite, table_add, NULL);
    abts_run_test(suite, table_nelts, NULL);
    abts_run_test(suite, table_clear, NULL);
    abts_run_test(suite, table_unset, NULL);
    abts_run_test(suite, table_overlap, NULL);
    abts_run_test(suite, table_overlap2, NULL);

    return suite;
}

