#include <mongoc.h>

#include <stdio.h>
#include <mongoc-set-private.h>

#include "json-test.h"

#include "mongoc-client-private.h"
#include "mongoc-server-description-private.h"
#include "mongoc-topology-description-private.h"
#include "mongoc-topology-private.h"
#include "mongoc-util-private.h"

#include "TestSuite.h"
#include "test-conveniences.h"

#include <limits.h>
#include <stdlib.h>

#ifdef _MSC_VER
#define PATH_MAX 1024
#define realpath(path, expanded) GetFullPathName(path, PATH_MAX, expanded, NULL)
#endif

#define MAX_NUM_TESTS 100

/* caller must clean up the returned description */
static mongoc_server_description_t *
_server_description_by_hostname(mongoc_topology_description_t *topology,
                                const char *address)
{
   mongoc_set_t *set = topology->servers;
   mongoc_server_description_t *server_iter;
   int i;

   for (i = 0; i < set->items_len; i++) {
      server_iter = (mongoc_server_description_t *)mongoc_set_get_item (topology->servers, i);
      if (strcasecmp(address, server_iter->connection_address) == 0) {
         return server_iter;
      }
   }
   return NULL;
}

static void
_topology_has_description(mongoc_topology_description_t *topology,
                          bson_t *server,
                          const char *address)
{
   mongoc_server_description_t *sd;
   bson_iter_t server_iter;
   const char *set_name;

   sd = _server_description_by_hostname(topology, address);
   assert (sd);

   bson_iter_init(&server_iter, server);
   while (bson_iter_next (&server_iter)) {
      if (strcmp("setName", bson_iter_key (&server_iter)) == 0) {
         set_name = bson_iter_utf8(&server_iter, NULL);
         if (set_name) {
            assert (sd->set_name);
            ASSERT_CMPSTR (sd->set_name, set_name);
         }
      } else if (strcmp("type", bson_iter_key (&server_iter)) == 0) {
         assert (sd->type == server_type_from_test(bson_iter_utf8(&server_iter, NULL)));
      } else if (strcmp("setVersion", bson_iter_key (&server_iter)) == 0) {
         int64_t expected_set_version;
         if (BSON_ITER_HOLDS_NULL (&server_iter)) {
            expected_set_version = MONGOC_NO_SET_VERSION;
         } else {
            expected_set_version = bson_iter_as_int64 (&server_iter);
         }
         assert (sd->set_version == expected_set_version);
      } else if (strcmp("electionId", bson_iter_key (&server_iter)) == 0) {
         bson_oid_t expected_oid;
         if (BSON_ITER_HOLDS_NULL (&server_iter)) {
            bson_oid_init_from_string (&expected_oid,
                                       "000000000000000000000000");
         } else {
            ASSERT (BSON_ITER_HOLDS_OID (&server_iter));
            bson_oid_copy (bson_iter_oid (&server_iter), &expected_oid);
         }

         ASSERT_CMPOID (&sd->election_id, &expected_oid);
      } else {
         fprintf (stderr, "ERROR: unparsed field %s\n", bson_iter_key(&server_iter));
         assert (0);
      }
   }
}

/*
 *-----------------------------------------------------------------------
 *
 * Run the JSON tests from the Server Discovery and Monitoring spec.
 *
 *-----------------------------------------------------------------------
 */
