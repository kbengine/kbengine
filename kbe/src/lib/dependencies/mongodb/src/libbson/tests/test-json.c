#include <bson.h>
#include <bcon.h>
#include <assert.h>
#include <fcntl.h>
#include <stdio.h>

#include "bson-tests.h"
#include "TestSuite.h"

#ifndef BINARY_DIR
# define BINARY_DIR "tests/binary"
#endif

#ifndef JSON_DIR
# define JSON_DIR "tests/json"
#endif


static void
test_bson_as_json (void)
{
   bson_oid_t oid;
   bson_t *b;
   bson_t *b2;
   char *str;
   size_t len;
   int i;

   bson_oid_init_from_string(&oid, "123412341234abcdabcdabcd");

   b = bson_new();
   assert(bson_append_utf8(b, "utf8", -1, "bar", -1));
   assert(bson_append_int32(b, "int32", -1, 1234));
   assert(bson_append_int64(b, "int64", -1, 4321));
   assert(bson_append_double(b, "double", -1, 123.4));
   assert(bson_append_undefined(b, "undefined", -1));
   assert(bson_append_null(b, "null", -1));
   assert(bson_append_oid(b, "oid", -1, &oid));
   assert(bson_append_bool(b, "true", -1, true));
   assert(bson_append_bool(b, "false", -1, false));
   assert(bson_append_time_t(b, "date", -1, time(NULL)));
   assert(bson_append_timestamp(b, "timestamp", -1, (uint32_t)time(NULL), 1234));
   assert(bson_append_regex(b, "regex", -1, "^abcd", "xi"));
   assert(bson_append_dbpointer(b, "dbpointer", -1, "mycollection", &oid));
   assert(bson_append_minkey(b, "minkey", -1));
   assert(bson_append_maxkey(b, "maxkey", -1));
   assert(bson_append_symbol(b, "symbol", -1, "var a = {};", -1));

   b2 = bson_new();
   assert(bson_append_int32(b2, "0", -1, 60));
   assert(bson_append_document(b, "document", -1, b2));
   assert(bson_append_array(b, "array", -1, b2));

   {
      const uint8_t binary[] = { 0, 1, 2, 3, 4 };
      assert(bson_append_binary(b, "binary", -1, BSON_SUBTYPE_BINARY,
                                binary, sizeof binary));
   }

   for (i = 0; i < 1000; i++) {
      str = bson_as_json(b, &len);
      bson_free(str);
   }

   bson_destroy(b);
   bson_destroy(b2);
}


static void
test_bson_as_json_string (void)
{
   size_t len;
   bson_t *b;
   char *str;

   b = bson_new();
   assert(bson_append_utf8(b, "foo", -1, "bar", -1));
   str = bson_as_json(b, &len);
   assert(len == 17);
   assert(!strcmp("{ \"foo\" : \"bar\" }", str));
   bson_free(str);
   bson_destroy(b);
}


static void
test_bson_as_json_int32 (void)
{
   size_t len;
   bson_t *b;
   char *str;

   b = bson_new();
   assert(bson_append_int32(b, "foo", -1, 1234));
   str = bson_as_json(b, &len);
   assert(len == 16);
   assert(!strcmp("{ \"foo\" : 1234 }", str));
   bson_free(str);
   bson_destroy(b);
}


static void
test_bson_as_json_int64 (void)
{
   size_t len;
   bson_t *b;
   char *str;

   b = bson_new();
   assert(bson_append_int64(b, "foo", -1, 341234123412341234ULL));
   str = bson_as_json(b, &len);
   assert(len == 30);
   assert(!strcmp("{ \"foo\" : 341234123412341234 }", str));
   bson_free(str);
   bson_destroy(b);
}


static void
test_bson_as_json_double (void)
{
   size_t len;
   bson_t *b;
   char *str;

   b = bson_new();
   assert(bson_append_double(b, "foo", -1, 123.456));
   assert(bson_append_double(b, "bar", -1, 3));
   assert(bson_append_double(b, "baz", -1, -1));
   assert(bson_append_double(b, "quux", -1, 1.0001));
   assert(bson_append_double(b, "huge", -1, 1e99));
   str = bson_as_json(b, &len);
   ASSERT_CMPSTR (str,
                  "{"
                  " \"foo\" : 123.456,"
                  " \"bar\" : 3,"
                  " \"baz\" : -1,"
                  " \"quux\" : 1.0001,"
                  " \"huge\" : 1e+99 }");
   bson_free(str);
   bson_destroy(b);
}


