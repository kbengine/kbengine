/*
 * Copyright 2013 MongoDB, Inc.
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


#include <bcon.h>
#include <stdio.h>

#include "mongoc-bulk-operation.h"
#include "mongoc-bulk-operation-private.h"
#include "mongoc-client-private.h"
#include "mongoc-find-and-modify-private.h"
#include "mongoc-find-and-modify.h"
#include "mongoc-collection.h"
#include "mongoc-collection-private.h"
#include "mongoc-cursor-private.h"
#include "mongoc-cursor-cursorid-private.h"
#include "mongoc-cursor-array-private.h"
#include "mongoc-error.h"
#include "mongoc-index.h"
#include "mongoc-log.h"
#include "mongoc-trace.h"
#include "mongoc-read-concern-private.h"
#include "mongoc-write-concern-private.h"


#undef MONGOC_LOG_DOMAIN
#define MONGOC_LOG_DOMAIN "collection"


#define _BSON_APPEND_WRITE_CONCERN(_bson, _write_concern) \
      do { \
         const bson_t *write_concern_bson; \
         mongoc_write_concern_t *write_concern_copy = NULL; \
         if (_write_concern->frozen) { \
            write_concern_bson = _mongoc_write_concern_get_bson (_write_concern); \
         } else { \
            /* _mongoc_write_concern_get_bson will freeze the write_concern */ \
            write_concern_copy = mongoc_write_concern_copy (_write_concern); \
            write_concern_bson = _mongoc_write_concern_get_bson (write_concern_copy); \
         } \
         BSON_APPEND_DOCUMENT (_bson, "writeConcern", write_concern_bson); \
         if (write_concern_copy) { \
            mongoc_write_concern_destroy (write_concern_copy); \
         } \
      } while(0); \

static bool
validate_name (const char *str)
{
   const char *c;

   if (str && *str) {
      for (c = str; *c; c++) {
         switch (*c) {
         case '/':
         case '\\':
         case '.':
         case '"':
         case '*':
         case '<':
         case '>':
         case ':':
         case '|':
         case '?':
            return false;
         default:
            break;
         }
      }
      return ((0 != strcmp (str, "oplog.$main")) &&
              (0 != strcmp (str, "$cmd")));
   }

   return false;
}

static mongoc_cursor_t *
_mongoc_collection_cursor_new (mongoc_collection_t *collection,
                               mongoc_query_flags_t flags)
{
  return _mongoc_cursor_new (collection->client,
                             collection->ns,
                             flags,
                             0,                  /* skip */
                             0,                  /* limit */
                             0,                  /* batch_size */
                             false,              /* is_command */
                             NULL,               /* query */
                             NULL,               /* fields */
                             NULL,               /* read prefs */
                             NULL);              /* read concern */
}

static void
_mongoc_collection_write_command_execute (mongoc_write_command_t       *command,
                                          const mongoc_collection_t    *collection,
                                          const mongoc_write_concern_t *write_concern,
                                          mongoc_write_result_t        *result)
{
   mongoc_server_stream_t *server_stream;

   ENTRY;

   server_stream = mongoc_cluster_stream_for_writes (&collection->client->cluster,
                                                     &result->error);

   if (!server_stream) {
      /* result->error has been filled out */
      EXIT;
   }

   _mongoc_write_command_execute (command, collection->client, server_stream,
                                  collection->db, collection->collection,
                                  write_concern, 0 /* offset */, result);

   mongoc_server_stream_cleanup (server_stream);

   EXIT;
}

/*
 *--------------------------------------------------------------------------
 *
 * _mongoc_collection_new --
 *
 *       INTERNAL API
 *
 *       Create a new mongoc_collection_t structure for the given client.
 *
 *       @client must remain valid during the lifetime of this structure.
 *       @db is the db name of the collection.
 *       @collection is the name of the collection.
 *       @read_prefs is the default read preferences to apply or NULL.
 *       @read_concern is the default read concern to apply or NULL.
 *       @write_concern is the default write concern to apply or NULL.
 *
 * Returns:
 *       A newly allocated mongoc_collection_t that should be freed with
 *       mongoc_collection_destroy().
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

mongoc_collection_t *
_mongoc_collection_new (mongoc_client_t              *client,
                        const char                   *db,
                        const char                   *collection,
                        const mongoc_read_prefs_t    *read_prefs,
                        const mongoc_read_concern_t  *read_concern,
                        const mongoc_write_concern_t *write_concern)
{
   mongoc_collection_t *col;

   ENTRY;

   BSON_ASSERT (client);
   BSON_ASSERT (db);
   BSON_ASSERT (collection);

   col = (mongoc_collection_t *)bson_malloc0(sizeof *col);
   col->client = client;
   col->write_concern = write_concern ?
      mongoc_write_concern_copy(write_concern) :
      mongoc_write_concern_new();
   col->read_concern = read_concern ?
      mongoc_read_concern_copy(read_concern) :
      mongoc_read_concern_new();
   col->read_prefs = read_prefs ?
      mongoc_read_prefs_copy(read_prefs) :
      mongoc_read_prefs_new(MONGOC_READ_PRIMARY);

   bson_snprintf (col->ns, sizeof col->ns, "%s.%s", db, collection);
   bson_snprintf (col->db, sizeof col->db, "%s", db);
   bson_snprintf (col->collection, sizeof col->collection, "%s", collection);

   col->collectionlen = (uint32_t)strlen(col->collection);
   col->nslen = (uint32_t)strlen(col->ns);

   _mongoc_buffer_init(&col->buffer, NULL, 0, NULL, NULL);

   col->gle = NULL;

   RETURN(col);
}


/*
 *--------------------------------------------------------------------------
 *
 * mongoc_collection_destroy --
 *
 *       Release resources associated with @collection and frees the
 *       structure.
 *
 * Returns:
 *       None.
 *
 * Side effects:
 *       Everything.
 *
 *--------------------------------------------------------------------------
 */

void
mongoc_collection_destroy (mongoc_collection_t *collection) /* IN */
{
   ENTRY;

   BSON_ASSERT (collection);

   bson_clear (&collection->gle);

   _mongoc_buffer_destroy(&collection->buffer);

   if (collection->read_prefs) {
      mongoc_read_prefs_destroy(collection->read_prefs);
      collection->read_prefs = NULL;
   }

   if (collection->read_concern) {
      mongoc_read_concern_destroy(collection->read_concern);
      collection->read_concern = NULL;
   }

   if (collection->write_concern) {
      mongoc_write_concern_destroy(collection->write_concern);
      collection->write_concern = NULL;
   }

   bson_free(collection);

   EXIT;
}


/*
 *--------------------------------------------------------------------------
 *
 * mongoc_collection_copy --
 *
 *       Returns a copy of @collection that needs to be freed by calling
 *       mongoc_collection_destroy.
 *
 * Returns:
 *       A copy of this collection.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

mongoc_collection_t *
mongoc_collection_copy (mongoc_collection_t *collection) /* IN */
{
   ENTRY;

   BSON_ASSERT (collection);

   RETURN(_mongoc_collection_new(collection->client, collection->db,
                                 collection->collection, collection->read_prefs,
                                 collection->read_concern,
                                 collection->write_concern));
}


