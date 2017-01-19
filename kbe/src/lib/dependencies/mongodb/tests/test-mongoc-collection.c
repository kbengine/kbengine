#include <bcon.h>
#include <mongoc.h>
#include <mongoc-client-private.h>
#include <mongoc-cursor-private.h>
#include <mongoc-collection-private.h>

#include "TestSuite.h"

#include "test-libmongoc.h"
#include "test-conveniences.h"
#include "mongoc-tests.h"
#include "mock_server/future-functions.h"
#include "mock_server/mock-server.h"


static mongoc_database_t *
get_test_database (mongoc_client_t *client)
{
   return mongoc_client_get_database (client, "test");
}


static mongoc_collection_t *
get_test_collection (mongoc_client_t *client,
                     const char      *prefix)
{
   mongoc_collection_t *ret;
   char *str;

   str = gen_collection_name (prefix);
   ret = mongoc_client_get_collection (client, "test", str);
   bson_free (str);

   return ret;
}


static void
test_copy (void)
{
   mongoc_database_t *database;
   mongoc_collection_t *collection;
   mongoc_collection_t *copy;
   mongoc_client_t *client;

   client = test_framework_client_new ();
   ASSERT (client);

   database = get_test_database (client);
   ASSERT (database);

   collection = get_test_collection (client, "test_insert");
   ASSERT (collection);

   copy = mongoc_collection_copy(collection);
   ASSERT (copy);
   ASSERT (copy->client == collection->client);
   ASSERT (strcmp(copy->ns, collection->ns) == 0);

   mongoc_collection_destroy(copy);
   mongoc_collection_destroy(collection);
   mongoc_database_destroy(database);
   mongoc_client_destroy(client);
}


static void
test_insert (void)
{
   mongoc_database_t *database;
   mongoc_collection_t *collection;
   mongoc_client_t *client;
   bson_context_t *context;
   bson_error_t error;
   bool r;
   bson_oid_t oid;
   unsigned i;
   bson_t b;


   client = test_framework_client_new ();
   ASSERT (client);

   database = get_test_database (client);
   ASSERT (database);

   collection = get_test_collection (client, "test_insert");
   ASSERT (collection);

   mongoc_collection_drop(collection, &error);

   context = bson_context_new(BSON_CONTEXT_NONE);
   ASSERT (context);

   for (i = 0; i < 10; i++) {
      bson_init(&b);
      bson_oid_init(&oid, context);
      bson_append_oid(&b, "_id", 3, &oid);
      bson_append_utf8(&b, "hello", 5, "/world", 5);
      ASSERT_OR_PRINT (mongoc_collection_insert(
         collection, MONGOC_INSERT_NONE, &b, NULL, &error), error);

      bson_destroy(&b);
   }

   bson_init (&b);
   BSON_APPEND_INT32 (&b, "$hello", 1);
   r = mongoc_collection_insert (collection, MONGOC_INSERT_NONE, &b, NULL,
                                 &error);
   ASSERT (!r);
   ASSERT (error.domain == MONGOC_ERROR_BSON);
   ASSERT (error.code == MONGOC_ERROR_BSON_INVALID);
   bson_destroy (&b);

   ASSERT_OR_PRINT (mongoc_collection_drop (collection, &error),
                    error);

   mongoc_collection_destroy(collection);
   mongoc_database_destroy(database);
   bson_context_destroy(context);
   mongoc_client_destroy(client);
}

/* CDRIVER-759, a 2.4 mongos responds to getLastError after an oversized insert:
 *
 * { err: "assertion src/mongo/s/strategy_shard.cpp:461", n: 0, ok: 1.0 }
 *
 * There's an "err" but no "code".
*/

static void
test_legacy_insert_oversize_mongos (void)
{
   mock_server_t *server;
   mongoc_client_t *client;
   mongoc_collection_t *collection;
   bson_t b = BSON_INITIALIZER;
   bson_error_t error;
   future_t *future;
   request_t *request;

   server = mock_server_with_autoismaster (0);
   mock_server_run (server);

   client = mongoc_client_new_from_uri (mock_server_get_uri (server));
      ASSERT (client);

   collection = mongoc_client_get_collection (client, "test", "test");
   future = future_collection_insert (collection, MONGOC_INSERT_NONE,
                                      &b, NULL, &error);

   request = mock_server_receives_insert (server, "test.test",
                                          MONGOC_INSERT_NONE, "{}");
   request_destroy (request);
   request = mock_server_receives_gle (server, "test");
   mock_server_replies_simple (request, "{'err': 'oh no!', 'n': 0, 'ok': 1}");
   ASSERT (!future_get_bool (future));
   ASSERT_ERROR_CONTAINS (error,
                          MONGOC_ERROR_COMMAND,
                          MONGOC_ERROR_COLLECTION_INSERT_FAILED,
                          "oh no!");

   request_destroy (request);
   future_destroy (future);
   mongoc_collection_destroy(collection);
   mongoc_client_destroy(client);
   mock_server_destroy (server);
}


static void
test_insert_bulk (void)
{
   mongoc_collection_t *collection;
   mongoc_database_t *database;
   mongoc_client_t *client;
   bson_context_t *context;
   bson_error_t error;
   bool r;
   bson_oid_t oid;
   unsigned i;
   bson_t q;
   bson_t b[10];
   bson_t *bptr[10];
   int64_t count;

   client = test_framework_client_new ();
   ASSERT (client);

   database = get_test_database (client);
   ASSERT (database);

   collection = get_test_collection (client, "test_insert_bulk");
   ASSERT (collection);

   mongoc_collection_drop(collection, &error);

   context = bson_context_new(BSON_CONTEXT_NONE);
   ASSERT (context);

   bson_init(&q);
   bson_append_int32(&q, "n", -1, 0);

   for (i = 0; i < 10; i++) {
      bson_init(&b[i]);
      bson_oid_init(&oid, context);
      bson_append_oid(&b[i], "_id", -1, &oid);
      bson_append_int32(&b[i], "n", -1, i % 2);
      bptr[i] = &b[i];
   }

   BEGIN_IGNORE_DEPRECATIONS;
   ASSERT_OR_PRINT (mongoc_collection_insert_bulk (collection,
                                                   MONGOC_INSERT_NONE,
                                                   (const bson_t **)bptr,
                                                   10, NULL, &error), error);
   END_IGNORE_DEPRECATIONS;

   count = mongoc_collection_count (collection, MONGOC_QUERY_NONE, &q,
                                    0, 0, NULL, &error);
   ASSERT (count == 5);

   for (i = 8; i < 10; i++) {
      bson_destroy(&b[i]);
      bson_init(&b[i]);
      bson_oid_init(&oid, context);
      bson_append_oid(&b[i], "_id", -1, &oid);
      bson_append_int32(&b[i], "n", -1, i % 2);
      bptr[i] = &b[i];
   }

   BEGIN_IGNORE_DEPRECATIONS;
   r = mongoc_collection_insert_bulk (collection, MONGOC_INSERT_NONE,
                                      (const bson_t **)bptr, 10, NULL, &error);
   END_IGNORE_DEPRECATIONS;

   ASSERT (!r);
   ASSERT (error.code == 11000);

   count = mongoc_collection_count (collection, MONGOC_QUERY_NONE,
                                    &q, 0, 0, NULL, &error);
   ASSERT (count == 5);

   BEGIN_IGNORE_DEPRECATIONS;
   r = mongoc_collection_insert_bulk (collection, MONGOC_INSERT_CONTINUE_ON_ERROR,
                                      (const bson_t **)bptr, 10, NULL, &error);
   END_IGNORE_DEPRECATIONS;
   ASSERT (!r);
   ASSERT (error.code == 11000);

   count = mongoc_collection_count (collection, MONGOC_QUERY_NONE,
                                    &q, 0, 0, NULL, &error);
   ASSERT (count == 6);

   /* test validate */
   for (i = 0; i < 10; i++) {
      bson_destroy (&b[i]);
      bson_init (&b[i]);
      BSON_APPEND_INT32 (&b[i], "$invalid_dollar_prefixed_name", i);
      bptr[i] = &b[i];
   }
   BEGIN_IGNORE_DEPRECATIONS;
   r = mongoc_collection_insert_bulk (collection, MONGOC_INSERT_NONE,
                                      (const bson_t **)bptr, 10, NULL, &error);
   END_IGNORE_DEPRECATIONS;
   ASSERT (!r);
   ASSERT (error.domain == MONGOC_ERROR_BSON);
   ASSERT (error.code == MONGOC_ERROR_BSON_INVALID);

   bson_destroy(&q);
   for (i = 0; i < 10; i++) {
      bson_destroy(&b[i]);
   }

   ASSERT_OR_PRINT (mongoc_collection_drop (collection, &error), error);

   mongoc_collection_destroy(collection);
   mongoc_database_destroy(database);
   bson_context_destroy(context);
   mongoc_client_destroy(client);
}


static void
test_insert_bulk_empty (void)
{
   mongoc_collection_t *collection;
   mongoc_database_t *database;
   mongoc_client_t *client;
   bson_error_t error;
   bson_t *bptr = NULL;

   client = test_framework_client_new ();
   database = get_test_database (client);
   collection = get_test_collection (client, "test_insert_bulk_empty");

   BEGIN_IGNORE_DEPRECATIONS;
   ASSERT (!mongoc_collection_insert_bulk (collection,
                                           MONGOC_INSERT_NONE,
                                           (const bson_t **)&bptr,
                                           0, NULL, &error));
   END_IGNORE_DEPRECATIONS;

   ASSERT_CMPINT (MONGOC_ERROR_COLLECTION, ==, error.domain);
   ASSERT_CMPINT (MONGOC_ERROR_COLLECTION_INSERT_FAILED, ==, error.code);
   ASSERT_CONTAINS (error.message, "empty insert");

   mongoc_collection_destroy(collection);
   mongoc_database_destroy(database);
   mongoc_client_destroy(client);
}


static void
auto_ismaster (mock_server_t *server,
               int32_t max_wire_version,
               int32_t max_message_size,
               int32_t max_bson_size,
               int32_t max_batch_size)
{
   char *response = bson_strdup_printf (
      "{'ismaster': true, "
      " 'maxWireVersion': %d,"
      " 'maxBsonObjectSize': %d,"
      " 'maxMessageSizeBytes': %d,"
      " 'maxWriteBatchSize': %d }",
      max_wire_version, max_bson_size, max_message_size, max_batch_size);

   mock_server_auto_ismaster (server, response);

   bson_free (response);
}


char *
make_string (size_t len)
{
   char *s = (char *)bson_malloc (len);

   memset (s, 'a', len - 1);
   s[len - 1] = '\0';

   return s;
}


bson_t *
make_document (size_t bytes)
{
   bson_t *bson;
   bson_oid_t oid;
   char *s;
   size_t string_len;

   bson_oid_init (&oid, NULL);
   bson = bson_new ();
   BSON_APPEND_OID (bson, "_id", &oid);

   /* make the document exactly n bytes by appending a string. a string has
    * 7 bytes overhead (1 for type code, 2 for key, 4 for length prefix), so
    * make the string (n_bytes - current_length - 7) bytes long. */
   ASSERT_CMPUINT((unsigned int)bytes, >=, bson->len + 7);
   string_len = bytes - bson->len - 7;
   s = make_string (string_len);
   BSON_APPEND_UTF8 (bson, "s", s);
   bson_free (s);
   ASSERT_CMPUINT ((unsigned int) bytes, ==, bson->len);

   return bson;
}


void
make_bulk_insert (bson_t **bsons,
                  int n,
                  size_t bytes)
{
   int i;

   for (i = 0; i < n; i++) {
      bsons[i] = make_document (bytes);
   }
}


static void
destroy_all (bson_t **ptr,
             int n)
{
   int i;

   for (i = 0; i < n; i++) {
      bson_destroy (ptr[i]);
   }
}


