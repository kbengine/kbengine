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

#include "mongoc-array-private.h"
#include "mongoc-error.h"
#include "mongoc-server-description-private.h"
#include "mongoc-topology-description-private.h"
#include "mongoc-trace.h"
#include "mongoc-util-private.h"


static void
_mongoc_topology_server_dtor (void *server_,
                              void *ctx_)
{
   mongoc_server_description_destroy ((mongoc_server_description_t *)server_);
}

/*
 *--------------------------------------------------------------------------
 *
 * mongoc_topology_description_init --
 *
 *       Initialize the given topology description
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
mongoc_topology_description_init (mongoc_topology_description_t     *description,
                                  mongoc_topology_description_type_t type)
{
   ENTRY;

   BSON_ASSERT (description);
   BSON_ASSERT (type == MONGOC_TOPOLOGY_UNKNOWN ||
                        type == MONGOC_TOPOLOGY_SINGLE ||
                        type == MONGOC_TOPOLOGY_RS_NO_PRIMARY);

   memset (description, 0, sizeof (*description));

   description->type = type;
   description->servers = mongoc_set_new(8, _mongoc_topology_server_dtor, NULL);
   description->set_name = NULL;
   description->max_set_version = MONGOC_NO_SET_VERSION;
   description->compatible = true;
   description->compatibility_error = NULL;
   description->stale = true;

   EXIT;
}

/*
 *--------------------------------------------------------------------------
 *
 * mongoc_topology_description_destroy --
 *
 *       Destroy allocated resources within @description
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
mongoc_topology_description_destroy (mongoc_topology_description_t *description)
{
   ENTRY;

   BSON_ASSERT(description);

   mongoc_set_destroy(description->servers);

   if (description->set_name) {
      bson_free (description->set_name);
   }

   if (description->compatibility_error) {
      bson_free (description->compatibility_error);
   }

   EXIT;
}

/* find the primary, then stop iterating */
static bool
_mongoc_topology_description_has_primary_cb (void *item,
                                             void *ctx /* OUT */)
{
   mongoc_server_description_t *server = (mongoc_server_description_t *)item;
   mongoc_server_description_t **primary = (mongoc_server_description_t **)ctx;

   /* TODO should this include MONGOS? */
   if (server->type == MONGOC_SERVER_RS_PRIMARY || server->type == MONGOC_SERVER_STANDALONE) {
      *primary = (mongoc_server_description_t *)item;
      return false;
   }
   return true;
}

/*
 *--------------------------------------------------------------------------
 *
 * _mongoc_topology_description_has_primary --
 *
 *       If topology has a primary, return it.
 *
 * Returns:
 *       A pointer to the primary, or NULL.
 *
 * Side effects:
 *       None
 *
 *--------------------------------------------------------------------------
 */
static mongoc_server_description_t *
_mongoc_topology_description_has_primary (mongoc_topology_description_t *description)
{
   mongoc_server_description_t *primary = NULL;

   mongoc_set_for_each(description->servers, _mongoc_topology_description_has_primary_cb, &primary);

   return primary;
}

/*
 *--------------------------------------------------------------------------
 *
 * _mongoc_topology_description_later_election --
 *
 *       Check if we've seen a more recent election in the replica set
 *       than this server has.
 *
 * Returns:
 *       True if the topology description's max replica set version plus
 *       election id is later than the server description's.
 *
 * Side effects:
 *       None
 *
 *--------------------------------------------------------------------------
 */
static bool
_mongoc_topology_description_later_election (mongoc_topology_description_t *td,
                                             mongoc_server_description_t   *sd)
{
   /* initially max_set_version is -1 and max_election_id is zeroed */
   return td->max_set_version > sd->set_version ||
      (td->max_set_version == sd->set_version &&
         bson_oid_compare (&td->max_election_id, &sd->election_id) > 0);
}

/*
 *--------------------------------------------------------------------------
 *
 * _mongoc_topology_description_set_max_set_version --
 *
 *       Remember that we've seen a new replica set version. Unconditionally
 *       sets td->set_version to sd->set_version.
 *
 *--------------------------------------------------------------------------
 */
static void
_mongoc_topology_description_set_max_set_version (
   mongoc_topology_description_t *td,
   mongoc_server_description_t   *sd)
{
   td->max_set_version = sd->set_version;
}

/*
 *--------------------------------------------------------------------------
 *
 * _mongoc_topology_description_set_max_election_id --
 *
 *       Remember that we've seen a new election id. Unconditionally sets
 *       td->max_election_id to sd->election_id.
 *
 *--------------------------------------------------------------------------
 */
static void
_mongoc_topology_description_set_max_election_id (
   mongoc_topology_description_t *td,
   mongoc_server_description_t   *sd)
{
   bson_oid_copy (&sd->election_id, &td->max_election_id);
}