/*
 *--------------------------------------------------------------------------
 *
 * mongoc_collection_aggregate --
 *
 *       Send an "aggregate" command to the MongoDB server.
 *
 *       This varies it's behavior based on the wire version.  If we're on
 *       wire_version > 0, we use the new aggregate command, which returns a
 *       database cursor.  On wire_version == 0, we create synthetic cursor on
 *       top of the array returned in result.
 *
 *       This function will always return a new mongoc_cursor_t that should
 *       be freed with mongoc_cursor_destroy().
 *
 *       The cursor may fail once iterated upon, so check
 *       mongoc_cursor_error() if mongoc_cursor_next() returns false.
 *
 *       See http://docs.mongodb.org/manual/aggregation/ for more
 *       information on how to build aggregation pipelines.
 *
 * Requires:
 *       MongoDB >= 2.1.0
 *
 * Parameters:
 *       @flags: bitwise or of mongoc_query_flags_t or 0.
 *       @pipeline: A bson_t containing the pipeline request. @pipeline
 *                  will be sent as an array type in the request.
 *       @options:  A bson_t containing aggregation options, such as
 *                  bypassDocumentValidation (used with $out pipeline),
 *                  maxTimeMS (declaring maximum server execution time) and
 *                  explain (return information on the processing of the pipeline).
 *       @read_prefs: Optional read preferences for the command.
 *
 * Returns:
 *       A newly allocated mongoc_cursor_t that should be freed with
 *       mongoc_cursor_destroy().
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

mongoc_cursor_t *
mongoc_collection_aggregate (mongoc_collection_t       *collection, /* IN */
                             mongoc_query_flags_t       flags,      /* IN */
                             const bson_t              *pipeline,   /* IN */
                             const bson_t              *options,    /* IN */
                             const mongoc_read_prefs_t *read_prefs) /* IN */
{
   mongoc_server_description_t *selected_server;
   mongoc_cursor_t *cursor;
   bson_iter_t iter;
   bson_t command;
   bson_t child;
   int32_t batch_size = 0;
   bool use_cursor;

   ENTRY;

   BSON_ASSERT (collection);
   BSON_ASSERT (pipeline);

   bson_init (&command);

   if (!read_prefs) {
      read_prefs = collection->read_prefs;
   }

   cursor = _mongoc_collection_cursor_new (collection, flags);
   selected_server = mongoc_topology_select(collection->client->topology,
                                            MONGOC_SS_READ,
                                            read_prefs,
                                            15,
                                            &cursor->error);

   if (!selected_server) {
      GOTO (done);
   }

   cursor->hint = selected_server->id;
   use_cursor = 
      selected_server->max_wire_version >= WIRE_VERSION_AGG_CURSOR;

   BSON_APPEND_UTF8 (&command, "aggregate", collection->collection);

   /*
    * The following will allow @pipeline to be either an array of
    * items for the pipeline, or {"pipeline": [...]}.
    */
   if (bson_iter_init_find (&iter, pipeline, "pipeline") &&
       BSON_ITER_HOLDS_ARRAY (&iter)) {
      if (!bson_append_iter (&command, "pipeline", 8, &iter)) {
         bson_set_error (&cursor->error,
               MONGOC_ERROR_COMMAND,
               MONGOC_ERROR_COMMAND_INVALID_ARG,
               "Failed to append \"pipeline\" to create command.");
         GOTO (done);
      }
   } else {
      BSON_APPEND_ARRAY (&command, "pipeline", pipeline);
   }

   /* for newer version, we include a cursor subdocument */
   if (use_cursor) {
      bson_append_document_begin (&command, "cursor", 6, &child);

      if (options && bson_iter_init (&iter, options)) {
         while (bson_iter_next (&iter)) {
            if (BSON_ITER_IS_KEY (&iter, "batchSize") &&
                (BSON_ITER_HOLDS_INT32 (&iter) ||
                 BSON_ITER_HOLDS_INT64 (&iter) ||
                 BSON_ITER_HOLDS_DOUBLE (&iter))) {
               batch_size = (int32_t)bson_iter_as_int64 (&iter);
               BSON_APPEND_INT32 (&child, "batchSize", batch_size);
            }
         }
      }

      bson_append_document_end (&command, &child);
   }

   if (options && bson_iter_init (&iter, options)) {
      while (bson_iter_next (&iter)) {
         if (! (BSON_ITER_IS_KEY (&iter, "batchSize") || BSON_ITER_IS_KEY (&iter, "cursor"))) {
            if (!bson_append_iter (&command, bson_iter_key (&iter), -1, &iter)) {
               bson_set_error (&cursor->error,
                     MONGOC_ERROR_COMMAND,
                     MONGOC_ERROR_COMMAND_INVALID_ARG,
                     "Failed to append \"batchSize\" or \"cursor\" to create command.");
               GOTO (done);
            }
         }
      }
   }

   if (collection->read_concern->level != NULL) {
      const bson_t *read_concern_bson;

      if (selected_server->max_wire_version < WIRE_VERSION_READ_CONCERN) {
         bson_set_error (&cursor->error,
                         MONGOC_ERROR_COMMAND,
                         MONGOC_ERROR_PROTOCOL_BAD_WIRE_VERSION,
                         "The selected server does not support readConcern");
         GOTO (done);
      }

      read_concern_bson = _mongoc_read_concern_get_bson (collection->read_concern);
      BSON_APPEND_DOCUMENT (&command, "readConcern", read_concern_bson);
   }

   if (use_cursor) {
      _mongoc_cursor_cursorid_init (cursor, &command);
   } else {
      /* for older versions we get an array that we can create a synthetic
       * cursor on top of */
      _mongoc_cursor_array_init (cursor, &command, "result");
   }

done:
   if (selected_server) {
      mongoc_server_description_destroy (selected_server);
   }

   bson_destroy(&command);

   /* we always return the cursor, even if it fails; users can detect the
    * failure on performing a cursor operation. see CDRIVER-880. */
   RETURN (cursor);
}


/*
 *--------------------------------------------------------------------------
 *
 * mongoc_collection_find --
 *
 *       Performs a query against the configured MongoDB server. If @read_prefs
 *       is provided, it will be used to locate a MongoDB node in the cluster
 *       to deliver the query to.
 *
 *       @flags may be bitwise-or'd flags or MONGOC_QUERY_NONE.
 *
 *       @skip may contain the number of documents to skip before returning the
 *       matching document.
 *
 *       @limit may contain the maximum number of documents that may be
 *       returned.
 *
 *       This function will always return a cursor, with the exception of
 *       invalid API use.
 *
 * Parameters:
 *       @collection: A mongoc_collection_t.
 *       @flags: A bitwise or of mongoc_query_flags_t.
 *       @skip: The number of documents to skip.
 *       @limit: The maximum number of items.
 *       @batch_size: The batch size
 *       @query: The query to locate matching documents.
 *       @fields: The fields to return, or NULL for all fields.
 *       @read_prefs: Read preferences to choose cluster node.
 *
 * Returns:
 *       A newly allocated mongoc_cursor_t that should be freed with
 *       mongoc_cursor_destroy().
 *
 *       The client used by mongoc_collection_t must be valid for the
 *       lifetime of the resulting mongoc_cursor_t.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

mongoc_cursor_t *
mongoc_collection_find (mongoc_collection_t       *collection, /* IN */
                        mongoc_query_flags_t       flags,      /* IN */
                        uint32_t                   skip,       /* IN */
                        uint32_t                   limit,      /* IN */
                        uint32_t                   batch_size, /* IN */
                        const bson_t              *query,      /* IN */
                        const bson_t              *fields,     /* IN */
                        const mongoc_read_prefs_t *read_prefs) /* IN */
{
   BSON_ASSERT (collection);
   BSON_ASSERT (query);

   bson_clear (&collection->gle);

   if (!read_prefs) {
      read_prefs = collection->read_prefs;
   }

   return _mongoc_cursor_new (collection->client, collection->ns, flags, skip,
                              limit, batch_size, false, query, fields, read_prefs,
                              collection->read_concern);
}