static void
receive_bulk (mock_server_t *server,
              int n,
              mongoc_insert_flags_t flags)
{
   request_t *request;

   request = mock_server_receives_bulk_insert (server, "test.test",
                                               flags, n);
   assert (request);
   request_destroy (request);

   request = mock_server_receives_gle (server, "test");
   mock_server_replies_simple (request, "{'ok': 1.0, 'n': 0, 'err': null}");
   request_destroy (request);
}


static void
test_legacy_bulk_insert_large (void)
{
   enum { N_BSONS = 10 };

   mock_server_t *server;
   mongoc_client_t *client;
   mongoc_collection_t *collection;
   bson_t *bsons[N_BSONS];
   bson_error_t error;
   future_t *future;

   server = mock_server_new ();
   mock_server_run (server);

   client = mongoc_client_new_from_uri (mock_server_get_uri (server));
   ASSERT (client);

   collection = mongoc_client_get_collection (client, "test", "test");

   /* docs size 50 bytes each */
   make_bulk_insert (bsons, N_BSONS, 50);

   /* max message of 240 bytes, so 4 docs per batch, 3 batches. */
   auto_ismaster (server,
                  0,     /* max_wire_version */
                  240,   /* max_message_size */
                  1000,  /* max_bson_size */
                  1000); /* max_write_batch_size */

   future = future_collection_insert_bulk (collection, MONGOC_INSERT_NONE,
                                           (const bson_t **) bsons, 10, NULL,
                                           &error);
   receive_bulk (server, 4, MONGOC_INSERT_NONE);
   receive_bulk (server, 4, MONGOC_INSERT_NONE);
   receive_bulk (server, 2, MONGOC_INSERT_NONE);

   ASSERT_OR_PRINT (future_get_bool (future), error);

   future_destroy (future);
   destroy_all (bsons, N_BSONS);
   mongoc_collection_destroy(collection);
   mongoc_client_destroy(client);
   mock_server_destroy (server);
}


/* verify an insert command's "documents" array has keys "0", "1", "2", ... */
static void
verify_keys (uint32_t n_documents,
             const bson_t *insert_command)
{
   bson_iter_t iter;
   uint32_t len;
   const uint8_t *data;
   bson_t document;
   char str[16];
   const char *key;
   uint32_t i;

   ASSERT (bson_iter_init_find (&iter, insert_command, "documents"));
   bson_iter_array (&iter, &len, &data);
   ASSERT (bson_init_static (&document, data, len));

   for (i = 0; i < n_documents; i++) {
      bson_uint32_to_string (i, &key, str, sizeof str);
      ASSERT (bson_has_field (&document, key));
   }
}


/* CDRIVER-845: "insert" command must have array keys "0", "1", "2", ... */
static void
test_insert_command_keys (void)
{
   mock_server_t *server;
   mongoc_client_t *client;
   mongoc_collection_t *collection;
   mongoc_bulk_operation_t *bulk;
   uint32_t i;
   bson_t *doc;
   bson_t reply;
   bson_error_t error;
   future_t *future;
   request_t *request;

   /* maxWireVersion 3 allows write commands */
   server = mock_server_with_autoismaster (3);
   mock_server_run (server);

   client = mongoc_client_new_from_uri (mock_server_get_uri (server));
   collection = mongoc_client_get_collection (client, "test", "test");
   bulk = mongoc_collection_create_bulk_operation (collection, true, NULL);

   for (i = 0; i < 3; i++) {
      doc = BCON_NEW ("_id", BCON_INT32 (i));
      mongoc_bulk_operation_insert (bulk, doc);
      bson_destroy (doc);
   }

   future = future_bulk_operation_execute (bulk, &reply, &error);
   request = mock_server_receives_command (server, "test", MONGOC_QUERY_NONE,
                                           "{'insert': 'test'}");

   verify_keys (3, request_get_doc (request, 0));
   mock_server_replies_simple (request, "{'ok': 1}");

   ASSERT_OR_PRINT (future_get_uint32_t (future), error);

   bson_destroy (&reply);
   future_destroy (future);
   request_destroy (request);
   mongoc_bulk_operation_destroy (bulk);
   mongoc_collection_destroy(collection);
   mongoc_client_destroy(client);
   mock_server_destroy (server);
}


/* number of docs that should go in a batch that starts at "offset" */
int
expected_batch_size (const bson_t **bsons,
                     int n_bsons,
                     int max_message_size,
                     int max_bson_size,
                     bool continue_on_err,
                     int *offset,            /* IN / OUT */
                     bool *has_oversized)    /* OUT */
{
   int batch_sz = 0;
   int msg_sz = 0;
   bool oversized;
   int n_oversized = 0;

   for (; *offset < n_bsons; (*offset)++) {
      oversized = (bsons[*offset]->len > max_bson_size);

      if (oversized) {
         n_oversized++;

         if (!continue_on_err) {
            /* stop here */
            return batch_sz;
         }
      } else {
         /* not oversized, regular document */
         msg_sz += bsons[*offset]->len;

         if (msg_sz >= max_message_size) {
            /* batch is full of regular documents */
            break;
         }

         batch_sz++;
      }
   }

   *has_oversized = (bool) n_oversized;

   return batch_sz;
}


static void
_test_legacy_bulk_insert (const bson_t **bsons,
                          int n_bsons,
                          bool continue_on_err,
                          const char *err_msg,
                          const char *gle_json,
                          ...)
{
   const int MAX_MESSAGE_SIZE = 300;
   const int MAX_BSON_SIZE = 200;

   va_list args;
   char *gle_json_formatted;
   mock_server_t *server;
   mongoc_client_t *client;
   mongoc_collection_t *collection;
   bson_error_t error;
   future_t *future;
   mongoc_insert_flags_t flags;
   int offset;
   bool has_oversized = false;
   int batch_sz;
   const bson_t *gle;

   va_start (args, gle_json);
   gle_json_formatted = bson_strdupv_printf (gle_json, args);
   va_end (args);
   
   server = mock_server_new ();
   mock_server_run (server);

   client = mongoc_client_new_from_uri (mock_server_get_uri (server));
   ASSERT (client);

   collection = mongoc_client_get_collection (client, "test", "test");

   auto_ismaster (server,
                  0,
                  MAX_MESSAGE_SIZE,
                  MAX_BSON_SIZE,
                  1 /* max_write_batch_size, irrelevant */ );

   flags = continue_on_err ?
           MONGOC_INSERT_CONTINUE_ON_ERROR :
           MONGOC_INSERT_NONE;

   future = future_collection_insert_bulk (
      collection, flags, bsons, (uint32_t) n_bsons, NULL, &error);

   offset = 0;

   /* mock server receives each batch. check each is the right size. */
   while ((batch_sz = expected_batch_size (bsons, n_bsons,
                                           MAX_MESSAGE_SIZE, MAX_BSON_SIZE,
                                           continue_on_err,
                                           &offset, &has_oversized))) {
      receive_bulk (server, batch_sz, flags);
      if (has_oversized && !continue_on_err) {
         break;
      }
   }

   /* mongoc_collection_insert_bulk returns false, there was an error */
   assert (!future_get_bool (future));

   /* TODO: CDRIVER-662, should always be MONGOC_ERROR_BSON */
   assert (
      (error.domain == MONGOC_ERROR_COMMAND) ||
      (error.domain == MONGOC_ERROR_BSON &&
       error.code == MONGOC_ERROR_BSON_INVALID));

   ASSERT_STARTSWITH (error.message, err_msg);

   gle = mongoc_collection_get_last_error (collection);
   assert (gle);

   /* TODO: should contain inserted ids, CDRIVER-703 */
   ASSERT_MATCH (gle, gle_json_formatted);

   future_destroy (future);
   mongoc_collection_destroy(collection);
   mongoc_client_destroy(client);
   mock_server_destroy (server);
   bson_free (gle_json_formatted);
}


static void
_test_legacy_bulk_insert_oversized_middle (bool continue_on_err)
{
   enum { N_BSONS = 5 };
   
   bson_t *bsons[N_BSONS];

   /* first batch */
   bsons[0] = make_document (100);
   bsons[1] = make_document (100);

   /* second batch */
   bsons[2] = make_document (100);
   bsons[3] = make_document (300);  /* too big */

   /* final batch, only sent if continue_on_err */
   bsons[4] = make_document (100);

   _test_legacy_bulk_insert (
      (const bson_t **) bsons, N_BSONS, continue_on_err,
      "Document 3 is too large",
      "{'nInserted': %d,"
      " 'nMatched': 0,"
      " 'nRemoved': 0,"
      " 'nUpserted': 0,"
      " 'writeErrors': [{'index': 3}]}",
      continue_on_err ? 4 : 3);

   destroy_all (bsons, N_BSONS);
}

static void
test_legacy_bulk_insert_oversized_middle (void)
{
   _test_legacy_bulk_insert_oversized_middle (false);
}


static void
test_legacy_bulk_insert_oversized_continue_middle (void)
{
   _test_legacy_bulk_insert_oversized_middle (true);
}


static void
_test_legacy_bulk_insert_oversized_first (bool continue_on_err)
{
   enum { N_BSONS = 2 };

   bson_t *bsons[N_BSONS];

   bsons[0] = make_document (300);   /* too big */ 
   bsons[1] = make_document (100);

   _test_legacy_bulk_insert (
      (const bson_t **) bsons, N_BSONS, continue_on_err,
      "Document 0 is too large",
      "{'nInserted': %d,"
      " 'nMatched': 0,"
      " 'nRemoved': 0,"
      " 'nUpserted': 0,"
      " 'writeErrors': [{'index': 0}]}",
      continue_on_err ? 1 : 0);

   destroy_all (bsons, N_BSONS);
}


static void
test_legacy_bulk_insert_oversized_first (void)
{
   _test_legacy_bulk_insert_oversized_first (false);
}


static void
test_legacy_bulk_insert_oversized_first_continue (void)
{
   _test_legacy_bulk_insert_oversized_first (true);
}


static void
_test_legacy_bulk_insert_oversized_last (bool continue_on_err)
{
   enum { N_BSONS = 2 };

   bson_t *bsons[N_BSONS];

   bsons[0] = make_document (100);
   bsons[1] = make_document (300);  /* too big */

   _test_legacy_bulk_insert (
      (const bson_t **) bsons, N_BSONS, continue_on_err,
      "Document 1 is too large",
      "{'nInserted': 1,"
      " 'nMatched': 0,"
      " 'nRemoved': 0,"
      " 'nUpserted': 0,"
      " 'writeErrors': [{'index': 1}]}");

   destroy_all (bsons, N_BSONS);
}


static void
test_legacy_bulk_insert_oversized_last (void)
{
   _test_legacy_bulk_insert_oversized_last (false);
}



static void
test_legacy_bulk_insert_oversized_last_continue (void)
{
   _test_legacy_bulk_insert_oversized_last (true);
}


static void
test_save (void)
{
   mongoc_collection_t *collection;
   mongoc_database_t *database;
   mongoc_client_t *client;
   bson_context_t *context;
   bson_error_t error;
   bson_oid_t oid;
   unsigned i;
   bson_t b;

   client = test_framework_client_new ();
   ASSERT (client);

   database = get_test_database (client);
   ASSERT (database);

   collection = get_test_collection (client, "test_save");
   ASSERT (collection);

   mongoc_collection_drop (collection, &error);

   context = bson_context_new(BSON_CONTEXT_NONE);
   ASSERT (context);

   for (i = 0; i < 10; i++) {
      bson_init(&b);
      bson_oid_init(&oid, context);
      bson_append_oid(&b, "_id", 3, &oid);
      bson_append_utf8(&b, "hello", 5, "/world", 5);
      ASSERT_OR_PRINT (mongoc_collection_save(collection, &b, NULL, &error),
                       error);
      bson_destroy(&b);
   }

   bson_destroy (&b);

   ASSERT_OR_PRINT (mongoc_collection_drop (collection, &error), error);

   mongoc_collection_destroy(collection);
   mongoc_database_destroy(database);
   bson_context_destroy(context);
   mongoc_client_destroy(client);
}