static bool
_mongoc_topology_description_server_is_candidate (
   mongoc_server_description_type_t   desc_type,
   mongoc_read_mode_t                 read_mode,
   mongoc_topology_description_type_t topology_type)
{
   switch ((int)topology_type) {
   case MONGOC_TOPOLOGY_SINGLE:
      switch ((int)desc_type) {
      case MONGOC_SERVER_STANDALONE:
         return true;
      default:
         return false;
      }

   case MONGOC_TOPOLOGY_RS_NO_PRIMARY:
   case MONGOC_TOPOLOGY_RS_WITH_PRIMARY:
      switch ((int)read_mode) {
      case MONGOC_READ_PRIMARY:
         switch ((int)desc_type) {
         case MONGOC_SERVER_POSSIBLE_PRIMARY:
         case MONGOC_SERVER_RS_PRIMARY:
            return true;
         default:
            return false;
         }
      case MONGOC_READ_SECONDARY:
         switch ((int)desc_type) {
         case MONGOC_SERVER_RS_SECONDARY:
            return true;
         default:
            return false;
         }
      default:
         switch ((int)desc_type) {
         case MONGOC_SERVER_POSSIBLE_PRIMARY:
         case MONGOC_SERVER_RS_PRIMARY:
         case MONGOC_SERVER_RS_SECONDARY:
            return true;
         default:
            return false;
         }
      }

   case MONGOC_TOPOLOGY_SHARDED:
      switch ((int)desc_type) {
      case MONGOC_SERVER_MONGOS:
         return true;
      default:
         return false;
      }
   default:
      return false;
   }
}

typedef struct _mongoc_suitable_data_t
{
   mongoc_read_mode_t                 read_mode;
   mongoc_topology_description_type_t topology_type;
   mongoc_server_description_t       *primary; /* OUT */
   mongoc_server_description_t      **candidates; /* OUT */
   size_t                             candidates_len; /* OUT */
   bool                               has_secondary; /* OUT */
} mongoc_suitable_data_t;

static bool
_mongoc_replica_set_read_suitable_cb (void *item,
                                      void *ctx)
{
   mongoc_server_description_t *server = (mongoc_server_description_t *)item;
   mongoc_suitable_data_t *data = (mongoc_suitable_data_t *)ctx;

   if (_mongoc_topology_description_server_is_candidate (server->type,
                                                         data->read_mode,
                                                         data->topology_type)) {

      if (server->type == MONGOC_SERVER_RS_PRIMARY) {
         data->primary = server;

         if (data->read_mode == MONGOC_READ_PRIMARY ||
             data->read_mode == MONGOC_READ_PRIMARY_PREFERRED) {
            /* we want a primary and we have one, done! */
            return false;
         }
      }

      if (server->type == MONGOC_SERVER_RS_SECONDARY) {
         data->has_secondary = true;
      }

      /* add to our candidates */
      data->candidates[data->candidates_len++] = server;
   }
   return true;
}

/* if any mongos are candidates, add them to the candidates array */
static bool
_mongoc_find_suitable_mongos_cb (void *item,
                                 void *ctx)
{
   mongoc_server_description_t *server = (mongoc_server_description_t *)item;
   mongoc_suitable_data_t *data = (mongoc_suitable_data_t *)ctx;

   if (_mongoc_topology_description_server_is_candidate (server->type,
                                                         data->read_mode,
                                                         data->topology_type)) {
      data->candidates[data->candidates_len++] = server;
   }
   return true;
}

/*
 *-------------------------------------------------------------------------
 *
 * mongoc_topology_description_suitable_servers --
 *
 *       Return an array of suitable server descriptions for this
 *       operation and read preference.
 *
 *       NOTE: this method should only be called while holding the mutex on
 *       the owning topology object.
 *
 * Returns:
 *       Array of server descriptions, or NULL upon failure.
 *
 * Side effects:
 *       None.
 *
 *-------------------------------------------------------------------------
 */

