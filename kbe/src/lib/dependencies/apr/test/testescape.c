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

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "apr_escape.h"
#include "apr_strings.h"

#include "abts.h"
#include "testutil.h"

static void test_escape(abts_case *tc, void *data)
{
    apr_pool_t *pool;
    const char *src, *target;
    const char *dest;
    const void *vdest;
    apr_size_t len, vlen;

    apr_pool_create(&pool, NULL);

    src = "Hello World &;`'\"|*?~<>^()[]{}$\\";
    target = "Hello World \\&\\;\\`\\'\\\"\\|\\*\\?\\~\\<\\>\\^\\(\\)\\[\\]\\{\\}\\$\\\\";
    dest = apr_pescape_shell(pool, src);
    ABTS_ASSERT(tc,
                apr_psprintf(pool, "shell escaped (%s) does not match expected output (%s)",
                             dest, target),
                (strcmp(dest, target) == 0));
    apr_escape_shell(NULL, src, APR_ESCAPE_STRING, &len);
    ABTS_ASSERT(tc,
            apr_psprintf(pool, "size mismatch (%" APR_SIZE_T_FMT "!=%" APR_SIZE_T_FMT ")", len, strlen(dest) + 1),
            (len == strlen(dest) + 1));

#if !(defined(OS2) || defined(WIN32))
    /* Now try with newline, which is converted to a space on OS/2 and Windows.
     */
    src = "Hello World &;`'\"|*?~<>^()[]{}$\\\n";
    target = "Hello World \\&\\;\\`\\'\\\"\\|\\*\\?\\~\\<\\>\\^\\(\\)\\[\\]\\{\\}\\$\\\\\\\n";
    dest = apr_pescape_shell(pool, src);
    ABTS_ASSERT(tc,
                apr_psprintf(pool, "shell escaped (%s) does not match expected output (%s)",
                             dest, target),
                (strcmp(dest, target) == 0));
    apr_escape_shell(NULL, src, APR_ESCAPE_STRING, &len);
    ABTS_ASSERT(tc,
            apr_psprintf(pool, "size mismatch (%" APR_SIZE_T_FMT "!=%" APR_SIZE_T_FMT ")", len, strlen(dest) + 1),
            (len == strlen(dest) + 1));
#endif

    src = "Hello";
    dest = apr_punescape_url(pool, src, NULL, NULL, 0);
    ABTS_PTR_EQUAL(tc, src, dest);

    src = "Hello";
    dest = apr_punescape_url(pool, src, NULL, NULL, 1);
    ABTS_PTR_EQUAL(tc, src, dest);

    src = "Hello%20";
    dest = apr_punescape_url(pool, src, " ", NULL, 0);
    ABTS_PTR_EQUAL(tc, NULL, dest);

    src = "Hello%20World";
    target = "Hello World";
    dest = apr_punescape_url(pool, src, NULL, NULL, 0);
    ABTS_STR_EQUAL(tc, target, dest);
    apr_unescape_url(NULL, src, APR_ESCAPE_STRING, NULL, NULL, 0, &len);
    ABTS_ASSERT(tc,
            apr_psprintf(pool, "size mismatch (%" APR_SIZE_T_FMT "!=%" APR_SIZE_T_FMT ")", len, strlen(dest) + 1),
            (len == strlen(dest) + 1));

    src = "Hello+World";
    target = "Hello World";
    dest = apr_punescape_url(pool, src, NULL, NULL, 1);
    ABTS_STR_EQUAL(tc, target, dest);
    apr_unescape_url(NULL, src, APR_ESCAPE_STRING, NULL, NULL, 1, &len);
    ABTS_ASSERT(tc,
            apr_psprintf(pool, "size mismatch (%" APR_SIZE_T_FMT "!=%" APR_SIZE_T_FMT ")", len, strlen(dest) + 1),
            (len == strlen(dest) + 1));

    src = "Hello%20World";
    target = "Hello%20World";
    dest = apr_punescape_url(pool, src, NULL, " ", 0);
    ABTS_STR_EQUAL(tc, target, dest);
    apr_unescape_url(NULL, src, APR_ESCAPE_STRING, NULL, " ", 0, &len);
    ABTS_ASSERT(tc,
            apr_psprintf(pool, "size mismatch (%" APR_SIZE_T_FMT "!=%" APR_SIZE_T_FMT ")", len, strlen(dest) + 1),
            (len == strlen(dest) + 1));

    src = "Hello";
    dest = apr_pescape_path_segment(pool, src);
    ABTS_PTR_EQUAL(tc, src, dest);

    src = "$-_.+!*'(),:@&=/~Hello World";
    target = "$-_.+!*'(),:@&=%2f~Hello%20World";
    dest = apr_pescape_path_segment(pool, src);
    ABTS_STR_EQUAL(tc, target, dest);
    apr_escape_path_segment(NULL, src, APR_ESCAPE_STRING, &len);
    ABTS_ASSERT(tc,
            apr_psprintf(pool, "size mismatch (%" APR_SIZE_T_FMT "!=%" APR_SIZE_T_FMT ")", len, strlen(dest) + 1),
            (len == strlen(dest) + 1));

    src = "Hello";
    dest = apr_pescape_path(pool, src, 0);
    ABTS_PTR_EQUAL(tc, src, dest);

    src = "$-_.+!*'(),:@&=/~Hello World";
    target = "./$-_.+!*'(),:@&=/~Hello%20World";
    dest = apr_pescape_path(pool, src, 0);
    ABTS_STR_EQUAL(tc, target, dest);
    apr_escape_path(NULL, src, APR_ESCAPE_STRING, 0, &len);
    ABTS_ASSERT(tc,
            apr_psprintf(pool, "size mismatch (%" APR_SIZE_T_FMT "!=%" APR_SIZE_T_FMT ")", len, strlen(dest) + 1),
            (len == strlen(dest) + 1));

    src = "Hello";
    dest = apr_pescape_path(pool, src, 1);
    ABTS_PTR_EQUAL(tc, src, dest);

    src = "$-_.+!*'(),:@&=/~Hello World";
    target = "$-_.+!*'(),:@&=/~Hello%20World";
    dest = apr_pescape_path(pool, src, 1);
    ABTS_STR_EQUAL(tc, target, dest);
    apr_escape_path(NULL, src, APR_ESCAPE_STRING, 1, &len);
    ABTS_ASSERT(tc,
            apr_psprintf(pool, "size mismatch (%" APR_SIZE_T_FMT "!=%" APR_SIZE_T_FMT ")", len, strlen(dest) + 1),
            (len == strlen(dest) + 1));

    src = "Hello";
    dest = apr_pescape_urlencoded(pool, src);
    ABTS_PTR_EQUAL(tc, src, dest);

    src = "$-_.+!*'(),:@&=/~Hello World";
    target = "%24-_.%2b%21*%27%28%29%2c%3a%40%26%3d%2f%7eHello+World";
    dest = apr_pescape_urlencoded(pool, src);
    ABTS_STR_EQUAL(tc, target, dest);
    apr_escape_urlencoded(NULL, src, APR_ESCAPE_STRING, &len);
    ABTS_ASSERT(tc,
            apr_psprintf(pool, "size mismatch (%" APR_SIZE_T_FMT "!=%" APR_SIZE_T_FMT ")", len, strlen(dest) + 1),
            (len == strlen(dest) + 1));

    src = "Hello";
    dest = apr_pescape_entity(pool, src, 0);
    ABTS_PTR_EQUAL(tc, src, dest);

    src = "\xFF<>&\'\"Hello World";
    target = "\xFF&lt;&gt;&amp;'&quot;Hello World";
    dest = apr_pescape_entity(pool, src, 0);
    ABTS_STR_EQUAL(tc, target, dest);
    apr_escape_entity(NULL, src, APR_ESCAPE_STRING, 0, &len);
    ABTS_ASSERT(tc,
            apr_psprintf(pool, "size mismatch (%" APR_SIZE_T_FMT "!=%" APR_SIZE_T_FMT ")", len, strlen(dest) + 1),
            (len == strlen(dest) + 1));

#if !APR_CHARSET_EBCDIC
    src = "Hello";
    dest = apr_pescape_entity(pool, src, 1);
    ABTS_PTR_EQUAL(tc, src, dest);

    src = "\xFF<>&\'\"Hello World";
    target = "&#255&lt;&gt;&amp;'&quot;Hello World";
    dest = apr_pescape_entity(pool, src, 1);
    ABTS_STR_EQUAL(tc, target, dest);
    apr_escape_entity(NULL, src, APR_ESCAPE_STRING, 1, &len);
    ABTS_ASSERT(tc,
            apr_psprintf(pool, "size mismatch (%" APR_SIZE_T_FMT "!=%" APR_SIZE_T_FMT ")", len, strlen(dest) + 1),
            (len == strlen(dest) + 1));

    src = "Hello";
    dest = apr_punescape_entity(pool, src);
    ABTS_PTR_EQUAL(tc, src, dest);

    src = "\xFF&lt;&gt;&amp;'&quot;Hello World";
    target = "\xFF<>&\'\"Hello World";
    dest = apr_punescape_entity(pool, src);
    ABTS_STR_EQUAL(tc, target, dest);
    apr_unescape_entity(NULL, src, APR_ESCAPE_STRING, &len);
    ABTS_ASSERT(tc,
            apr_psprintf(pool, "size mismatch (%" APR_SIZE_T_FMT "!=%" APR_SIZE_T_FMT ")", len, strlen(dest) + 1),
            (len == strlen(dest) + 1));

    src = "&#255;&lt;&gt;&amp;'&quot;Hello World";
    target = "\xFF<>&\'\"Hello World";
    dest = apr_punescape_entity(pool, src);
    ABTS_STR_EQUAL(tc, target, dest);
    apr_unescape_entity(NULL, src, APR_ESCAPE_STRING, &len);
    ABTS_ASSERT(tc,
            apr_psprintf(pool, "size mismatch (%" APR_SIZE_T_FMT "!=%" APR_SIZE_T_FMT ")", len, strlen(dest) + 1),
            (len == strlen(dest) + 1));

    src = "&#32;&lt;&gt;&amp;'&quot;Hello World";
    target = " <>&\'\"Hello World";
    dest = apr_punescape_entity(pool, src);
    ABTS_STR_EQUAL(tc, target, dest);
    apr_unescape_entity(NULL, src, APR_ESCAPE_STRING, &len);
    ABTS_ASSERT(tc,
            apr_psprintf(pool, "size mismatch (%" APR_SIZE_T_FMT "!=%" APR_SIZE_T_FMT ")", len, strlen(dest) + 1),
            (len == strlen(dest) + 1));
#endif

    src = "Hello";
    dest = apr_pescape_echo(pool, src, 0);
    ABTS_PTR_EQUAL(tc, src, dest);

    src = "\a\b\f\\n\r\t\v\"Hello World\"";
    target = "\\a\\b\\f\\\\n\\r\\t\\v\"Hello World\"";
    dest = apr_pescape_echo(pool, src, 0);
    ABTS_STR_EQUAL(tc, target, dest);
    apr_escape_echo(NULL, src, APR_ESCAPE_STRING, 0, &len);
    ABTS_ASSERT(tc,
            apr_psprintf(pool, "size mismatch (%" APR_SIZE_T_FMT "!=%" APR_SIZE_T_FMT ")", len, strlen(dest) + 1),
            (len == strlen(dest) + 1));

    src = "\a\b\f\\n\r\t\v\"Hello World\"";
    target = "\\a\\b\\f\\\\n\\r\\t\\v\\\"Hello World\\\"";
    dest = apr_pescape_echo(pool, src, 1);
    ABTS_STR_EQUAL(tc, target, dest);
    apr_escape_echo(NULL, src, APR_ESCAPE_STRING, 1, &len);
    ABTS_ASSERT(tc,
            apr_psprintf(pool, "size mismatch (%" APR_SIZE_T_FMT "!=%" APR_SIZE_T_FMT ")", len, strlen(dest) + 1),
            (len == strlen(dest) + 1));

    src = "\xFF\x00\xFF\x00";
    target = "ff00ff00";
    dest = apr_pescape_hex(pool, src, 4, 0);
    ABTS_STR_EQUAL(tc, target, dest);
    apr_escape_hex(NULL, src, 4, 0, &len);
    ABTS_ASSERT(tc,
            apr_psprintf(pool, "size mismatch (%" APR_SIZE_T_FMT "!=%" APR_SIZE_T_FMT ")", len, strlen(dest) + 1),
            (len == strlen(dest) + 1));

    src = "\xFF\x00\xFF\x00";
    target = "ff:00:ff:00";
    dest = apr_pescape_hex(pool, src, 4, 1);
    ABTS_STR_EQUAL(tc, target, dest);
    apr_escape_hex(NULL, src, 4, 1, &len);
    ABTS_ASSERT(tc,
            apr_psprintf(pool, "size mismatch (%" APR_SIZE_T_FMT "!=%" APR_SIZE_T_FMT ")", len, strlen(dest) + 1),
            (len == strlen(dest) + 1));

    src = "ff:00:ff:00";
    target = "\xFF\x00\xFF\x00";
    vdest = apr_punescape_hex(pool, src, 1, &vlen);
    ABTS_ASSERT(tc, "apr_punescape_hex target!=dest", memcmp(target, vdest, 4) == 0);
    ABTS_INT_EQUAL(tc, (int)vlen, 4);
    apr_unescape_hex(NULL, src, APR_ESCAPE_STRING, 1, &len);
    ABTS_ASSERT(tc,
            apr_psprintf(pool, "size mismatch (%" APR_SIZE_T_FMT "!=%" APR_SIZE_T_FMT ")", len, (apr_size_t)4),
            (len == 4));

    apr_pool_destroy(pool);
}

abts_suite *testescape(abts_suite *suite)
{
    suite = ADD_SUITE(suite);

    abts_run_test(suite, test_escape, NULL);

    return suite;
}