static void
test_regex (void)
{
   mongoc_collection_t *collection;
   mongoc_database_t *database;
   mongoc_write_concern_t *wr;
   mongoc_client_t *client;
   bson_error_t error;
   int64_t count;
   bson_t q = BSON_INITIALIZER;
   bson_t *doc;

   client = test_framework_client_new ();
   ASSERT (client);

   database = get_test_database (client);
   ASSERT (database);

   collection = get_test_collection (client, "test_regex");
   ASSERT (collection);

   wr = mongoc_write_concern_new ();
   mongoc_write_concern_set_journal (wr, true);

   doc = BCON_NEW ("hello", "/world");
   ASSERT_OR_PRINT (mongoc_collection_insert (collection, MONGOC_INSERT_NONE,
                                              doc, wr, &error), error);

   BSON_APPEND_REGEX (&q, "hello", "^/wo", "i");

   count = mongoc_collection_count (collection,
                                    MONGOC_QUERY_NONE,
                                    &q,
                                    0,
                                    0,
                                    NULL,
                                    &error);

   ASSERT (count > 0);
   ASSERT_OR_PRINT (mongoc_collection_drop (collection, &error), error);

   mongoc_write_concern_destroy (wr);
   bson_destroy (&q);
   bson_destroy (doc);
   mongoc_collection_destroy (collection);
   mongoc_database_destroy(database);
   mongoc_client_destroy (client);
}


static void
test_update (void)
{
   mongoc_collection_t *collection;
   mongoc_database_t *database;
   mongoc_client_t *client;
   bson_context_t *context;
   bson_error_t error;
   bool r;
   bson_oid_t oid;
   unsigned i;
   bson_t b;
   bson_t q;
   bson_t u;
   bson_t set;

   client = test_framework_client_new ();
   ASSERT (client);

   database = get_test_database (client);
   ASSERT (database);

   collection = get_test_collection (client, "test_update");
   ASSERT (collection);

   context = bson_context_new(BSON_CONTEXT_NONE);
   ASSERT (context);

   for (i = 0; i < 10; i++) {
      bson_init(&b);
      bson_oid_init(&oid, context);
      bson_append_oid(&b, "_id", 3, &oid);
      bson_append_utf8(&b, "utf8", 4, "utf8 string", 11);
      bson_append_int32(&b, "int32", 5, 1234);
      bson_append_int64(&b, "int64", 5, 12345678);
      bson_append_bool(&b, "bool", 4, 1);

      ASSERT_OR_PRINT (mongoc_collection_insert(collection, MONGOC_INSERT_NONE,
                                                &b, NULL, &error), error);

      bson_init(&q);
      bson_append_oid(&q, "_id", 3, &oid);

      bson_init(&u);
      bson_append_document_begin(&u, "$set", 4, &set);
      bson_append_utf8(&set, "utf8", 4, "updated", 7);
      bson_append_document_end(&u, &set);

      ASSERT_OR_PRINT (mongoc_collection_update(collection, MONGOC_UPDATE_NONE,
                                                &q, &u, NULL, &error), error);

      bson_destroy(&b);
      bson_destroy(&q);
      bson_destroy(&u);
   }

   bson_init(&q);
   bson_init(&u);
   BSON_APPEND_INT32 (&u, "abcd", 1);
   BSON_APPEND_INT32 (&u, "$hi", 1);
   r = mongoc_collection_update(collection, MONGOC_UPDATE_NONE, &q, &u, NULL, &error);
   ASSERT (!r);
   ASSERT (error.domain == MONGOC_ERROR_BSON);
   ASSERT (error.code == MONGOC_ERROR_BSON_INVALID);
   bson_destroy(&q);
   bson_destroy(&u);

   bson_init(&q);
   bson_init(&u);
   BSON_APPEND_INT32 (&u, "a.b.c.d", 1);
   r = mongoc_collection_update(collection, MONGOC_UPDATE_NONE, &q, &u, NULL, &error);
   ASSERT (!r);
   ASSERT (error.domain == MONGOC_ERROR_BSON);
   ASSERT (error.code == MONGOC_ERROR_BSON_INVALID);
   bson_destroy(&q);
   bson_destroy(&u);

   ASSERT_OR_PRINT (mongoc_collection_drop (collection, &error), error);

   mongoc_collection_destroy(collection);
   mongoc_database_destroy(database);
   bson_context_destroy(context);
   mongoc_client_destroy(client);
}


static void
test_remove (void)
{
   mongoc_collection_t *collection;
   mongoc_database_t *database;
   mongoc_client_t *client;
   bson_context_t *context;
   bson_error_t error;
   bool r;
   bson_oid_t oid;
   bson_t b;
   int i;

   client = test_framework_client_new ();
   ASSERT (client);

   database = get_test_database (client);
   ASSERT (database);

   collection = get_test_collection (client, "test_remove");
   ASSERT (collection);

   context = bson_context_new(BSON_CONTEXT_NONE);
   ASSERT (context);

   for (i = 0; i < 100; i++) {
      bson_init(&b);
      bson_oid_init(&oid, context);
      bson_append_oid(&b, "_id", 3, &oid);
      bson_append_utf8(&b, "hello", 5, "world", 5);
      r = mongoc_collection_insert(collection, MONGOC_INSERT_NONE, &b, NULL,
                                   &error);
      if (!r) {
         MONGOC_WARNING("%s\n", error.message);
      }
      ASSERT (r);
      bson_destroy(&b);

      bson_init(&b);
      bson_append_oid(&b, "_id", 3, &oid);
      r = mongoc_collection_remove(collection, MONGOC_REMOVE_NONE, &b, NULL,
                                   &error);
      if (!r) {
         MONGOC_WARNING("%s\n", error.message);
      }
      ASSERT (r);
      bson_destroy(&b);
   }

   ASSERT_OR_PRINT (mongoc_collection_drop (collection, &error), error);

   mongoc_collection_destroy(collection);
   mongoc_database_destroy(database);
   bson_context_destroy(context);
   mongoc_client_destroy(client);
}

static void
test_index (void)
{
   mongoc_collection_t *collection;
   mongoc_database_t *database;
   mongoc_client_t *client;
   mongoc_index_opt_t opt;
   bson_error_t error;
   bson_t keys;

   mongoc_index_opt_init(&opt);

   client = test_framework_client_new ();
   ASSERT (client);

   database = get_test_database (client);
   ASSERT (database);

   collection = get_test_collection (client, "test_index");
   ASSERT (collection);

   bson_init(&keys);
   bson_append_int32(&keys, "hello", -1, 1);
   ASSERT_OR_PRINT (mongoc_collection_create_index(collection, &keys,
                                                   &opt, &error), error);

   ASSERT_OR_PRINT (mongoc_collection_create_index(collection, &keys,
                                                   &opt, &error), error);

   ASSERT_OR_PRINT (mongoc_collection_drop_index(collection, "hello_1", &error),
                    error);

   bson_destroy(&keys);

   ASSERT_OR_PRINT (mongoc_collection_drop (collection, &error), error);

   mongoc_collection_destroy(collection);
   mongoc_database_destroy(database);
   mongoc_client_destroy(client);
}

static void
test_index_compound (void)
{
   mongoc_collection_t *collection;
   mongoc_database_t *database;
   mongoc_client_t *client;
   mongoc_index_opt_t opt;
   bson_error_t error;
   bson_t keys;

   mongoc_index_opt_init(&opt);

   client = test_framework_client_new ();
   ASSERT (client);

   database = get_test_database (client);
   ASSERT (database);

   collection = get_test_collection (client, "test_index_compound");
   ASSERT (collection);

   bson_init(&keys);
   bson_append_int32(&keys, "hello", -1, 1);
   bson_append_int32(&keys, "world", -1, -1);
   ASSERT_OR_PRINT (mongoc_collection_create_index(collection, &keys,
                                                   &opt, &error), error);

   ASSERT_OR_PRINT (mongoc_collection_create_index(collection, &keys,
                                                   &opt, &error), error);

   ASSERT_OR_PRINT (mongoc_collection_drop_index(collection, "hello_1_world_-1",
                                                 &error), error);

   bson_destroy(&keys);

   ASSERT_OR_PRINT (mongoc_collection_drop (collection, &error), error);

   mongoc_collection_destroy(collection);
   mongoc_database_destroy(database);
   mongoc_client_destroy(client);
}

static void
test_index_geo (void)
{
   mongoc_server_description_t *description;
   mongoc_collection_t *collection;
   mongoc_database_t *database;
   mongoc_client_t *client;
   mongoc_index_opt_t opt;
   mongoc_index_opt_geo_t geo_opt;
   bson_error_t error;
   bool r;
   bson_t keys;
   uint32_t id;

   mongoc_index_opt_init(&opt);
   mongoc_index_opt_geo_init(&geo_opt);

   client = test_framework_client_new ();
   ASSERT (client);

   database = get_test_database (client);
   ASSERT (database);

   collection = get_test_collection (client, "test_geo_index");
   ASSERT (collection);

   /* Create a basic 2d index */
   bson_init(&keys);
   BSON_APPEND_UTF8(&keys, "location", "2d");
   ASSERT_OR_PRINT (mongoc_collection_create_index(collection, &keys,
                                                   &opt, &error), error);

   ASSERT_OR_PRINT (mongoc_collection_drop_index(collection,
                                                 "location_2d", &error), error);

   /* Create a 2d index with bells and whistles */
   bson_init(&keys);
   BSON_APPEND_UTF8(&keys, "location", "2d");

   geo_opt.twod_location_min = -123;
   geo_opt.twod_location_max = +123;
   geo_opt.twod_bits_precision = 30;
   opt.geo_options = &geo_opt;

   /* TODO this hack is needed for single-threaded tests */
   id = client->topology->description.servers->items[0].id;
   description = mongoc_topology_server_by_id(client->topology, id, &error);
   ASSERT_OR_PRINT (description, error);

   if (description->max_wire_version > 0) {
      ASSERT_OR_PRINT (mongoc_collection_create_index(collection, &keys,
                                                      &opt, &error), error);

      ASSERT_OR_PRINT (mongoc_collection_drop_index(collection, "location_2d",
                                                    &error), error);
   }

   /* Create a Haystack index */
   bson_init(&keys);
   BSON_APPEND_UTF8(&keys, "location", "geoHaystack");
   BSON_APPEND_INT32(&keys, "category", 1);

   mongoc_index_opt_geo_init(&geo_opt);
   geo_opt.haystack_bucket_size = 5;

   opt.geo_options = &geo_opt;

   if (description->max_wire_version > 0) {
      ASSERT_OR_PRINT (mongoc_collection_create_index(collection, &keys,
                                                      &opt, &error), error);

      r = mongoc_collection_drop_index(collection,
                                       "location_geoHaystack_category_1",
                                       &error);
      ASSERT_OR_PRINT (r, error);
   }

   mongoc_server_description_destroy(description);
   mongoc_collection_destroy(collection);
   mongoc_database_destroy(database);
   mongoc_client_destroy(client);
}

static char *
storage_engine (mongoc_client_t *client)
{
   bson_iter_t iter;
   bson_error_t error;
   bson_t cmd = BSON_INITIALIZER;
   bson_t reply;

   /* NOTE: this default will change eventually */
   char *engine = bson_strdup("mmapv1");

   BSON_APPEND_INT32 (&cmd, "getCmdLineOpts", 1);
   ASSERT_OR_PRINT (mongoc_client_command_simple(client, "admin",
                                                 &cmd, NULL, &reply, &error),
                    error);

   if (bson_iter_init_find (&iter, &reply, "parsed.storage.engine")) {
      engine = bson_strdup(bson_iter_utf8(&iter, NULL));
   }

   bson_destroy (&reply);
   bson_destroy (&cmd);

   return engine;
}