static void
test_sdam_cb (bson_t *test)
{
   mongoc_server_description_t *sd;
   mongoc_client_t *client;
   bson_error_t error;
   bson_t ismasters;
   bson_t phase;
   bson_t phases;
   bson_t ismaster;
   bson_t response;
   bson_t servers;
   bson_t server;
   bson_t outcome;
   bson_iter_t phase_iter;
   bson_iter_t phase_field_iter;
   bson_iter_t ismaster_iter;
   bson_iter_t ismaster_field_iter;
   bson_iter_t servers_iter;
   bson_iter_t outcome_iter;
   bson_iter_t iter;
   const char *set_name;
   const char *hostname;

   /* parse out the uri and use it to create a client */
   assert (bson_iter_init_find(&iter, test, "uri"));
   client = mongoc_client_new(bson_iter_utf8(&iter, NULL));

   /* for each phase, parse and validate */
   assert (bson_iter_init_find(&iter, test, "phases"));
   bson_iter_bson (&iter, &phases);
   bson_iter_init (&phase_iter, &phases);

   while (bson_iter_next (&phase_iter)) {
      bson_iter_bson (&phase_iter, &phase);

      /* grab ismaster responses out and feed them to topology */
      assert (bson_iter_init_find(&phase_field_iter, &phase, "responses"));
      bson_iter_bson (&phase_field_iter, &ismasters);
      bson_iter_init (&ismaster_iter, &ismasters);

      while (bson_iter_next (&ismaster_iter)) {
         bson_iter_bson (&ismaster_iter, &ismaster);

         /* fetch server description for this server based on its hostname */
         bson_iter_init_find (&ismaster_field_iter, &ismaster, "0");
         sd = _server_description_by_hostname(&client->topology->description,
                                              bson_iter_utf8(&ismaster_field_iter, NULL));

         /* if server has been removed from topology, skip */
         /* TODO: ASSURE that the manager has the same behavior */
         if (!sd) continue;

         bson_iter_init_find (&ismaster_field_iter, &ismaster, "1");
         bson_iter_bson (&ismaster_field_iter, &response);

         /* send ismaster through the topology description's handler */
         mongoc_topology_description_handle_ismaster(&client->topology->description,
                                                     sd,
                                                     &response,
                                                     15,
                                                     &error);
      }

      /* parse out "outcome" and validate */
      assert (bson_iter_init_find(&phase_field_iter, &phase, "outcome"));
      bson_iter_bson (&phase_field_iter, &outcome);
      bson_iter_init (&outcome_iter, &outcome);

      while (bson_iter_next (&outcome_iter)) {
         if (strcmp ("servers", bson_iter_key (&outcome_iter)) == 0) {
            bson_iter_bson (&outcome_iter, &servers);
            ASSERT_CMPINT (
               bson_count_keys (&servers), ==,
               (int) client->topology->description.servers->items_len);

            bson_iter_init (&servers_iter, &servers);

            /* for each server, ensure topology has a matching entry */
            while (bson_iter_next (&servers_iter)) {
               hostname = bson_iter_key (&servers_iter);
               bson_iter_bson (&servers_iter, &server);

               _topology_has_description(&client->topology->description,
                                         &server,
                                         hostname);
            }

         } else if (strcmp ("setName", bson_iter_key (&outcome_iter)) == 0) {
            set_name = bson_iter_utf8(&outcome_iter, NULL);
            if (set_name) {
               assert (&client->topology->description.set_name);
               ASSERT_CMPSTR (client->topology->description.set_name, set_name);
            }
         } else if (strcmp ("topologyType", bson_iter_key (&outcome_iter)) == 0) {
            ASSERT_CMPSTR (
               topology_type_to_string(client->topology->description.type),
               bson_iter_utf8(&outcome_iter, NULL));
         } else {
            fprintf (stderr, "ERROR: unparsed test field %s\n", bson_iter_key (&outcome_iter));
            assert (false);
         }
      }
   }
   mongoc_client_destroy (client);
}

/*
 *-----------------------------------------------------------------------
 *
 * Runner for the JSON tests for server discovery and monitoring..
 *
 *-----------------------------------------------------------------------
 */
static void
test_all_spec_tests (TestSuite *suite)
{
   char resolved[PATH_MAX];

   /* Single */
   if (realpath ("tests/json/server_discovery_and_monitoring/single", resolved)) {
      install_json_test_suite(suite, resolved, &test_sdam_cb);
   }

   /* Replica set */
   if (realpath ("tests/json/server_discovery_and_monitoring/rs", resolved)) {
      install_json_test_suite(suite, resolved, &test_sdam_cb);
   }

   /* Sharded */
   if (realpath ("tests/json/server_discovery_and_monitoring/sharded", resolved)) {
      install_json_test_suite(suite, resolved, &test_sdam_cb);
   }

   /* Tests not in official Server Discovery And Monitoring Spec */
   if (realpath ("tests/json/server_discovery_and_monitoring/supplemental", resolved)) {
      install_json_test_suite(suite, resolved, &test_sdam_cb);
   }
}

void
test_sdam_install (TestSuite *suite)
{
   test_all_spec_tests(suite);
}