/*
 *--------------------------------------------------------------------------
 *
 * mongoc_collection_command --
 *
 *       Executes a command on a cluster node matching @read_prefs. If
 *       @read_prefs is not provided, it will be run on the primary node.
 *
 *       This function will always return a mongoc_cursor_t.
 *
 * Parameters:
 *       @collection: A mongoc_collection_t.
 *       @flags: Bitwise-or'd flags for command.
 *       @skip: Number of documents to skip, typically 0.
 *       @limit : Number of documents to return
 *       @batch_size : Batch size
 *       @query: The command to execute.
 *       @fields: The fields to return, or NULL.
 *       @read_prefs: Command read preferences or NULL.
 *
 * Returns:
 *       None.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

mongoc_cursor_t *
mongoc_collection_command (mongoc_collection_t       *collection,
                           mongoc_query_flags_t       flags,
                           uint32_t                   skip,
                           uint32_t                   limit,
                           uint32_t                   batch_size,
                           const bson_t              *query,
                           const bson_t              *fields,
                           const mongoc_read_prefs_t *read_prefs)
{
   char ns[MONGOC_NAMESPACE_MAX];

   BSON_ASSERT (collection);
   BSON_ASSERT (query);

   if (!read_prefs) {
      read_prefs = collection->read_prefs;
   }

   bson_clear (&collection->gle);

   if (NULL == strstr (collection->collection, "$cmd")) {
      bson_snprintf (ns, sizeof ns, "%s", collection->db);
   } else {
      bson_snprintf (ns, sizeof ns, "%s.%s",
                     collection->db, collection->collection);
   }

   return mongoc_client_command (collection->client, ns, flags,
                                 skip, limit, batch_size, query, fields, read_prefs);
}

bool
mongoc_collection_command_simple (mongoc_collection_t       *collection,
                                  const bson_t              *command,
                                  const mongoc_read_prefs_t *read_prefs,
                                  bson_t                    *reply,
                                  bson_error_t              *error)
{
   BSON_ASSERT (collection);
   BSON_ASSERT (command);

   bson_clear (&collection->gle);

   return mongoc_client_command_simple (collection->client, collection->db,
                                        command, read_prefs, reply, error);
}

/*
 *--------------------------------------------------------------------------
 *
 * mongoc_collection_count --
 *
 *       Count the number of documents matching @query.
 *
 * Parameters:
 *       @flags: A mongoc_query_flags_t describing the query flags or 0.
 *       @query: The query to perform or NULL for {}.
 *       @skip: The $skip to perform within the query or 0.
 *       @limit: The $limit to perform within the query or 0.
 *       @read_prefs: desired read preferences or NULL.
 *       @error: A location for an error or NULL.
 *
 * Returns:
 *       -1 on failure; otherwise the number of matching documents.
 *
 * Side effects:
 *       @error is set upon failure if non-NULL.
 *
 *--------------------------------------------------------------------------
 */

int64_t
mongoc_collection_count (mongoc_collection_t       *collection,  /* IN */
                         mongoc_query_flags_t       flags,       /* IN */
                         const bson_t              *query,       /* IN */
                         int64_t                    skip,        /* IN */
                         int64_t                    limit,       /* IN */
                         const mongoc_read_prefs_t *read_prefs,  /* IN */
                         bson_error_t              *error)       /* OUT */
{
   return mongoc_collection_count_with_opts (
      collection,
      flags,
      query,
      skip,
      limit,
      NULL,
      read_prefs,
      error);
}


int64_t
mongoc_collection_count_with_opts (mongoc_collection_t       *collection,  /* IN */
                                   mongoc_query_flags_t       flags,       /* IN */
                                   const bson_t              *query,       /* IN */
                                   int64_t                    skip,        /* IN */
                                   int64_t                    limit,       /* IN */
                                   const bson_t               *opts,       /* IN */
                                   const mongoc_read_prefs_t *read_prefs,  /* IN */
                                   bson_error_t              *error)       /* OUT */
{
   mongoc_server_stream_t *server_stream;
   mongoc_cluster_t *cluster;
   bson_iter_t iter;
   int64_t ret = -1;
   bool success;
   bson_t reply;
   bson_t cmd;
   bson_t q;

   ENTRY;


   cluster = &collection->client->cluster;
   server_stream = mongoc_cluster_stream_for_writes (cluster, error);
   if (!server_stream) {
      RETURN (-1);
   }

   BSON_ASSERT (collection);

   bson_init(&cmd);
   bson_append_utf8(&cmd, "count", 5, collection->collection,
                    collection->collectionlen);
   if (query) {
      bson_append_document(&cmd, "query", 5, query);
   } else {
      bson_init(&q);
      bson_append_document(&cmd, "query", 5, &q);
      bson_destroy(&q);
   }
   if (limit) {
      bson_append_int64(&cmd, "limit", 5, limit);
   }
   if (skip) {
      bson_append_int64(&cmd, "skip", 4, skip);
   }
   if (collection->read_concern->level != NULL) {
      const bson_t *read_concern_bson;

      if (server_stream->sd->max_wire_version < WIRE_VERSION_READ_CONCERN) {
         bson_set_error (error,
                         MONGOC_ERROR_COMMAND,
                         MONGOC_ERROR_PROTOCOL_BAD_WIRE_VERSION,
                         "The selected server does not support readConcern");
         bson_destroy (&cmd);
         mongoc_server_stream_cleanup (server_stream);
         RETURN (-1);
      }

      read_concern_bson = _mongoc_read_concern_get_bson (collection->read_concern);
      BSON_APPEND_DOCUMENT (&cmd, "readConcern", read_concern_bson);
   }
   if (opts) {
       bson_concat(&cmd, opts);
   }

   success = mongoc_cluster_run_command (cluster, server_stream->stream,
                                         server_stream->sd->id,
                                         MONGOC_QUERY_SLAVE_OK, collection->db,
                                         &cmd, &reply, error);

   if (success && bson_iter_init_find(&iter, &reply, "n")) {
      ret = bson_iter_as_int64(&iter);
   }
   bson_destroy (&reply);
   bson_destroy (&cmd);
   mongoc_server_stream_cleanup (server_stream);

   RETURN (ret);
}


/*
 *--------------------------------------------------------------------------
 *
 * mongoc_collection_drop --
 *
 *       Request the MongoDB server drop the collection.
 *
 * Returns:
 *       true if successful; otherwise false and @error is set.
 *
 * Side effects:
 *       @error is set upon failure.
 *
 *--------------------------------------------------------------------------
 */

bool
mongoc_collection_drop (mongoc_collection_t *collection, /* IN */
                        bson_error_t        *error)      /* OUT */
{
   bool ret;
   bson_t cmd;

   BSON_ASSERT (collection);

   bson_init(&cmd);
   bson_append_utf8(&cmd, "drop", 4, collection->collection,
                    collection->collectionlen);
   ret = mongoc_collection_command_simple(collection, &cmd, NULL, NULL, error);
   bson_destroy(&cmd);

   return ret;
}


/*
 *--------------------------------------------------------------------------
 *
 * mongoc_collection_drop_index --
 *
 *       Request the MongoDB server drop the named index.
 *
 * Returns:
 *       true if successful; otherwise false and @error is set.
 *
 * Side effects:
 *       @error is setup upon failure if non-NULL.
 *
 *--------------------------------------------------------------------------
 */

bool
mongoc_collection_drop_index (mongoc_collection_t *collection, /* IN */
                              const char          *index_name, /* IN */
                              bson_error_t        *error)      /* OUT */
{
   bool ret;
   bson_t cmd;

   BSON_ASSERT (collection);
   BSON_ASSERT (index_name);

   bson_init(&cmd);
   bson_append_utf8(&cmd, "dropIndexes", -1, collection->collection,
                    collection->collectionlen);
   bson_append_utf8(&cmd, "index", -1, index_name, -1);
   ret = mongoc_collection_command_simple(collection, &cmd, NULL, NULL, error);
   bson_destroy(&cmd);

   return ret;
}


char *
mongoc_collection_keys_to_index_string (const bson_t *keys)
{
   bson_string_t *s;
   bson_iter_t iter;
   int i = 0;

   BSON_ASSERT (keys);

   if (!bson_iter_init (&iter, keys)) {
      return NULL;
   }

   s = bson_string_new (NULL);

   while (bson_iter_next (&iter)) {

      /* Index type can be specified as a string ("2d") or as an integer representing direction */
      if (bson_iter_type(&iter) == BSON_TYPE_UTF8)
      {
          bson_string_append_printf (s,
                                     (i++ ? "_%s_%s" : "%s_%s"),
                                     bson_iter_key (&iter),
                                     bson_iter_utf8 (&iter, NULL));
      }
      else
      {

          bson_string_append_printf (s,
                                     (i++ ? "_%s_%d" : "%s_%d"),
                                     bson_iter_key (&iter),
                                     bson_iter_int32 (&iter));
      }

   }

   return bson_string_free (s, false);
}


/*
 *--------------------------------------------------------------------------
 *
 * _mongoc_collection_create_index_legacy --
 *
 *       Request the MongoDB server create the named index.
 *
 * Returns:
 *       true if successful; otherwise false and @error is set.
 *
 * Side effects:
 *       @error is setup upon failure if non-NULL.
 *
 *--------------------------------------------------------------------------
 */