void
mongoc_topology_description_suitable_servers (
   mongoc_array_t                *set, /* OUT */
   mongoc_ss_optype_t             optype,
   mongoc_topology_description_t *topology,
   const mongoc_read_prefs_t     *read_pref,
   size_t                         local_threshold_ms)
{
   mongoc_suitable_data_t data;
   mongoc_server_description_t **candidates;
   mongoc_server_description_t *server;
   int64_t nearest = -1;
   int i;
   mongoc_read_mode_t read_mode = mongoc_read_prefs_get_mode(read_pref);

   candidates = (mongoc_server_description_t **)bson_malloc0(sizeof(*candidates) * topology->servers->items_len);

   data.read_mode = read_mode;
   data.topology_type = topology->type;
   data.primary = NULL;
   data.candidates = candidates;
   data.candidates_len = 0;
   data.has_secondary = false;

   /* Single server --
    * Either it is suitable or it isn't */
   if (topology->type == MONGOC_TOPOLOGY_SINGLE) {
      server = (mongoc_server_description_t *)mongoc_set_get_item (topology->servers, 0);
      if (_mongoc_topology_description_server_is_candidate (server->type, read_mode, topology->type)) {
         _mongoc_array_append_val (set, server);
      }
      goto DONE;
   }

   /* Replica sets --
    * Find suitable servers based on read mode */
   if (topology->type == MONGOC_TOPOLOGY_RS_NO_PRIMARY ||
       topology->type == MONGOC_TOPOLOGY_RS_WITH_PRIMARY) {

      if (optype == MONGOC_SS_READ) {

         mongoc_set_for_each(topology->servers, _mongoc_replica_set_read_suitable_cb, &data);

         /* if we have a primary, it's a candidate, for some read modes we are done */
         if (read_mode == MONGOC_READ_PRIMARY || read_mode == MONGOC_READ_PRIMARY_PREFERRED) {
            if (data.primary) {
               _mongoc_array_append_val (set, data.primary);
               goto DONE;
            }
         }

         if (! mongoc_server_description_filter_eligible (data.candidates, data.candidates_len, read_pref)) {
            if (read_mode == MONGOC_READ_NEAREST) {
               goto DONE;
            } else {
               data.has_secondary = false;
            }
         }

         if (data.has_secondary &&
             (read_mode == MONGOC_READ_SECONDARY || read_mode == MONGOC_READ_SECONDARY_PREFERRED)) {
            /* secondary or secondary preferred and we have one. */

            for (i = 0; i < data.candidates_len; i++) {
               if (candidates[i] && candidates[i]->type == MONGOC_SERVER_RS_PRIMARY) {
                  candidates[i] = NULL;
               }
            }
         } else if (read_mode == MONGOC_READ_SECONDARY_PREFERRED && data.primary) {
            /* secondary preferred, but only the one primary is a candidate */
            _mongoc_array_append_val (set, data.primary);
            goto DONE;
         }

      } else if (topology->type == MONGOC_TOPOLOGY_RS_WITH_PRIMARY) {
         /* includes optype == MONGOC_SS_WRITE as the exclusion of the above if */
         mongoc_set_for_each(topology->servers, _mongoc_topology_description_has_primary_cb,
                             &data.primary);
         if (data.primary) {
            _mongoc_array_append_val (set, data.primary);
            goto DONE;
         }
      }
   }

   /* Sharded clusters --
    * All candidates in the latency window are suitable */
   if (topology->type == MONGOC_TOPOLOGY_SHARDED) {
      mongoc_set_for_each (topology->servers, _mongoc_find_suitable_mongos_cb, &data);
   }

   /* Ways to get here:
    *   - secondary read
    *   - secondary preferred read
    *   - primary_preferred and no primary read
    *   - sharded anything
    * Find the nearest, then select within the window */

   for (i = 0; i < data.candidates_len; i++) {
      if (candidates[i] && (nearest == -1 || nearest > candidates[i]->round_trip_time)) {
         nearest = candidates[i]->round_trip_time;
      }
   }

   for (i = 0; i < data.candidates_len; i++) {
      if (candidates[i] && (candidates[i]->round_trip_time <= nearest + local_threshold_ms)) {
         _mongoc_array_append_val (set, candidates[i]);
      }
   }

DONE:

   bson_free (candidates);

   return;
}


/*
 *-------------------------------------------------------------------------
 *
 * mongoc_topology_description_select --
 *
 *      Return a server description of a node that is appropriate for
 *      the given read preference and operation type.
 *
 *      NOTE: this method simply attempts to select a server from the
 *      current topology, it does not retry or trigger topology checks.
 *
 *      NOTE: this method should only be called while holding the mutex on
 *      the owning topology object.
 *
 * Returns:
 *      Selected server description, or NULL upon failure.
 *
 * Side effects:
 *      None.
 *
 *-------------------------------------------------------------------------
 */

mongoc_server_description_t *
mongoc_topology_description_select (mongoc_topology_description_t *topology,
                                    mongoc_ss_optype_t             optype,
                                    const mongoc_read_prefs_t     *read_pref,
                                    int64_t                        local_threshold_ms)
{
   mongoc_array_t suitable_servers;
   mongoc_server_description_t *sd = NULL;

   ENTRY;

   if (!topology->compatible) {
      /* TODO, should we return an error object here,
         or just treat as a case where there are no suitable servers? */
      RETURN(NULL);
   }

   if (topology->type == MONGOC_TOPOLOGY_SINGLE) {
      sd = (mongoc_server_description_t *)mongoc_set_get_item (topology->servers, 0);

      if (sd->has_is_master) {
         RETURN(sd);
      } else {
         RETURN(NULL);
      }
   }

   _mongoc_array_init(&suitable_servers, sizeof(mongoc_server_description_t *));

   mongoc_topology_description_suitable_servers(&suitable_servers, optype,
                                                 topology, read_pref, local_threshold_ms);
   if (suitable_servers.len != 0) {
      sd = _mongoc_array_index(&suitable_servers, mongoc_server_description_t*,
                               rand() % suitable_servers.len);
   }

   _mongoc_array_destroy (&suitable_servers);

   RETURN(sd);
}

/*
 *--------------------------------------------------------------------------
 *
 * mongoc_topology_description_server_by_id --
 *
 *       Get the server description for @id, if that server is present
 *       in @description. Otherwise, return NULL and fill out optional
 *       @error.
 *
 *       NOTE: In most cases, caller should create a duplicate of the
 *       returned server description. Caller should hold the mutex on the
 *       owning topology object while calling this method and while using
 *       the returned reference.
 *
 * Returns:
 *       A mongoc_server_description_t *, or NULL.
 *
 * Side effects:
 *      Fills out optional @error if server not found.
 *
 *--------------------------------------------------------------------------
 */