static void
test_bson_as_json_utf8 (void)
{
   /* euro currency symbol */
#define EU "\xe2\x82\xac"
#define FIVE_EUROS EU EU EU EU EU
   size_t len;
   bson_t *b;
   char *str;

   b = bson_new();
   assert(bson_append_utf8(b, FIVE_EUROS, -1, FIVE_EUROS, -1));
   str = bson_as_json(b, &len);
   assert(!strcmp(str, "{ \"" FIVE_EUROS "\" : \"" FIVE_EUROS "\" }"));
   bson_free(str);
   bson_destroy(b);
}


static void
test_bson_as_json_stack_overflow (void)
{
   uint8_t *buf;
   bson_t b;
   size_t buflen = 1024 * 1024 * 17;
   char *str;
   int fd;
   ssize_t r;

   buf = bson_malloc0(buflen);

   fd = bson_open(BINARY_DIR"/stackoverflow.bson", O_RDONLY);
   BSON_ASSERT(-1 != fd);

   r = bson_read(fd, buf, buflen);
   BSON_ASSERT(r == 16777220);

   r = bson_init_static(&b, buf, 16777220);
   BSON_ASSERT(r);

   str = bson_as_json(&b, NULL);
   BSON_ASSERT(str);

   r = !!strstr(str, "...");
   BSON_ASSERT(str);

   bson_free(str);
   bson_destroy(&b);
   bson_free(buf);
}


static void
test_bson_corrupt (void)
{
   uint8_t *buf;
   bson_t b;
   size_t buflen = 1024;
   char *str;
   int fd;
   ssize_t r;

   buf = bson_malloc0(buflen);

   fd = bson_open(BINARY_DIR"/test55.bson", O_RDONLY);
   BSON_ASSERT(-1 != fd);

   r = bson_read(fd, buf, buflen);
   BSON_ASSERT(r == 24);

   r = bson_init_static(&b, buf, (uint32_t)r);
   BSON_ASSERT(r);

   str = bson_as_json(&b, NULL);
   BSON_ASSERT(!str);

   bson_destroy(&b);
   bson_free(buf);
}

static void
test_bson_corrupt_utf8 (void)
{
   uint8_t *buf;
   bson_t b;
   size_t buflen = 1024;
   char *str;
   int fd;
   ssize_t r;

   buf = bson_malloc0(buflen);

   fd = bson_open(BINARY_DIR"/test56.bson", O_RDONLY);
   BSON_ASSERT(-1 != fd);

   r = bson_read(fd, buf, buflen);
   BSON_ASSERT(r == 42);

   r = bson_init_static(&b, buf, (uint32_t)r);
   BSON_ASSERT(r);

   str = bson_as_json(&b, NULL);
   BSON_ASSERT(!str);

   bson_destroy(&b);
   bson_free(buf);
}


static void
test_bson_corrupt_binary (void)
{
   uint8_t *buf;
   bson_t b;
   size_t buflen = 1024;
   char *str;
   int fd;
   ssize_t r;

   buf = bson_malloc0(buflen);

   fd = bson_open(BINARY_DIR"/test57.bson", O_RDONLY);
   BSON_ASSERT(-1 != fd);

   r = bson_read(fd, buf, buflen);
   BSON_ASSERT(r == 26);

   r = bson_init_static(&b, buf, (uint32_t)r);
   BSON_ASSERT(r);

   str = bson_as_json(&b, NULL);
   BSON_ASSERT(!str);

   bson_destroy(&b);
   bson_free(buf);
}

static void
_test_bson_json_read_compare (const char *json,
                              int         size,
                              ...)
{
   bson_error_t error = { 0 };
   bson_json_reader_t *reader;
   va_list ap;
   int r;
   bson_t *compare;
   bson_t bson = BSON_INITIALIZER;

   reader = bson_json_data_reader_new ((size == 1), size);
   bson_json_data_reader_ingest(reader, (uint8_t *)json, strlen(json));

   va_start (ap, size);

   while ((r = bson_json_reader_read (reader, &bson, &error))) {
      if (r == -1) {
         fprintf (stderr, "%s\n", error.message);
         abort ();
      }

      compare = va_arg (ap, bson_t *);

      assert (compare);

      bson_eq_bson (&bson, compare);

      bson_destroy (compare);

      bson_reinit (&bson);
   }

   va_end (ap);

   bson_json_reader_destroy (reader);
   bson_destroy (&bson);
}

