/*
 * Copyright 2014 MongoDB, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "mongoc-host-list.h"
#include "mongoc-host-list-private.h"
#include "mongoc-read-prefs.h"
#include "mongoc-server-description-private.h"
#include "mongoc-trace.h"
#include "mongoc-uri.h"
#include "mongoc-util-private.h"

#include <stdio.h>

#define ALPHA 0.2


static uint8_t kMongocEmptyBson[] = { 5, 0, 0, 0, 0 };

static bson_oid_t kObjectIdZero = { {0} };

/* Destroy allocated resources within @description, but don't free it */
void
mongoc_server_description_cleanup (mongoc_server_description_t *sd)
{
   BSON_ASSERT(sd);

   bson_destroy (&sd->last_is_master);
}

/* Reset fields inside this sd, but keep same id, host information, and RTT,
   and leave ismaster in empty inited state */
void
mongoc_server_description_reset (mongoc_server_description_t *sd)
{
   BSON_ASSERT(sd);

   /* set other fields to default or empty states. election_id is zeroed. */
   memset (&sd->set_name, 0, sizeof (*sd) - ((char*)&sd->set_name - (char*)sd));
   sd->set_name = NULL;
   sd->type = MONGOC_SERVER_UNKNOWN;

   sd->min_wire_version = MONGOC_DEFAULT_WIRE_VERSION;
   sd->max_wire_version = MONGOC_DEFAULT_WIRE_VERSION;
   sd->max_msg_size = MONGOC_DEFAULT_MAX_MSG_SIZE;
   sd->max_bson_obj_size = MONGOC_DEFAULT_BSON_OBJ_SIZE;
   sd->max_write_batch_size = MONGOC_DEFAULT_WRITE_BATCH_SIZE;

   /* always leave last ismaster in an init-ed state until we destroy sd */
   bson_destroy (&sd->last_is_master);
   bson_init (&sd->last_is_master);
   sd->has_is_master = false;
}

/*
 *--------------------------------------------------------------------------
 *
 * mongoc_server_description_init --
 *
 *       Initialize a new server_description_t.
 *
 * Returns:
 *       None.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */
void
mongoc_server_description_init (mongoc_server_description_t *sd,
                                const char                  *address,
                                uint32_t                     id)
{
   ENTRY;

   BSON_ASSERT (sd);
   BSON_ASSERT (address);

   memset (sd, 0, sizeof *sd);

   sd->id = id;
   sd->type = MONGOC_SERVER_UNKNOWN;
   sd->round_trip_time = -1;

   sd->set_name = NULL;
   sd->set_version = MONGOC_NO_SET_VERSION;
   sd->current_primary = NULL;

   if (!_mongoc_host_list_from_string(&sd->host, address)) {
      MONGOC_WARNING("Failed to parse uri for %s", address);
      return;
   }

   sd->connection_address = sd->host.host_and_port;

   sd->me = NULL;
   sd->min_wire_version = MONGOC_DEFAULT_WIRE_VERSION;
   sd->max_wire_version = MONGOC_DEFAULT_WIRE_VERSION;
   sd->max_msg_size = MONGOC_DEFAULT_MAX_MSG_SIZE;
   sd->max_bson_obj_size = MONGOC_DEFAULT_BSON_OBJ_SIZE;
   sd->max_write_batch_size = MONGOC_DEFAULT_WRITE_BATCH_SIZE;

   bson_init_static (&sd->hosts, kMongocEmptyBson, sizeof (kMongocEmptyBson));
   bson_init_static (&sd->passives, kMongocEmptyBson, sizeof (kMongocEmptyBson));
   bson_init_static (&sd->arbiters, kMongocEmptyBson, sizeof (kMongocEmptyBson));
   bson_init_static (&sd->tags, kMongocEmptyBson, sizeof (kMongocEmptyBson));

   bson_init (&sd->last_is_master);

   EXIT;
}

/*
 *--------------------------------------------------------------------------
 *
 * mongoc_server_description_destroy --
 *
 *       Destroy allocated resources within @description and free
 *       @description.
 *
 * Returns:
 *       None.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */
void
mongoc_server_description_destroy (mongoc_server_description_t *description)
{
   ENTRY;

   mongoc_server_description_cleanup(description);

   bson_free(description);

   EXIT;
}

/*
 *--------------------------------------------------------------------------
 *
 * mongoc_server_description_has_rs_member --
 *
 *       Return true if this address is included in server's list of rs
 *       members, false otherwise.
 *
 * Returns:
 *       true, false
 *
 * Side effects:
 *       None
 *
 *--------------------------------------------------------------------------
 */
bool
mongoc_server_description_has_rs_member(mongoc_server_description_t *server,
                                        const char                  *address)
{
   bson_iter_t member_iter;
   const bson_t *rs_members[3];
   int i;

   if (server->type != MONGOC_SERVER_UNKNOWN) {
      rs_members[0] = &server->hosts;
      rs_members[1] = &server->arbiters;
      rs_members[2] = &server->passives;

      for (i = 0; i < 3; i++) {
         bson_iter_init (&member_iter, rs_members[i]);

         while (bson_iter_next (&member_iter)) {
            if (strcasecmp (address, bson_iter_utf8 (&member_iter, NULL)) == 0) {
               return true;
            }
         }
      }
   }

   return false;
}

/*
 *--------------------------------------------------------------------------
 *
 * mongoc_server_description_has_set_version --
 *
 *      Did this server's ismaster response have a "setVersion" field?
 *
 * Returns:
 *      True if the server description's setVersion is set.
 *
 *--------------------------------------------------------------------------
 */

bool
mongoc_server_description_has_set_version (mongoc_server_description_t *description)
{
   return description->set_version != MONGOC_NO_SET_VERSION;
}

/*
 *--------------------------------------------------------------------------
 *
 * mongoc_server_description_has_election_id --
 *
 *      Did this server's ismaster response have an "electionId" field?
 *
 * Returns:
 *      True if the server description's electionId is set.
 *
 *--------------------------------------------------------------------------
 */

bool
mongoc_server_description_has_election_id (mongoc_server_description_t *description)
{
   return 0 != bson_oid_compare (&description->election_id, &kObjectIdZero);
}

/*
 *--------------------------------------------------------------------------
 *
 * mongoc_server_description_id --
 *
 *      Get the id of this server.
 *
 * Returns:
 *      Server's id.
 *
 *--------------------------------------------------------------------------
 */

uint32_t
mongoc_server_description_id (mongoc_server_description_t *description)
{
   return description->id;
}

/*
 *--------------------------------------------------------------------------
 *
 * mongoc_server_description_host --
 *
 *      Return a reference to the host associated with this server description.
 *
 * Returns:
 *      A mongoc_host_list_t *, this server description's host.
 *
 *--------------------------------------------------------------------------
 */

mongoc_host_list_t *
mongoc_server_description_host (mongoc_server_description_t *description)
{
   return &description->host;
}

/*
 *--------------------------------------------------------------------------
 *
 * mongoc_server_description_set_state --
 *
 *       Set the server description's server type.
 *
 *--------------------------------------------------------------------------
 */
void
mongoc_server_description_set_state (mongoc_server_description_t *description,
                                     mongoc_server_description_type_t type)
{
   description->type = type;
}


/*
 *--------------------------------------------------------------------------
 *
 * mongoc_server_description_set_set_version --
 *
 *       Set the replica set version of this server.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */
void
mongoc_server_description_set_set_version (mongoc_server_description_t *description,
                                           int64_t set_version)
{
   description->set_version = set_version;
}

/*
 *--------------------------------------------------------------------------
 *
 * mongoc_server_description_set_election_id --
 *
 *       Set the election_id of this server. Copies the given ObjectId or,
 *       if it is NULL, zeroes description's election_id.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */
void
mongoc_server_description_set_election_id (mongoc_server_description_t *description,
                                           const bson_oid_t *election_id)
{
   if (election_id) {
      bson_oid_copy_unsafe (election_id, &description->election_id);
   } else {
      bson_oid_copy_unsafe (&kObjectIdZero, &description->election_id);
   }
}