mongoc_server_description_t *
mongoc_topology_description_server_by_id (mongoc_topology_description_t *description,
                                          uint32_t id,
                                          bson_error_t *error)
{
   mongoc_server_description_t *sd;

   BSON_ASSERT (description);

   sd = (mongoc_server_description_t *)mongoc_set_get(description->servers, id);
   if (!sd) {
      bson_set_error (error,
                      MONGOC_ERROR_STREAM,
                      MONGOC_ERROR_STREAM_NOT_ESTABLISHED,
                      "Could not find description for node %u", id);
   }

   return sd;
}

/*
 *--------------------------------------------------------------------------
 *
 * _mongoc_topology_description_remove_server --
 *
 *       If present, remove this server from this topology description.
 *
 * Returns:
 *       None.
 *
 * Side effects:
 *       Removes the server description from topology and destroys it.
 *
 *--------------------------------------------------------------------------
 */
static void
_mongoc_topology_description_remove_server (mongoc_topology_description_t *description,
                                            mongoc_server_description_t   *server)
{
   BSON_ASSERT (description);
   BSON_ASSERT (server);

   mongoc_set_rm(description->servers, server->id);
}

typedef struct _mongoc_address_and_id_t {
   const char *address; /* IN */
   bool found; /* OUT */
   uint32_t id; /* OUT */
} mongoc_address_and_id_t;

/* find the given server and stop iterating */
static bool
_mongoc_topology_description_has_server_cb (void *item,
                                            void *ctx /* IN - OUT */)
{
   mongoc_server_description_t *server = (mongoc_server_description_t *)item;
   mongoc_address_and_id_t *data = (mongoc_address_and_id_t *)ctx;

   if (strcasecmp (data->address, server->connection_address) == 0) {
      data->found = true;
      data->id = server->id;
      return false;
   }
   return true;
}

/*
 *--------------------------------------------------------------------------
 *
 * _mongoc_topology_description_has_set_version --
 *
 *       Whether @topology's max replica set version has been set.
 *
 * Returns:
 *       True if the max setVersion was ever set.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */
static bool
_mongoc_topology_description_has_set_version (mongoc_topology_description_t *td)
{
   return td->max_set_version != MONGOC_NO_SET_VERSION;
}

/*
 *--------------------------------------------------------------------------
 *
 * _mongoc_topology_description_topology_has_server --
 *
 *       Return true if @server is in @topology. If so, place its id in
 *       @id if given.
 *
 * Returns:
 *       True if server is in topology, false otherwise.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */
static bool
_mongoc_topology_description_has_server (mongoc_topology_description_t *description,
                                         const char                    *address,
                                         uint32_t                      *id /* OUT */)
{
   mongoc_address_and_id_t data;

   BSON_ASSERT (description);
   BSON_ASSERT (address);

   data.address = address;
   data.found = false;
   mongoc_set_for_each (description->servers, _mongoc_topology_description_has_server_cb, &data);

   if (data.found && id) {
      *id = data.id;
   }

   return data.found;
}

typedef struct _mongoc_address_and_type_t
{
   const char *address;
   mongoc_server_description_type_t type;
} mongoc_address_and_type_t;

static bool
_mongoc_label_unknown_member_cb (void *item,
                                 void *ctx)
{
   mongoc_server_description_t *server = (mongoc_server_description_t *)item;
   mongoc_address_and_type_t *data = (mongoc_address_and_type_t *)ctx;

   if (strcasecmp (server->connection_address, data->address) == 0 &&
       server->type == MONGOC_SERVER_UNKNOWN) {
      mongoc_server_description_set_state(server, data->type);
      return false;
   }
   return true;
}

/*
 *--------------------------------------------------------------------------
 *
 * _mongoc_topology_description_label_unknown_member --
 *
 *       Find the server description with the given @address and if its
 *       type is UNKNOWN, set its type to @type.
 *
 * Returns:
 *       None.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */
static void
_mongoc_topology_description_label_unknown_member (mongoc_topology_description_t *description,
                                                   const char *address,
                                                   mongoc_server_description_type_t type)
{
   mongoc_address_and_type_t data;

   BSON_ASSERT (description);
   BSON_ASSERT (address);

   data.type = type;
   data.address = address;

   mongoc_set_for_each(description->servers, _mongoc_label_unknown_member_cb, &data);
}

/*
 *--------------------------------------------------------------------------
 *
 * _mongoc_topology_description_set_state --
 *
 *       Change the state of this cluster and unblock things waiting
 *       on a change of topology type.
 *
 * Returns:
 *       None.
 *
 * Side effects:
 *       Unblocks anything waiting on this description to change states.
 *
 *--------------------------------------------------------------------------
 */
static void
_mongoc_topology_description_set_state (mongoc_topology_description_t *description,
                                        mongoc_topology_description_type_t type)
{
   description->type = type;
}