static void
test_bson_json_read(void)
{
   const char * json = "{ \n\
      \"foo\" : \"bar\", \n\
      \"bar\" : 12341, \n\
      \"baz\" : 123.456, \n\
      \"map\" : { \"a\" : 1 }, \n\
      \"array\" : [ 1, 2, 3, 4 ], \n\
      \"null\" : null, \n\
      \"boolean\" : true, \n\
      \"oid\" : { \n\
        \"$oid\" : \"000000000000000000000000\" \n\
      }, \n\
      \"binary\" : { \n\
        \"$type\" : \"00\", \n\
        \"$binary\" : \"ZGVhZGJlZWY=\" \n\
      }, \n\
      \"regex\" : { \n\
        \"$regex\" : \"foo|bar\", \n\
        \"$options\" : \"ism\" \n\
      }, \n\
      \"date\" : { \n\
        \"$date\" : 10000 \n\
      }, \n\
      \"ref\" : { \n\
        \"$ref\" : \"foo\", \n\
        \"$id\" : {\"$oid\": \"000000000000000000000000\"} \n\
      }, \n\
      \"undefined\" : { \n\
        \"$undefined\" : true \n\
      }, \n\
      \"minkey\" : { \n\
        \"$minKey\" : 1 \n\
      }, \n\
      \"maxkey\" : { \n\
        \"$maxKey\" : 1 \n\
      }, \n\
      \"timestamp\" : { \n\
        \"$timestamp\" : { \n\
           \"t\" : 100, \n\
           \"i\" : 1000 \n\
        } \n\
      } \n\
   } { \"after\": \"b\" } { \"twice\" : true }";

   bson_oid_t oid;
   bson_t * first, *second, *third;

   bson_oid_init_from_string(&oid, "000000000000000000000000");

   first = BCON_NEW(
      "foo", "bar",
      "bar", BCON_INT32(12341),
      "baz", BCON_DOUBLE(123.456),
      "map", "{",
         "a", BCON_INT32(1),
      "}",
      "array", "[", BCON_INT32(1), BCON_INT32(2), BCON_INT32(3), BCON_INT32(4), "]",
      "null", BCON_NULL,
      "boolean", BCON_BOOL(true),
      "oid", BCON_OID(&oid),
      "binary", BCON_BIN(BSON_SUBTYPE_BINARY, (const uint8_t *)"deadbeef", 8),
      "regex", BCON_REGEX("foo|bar", "ism"),
      "date", BCON_DATE_TIME(10000),
      "ref", "{", "$ref", BCON_UTF8("foo"), "$id", BCON_OID(&oid), "}",
      "undefined", BCON_UNDEFINED,
      "minkey", BCON_MINKEY,
      "maxkey", BCON_MAXKEY,
      "timestamp", BCON_TIMESTAMP(100, 1000)
   );

   second = BCON_NEW("after", "b");
   third = BCON_NEW("twice", BCON_BOOL(true));

   _test_bson_json_read_compare(json, 5, first, second, third, NULL);
}

static void
test_json_reader_new_from_file (void)
{
   const char *path = JSON_DIR"/test.json";
   const char *bar;
   const bson_oid_t *oid;
   bson_oid_t oid_expected;
   int32_t one;
   bson_t bson = BSON_INITIALIZER;
   bson_json_reader_t *reader;
   bson_error_t error;

   reader = bson_json_reader_new_from_file (path, &error);
   assert (reader);

   /* read two documents */
   ASSERT_CMPINT (1, ==, bson_json_reader_read (reader, &bson, &error));

   BCON_EXTRACT (&bson, "foo", BCONE_UTF8 (bar), "a", BCONE_INT32 (one));
   ASSERT_CMPSTR ("bar", bar);
   ASSERT_CMPINT (1, ==, one);

   bson_reinit (&bson);
   ASSERT_CMPINT (1, ==, bson_json_reader_read (reader, &bson, &error));

   BCON_EXTRACT (&bson, "_id", BCONE_OID (oid));
   bson_oid_init_from_string (&oid_expected, "aabbccddeeff001122334455");
   assert (bson_oid_equal (&oid_expected, oid));

   bson_destroy (&bson);
   bson_json_reader_destroy (reader);
}

static void
test_json_reader_new_from_bad_path (void)
{
   const char *bad_path = JSON_DIR"/does-not-exist";
   bson_json_reader_t *reader;
   bson_error_t error;

   reader = bson_json_reader_new_from_file (bad_path, &error);
   assert (!reader);
   ASSERT_CMPINT (BSON_ERROR_READER, ==, error.domain);
   ASSERT_CMPINT (BSON_ERROR_READER_BADFD, ==, error.code);
}

