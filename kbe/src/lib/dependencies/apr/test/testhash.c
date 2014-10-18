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
#include "apr_hash.h"

#define MAX_LTH 256
#define MAX_DEPTH 11

static int comp_string(const void *str1, const void *str2)
{
    return strcmp(str1,str2);
}

static void dump_hash(apr_pool_t *p, apr_hash_t *h, char str[][MAX_LTH]) 
{
    apr_hash_index_t *hi;
    int i = 0;

    for (hi = apr_hash_first(p, h); hi; hi = apr_hash_next(hi)) {
        const char *key = apr_hash_this_key(hi);
        apr_ssize_t len = apr_hash_this_key_len(hi);
        char *val = apr_hash_this_val(hi);

        str[i][0]='\0';
        apr_snprintf(str[i], MAX_LTH, "%sKey %s (%" APR_SSIZE_T_FMT ") Value %s\n",
                 str[i], key, len, val);
        i++;
    }
    str[i][0]='\0';
    apr_snprintf(str[i], MAX_LTH, "%s#entries %d\n", str[i], i);

    /* Sort the result strings so that they can be checked for expected results easily,
     * without having to worry about platform quirks
     */
    qsort(
        str, /* Pointer to elements */
        i,   /* number of elements */
        MAX_LTH, /* size of one element */
        comp_string /* Pointer to comparison routine */
    );
}

static void sum_hash(apr_pool_t *p, apr_hash_t *h, int *pcount, int *keySum, int *valSum) 
{
    apr_hash_index_t *hi;
    void *val, *key;
    int count = 0;

    *keySum = 0;
    *valSum = 0;
    *pcount = 0;
    for (hi = apr_hash_first(p, h); hi; hi = apr_hash_next(hi)) {
        apr_hash_this(hi, (void*)&key, NULL, &val);
        *valSum += *(int *)val;
        *keySum += *(int *)key;
        count++;
    }
    *pcount=count;
}

static void hash_make(abts_case *tc, void *data)
{
    apr_hash_t *h = NULL;

    h = apr_hash_make(p);
    ABTS_PTR_NOTNULL(tc, h);
}

static void hash_set(abts_case *tc, void *data)
{
    apr_hash_t *h = NULL;
    char *result = NULL;

    h = apr_hash_make(p);
    ABTS_PTR_NOTNULL(tc, h);

    apr_hash_set(h, "key", APR_HASH_KEY_STRING, "value");
    result = apr_hash_get(h, "key", APR_HASH_KEY_STRING);
    ABTS_STR_EQUAL(tc, "value", result);
}

static void hash_reset(abts_case *tc, void *data)
{
    apr_hash_t *h = NULL;
    char *result = NULL;

    h = apr_hash_make(p);
    ABTS_PTR_NOTNULL(tc, h);

    apr_hash_set(h, "key", APR_HASH_KEY_STRING, "value");
    result = apr_hash_get(h, "key", APR_HASH_KEY_STRING);
    ABTS_STR_EQUAL(tc, "value", result);

    apr_hash_set(h, "key", APR_HASH_KEY_STRING, "new");
    result = apr_hash_get(h, "key", APR_HASH_KEY_STRING);
    ABTS_STR_EQUAL(tc, "new", result);
}

static void same_value(abts_case *tc, void *data)
{
    apr_hash_t *h = NULL;
    char *result = NULL;

    h = apr_hash_make(p);
    ABTS_PTR_NOTNULL(tc, h);

    apr_hash_set(h, "same1", APR_HASH_KEY_STRING, "same");
    result = apr_hash_get(h, "same1", APR_HASH_KEY_STRING);
    ABTS_STR_EQUAL(tc, "same", result);

    apr_hash_set(h, "same2", APR_HASH_KEY_STRING, "same");
    result = apr_hash_get(h, "same2", APR_HASH_KEY_STRING);
    ABTS_STR_EQUAL(tc, "same", result);
}

static unsigned int hash_custom( const char *key, apr_ssize_t *klen)
{
    unsigned int hash = 0;
    while( *klen ) {
        (*klen) --;
        hash = hash * 33 + key[ *klen ];
    }
    return hash;
}

static void same_value_custom(abts_case *tc, void *data)
{
    apr_hash_t *h = NULL;
    char *result = NULL;

    h = apr_hash_make_custom(p, hash_custom);
    ABTS_PTR_NOTNULL(tc, h);

    apr_hash_set(h, "same1", 5, "same");
    result = apr_hash_get(h, "same1", 5);
    ABTS_STR_EQUAL(tc, "same", result);

    apr_hash_set(h, "same2", 5, "same");
    result = apr_hash_get(h, "same2", 5);
    ABTS_STR_EQUAL(tc, "same", result);
}