static void
_update_rs_type (mongoc_topology_description_t *topology)
{
   if (_mongoc_topology_description_has_primary(topology)) {
      _mongoc_topology_description_set_state(topology, MONGOC_TOPOLOGY_RS_WITH_PRIMARY);
   }
   else {
      _mongoc_topology_description_set_state(topology, MONGOC_TOPOLOGY_RS_NO_PRIMARY);
   }
}


/*
 *--------------------------------------------------------------------------
 *
 * _mongoc_topology_description_check_if_has_primary --
 *
 *       If there is a primary in topology, set topology
 *       type to RS_WITH_PRIMARY, otherwise set it to
 *       RS_NO_PRIMARY.
 *
 * Returns:
 *       None.
 *
 * Side effects:
 *       Changes the topology type.
 *
 *--------------------------------------------------------------------------
 */
static void
_mongoc_topology_description_check_if_has_primary (mongoc_topology_description_t *topology,
                                                   mongoc_server_description_t   *server)
{
   _update_rs_type (topology);
}

/*
 *--------------------------------------------------------------------------
 *
 * mongoc_topology_description_invalidate_server --
 *
 *      Invalidate a server if a network error occurred while using it in
 *      another part of the client. Server description is set to type
 *      UNKNOWN and other parameters are reset to defaults.
 *
 *      NOTE: this method should only be called while holding the mutex on
 *      the owning topology object.
 *
 *--------------------------------------------------------------------------
 */
void
mongoc_topology_description_invalidate_server (mongoc_topology_description_t *topology,
                                               uint32_t                       id)
{
   mongoc_server_description_t *sd;
   bson_error_t error;

   sd = mongoc_topology_description_server_by_id (topology, id, NULL);
   mongoc_topology_description_handle_ismaster (topology, sd, NULL, 0, &error);

   return;
}

/*
 *--------------------------------------------------------------------------
 *
 * mongoc_topology_description_add_server --
 *
 *       Add the specified server to the cluster topology if it is not
 *       already a member. If @id, place its id in @id.
 *
 *       NOTE: this method should only be called while holding the mutex on
 *       the owning topology object.
 *
 * Return:
 *       True if the server was added or already existed in the topology,
 *       false if an error occurred.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */
bool
mongoc_topology_description_add_server (mongoc_topology_description_t *topology,
                                        const char                    *server,
                                        uint32_t                      *id /* OUT */)
{
   uint32_t server_id;
   mongoc_server_description_t *description;

   BSON_ASSERT (topology);
   BSON_ASSERT (server);

   if (!_mongoc_topology_description_has_server(topology, server, &server_id)){

      /* TODO this might not be an accurate count in all cases */
      server_id = ++topology->max_server_id;

      description = (mongoc_server_description_t *)bson_malloc0(sizeof *description);
      mongoc_server_description_init(description, server, server_id);

      mongoc_set_add(topology->servers, server_id, description);
   }

   if (id) {
      *id = server_id;
   }

   return true;
}


static void
_mongoc_topology_description_add_new_servers (
   mongoc_topology_description_t *topology,
   mongoc_server_description_t *server)
{
   bson_iter_t member_iter;
   const bson_t *rs_members[3];
   int i;

   rs_members[0] = &server->hosts;
   rs_members[1] = &server->arbiters;
   rs_members[2] = &server->passives;

   for (i = 0; i < 3; i++) {
      bson_iter_init (&member_iter, rs_members[i]);

      while (bson_iter_next (&member_iter)) {
         mongoc_topology_description_add_server(topology, bson_iter_utf8(&member_iter, NULL), NULL);
      }
   }
}

typedef struct _mongoc_primary_and_topology_t {
   mongoc_topology_description_t *topology;
   mongoc_server_description_t *primary;
} mongoc_primary_and_topology_t;

/* invalidate old primaries */
static bool
_mongoc_topology_description_invalidate_primaries_cb (void *item,
                                                      void *ctx)
{
   mongoc_server_description_t *server = (mongoc_server_description_t *)item;
   mongoc_primary_and_topology_t *data = (mongoc_primary_and_topology_t *)ctx;

   if (server->id != data->primary->id &&
       server->type == MONGOC_SERVER_RS_PRIMARY) {
      mongoc_server_description_set_state (server, MONGOC_SERVER_UNKNOWN);
      mongoc_server_description_set_set_version (server, MONGOC_NO_SET_VERSION);
      mongoc_server_description_set_election_id (server, NULL);
   }
   return true;
}


/* Remove and destroy all replica set members not in primary's hosts lists */
static void
_mongoc_topology_description_remove_unreported_servers (
   mongoc_topology_description_t *topology,
   mongoc_server_description_t *primary)
{
   mongoc_array_t to_remove;
   int i;
   mongoc_server_description_t *member;
   const char *address;

   _mongoc_array_init (&to_remove,
                       sizeof (mongoc_server_description_t *));

   /* Accumulate servers to be removed - do this before calling
    * _mongoc_topology_description_remove_server, which could call
    * mongoc_server_description_cleanup on the primary itself if it
    * doesn't report its own connection_address in its hosts list.
    * See hosts_differ_from_seeds.json */
   for (i = 0; i < topology->servers->items_len; i++) {
      member = (mongoc_server_description_t *)mongoc_set_get_item (topology->servers, i);
      address = member->connection_address;
      if (!mongoc_server_description_has_rs_member(primary, address)) {
         _mongoc_array_append_val (&to_remove, member);
      }
   }

   /* now it's safe to call _mongoc_topology_description_remove_server,
    * even on the primary */
   for (i = 0; i < to_remove.len; i++) {
      member = _mongoc_array_index (
         &to_remove, mongoc_server_description_t *, i);

      _mongoc_topology_description_remove_server (topology, member);
   }

   _mongoc_array_destroy (&to_remove);
}


