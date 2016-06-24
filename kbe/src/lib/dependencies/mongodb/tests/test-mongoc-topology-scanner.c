#include <mongoc.h>

#include "mongoc-util-private.h"
#include "mongoc-client-private.h"

#include "mongoc-tests.h"
#include "TestSuite.h"
#include "mock_server/mock-server.h"
#include "mock_server/future.h"
#include "mock_server/future-functions.h"

#undef MONGOC_LOG_DOMAIN
#define MONGOC_LOG_DOMAIN "topology-scanner-test"

#define TIMEOUT 20000 /* milliseconds */
#define NSERVERS 10

#define TRUST_DIR "tests/trust_dir"
#define CAFILE TRUST_DIR "/verify/mongo_root.pem"
#define PEMFILE_NOPASS TRUST_DIR "/keys/mongodb.com.pem"

static void
test_topology_scanner_helper (uint32_t      id,
                              const bson_t *bson,
                              int64_t       rtt_msec,
                              void         *data,
                              bson_error_t *error)
{
   bson_iter_t iter;
   int *finished = (int*)data;
   uint32_t max_wire_version;

   if (error->code) {
      fprintf (stderr, "scanner error: %s\n", error->message);
      abort ();
   }

   /* mock servers are configured to return their ids as max wire version */
   assert (bson);
   assert (bson_iter_init_find (&iter, bson, "maxWireVersion"));
   assert (BSON_ITER_HOLDS_INT32 (&iter));
   max_wire_version = (uint32_t) bson_iter_int32 (&iter);
   ASSERT_CMPINT (max_wire_version, ==, id);

   (*finished)--;
}

static void
_test_topology_scanner(bool with_ssl)
{
   mock_server_t *servers[NSERVERS];
   mongoc_topology_scanner_t *topology_scanner;
   int i;
   bson_t q = BSON_INITIALIZER;
   int finished = NSERVERS * 3;
   bool more_to_do;

#ifdef MONGOC_ENABLE_SSL
   mongoc_ssl_opt_t sopt = { 0 };
   mongoc_ssl_opt_t copt = { 0 };
#endif

   topology_scanner = mongoc_topology_scanner_new (
         NULL, &test_topology_scanner_helper, &finished);

#ifdef MONGOC_ENABLE_SSL
   if (with_ssl) {
      copt.ca_file = CAFILE;
      copt.weak_cert_validation = 1;

      mongoc_topology_scanner_set_ssl_opts (topology_scanner, &copt);
   }
#endif

   for (i = 0; i < NSERVERS; i++) {
      /* use max wire versions just to distinguish among responses */
      servers[i] = mock_server_with_autoismaster (i);
      mock_server_set_rand_delay (servers[i], true);

#ifdef MONGOC_ENABLE_SSL
      if (with_ssl) {
         sopt.pem_file = PEMFILE_NOPASS;
         sopt.ca_file = CAFILE;

         mock_server_set_ssl_opts (servers[i], &sopt);
      }
#endif

      mock_server_run (servers[i]);

      mongoc_topology_scanner_add(
            topology_scanner,
            mongoc_uri_get_hosts (mock_server_get_uri (servers[i])),
            (uint32_t) i);
   }

   for (i = 0; i < 3; i++) {
      mongoc_topology_scanner_start (topology_scanner, TIMEOUT, false);

      more_to_do = mongoc_topology_scanner_work (topology_scanner, TIMEOUT);

      assert(! more_to_do);

      mongoc_topology_scanner_reset (topology_scanner);
   }

   assert(finished == 0);

   mongoc_topology_scanner_destroy (topology_scanner);

   bson_destroy (&q);

   for (i = 0; i < NSERVERS; i++) {
      mock_server_destroy (servers[i]);
   }
}


void
test_topology_scanner ()
{
   _test_topology_scanner (false);
}


#ifdef MONGOC_ENABLE_SSL
void
test_topology_scanner_ssl ()
{
   _test_topology_scanner (true);
}
#endif


/* 
 * Servers discovered by a scan should be checked during that scan, CDRIVER-751. 
 */