static void
test_index_storage (void)
{
   mongoc_collection_t *collection = NULL;
   mongoc_database_t *database = NULL;
   mongoc_client_t *client = NULL;
   mongoc_index_opt_t opt;
   mongoc_index_opt_wt_t wt_opt;
   bson_error_t error;
   bson_t keys;
   char *engine = NULL;

   client = test_framework_client_new ();
   ASSERT (client);

   /* Skip unless we are on WT */
   engine = storage_engine(client);
   if (strcmp("wiredTiger", engine) != 0) {
      goto cleanup;
   }

   mongoc_index_opt_init (&opt);
   mongoc_index_opt_wt_init (&wt_opt);

   database = get_test_database (client);
   ASSERT (database);

   collection = get_test_collection (client, "test_storage_index");
   ASSERT (collection);

   /* Create a simple index */
   bson_init (&keys);
   bson_append_int32 (&keys, "hello", -1, 1);

   /* Add storage option to the index */
   wt_opt.base.type = MONGOC_INDEX_STORAGE_OPT_WIREDTIGER;
   wt_opt.config_str = "block_compressor=zlib";

   opt.storage_options = (mongoc_index_opt_storage_t *)&wt_opt;

   ASSERT_OR_PRINT (mongoc_collection_create_index (collection, &keys,
                                                    &opt, &error), error);

 cleanup:
   if (engine) bson_free (engine);
   if (collection) mongoc_collection_destroy (collection);
   if (database) mongoc_database_destroy (database);
   if (client) mongoc_client_destroy (client);
}

static void
test_count (void)
{
   mongoc_collection_t *collection;
   mongoc_client_t *client;
   bson_error_t error;
   int64_t count;
   bson_t b;

   client = test_framework_client_new ();
   ASSERT (client);

   collection = mongoc_client_get_collection(client, "test", "test");
   ASSERT (collection);

   bson_init(&b);
   count = mongoc_collection_count(collection, MONGOC_QUERY_NONE, &b,
                                   0, 0, NULL, &error);
   bson_destroy(&b);

   if (count == -1) {
      MONGOC_WARNING("%s\n", error.message);
   }
   ASSERT (count != -1);

   mongoc_collection_destroy(collection);
   mongoc_client_destroy(client);
}


static void
test_count_read_concern (void)
{
   mongoc_collection_t *collection;
   mongoc_client_t *client;
   mongoc_read_concern_t *rc;
   mock_server_t *server;
   request_t *request;
   bson_error_t error;
   future_t *future;
   int64_t count;
   bson_t b;

   /* wire protocol version 4 */
   server = mock_server_with_autoismaster (WIRE_VERSION_READ_CONCERN);
   mock_server_run (server);
   client = mongoc_client_new_from_uri (mock_server_get_uri (server));
   ASSERT (client);

   collection = mongoc_client_get_collection (client, "test", "test");
   ASSERT (collection);

   bson_init(&b);
   future = future_collection_count (collection, MONGOC_QUERY_NONE, &b,
                                     0, 0, NULL, &error);
   bson_destroy(&b);
   request = mock_server_receives_command (
      server, "test", MONGOC_QUERY_SLAVE_OK,
      "{ 'count' : 'test', 'query' : {  } }");

   mock_server_replies_simple (request, "{ 'n' : 42, 'ok' : 1 } ");
   count = future_get_int64_t (future);
   ASSERT_OR_PRINT (count == 42, error);
   request_destroy (request);
   future_destroy (future);

   /* readConcern: { level: majority } */
   rc = mongoc_read_concern_new ();
   mongoc_read_concern_set_level (rc, MONGOC_READ_CONCERN_LEVEL_MAJORITY);
   mongoc_collection_set_read_concern (collection, rc);

   bson_init(&b);
   future = future_collection_count (collection, MONGOC_QUERY_NONE, &b,
                                     0, 0, NULL, &error);
   bson_destroy(&b);
   request = mock_server_receives_command (
      server, "test", MONGOC_QUERY_SLAVE_OK,
      "{ 'count' : 'test', 'query' : {  }, 'readConcern': {'level': 'majority'}}");

   mock_server_replies_simple (request, "{ 'n' : 43, 'ok' : 1 } ");
   count = future_get_int64_t (future);
   ASSERT_OR_PRINT (count == 43, error);
   mongoc_read_concern_destroy (rc);
   request_destroy (request);
   future_destroy (future);

   /* readConcern: { level: local } */
   rc = mongoc_read_concern_new ();
   mongoc_read_concern_set_level (rc, MONGOC_READ_CONCERN_LEVEL_LOCAL);
   mongoc_collection_set_read_concern (collection, rc);

   bson_init(&b);
   future = future_collection_count (collection, MONGOC_QUERY_NONE, &b,
                                     0, 0, NULL, &error);
   bson_destroy(&b);
   request = mock_server_receives_command (
      server, "test", MONGOC_QUERY_SLAVE_OK,
      "{ 'count' : 'test', 'query' : {  }, 'readConcern': {'level': 'local'}}");

   mock_server_replies_simple (request, "{ 'n' : 44, 'ok' : 1 } ");
   count = future_get_int64_t (future);
   ASSERT_OR_PRINT (count == 44, error);
   mongoc_read_concern_destroy (rc);
   request_destroy (request);
   future_destroy (future);

   /* readConcern: { level: futureCompatible } */
   rc = mongoc_read_concern_new ();
   mongoc_read_concern_set_level (rc, "futureCompatible");
   mongoc_collection_set_read_concern (collection, rc);

   bson_init(&b);
   future = future_collection_count (collection, MONGOC_QUERY_NONE, &b,
                                     0, 0, NULL, &error);
   bson_destroy(&b);
   request = mock_server_receives_command (
      server, "test", MONGOC_QUERY_SLAVE_OK,
      "{ 'count' : 'test', 'query' : {  }, 'readConcern': {'level': 'futureCompatible'}}");

   mock_server_replies_simple (request, "{ 'n' : 45, 'ok' : 1 } ");
   count = future_get_int64_t (future);
   ASSERT_OR_PRINT (count == 45, error);
   mongoc_read_concern_destroy (rc);
   request_destroy (request);
   future_destroy (future);

   /* Setting readConcern to NULL should not send readConcern */
   rc = mongoc_read_concern_new ();
   mongoc_read_concern_set_level (rc, NULL);
   mongoc_collection_set_read_concern (collection, rc);

   bson_init(&b);
   future = future_collection_count (collection, MONGOC_QUERY_NONE, &b,
                                     0, 0, NULL, &error);
   bson_destroy(&b);
   request = mock_server_receives_command (
      server, "test", MONGOC_QUERY_SLAVE_OK,
      "{ 'count' : 'test', 'query' : {  }, 'readConcern': { '$exists': false }}");

   mock_server_replies_simple (request, "{ 'n' : 46, 'ok' : 1 } ");
   count = future_get_int64_t (future);
   ASSERT_OR_PRINT (count == 46, error);
   mongoc_read_concern_destroy (rc);
   request_destroy (request);
   future_destroy (future);

   /* Fresh read_concern should not send readConcern */
   rc = mongoc_read_concern_new ();
   mongoc_collection_set_read_concern (collection, rc);

   bson_init(&b);
   future = future_collection_count (collection, MONGOC_QUERY_NONE, &b,
                                     0, 0, NULL, &error);
   bson_destroy(&b);
   request = mock_server_receives_command (
      server, "test", MONGOC_QUERY_SLAVE_OK,
      "{ 'count' : 'test', 'query' : {  }, 'readConcern': { '$exists': false }}");

   mock_server_replies_simple (request, "{ 'n' : 47, 'ok' : 1 } ");
   count = future_get_int64_t (future);
   ASSERT_OR_PRINT (count == 47, error);

   mongoc_read_concern_destroy (rc);
   request_destroy (request);
   future_destroy (future);
   mongoc_collection_destroy (collection);
   mongoc_client_destroy (client);
   mock_server_destroy (server);
}


static void
_test_count_read_concern_live (bool supports_read_concern)
{
   mongoc_collection_t *collection;
   mongoc_client_t *client;
   mongoc_read_concern_t *rc;
   bson_error_t error;
   int64_t count;
   bson_t b;


   client = test_framework_client_new ();
   ASSERT (client);

   collection = mongoc_client_get_collection (client, "test", "test");
   ASSERT (collection);

   mongoc_collection_drop (collection, &error);

   bson_init(&b);
   count = mongoc_collection_count (collection, MONGOC_QUERY_NONE, &b,
                                    0, 0, NULL, &error);
   bson_destroy(&b);
   ASSERT_OR_PRINT (count == 0, error);

   /* Setting readConcern to NULL should not send readConcern */
   rc = mongoc_read_concern_new ();
   mongoc_read_concern_set_level (rc, NULL);
   mongoc_collection_set_read_concern (collection, rc);

   bson_init(&b);
   count = mongoc_collection_count (collection, MONGOC_QUERY_NONE, &b,
                                    0, 0, NULL, &error);
   bson_destroy(&b);
   ASSERT_OR_PRINT (count == 0, error);
   mongoc_read_concern_destroy (rc);

   /* readConcern: { level: local } should raise error pre 3.2 */
   rc = mongoc_read_concern_new ();
   mongoc_read_concern_set_level (rc, MONGOC_READ_CONCERN_LEVEL_LOCAL);
   mongoc_collection_set_read_concern (collection, rc);

   bson_init(&b);
   count = mongoc_collection_count (collection, MONGOC_QUERY_NONE, &b,
                                    0, 0, NULL, &error);
   bson_destroy(&b);
   if (supports_read_concern) {
      ASSERT_OR_PRINT (count == 0, error);
   } else {
      ASSERT_ERROR_CONTAINS(error, MONGOC_ERROR_COMMAND,
            MONGOC_ERROR_PROTOCOL_BAD_WIRE_VERSION,
            "The selected server does not support readConcern") 
   }
   mongoc_read_concern_destroy (rc);

   /* readConcern: { level: majority } should raise error pre 3.2 */
   rc = mongoc_read_concern_new ();
   mongoc_read_concern_set_level (rc, MONGOC_READ_CONCERN_LEVEL_MAJORITY);
   mongoc_collection_set_read_concern (collection, rc);

   bson_init(&b);
   count = mongoc_collection_count (collection, MONGOC_QUERY_NONE, &b,
                                    0, 0, NULL, &error);
   bson_destroy(&b);
   if (supports_read_concern) {
      ASSERT_OR_PRINT (count == 0, error);
   } else {
      ASSERT_ERROR_CONTAINS(error, MONGOC_ERROR_COMMAND,
            MONGOC_ERROR_PROTOCOL_BAD_WIRE_VERSION,
            "The selected server does not support readConcern") 
   }
   mongoc_read_concern_destroy (rc);

   mongoc_collection_destroy (collection);
   mongoc_client_destroy (client);
}

int
mongod_supports_majority_read_concern (void)
{
   return test_framework_getenv_bool ("MONGOC_ENABLE_MAJORITY_READ_CONCERN");
}

static void
test_count_read_concern_live (void *context)
{
   if (test_framework_max_wire_version_at_least (WIRE_VERSION_READ_CONCERN)) {
      _test_count_read_concern_live (true);
   } else {
      _test_count_read_concern_live (false);
   }
}


static void
test_count_with_opts (void)
{
   mongoc_collection_t *collection;
   mongoc_client_t *client;
   bson_error_t error;
   int64_t count;
   bson_t b;
   bson_t opts;

   client = test_framework_client_new ();
   ASSERT (client);

   collection = mongoc_client_get_collection (client, "test", "test");
   ASSERT (collection);

   bson_init (&opts);

   BSON_APPEND_UTF8 (&opts, "hint", "_id_");

   bson_init (&b);
   count = mongoc_collection_count_with_opts (collection, MONGOC_QUERY_NONE, &b,
                                              0, 0, &opts, NULL, &error);
   bson_destroy (&b);
   bson_destroy (&opts);

   if (count == -1) {
      MONGOC_WARNING ("%s\n", error.message);
   }

   ASSERT (count != -1);

   mongoc_collection_destroy (collection);
   mongoc_client_destroy (client);
}