static void key_space(abts_case *tc, void *data)
{
    apr_hash_t *h = NULL;
    char *result = NULL;

    h = apr_hash_make(p);
    ABTS_PTR_NOTNULL(tc, h);

    apr_hash_set(h, "key with space", APR_HASH_KEY_STRING, "value");
    result = apr_hash_get(h, "key with space", APR_HASH_KEY_STRING);
    ABTS_STR_EQUAL(tc, "value", result);
}

static void hash_clear(abts_case *tc, void *data)
{
    apr_hash_t *h;
    int i, *e;

    h = apr_hash_make(p);
    ABTS_PTR_NOTNULL(tc, h);

    for (i = 1; i <= 10; i++) {
        e = apr_palloc(p, sizeof(int));
        *e = i;
        apr_hash_set(h, e, sizeof(*e), e);
    }
    apr_hash_clear(h);
    i = apr_hash_count(h);
    ABTS_INT_EQUAL(tc, 0, i);
}

/* This is kind of a hack, but I am just keeping an existing test.  This is
 * really testing apr_hash_first, apr_hash_next, and apr_hash_this which 
 * should be tested in three separate tests, but this will do for now.
 */
static void hash_traverse(abts_case *tc, void *data)
{
    apr_hash_t *h;
    char StrArray[MAX_DEPTH][MAX_LTH];

    h = apr_hash_make(p);
    ABTS_PTR_NOTNULL(tc, h);

    apr_hash_set(h, "OVERWRITE", APR_HASH_KEY_STRING, "should not see this");
    apr_hash_set(h, "FOO3", APR_HASH_KEY_STRING, "bar3");
    apr_hash_set(h, "FOO3", APR_HASH_KEY_STRING, "bar3");
    apr_hash_set(h, "FOO1", APR_HASH_KEY_STRING, "bar1");
    apr_hash_set(h, "FOO2", APR_HASH_KEY_STRING, "bar2");
    apr_hash_set(h, "FOO4", APR_HASH_KEY_STRING, "bar4");
    apr_hash_set(h, "SAME1", APR_HASH_KEY_STRING, "same");
    apr_hash_set(h, "SAME2", APR_HASH_KEY_STRING, "same");
    apr_hash_set(h, "OVERWRITE", APR_HASH_KEY_STRING, "Overwrite key");

    dump_hash(p, h, StrArray);

    ABTS_STR_EQUAL(tc, "Key FOO1 (4) Value bar1\n", StrArray[0]);
    ABTS_STR_EQUAL(tc, "Key FOO2 (4) Value bar2\n", StrArray[1]);
    ABTS_STR_EQUAL(tc, "Key FOO3 (4) Value bar3\n", StrArray[2]);
    ABTS_STR_EQUAL(tc, "Key FOO4 (4) Value bar4\n", StrArray[3]);
    ABTS_STR_EQUAL(tc, "Key OVERWRITE (9) Value Overwrite key\n", StrArray[4]);
    ABTS_STR_EQUAL(tc, "Key SAME1 (5) Value same\n", StrArray[5]);
    ABTS_STR_EQUAL(tc, "Key SAME2 (5) Value same\n", StrArray[6]);
    ABTS_STR_EQUAL(tc, "#entries 7\n", StrArray[7]);
}

/* This is kind of a hack, but I am just keeping an existing test.  This is
 * really testing apr_hash_first, apr_hash_next, and apr_hash_this which 
 * should be tested in three separate tests, but this will do for now.
 */
static void summation_test(abts_case *tc, void *data)
{
    apr_hash_t *h;
    int sumKeys, sumVal, trySumKey, trySumVal;
    int i, j, *val, *key;

    h =apr_hash_make(p);
    ABTS_PTR_NOTNULL(tc, h);

    sumKeys = 0;
    sumVal = 0;
    trySumKey = 0;
    trySumVal = 0;

    for (i = 0; i < 100; i++) {
        j = i * 10 + 1;
        sumKeys += j;
        sumVal += i;
        key = apr_palloc(p, sizeof(int));
        *key = j;
        val = apr_palloc(p, sizeof(int));
        *val = i;
        apr_hash_set(h, key, sizeof(int), val);
    }

    sum_hash(p, h, &i, &trySumKey, &trySumVal);
    ABTS_INT_EQUAL(tc, 100, i);
    ABTS_INT_EQUAL(tc, sumVal, trySumVal);
    ABTS_INT_EQUAL(tc, sumKeys, trySumKey);
}