static bool
_mongoc_collection_create_index_legacy (mongoc_collection_t      *collection,
                                        const bson_t             *keys,
                                        const mongoc_index_opt_t *opt,
                                        bson_error_t             *error)
{
   const mongoc_index_opt_t *def_opt;
   mongoc_collection_t *col;
   bool ret;
   bson_t insert;
   char *name;

   BSON_ASSERT (collection);

   /*
    * TODO: this is supposed to be cached and cheap... make it that way
    */

   def_opt = mongoc_index_opt_get_default ();
   opt = opt ? opt : def_opt;

   if (!opt->is_initialized) {
      MONGOC_WARNING("Options have not yet been initialized");
      return false;
   }

   bson_init (&insert);

   bson_append_document (&insert, "key", -1, keys);
   bson_append_utf8 (&insert, "ns", -1, collection->ns, -1);

   if (opt->background != def_opt->background) {
      bson_append_bool (&insert, "background", -1, opt->background);
   }

   if (opt->unique != def_opt->unique) {
      bson_append_bool (&insert, "unique", -1, opt->unique);
   }

   if (opt->name != def_opt->name) {
      bson_append_utf8 (&insert, "name", -1, opt->name, -1);
   } else {
      name = mongoc_collection_keys_to_index_string(keys);
      if (!name) {
         bson_set_error (error,
                         MONGOC_ERROR_BSON,
                         MONGOC_ERROR_BSON_INVALID,
                         "Cannot generate index name from invalid `keys` argument"
                         );
         bson_destroy (&insert);
         return false;
      }
      bson_append_utf8 (&insert, "name", -1, name, -1);
      bson_free (name);
   }

   if (opt->drop_dups != def_opt->drop_dups) {
      bson_append_bool (&insert, "dropDups", -1, opt->drop_dups);
   }

   if (opt->sparse != def_opt->sparse) {
      bson_append_bool (&insert, "sparse", -1, opt->sparse);
   }

   if (opt->expire_after_seconds != def_opt->expire_after_seconds) {
      bson_append_int32 (&insert,
                         "expireAfterSeconds", -1,
                         opt->expire_after_seconds);
   }

   if (opt->v != def_opt->v) {
      bson_append_int32 (&insert, "v", -1, opt->v);
   }

   if (opt->weights != def_opt->weights) {
      bson_append_document (&insert, "weights", -1, opt->weights);
   }

   if (opt->default_language != def_opt->default_language) {
      bson_append_utf8 (&insert,
                        "default_language", -1,
                        opt->default_language, -1);
   }

   if (opt->language_override != def_opt->language_override) {
      bson_append_utf8 (&insert,
                        "language_override", -1,
                        opt->language_override, -1);
   }

   col = mongoc_client_get_collection (collection->client, collection->db,
                                       "system.indexes");

   ret = mongoc_collection_insert (col,
                                  (mongoc_insert_flags_t)MONGOC_INSERT_NO_VALIDATE,
                                  &insert, NULL, error);

   mongoc_collection_destroy(col);

   bson_destroy (&insert);

   return ret;
}


bool
mongoc_collection_create_index (mongoc_collection_t      *collection,
                                const bson_t             *keys,
                                const mongoc_index_opt_t *opt,
                                bson_error_t             *error)
{
   const mongoc_index_opt_t *def_opt;
   const mongoc_index_opt_geo_t *def_geo;
   bson_error_t local_error;
   const char *name;
   bson_t cmd = BSON_INITIALIZER;
   bson_t ar;
   bson_t doc;
   bson_t reply;
   bson_t storage_doc;
   bson_t wt_doc;
   const mongoc_index_opt_geo_t *geo_opt;
   const mongoc_index_opt_storage_t *storage_opt;
   const mongoc_index_opt_wt_t *wt_opt;
   char *alloc_name = NULL;
   bool ret = false;

   BSON_ASSERT (collection);
   BSON_ASSERT (keys);

   def_opt = mongoc_index_opt_get_default ();
   opt = opt ? opt : def_opt;

   /*
    * Generate the key name if it was not provided.
    */
   name = (opt->name != def_opt->name) ? opt->name : NULL;
   if (!name) {
      alloc_name = mongoc_collection_keys_to_index_string (keys);
      if (alloc_name) {
         name = alloc_name;
      } else {
         bson_set_error (error,
                         MONGOC_ERROR_BSON,
                         MONGOC_ERROR_BSON_INVALID,
                         "Cannot generate index name from invalid `keys` argument"
                         );
         bson_destroy (&cmd);
         return false;
      }
   }

   /*
    * Build our createIndexes command to send to the server.
    */
   BSON_APPEND_UTF8 (&cmd, "createIndexes", collection->collection);
   bson_append_array_begin (&cmd, "indexes", 7, &ar);
   bson_append_document_begin (&ar, "0", 1, &doc);
   BSON_APPEND_DOCUMENT (&doc, "key", keys);
   BSON_APPEND_UTF8 (&doc, "name", name);
   if (opt->background) {
      BSON_APPEND_BOOL (&doc, "background", true);
   }
   if (opt->unique) {
      BSON_APPEND_BOOL (&doc, "unique", true);
   }
   if (opt->drop_dups) {
      BSON_APPEND_BOOL (&doc, "dropDups", true);
   }
   if (opt->sparse) {
      BSON_APPEND_BOOL (&doc, "sparse", true);
   }
   if (opt->expire_after_seconds != def_opt->expire_after_seconds) {
      BSON_APPEND_INT32 (&doc, "expireAfterSeconds", opt->expire_after_seconds);
   }
   if (opt->v != def_opt->v) {
      BSON_APPEND_INT32 (&doc, "v", opt->v);
   }
   if (opt->weights && (opt->weights != def_opt->weights)) {
      BSON_APPEND_DOCUMENT (&doc, "weights", opt->weights);
   }
   if (opt->default_language != def_opt->default_language) {
      BSON_APPEND_UTF8 (&doc, "default_language", opt->default_language);
   }
   if (opt->language_override != def_opt->language_override) {
      BSON_APPEND_UTF8 (&doc, "language_override", opt->language_override);
   }
   if (opt->partial_filter_expression) {
      BSON_APPEND_DOCUMENT (&doc, "partialFilterExpression", opt->partial_filter_expression);
   }
   if (opt->geo_options) {
       geo_opt = opt->geo_options;
       def_geo = mongoc_index_opt_geo_get_default ();
       if (geo_opt->twod_sphere_version != def_geo->twod_sphere_version) {
          BSON_APPEND_INT32 (&doc, "2dsphereIndexVersion", geo_opt->twod_sphere_version);
       }
       if (geo_opt->twod_bits_precision != def_geo->twod_bits_precision) {
          BSON_APPEND_INT32 (&doc, "bits", geo_opt->twod_bits_precision);
       }
       if (geo_opt->twod_location_min != def_geo->twod_location_min) {
          BSON_APPEND_DOUBLE (&doc, "min", geo_opt->twod_location_min);
       }
       if (geo_opt->twod_location_max != def_geo->twod_location_max) {
          BSON_APPEND_DOUBLE (&doc, "max", geo_opt->twod_location_max);
       }
       if (geo_opt->haystack_bucket_size != def_geo->haystack_bucket_size) {
          BSON_APPEND_DOUBLE (&doc, "bucketSize", geo_opt->haystack_bucket_size);
       }
   }

   if (opt->storage_options) {
      storage_opt = opt->storage_options;
      switch (storage_opt->type) {
      case MONGOC_INDEX_STORAGE_OPT_WIREDTIGER:
         wt_opt = (mongoc_index_opt_wt_t *)storage_opt;
         BSON_APPEND_DOCUMENT_BEGIN (&doc, "storageEngine", &storage_doc);
         BSON_APPEND_DOCUMENT_BEGIN (&storage_doc, "wiredTiger", &wt_doc);
         BSON_APPEND_UTF8 (&wt_doc, "configString", wt_opt->config_str);
         bson_append_document_end (&storage_doc, &wt_doc);
         bson_append_document_end (&doc, &storage_doc);
         break;
      default:
         break;
      }
   }

   bson_append_document_end (&ar, &doc);
   bson_append_array_end (&cmd, &ar);

   ret = mongoc_collection_command_simple (collection, &cmd, NULL, &reply,
                                           &local_error);

   /*
    * If we failed due to the command not being found, then use the legacy
    * version which performs an insert into the system.indexes collection.
    */
   if (!ret) {
      if (local_error.code == MONGOC_ERROR_QUERY_COMMAND_NOT_FOUND) {
         ret = _mongoc_collection_create_index_legacy (collection, keys, opt,
                                                       error);
      } else if (error) {
         memcpy (error, &local_error, sizeof *error);
      }
   }

   bson_destroy (&cmd);
   bson_destroy (&reply);
   bson_free (alloc_name);

   return ret;
}