static void
test_drop (void)
{
   mongoc_collection_t *collection;
   mongoc_database_t *database;
   mongoc_client_t *client;
   bson_error_t error;
   bson_t *doc;

   client = test_framework_client_new ();
   ASSERT (client);

   database = get_test_database (client);
   ASSERT (database);

   collection = get_test_collection (client, "test_drop");
   ASSERT (collection);

   doc = BCON_NEW("hello", "world");
   ASSERT_OR_PRINT (mongoc_collection_insert(collection,  MONGOC_INSERT_NONE,
                                             doc, NULL, &error), error);
   bson_destroy (doc);

   ASSERT_OR_PRINT (mongoc_collection_drop(collection, &error), error);
   ASSERT (!mongoc_collection_drop(collection, &error));

   mongoc_collection_destroy(collection);
   mongoc_database_destroy(database);
   mongoc_client_destroy(client);
}


static void
test_aggregate_bypass (void *context)
{
   mongoc_collection_t *data_collection;
   mongoc_collection_t *out_collection;
   mongoc_bulk_operation_t *bulk;
   mongoc_database_t *database;
   mongoc_client_t *client;
   mongoc_cursor_t *cursor;
   bson_error_t error;
   const bson_t *doc;
   bson_t *pipeline;
   bson_t *options;
   char *collname;
   char *dbname;
   bson_t reply;
   bool r;
   int i;

   client = test_framework_client_new ();
   assert (client);

   dbname = gen_collection_name ("dbtest");
   collname = gen_collection_name ("data");
   database = mongoc_client_get_database (client, dbname);
   data_collection = mongoc_database_get_collection (database, collname);
   bson_free (collname);

   collname = gen_collection_name ("bypass");
   options = tmp_bson ("{'validator': {'number': {'$gte': 5}}, 'validationAction': 'error'}");
   ASSERT_OR_PRINT (mongoc_database_create_collection (database, collname, options, &error), error);
   out_collection = mongoc_database_get_collection (database, collname);

   bson_free (dbname);
   bson_free (collname);

   /* Generate some example data */
   bulk = mongoc_collection_create_bulk_operation(data_collection, true, NULL);
   for (i = 0; i < 3; i++) {
      bson_t *document = tmp_bson (bson_strdup_printf ("{'number': 3, 'high': %d }", i));

      mongoc_bulk_operation_insert (bulk, document);
   }
   r = mongoc_bulk_operation_execute (bulk, &reply, &error);
   ASSERT_OR_PRINT(r, error);
   mongoc_bulk_operation_destroy (bulk);
   /* }}} */

   pipeline = tmp_bson (bson_strdup_printf ("[{'$out': '%s'}]", out_collection->collection));

   cursor = mongoc_collection_aggregate(data_collection, MONGOC_QUERY_NONE, pipeline, NULL, NULL);
   ASSERT (cursor);
   r = mongoc_cursor_next (cursor, &doc);
   ASSERT (!r);
   ASSERT (mongoc_cursor_error (cursor, &error));
   ASSERT_STARTSWITH (error.message, "insert for $out failed");

   options = tmp_bson("{'bypassDocumentValidation': true}");
   cursor = mongoc_collection_aggregate(data_collection, MONGOC_QUERY_NONE, pipeline, options, NULL);
   ASSERT (cursor);
   ASSERT (!mongoc_cursor_error (cursor, &error));


   ASSERT_OR_PRINT (mongoc_collection_drop(data_collection, &error), error);
   ASSERT_OR_PRINT (mongoc_collection_drop(out_collection, &error), error);
   mongoc_collection_destroy(data_collection);
   mongoc_collection_destroy(out_collection);
   mongoc_database_destroy(database);
   mongoc_client_destroy(client);
}


static void
test_aggregate (void)
{
   mongoc_collection_t *collection;
   mongoc_database_t *database;
   mongoc_client_t *client;
   mongoc_cursor_t *cursor;
   const bson_t *doc;
   bson_error_t error;
   bool did_alternate = false;
   bool r;
   bson_t opts;
   bson_t *pipeline;
   bson_t *broken_pipeline;
   bson_t *b;
   bson_iter_t iter;
   int i, j;

   client = test_framework_client_new ();
   ASSERT (client);

   database = get_test_database (client);
   ASSERT (database);

   collection = get_test_collection (client, "test_aggregate");
   ASSERT (collection);

   pipeline = BCON_NEW ("pipeline", "[", "{", "$match", "{", "hello", BCON_UTF8 ("world"), "}", "}", "]");
   broken_pipeline = BCON_NEW ("pipeline", "[", "{", "$asdf", "{", "foo", BCON_UTF8 ("bar"), "}", "}", "]");
   b = BCON_NEW ("hello", BCON_UTF8 ("world"));

again:
   mongoc_collection_drop(collection, &error);

   for (i = 0; i < 2; i++) {
      ASSERT_OR_PRINT (mongoc_collection_insert(
         collection,
         MONGOC_INSERT_NONE, b, NULL, &error), error);
   }

   cursor = mongoc_collection_aggregate (collection, MONGOC_QUERY_NONE, broken_pipeline, NULL, NULL);
   ASSERT (cursor);

   r = mongoc_cursor_next (cursor, &doc);
   ASSERT (!r);
   ASSERT (mongoc_cursor_error (cursor, &error));
   ASSERT (error.code == 16436);
   mongoc_cursor_destroy (cursor);

   for (i = 0; i < 2; i++) {
      if (i % 2 == 0) {
         cursor = mongoc_collection_aggregate(collection, MONGOC_QUERY_NONE, pipeline, NULL, NULL);
         ASSERT (cursor);
      } else {
         bson_init (&opts);

         /* servers < 2.6 error is passed allowDiskUse */
         if (test_framework_max_wire_version_at_least (2)) {
            BSON_APPEND_BOOL (&opts, "allowDiskUse", true);
         }

         /* this is ok, the driver silently omits batchSize if server < 2.6 */
         BSON_APPEND_INT32 (&opts, "batchSize", 10);
         cursor = mongoc_collection_aggregate(collection, MONGOC_QUERY_NONE, pipeline, &opts, NULL);
         ASSERT (cursor);

         bson_destroy (&opts);
      }

      for (j = 0; j < 2; j++) {
         r = mongoc_cursor_next(cursor, &doc);
         if (mongoc_cursor_error(cursor, &error)) {
            fprintf (stderr, "[%d.%d] %s",
                     error.domain, error.code, error.message);

            abort ();
         }

         ASSERT (r);
         ASSERT (doc);

         ASSERT (bson_iter_init_find (&iter, doc, "hello") &&
                 BSON_ITER_HOLDS_UTF8 (&iter));
      }

      r = mongoc_cursor_next(cursor, &doc);
      if (mongoc_cursor_error(cursor, &error)) {
         fprintf (stderr, "%s", error.message);
         abort ();
      }

      ASSERT (!r);
      ASSERT (!doc);

      mongoc_cursor_destroy(cursor);
   }

   if (!did_alternate) {
      did_alternate = true;
      bson_destroy (pipeline);
      pipeline = BCON_NEW ("0", "{", "$match", "{", "hello", BCON_UTF8 ("world"), "}", "}");
      goto again;
   }

   ASSERT_OR_PRINT (mongoc_collection_drop(collection, &error), error);

   mongoc_collection_destroy(collection);
   mongoc_database_destroy(database);
   mongoc_client_destroy(client);
   bson_destroy(b);
   bson_destroy(pipeline);
   bson_destroy (broken_pipeline);
}


static void
test_aggregate_large (void)
{
   mongoc_client_t *client;
   mongoc_collection_t *collection;
   mongoc_bulk_operation_t *bulk;
   bson_iter_t iter;
   int32_t i;
   uint32_t hint;
   mongoc_cursor_t *cursor;
   bson_t *inserted_doc;
   bson_error_t error;
   bson_t *pipeline;
   const bson_t *doc;

   client = test_framework_client_new ();
   ASSERT (client);

   collection = get_test_collection (client, "test_aggregate_large");
   ASSERT (collection);

   bulk = mongoc_collection_create_bulk_operation (collection, true, NULL);

   /* ensure a few batches */
   inserted_doc = tmp_bson ("{'_id': 0}");

   for (i = 0; i < 2000; i++) {
      bson_iter_init_find (&iter, inserted_doc, "_id");
      bson_iter_overwrite_int32 (&iter, i);
      mongoc_bulk_operation_insert (bulk, inserted_doc);
   }

   hint = mongoc_bulk_operation_execute (bulk, NULL, &error);
   ASSERT_OR_PRINT (hint > 0, error);

   pipeline = tmp_bson ("[{'$sort': {'_id': 1}}]");

   cursor = mongoc_collection_aggregate (collection, MONGOC_QUERY_NONE,
                                         pipeline, NULL, NULL);
   ASSERT (cursor);

   i = 0;
   while (mongoc_cursor_next (cursor, &doc)) {
      ASSERT (bson_iter_init_find (&iter, doc, "_id"));
      ASSERT_CMPINT (i, ==, bson_iter_int32 (&iter));
      i++;
   }

   ASSERT_OR_PRINT (!mongoc_cursor_error (cursor, &error), error);
   ASSERT_CMPINT (i, ==, 2000);

   mongoc_bulk_operation_destroy (bulk);
   mongoc_cursor_destroy (cursor);
   mongoc_collection_destroy (collection);
   mongoc_client_destroy (client);
}


typedef struct {
    bool with_batch_size;
    bool with_options;
} test_aggregate_context_t;


static const char *
options_json (test_aggregate_context_t *c)
{
   if (c->with_batch_size && c->with_options) {
      return "{'foo': 1, 'batchSize': 11}";
   } else if (c->with_batch_size) {
      return "{'batchSize': 11}";
   } else if (c->with_options) {
      return "{'foo': 1}";
   } else {
      return "{}";
   }
}


static void
test_aggregate_legacy (void *data)
{
   test_aggregate_context_t *context = (test_aggregate_context_t *) data;
   mock_server_t *server;
   mongoc_client_t *client;
   mongoc_collection_t *collection;
   future_t *future;
   request_t *request;
   mongoc_cursor_t *cursor;
   const bson_t *doc;

   /* wire protocol version 0 */
   server = mock_server_with_autoismaster (0);
   mock_server_run (server);
   client = mongoc_client_new_from_uri (mock_server_get_uri (server));
   collection = mongoc_client_get_collection (client, "db", "collection");

   cursor = mongoc_collection_aggregate (
      collection,
      MONGOC_QUERY_NONE,
      tmp_bson ("[{'a': 1}]"),
      tmp_bson (options_json (context)),
      NULL);

   future = future_cursor_next (cursor, &doc);

   /* no "cursor" argument */
   request = mock_server_receives_command (
      server, "db", MONGOC_QUERY_SLAVE_OK,
      "{'aggregate': 'collection',"
      " 'pipeline': [{'a': 1}]},"
      " 'cursor': {'$exists': false} %s",
      context->with_options ? ", 'foo': 1" : "");

   mock_server_replies_simple (request, "{'ok': 1, 'result': [{'_id': 123}]}");
   assert (future_get_bool (future));
   ASSERT_MATCH (doc, "{'_id': 123}");

   /* cursor is completed */
   assert (!mongoc_cursor_next (cursor, &doc));

   mongoc_cursor_destroy (cursor);
   request_destroy (request);
   future_destroy (future);
   mongoc_collection_destroy (collection);
   mongoc_client_destroy (client);
   mock_server_destroy (server);
}


static void
test_aggregate_modern (void *data)
{
   test_aggregate_context_t *context = (test_aggregate_context_t *) data;
   mock_server_t *server;
   mongoc_client_t *client;
   mongoc_collection_t *collection;
   future_t *future;
   request_t *request;
   mongoc_cursor_t *cursor;
   const bson_t *doc;

   /* wire protocol version 1 */
   server = mock_server_with_autoismaster (1);
   mock_server_run (server);
   client = mongoc_client_new_from_uri (mock_server_get_uri (server));
   collection = mongoc_client_get_collection (client, "db", "collection");

   cursor = mongoc_collection_aggregate (
      collection,
      MONGOC_QUERY_NONE,
      tmp_bson ("[{'a': 1}]"),
      tmp_bson (options_json (context)),
      NULL);

   ASSERT (cursor);
   future = future_cursor_next (cursor, &doc);

   /* "cursor" argument always sent if wire version >= 1 */
   request = mock_server_receives_command (
      server, "db", MONGOC_QUERY_SLAVE_OK,
      "{'aggregate': 'collection',"
      " 'pipeline': [{'a': 1}],"
      " 'cursor': %s %s}",
      context->with_batch_size ? "{'batchSize': 11}" : "{'$empty': true}",
      context->with_options ? ", 'foo': 1" : "");

   mock_server_replies_simple (request,
                               "{'ok': 1,"
                               " 'cursor': {"
                               "    'id': 0,"
                               "    'ns': 'db.collection',"
                               "    'firstBatch': [{'_id': 123}]"
                               "}}");

   ASSERT (future_get_bool (future));
   ASSERT_MATCH (doc, "{'_id': 123}");

   /* cursor is completed */
   assert (!mongoc_cursor_next (cursor, &doc));

   mongoc_cursor_destroy (cursor);
   request_destroy (request);
   future_destroy (future);
   mongoc_collection_destroy (collection);
   mongoc_client_destroy (client);
   mock_server_destroy (server);
}