static void delete_key(abts_case *tc, void *data)
{
    apr_hash_t *h = NULL;
    char *result = NULL;

    h = apr_hash_make(p);
    ABTS_PTR_NOTNULL(tc, h);

    apr_hash_set(h, "key", APR_HASH_KEY_STRING, "value");
    apr_hash_set(h, "key2", APR_HASH_KEY_STRING, "value2");

    result = apr_hash_get(h, "key", APR_HASH_KEY_STRING);
    ABTS_STR_EQUAL(tc, "value", result);

    result = apr_hash_get(h, "key2", APR_HASH_KEY_STRING);
    ABTS_STR_EQUAL(tc, "value2", result);

    apr_hash_set(h, "key", APR_HASH_KEY_STRING, NULL);

    result = apr_hash_get(h, "key", APR_HASH_KEY_STRING);
    ABTS_PTR_EQUAL(tc, NULL, result);

    result = apr_hash_get(h, "key2", APR_HASH_KEY_STRING);
    ABTS_STR_EQUAL(tc, "value2", result);
}

static void hash_count_0(abts_case *tc, void *data)
{
    apr_hash_t *h = NULL;
    int count;

    h = apr_hash_make(p);
    ABTS_PTR_NOTNULL(tc, h);

    count = apr_hash_count(h);
    ABTS_INT_EQUAL(tc, 0, count);
}

static void hash_count_1(abts_case *tc, void *data)
{
    apr_hash_t *h = NULL;
    int count;

    h = apr_hash_make(p);
    ABTS_PTR_NOTNULL(tc, h);

    apr_hash_set(h, "key", APR_HASH_KEY_STRING, "value");

    count = apr_hash_count(h);
    ABTS_INT_EQUAL(tc, 1, count);
}

static void hash_count_5(abts_case *tc, void *data)
{
    apr_hash_t *h = NULL;
    int count;

    h = apr_hash_make(p);
    ABTS_PTR_NOTNULL(tc, h);

    apr_hash_set(h, "key1", APR_HASH_KEY_STRING, "value1");
    apr_hash_set(h, "key2", APR_HASH_KEY_STRING, "value2");
    apr_hash_set(h, "key3", APR_HASH_KEY_STRING, "value3");
    apr_hash_set(h, "key4", APR_HASH_KEY_STRING, "value4");
    apr_hash_set(h, "key5", APR_HASH_KEY_STRING, "value5");

    count = apr_hash_count(h);
    ABTS_INT_EQUAL(tc, 5, count);
}

static void overlay_empty(abts_case *tc, void *data)
{
    apr_hash_t *base = NULL;
    apr_hash_t *overlay = NULL;
    apr_hash_t *result = NULL;
    int count;
    char StrArray[MAX_DEPTH][MAX_LTH];

    base = apr_hash_make(p);
    overlay = apr_hash_make(p);
    ABTS_PTR_NOTNULL(tc, base);
    ABTS_PTR_NOTNULL(tc, overlay);

    apr_hash_set(base, "key1", APR_HASH_KEY_STRING, "value1");
    apr_hash_set(base, "key2", APR_HASH_KEY_STRING, "value2");
    apr_hash_set(base, "key3", APR_HASH_KEY_STRING, "value3");
    apr_hash_set(base, "key4", APR_HASH_KEY_STRING, "value4");
    apr_hash_set(base, "key5", APR_HASH_KEY_STRING, "value5");

    result = apr_hash_overlay(p, overlay, base);

    count = apr_hash_count(result);
    ABTS_INT_EQUAL(tc, 5, count);

    dump_hash(p, result, StrArray);

    ABTS_STR_EQUAL(tc, "Key key1 (4) Value value1\n", StrArray[0]);
    ABTS_STR_EQUAL(tc, "Key key2 (4) Value value2\n", StrArray[1]);
    ABTS_STR_EQUAL(tc, "Key key3 (4) Value value3\n", StrArray[2]);
    ABTS_STR_EQUAL(tc, "Key key4 (4) Value value4\n", StrArray[3]);
    ABTS_STR_EQUAL(tc, "Key key5 (4) Value value5\n", StrArray[4]);
    ABTS_STR_EQUAL(tc, "#entries 5\n", StrArray[5]);
}

