#include <mongoc.h>
#include <assert.h>

#include "TestSuite.h"
#include "test-libmongoc.h"
#include "mock_server/mock-rs.h"
#include "mock_server/future-functions.h"
#include "mongoc-cursor-private.h"
#include "mongoc-collection-private.h"
#include "test-conveniences.h"


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
test_get_host (void)
{
   const mongoc_host_list_t *hosts;
   mongoc_host_list_t host;
   mongoc_client_t *client;
   mongoc_cursor_t *cursor;
   char *uri_str = test_framework_get_uri_str ();
   mongoc_uri_t *uri;
   const bson_t *doc;
   bson_error_t error;
   bool r;
   bson_t q = BSON_INITIALIZER;

   uri = mongoc_uri_new (uri_str);

   hosts = mongoc_uri_get_hosts(uri);

   client = test_framework_client_new ();
   cursor = _mongoc_cursor_new(client, "test.test", MONGOC_QUERY_NONE, 0, 1, 1,
                               false, &q, NULL, NULL, NULL);
   r = mongoc_cursor_next(cursor, &doc);
   if (!r && mongoc_cursor_error(cursor, &error)) {
      MONGOC_ERROR("%s", error.message);
      abort();
   }

   assert (doc == mongoc_cursor_current (cursor));

   mongoc_cursor_get_host(cursor, &host);

   /* In a production deployment the driver can discover servers not in the seed
    * list, but for this test assume the cursor uses one of the seeds. */
   while (hosts) {
      if (!strcmp (host.host_and_port, hosts->host_and_port)) {
         /* the cursor is using this server */
         ASSERT_CMPSTR (host.host, hosts->host);
         ASSERT_CMPINT (host.port, ==, hosts->port);
         ASSERT_CMPINT (host.family, ==, hosts->family);
         break;
      }

      hosts = hosts->next;
   }

   if (!hosts) {
      MONGOC_ERROR ("cursor using host %s not in seeds: %s",
                    host.host_and_port, uri_str);
      abort ();
   }

   bson_free (uri_str);
   mongoc_uri_destroy(uri);
   mongoc_cursor_destroy (cursor);
   mongoc_client_destroy (client);
}

static void
test_clone (void)
{
   mongoc_cursor_t *clone;
   mongoc_cursor_t *cursor;
   mongoc_client_t *client;
   const bson_t *doc;
   bson_error_t error;
   bool r;
   bson_t q = BSON_INITIALIZER;

   client = test_framework_client_new ();

   {
      /*
       * Ensure test.test has a document.
       */

      mongoc_collection_t *col;

      col = mongoc_client_get_collection (client, "test", "test");
      r = mongoc_collection_insert (col, MONGOC_INSERT_NONE, &q, NULL, &error);
      ASSERT (r);

      mongoc_collection_destroy (col);
   }

   cursor = _mongoc_cursor_new(client, "test.test", MONGOC_QUERY_NONE, 0, 1, 1,
                               false, &q, NULL, NULL, NULL);
   ASSERT(cursor);

   r = mongoc_cursor_next(cursor, &doc);
   if (!r || mongoc_cursor_error(cursor, &error)) {
      MONGOC_ERROR("%s", error.message);
      abort();
   }
   ASSERT (doc);

   clone = mongoc_cursor_clone(cursor);
   ASSERT(cursor);

   r = mongoc_cursor_next(clone, &doc);
   if (!r || mongoc_cursor_error(clone, &error)) {
      MONGOC_ERROR("%s", error.message);
      abort();
   }
   ASSERT (doc);

   mongoc_cursor_destroy(cursor);
   mongoc_cursor_destroy(clone);
   mongoc_client_destroy(client);
}


static void
test_invalid_query (void)
{
   mongoc_client_t *client;
   mongoc_cursor_t *cursor;
   bson_error_t error;
   const bson_t *doc = NULL;
   bson_t *q;
   bool r;

   client = test_framework_client_new ();
   assert (client);

   q = BCON_NEW ("foo", BCON_INT32 (1), "$orderby", "{", "}");

   cursor = _mongoc_cursor_new (client, "test.test", MONGOC_QUERY_NONE, 0, 1, 1,
                                false, q, NULL, NULL, NULL);
   assert (!mongoc_cursor_is_alive (cursor));
   r = mongoc_cursor_next (cursor, &doc);
   assert (!r);
   mongoc_cursor_error (cursor, &error);
   assert (strstr (error.message, "$query"));
   assert (error.domain == MONGOC_ERROR_CURSOR);
   assert (error.code == MONGOC_ERROR_CURSOR_INVALID_CURSOR);
   assert (doc == NULL);

   bson_destroy (q);
   mongoc_cursor_destroy (cursor);
   mongoc_client_destroy (client);
}