bool
mongoc_collection_ensure_index (mongoc_collection_t      *collection,
                                const bson_t             *keys,
                                const mongoc_index_opt_t *opt,
                                bson_error_t             *error)
{
   return mongoc_collection_create_index (collection, keys, opt, error);
}

mongoc_cursor_t *
_mongoc_collection_find_indexes_legacy (mongoc_collection_t *collection,
                                        bson_error_t        *error)
{
   mongoc_database_t *db;
   mongoc_collection_t *idx_collection;
   mongoc_read_prefs_t *read_prefs;
   bson_t query = BSON_INITIALIZER;
   mongoc_cursor_t *cursor;

   BSON_ASSERT (collection);

   BSON_APPEND_UTF8 (&query, "ns", collection->ns);

   db = mongoc_client_get_database (collection->client, collection->db);
   BSON_ASSERT (db);

   idx_collection = mongoc_database_get_collection (db, "system.indexes");
   BSON_ASSERT (idx_collection);

   read_prefs = mongoc_read_prefs_new (MONGOC_READ_PRIMARY);

   cursor = mongoc_collection_find (idx_collection, MONGOC_QUERY_NONE, 0, 0, 0,
                                    &query, NULL, read_prefs);

   mongoc_read_prefs_destroy (read_prefs);
   mongoc_collection_destroy (idx_collection);
   mongoc_database_destroy (db);

   return cursor;
}

mongoc_cursor_t *
mongoc_collection_find_indexes (mongoc_collection_t *collection,
                                bson_error_t        *error)
{
   mongoc_cursor_t *cursor;
   bson_t cmd = BSON_INITIALIZER;
   bson_t child;

   BSON_ASSERT (collection);

   bson_append_utf8 (&cmd, "listIndexes", -1, collection->collection,
                     collection->collectionlen);

   BSON_APPEND_DOCUMENT_BEGIN (&cmd, "cursor", &child);
   bson_append_document_end (&cmd, &child);

   cursor = _mongoc_collection_cursor_new (collection, MONGOC_QUERY_SLAVE_OK);
   _mongoc_cursor_cursorid_init (cursor, &cmd);

   if (_mongoc_cursor_cursorid_prime (cursor)) {
       /* intentionally empty */
   } else {
      if (mongoc_cursor_error (cursor, error)) {
         mongoc_cursor_destroy (cursor);

         if (error->code == MONGOC_ERROR_COLLECTION_DOES_NOT_EXIST) {
            bson_t empty_arr = BSON_INITIALIZER;
            /* collection does not exist. in accordance with the spec we return
             * an empty array. Also we need to clear out the error. */
            error->code = 0;
            error->domain = 0;
            cursor = _mongoc_collection_cursor_new (collection,
                                                    MONGOC_QUERY_SLAVE_OK);

            _mongoc_cursor_array_init (cursor, NULL, NULL);
            _mongoc_cursor_array_set_bson (cursor, &empty_arr);
         } else if (error->code == MONGOC_ERROR_QUERY_COMMAND_NOT_FOUND) {
            /* talking to an old server. */
            /* clear out error. */
            error->code = 0;
            error->domain = 0;
            cursor = _mongoc_collection_find_indexes_legacy (collection, error);
         }
      }
   }

   bson_destroy (&cmd);

   return cursor;
}

/*
 *--------------------------------------------------------------------------
 *
 * mongoc_collection_insert_bulk --
 *
 *       Bulk insert documents into a MongoDB collection.
 *
 * Parameters:
 *       @collection: A mongoc_collection_t.
 *       @flags: flags for the insert or 0.
 *       @documents: The documents to insert.
 *       @n_documents: The number of documents to insert.
 *       @write_concern: A write concern or NULL.
 *       @error: a location for an error or NULL.
 *
 * Returns:
 *       true if successful; otherwise false and @error is set.
 *
 *       If the write concern does not dictate checking the result of the
 *       insert, then true may be returned even though the document was
 *       not actually inserted on the MongoDB server or cluster.
 *
 * Side effects:
 *       @collection->gle is setup, depending on write_concern->w value.
 *       @error may be set upon failure if non-NULL.
 *
 *--------------------------------------------------------------------------
 */

bool
mongoc_collection_insert_bulk (mongoc_collection_t           *collection,
                               mongoc_insert_flags_t          flags,
                               const bson_t                 **documents,
                               uint32_t                       n_documents,
                               const mongoc_write_concern_t  *write_concern,
                               bson_error_t                  *error)
{
   mongoc_write_command_t command;
   mongoc_write_result_t result;
   mongoc_bulk_write_flags_t write_flags = MONGOC_BULK_WRITE_FLAGS_INIT;
   bool ret;
   uint32_t i;

   BSON_ASSERT (collection);
   BSON_ASSERT (documents);

   if (!write_concern) {
      write_concern = collection->write_concern;
   }

   if (!(flags & MONGOC_INSERT_NO_VALIDATE)) {
      int vflags = (BSON_VALIDATE_UTF8 | BSON_VALIDATE_UTF8_ALLOW_NULL
                  | BSON_VALIDATE_DOLLAR_KEYS | BSON_VALIDATE_DOT_KEYS);

      for (i = 0; i < n_documents; i++) {
         if (!bson_validate (documents[i], (bson_validate_flags_t)vflags, NULL)) {
            bson_set_error (error,
                            MONGOC_ERROR_BSON,
                            MONGOC_ERROR_BSON_INVALID,
                            "A document was corrupt or contained "
                            "invalid characters . or $");
            RETURN (false);
         }
      }
   }

   bson_clear (&collection->gle);

   _mongoc_write_result_init (&result);

   write_flags.ordered = !(flags & MONGOC_INSERT_CONTINUE_ON_ERROR);

   _mongoc_write_command_init_insert (&command, NULL, write_flags, true);

   for (i = 0; i < n_documents; i++) {
      _mongoc_write_command_insert_append (&command, documents[i]);
   }

   _mongoc_collection_write_command_execute (&command, collection,
                                             write_concern, &result);

   collection->gle = bson_new ();
   ret = _mongoc_write_result_complete (&result, collection->gle, error);

   _mongoc_write_result_destroy (&result);
   _mongoc_write_command_destroy (&command);

   return ret;
}


/*
 *--------------------------------------------------------------------------
 *
 * mongoc_collection_insert --
 *
 *       Insert a document into a MongoDB collection.
 *
 * Parameters:
 *       @collection: A mongoc_collection_t.
 *       @flags: flags for the insert or 0.
 *       @document: The document to insert.
 *       @write_concern: A write concern or NULL.
 *       @error: a location for an error or NULL.
 *
 * Returns:
 *       true if successful; otherwise false and @error is set.
 *
 *       If the write concern does not dictate checking the result of the
 *       insert, then true may be returned even though the document was
 *       not actually inserted on the MongoDB server or cluster.
 *
 * Side effects:
 *       @collection->gle is setup, depending on write_concern->w value.
 *       @error may be set upon failure if non-NULL.
 *
 *--------------------------------------------------------------------------
 */