/*
 *--------------------------------------------------------------------------
 *
 * _mongoc_topology_description_matches_me --
 *
 *       Server Discovery And Monitoring Spec: "Removal from the topology of
 *       seed list members where the "me" property does not match the address
 *       used to connect prevents clients from being able to select a server,
 *       only to fail to re-select that server once the primary has responded.
 *
 * Returns:
 *       True if "me" matches "connection_address".
 *
 * Side Effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */
static bool
_mongoc_topology_description_matches_me (mongoc_server_description_t *server)
{
   BSON_ASSERT (server->connection_address);

   if (!server->me) {
      /* "me" is unknown: consider it a match */
      return true;
   }

   return strcasecmp (server->connection_address, server->me) == 0;
}


/*
 *--------------------------------------------------------------------------
 *
 * _mongoc_update_rs_from_primary --
 *
 *       First, determine that this is really the primary:
 *          -If this node isn't in the cluster, do nothing.
 *          -If the cluster's set name is null, set it to node's set name.
 *           Otherwise if the cluster's set name is different from node's,
 *           we found a rogue primary, so remove it from the cluster and
 *           check the cluster for a primary, then return.
 *          -If any of the members of cluster reports an address different
 *           from node's, node cannot be the primary.
 *       Now that we know this is the primary:
 *          -If any hosts, passives, or arbiters in node's description aren't
 *           in the cluster, add them as UNKNOWN servers.
 *          -If the cluster has any servers that aren't in node's description,
 *           remove and destroy them.
 *       Finally, check the cluster for the new primary.
 *
 * Returns:
 *       None.
 *
 * Side effects:
 *       Changes to the cluster, possible removal of cluster nodes.
 *
 *--------------------------------------------------------------------------
 */
static void
_mongoc_topology_description_update_rs_from_primary (mongoc_topology_description_t *topology,
                                                     mongoc_server_description_t   *server)
{
   mongoc_primary_and_topology_t data;

   BSON_ASSERT (topology);
   BSON_ASSERT (server);

   if (!_mongoc_topology_description_has_server(topology, server->connection_address, NULL)) return;

   /* If server->set_name was null this function wouldn't be called from
    * mongoc_server_description_handle_ismaster(). static code analyzers however
    * don't know that so we check for it explicitly. */
   if (server->set_name) {
      /* 'Server' can only be the primary if it has the right rs name  */

      if (!topology->set_name) {
         topology->set_name = bson_strdup (server->set_name);
      }
      else if (strcmp(topology->set_name, server->set_name) != 0) {
         _mongoc_topology_description_remove_server(topology, server);
         _update_rs_type (topology);
         return;
      }
   }

   if (mongoc_server_description_has_set_version (server) &&
       mongoc_server_description_has_election_id (server)) {
      /* Server Discovery And Monitoring Spec: "The client remembers the
       * greatest electionId reported by a primary, and distrusts primaries
       * with lesser electionIds. This prevents the client from oscillating
       * between the old and new primary during a split-brain period."
       */
      if (_mongoc_topology_description_later_election (topology, server)) {
         /* stale primary */
         mongoc_topology_description_invalidate_server (topology, server->id);
         _update_rs_type (topology);
         return;
      }

      /* server's electionId >= topology's max electionId */
      _mongoc_topology_description_set_max_election_id (topology, server);
   }

   if (mongoc_server_description_has_set_version (server) &&
         (! _mongoc_topology_description_has_set_version (topology) ||
            server->set_version > topology->max_set_version)) {
      _mongoc_topology_description_set_max_set_version (topology, server);
   }

   /* 'Server' is the primary! Invalidate other primaries if found */
   data.primary = server;
   data.topology = topology;
   mongoc_set_for_each(topology->servers, _mongoc_topology_description_invalidate_primaries_cb, &data);

   /* Add to topology description any new servers primary knows about */
   _mongoc_topology_description_add_new_servers (topology, server);

   /* Remove from topology description any servers primary doesn't know about */
   _mongoc_topology_description_remove_unreported_servers (topology, server);

   /* Finally, set topology type */
   _update_rs_type (topology);
}

/*
 *--------------------------------------------------------------------------
 *
 * _mongoc_topology_description_update_rs_without_primary --
 *
 *       Update cluster's information when there is no primary.
 *
 * Returns:
 *       None.
 *
 * Side Effects:
 *       Alters cluster state, may remove node from cluster.
 *
 *--------------------------------------------------------------------------
 */