static void
test_validate (void)
{
   mongoc_collection_t *collection;
   mongoc_client_t *client;
   bson_iter_t iter;
   bson_error_t error;
   bson_t doc = BSON_INITIALIZER;
   bson_t opts = BSON_INITIALIZER;
   bson_t reply;
   bool r;

   client = test_framework_client_new ();
   ASSERT (client);

   collection = get_test_collection (client, "test_validate");
   ASSERT (collection);

   ASSERT_OR_PRINT (mongoc_collection_insert(collection, MONGOC_INSERT_NONE,
                                             &doc, NULL, &error), error);

   BSON_APPEND_BOOL (&opts, "full", true);

   ASSERT_OR_PRINT (mongoc_collection_validate (collection, &opts,
                                                &reply, &error), error);

   assert (bson_iter_init_find (&iter, &reply, "valid"));

   bson_destroy (&reply);

   bson_reinit (&opts);
   BSON_APPEND_UTF8 (&opts, "full", "bad_value");

   r = mongoc_collection_validate (collection, &opts, &reply, &error);
   assert (!r);
   assert (error.domain == MONGOC_ERROR_BSON);
   assert (error.code == MONGOC_ERROR_BSON_INVALID);

   ASSERT_OR_PRINT (mongoc_collection_drop (collection, &error), error);

   mongoc_collection_destroy (collection);
   mongoc_client_destroy (client);
   bson_destroy (&doc);
   bson_destroy (&opts);
}


static void
test_rename (void)
{
   mongoc_collection_t *collection;
   mongoc_client_t *client;
   bson_error_t error;
   bson_t doc = BSON_INITIALIZER;

   client = test_framework_client_new ();
   ASSERT (client);

   collection = get_test_collection (client, "test_rename");
   ASSERT (collection);

   ASSERT_OR_PRINT (mongoc_collection_insert (
      collection, MONGOC_INSERT_NONE, &doc, NULL, &error), error);

   ASSERT_OR_PRINT (mongoc_collection_rename (
      collection, "test", "test_rename_2", false, &error), error);

   ASSERT_OR_PRINT (mongoc_collection_drop (collection, &error), error);

   mongoc_collection_destroy (collection);
   mongoc_client_destroy (client);
   bson_destroy (&doc);
}


static void
test_stats (void)
{
   mongoc_collection_t *collection;
   mongoc_client_t *client;
   bson_error_t error;
   bson_iter_t iter;
   bson_t stats;
   bson_t doc = BSON_INITIALIZER;

   client = test_framework_client_new ();
   ASSERT (client);

   collection = get_test_collection (client, "test_stats");
   ASSERT (collection);

   ASSERT_OR_PRINT (mongoc_collection_insert (
      collection, MONGOC_INSERT_NONE, &doc, NULL, &error), error);

   ASSERT_OR_PRINT (mongoc_collection_stats (
      collection, NULL, &stats, &error), error);

   assert (bson_iter_init_find (&iter, &stats, "ns"));

   assert (bson_iter_init_find (&iter, &stats, "count"));
   assert (bson_iter_as_int64 (&iter) >= 1);

   bson_destroy (&stats);

   ASSERT_OR_PRINT (mongoc_collection_drop (collection, &error), error);

   mongoc_collection_destroy (collection);
   mongoc_client_destroy (client);
   bson_destroy (&doc);
}


static void
test_find_and_modify_write_concern (int wire_version)
{
   mongoc_collection_t *collection;
   mongoc_client_t *client;
   mock_server_t *server;
   request_t *request;
   future_t *future;
   bson_error_t error;
   bson_t *update;
   bson_t doc = BSON_INITIALIZER;
   bson_t reply;
   mongoc_write_concern_t *write_concern;

   server = mock_server_new ();
   mock_server_run (server);

   client = mongoc_client_new_from_uri (mock_server_get_uri (server));
   ASSERT (client);

   collection = mongoc_client_get_collection (client, "test", "test_find_and_modify");

   auto_ismaster (server,
                  wire_version,          /* max_wire_version */
                  48000000,   /* max_message_size */
                  16777216,   /* max_bson_size */
                  1000);      /* max_write_batch_size */

   BSON_APPEND_INT32 (&doc, "superduper", 77889);

   update = BCON_NEW ("$set", "{",
                         "superduper", BCON_INT32 (1234),
                      "}");

   write_concern = mongoc_write_concern_new ();
   mongoc_write_concern_set_w (write_concern, 42);
   mongoc_collection_set_write_concern (collection, write_concern);
   future = future_collection_find_and_modify (collection,
                                               &doc,
                                               NULL,
                                               update,
                                               NULL,
                                               false,
                                               false,
                                               true,
                                               &reply,
                                               &error);

   if (wire_version >= 4) {
      request = mock_server_receives_command (server, "test", MONGOC_QUERY_NONE,
       "{ 'findAndModify' : 'test_find_and_modify', "
         "'query' : { 'superduper' : 77889 },"
         "'update' : { '$set' : { 'superduper' : 1234 } },"
         "'new' : true,"
         "'writeConcern' : { 'w' : 42 } }");
   } else {
      request = mock_server_receives_command (server, "test", MONGOC_QUERY_NONE,
       "{ 'findAndModify' : 'test_find_and_modify', "
         "'query' : { 'superduper' : 77889 },"
         "'update' : { '$set' : { 'superduper' : 1234 } },"
         "'new' : true }");
   }

   mock_server_replies_simple (request, "{ 'value' : null, 'ok' : 1 }");
   ASSERT_OR_PRINT (future_get_bool (future), error);

   future_destroy (future);

   bson_destroy (&reply);
   bson_destroy (update);

   mongoc_write_concern_destroy (write_concern);
   mongoc_collection_destroy (collection);
   mongoc_client_destroy (client);
   bson_destroy (&doc);
}

static void
test_find_and_modify_write_concern_wire_32 (void)
{
   test_find_and_modify_write_concern (4);
}

static void
test_find_and_modify_write_concern_wire_pre_32 (void)
{
   test_find_and_modify_write_concern (2);
}

static void
test_find_and_modify (void)
{
   mongoc_collection_t *collection;
   mongoc_client_t *client;
   bson_error_t error;
   bson_iter_t iter;
   bson_iter_t citer;
   bson_t *update;
   bson_t doc = BSON_INITIALIZER;
   bson_t reply;

   client = test_framework_client_new ();
   ASSERT (client);

   collection = get_test_collection (client, "test_find_and_modify");
   ASSERT (collection);

   BSON_APPEND_INT32 (&doc, "superduper", 77889);

   ASSERT_OR_PRINT (mongoc_collection_insert (
      collection, MONGOC_INSERT_NONE, &doc, NULL, &error), error);

   update = BCON_NEW ("$set", "{",
                         "superduper", BCON_INT32 (1234),
                      "}");

   ASSERT_OR_PRINT (mongoc_collection_find_and_modify (collection,
                                                       &doc,
                                                       NULL,
                                                       update,
                                                       NULL,
                                                       false,
                                                       false,
                                                       true,
                                                       &reply,
                                                       &error), error);

   assert (bson_iter_init_find (&iter, &reply, "value"));
   assert (BSON_ITER_HOLDS_DOCUMENT (&iter));
   assert (bson_iter_recurse (&iter, &citer));
   assert (bson_iter_find (&citer, "superduper"));
   assert (BSON_ITER_HOLDS_INT32 (&citer));
   assert (bson_iter_int32 (&citer) == 1234);

   assert (bson_iter_init_find (&iter, &reply, "lastErrorObject"));
   assert (BSON_ITER_HOLDS_DOCUMENT (&iter));
   assert (bson_iter_recurse (&iter, &citer));
   assert (bson_iter_find (&citer, "updatedExisting"));
   assert (BSON_ITER_HOLDS_BOOL (&citer));
   assert (bson_iter_bool (&citer));

   bson_destroy (&reply);
   bson_destroy (update);

   ASSERT_OR_PRINT (mongoc_collection_drop (collection, &error), error);

   mongoc_collection_destroy (collection);
   mongoc_client_destroy (client);
   bson_destroy (&doc);
}


static void
test_large_return (void)
{
   mongoc_collection_t *collection;
   mongoc_client_t *client;
   mongoc_cursor_t *cursor;
   bson_error_t error;
   const bson_t *doc = NULL;
   bson_oid_t oid;
   bson_t insert_doc = BSON_INITIALIZER;
   bson_t query = BSON_INITIALIZER;
   size_t len;
   char *str;
   bool r;

   client = test_framework_client_new ();
   ASSERT (client);

   collection = get_test_collection (client, "test_large_return");
   ASSERT (collection);

   len = 1024 * 1024 * 4;
   str = (char *)bson_malloc (len);
   memset (str, (int)' ', len);
   str [len - 1] = '\0';

   bson_oid_init (&oid, NULL);
   BSON_APPEND_OID (&insert_doc, "_id", &oid);
   BSON_APPEND_UTF8 (&insert_doc, "big", str);

   ASSERT_OR_PRINT (mongoc_collection_insert (
      collection, MONGOC_INSERT_NONE, &insert_doc, NULL, &error), error);

   bson_destroy (&insert_doc);

   BSON_APPEND_OID (&query, "_id", &oid);

   cursor = mongoc_collection_find (collection, MONGOC_QUERY_NONE, 0, 0, 0, &query, NULL, NULL);
   assert (cursor);
   bson_destroy (&query);

   ASSERT_OR_PRINT (mongoc_cursor_next (cursor, &doc), error);
   assert (doc);

   r = mongoc_cursor_next (cursor, &doc);
   assert (!r);

   mongoc_cursor_destroy (cursor);

   ASSERT_OR_PRINT (mongoc_collection_drop (collection, &error), error);

   mongoc_collection_destroy (collection);
   mongoc_client_destroy (client);
   bson_free (str);
}


static void
test_many_return (void)
{
   enum { N_BSONS = 5000 };

   mongoc_collection_t *collection;
   mongoc_client_t *client;
   mongoc_cursor_t *cursor;
   bson_error_t error;
   const bson_t *doc = NULL;
   bson_oid_t oid;
   bson_t query = BSON_INITIALIZER;
   bson_t *docs[N_BSONS];
   bool r;
   int i;

   client = test_framework_client_new ();
   ASSERT (client);

   collection = get_test_collection (client, "test_many_return");
   ASSERT (collection);

   for (i = 0; i < N_BSONS; i++) {
      docs [i] = bson_new ();
      bson_oid_init (&oid, NULL);
      BSON_APPEND_OID (docs [i], "_id", &oid);
   }

BEGIN_IGNORE_DEPRECATIONS;

   ASSERT_OR_PRINT (mongoc_collection_insert_bulk (
                       collection, MONGOC_INSERT_NONE, (const bson_t **)docs,
                       (uint32_t) N_BSONS, NULL, &error), error);

END_IGNORE_DEPRECATIONS;

   cursor = mongoc_collection_find (collection, MONGOC_QUERY_NONE, 0, 0, 6000,
                                    &query, NULL, NULL);
   assert (cursor);
   bson_destroy (&query);

   i = 0;

   while (mongoc_cursor_next (cursor, &doc)) {
      assert (doc);
      i++;
   }

   assert (i == N_BSONS);

   r = mongoc_cursor_next (cursor, &doc);
   assert (!r);

   mongoc_cursor_destroy (cursor);

   ASSERT_OR_PRINT (mongoc_collection_drop (collection, &error), error);

   destroy_all (docs, N_BSONS);
   mongoc_collection_destroy (collection);
   mongoc_client_destroy (client);
}