bool
mongoc_collection_insert (mongoc_collection_t          *collection,
                          mongoc_insert_flags_t         flags,
                          const bson_t                 *document,
                          const mongoc_write_concern_t *write_concern,
                          bson_error_t                 *error)
{
   mongoc_bulk_write_flags_t write_flags = MONGOC_BULK_WRITE_FLAGS_INIT;
   mongoc_write_command_t command;
   mongoc_write_result_t result;
   bool ret;

   ENTRY;

   BSON_ASSERT (collection);
   BSON_ASSERT (document);

   bson_clear (&collection->gle);

   if (!write_concern) {
      write_concern = collection->write_concern;
   }

   if (!(flags & MONGOC_INSERT_NO_VALIDATE)) {
      int vflags = (BSON_VALIDATE_UTF8 | BSON_VALIDATE_UTF8_ALLOW_NULL
                  | BSON_VALIDATE_DOLLAR_KEYS | BSON_VALIDATE_DOT_KEYS);

      if (!bson_validate (document, (bson_validate_flags_t)vflags, NULL)) {
         bson_set_error (error,
                         MONGOC_ERROR_BSON,
                         MONGOC_ERROR_BSON_INVALID,
                         "A document was corrupt or contained "
                         "invalid characters . or $");
         RETURN (false);
      }
   }

   _mongoc_write_result_init (&result);
   _mongoc_write_command_init_insert (&command, document, write_flags, false);

   _mongoc_collection_write_command_execute (&command, collection,
                                             write_concern, &result);

   collection->gle = bson_new ();
   ret = _mongoc_write_result_complete (&result, collection->gle, error);

   _mongoc_write_result_destroy (&result);
   _mongoc_write_command_destroy (&command);

   RETURN (ret);
}


/*
 *--------------------------------------------------------------------------
 *
 * mongoc_collection_update --
 *
 *       Updates one or more documents matching @selector with @update.
 *
 * Parameters:
 *       @collection: A mongoc_collection_t.
 *       @flags: The flags for the update.
 *       @selector: A bson_t containing your selector.
 *       @update: A bson_t containing your update document.
 *       @write_concern: The write concern or NULL.
 *       @error: A location for an error or NULL.
 *
 * Returns:
 *       true if successful; otherwise false and @error is set.
 *
 * Side effects:
 *       @collection->gle is setup, depending on write_concern->w value.
 *       @error is setup upon failure.
 *
 *--------------------------------------------------------------------------
 */

bool
mongoc_collection_update (mongoc_collection_t          *collection,
                          mongoc_update_flags_t         uflags,
                          const bson_t                 *selector,
                          const bson_t                 *update,
                          const mongoc_write_concern_t *write_concern,
                          bson_error_t                 *error)
{
   mongoc_bulk_write_flags_t write_flags = MONGOC_BULK_WRITE_FLAGS_INIT;
   mongoc_write_command_t command;
   mongoc_write_result_t result;
   bson_iter_t iter;
   size_t err_offset;
   bool ret;
   int vflags = (BSON_VALIDATE_UTF8 | BSON_VALIDATE_UTF8_ALLOW_NULL
               | BSON_VALIDATE_DOLLAR_KEYS | BSON_VALIDATE_DOT_KEYS);
   int flags = uflags;

   ENTRY;

   BSON_ASSERT (collection);
   BSON_ASSERT (selector);
   BSON_ASSERT (update);

   bson_clear (&collection->gle);

   if (!write_concern) {
      write_concern = collection->write_concern;
   }

   if (!((uint32_t)flags & MONGOC_UPDATE_NO_VALIDATE) &&
       bson_iter_init (&iter, update) &&
       bson_iter_next (&iter) &&
       (bson_iter_key (&iter) [0] != '$') &&
       !bson_validate (update, (bson_validate_flags_t)vflags, &err_offset)) {
      bson_set_error (error,
                      MONGOC_ERROR_BSON,
                      MONGOC_ERROR_BSON_INVALID,
                      "update document is corrupt or contains "
                      "invalid keys including $ or .");
      return false;
   } else {
      flags = (uint32_t)flags & ~MONGOC_UPDATE_NO_VALIDATE;
   }

   _mongoc_write_result_init (&result);
   _mongoc_write_command_init_update (&command,
                                      selector,
                                      update,
                                      !!(flags & MONGOC_UPDATE_UPSERT),
                                      !!(flags & MONGOC_UPDATE_MULTI_UPDATE),
                                      write_flags);

   _mongoc_collection_write_command_execute (&command, collection,
                                             write_concern, &result);

   collection->gle = bson_new ();
   ret = _mongoc_write_result_complete (&result, collection->gle, error);

   _mongoc_write_result_destroy (&result);
   _mongoc_write_command_destroy (&command);

   RETURN (ret);
}


/*
 *--------------------------------------------------------------------------
 *
 * mongoc_collection_save --
 *
 *       Save @document to @collection.
 *
 *       If the document has an _id field, it will be updated. Otherwise,
 *       the document will be inserted into the collection.
 *
 * Returns:
 *       true if successful; otherwise false and @error is set.
 *
 * Side effects:
 *       @error is set upon failure if non-NULL.
 *
 *--------------------------------------------------------------------------
 */

bool
mongoc_collection_save (mongoc_collection_t          *collection,
                        const bson_t                 *document,
                        const mongoc_write_concern_t *write_concern,
                        bson_error_t                 *error)
{
   bson_iter_t iter;
   bool ret;
   bson_t selector;

   BSON_ASSERT (collection);
   BSON_ASSERT (document);

   if (!bson_iter_init_find(&iter, document, "_id")) {
      return mongoc_collection_insert(collection,
                                      MONGOC_INSERT_NONE,
                                      document,
                                      write_concern,
                                      error);
   }

   bson_init(&selector);
   if (!bson_append_iter(&selector, NULL, 0, &iter)) {
      bson_set_error (error,
            MONGOC_ERROR_COMMAND,
            MONGOC_ERROR_COMMAND_INVALID_ARG,
            "Failed to append bson to create update.");
      bson_destroy (&selector);
      return NULL;
   }

   ret = mongoc_collection_update(collection,
                                  MONGOC_UPDATE_UPSERT,
                                  &selector,
                                  document,
                                  write_concern,
                                  error);

   bson_destroy(&selector);

   return ret;
}


/*
 *--------------------------------------------------------------------------
 *
 * mongoc_collection_remove --
 *
 *       Delete one or more items from a collection. If you want to
 *       limit to a single delete, provided MONGOC_REMOVE_SINGLE_REMOVE
 *       for @flags.
 *
 * Parameters:
 *       @collection: A mongoc_collection_t.
 *       @flags: the delete flags or 0.
 *       @selector: A selector of documents to delete.
 *       @write_concern: A write concern or NULL. If NULL, the default
 *                       write concern for the collection will be used.
 *       @error: A location for an error or NULL.
 *
 * Returns:
 *       true if successful; otherwise false and error is set.
 *
 *       If the write concern does not dictate checking the result, this
 *       function may return true even if it failed.
 *
 * Side effects:
 *       @collection->gle is setup, depending on write_concern->w value.
 *       @error is setup upon failure.
 *
 *--------------------------------------------------------------------------
 */

bool
mongoc_collection_remove (mongoc_collection_t          *collection,
                          mongoc_remove_flags_t         flags,
                          const bson_t                 *selector,
                          const mongoc_write_concern_t *write_concern,
                          bson_error_t                 *error)
{
   mongoc_bulk_write_flags_t write_flags = MONGOC_BULK_WRITE_FLAGS_INIT;
   mongoc_write_command_t command;
   mongoc_write_result_t result;
   bool multi;
   bool ret;

   ENTRY;

   BSON_ASSERT (collection);
   BSON_ASSERT (selector);

   bson_clear (&collection->gle);

   if (!write_concern) {
      write_concern = collection->write_concern;
   }

   multi = !(flags & MONGOC_REMOVE_SINGLE_REMOVE);

   _mongoc_write_result_init (&result);
   _mongoc_write_command_init_delete (&command, selector, multi, write_flags);

   _mongoc_collection_write_command_execute (&command, collection,
                                             write_concern, &result);

   collection->gle = bson_new ();
   ret = _mongoc_write_result_complete (&result, collection->gle, error);

   _mongoc_write_result_destroy (&result);
   _mongoc_write_command_destroy (&command);

   RETURN (ret);
}


bool
mongoc_collection_delete (mongoc_collection_t          *collection,
                          mongoc_delete_flags_t         flags,
                          const bson_t                 *selector,
                          const mongoc_write_concern_t *write_concern,
                          bson_error_t                 *error)
{
   return mongoc_collection_remove (collection, (mongoc_remove_flags_t)flags,
                                    selector, write_concern, error);
}