/* test killing a cursor with mongo_cursor_destroy and a real server */
static void
test_kill_cursor_live (void)
{
   mongoc_client_t *client;
   mongoc_collection_t *collection;
   bson_t *b;
   mongoc_bulk_operation_t *bulk;
   int i;
   bson_error_t error;
   uint32_t hint;
   bool r;
   mongoc_cursor_t *cursor;
   const bson_t *doc;
   int64_t cursor_id;

   client = test_framework_client_new ();
   collection = get_test_collection (client, "test");
   b = tmp_bson ("{}");
   bulk = mongoc_collection_create_bulk_operation (collection, true, NULL);
   for (i = 0; i < 200; i++) {
      mongoc_bulk_operation_insert (bulk, b);
   }

   hint = mongoc_bulk_operation_execute (bulk, NULL, &error);
   ASSERT_OR_PRINT (hint > 0, error);

   cursor = mongoc_collection_find (collection, MONGOC_QUERY_NONE,
                                    0, 0, 0, /* batch size 2 */
                                    b, NULL, NULL);

   r = mongoc_cursor_next (cursor, &doc);
   ASSERT (r);
   cursor_id = mongoc_cursor_get_id (cursor);
   ASSERT (cursor_id);

   /* sends OP_KILLCURSORS or killCursors command to server */
   mongoc_cursor_destroy (cursor);

   cursor = _mongoc_cursor_new (client, collection->ns, MONGOC_QUERY_NONE,
                                0, 0, 0, false /* is_command */,
                                b, NULL, NULL, NULL);

   cursor->rpc.reply.cursor_id = cursor_id;
   cursor->sent = true;
   cursor->end_of_event = true;  /* meaning, "finished reading first batch" */
   r = mongoc_cursor_next (cursor, &doc);
   ASSERT (!r);
   ASSERT (mongoc_cursor_error (cursor, &error));
   ASSERT_ERROR_CONTAINS (error,
                          MONGOC_ERROR_CURSOR,
                          16,
                          "cursor is invalid");

   mongoc_cursor_destroy (cursor);
   mongoc_bulk_operation_destroy (bulk);
   mongoc_collection_destroy (collection);
   mongoc_client_destroy (client);
}


/* test OP_KILLCURSORS or the killCursors command with mock servers */
static void
_test_kill_cursors (bool pooled,
                    bool use_killcursors_cmd)
{
   mock_rs_t *rs;
   mongoc_client_pool_t *pool = NULL;
   mongoc_client_t *client;
   mongoc_collection_t *collection;
   bson_t *q = BCON_NEW ("a", BCON_INT32 (1));
   mongoc_read_prefs_t *prefs;
   mongoc_cursor_t *cursor;
   const bson_t *doc = NULL;
   future_t *future;
   request_t *request;
   bson_error_t error;
   request_t *kill_cursors;
   const char *ns_out;
   int64_t cursor_id_out;

   rs = mock_rs_with_autoismaster (
      use_killcursors_cmd ? 4 : 3, /* wire version */
      true,                        /* has primary */
      5,                           /* number of secondaries */
      0);                          /* number of arbiters */

   mock_rs_run (rs);

   if (pooled) {
      pool = mongoc_client_pool_new (mock_rs_get_uri (rs));
      client = mongoc_client_pool_pop (pool);
   } else {
      client = mongoc_client_new_from_uri (mock_rs_get_uri (rs));
   }

   collection = mongoc_client_get_collection (client, "db", "collection");

   prefs = mongoc_read_prefs_new (MONGOC_READ_SECONDARY);
   cursor = mongoc_collection_find (collection, MONGOC_QUERY_NONE, 0, 0, 0,
                                    q, NULL, prefs);

   future = future_cursor_next (cursor, &doc);
   request = mock_rs_receives_request (rs);

   /* reply as appropriate to OP_QUERY or find command */
   mock_rs_replies_to_find (request, MONGOC_QUERY_SLAVE_OK, 123, 1,
                            "db.collection", "{'b': 1}", use_killcursors_cmd);

   if (!future_get_bool (future)) {
      mongoc_cursor_error (cursor, &error);
      fprintf (stderr, "%s\n", error.message);
      abort ();
   };

   ASSERT_MATCH (doc, "{'b': 1}");
   ASSERT_CMPINT (123, ==, (int) mongoc_cursor_get_id (cursor));

   future_destroy (future);
   future = future_cursor_destroy (cursor);

   if (use_killcursors_cmd) {
      kill_cursors = mock_rs_receives_command (
         rs, "db",
         MONGOC_QUERY_SLAVE_OK,
         NULL);

      /* mock server framework can't test "cursors" array, CDRIVER-994 */
      ASSERT (BCON_EXTRACT ((bson_t *) request_get_doc (kill_cursors, 0),
                            "killCursors", BCONE_UTF8 (ns_out),
                            "cursors", "[", BCONE_INT64 (cursor_id_out), "]"));

      ASSERT_CMPSTR ("collection", ns_out);
      ASSERT_CMPINT64 ((int64_t) 123, ==, cursor_id_out);

      mock_rs_replies_simple (request, "{'ok': 1}");
   } else {
      kill_cursors = mock_rs_receives_kill_cursors (rs, 123);
   }

   /* OP_KILLCURSORS was sent to the right secondary */
   ASSERT_CMPINT (request_get_server_port (kill_cursors), ==,
                  request_get_server_port (request));

   assert (future_wait (future));

   request_destroy (kill_cursors);
   request_destroy (request);
   future_destroy (future);
   mongoc_read_prefs_destroy (prefs);
   mongoc_collection_destroy (collection);
   bson_destroy (q);

   if (pooled) {
      mongoc_client_pool_push (pool, client);
      mongoc_client_pool_destroy (pool);
   } else {
      mongoc_client_destroy (client);
   }

   mock_rs_destroy (rs);
}