static void
test_bson_json_error (const char              *json,
                      int                      domain,
                      bson_json_error_code_t   code)
{
   bson_error_t error;
   bson_t * bson;

   bson = bson_new_from_json ((const uint8_t *)json, strlen(json), &error);

   assert (! bson);
   assert (error.domain == domain);
   assert (error.code == code);
}

static void
test_bson_json_read_missing_complex(void)
{
   const char * json = "{ \n\
      \"foo\" : { \n\
         \"$options\" : \"ism\"\n\
      }\n\
   }";

   test_bson_json_error (json, BSON_ERROR_JSON,
                         BSON_JSON_ERROR_READ_INVALID_PARAM);
}

static void
test_bson_json_read_invalid_json(void)
{
   const char *json = "{ \n\
      \"foo\" : { \n\
   }";
   bson_t *b;

   test_bson_json_error (json, BSON_ERROR_JSON,
                         BSON_JSON_ERROR_READ_CORRUPT_JS);

   b = bson_new_from_json ((uint8_t *)"1", 1, NULL);
   assert (!b);

   b = bson_new_from_json ((uint8_t *)"*", 1, NULL);
   assert (!b);

   b = bson_new_from_json ((uint8_t *)"", 0, NULL);
   assert (!b);

   b = bson_new_from_json ((uint8_t *)"asdfasdf", -1, NULL);
   assert (!b);

   b = bson_new_from_json ((uint8_t *)"{\"a\":*}", -1, NULL);
   assert (!b);
}

static ssize_t
test_bson_json_read_bad_cb_helper(void *_ctx, uint8_t * buf, size_t len)
{
   return -1;
}

static void
test_bson_json_read_bad_cb(void)
{
   bson_error_t error;
   bson_json_reader_t *reader;
   int r;
   bson_t bson = BSON_INITIALIZER;

   reader = bson_json_reader_new (NULL, &test_bson_json_read_bad_cb_helper, NULL, false, 0);

   r = bson_json_reader_read (reader, &bson, &error);

   assert(r == -1);
   assert(error.domain == BSON_ERROR_JSON);
   assert(error.code == BSON_JSON_ERROR_READ_CB_FAILURE);

   bson_json_reader_destroy (reader);
   bson_destroy (&bson);
}

static ssize_t
test_bson_json_read_invalid_helper (void *ctx, uint8_t *buf, size_t len)
{
   assert (len);
   *buf = 0x80;  /* no UTF-8 sequence can start with 0x80 */
   return 1;
}

static void
test_bson_json_read_invalid(void)
{
   bson_error_t error;
   bson_json_reader_t *reader;
   int r;
   bson_t bson = BSON_INITIALIZER;

   reader = bson_json_reader_new (NULL, test_bson_json_read_invalid_helper, NULL, false, 0);

   r = bson_json_reader_read (reader, &bson, &error);

   assert(r == -1);
   assert(error.domain == BSON_ERROR_JSON);
   assert(error.code == BSON_JSON_ERROR_READ_CORRUPT_JS);

   bson_json_reader_destroy (reader);
   bson_destroy (&bson);
}

static void
test_bson_json_number_long (void)
{
   bson_error_t error;
   bson_iter_t iter;
   const char *json = "{ \"key\": { \"$numberLong\": \"4611686018427387904\" }}";
   const char *json2 = "{ \"key\": { \"$numberLong\": \"461168601abcd\" }}";
   bson_t b;
   bool r;

   r = bson_init_from_json (&b, json, -1, &error);
   if (!r) fprintf (stderr, "%s\n", error.message);
   assert (r);
   assert (bson_iter_init (&iter, &b));
   assert (bson_iter_find (&iter, "key"));
   assert (BSON_ITER_HOLDS_INT64 (&iter));
   assert (bson_iter_int64 (&iter) == 4611686018427387904LL);
   bson_destroy (&b);

   assert (!bson_init_from_json (&b, json2, -1, &error));
}

static const bson_oid_t *
oid_zero (void)
{
   static bool initialized = false;
   static bson_oid_t oid;

   if (!initialized) {
      bson_oid_init_from_string (&oid, "000000000000000000000000");
      initialized = true;
   }

   return &oid;
}