void
test_topology_scanner_discovery ()
{
   mock_server_t *primary;
   mock_server_t *secondary;
   char *primary_response;
   char *secondary_response;
   mongoc_client_t *client;
   mongoc_topology_scanner_t *scanner;
   char *uri_str;
   mongoc_read_prefs_t *secondary_pref;
   bson_error_t error;
   future_t *future;
   request_t *request;
   mongoc_server_description_t *sd;

   primary = mock_server_new ();
   secondary = mock_server_new ();
   mock_server_run (primary);
   mock_server_run (secondary);

   primary_response = bson_strdup_printf (
      "{'ok': 1, "
      " 'ismaster': true,"
      " 'setName': 'rs',"
      " 'hosts': ['%s', '%s']}",
      mock_server_get_host_and_port (primary),
      mock_server_get_host_and_port (secondary));

   secondary_response = bson_strdup_printf (
      "{'ok': 1, "
      " 'ismaster': false,"
      " 'secondary': true,"
      " 'setName': 'rs',"
      " 'hosts': ['%s', '%s']}",
      mock_server_get_host_and_port (primary),
      mock_server_get_host_and_port (secondary));

   uri_str = bson_strdup_printf ("mongodb://%s/?replicaSet=rs",
                                 mock_server_get_host_and_port (primary));
   client = mongoc_client_new (uri_str);
   scanner = client->topology->scanner;
   secondary_pref = mongoc_read_prefs_new (MONGOC_READ_SECONDARY_PREFERRED);

   future = future_topology_select (client->topology, MONGOC_SS_READ,
                                    secondary_pref, 15, &error);

   /* a single scan discovers *and* checks the secondary */
   AWAIT (scanner->async->ncmds == 1);
   request = mock_server_receives_ismaster (primary);
   mock_server_replies_simple (request, primary_response);
   request_destroy (request);

   /* let client process that response */
   _mongoc_usleep (250 * 1000);

   /* a check of the secondary is scheduled in this scan */
   AWAIT (scanner->async->ncmds == 1);

   request = mock_server_receives_ismaster (secondary);
   mock_server_replies_simple (request, secondary_response);
   /* scan completes */
   AWAIT (scanner->async->ncmds == 0);
   ASSERT_OR_PRINT ((sd = future_get_mongoc_server_description_ptr (future)),
                    error);

   ASSERT_CMPSTR (sd->host.host_and_port,
                  mock_server_get_host_and_port (secondary));

   mongoc_server_description_destroy (sd);
   future_destroy (future);
   request_destroy (request);
   mongoc_read_prefs_destroy (secondary_pref);
   bson_free (secondary_response);
   bson_free (primary_response);
   bson_free (uri_str);
   mongoc_client_destroy (client);
   mock_server_destroy (secondary);
   mock_server_destroy (primary);
}


/* scanner shouldn't spin if two primaries point at each other */
void
test_topology_scanner_oscillate ()
{
   mock_server_t *server0;
   mock_server_t *server1;
   char *server0_response;
   char *server1_response;
   mongoc_client_t *client;
   mongoc_topology_scanner_t *scanner;
   char *uri_str;
   mongoc_read_prefs_t *primary_pref;
   bson_error_t error;
   future_t *future;
   request_t *request;

   server0 = mock_server_new ();
   server1 = mock_server_new ();
   mock_server_run (server0);
   mock_server_run (server1);

   /* server 0 says it's primary, but only server 1 is in the set */
   server0_response = bson_strdup_printf (
      "{'ok': 1, "
      " 'ismaster': true,"
      " 'setName': 'rs',"
      " 'hosts': ['%s']}",
      mock_server_get_host_and_port (server1));

   /* the opposite */
   server1_response = bson_strdup_printf (
      "{'ok': 1, "
      " 'ismaster': true,"
      " 'setName': 'rs',"
      " 'hosts': ['%s']}",
      mock_server_get_host_and_port (server0));

   /* start with server 0 */
   uri_str = bson_strdup_printf ("mongodb://%s/?replicaSet=rs",
                                 mock_server_get_host_and_port (server0));
   client = mongoc_client_new (uri_str);
   scanner = client->topology->scanner;
   primary_pref = mongoc_read_prefs_new (MONGOC_READ_PRIMARY);

   assert (!scanner->async->ncmds);
   future = future_topology_select (client->topology, MONGOC_SS_READ,
                                    primary_pref, 15, &error);

   /* a single scan discovers servers 0 and 1 */
   request = mock_server_receives_ismaster (server0);
   mock_server_replies_simple (request, server0_response);
   request_destroy (request);

   /* let client process that response */
   _mongoc_usleep (250 * 1000);
   AWAIT (scanner->async->ncmds == 1);

   request = mock_server_receives_ismaster (server1);
   mock_server_replies_simple (request, server1_response);

   /* we don't schedule another check of server0 */
   _mongoc_usleep (250 * 1000);
   AWAIT (scanner->async->ncmds == 0);

   assert (!future_get_mongoc_server_description_ptr (future));
   assert (scanner->async->ncmds == 0);

   future_destroy (future);
   request_destroy (request);
   mongoc_read_prefs_destroy (primary_pref);
   bson_free (server1_response);
   bson_free (server0_response);
   bson_free (uri_str);
   mongoc_client_destroy (client);
   mock_server_destroy (server1);
   mock_server_destroy (server0);
}


void
test_topology_scanner_install (TestSuite *suite)
{
   TestSuite_Add (suite, "/TOPOLOGY/scanner", test_topology_scanner);
#ifdef MONGOC_ENABLE_SSL
   TestSuite_Add (suite, "/TOPOLOGY/scanner_ssl", test_topology_scanner_ssl);
#endif
   TestSuite_Add (suite, "/TOPOLOGY/scanner_discovery",
                  test_topology_scanner_discovery);
   TestSuite_Add (suite, "/TOPOLOGY/scanner_oscillate",
                  test_topology_scanner_oscillate);
}