/*
 *-------------------------------------------------------------------------
 *
 * mongoc_server_description_update_rtt --
 *
 *       Calculate this server's rtt calculation using an exponentially-
 *       weighted moving average formula.
 *
 * Side effects:
 *       None.
 *
 *-------------------------------------------------------------------------
 */
void
mongoc_server_description_update_rtt (mongoc_server_description_t *server,
                                      int64_t                      new_time)
{
   if (server->round_trip_time == -1) {
      server->round_trip_time = new_time;
   } else {
      server->round_trip_time = ALPHA * new_time + (1 - ALPHA) * server->round_trip_time;
   }
}

/*
 *-------------------------------------------------------------------------
 *
 * Called during SDAM, from topology description's ismaster handler.
 *
 *-------------------------------------------------------------------------
 */

void
mongoc_server_description_handle_ismaster (
   mongoc_server_description_t   *sd,
   const bson_t                  *ismaster_response,
   int64_t                        rtt_msec,
   bson_error_t                  *error)
{
   bson_iter_t iter;
   bool is_master = false;
   bool is_shard = false;
   bool is_secondary = false;
   bool is_arbiter = false;
   bool is_replicaset = false;
   bool is_hidden = false;
   const uint8_t *bytes;
   uint32_t len;
   int num_keys = 0;
   ENTRY;

   BSON_ASSERT (sd);

   mongoc_server_description_reset (sd);
   if (!ismaster_response) {
      EXIT;
   }

   bson_destroy (&sd->last_is_master);
   bson_copy_to (ismaster_response, &sd->last_is_master);
   sd->has_is_master = true;

   bson_iter_init (&iter, &sd->last_is_master);

   while (bson_iter_next (&iter)) {
      num_keys++;
      if (strcmp ("ok", bson_iter_key (&iter)) == 0) {
         /* ismaster responses never have ok: 0, but spec requires we check */
         if (! bson_iter_as_bool (&iter)) goto failure;
      } else if (strcmp ("ismaster", bson_iter_key (&iter)) == 0) {
         if (! BSON_ITER_HOLDS_BOOL (&iter)) goto failure;
         is_master = bson_iter_bool (&iter);
      } else if (strcmp ("me", bson_iter_key (&iter)) == 0) {
         if (! BSON_ITER_HOLDS_UTF8 (&iter)) goto failure;
         sd->me = bson_iter_utf8 (&iter, NULL);
      } else if (strcmp ("maxMessageSizeBytes", bson_iter_key (&iter)) == 0) {
         if (! BSON_ITER_HOLDS_INT32 (&iter)) goto failure;
         sd->max_msg_size = bson_iter_int32 (&iter);
      } else if (strcmp ("maxBsonObjectSize", bson_iter_key (&iter)) == 0) {
         if (! BSON_ITER_HOLDS_INT32 (&iter)) goto failure;
         sd->max_bson_obj_size = bson_iter_int32 (&iter);
      } else if (strcmp ("maxWriteBatchSize", bson_iter_key (&iter)) == 0) {
         if (! BSON_ITER_HOLDS_INT32 (&iter)) goto failure;
         sd->max_write_batch_size = bson_iter_int32 (&iter);
      } else if (strcmp ("minWireVersion", bson_iter_key (&iter)) == 0) {
         if (! BSON_ITER_HOLDS_INT32 (&iter)) goto failure;
         sd->min_wire_version = bson_iter_int32 (&iter);
      } else if (strcmp ("maxWireVersion", bson_iter_key (&iter)) == 0) {
         if (! BSON_ITER_HOLDS_INT32 (&iter)) goto failure;
         sd->max_wire_version = bson_iter_int32 (&iter);
      } else if (strcmp ("msg", bson_iter_key (&iter)) == 0) {
         if (! BSON_ITER_HOLDS_UTF8 (&iter)) goto failure;
         is_shard = !!bson_iter_utf8 (&iter, NULL);
      } else if (strcmp ("setName", bson_iter_key (&iter)) == 0) {
         if (! BSON_ITER_HOLDS_UTF8 (&iter)) goto failure;
         sd->set_name = bson_iter_utf8 (&iter, NULL);
      } else if (strcmp ("setVersion", bson_iter_key (&iter)) == 0) {
         mongoc_server_description_set_set_version (sd,
                                                    bson_iter_as_int64 (&iter));
      } else if (strcmp ("electionId", bson_iter_key (&iter)) == 0) {
         if (! BSON_ITER_HOLDS_OID (&iter)) goto failure;
         mongoc_server_description_set_election_id (sd, bson_iter_oid (&iter));
      } else if (strcmp ("secondary", bson_iter_key (&iter)) == 0) {
         if (! BSON_ITER_HOLDS_BOOL (&iter)) goto failure;
         is_secondary = bson_iter_bool (&iter);
      } else if (strcmp ("hosts", bson_iter_key (&iter)) == 0) {
         if (! BSON_ITER_HOLDS_ARRAY (&iter)) goto failure;
         bson_iter_array (&iter, &len, &bytes);
         bson_init_static (&sd->hosts, bytes, len);
      } else if (strcmp ("passives", bson_iter_key (&iter)) == 0) {
         if (! BSON_ITER_HOLDS_ARRAY (&iter)) goto failure;
         bson_iter_array (&iter, &len, &bytes);
         bson_init_static (&sd->passives, bytes, len);
      } else if (strcmp ("arbiters", bson_iter_key (&iter)) == 0) {
         if (! BSON_ITER_HOLDS_ARRAY (&iter)) goto failure;
         bson_iter_array (&iter, &len, &bytes);
         bson_init_static (&sd->arbiters, bytes, len);
      } else if (strcmp ("primary", bson_iter_key (&iter)) == 0) {
         if (! BSON_ITER_HOLDS_UTF8 (&iter)) goto failure;
         sd->current_primary = bson_iter_utf8 (&iter, NULL);
      } else if (strcmp ("arbiterOnly", bson_iter_key (&iter)) == 0) {
         if (! BSON_ITER_HOLDS_BOOL (&iter)) goto failure;
         is_arbiter = bson_iter_bool (&iter);
      } else if (strcmp ("isreplicaset", bson_iter_key (&iter)) == 0) {
         if (! BSON_ITER_HOLDS_BOOL (&iter)) goto failure;
         is_replicaset = bson_iter_bool (&iter);
      } else if (strcmp ("tags", bson_iter_key (&iter)) == 0) {
         if (! BSON_ITER_HOLDS_DOCUMENT (&iter)) goto failure;
         bson_iter_document (&iter, &len, &bytes);
         bson_init_static (&sd->tags, bytes, len);
      } else if (strcmp ("hidden", bson_iter_key (&iter)) == 0) {
         is_hidden = bson_iter_bool (&iter);
      }
   }

   if (is_shard) {
      sd->type = MONGOC_SERVER_MONGOS;
   } else if (sd->set_name) {
      if (is_hidden) {
         sd->type = MONGOC_SERVER_RS_OTHER;
      } else if (is_master) {
         sd->type = MONGOC_SERVER_RS_PRIMARY;
      } else if (is_secondary) {
         sd->type = MONGOC_SERVER_RS_SECONDARY;
      } else if (is_arbiter) {
         sd->type = MONGOC_SERVER_RS_ARBITER;
      } else {
         sd->type = MONGOC_SERVER_RS_OTHER;
      }
   } else if (is_replicaset) {
      sd->type = MONGOC_SERVER_RS_GHOST;
   } else if (num_keys > 0) {
      sd->type = MONGOC_SERVER_STANDALONE;
   } else {
      sd->type = MONGOC_SERVER_UNKNOWN;
   }

   mongoc_server_description_update_rtt(sd, rtt_msec);

   EXIT;

failure:
   sd->type = MONGOC_SERVER_UNKNOWN;
   sd->round_trip_time = -1;

   EXIT;
}