static void overlay_2unique(abts_case *tc, void *data)
{
    apr_hash_t *base = NULL;
    apr_hash_t *overlay = NULL;
    apr_hash_t *result = NULL;
    int count;
    char StrArray[MAX_DEPTH][MAX_LTH];

    base = apr_hash_make(p);
    overlay = apr_hash_make(p);
    ABTS_PTR_NOTNULL(tc, base);
    ABTS_PTR_NOTNULL(tc, overlay);

    apr_hash_set(base, "base1", APR_HASH_KEY_STRING, "value1");
    apr_hash_set(base, "base2", APR_HASH_KEY_STRING, "value2");
    apr_hash_set(base, "base3", APR_HASH_KEY_STRING, "value3");
    apr_hash_set(base, "base4", APR_HASH_KEY_STRING, "value4");
    apr_hash_set(base, "base5", APR_HASH_KEY_STRING, "value5");

    apr_hash_set(overlay, "overlay1", APR_HASH_KEY_STRING, "value1");
    apr_hash_set(overlay, "overlay2", APR_HASH_KEY_STRING, "value2");
    apr_hash_set(overlay, "overlay3", APR_HASH_KEY_STRING, "value3");
    apr_hash_set(overlay, "overlay4", APR_HASH_KEY_STRING, "value4");
    apr_hash_set(overlay, "overlay5", APR_HASH_KEY_STRING, "value5");

    result = apr_hash_overlay(p, overlay, base);

    count = apr_hash_count(result);
    ABTS_INT_EQUAL(tc, 10, count);

    dump_hash(p, result, StrArray);

    ABTS_STR_EQUAL(tc, "Key base1 (5) Value value1\n", StrArray[0]);
    ABTS_STR_EQUAL(tc, "Key base2 (5) Value value2\n", StrArray[1]);
    ABTS_STR_EQUAL(tc, "Key base3 (5) Value value3\n", StrArray[2]);
    ABTS_STR_EQUAL(tc, "Key base4 (5) Value value4\n", StrArray[3]);
    ABTS_STR_EQUAL(tc, "Key base5 (5) Value value5\n", StrArray[4]);
    ABTS_STR_EQUAL(tc, "Key overlay1 (8) Value value1\n", StrArray[5]);
    ABTS_STR_EQUAL(tc, "Key overlay2 (8) Value value2\n", StrArray[6]);
    ABTS_STR_EQUAL(tc, "Key overlay3 (8) Value value3\n", StrArray[7]);
    ABTS_STR_EQUAL(tc, "Key overlay4 (8) Value value4\n", StrArray[8]);
    ABTS_STR_EQUAL(tc, "Key overlay5 (8) Value value5\n", StrArray[9]);
    ABTS_STR_EQUAL(tc, "#entries 10\n", StrArray[10]);
}

static void overlay_same(abts_case *tc, void *data)
{
    apr_hash_t *base = NULL;
    apr_hash_t *result = NULL;
    int count;
    char StrArray[MAX_DEPTH][MAX_LTH];

    base = apr_hash_make(p);
    ABTS_PTR_NOTNULL(tc, base);

    apr_hash_set(base, "base1", APR_HASH_KEY_STRING, "value1");
    apr_hash_set(base, "base2", APR_HASH_KEY_STRING, "value2");
    apr_hash_set(base, "base3", APR_HASH_KEY_STRING, "value3");
    apr_hash_set(base, "base4", APR_HASH_KEY_STRING, "value4");
    apr_hash_set(base, "base5", APR_HASH_KEY_STRING, "value5");

    result = apr_hash_overlay(p, base, base);

    count = apr_hash_count(result);
    ABTS_INT_EQUAL(tc, 5, count);

    dump_hash(p, result, StrArray);

    ABTS_STR_EQUAL(tc, "Key base1 (5) Value value1\n", StrArray[0]);
    ABTS_STR_EQUAL(tc, "Key base2 (5) Value value2\n", StrArray[1]);
    ABTS_STR_EQUAL(tc, "Key base3 (5) Value value3\n", StrArray[2]);
    ABTS_STR_EQUAL(tc, "Key base4 (5) Value value4\n", StrArray[3]);
    ABTS_STR_EQUAL(tc, "Key base5 (5) Value value5\n", StrArray[4]);
    ABTS_STR_EQUAL(tc, "#entries 5\n", StrArray[5]);
}

