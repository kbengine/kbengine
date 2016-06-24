#include <mongoc.h>

#include "TestSuite.h"
#include "test-conveniences.h"
#include "test-libmongoc.h"

static void
server_selection_error_dns (const char *uri, const char *errmsg, bool assert_as)
{

   mongoc_client_t *client;
   mongoc_collection_t *collection;
   bson_error_t error;
   bson_t *command;
   bson_t reply;
   bool success;

   client = mongoc_client_new (uri);

   collection = mongoc_client_get_collection (client, "test", "test");

   command = tmp_bson("{'ping': 1}");
   success = mongoc_collection_command_simple (collection, command, NULL, &reply, &error);
   ASSERT_OR_PRINT(success == assert_as, error);

   if (!success && errmsg) {
      ASSERT_CMPSTR(error.message, errmsg);
   }

   bson_destroy (&reply);
   mongoc_collection_destroy (collection);
   mongoc_client_destroy (client);
}

static void
test_server_selection_error_dns_single (void)
{
   server_selection_error_dns (
      "mongodb://non-existing-localhost:27017/",
      "No suitable servers found (`serverselectiontryonce` set): [Failed to resolve 'non-existing-localhost']",
      false
   );
}

static void
test_server_selection_error_dns_multi_fail (void)
{
   server_selection_error_dns (
      "mongodb://non-existing-localhost:27017,other-non-existing-localhost:27017/",
      "No suitable servers found (`serverselectiontryonce` set): [Failed to resolve 'non-existing-localhost'] [Failed to resolve 'other-non-existing-localhost']",
      false
   );
}

static void
test_server_selection_error_dns_multi_success (void *context)
{
   char *uri_str;

   uri_str = bson_strdup_printf (
      "mongodb://non-existing-localhost:27017,"
      "%s:%d,"
      "other-non-existing-localhost:27017/",
      test_framework_get_host (),
      test_framework_get_port ());

   server_selection_error_dns (uri_str, "", true);

   bson_free (uri_str);
}

static void
test_server_selection_uds_auth_failure (void *context)
{
   mongoc_client_t *client;
   bson_error_t error;
   bson_t reply;
   char *path;
   char *uri;

   path = test_framework_get_unix_domain_socket_path ();
   uri = bson_strdup_printf ("mongodb://user:wrongpass@%s", path);
   client = mongoc_client_new (uri);
   test_framework_set_ssl_opts (client);

   assert (client);

   ASSERT_OR_PRINT (! mongoc_client_get_server_status (client, NULL,
                                                       &reply, &error), error);

   ASSERT_CMPINT (error.domain, ==, MONGOC_ERROR_CLIENT);
   ASSERT_CMPINT (error.code, ==, MONGOC_ERROR_CLIENT_AUTHENTICATE);

   bson_destroy (&reply);
   bson_free (path);
   bson_free (uri);

   mongoc_client_destroy (client);
}

static void
test_server_selection_uds_not_found (void *context)
{
   mongoc_client_t *client;
   bson_error_t error;
   bson_t reply;

   client = mongoc_client_new ("mongodb:///tmp/mongodb-so-close.sock");
   test_framework_set_ssl_opts (client);

   assert (client);

   ASSERT_OR_PRINT (! mongoc_client_get_server_status (client, NULL,
                                                       &reply, &error), error);

   ASSERT_CMPINT (error.domain, ==, MONGOC_ERROR_SERVER_SELECTION);
   ASSERT_CMPINT (error.code, ==, MONGOC_ERROR_SERVER_SELECTION_FAILURE);

   bson_destroy (&reply);

   mongoc_client_destroy (client);
}


void
test_server_selection_errors_install (TestSuite *suite)
{
   TestSuite_Add (suite, "/server_selection/errors/dns/single", test_server_selection_error_dns_single);
   TestSuite_Add (suite, "/server_selection/errors/dns/multi/fail", test_server_selection_error_dns_multi_fail);
   TestSuite_AddFull (suite, "/server_selection/errors/dns/multi/success", test_server_selection_error_dns_multi_success, NULL, NULL, test_framework_skip_if_single);
   TestSuite_AddFull (suite, "/server_selection/errors/uds/auth_failure", test_server_selection_uds_auth_failure, NULL, NULL, test_framework_skip_if_windows);
   TestSuite_AddFull (suite, "/server_selection/errors/uds/not_found", test_server_selection_uds_not_found, NULL, NULL, test_framework_skip_if_windows);
}