static void
test_kill_cursors_single (void)
{
   _test_kill_cursors (false, false);
}


static void
test_kill_cursors_pooled (void)
{
   _test_kill_cursors (true, false);
}


static void
test_kill_cursors_single_cmd (void)
{
   _test_kill_cursors (false, true);
}


static void
test_kill_cursors_pooled_cmd (void)
{
   _test_kill_cursors (true, true);
}


static void
_test_getmore_fail (bool has_primary,
                    bool pooled)
{
   mock_rs_t *rs;
   mongoc_client_pool_t *pool = NULL;
   mongoc_client_t *client;
   mongoc_collection_t *collection;
   bson_t *q = BCON_NEW ("a", BCON_INT32 (1));
   mongoc_read_prefs_t *prefs;
   mongoc_cursor_t *cursor;
   const bson_t *doc = NULL;
   future_t *future;
   request_t *request;

   /* wire version 0, five secondaries, no arbiters */
   rs = mock_rs_with_autoismaster (0, has_primary, 5, 0);
   mock_rs_run (rs);

   if (pooled) {
      pool = mongoc_client_pool_new (mock_rs_get_uri (rs));
      client = mongoc_client_pool_pop (pool);
   } else {
      client = mongoc_client_new_from_uri (mock_rs_get_uri (rs));
   }

   collection = mongoc_client_get_collection (client, "test", "test");

   prefs = mongoc_read_prefs_new (MONGOC_READ_SECONDARY);
   cursor = mongoc_collection_find (collection, MONGOC_QUERY_NONE, 0, 0, 0,
                                    q, NULL, prefs);

   future = future_cursor_next (cursor, &doc);
   request = mock_rs_receives_query (rs, "test.test", MONGOC_QUERY_SLAVE_OK,
                                     0, 0, "{'a': 1}", NULL);

   mock_rs_replies (request, 0, 123, 0, 1, "{'b': 1}");
   assert (future_get_bool (future));
   ASSERT_MATCH (doc, "{'b': 1}");
   ASSERT_CMPINT (123, ==, (int) mongoc_cursor_get_id (cursor));

   future_destroy (future);
   future = future_cursor_next (cursor, &doc);
   request = mock_rs_receives_getmore (rs, "test.test", 0, 123);
   suppress_one_message ();
   mock_rs_hangs_up (request);
   assert (! future_get_bool (future));
   request_destroy (request);

   future_destroy (future);
   future = future_cursor_destroy (cursor);

   /* driver does not reconnect just to send killcursors */
   mock_rs_set_request_timeout_msec (rs, 100);
   assert (! mock_rs_receives_kill_cursors (rs, 123));

   future_wait (future);
   future_destroy (future);
   mongoc_read_prefs_destroy (prefs);
   mongoc_collection_destroy (collection);
   bson_destroy (q);

   if (pooled) {
      mongoc_client_pool_push (pool, client);
      mongoc_client_pool_destroy (pool);
   } else {
      mongoc_client_destroy (client);
   }

   mock_rs_destroy (rs);
}


static void
test_getmore_fail_with_primary_single (void)
{
   _test_getmore_fail (true, false);
}


static void
test_getmore_fail_with_primary_pooled (void)
{
   _test_getmore_fail (true, true);
}


static void
test_getmore_fail_no_primary_pooled (void)
{
   _test_getmore_fail (false, true);
}


static void
test_getmore_fail_no_primary_single (void)
{
   _test_getmore_fail (false, false);
}