/*
 *--------------------------------------------------------------------------
 *
 * mongoc_collection_get_read_prefs --
 *
 *       Fetch the default read preferences for the collection.
 *
 * Returns:
 *       A mongoc_read_prefs_t that should not be modified or freed.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

const mongoc_read_prefs_t *
mongoc_collection_get_read_prefs (const mongoc_collection_t *collection)
{
   BSON_ASSERT (collection);
   return collection->read_prefs;
}


/*
 *--------------------------------------------------------------------------
 *
 * mongoc_collection_set_read_prefs --
 *
 *       Sets the default read preferences for the collection instance.
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
mongoc_collection_set_read_prefs (mongoc_collection_t       *collection,
                                  const mongoc_read_prefs_t *read_prefs)
{
   BSON_ASSERT (collection);

   if (collection->read_prefs) {
      mongoc_read_prefs_destroy(collection->read_prefs);
      collection->read_prefs = NULL;
   }

   if (read_prefs) {
      collection->read_prefs = mongoc_read_prefs_copy(read_prefs);
   }
}


/*
 *--------------------------------------------------------------------------
 *
 * mongoc_collection_get_read_concern --
 *
 *       Fetches the default read concern for the collection instance.
 *
 * Returns:
 *       A mongoc_read_concern_t that should not be modified or freed.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

const mongoc_read_concern_t *
mongoc_collection_get_read_concern (const mongoc_collection_t *collection)
{
   BSON_ASSERT (collection);

   return collection->read_concern;
}


/*
 *--------------------------------------------------------------------------
 *
 * mongoc_collection_set_read_concern --
 *
 *       Sets the default read concern for the collection instance.
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
mongoc_collection_set_read_concern (mongoc_collection_t          *collection,
                                    const mongoc_read_concern_t *read_concern)
{
   BSON_ASSERT (collection);

   if (collection->read_concern) {
      mongoc_read_concern_destroy (collection->read_concern);
      collection->read_concern = NULL;
   }

   if (read_concern) {
      collection->read_concern = mongoc_read_concern_copy (read_concern);
   }
}


/*
 *--------------------------------------------------------------------------
 *
 * mongoc_collection_get_write_concern --
 *
 *       Fetches the default write concern for the collection instance.
 *
 * Returns:
 *       A mongoc_write_concern_t that should not be modified or freed.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

const mongoc_write_concern_t *
mongoc_collection_get_write_concern (const mongoc_collection_t *collection)
{
   BSON_ASSERT (collection);

   return collection->write_concern;
}


/*
 *--------------------------------------------------------------------------
 *
 * mongoc_collection_set_write_concern --
 *
 *       Sets the default write concern for the collection instance.
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
mongoc_collection_set_write_concern (mongoc_collection_t          *collection,
                                     const mongoc_write_concern_t *write_concern)
{
   BSON_ASSERT (collection);

   if (collection->write_concern) {
      mongoc_write_concern_destroy(collection->write_concern);
      collection->write_concern = NULL;
   }

   if (write_concern) {
      collection->write_concern = mongoc_write_concern_copy(write_concern);
   }
}


/*
 *--------------------------------------------------------------------------
 *
 * mongoc_collection_get_name --
 *
 *       Returns the name of the collection, excluding the database name.
 *
 * Returns:
 *       A string which should not be modified or freed.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

const char *
mongoc_collection_get_name (mongoc_collection_t *collection)
{
   BSON_ASSERT (collection);

   return collection->collection;
}


/*
 *--------------------------------------------------------------------------
 *
 * mongoc_collection_get_last_error --
 *
 *       Returns getLastError document, according to write_concern on last
 *       executed command for current collection instance.
 *
 * Returns:
 *       NULL or a bson_t that should not be modified or freed. This value
 *       is not guaranteed to be persistent between calls into the
 *       mongoc_collection_t instance, and therefore must be copied if
 *       you would like to keep it around.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

const bson_t *
mongoc_collection_get_last_error (const mongoc_collection_t *collection) /* IN */
{
   BSON_ASSERT (collection);

   return collection->gle;
}


/*
 *--------------------------------------------------------------------------
 *
 * mongoc_collection_validate --
 *
 *       Helper to call the validate command on the MongoDB server to
 *       validate the collection.
 *
 *       Options may be additional options, or NULL.
 *       Currently supported options are:
 *
 *          "full": Boolean
 *
 *       If full is true, then perform a more resource intensive
 *       validation.
 *
 *       The result is stored in reply.
 *
 * Returns:
 *       true if successful; otherwise false and @error is set.
 *
 * Side effects:
 *       @reply is set if successful.
 *       @error may be set.
 *
 *--------------------------------------------------------------------------
 */

bool
mongoc_collection_validate (mongoc_collection_t *collection, /* IN */
                            const bson_t        *options,    /* IN */
                            bson_t              *reply,      /* OUT */
                            bson_error_t        *error)      /* IN */
{
   bson_iter_t iter;
   bson_t cmd = BSON_INITIALIZER;
   bool ret;

   BSON_ASSERT (collection);

   if (options &&
       bson_iter_init_find (&iter, options, "full") &&
       !BSON_ITER_HOLDS_BOOL (&iter)) {
      bson_set_error (error,
                      MONGOC_ERROR_BSON,
                      MONGOC_ERROR_BSON_INVALID,
                      "'full' must be a boolean value.");
      return false;
   }

   bson_append_utf8 (&cmd, "validate", 8,
                     collection->collection,
                     collection->collectionlen);

   if (options) {
      bson_concat (&cmd, options);
   }

   ret = mongoc_collection_command_simple (collection, &cmd, NULL, reply, error);

   bson_destroy (&cmd);

   return ret;
}


/*
 *--------------------------------------------------------------------------
 *
 * mongoc_collection_rename --
 *
 *       Rename the collection to @new_name.
 *
 *       If @new_db is NULL, the same db will be used.
 *
 *       If @drop_target_before_rename is true, then a collection named
 *       @new_name will be dropped before renaming @collection to
 *       @new_name.
 *
 * Returns:
 *       true on success; false on failure and @error is set.
 *
 * Side effects:
 *       @error is set on failure.
 *
 *--------------------------------------------------------------------------
 */

bool
mongoc_collection_rename (mongoc_collection_t *collection,
                          const char          *new_db,
                          const char          *new_name,
                          bool                 drop_target_before_rename,
                          bson_error_t        *error)
{
   bson_t cmd = BSON_INITIALIZER;
   char newns [MONGOC_NAMESPACE_MAX + 1];
   bool ret;

   BSON_ASSERT (collection);
   BSON_ASSERT (new_name);

   if (!validate_name (new_name)) {
      bson_set_error (error,
                      MONGOC_ERROR_NAMESPACE,
                      MONGOC_ERROR_NAMESPACE_INVALID,
                      "\"%s\" is an invalid collection name.",
                      new_name);
      return false;
   }

   bson_snprintf (newns, sizeof newns, "%s.%s",
                  new_db ? new_db : collection->db,
                  new_name);

   BSON_APPEND_UTF8 (&cmd, "renameCollection", collection->ns);
   BSON_APPEND_UTF8 (&cmd, "to", newns);

   if (drop_target_before_rename) {
      BSON_APPEND_BOOL (&cmd, "dropTarget", true);
   }

   ret = mongoc_client_command_simple (collection->client, "admin",
                                       &cmd, NULL, NULL, error);

   if (ret) {
      if (new_db) {
         bson_snprintf (collection->db, sizeof collection->db, "%s", new_db);
      }

      bson_snprintf (collection->collection, sizeof collection->collection,
                     "%s", new_name);
      collection->collectionlen = (int) strlen (collection->collection);

      bson_snprintf (collection->ns, sizeof collection->ns,
                     "%s.%s", collection->db, new_name);
      collection->nslen = (int) strlen (collection->ns);
   }

   bson_destroy (&cmd);

   return ret;
}


/*
 *--------------------------------------------------------------------------
 *
 * mongoc_collection_stats --
 *
 *       Fetches statistics about the collection.
 *
 *       The result is stored in @stats, which should NOT be an initialized
 *       bson_t or a leak will occur.
 *
 *       @stats, @options, and @error are optional.
 *
 * Returns:
 *       true on success and @stats is set.
 *       false on failure and @error is set.
 *
 * Side effects:
 *       @stats and @error.
 *
 *--------------------------------------------------------------------------
 */