static void
test_bson_json_dbref (void)
{
   bson_error_t error;

   const char *json_with_objectid =
      "{ \"key\": {"
      "\"$ref\": \"collection\","
      "\"$id\": {\"$oid\": \"000000000000000000000000\"}}}";

   bson_t *bson_with_objectid = BCON_NEW (
      "key", "{",
      "$ref", BCON_UTF8 ("collection"),
      "$id", BCON_OID (oid_zero ()), "}");

   const char *json_with_int_id = "{ \"key\": {"
      "\"$ref\": \"collection\","
      "\"$id\": 1}}}";

   bson_t *bson_with_int_id = BCON_NEW (
      "key", "{",
      "$ref", BCON_UTF8 ("collection"),
      "$id", BCON_INT32 (1), "}");

   const char *json_with_subdoc_id = "{ \"key\": {"
      "\"$ref\": \"collection\","
      "\"$id\": {\"a\": 1}}}";

   bson_t *bson_with_subdoc_id = BCON_NEW (
      "key", "{",
      "$ref", BCON_UTF8 ("collection"),
      "$id", "{", "a", BCON_INT32 (1), "}", "}");

   const char *json_with_metadata = "{ \"key\": {"
      "\"$ref\": \"collection\","
      "\"$id\": 1,"
      "\"meta\": true}}";

   bson_t *bson_with_metadata = BCON_NEW (
      "key", "{",
      "$ref", BCON_UTF8 ("collection"),
      "$id", BCON_INT32 (1),
      "meta", BCON_BOOL (true), "}");

   bson_t b;
   bool r;

   typedef struct {
      const char *json;
      bson_t *expected_bson;
   } dbref_test_t;

   dbref_test_t tests[] = {
      {json_with_objectid, bson_with_objectid},
      {json_with_int_id, bson_with_int_id},
      {json_with_subdoc_id, bson_with_subdoc_id},
      {json_with_metadata, bson_with_metadata},
   };

   int n_tests = sizeof (tests) / sizeof (dbref_test_t);
   int i;

   for (i = 0; i < n_tests; i++) {
      r = bson_init_from_json (&b, tests[i].json, -1, &error);
      if (!r) {
         fprintf (stderr, "%s\n", error.message);
      }

      assert (r);
      bson_eq_bson (&b, tests[i].expected_bson);
      bson_destroy (&b);
   }

   for (i = 0; i < n_tests; i++) {
      bson_destroy (tests[i].expected_bson);
   }
}

static void
test_bson_json_inc (void)
{
   /* test that reproduces a bug with special mode checking.  Specifically,
    * mistaking '$inc' for '$id'
    *
    * From https://github.com/mongodb/mongo-c-driver/issues/62
    */
   bson_error_t error;
   const char *json = "{ \"$inc\" : { \"ref\" : 1 } }";
   bson_t b;
   bool r;

   r = bson_init_from_json (&b, json, -1, &error);
   if (!r) fprintf (stderr, "%s\n", error.message);
   assert (r);
   bson_destroy (&b);
}

static void
test_bson_json_array (void)
{
   bson_error_t error;
   const char *json = "[ 0, 1, 2, 3 ]";
   bson_t b, compare;
   bool r;

   bson_init(&compare);
   bson_append_int32(&compare, "0", 1, 0);
   bson_append_int32(&compare, "1", 1, 1);
   bson_append_int32(&compare, "2", 1, 2);
   bson_append_int32(&compare, "3", 1, 3);

   r = bson_init_from_json (&b, json, -1, &error);
   if (!r) fprintf (stderr, "%s\n", error.message);
   assert (r);

   bson_eq_bson (&b, &compare);
   bson_destroy (&compare);
   bson_destroy (&b);
}

static void
test_bson_json_date_check (bool        should_work,
                           const char *json,
                           int64_t     value)
{
   bson_error_t error = { 0 };
   bson_t b, compare;
   bool r;

   if (should_work) {
      bson_init (&compare);

      BSON_APPEND_DATE_TIME (&compare, "dt", value);

      r = bson_init_from_json (&b, json, -1, &error);

      if (!r) { fprintf (stderr, "%s\n", error.message); }

      assert (r);

      bson_eq_bson (&b, &compare);
      bson_destroy (&compare);
      bson_destroy (&b);
   } else {
      r = bson_init_from_json (&b, json, -1, &error);

      if (r) { fprintf (stderr, "parsing %s should fail\n", json); }

      assert (!r);
   }
}