/* We already test that mongoc_cursor_destroy sends OP_KILLCURSORS in
 * test_kill_cursors_single / pooled. Here, test explicit
 * mongoc_client_kill_cursor. */
static void
_test_client_kill_cursor (bool has_primary,
                          bool wire_version_4)
{
   mock_rs_t *rs;
   mongoc_client_t *client;
   mongoc_read_prefs_t *read_prefs;
   bson_error_t error;
   future_t *future;
   request_t *request;

   rs = mock_rs_with_autoismaster (wire_version_4 ? 4 : 3,
                                   has_primary,  /* maybe a primary*/
                                   1,            /* definitely a secondary */
                                   0);           /* no arbiter */
   mock_rs_run (rs);
   client = mongoc_client_new_from_uri (mock_rs_get_uri (rs));
   read_prefs = mongoc_read_prefs_new (MONGOC_READ_SECONDARY);

   /* make client open a connection - it won't open one to kill a cursor */
   future = future_client_command_simple (client, "admin",
                                          tmp_bson ("{'foo': 1}"),
                                          read_prefs, NULL, &error);

   request = mock_rs_receives_command (rs, "admin",
                                       MONGOC_QUERY_SLAVE_OK, NULL);

   mock_rs_replies_simple (request, "{'ok': 1}");
   ASSERT_OR_PRINT (future_get_bool (future), error);
   request_destroy (request);
   future_destroy (future);

   future = future_client_kill_cursor (client, 123);
   mock_rs_set_request_timeout_msec (rs, 100);

   /* we don't pass namespace so client always sends legacy OP_KILLCURSORS */
   request = mock_rs_receives_kill_cursors (rs, 123);

   if (has_primary) {
      assert (request);

      /* weird but true. see mongoc_client_kill_cursor's documentation */
      assert (mock_rs_request_is_to_primary (rs, request));

      request_destroy (request);  /* server has no reply to OP_KILLCURSORS */
   } else {
      /* TODO: catch and check warning */
      assert (!request);
   }

   future_wait (future);  /* no return value */
   future_destroy (future);
   mongoc_read_prefs_destroy (read_prefs);
   mongoc_client_destroy (client);
   mock_rs_destroy (rs);
}

static void
test_client_kill_cursor_with_primary (void)
{
   _test_client_kill_cursor (true, false);
}


static void
test_client_kill_cursor_without_primary (void)
{
   _test_client_kill_cursor (false, false);
}


static void
test_client_kill_cursor_with_primary_wire_version_4 (void)
{
   _test_client_kill_cursor (true, true);
}


static void
test_client_kill_cursor_without_primary_wire_version_4 (void)
{
   _test_client_kill_cursor (false, true);
}


void
test_cursor_install (TestSuite *suite)
{
   TestSuite_Add (suite, "/Cursor/get_host", test_get_host);
   TestSuite_Add (suite, "/Cursor/clone", test_clone);
   TestSuite_Add (suite, "/Cursor/invalid_query", test_invalid_query);
   TestSuite_Add (suite, "/Cursor/kill/live", test_kill_cursor_live);
   TestSuite_Add (suite, "/Cursor/kill/single", test_kill_cursors_single);
   TestSuite_Add (suite, "/Cursor/kill/pooled", test_kill_cursors_pooled);
   TestSuite_Add (suite, "/Cursor/kill/single/cmd", test_kill_cursors_single_cmd);
   TestSuite_Add (suite, "/Cursor/kill/pooled/cmd", test_kill_cursors_pooled_cmd);
   TestSuite_Add (suite, "/Cursor/getmore_fail/with_primary/pooled",
                  test_getmore_fail_with_primary_pooled);
   TestSuite_Add (suite, "/Cursor/getmore_fail/with_primary/single",
                  test_getmore_fail_with_primary_single);
   TestSuite_Add (suite, "/Cursor/getmore_fail/no_primary/pooled",
                  test_getmore_fail_no_primary_pooled);
   TestSuite_Add (suite, "/Cursor/getmore_fail/no_primary/single",
                  test_getmore_fail_no_primary_single);

   TestSuite_Add (suite, "/Cursor/client_kill_cursor/with_primary",
                  test_client_kill_cursor_with_primary);
   TestSuite_Add (suite, "/Cursor/client_kill_cursor/without_primary",
                  test_client_kill_cursor_without_primary);
   TestSuite_Add (suite, "/Cursor/client_kill_cursor/with_primary/wv4",
                  test_client_kill_cursor_with_primary_wire_version_4);
   TestSuite_Add (suite, "/Cursor/client_kill_cursor/without_primary/wv4",
                  test_client_kill_cursor_without_primary_wire_version_4);
}