/*
 *-------------------------------------------------------------------------
 *
 * mongoc_server_description_new_copy --
 *
 *       A copy of a server description that you must destroy, or NULL.
 *
 *-------------------------------------------------------------------------
 */
mongoc_server_description_t *
mongoc_server_description_new_copy (const mongoc_server_description_t *description)
{
   mongoc_server_description_t *copy;

   if (!description) {
      return NULL;
   }

   copy = (mongoc_server_description_t *)bson_malloc0(sizeof (*copy));

   copy->id = description->id;
   memcpy (&copy->host, &description->host, sizeof (copy->host));
   copy->round_trip_time = -1;

   copy->connection_address = copy->host.host_and_port;

   /* wait for handle_ismaster to fill these in properly */
   copy->has_is_master = false;
   copy->set_version = MONGOC_NO_SET_VERSION;
   bson_init_static (&copy->hosts, kMongocEmptyBson, sizeof (kMongocEmptyBson));
   bson_init_static (&copy->passives, kMongocEmptyBson, sizeof (kMongocEmptyBson));
   bson_init_static (&copy->arbiters, kMongocEmptyBson, sizeof (kMongocEmptyBson));
   bson_init_static (&copy->tags, kMongocEmptyBson, sizeof (kMongocEmptyBson));

   bson_init (&copy->last_is_master);

   if (description->has_is_master) {
      mongoc_server_description_handle_ismaster (copy, &description->last_is_master,
                                                 description->round_trip_time, NULL);
   }
   /* Preserve the error */
   memcpy (&copy->error, &description->error, sizeof copy->error);
   return copy;
}