static void
_mongoc_topology_description_update_rs_without_primary (mongoc_topology_description_t *topology,
                                                        mongoc_server_description_t   *server)
{
   BSON_ASSERT (topology);
   BSON_ASSERT (server);

   if (!_mongoc_topology_description_has_server(topology, server->connection_address, NULL)) {
      return;
   }

   /* make sure we're talking about the same replica set */
   if (server->set_name) {
      if (!topology->set_name) {
         topology->set_name = bson_strdup(server->set_name);
      }
      else if (strcmp(topology->set_name, server->set_name)!= 0) {
         _mongoc_topology_description_remove_server(topology, server);
         return;
      }
   }

   /* Add new servers that this replica set member knows about */
   _mongoc_topology_description_add_new_servers (topology, server);

   if (!_mongoc_topology_description_matches_me (server)) {
      _mongoc_topology_description_remove_server(topology, server);
      return;
   }

   /* If this server thinks there is a primary, label it POSSIBLE_PRIMARY */
   if (server->current_primary) {
      _mongoc_topology_description_label_unknown_member(topology,
                                                        server->current_primary,
                                                        MONGOC_SERVER_POSSIBLE_PRIMARY);
   }
}

/*
 *--------------------------------------------------------------------------
 *
 * _mongoc_topology_description_update_rs_with_primary_from_member --
 *
 *       Update cluster's information when there is a primary, but the
 *       update is coming from another replica set member.
 *
 * Returns:
 *       None.
 *
 * Side Effects:
 *       Alters cluster state.
 *
 *--------------------------------------------------------------------------
 */
static void
_mongoc_topology_description_update_rs_with_primary_from_member (mongoc_topology_description_t *topology,
                                                                 mongoc_server_description_t   *server)
{
   BSON_ASSERT (topology);
   BSON_ASSERT (server);

   if (!_mongoc_topology_description_has_server(topology, server->connection_address, NULL)) {
      return;
   }

   /* set_name should never be null here */
   if (strcmp(topology->set_name, server->set_name) != 0) {
      _mongoc_topology_description_remove_server(topology, server);
      _update_rs_type (topology);
      return;
   }

   if (!_mongoc_topology_description_matches_me (server)) {
      _mongoc_topology_description_remove_server(topology, server);
      return;
   }

   /* If there is no primary, label server's current_primary as the POSSIBLE_PRIMARY */
   if (!_mongoc_topology_description_has_primary(topology) && server->current_primary) {
      _mongoc_topology_description_set_state(topology, MONGOC_TOPOLOGY_RS_NO_PRIMARY);
      _mongoc_topology_description_label_unknown_member(topology,
                                                        server->current_primary,
                                                        MONGOC_SERVER_POSSIBLE_PRIMARY);
   }
}

/*
 *--------------------------------------------------------------------------
 *
 * _mongoc_topology_description_set_topology_type_to_sharded --
 *
 *       Sets topology's type to SHARDED.
 *
 * Returns:
 *       None
 *
 * Side effects:
 *       Alter's topology's type
 *
 *--------------------------------------------------------------------------
 */
static void
_mongoc_topology_description_set_topology_type_to_sharded (mongoc_topology_description_t *topology,
                                                           mongoc_server_description_t   *server)
{
   _mongoc_topology_description_set_state(topology, MONGOC_TOPOLOGY_SHARDED);
}

/*
 *--------------------------------------------------------------------------
 *
 * _mongoc_topology_description_transition_unknown_to_rs_no_primary --
 *
 *       Encapsulates transition from cluster state UNKNOWN to
 *       RS_NO_PRIMARY. Sets the type to RS_NO_PRIMARY,
 *       then updates the replica set accordingly.
 *
 * Returns:
 *       None.
 *
 * Side effects:
 *       Changes topology state.
 *
 *--------------------------------------------------------------------------
 */
static void
_mongoc_topology_description_transition_unknown_to_rs_no_primary (mongoc_topology_description_t *topology,
                                                                  mongoc_server_description_t   *server)
{
   _mongoc_topology_description_set_state(topology, MONGOC_TOPOLOGY_RS_NO_PRIMARY);
   _mongoc_topology_description_update_rs_without_primary(topology, server);
}

/*
 *--------------------------------------------------------------------------
 *
 * _mongoc_topology_description_remove_and_check_primary --
 *
 *       Remove the server and check if the topology still has a primary.
 *
 * Returns:
 *       None.
 *
 * Side effects:
 *       Removes server from topology and destroys it.
 *
 *--------------------------------------------------------------------------
 */
static void
_mongoc_topology_description_remove_and_check_primary (mongoc_topology_description_t *topology,
                                                       mongoc_server_description_t   *server)
{
   _mongoc_topology_description_remove_server(topology, server);
   _update_rs_type (topology);
}

/*
 *--------------------------------------------------------------------------
 *
 * _mongoc_topology_description_update_unknown_with_standalone --
 *
 *       If the cluster doesn't contain this server, do nothing.
 *       Otherwise, if the topology only has one seed, change its
 *       type to SINGLE. If the topology has multiple seeds, it does not
 *       include us, so remove this server and destroy it.
 *
 * Returns:
 *       None.
 *
 * Side effects:
 *       Changes the topology type, might remove server from topology.
 *
 *--------------------------------------------------------------------------
 */
