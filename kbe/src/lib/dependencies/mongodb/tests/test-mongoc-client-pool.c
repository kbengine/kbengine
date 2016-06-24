#include <mongoc.h>
#include "mongoc-client-pool-private.h"
#include "mongoc-array-private.h"


#include "TestSuite.h"
#include "test-libmongoc.h"


static void
test_mongoc_client_pool_basic (void)
{
   mongoc_client_pool_t *pool;
   mongoc_client_t *client;
   mongoc_uri_t *uri;

   uri = mongoc_uri_new("mongodb://127.0.0.1?maxpoolsize=1&minpoolsize=1");
   pool = mongoc_client_pool_new(uri);
   client = mongoc_client_pool_pop(pool);
   assert(client);
   mongoc_client_pool_push(pool, client);
   mongoc_uri_destroy(uri);
   mongoc_client_pool_destroy(pool);
}


static void
test_mongoc_client_pool_try_pop (void)
{
   mongoc_client_pool_t *pool;
   mongoc_client_t *client;
   mongoc_uri_t *uri;

   uri = mongoc_uri_new("mongodb://127.0.0.1?maxpoolsize=1&minpoolsize=1");
   pool = mongoc_client_pool_new(uri);
   client = mongoc_client_pool_pop(pool);
   assert(client);
   assert(!mongoc_client_pool_try_pop(pool));
   mongoc_client_pool_push(pool, client);
   mongoc_uri_destroy(uri);
   mongoc_client_pool_destroy(pool);
}

static void
test_mongoc_client_pool_min_size_dispose (void)
{
   mongoc_client_pool_t *pool;
   mongoc_client_t *client;
   mongoc_uri_t *uri;
   mongoc_array_t conns;
   int i;

   _mongoc_array_init (&conns, sizeof client);

   uri = mongoc_uri_new ("mongodb://127.0.0.1?maxpoolsize=10&minpoolsize=3");
   pool = mongoc_client_pool_new (uri);

   for (i = 0; i < 10; i++) {
      client = mongoc_client_pool_pop (pool);
      assert (client);
      _mongoc_array_append_val (&conns, client);
      assert (mongoc_client_pool_get_size (pool) == i + 1);
   }

   for (i = 0; i < 10; i++) {
      client = _mongoc_array_index (&conns, mongoc_client_t *, i);
      assert (client);
      mongoc_client_pool_push (pool, client);
   }

   assert (mongoc_client_pool_get_size (pool) == 3);
   _mongoc_array_clear (&conns);
   _mongoc_array_destroy (&conns);
   mongoc_uri_destroy (uri);
   mongoc_client_pool_destroy (pool);
}

static void
test_mongoc_client_pool_set_max_size (void)
{
   mongoc_client_pool_t *pool;
   mongoc_client_t *client;
   mongoc_uri_t *uri;
   mongoc_array_t conns;
   int i;

   _mongoc_array_init (&conns, sizeof client);

   uri = mongoc_uri_new ("mongodb://127.0.0.1?maxpoolsize=10&minpoolsize=3");
   pool = mongoc_client_pool_new (uri);

   for (i = 0; i < 5; i++) {
      client = mongoc_client_pool_pop (pool);
      assert (client);
      _mongoc_array_append_val (&conns, client);
      assert (mongoc_client_pool_get_size (pool) == i + 1);
   }

   mongoc_client_pool_max_size(pool,3);

   assert(mongoc_client_pool_try_pop(pool) == NULL);

   for (i = 0; i < 5; i++) {
      client = _mongoc_array_index (&conns, mongoc_client_t *, i);
      assert (client);
      mongoc_client_pool_push (pool, client);
   }

   _mongoc_array_clear (&conns);
   _mongoc_array_destroy (&conns);
   mongoc_uri_destroy (uri);
   mongoc_client_pool_destroy (pool);
}

static void
test_mongoc_client_pool_set_min_size (void)
{
   mongoc_client_pool_t *pool;
   mongoc_client_t *client;
   mongoc_uri_t *uri;
   mongoc_array_t conns;
   int i;

   _mongoc_array_init (&conns, sizeof client);

   uri = mongoc_uri_new ("mongodb://127.0.0.1?maxpoolsize=10&minpoolsize=3");
   pool = mongoc_client_pool_new (uri);

   for (i = 0; i < 10; i++) {
      client = mongoc_client_pool_pop (pool);
      assert (client);
      _mongoc_array_append_val (&conns, client);
      assert (mongoc_client_pool_get_size (pool) == i + 1);
   }

   mongoc_client_pool_min_size(pool,7);

   for (i = 0; i < 10; i++) {
      client = _mongoc_array_index (&conns, mongoc_client_t *, i);
      assert (client);
      mongoc_client_pool_push (pool, client);
   }

   assert (mongoc_client_pool_get_size (pool) == 7);
   
   _mongoc_array_clear (&conns);
   _mongoc_array_destroy (&conns);
   mongoc_uri_destroy (uri);
   mongoc_client_pool_destroy (pool);
}

#ifndef MONGOC_ENABLE_SSL
static void
test_mongoc_client_pool_ssl_disabled (void)
{
   mongoc_uri_t *uri = mongoc_uri_new ("mongodb://host/?ssl=true");

   ASSERT (uri);
   suppress_one_message ();
   ASSERT (NULL == mongoc_client_pool_new (uri));

   mongoc_uri_destroy (uri);
}
#endif

void
test_client_pool_install (TestSuite *suite)
{
   TestSuite_Add (suite, "/ClientPool/basic", test_mongoc_client_pool_basic);
   TestSuite_Add (suite, "/ClientPool/try_pop", test_mongoc_client_pool_try_pop);
   TestSuite_Add (suite, "/ClientPool/min_size_dispose", test_mongoc_client_pool_min_size_dispose);
   TestSuite_Add (suite, "/ClientPool/set_max_size", test_mongoc_client_pool_set_max_size);
   TestSuite_Add (suite, "/ClientPool/set_min_size", test_mongoc_client_pool_set_min_size);

#ifndef MONGOC_ENABLE_SSL
   TestSuite_Add (suite, "/ClientPool/ssl_disabled", test_mongoc_client_pool_ssl_disabled);
#endif
}
