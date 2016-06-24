#include <mongoc.h>

#include <stdio.h>

#include "json-test.h"

#include "mongoc-server-description-private.h"
#include "mongoc-topology-description-private.h"
#include "mongoc-topology-private.h"

#include "TestSuite.h"
#include "test-conveniences.h"

#ifdef _MSC_VER
#define PATH_MAX 1024
#define realpath(path, expanded) GetFullPathName(path, PATH_MAX, expanded, NULL)
#endif

#include <limits.h>
#include <stdlib.h>

static mongoc_ss_optype_t
optype_from_test(const char *op)
{
   if (strcmp(op, "read") == 0) {
      return MONGOC_SS_READ;
   } else if (strcmp(op, "write") == 0) {
      return MONGOC_SS_WRITE;
   }
   return 0;
}

static mongoc_read_mode_t
read_mode_from_test(const char *mode)
{
   if (strcmp(mode, "Primary") == 0) {
      return MONGOC_READ_PRIMARY;
   } else if (strcmp(mode, "PrimaryPreferred") == 0) {
      return MONGOC_READ_PRIMARY_PREFERRED;
   } else if (strcmp(mode, "Secondary") == 0) {
      return MONGOC_READ_SECONDARY;
   } else if (strcmp(mode, "SecondaryPreferred") == 0) {
      return MONGOC_READ_SECONDARY_PREFERRED;
   } else if (strcmp(mode, "Nearest") == 0) {
      return MONGOC_READ_NEAREST;
   }
   return 0;
}

/*
 *-----------------------------------------------------------------------
 *
 * test_rtt_calculation_cb --
 *
 *       Runs the JSON tests for RTT calculation included with the
 *       Server Selection spec.
 *
 *-----------------------------------------------------------------------
 */

static void
test_rtt_calculation_cb (bson_t *test)
{
   mongoc_server_description_t *description;
   bson_iter_t iter;

   BSON_ASSERT (test);

   description = (mongoc_server_description_t *)bson_malloc0(sizeof *description);
   mongoc_server_description_init(description, "localhost:27017", 1);

   /* parse RTT into server description */
   assert(bson_iter_init_find(&iter, test, "avg_rtt_ms"));
   description->round_trip_time = bson_iter_int64(&iter);

   /* update server description with new rtt */
   assert(bson_iter_init_find(&iter, test, "new_rtt_ms"));
   mongoc_server_description_update_rtt(description, bson_iter_int64(&iter));

   /* ensure new RTT was calculated correctly */
   assert(bson_iter_init_find(&iter, test, "new_avg_rtt"));
   assert(description->round_trip_time == bson_iter_int64(&iter));

   mongoc_server_description_destroy(description);
}

/*
 *-----------------------------------------------------------------------
 *
 * test_server_selection_logic_cb --
 *
 *      Runs the JSON tests for server selection logic that are
 *      included with the Server Selection spec.
 *
 *-----------------------------------------------------------------------
 */