static void overlay_fetch(abts_case *tc, void *data)
{
    apr_hash_t *base = NULL;
    apr_hash_t *overlay = NULL;
    apr_hash_t *result = NULL;
    int count;

    base = apr_hash_make(p);
    overlay = apr_hash_make(p);
    ABTS_PTR_NOTNULL(tc, base);
    ABTS_PTR_NOTNULL(tc, overlay);

    apr_hash_set(base, "base1", APR_HASH_KEY_STRING, "value1");
    apr_hash_set(base, "base2", APR_HASH_KEY_STRING, "value2");
    apr_hash_set(base, "base3", APR_HASH_KEY_STRING, "value3");
    apr_hash_set(base, "base4", APR_HASH_KEY_STRING, "value4");
    apr_hash_set(base, "base5", APR_HASH_KEY_STRING, "value5");

    apr_hash_set(overlay, "overlay1", APR_HASH_KEY_STRING, "value1");
    apr_hash_set(overlay, "overlay2", APR_HASH_KEY_STRING, "value2");
    apr_hash_set(overlay, "overlay3", APR_HASH_KEY_STRING, "value3");
    apr_hash_set(overlay, "overlay4", APR_HASH_KEY_STRING, "value4");
    apr_hash_set(overlay, "overlay5", APR_HASH_KEY_STRING, "value5");

    result = apr_hash_overlay(p, overlay, base);

    count = apr_hash_count(result);
    ABTS_INT_EQUAL(tc, 10, count);

    ABTS_STR_EQUAL(tc, "value1",
                       apr_hash_get(result, "base1", APR_HASH_KEY_STRING));
    ABTS_STR_EQUAL(tc, "value2",
                       apr_hash_get(result, "base2", APR_HASH_KEY_STRING));
    ABTS_STR_EQUAL(tc, "value3",
                       apr_hash_get(result, "base3", APR_HASH_KEY_STRING));
    ABTS_STR_EQUAL(tc, "value4",
                       apr_hash_get(result, "base4", APR_HASH_KEY_STRING));
    ABTS_STR_EQUAL(tc, "value5",
                       apr_hash_get(result, "base5", APR_HASH_KEY_STRING));
    ABTS_STR_EQUAL(tc, "value1",
                       apr_hash_get(result, "overlay1", APR_HASH_KEY_STRING));
    ABTS_STR_EQUAL(tc, "value2",
                       apr_hash_get(result, "overlay2", APR_HASH_KEY_STRING));
    ABTS_STR_EQUAL(tc, "value3",
                       apr_hash_get(result, "overlay3", APR_HASH_KEY_STRING));
    ABTS_STR_EQUAL(tc, "value4",
                       apr_hash_get(result, "overlay4", APR_HASH_KEY_STRING));
    ABTS_STR_EQUAL(tc, "value5",
                       apr_hash_get(result, "overlay5", APR_HASH_KEY_STRING));

    ABTS_STR_EQUAL(tc, "value1",
                       apr_hash_get(base, "base1", APR_HASH_KEY_STRING));
    ABTS_STR_EQUAL(tc, "value2",
                       apr_hash_get(base, "base2", APR_HASH_KEY_STRING));
    ABTS_STR_EQUAL(tc, "value3",
                       apr_hash_get(base, "base3", APR_HASH_KEY_STRING));
    ABTS_STR_EQUAL(tc, "value4",
                       apr_hash_get(base, "base4", APR_HASH_KEY_STRING));
    ABTS_STR_EQUAL(tc, "value5",
                       apr_hash_get(base, "base5", APR_HASH_KEY_STRING));

    ABTS_STR_EQUAL(tc, "value1",
                       apr_hash_get(overlay, "overlay1", APR_HASH_KEY_STRING));
    ABTS_STR_EQUAL(tc, "value2",
                       apr_hash_get(overlay, "overlay2", APR_HASH_KEY_STRING));
    ABTS_STR_EQUAL(tc, "value3",
                       apr_hash_get(overlay, "overlay3", APR_HASH_KEY_STRING));
    ABTS_STR_EQUAL(tc, "value4",
                       apr_hash_get(overlay, "overlay4", APR_HASH_KEY_STRING));
    ABTS_STR_EQUAL(tc, "value5",
                       apr_hash_get(overlay, "overlay5", APR_HASH_KEY_STRING));
}

abts_suite *testhash(abts_suite *suite)
{
    suite = ADD_SUITE(suite)

    abts_run_test(suite, hash_make, NULL);
    abts_run_test(suite, hash_set, NULL);
    abts_run_test(suite, hash_reset, NULL);
    abts_run_test(suite, same_value, NULL);
    abts_run_test(suite, same_value_custom, NULL);
    abts_run_test(suite, key_space, NULL);
    abts_run_test(suite, delete_key, NULL);

    abts_run_test(suite, hash_count_0, NULL);
    abts_run_test(suite, hash_count_1, NULL);
    abts_run_test(suite, hash_count_5, NULL);

    abts_run_test(suite, hash_clear, NULL);
    abts_run_test(suite, hash_traverse, NULL);
    abts_run_test(suite, summation_test, NULL);

    abts_run_test(suite, overlay_empty, NULL);
    abts_run_test(suite, overlay_2unique, NULL);
    abts_run_test(suite, overlay_same, NULL);
    abts_run_test(suite, overlay_fetch, NULL);

    return suite;
}