/* use a mock server to test the "limit" parameter */
static void
test_find_limit (void)
{
   mock_server_t *server;
   mongoc_client_t *client;
   mongoc_collection_t *collection;
   mongoc_cursor_t *cursor;
   future_t *future;
   request_t *request;
   const bson_t *doc;

   server = mock_server_with_autoismaster (0);
   mock_server_run (server);

   client = mongoc_client_new_from_uri (mock_server_get_uri (server));
   collection = mongoc_client_get_collection (client, "test", "test");
   cursor = mongoc_collection_find (collection,
                                    MONGOC_QUERY_NONE,
                                    0 /* skip */,
                                    2 /* limit */,
                                    0 /* batch_size */,
                                    tmp_bson ("{}"),
                                    NULL,
                                    NULL);

   future = future_cursor_next (cursor, &doc);
   request = mock_server_receives_query (server,
                                         "test.test",
                                         MONGOC_QUERY_SLAVE_OK,
                                         0 /* skip */,
                                         2 /* n_return */,
                                         "{}",
                                         NULL);

   mock_server_replies_simple (request, "{}");
   assert (future_get_bool (future));

   future_destroy (future);
   request_destroy (request);
   mongoc_cursor_destroy (cursor);
   mongoc_collection_destroy(collection);
   mongoc_client_destroy(client);
   mock_server_destroy (server);
}


/* use a mock server to test the "batch_size" parameter */
static void
test_find_batch_size (void)
{
   mock_server_t *server;
   mongoc_client_t *client;
   mongoc_collection_t *collection;
   mongoc_cursor_t *cursor;
   future_t *future;
   request_t *request;
   const bson_t *doc;

   server = mock_server_with_autoismaster (0);
   mock_server_run (server);

   client = mongoc_client_new_from_uri (mock_server_get_uri (server));
   collection = mongoc_client_get_collection (client, "test", "test");
   cursor = mongoc_collection_find (collection,
                                    MONGOC_QUERY_NONE,
                                    0 /* skip */,
                                    0 /* limit */,
                                    2 /* batch_size */,
                                    tmp_bson ("{}"),
                                    NULL,
                                    NULL);

   future = future_cursor_next (cursor, &doc);
   request = mock_server_receives_query (server,
                                         "test.test",
                                         MONGOC_QUERY_SLAVE_OK,
                                         0 /* skip */,
                                         2 /* n_return */,
                                         "{}",
                                         NULL);

   mock_server_replies_simple (request, "{}");
   assert (future_get_bool (future));

   future_destroy (future);
   request_destroy (request);
   mongoc_cursor_destroy (cursor);
   mongoc_collection_destroy(collection);
   mongoc_client_destroy(client);
   mock_server_destroy (server);
}


static void
test_command_fq (void *context)
{
   mongoc_client_t *client;
   mongoc_cursor_t *cursor;
   const bson_t *doc = NULL;
   bson_iter_t iter;
   bson_t *cmd;
   bool r;

   client = test_framework_client_new ();
   ASSERT (client);

   cmd = tmp_bson ("{ 'dbstats': 1}");

   cursor = mongoc_client_command (client, "sometest.$cmd", MONGOC_QUERY_SLAVE_OK,
                                 0, -1, 0, cmd, NULL, NULL);
   r = mongoc_cursor_next (cursor, &doc);
   assert (r);

   if (bson_iter_init_find (&iter, doc, "db") &&
       BSON_ITER_HOLDS_UTF8 (&iter)) {
      ASSERT_CMPSTR (bson_iter_utf8 (&iter, NULL), "sometest");
   } else {
      fprintf(stderr, "dbstats didn't return 'db' key?");
      abort();
   }


   r = mongoc_cursor_next (cursor, &doc);
   assert (!r);

   mongoc_cursor_destroy (cursor);
   mongoc_client_destroy (client);
}

static void
test_get_index_info (void)
{
   mongoc_collection_t *collection;
   mongoc_client_t *client;
   mongoc_index_opt_t opt1;
   mongoc_index_opt_t opt2;
   bson_error_t error = { 0 };
   mongoc_cursor_t *cursor;
   const bson_t *indexinfo;
   bson_t indexkey1;
   bson_t indexkey2;
   bson_t dummy = BSON_INITIALIZER;
   bson_iter_t idx_spec_iter;
   bson_iter_t idx_spec_iter_copy;
   bool r;
   const char *cur_idx_name;
   char *idx1_name = NULL;
   char *idx2_name = NULL;
   const char *id_idx_name = "_id_";
   int num_idxs = 0;

   client = test_framework_client_new ();
   ASSERT (client);

   collection = get_test_collection (client, "test_get_index_info");
   ASSERT (collection);

   /*
    * Try it on a collection that doesn't exist.
    */
   cursor = mongoc_collection_find_indexes (collection, &error);

   ASSERT (cursor);
   ASSERT (!error.domain);
   ASSERT (!error.code);

   ASSERT (!mongoc_cursor_next( cursor, &indexinfo ));

   mongoc_cursor_destroy (cursor);

   /* insert a dummy document so that the collection actually exists */
   r = mongoc_collection_insert (collection, MONGOC_INSERT_NONE, &dummy, NULL,
                                 &error);
   ASSERT (r);

   /* Try it on a collection with no secondary indexes.
    * We should just get back the index on _id.
    */
   cursor = mongoc_collection_find_indexes (collection, &error);
   ASSERT (cursor);
   ASSERT (!error.domain);
   ASSERT (!error.code);

   while (mongoc_cursor_next (cursor, &indexinfo)) {
      if (bson_iter_init (&idx_spec_iter, indexinfo) &&
          bson_iter_find (&idx_spec_iter, "name") &&
          BSON_ITER_HOLDS_UTF8 (&idx_spec_iter) &&
          (cur_idx_name = bson_iter_utf8 (&idx_spec_iter, NULL))) {
         assert (0 == strcmp (cur_idx_name, id_idx_name));
         ++num_idxs;
      } else {
         assert (false);
      }
   }

   assert (1 == num_idxs);

   mongoc_cursor_destroy (cursor);

   num_idxs = 0;
   indexinfo = NULL;

   bson_init (&indexkey1);
   BSON_APPEND_INT32 (&indexkey1, "raspberry", 1);
   idx1_name = mongoc_collection_keys_to_index_string (&indexkey1);
   mongoc_index_opt_init (&opt1);
   opt1.background = true;
   ASSERT_OR_PRINT (mongoc_collection_create_index (
      collection, &indexkey1, &opt1, &error), error);

   bson_init (&indexkey2);
   BSON_APPEND_INT32 (&indexkey2, "snozzberry", 1);
   idx2_name = mongoc_collection_keys_to_index_string (&indexkey2);
   mongoc_index_opt_init (&opt2);
   opt2.unique = true;
   ASSERT_OR_PRINT (mongoc_collection_create_index (
      collection, &indexkey2, &opt2, &error), error);

   /*
    * Now we try again after creating two indexes.
    */
   cursor = mongoc_collection_find_indexes (collection, &error);
   ASSERT (cursor);
   ASSERT (!error.domain);
   ASSERT (!error.code);

   while (mongoc_cursor_next (cursor, &indexinfo)) {
      if (bson_iter_init (&idx_spec_iter, indexinfo) &&
          bson_iter_find (&idx_spec_iter, "name") &&
          BSON_ITER_HOLDS_UTF8 (&idx_spec_iter) &&
          (cur_idx_name = bson_iter_utf8 (&idx_spec_iter, NULL))) {
         if (0 == strcmp (cur_idx_name, idx1_name)) {
            /* need to use the copy of the iter since idx_spec_iter may have gone
             * past the key we want */
            ASSERT (bson_iter_init_find (&idx_spec_iter_copy, indexinfo, "background"));
            ASSERT (BSON_ITER_HOLDS_BOOL (&idx_spec_iter_copy));
            ASSERT (bson_iter_bool (&idx_spec_iter_copy));
         } else if (0 == strcmp (cur_idx_name, idx2_name)) {
            ASSERT (bson_iter_init_find (&idx_spec_iter_copy, indexinfo, "unique"));
            ASSERT (BSON_ITER_HOLDS_BOOL (&idx_spec_iter_copy));
            ASSERT (bson_iter_bool (&idx_spec_iter_copy));
         } else {
            ASSERT ((0 == strcmp (cur_idx_name, id_idx_name)));
         }

         ++num_idxs;
      } else {
         assert (false);
      }
   }

   assert (3 == num_idxs);

   mongoc_cursor_destroy (cursor);

   bson_free (idx1_name);
   bson_free (idx2_name);

   mongoc_collection_destroy (collection);
   mongoc_client_destroy (client);
}


static void
test_aggregate_install (TestSuite *suite) {
   static test_aggregate_context_t test_aggregate_contexts[2][2][2];

   int wire_version, with_batch_size, with_options;
   char *legacy_or_modern;
   TestFuncWC func;
   char *name;
   test_aggregate_context_t *context;

   for (wire_version = 0; wire_version < 2; wire_version++) {
      for (with_batch_size = 0; with_batch_size < 2; with_batch_size++) {
         for (with_options = 0; with_options < 2; with_options++) {
            legacy_or_modern = wire_version ? "legacy" : "modern";
            func = wire_version ? test_aggregate_legacy : test_aggregate_modern;

            context = &test_aggregate_contexts
               [wire_version][with_batch_size][with_options];

            context->with_batch_size = (bool) with_batch_size;
            context->with_options = (bool) with_options;

            name = bson_strdup_printf (
               "/Collection/aggregate/%s/%s/%s",
               legacy_or_modern,
               context->with_batch_size ? "batch_size" : "no_batch_size",
               context->with_options ? "with_options" : "no_options");

            TestSuite_AddWC (suite, name, func, NULL, (void *) context);
            bson_free (name);
         }
      }
   }
}