static void
test_server_selection_logic_cb (bson_t *test)
{
   mongoc_topology_description_t topology;
   mongoc_server_description_t *sd;
   mongoc_read_prefs_t *read_prefs;
   mongoc_read_mode_t read_mode;
   mongoc_ss_optype_t op;
   bson_iter_t iter;
   bson_iter_t topology_iter;
   bson_iter_t server_iter;
   bson_iter_t sd_iter;
   bson_iter_t sd_child_iter;
   bson_iter_t read_pref_iter;
   bson_t test_topology;
   bson_t test_servers;
   bson_t server;
   bson_t candidates;
   bson_t eligible;
   bson_t suitable;
   bson_t latency;
   bson_t test_read_pref;
   bson_t test_tags;
   const char *type;
   int j = 0;

   mongoc_array_t selected_servers;

   BSON_ASSERT (test);

   /* pull out topology description field */
   assert(bson_iter_init_find(&iter, test, "topology_description"));
   bson_iter_bson (&iter, &test_topology);

   /* set topology state from test */
   assert(bson_iter_init_find(&topology_iter, &test_topology, "type"));
   type = bson_iter_utf8(&topology_iter, NULL);
   if (strcmp(type, "Single") == 0) {
      mongoc_topology_description_init(&topology, MONGOC_TOPOLOGY_SINGLE);
   } else {
      mongoc_topology_description_init(&topology, MONGOC_TOPOLOGY_UNKNOWN);
      topology.type = topology_type_from_test(bson_iter_utf8(&topology_iter, NULL));
   }

   /* for each server description in test, add server to our topology */
   assert(bson_iter_init_find(&topology_iter, &test_topology, "servers"));
   bson_iter_bson (&topology_iter, &test_servers);

   bson_iter_init(&server_iter, &test_servers);
   while (bson_iter_next (&server_iter)) {
      bson_iter_bson (&server_iter, &server);

      /* initialize new server description with given address */
      sd = (mongoc_server_description_t *)bson_malloc0(sizeof *sd);
      assert(bson_iter_init_find(&sd_iter, &server, "address"));
      mongoc_server_description_init(sd, bson_iter_utf8(&sd_iter, NULL), j++);

      /* set description rtt */
      assert(bson_iter_init_find(&sd_iter, &server, "avg_rtt_ms"));
      sd->round_trip_time = bson_iter_int32(&sd_iter);

      /* set description type */
      assert(bson_iter_init_find(&sd_iter, &server, "type"));
      sd->type = server_type_from_test(bson_iter_utf8(&sd_iter, NULL));

      /* set description tags */
      /* TODO FIX ONCE ARRAYS OF TAG SETS GO AWAY IN TESTS */
      assert(bson_iter_init_find(&sd_iter, &server, "tags"));
      bson_iter_recurse(&sd_iter, &sd_child_iter);
      bson_iter_next(&sd_child_iter);
      bson_iter_bson (&sd_child_iter, &sd->tags);

      /* add new server to our topology description */
      mongoc_set_add(topology.servers, sd->id, sd);
   }

   /* create read preference document from test */
   assert (bson_iter_init_find(&iter, test, "read_preference"));
   bson_iter_bson (&iter, &test_read_pref);

   assert (bson_iter_init_find(&read_pref_iter, &test_read_pref, "mode"));
   read_mode = read_mode_from_test (bson_iter_utf8 (&read_pref_iter, NULL));
   ASSERT (read_mode != 0);
   read_prefs = mongoc_read_prefs_new (read_mode);

   assert (bson_iter_init_find(&read_pref_iter, &test_read_pref, "tags"));
   bson_iter_bson (&read_pref_iter, &test_tags);
   mongoc_read_prefs_set_tags(read_prefs, &test_tags);

   /* get optype */
   assert (bson_iter_init_find(&iter, test, "operation"));
   op = optype_from_test(bson_iter_utf8(&iter, NULL));
   ASSERT(op != 0);

   /* read in candidate servers */
   assert (bson_iter_init_find(&iter, test, "candidate_servers"));
   bson_iter_bson (&iter, &candidates);

   /* read in eligible servers */
   assert (bson_iter_init_find(&iter, test, "eligible_servers"));
   bson_iter_bson (&iter, &eligible);

   /* read in suitable servers */
   assert (bson_iter_init_find(&iter, test, "suitable_servers"));
   bson_iter_bson (&iter, &suitable);

   /* read in latency window servers */
   assert (bson_iter_init_find(&iter, test, "in_latency_window"));
   bson_iter_bson (&iter, &latency);

   _mongoc_array_init (&selected_servers, sizeof(mongoc_server_description_t*));

   mongoc_topology_description_suitable_servers (&selected_servers, op, &topology,
                                                       read_prefs, 15);

   bson_iter_init (&iter, &latency);

   assert (bson_count_keys(&latency) == selected_servers.len);

   while (bson_iter_next(&iter)) {
      bool found = false;
      bson_iter_t host;

      bson_iter_recurse(&iter, &host);
      bson_iter_find (&host, "address");

      for (j = 0; j < selected_servers.len; j++) {
         sd = _mongoc_array_index (&selected_servers, mongoc_server_description_t*, j);

         if (strcmp(sd->host.host_and_port, bson_iter_utf8(&host, NULL)) == 0) {
            found = true;
            break;
         }
      }
      assert (found);
   }

   mongoc_read_prefs_destroy (read_prefs);
   mongoc_topology_description_destroy(&topology);
   _mongoc_array_destroy (&selected_servers);
}

/*
 *-----------------------------------------------------------------------
 *
 * Runner for the JSON tests for server selection.
 *
 *-----------------------------------------------------------------------
 */
static void
test_all_spec_tests (TestSuite *suite)
{
   char resolved[PATH_MAX];

   /* RTT calculation */
   if (realpath ("tests/json/server_selection/rtt", resolved)) {
      install_json_test_suite(suite, resolved, &test_rtt_calculation_cb);
   }

   /* SS logic */
   if (realpath ("tests/json/server_selection/server_selection", resolved)) {
      install_json_test_suite(suite, resolved, &test_server_selection_logic_cb);
   }
}

void
test_server_selection_install (TestSuite *suite)
{
   test_all_spec_tests(suite);
}