static void
test_bson_json_date (void)
{
   test_bson_json_date_check (true,
                              "{ \"dt\" : { \"$date\" : \"1970-01-01T00:00:00.000Z\" } }",
                              0);
   test_bson_json_date_check (true,
                              "{ \"dt\" : { \"$date\" : \"1969-12-31T16:00:00.000-0800\" } }",
                              0);
   test_bson_json_date_check (true,
                              "{ \"dt\" : { \"$date\" : -62135593139000 } }",
                              -62135593139000);
   test_bson_json_date_check (true,
                              "{ \"dt\" : { \"$date\" : { \"$numberLong\" : \"-62135593139000\" } } }",
                              -62135593139000);

   test_bson_json_date_check (false,
                              "{ \"dt\" : { \"$date\" : \"1970-01-01T01:00:00.000+01:00\" } }",
                              0);
}

static void
test_bson_array_as_json (void)
{
   bson_t d = BSON_INITIALIZER;
   size_t len;
   char *str;

   str = bson_array_as_json (&d, &len);
   assert (0 == strcmp (str, "[ ]"));
   assert (len == 3);
   bson_free (str);

   BSON_APPEND_INT32 (&d, "0", 1);
   str = bson_array_as_json (&d, &len);
   assert (0 == strcmp (str, "[ 1 ]"));
   assert (len == 5);
   bson_free (str);

   bson_destroy (&d);
}


static void
test_bson_as_json_spacing (void)
{
   bson_t d = BSON_INITIALIZER;
   size_t len;
   char *str;

   str = bson_as_json (&d, &len);
   assert (0 == strcmp (str, "{ }"));
   assert (len == 3);
   bson_free (str);

   BSON_APPEND_INT32 (&d, "a", 1);
   str = bson_as_json (&d, &len);
   assert (0 == strcmp (str, "{ \"a\" : 1 }"));
   assert (len == 11);
   bson_free (str);

   bson_destroy (&d);
}

void
test_json_install (TestSuite *suite)
{
   TestSuite_Add (suite, "/bson/as_json/x1000", test_bson_as_json);
   TestSuite_Add (suite, "/bson/as_json/string", test_bson_as_json_string);
   TestSuite_Add (suite, "/bson/as_json/int32", test_bson_as_json_int32);
   TestSuite_Add (suite, "/bson/as_json/int64", test_bson_as_json_int64);
   TestSuite_Add (suite, "/bson/as_json/double", test_bson_as_json_double);
   TestSuite_Add (suite, "/bson/as_json/utf8", test_bson_as_json_utf8);
   TestSuite_Add (suite, "/bson/as_json/stack_overflow", test_bson_as_json_stack_overflow);
   TestSuite_Add (suite, "/bson/as_json/corrupt", test_bson_corrupt);
   TestSuite_Add (suite, "/bson/as_json/corrupt_utf8", test_bson_corrupt_utf8);
   TestSuite_Add (suite, "/bson/as_json/corrupt_binary", test_bson_corrupt_binary);
   TestSuite_Add (suite, "/bson/as_json_spacing", test_bson_as_json_spacing);
   TestSuite_Add (suite, "/bson/array_as_json", test_bson_array_as_json);
   TestSuite_Add (suite, "/bson/json/read", test_bson_json_read);
   TestSuite_Add (suite, "/bson/json/inc", test_bson_json_inc);
   TestSuite_Add (suite, "/bson/json/array", test_bson_json_array);
   TestSuite_Add (suite, "/bson/json/date", test_bson_json_date);
   TestSuite_Add (suite, "/bson/json/read/missing_complex", test_bson_json_read_missing_complex);
   TestSuite_Add (suite, "/bson/json/read/invalid_json", test_bson_json_read_invalid_json);
   TestSuite_Add (suite, "/bson/json/read/bad_cb", test_bson_json_read_bad_cb);
   TestSuite_Add (suite, "/bson/json/read/invalid", test_bson_json_read_invalid);
   TestSuite_Add (suite, "/bson/json/read/file", test_json_reader_new_from_file);
   TestSuite_Add (suite, "/bson/json/read/bad_path", test_json_reader_new_from_bad_path);
   TestSuite_Add (suite, "/bson/json/read/invalid", test_bson_json_read_invalid);
   TestSuite_Add (suite, "/bson/json/read/invalid", test_bson_json_read_invalid);
   TestSuite_Add (suite, "/bson/json/read/$numberLong", test_bson_json_number_long);
   TestSuite_Add (suite, "/bson/json/read/dbref", test_bson_json_dbref);
}