static void
test_find_read_concern (void)
{
   mock_server_t *server;
   mongoc_client_t *client;
   mongoc_read_concern_t *rc;
   mongoc_collection_t *collection;
   mongoc_cursor_t *cursor;
   future_t *future;
   request_t *request;
   const bson_t *doc;

   server = mock_server_with_autoismaster (WIRE_VERSION_READ_CONCERN);
   mock_server_run (server);

   client = mongoc_client_new_from_uri (mock_server_get_uri (server));
   collection = mongoc_client_get_collection (client, "test", "test");

   /* No read_concern set */
   cursor = mongoc_collection_find (collection,
                                    MONGOC_QUERY_SLAVE_OK,
                                    0 /* skip */,
                                    0 /* limit */,
                                    0 /* batch_size */,
                                    tmp_bson ("{}"),
                                    NULL,
                                    NULL);

   future = future_cursor_next (cursor, &doc);
   request = mock_server_receives_command (server, "test", MONGOC_QUERY_SLAVE_OK,
                                           "{'find' : 'test', 'filter' : {  } }");
   mock_server_replies_simple (request, "{'ok': 1,"
         " 'cursor': {"
         "    'id': 0,"
         "    'ns': 'test.test',"
         "    'firstBatch': [{'_id': 123}]}}");
   ASSERT (future_get_bool (future));
   future_destroy (future);
   request_destroy (request);
   mongoc_cursor_destroy (cursor);

   /* readConcernLevel = local */
   rc = mongoc_read_concern_new ();
   mongoc_read_concern_set_level (rc, MONGOC_READ_CONCERN_LEVEL_LOCAL);
   mongoc_collection_set_read_concern (collection, rc);
   cursor = mongoc_collection_find (collection,
                                    MONGOC_QUERY_SLAVE_OK,
                                    0 /* skip */,
                                    0 /* limit */,
                                    0 /* batch_size */,
                                    tmp_bson ("{}"),
                                    NULL,
                                    NULL);

   future = future_cursor_next (cursor, &doc);
   request = mock_server_receives_command (server, "test", MONGOC_QUERY_SLAVE_OK,
         "{"
         "  'find' : 'test',"
         "  'filter' : {  },"
         "  'readConcern': {"
         "    'level': 'local'"
         "   }"
         "}");
   mock_server_replies_simple (request, "{'ok': 1,"
         " 'cursor': {"
         "    'id': 0,"
         "    'ns': 'test.test',"
         "    'firstBatch': [{'_id': 123}]}}");
   ASSERT (future_get_bool (future));
   future_destroy (future);
   request_destroy (request);
   mongoc_cursor_destroy (cursor);
   mongoc_read_concern_destroy (rc);

   /* readConcernLevel = random */
   rc = mongoc_read_concern_new ();
   mongoc_read_concern_set_level (rc, "random");
   mongoc_collection_set_read_concern (collection, rc);
   cursor = mongoc_collection_find (collection,
                                    MONGOC_QUERY_SLAVE_OK,
                                    0 /* skip */,
                                    0 /* limit */,
                                    0 /* batch_size */,
                                    tmp_bson ("{}"),
                                    NULL,
                                    NULL);

   future = future_cursor_next (cursor, &doc);
   request = mock_server_receives_command (server, "test", MONGOC_QUERY_SLAVE_OK,
         "{"
         "  'find' : 'test',"
         "  'filter' : {  },"
         "  'readConcern': {"
         "    'level': 'random'"
         "   }"
         "}");
   mock_server_replies_simple (request, "{'ok': 1,"
         " 'cursor': {"
         "    'id': 0,"
         "    'ns': 'test.test',"
         "    'firstBatch': [{'_id': 123}]}}");
   ASSERT (future_get_bool (future));
   future_destroy (future);
   request_destroy (request);
   mongoc_cursor_destroy (cursor);
   mongoc_read_concern_destroy (rc);

   /* empty readConcernLevel doesn't send anything */
   rc = mongoc_read_concern_new ();
   mongoc_collection_set_read_concern (collection, rc);
   cursor = mongoc_collection_find (collection,
                                    MONGOC_QUERY_SLAVE_OK,
                                    0 /* skip */,
                                    0 /* limit */,
                                    0 /* batch_size */,
                                    tmp_bson ("{}"),
                                    NULL,
                                    NULL);

   future = future_cursor_next (cursor, &doc);
   request = mock_server_receives_command (server, "test", MONGOC_QUERY_SLAVE_OK,
         "{"
         "  'find' : 'test',"
         "  'filter' : {  },"
         "  'readConcern': { '$exists': false }"
         "}");
   mock_server_replies_simple (request, "{'ok': 1,"
         " 'cursor': {"
         "    'id': 0,"
         "    'ns': 'test.test',"
         "    'firstBatch': [{'_id': 123}]}}");
   ASSERT (future_get_bool (future));
   future_destroy (future);
   request_destroy (request);
   mongoc_cursor_destroy (cursor);
   mongoc_read_concern_destroy (rc);

   /* readConcernLevel = NULL doesn't send anything */
   rc = mongoc_read_concern_new ();
   mongoc_read_concern_set_level (rc, NULL);
   mongoc_collection_set_read_concern (collection, rc);
   cursor = mongoc_collection_find (collection,
                                    MONGOC_QUERY_SLAVE_OK,
                                    0 /* skip */,
                                    0 /* limit */,
                                    0 /* batch_size */,
                                    tmp_bson ("{}"),
                                    NULL,
                                    NULL);

   future = future_cursor_next (cursor, &doc);
   request = mock_server_receives_command (server, "test", MONGOC_QUERY_SLAVE_OK,
         "{"
         "  'find' : 'test',"
         "  'filter' : {  },"
         "  'readConcern': { '$exists': false }"
         "}");
   mock_server_replies_simple (request, "{'ok': 1,"
         " 'cursor': {"
         "    'id': 0,"
         "    'ns': 'test.test',"
         "    'firstBatch': [{'_id': 123}]}}");
   ASSERT (future_get_bool (future));

   future_destroy (future);
   request_destroy (request);
   mongoc_cursor_destroy (cursor);
   mongoc_read_concern_destroy (rc);
   mongoc_collection_destroy(collection);
   mongoc_client_destroy(client);
   mock_server_destroy (server);
}


static void
test_aggregate_read_concern (void)
{
   mock_server_t *server;
   mongoc_client_t *client;
   mongoc_collection_t *collection;
   mongoc_read_concern_t *rc;
   future_t *future;
   request_t *request;
   mongoc_cursor_t *cursor;
   const bson_t *doc;

   server = mock_server_with_autoismaster (WIRE_VERSION_READ_CONCERN);
   mock_server_run (server);
   client = mongoc_client_new_from_uri (mock_server_get_uri (server));
   collection = mongoc_client_get_collection (client, "db", "collection");

   /* No readConcern */
   cursor = mongoc_collection_aggregate (
      collection,
      MONGOC_QUERY_NONE,
      tmp_bson ("[{'a': 1}]"),
      NULL,
      NULL);

   ASSERT (cursor);
   future = future_cursor_next (cursor, &doc);

   request = mock_server_receives_command (
      server, "db", MONGOC_QUERY_SLAVE_OK,
      "{"
      "  'aggregate' : 'collection',"
      "  'pipeline' : [{"
      "      'a' : 1"
      "  }],"
      "  'cursor' : {  },"
      "  'readConcern': { '$exists': false }"
      "}"
   );

   mock_server_replies_simple (request,
                               "{'ok': 1,"
                               " 'cursor': {"
                               "    'id': 0,"
                               "    'ns': 'db.collection',"
                               "    'firstBatch': [{'_id': 123}]"
                               "}}");

   ASSERT (future_get_bool (future));
   ASSERT_MATCH (doc, "{'_id': 123}");

   /* cursor is completed */
   assert (!mongoc_cursor_next (cursor, &doc));
   mongoc_cursor_destroy (cursor);
   request_destroy (request);
   future_destroy (future);

   /* readConcern: majority */
   rc = mongoc_read_concern_new ();
   mongoc_read_concern_set_level (rc, MONGOC_READ_CONCERN_LEVEL_MAJORITY);
   mongoc_collection_set_read_concern (collection, rc);
   cursor = mongoc_collection_aggregate (
      collection,
      MONGOC_QUERY_NONE,
      tmp_bson ("[{'a': 1}]"),
      NULL,
      NULL);

   ASSERT (cursor);
   future = future_cursor_next (cursor, &doc);

   request = mock_server_receives_command (
      server, "db", MONGOC_QUERY_SLAVE_OK,
      "{"
      "  'aggregate' : 'collection',"
      "  'pipeline' : [{"
      "      'a' : 1"
      "  }],"
      "  'cursor' : {  },"
      "  'readConcern': { 'level': 'majority'}"
      "}"
   );

   mock_server_replies_simple (request,
                               "{'ok': 1,"
                               " 'cursor': {"
                               "    'id': 0,"
                               "    'ns': 'db.collection',"
                               "    'firstBatch': [{'_id': 123}]"
                               "}}");

   ASSERT (future_get_bool (future));
   ASSERT_MATCH (doc, "{'_id': 123}");

   /* cursor is completed */
   assert (!mongoc_cursor_next (cursor, &doc));
   mongoc_cursor_destroy (cursor);
   request_destroy (request);
   future_destroy (future);

   mongoc_read_concern_destroy (rc);
   mongoc_collection_destroy (collection);
   mongoc_client_destroy (client);
   mock_server_destroy (server);
}



void
test_collection_install (TestSuite *suite)
{
   test_aggregate_install (suite);

   TestSuite_Add (suite, "/Collection/insert_bulk", test_insert_bulk);
   TestSuite_Add (suite,
                  "/Collection/insert_bulk_empty",
                  test_insert_bulk_empty);
   TestSuite_Add (suite,
                  "/Collection/bulk_insert/legacy/large",
                  test_legacy_bulk_insert_large);
   TestSuite_Add (suite,
                  "/Collection/bulk_insert/legacy/oversized_middle",
                  test_legacy_bulk_insert_oversized_middle);
   TestSuite_Add (suite,
                  "/Collection/bulk_insert/legacy/oversized_middle_continue",
                  test_legacy_bulk_insert_oversized_continue_middle);
   TestSuite_Add (suite,
                  "/Collection/bulk_insert/legacy/oversized_first",
                  test_legacy_bulk_insert_oversized_first);
   TestSuite_Add (suite,
                  "/Collection/bulk_insert/legacy/oversized_first_continue",
                  test_legacy_bulk_insert_oversized_first_continue);
   TestSuite_Add (suite,
                  "/Collection/bulk_insert/legacy/oversized_last",
                  test_legacy_bulk_insert_oversized_last);
   TestSuite_Add (suite,
                  "/Collection/bulk_insert/legacy/oversized_last_continue",
                  test_legacy_bulk_insert_oversized_last_continue);

   TestSuite_Add (suite, "/Collection/copy", test_copy);
   TestSuite_Add (suite, "/Collection/insert", test_insert);
   TestSuite_Add (suite, "/Collection/insert/oversize", test_legacy_insert_oversize_mongos);
   TestSuite_Add (suite, "/Collection/insert/keys", test_insert_command_keys);
   TestSuite_Add (suite, "/Collection/save", test_save);
   TestSuite_Add (suite, "/Collection/index", test_index);
   TestSuite_Add (suite, "/Collection/index_compound", test_index_compound);
   TestSuite_Add (suite, "/Collection/index_geo", test_index_geo);
   TestSuite_Add (suite, "/Collection/index_storage", test_index_storage);
   TestSuite_Add (suite, "/Collection/regex", test_regex);
   TestSuite_Add (suite, "/Collection/update", test_update);
   TestSuite_Add (suite, "/Collection/remove", test_remove);
   TestSuite_Add (suite, "/Collection/count", test_count);
   TestSuite_Add (suite, "/Collection/count_with_opts", test_count_with_opts);
   TestSuite_Add (suite, "/Collection/count/read_concern", test_count_read_concern);
   TestSuite_AddFull (suite, "/Collection/count/read_concern_live", test_count_read_concern_live, NULL, NULL, mongod_supports_majority_read_concern);
   TestSuite_Add (suite, "/Collection/drop", test_drop);
   TestSuite_Add (suite, "/Collection/aggregate", test_aggregate);
   TestSuite_Add (suite, "/Collection/aggregate/large", test_aggregate_large);
   TestSuite_Add (suite, "/Collection/aggregate/read_concern", test_aggregate_read_concern);
   TestSuite_AddFull (suite, "/Collection/aggregate/bypass_document_validation", test_aggregate_bypass, NULL, NULL, test_framework_skip_if_max_version_version_less_than_4);
   TestSuite_Add (suite, "/Collection/validate", test_validate);
   TestSuite_Add (suite, "/Collection/rename", test_rename);
   TestSuite_Add (suite, "/Collection/stats", test_stats);
   TestSuite_Add (suite, "/Collection/find_read_concern", test_find_read_concern);
   TestSuite_Add (suite, "/Collection/find_and_modify", test_find_and_modify);
   TestSuite_Add (suite, "/Collection/find_and_modify/write_concern",
                  test_find_and_modify_write_concern_wire_32);
   TestSuite_Add (suite, "/Collection/find_and_modify/write_concern_pre_32",
                  test_find_and_modify_write_concern_wire_pre_32);
   TestSuite_Add (suite, "/Collection/large_return", test_large_return);
   TestSuite_Add (suite, "/Collection/many_return", test_many_return);
   TestSuite_Add (suite, "/Collection/limit", test_find_limit);
   TestSuite_Add (suite, "/Collection/batch_size", test_find_batch_size);
   TestSuite_AddFull (suite, "/Collection/command_fully_qualified", test_command_fq, NULL, NULL, test_framework_skip_if_mongos);
   TestSuite_Add (suite, "/Collection/get_index_info", test_get_index_info);
}