bool
mongoc_collection_stats (mongoc_collection_t *collection,
                         const bson_t        *options,
                         bson_t              *stats,
                         bson_error_t        *error)
{
   bson_iter_t iter;
   bson_t cmd = BSON_INITIALIZER;
   bool ret;

   BSON_ASSERT (collection);

   if (options &&
       bson_iter_init_find (&iter, options, "scale") &&
       !BSON_ITER_HOLDS_INT32 (&iter)) {
      bson_set_error (error,
                      MONGOC_ERROR_BSON,
                      MONGOC_ERROR_BSON_INVALID,
                      "'scale' must be an int32 value.");
      return false;
   }

   BSON_APPEND_UTF8 (&cmd, "collStats", collection->collection);

   if (options) {
      bson_concat (&cmd, options);
   }

   ret = mongoc_collection_command_simple (collection, &cmd, NULL, stats, error);

   bson_destroy (&cmd);

   return ret;
}


mongoc_bulk_operation_t *
mongoc_collection_create_bulk_operation (
      mongoc_collection_t          *collection,
      bool                          ordered,
      const mongoc_write_concern_t *write_concern)
{
   mongoc_bulk_write_flags_t write_flags = MONGOC_BULK_WRITE_FLAGS_INIT;
   BSON_ASSERT (collection);

   if (!write_concern) {
      write_concern = collection->write_concern;
   }

   write_flags.ordered = ordered;

   return _mongoc_bulk_operation_new (collection->client,
                                      collection->db,
                                      collection->collection,
                                      write_flags,
                                      write_concern);
}

/*
 *--------------------------------------------------------------------------
 *
 * mongoc_collection_find_and_modify_with_opts --
 *
 *       Find a document in @collection matching @query, applying @opts.
 *
 *       If @reply is not NULL, then the result document will be placed
 *       in reply and should be released with bson_destroy().
 *
 *       See http://docs.mongodb.org/manual/reference/command/findAndModify/
 *       for more information.
 *
 * Returns:
 *       true on success; false on failure.
 *
 * Side effects:
 *       reply is initialized.
 *       error is set if false is returned.
 *
 *--------------------------------------------------------------------------
 */
bool
mongoc_collection_find_and_modify_with_opts (mongoc_collection_t                 *collection,
                                             const bson_t                        *query,
                                             const mongoc_find_and_modify_opts_t *opts,
                                             bson_t                              *reply,
                                             bson_error_t                        *error)
{
   mongoc_cluster_t *cluster;
   mongoc_server_stream_t *server_stream;
   bson_iter_t iter;
   bson_iter_t inner;
   const char *name;
   bson_t reply_local;
   bool ret;
   bson_t command = BSON_INITIALIZER;

   ENTRY;

   BSON_ASSERT (collection);
   BSON_ASSERT (query);


   cluster = &collection->client->cluster;
   server_stream = mongoc_cluster_stream_for_writes (cluster, error);
   if (!server_stream) {
      bson_destroy (&command);
      RETURN (false);
   }

   name = mongoc_collection_get_name (collection);
   BSON_APPEND_UTF8 (&command, "findAndModify", name);
   BSON_APPEND_DOCUMENT (&command, "query", query);

   if (opts->sort) {
      BSON_APPEND_DOCUMENT (&command, "sort", opts->sort);
   }

   if (opts->update) {
      BSON_APPEND_DOCUMENT (&command, "update", opts->update);
   }

   if (opts->fields) {
      BSON_APPEND_DOCUMENT (&command, "fields", opts->fields);
   }

   if (opts->flags & MONGOC_FIND_AND_MODIFY_REMOVE) {
      BSON_APPEND_BOOL (&command, "remove", true);
   }

   if (opts->flags & MONGOC_FIND_AND_MODIFY_UPSERT) {
      BSON_APPEND_BOOL (&command, "upsert", true);
   }

   if (opts->flags & MONGOC_FIND_AND_MODIFY_RETURN_NEW) {
      BSON_APPEND_BOOL (&command, "new", true);
   }

   if (opts->bypass_document_validation != MONGOC_BYPASS_DOCUMENT_VALIDATION_DEFAULT) {
      BSON_APPEND_BOOL (&command, "bypassDocumentValidation",
                        !!opts->bypass_document_validation);
   }

   if (server_stream->sd->max_wire_version >= WIRE_VERSION_FAM_WRITE_CONCERN) {
      if (!_mongoc_write_concern_is_valid (collection->write_concern)) {
         bson_set_error (error,
                         MONGOC_ERROR_COMMAND,
                         MONGOC_ERROR_COMMAND_INVALID_ARG,
                         "The write concern is invalid.");
         bson_destroy (&command);
         mongoc_server_stream_cleanup (server_stream);
         RETURN (false);
      }

      if (_mongoc_write_concern_needs_gle (collection->write_concern)) {
         _BSON_APPEND_WRITE_CONCERN (&command, collection->write_concern);
      }
   }

   ret = mongoc_cluster_run_command (cluster, server_stream->stream,
                                     server_stream->sd->id,
                                     MONGOC_QUERY_NONE, collection->db,
                                     &command, &reply_local, error);

   if (bson_iter_init_find (&iter, &reply_local, "writeConcernError") &&
         BSON_ITER_HOLDS_DOCUMENT (&iter)) {
      const char *errmsg = NULL;
      int32_t code = 0;

      bson_iter_recurse(&iter, &inner);
      while (bson_iter_next (&inner)) {
         if (BSON_ITER_IS_KEY (&inner, "code")) {
            code = bson_iter_int32 (&inner);
         } else if (BSON_ITER_IS_KEY (&inner, "errmsg")) {
            errmsg = bson_iter_utf8 (&inner, NULL);
         }
      }
      bson_set_error (error, MONGOC_ERROR_WRITE_CONCERN, code, "Write Concern error: %s", errmsg);
   }
   if (reply) {
      bson_copy_to (&reply_local, reply);
   }

   bson_destroy (&reply_local);
   bson_destroy (&command);
   mongoc_server_stream_cleanup (server_stream);

   RETURN (ret);
}

/*
 *--------------------------------------------------------------------------
 *
 * mongoc_collection_find_and_modify --
 *
 *       Find a document in @collection matching @query and update it with
 *       the update document @update.
 *
 *       If @reply is not NULL, then the result document will be placed
 *       in reply and should be released with bson_destroy().
 *
 *       If @remove is true, then the matching documents will be removed.
 *
 *       If @fields is not NULL, it will be used to select the desired
 *       resulting fields.
 *
 *       If @_new is true, then the new version of the document is returned
 *       instead of the old document.
 *
 *       See http://docs.mongodb.org/manual/reference/command/findAndModify/
 *       for more information.
 *
 * Returns:
 *       true on success; false on failure.
 *
 * Side effects:
 *       reply is initialized.
 *       error is set if false is returned.
 *
 *--------------------------------------------------------------------------
 */

bool
mongoc_collection_find_and_modify (mongoc_collection_t *collection,
                                   const bson_t        *query,
                                   const bson_t        *sort,
                                   const bson_t        *update,
                                   const bson_t        *fields,
                                   bool                 _remove,
                                   bool                 upsert,
                                   bool                 _new,
                                   bson_t              *reply,
                                   bson_error_t        *error)
{
   mongoc_find_and_modify_opts_t *opts;
   int flags = 0;
   bool ret;

   ENTRY;

   BSON_ASSERT (collection);
   BSON_ASSERT (query);
   BSON_ASSERT (update || _remove);


   if (_remove) {
      flags |= MONGOC_FIND_AND_MODIFY_REMOVE;
   }
   if (upsert) {
      flags |= MONGOC_FIND_AND_MODIFY_UPSERT;
   }
   if (_new) {
      flags |= MONGOC_FIND_AND_MODIFY_RETURN_NEW;
   }

   opts = mongoc_find_and_modify_opts_new ();

   mongoc_find_and_modify_opts_set_sort (opts, sort);
   mongoc_find_and_modify_opts_set_update (opts, update);
   mongoc_find_and_modify_opts_set_fields (opts, fields);
   mongoc_find_and_modify_opts_set_flags (opts, flags);

   ret = mongoc_collection_find_and_modify_with_opts (collection, query, opts, reply, error);
   mongoc_find_and_modify_opts_destroy (opts);

   return ret;
}