static void
_mongoc_topology_description_update_unknown_with_standalone (mongoc_topology_description_t *topology,
                                                             mongoc_server_description_t   *server)
{
   BSON_ASSERT (topology);
   BSON_ASSERT (server);

   if (!_mongoc_topology_description_has_server(topology, server->connection_address, NULL)) return;

   if (topology->servers->items_len > 1) {
      /* This cluster contains other servers, it cannot be a standalone. */
      _mongoc_topology_description_remove_server(topology, server);
   } else {
      _mongoc_topology_description_set_state(topology, MONGOC_TOPOLOGY_SINGLE);
   }
}

/*
 *--------------------------------------------------------------------------
 *
 *  This table implements the 'ToplogyType' table outlined in the Server
 *  Discovery and Monitoring spec. Each row represents a server type,
 *  and each column represents the topology type. Given a current topology
 *  type T and a newly-observed server type S, use the function at
 *  state_transions[S][T] to transition to a new state.
 *
 *  Rows should be read like so:
 *  { server type for this row
 *     UNKNOWN,
 *     SHARDED,
 *     RS_NO_PRIMARY,
 *     RS_WITH_PRIMARY
 *  }
 *
 *--------------------------------------------------------------------------
 */

typedef void (*transition_t)(mongoc_topology_description_t *topology,
                             mongoc_server_description_t   *server);

transition_t
gSDAMTransitionTable[MONGOC_SERVER_DESCRIPTION_TYPES][MONGOC_TOPOLOGY_DESCRIPTION_TYPES] = {
   { /* UNKNOWN */
      NULL, /* MONGOC_TOPOLOGY_UNKNOWN */
      NULL, /* MONGOC_TOPOLOGY_SHARDED */
      NULL, /* MONGOC_TOPOLOGY_RS_NO_PRIMARY */
      _mongoc_topology_description_check_if_has_primary /* MONGOC_TOPOLOGY_RS_WITH_PRIMARY */
   },
   { /* STANDALONE */
      _mongoc_topology_description_update_unknown_with_standalone,
      _mongoc_topology_description_remove_server,
      _mongoc_topology_description_remove_server,
      _mongoc_topology_description_remove_and_check_primary
   },
   { /* MONGOS */
      _mongoc_topology_description_set_topology_type_to_sharded,
      NULL,
      _mongoc_topology_description_remove_server,
      _mongoc_topology_description_remove_and_check_primary
   },
   { /* POSSIBLE_PRIMARY */
      NULL,
      NULL,
      NULL,
      NULL
   },
   { /* PRIMARY */
      _mongoc_topology_description_update_rs_from_primary,
      _mongoc_topology_description_remove_server,
      _mongoc_topology_description_update_rs_from_primary,
      _mongoc_topology_description_update_rs_from_primary
   },
   { /* SECONDARY */
      _mongoc_topology_description_transition_unknown_to_rs_no_primary,
      _mongoc_topology_description_remove_server,
      _mongoc_topology_description_update_rs_without_primary,
      _mongoc_topology_description_update_rs_with_primary_from_member
   },
   { /* ARBITER */
      _mongoc_topology_description_transition_unknown_to_rs_no_primary,
      _mongoc_topology_description_remove_server,
      _mongoc_topology_description_update_rs_without_primary,
      _mongoc_topology_description_update_rs_with_primary_from_member
   },
   { /* RS_OTHER */
      _mongoc_topology_description_transition_unknown_to_rs_no_primary,
      _mongoc_topology_description_remove_server,
      _mongoc_topology_description_update_rs_without_primary,
      _mongoc_topology_description_update_rs_with_primary_from_member
   },
   { /* RS_GHOST */
      NULL,
      _mongoc_topology_description_remove_server,
      NULL,
      _mongoc_topology_description_check_if_has_primary
   }
};

/*
 *--------------------------------------------------------------------------
 *
 * mongoc_topology_description_handle_ismaster --
 *
 *      Handle an ismaster. This is called by the background SDAM process,
 *      and by client when invalidating servers.
 *
 *      NOTE: this method should only be called while holding the mutex on
 *      the owning topology object.
 *
 *--------------------------------------------------------------------------
 */

void
mongoc_topology_description_handle_ismaster (
   mongoc_topology_description_t *topology,
   mongoc_server_description_t   *sd,
   const bson_t                  *ismaster_response,
   int64_t                        rtt_msec,
   bson_error_t                  *error)
{
   BSON_ASSERT (topology);
   BSON_ASSERT (sd);

   if (!_mongoc_topology_description_has_server (topology,
                                                 sd->connection_address,
                                                 NULL)) {
      MONGOC_DEBUG("Couldn't find %s in Topology Description", sd->connection_address);
      return;
   }

   mongoc_server_description_handle_ismaster (sd, ismaster_response, rtt_msec,
                                              error);

   if (gSDAMTransitionTable[sd->type][topology->type]) {
      TRACE("Transitioning to %d for %d", topology->type, sd->type);
      gSDAMTransitionTable[sd->type][topology->type] (topology, sd);
   } else {
      TRACE("No transition entry to %d for %d", topology->type, sd->type);
   }
}