/*
 *-------------------------------------------------------------------------
 *
 * mongoc_server_description_filter_eligible --
 *
 *       Given a set of server descriptions, determine which are eligible
 *       as per the Server Selection spec. Determines the number of
 *       eligible servers, and sets any servers that are NOT eligible to
 *       NULL in the descriptions set.
 *
 * Returns:
 *       Number of eligible servers found.
 *
 *-------------------------------------------------------------------------
 */

size_t
mongoc_server_description_filter_eligible (
   mongoc_server_description_t **descriptions,
   size_t                        description_len,
   const mongoc_read_prefs_t    *read_prefs)
{
   const bson_t *rp_tags;
   bson_iter_t rp_tagset_iter;
   bson_iter_t rp_iter;
   bson_iter_t sd_iter;
   uint32_t rp_len;
   uint32_t sd_len;
   const char *rp_val;
   const char *sd_val;
   bool *sd_matched = NULL;
   size_t found;
   size_t i;
   size_t rval = 0;

   if (!read_prefs) {
      /* NULL read_prefs is PRIMARY, no tags to filter by */
      return description_len;
   }

   rp_tags = mongoc_read_prefs_get_tags (read_prefs);

   if (bson_count_keys (rp_tags) == 0) {
      return description_len;
   }

   sd_matched = (bool *) bson_malloc0 (sizeof(bool) * description_len);

   bson_iter_init (&rp_tagset_iter, rp_tags);

   /* for each read preference tagset */
   while (bson_iter_next (&rp_tagset_iter)) {
      found = description_len;

      for (i = 0; i < description_len; i++) {
         sd_matched[i] = true;

         bson_iter_recurse (&rp_tagset_iter, &rp_iter);

         while (bson_iter_next (&rp_iter)) {
            /* TODO: can we have non-utf8 tags? */
            rp_val = bson_iter_utf8 (&rp_iter, &rp_len);

            if (bson_iter_init_find (&sd_iter, &descriptions[i]->tags, bson_iter_key (&rp_iter))) {

               /* If the server description has that key */
               sd_val = bson_iter_utf8 (&sd_iter, &sd_len);

               if (! (sd_len == rp_len && (0 == memcmp(rp_val, sd_val, rp_len)))) {
                  /* If the key value doesn't match, no match */
                  sd_matched[i] = false;
                  found--;
               }
            } else {
               /* If the server description doesn't have that key, no match */
               sd_matched[i] = false;
               found--;
               break;
            }
         }
      }

      if (found) {
         for (i = 0; i < description_len; i++) {
            if (! sd_matched[i]) {
               descriptions[i] = NULL;
            }
         }

         rval = found;
         goto CLEANUP;
      }
   }

   for (i = 0; i < description_len; i++) {
      if (! sd_matched[i]) {
         descriptions[i] = NULL;
      }
   }

CLEANUP:
   bson_free (sd_matched);

   return rval;
}
