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


#include "mongoc-cursor.h"
#include "mongoc-cursor-private.h"
#include "mongoc-client-private.h"
#include "mongoc-counters-private.h"
#include "mongoc-error.h"
#include "mongoc-log.h"
#include "mongoc-trace.h"
#include "mongoc-cursor-cursorid-private.h"
#include "mongoc-read-concern-private.h"
#include "mongoc-util-private.h"


#undef MONGOC_LOG_DOMAIN
#define MONGOC_LOG_DOMAIN "cursor"


#define CURSOR_FAILED(cursor_) ((cursor_)->error.domain != 0)

static const bson_t *
_mongoc_cursor_op_query (mongoc_cursor_t        *cursor,
                         mongoc_server_stream_t *server_stream);

static const bson_t *
_mongoc_cursor_find_command (mongoc_cursor_t *cursor);


static int32_t
_mongoc_n_return (mongoc_cursor_t * cursor)
{
   if (cursor->is_command) {
      /* commands always have n_return of 1 */
      return 1;
   } else if (cursor->limit < 0) {
      return cursor->limit;
   } else if (cursor->limit) {
      int32_t remaining = cursor->limit - cursor->count;
      BSON_ASSERT (remaining > 0);

      if (cursor->batch_size) {
         return BSON_MIN ((int32_t) cursor->batch_size, remaining);
      } else {
         /* batch_size 0 means accept the default */
         return remaining;
      }
   } else {
      return cursor->batch_size;
   }
}

mongoc_cursor_t *
_mongoc_cursor_new (mongoc_client_t           *client,
                    const char                *db_and_collection,
                    mongoc_query_flags_t       qflags,
                    uint32_t                   skip,
                    int32_t                    limit,
                    uint32_t                   batch_size,
                    bool                       is_command,
                    const bson_t              *query,
                    const bson_t              *fields,
                    const mongoc_read_prefs_t *read_prefs,
                    const mongoc_read_concern_t *read_concern)
{
   mongoc_cursor_t *cursor;
   bson_iter_t iter;
   int flags = qflags;
   const char *dot;

   ENTRY;

   BSON_ASSERT (client);
   BSON_ASSERT (db_and_collection);

   if (!read_concern) {
      read_concern = client->read_concern;
   }
   if (!read_prefs) {
      read_prefs = client->read_prefs;
   }

   cursor = (mongoc_cursor_t *)bson_malloc0 (sizeof *cursor);

   /*
    * Cursors execute their query lazily. This sadly means that we must copy
    * some extra data around between the bson_t structures. This should be
    * small in most cases, so it reduces to a pure memcpy. The benefit to this
    * design is simplified error handling by API consumers.
    */

   cursor->client = client;
   bson_strncpy (cursor->ns, db_and_collection, sizeof cursor->ns);

   cursor->nslen = (uint32_t)bson_strnlen (cursor->ns, sizeof cursor->ns);
   dot = strstr (db_and_collection, ".");

   if (dot) {
      cursor->dblen = (uint32_t)(dot - db_and_collection);
   } else {
      /* a database name with no collection name */
      cursor->dblen = cursor->nslen;
   }

   cursor->flags = (mongoc_query_flags_t)flags;
   cursor->skip = skip;
   cursor->limit = limit;
   cursor->batch_size = batch_size;
   cursor->is_command = is_command;
   cursor->has_fields = !!fields;

#define MARK_FAILED(c) \
   do { \
      bson_init (&(c)->query); \
      bson_init (&(c)->fields); \
      (c)->done = true; \
      (c)->end_of_event = true; \
      (c)->sent = true; \
   } while (0)

   /* we can't have exhaust queries with limits */
   if ((flags & MONGOC_QUERY_EXHAUST) && limit) {
      bson_set_error (&cursor->error,
                      MONGOC_ERROR_CURSOR,
                      MONGOC_ERROR_CURSOR_INVALID_CURSOR,
                      "Cannot specify MONGOC_QUERY_EXHAUST and set a limit.");
      MARK_FAILED (cursor);
      GOTO (finish);
   }

   /* we can't have exhaust queries with sharded clusters */
   if ((flags & MONGOC_QUERY_EXHAUST) &&
       (client->topology->description.type == MONGOC_TOPOLOGY_SHARDED)) {
      bson_set_error (&cursor->error,
                      MONGOC_ERROR_CURSOR,
                      MONGOC_ERROR_CURSOR_INVALID_CURSOR,
                      "Cannot specify MONGOC_QUERY_EXHAUST with sharded cluster.");
      MARK_FAILED (cursor);
      GOTO (finish);
   }

   /*
    * Check types of various optional parameters.
    */
   if (query && !is_command) {
      if (bson_iter_init_find (&iter, query, "$explain") &&
          !(BSON_ITER_HOLDS_BOOL (&iter) || BSON_ITER_HOLDS_INT32 (&iter))) {
         bson_set_error (&cursor->error,
                         MONGOC_ERROR_CURSOR,
                         MONGOC_ERROR_CURSOR_INVALID_CURSOR,
                         "$explain must be a boolean.");
         MARK_FAILED (cursor);
         GOTO (finish);
      }

      if (bson_iter_init_find (&iter, query, "$snapshot") &&
          !BSON_ITER_HOLDS_BOOL (&iter) &&
          !BSON_ITER_HOLDS_INT32 (&iter)) {
         bson_set_error (&cursor->error,
                         MONGOC_ERROR_CURSOR,
                         MONGOC_ERROR_CURSOR_INVALID_CURSOR,
                         "$snapshot must be a boolean.");
         MARK_FAILED (cursor);
         GOTO (finish);
      }
   }

   /*
    * Check if we have a mixed top-level query and dollar keys such
    * as $orderby. This is not allowed (you must use {$query:{}}.
    */
   if (query && bson_iter_init (&iter, query)) {
      bool found_dollar = false;
      bool found_non_dollar = false;

      while (bson_iter_next (&iter)) {
         if (bson_iter_key (&iter)[0] == '$') {
            found_dollar = true;
         } else {
            found_non_dollar = true;
         }
      }

      if (found_dollar && found_non_dollar) {
         bson_set_error (&cursor->error,
                         MONGOC_ERROR_CURSOR,
                         MONGOC_ERROR_CURSOR_INVALID_CURSOR,
                         "Cannot mix top-level query with dollar keys such "
                         "as $orderby. Use {$query: {},...} instead.");
         MARK_FAILED (cursor);
         GOTO (finish);
      }
   }

   /* don't use MARK_FAILED after this, you'll leak cursor->query */
   if (query) {
      bson_copy_to(query, &cursor->query);
   } else {
      bson_init(&cursor->query);
   }

   if (fields) {
      bson_copy_to(fields, &cursor->fields);
   } else {
      bson_init(&cursor->fields);
   }

   if (read_prefs) {
      cursor->read_prefs = mongoc_read_prefs_copy (read_prefs);
   }

   if (read_concern) {
      cursor->read_concern = mongoc_read_concern_copy (read_concern);
   }

   _mongoc_buffer_init(&cursor->buffer, NULL, 0, NULL, NULL);

finish:
   mongoc_counter_cursors_active_inc();

   RETURN (cursor);
}


void
mongoc_cursor_destroy (mongoc_cursor_t *cursor)
{
   ENTRY;

   BSON_ASSERT(cursor);

   if (cursor->iface.destroy) {
      cursor->iface.destroy(cursor);
   } else {
      _mongoc_cursor_destroy(cursor);
   }

   EXIT;
}

void
_mongoc_cursor_destroy (mongoc_cursor_t *cursor)
{
   char db[MONGOC_NAMESPACE_MAX];
   ENTRY;

   BSON_ASSERT (cursor);

   if (cursor->in_exhaust) {
      cursor->client->in_exhaust = false;
      if (!cursor->done) {
         /* The only way to stop an exhaust cursor is to kill the connection */
         mongoc_cluster_disconnect_node (&cursor->client->cluster,
                                         cursor->hint);
      }
   } else if (cursor->rpc.reply.cursor_id) {
      bson_strncpy (db, cursor->ns, cursor->dblen + 1);

      _mongoc_client_kill_cursor(cursor->client,
                                 cursor->hint,
                                 cursor->rpc.reply.cursor_id,
                                 db,
                                 cursor->ns + cursor->dblen + 1);
   }

   if (cursor->reader) {
      bson_reader_destroy(cursor->reader);
      cursor->reader = NULL;
   }

   bson_destroy(&cursor->query);
   bson_destroy(&cursor->fields);
   _mongoc_buffer_destroy(&cursor->buffer);
   mongoc_read_prefs_destroy(cursor->read_prefs);
   mongoc_read_concern_destroy(cursor->read_concern);

   bson_free(cursor);

   mongoc_counter_cursors_active_dec();
   mongoc_counter_cursors_disposed_inc();

   EXIT;
}


mongoc_server_stream_t *
_mongoc_cursor_fetch_stream (mongoc_cursor_t *cursor)
{
   mongoc_server_stream_t *server_stream;

   ENTRY;

   if (cursor->hint) {
      server_stream = mongoc_cluster_stream_for_server (&cursor->client->cluster,
                                                        cursor->hint,
                                                        true /* reconnect_ok */,
                                                        &cursor->error);
   } else {
      server_stream = mongoc_cluster_stream_for_reads (&cursor->client->cluster,
                                                       cursor->read_prefs,
                                                       &cursor->error);

      if (server_stream) {
         cursor->hint = server_stream->sd->id;
      }
   }

   RETURN (server_stream);
}


bool
_use_find_command (const mongoc_cursor_t *cursor,
                   const mongoc_server_stream_t *server_stream)
{
   /* Find, getMore And killCursors Commands Spec: "the find command cannot be
    * used to execute other commands" and "the find command does not support the
    * exhaust flag."
    */
   return server_stream->sd->max_wire_version >= WIRE_VERSION_FIND_CMD &&
          !cursor->is_command &&
          !(cursor->flags & MONGOC_QUERY_EXHAUST);
}


static const bson_t *
_mongoc_cursor_initial_query (mongoc_cursor_t *cursor)
{
   mongoc_server_stream_t *server_stream;
   const bson_t *b = NULL;

   ENTRY;

   BSON_ASSERT (cursor);

   server_stream = _mongoc_cursor_fetch_stream (cursor);

   if (!server_stream) {
      GOTO (done);
   }

   if (_use_find_command (cursor, server_stream)) {
      b = _mongoc_cursor_find_command (cursor);
   } else {
      /* When the user explicitly provides a readConcern -- but the server
       * doesn't support readConcern, we must error:
       * https://github.com/mongodb/specifications/blob/master/source/read-write-concern/read-write-concern.rst#errors-1
       */
      if (cursor->read_concern->level != NULL
          && server_stream->sd->max_wire_version < WIRE_VERSION_READ_CONCERN) {
         bson_set_error (&cursor->error,
                         MONGOC_ERROR_COMMAND,
                         MONGOC_ERROR_PROTOCOL_BAD_WIRE_VERSION,
                         "The selected server does not support readConcern");
      } else {
         b = _mongoc_cursor_op_query (cursor, server_stream);
      }
   }

done:
   /* no-op if server_stream is NULL */
   mongoc_server_stream_cleanup (server_stream);

   if (!b) {
      cursor->done = true;
   }

   RETURN (b);
}


static const bson_t *
_mongoc_cursor_op_query (mongoc_cursor_t        *cursor,
                         mongoc_server_stream_t *server_stream)
{
   mongoc_apply_read_prefs_result_t result = READ_PREFS_RESULT_INIT;
   mongoc_rpc_t rpc;
   uint32_t request_id;
   const bson_t *bson = NULL;

   ENTRY;

   cursor->sent = true;

   rpc.query.msg_len = 0;
   rpc.query.request_id = 0;
   rpc.query.response_to = 0;
   rpc.query.opcode = MONGOC_OPCODE_QUERY;
   rpc.query.flags = cursor->flags;
   rpc.query.collection = cursor->ns;
   rpc.query.skip = cursor->skip;
   if ((cursor->flags & MONGOC_QUERY_TAILABLE_CURSOR)) {
      rpc.query.n_return = 0;
   } else {
      rpc.query.n_return = _mongoc_n_return(cursor);
   }

   if (cursor->has_fields) {
      rpc.query.fields = bson_get_data (&cursor->fields);
   } else {
      rpc.query.fields = NULL;
   }

   apply_read_preferences (cursor->read_prefs, server_stream,
                           &cursor->query, cursor->flags, &result);

   rpc.query.query = bson_get_data (result.query_with_read_prefs);
   rpc.query.flags = result.flags;

   if (!mongoc_cluster_sendv_to_server (&cursor->client->cluster,
                                        &rpc, 1, server_stream,
                                        NULL, &cursor->error)) {
      GOTO (failure);
   }

   request_id = BSON_UINT32_FROM_LE (rpc.header.request_id);

   _mongoc_buffer_clear(&cursor->buffer, false);

   if (!_mongoc_client_recv(cursor->client,
                            &cursor->rpc,
                            &cursor->buffer,
                            server_stream,
                            &cursor->error)) {
      GOTO (failure);
   }

   if (cursor->rpc.header.opcode != MONGOC_OPCODE_REPLY) {
      bson_set_error (&cursor->error,
                      MONGOC_ERROR_PROTOCOL,
                      MONGOC_ERROR_PROTOCOL_INVALID_REPLY,
                      "Invalid opcode. Expected %d, got %d.",
                      MONGOC_OPCODE_REPLY, cursor->rpc.header.opcode);
      GOTO (failure);
   }

   if (cursor->rpc.header.response_to != request_id) {
      bson_set_error (&cursor->error,
                      MONGOC_ERROR_PROTOCOL,
                      MONGOC_ERROR_PROTOCOL_INVALID_REPLY,
                      "Invalid response_to for query. Expected %d, got %d.",
                      request_id, cursor->rpc.header.response_to);
      GOTO (failure);
   }

   if (cursor->is_command) {
       if (_mongoc_rpc_parse_command_error (&cursor->rpc,
                                            &cursor->error)) {
          GOTO (failure);
       }
   } else {
      if (_mongoc_rpc_parse_query_error (&cursor->rpc,
                                         &cursor->error)) {
         GOTO (failure);
      }
   }

   if (cursor->reader) {
      bson_reader_destroy (cursor->reader);
   }

   cursor->reader = bson_reader_new_from_data(
      cursor->rpc.reply.documents,
      (size_t) cursor->rpc.reply.documents_len);

   if ((cursor->flags & MONGOC_QUERY_EXHAUST)) {
      cursor->in_exhaust = true;
      cursor->client->in_exhaust = true;
   }

   cursor->done = false;
   cursor->end_of_event = false;
   cursor->sent = true;

   _mongoc_read_from_buffer (cursor, &bson);

   apply_read_prefs_result_cleanup (&result);

   RETURN (bson);

failure:
   cursor->done = true;

   apply_read_prefs_result_cleanup (&result);

   RETURN (false);
}


bool
_mongoc_cursor_run_command (mongoc_cursor_t *cursor,
                            const bson_t    *command)
{
   mongoc_cluster_t *cluster;
   mongoc_server_stream_t *server_stream;
   char cmd_ns[MONGOC_NAMESPACE_MAX];
   mongoc_apply_read_prefs_result_t read_prefs_result = READ_PREFS_RESULT_INIT;
   bool ret = false;
   bson_t bson;
   mongoc_rpc_t rpc;

   ENTRY;

   cluster = &cursor->client->cluster;

   server_stream = _mongoc_cursor_fetch_stream (cursor);

   if (!server_stream) {
      GOTO (done);
   }

   _mongoc_buffer_clear (&cursor->buffer, false);

   bson_snprintf (cmd_ns, sizeof cmd_ns, "%.*s.$cmd", cursor->dblen,
                  cursor->ns);

   apply_read_preferences (cursor->read_prefs, server_stream,
                           command, cursor->flags, &read_prefs_result);

   _mongoc_rpc_prep_command (&rpc,
                             cmd_ns,
                             read_prefs_result.query_with_read_prefs,
                             read_prefs_result.flags);

   if (!mongoc_cluster_run_command_rpc (cluster, server_stream->stream,
                                        server_stream->sd->id,
                                        _mongoc_get_command_name (&cursor->query),
                                        &rpc, &cursor->rpc, &cursor->buffer,
                                        &cursor->error)) {
      GOTO (done);
   }

   /* static-init "bson" to point into buffer */
   if (!_mongoc_rpc_reply_get_first (&cursor->rpc.reply, &bson)) {
      bson_set_error (&cursor->error,
                      MONGOC_ERROR_BSON,
                      MONGOC_ERROR_BSON_INVALID,
                      "Failed to decode reply BSON document.");
      GOTO (done);
   }

   if (_mongoc_rpc_parse_command_error (&cursor->rpc, &cursor->error)) {
      GOTO (done);
   }

   if (cursor->reader) {
      bson_reader_destroy (cursor->reader);
   }

   cursor->reader = bson_reader_new_from_data (
      cursor->rpc.reply.documents,
      (size_t)cursor->rpc.reply.documents_len);

   ret = true;

done:
   apply_read_prefs_result_cleanup (&read_prefs_result);
   mongoc_server_stream_cleanup (server_stream);

   return ret;
}


static bool
_invalid_field (const char      *query_field,
                mongoc_cursor_t *cursor)
{
   if (query_field[0] == '\0') {
      bson_set_error (&cursor->error,
                      MONGOC_ERROR_CURSOR,
                      MONGOC_ERROR_CURSOR_INVALID_CURSOR,
                      "empty string is not a valid query operator");
      return true;
   }

   return false;
}


static bool
_translate_query_opt (const char *query_field,
                      const char **cmd_field,
                      int *len)
{
   if (query_field[0] != '$') {
      *cmd_field = query_field;
      *len = -1;
      return true;
   }

   /* strip the leading '$' */
   query_field++;

   if (!strcmp ("query", query_field)) {
      *cmd_field = "filter";
      *len = 6;
   } else if (!strcmp ("orderby", query_field)) {
      *cmd_field = "sort";
      *len = 4;
   } else if (!strcmp ("showDiskLoc", query_field)) { /* <= MongoDb 3.0 */
      *cmd_field = "showRecordId";
      *len = 12;
   } else if (!strcmp("hint", query_field)) {
      *cmd_field = "hint";
      *len = 4;
   } else if (!strcmp("comment", query_field)) {
      *cmd_field = "comment";
      *len = 7;
   } else if (!strcmp("maxScan", query_field)) {
      *cmd_field = "maxScan";
      *len = 7;
   } else if (!strcmp("maxTimeMS", query_field)) {
      *cmd_field = "maxTimeMS";
      *len = 9;
   } else if (!strcmp("max", query_field)) {
      *cmd_field = "max";
      *len = 3;
   } else if (!strcmp("min", query_field)) {
      *cmd_field = "min";
      *len = 3;
   } else if (!strcmp("returnKey", query_field)) {
      *cmd_field = "returnKey";
      *len = 9;
   } else if (!strcmp("snapshot", query_field)) {
      *cmd_field = "snapshot";
      *len = 8;
   } else {
      /* not a special command field, must be a query operator like $or */
      return false;
   }

   return true;
}


static void
_mongoc_cursor_prepare_find_command_flags (mongoc_cursor_t *cursor,
                                           bson_t          *command)
{
   mongoc_query_flags_t flags = cursor->flags;

   if (flags & MONGOC_QUERY_TAILABLE_CURSOR) {
      bson_append_bool (command, "tailable", 8, true);
   }

   if (flags & MONGOC_QUERY_OPLOG_REPLAY) {
      bson_append_bool (command, "oplogReplay", 11, true);
   }

   if (flags & MONGOC_QUERY_NO_CURSOR_TIMEOUT) {
      bson_append_bool (command, "noCursorTimeout", 15, true);
   }

   if (flags & MONGOC_QUERY_AWAIT_DATA) {
      bson_append_bool (command, "awaitData", 9, true);
   }

   if (flags & MONGOC_QUERY_PARTIAL) {
      bson_append_bool (command, "allowPartialResults", 19, true);
   }
}


void
_mongoc_cursor_collection (const mongoc_cursor_t *cursor,
                           const char **collection,
                           int *collection_len)
{
   /* ns is like "db.collection". Collection name is located past the ".". */
   *collection = cursor->ns + (cursor->dblen + 1);
   /* Collection name's length is ns length, minus length of db name and ".". */
   *collection_len = cursor->nslen - cursor->dblen - 1;

   BSON_ASSERT (*collection_len > 0);
}


static bool
_mongoc_cursor_prepare_find_command (mongoc_cursor_t *cursor,
                                     bson_t          *command)
{
   const char *collection;
   int collection_len;
   bson_iter_t iter;
   const char *command_field;
   int len;
   const bson_value_t *value;

   _mongoc_cursor_collection (cursor, &collection, &collection_len);
   bson_append_utf8 (command, "find", 4, collection, collection_len);

   if (bson_empty0 (&cursor->query)) {
      /* Find, getMore And killCursors Commands Spec: filter "MUST be included
       * in the command".
       */
      bson_t empty = BSON_INITIALIZER;
      bson_append_document (command, "filter", 6, &empty);
   } else if (bson_has_field (&cursor->query, "$query")) {
      bson_iter_init (&iter, &cursor->query);
      while (bson_iter_next (&iter)) {
         if (_invalid_field (bson_iter_key (&iter), cursor)) {
            return false;
         }

         value = bson_iter_value (&iter);
         if (_translate_query_opt (bson_iter_key (&iter),
                                   &command_field,
                                   &len)) {
            bson_append_value (command, command_field, len, value);
         } else {
            bson_append_value (command, bson_iter_key (&iter), -1, value);
         }
      }
   } else if (bson_has_field (&cursor->query, "filter")) {
      bson_concat (command, &cursor->query);
   } else {
      /* cursor->query has no "$query", use it as the filter */
      bson_append_document (command, "filter", 6, &cursor->query);
   }

   if (!bson_empty0 (&cursor->fields)) {
      bson_append_document (command, "projection", 10, &cursor->fields);
   }

   if (cursor->skip) {
      bson_append_int64 (command, "skip", 4, cursor->skip);
   }

   if (cursor->limit) {
      if (cursor->limit < 0) {
         bson_append_bool (command, "singleBatch", 11, true);
      }

      bson_append_int64 (command, "limit", 5, labs(cursor->limit));
   }

   if (cursor->batch_size) {
      bson_append_int32 (command, "batchSize", 9, cursor->batch_size);
   }

   if (cursor->read_concern->level != NULL) {
      const bson_t *read_concern_bson;

      read_concern_bson = _mongoc_read_concern_get_bson (cursor->read_concern);
      BSON_APPEND_DOCUMENT (command, "readConcern", read_concern_bson);
   }

   _mongoc_cursor_prepare_find_command_flags (cursor, command);

   return true;
}


static const bson_t *
_mongoc_cursor_find_command (mongoc_cursor_t *cursor)
{
   bson_t command = BSON_INITIALIZER;
   const bson_t *bson = NULL;

   ENTRY;

   if (!_mongoc_cursor_prepare_find_command (cursor, &command)) {
      RETURN (NULL);
   }

   _mongoc_cursor_cursorid_init (cursor, &command);
   bson_destroy (&command);

   BSON_ASSERT (cursor->iface.next);
   _mongoc_cursor_cursorid_next (cursor, &bson);

   RETURN (bson);
}


static const bson_t *
_mongoc_cursor_get_more (mongoc_cursor_t *cursor)
{
   mongoc_server_stream_t *server_stream;
   const bson_t *b = NULL;

   ENTRY;

   BSON_ASSERT (cursor);

   server_stream = _mongoc_cursor_fetch_stream (cursor);
   if (!server_stream) {
      GOTO (failure);
   }

   if (!cursor->in_exhaust && !cursor->rpc.reply.cursor_id) {
      bson_set_error (&cursor->error,
                      MONGOC_ERROR_CURSOR,
                      MONGOC_ERROR_CURSOR_INVALID_CURSOR,
                      "No valid cursor was provided.");
      GOTO (failure);
   }

   if (!_mongoc_cursor_op_getmore (cursor, server_stream)) {
      GOTO (failure);
   }

   mongoc_server_stream_cleanup (server_stream);

   if (cursor->reader) {
      _mongoc_read_from_buffer (cursor, &b);
   }

   RETURN (b);

failure:
   cursor->done = true;

   mongoc_server_stream_cleanup (server_stream);

   RETURN (NULL);
}


bool
_mongoc_cursor_op_getmore (mongoc_cursor_t        *cursor,
                           mongoc_server_stream_t *server_stream)
{
   mongoc_rpc_t rpc;
   uint32_t request_id;
   bool ret = false;

   ENTRY;

   if (cursor->in_exhaust) {
      request_id = (uint32_t) cursor->rpc.header.request_id;
   } else {
      rpc.get_more.cursor_id = cursor->rpc.reply.cursor_id;
      rpc.get_more.msg_len = 0;
      rpc.get_more.request_id = 0;
      rpc.get_more.response_to = 0;
      rpc.get_more.opcode = MONGOC_OPCODE_GET_MORE;
      rpc.get_more.zero = 0;
      rpc.get_more.collection = cursor->ns;
      if ((cursor->flags & MONGOC_QUERY_TAILABLE_CURSOR)) {
         rpc.get_more.n_return = 0;
      } else {
         rpc.get_more.n_return = _mongoc_n_return(cursor);
      }

      if (!mongoc_cluster_sendv_to_server (&cursor->client->cluster,
                                           &rpc, 1, server_stream,
                                           NULL, &cursor->error)) {
         GOTO (done);
      }

      request_id = BSON_UINT32_FROM_LE (rpc.header.request_id);
   }

   _mongoc_buffer_clear (&cursor->buffer, false);

   if (!_mongoc_client_recv (cursor->client,
                             &cursor->rpc,
                             &cursor->buffer,
                             server_stream,
                             &cursor->error)) {
      GOTO (done);
   }

   if (cursor->rpc.header.opcode != MONGOC_OPCODE_REPLY) {
      bson_set_error (&cursor->error,
                      MONGOC_ERROR_PROTOCOL,
                      MONGOC_ERROR_PROTOCOL_INVALID_REPLY,
                      "Invalid opcode. Expected %d, got %d.",
                      MONGOC_OPCODE_REPLY, cursor->rpc.header.opcode);
      GOTO (done);
   }

   if (cursor->rpc.header.response_to != request_id) {
      bson_set_error (&cursor->error,
                      MONGOC_ERROR_PROTOCOL,
                      MONGOC_ERROR_PROTOCOL_INVALID_REPLY,
                      "Invalid response_to for getmore. Expected %d, got %d.",
                      request_id, cursor->rpc.header.response_to);
      GOTO (done);
   }

   if (_mongoc_rpc_parse_query_error (&cursor->rpc,
                                      &cursor->error)) {
      GOTO (done);
   }

   if (cursor->reader) {
      bson_reader_destroy (cursor->reader);
   }

   cursor->reader = bson_reader_new_from_data (
      cursor->rpc.reply.documents,
      (size_t)cursor->rpc.reply.documents_len);

   ret = true;

done:
   RETURN (ret);
}


bool
mongoc_cursor_error (mongoc_cursor_t *cursor,
                     bson_error_t    *error)
{
   bool ret;

   ENTRY;

   BSON_ASSERT(cursor);

   if (cursor->iface.error) {
      ret = cursor->iface.error(cursor, error);
   } else {
      ret = _mongoc_cursor_error(cursor, error);
   }

   RETURN(ret);
}


bool
_mongoc_cursor_error (mongoc_cursor_t *cursor,
                      bson_error_t    *error)
{
   ENTRY;

   BSON_ASSERT (cursor);

   if (BSON_UNLIKELY(CURSOR_FAILED (cursor))) {
      bson_set_error(error,
                     cursor->error.domain,
                     cursor->error.code,
                     "%s",
                     cursor->error.message);
      RETURN(true);
   }

   RETURN(false);
}


bool
mongoc_cursor_next (mongoc_cursor_t  *cursor,
                    const bson_t    **bson)
{
   bool ret;

   ENTRY;

   BSON_ASSERT(cursor);
   BSON_ASSERT(bson);

   TRACE ("cursor_id(%"PRId64")", cursor->rpc.reply.cursor_id);

   if (bson) {
      *bson = NULL;
   }

   if (CURSOR_FAILED (cursor)) {
      return false;
   }

   /*
    * We cannot proceed if another cursor is receiving results in exhaust mode.
    */
   if (cursor->client->in_exhaust && !cursor->in_exhaust) {
      bson_set_error (&cursor->error,
                      MONGOC_ERROR_CLIENT,
                      MONGOC_ERROR_CLIENT_IN_EXHAUST,
                      "Another cursor derived from this client is in exhaust.");
      RETURN (false);
   }

   if (cursor->iface.next) {
      ret = cursor->iface.next(cursor, bson);
   } else {
      ret = _mongoc_cursor_next(cursor, bson);
   }

   cursor->current = *bson;

   cursor->count++;

   RETURN(ret);
}


bool
_mongoc_read_from_buffer (mongoc_cursor_t *cursor,
                          const bson_t   **bson)
{
   bool eof = false;

   BSON_ASSERT (cursor->reader);

   *bson = bson_reader_read (cursor->reader, &eof);
   cursor->end_of_event = eof ? 1 : 0;

   return *bson ? true : false;
}


bool
_mongoc_cursor_next (mongoc_cursor_t  *cursor,
                     const bson_t    **bson)
{
   const bson_t *b = NULL;

   ENTRY;

   BSON_ASSERT (cursor);

   if (bson) {
      *bson = NULL;
   }

   if (cursor->done || CURSOR_FAILED (cursor)) {
      bson_set_error (&cursor->error,
                      MONGOC_ERROR_CURSOR,
                      MONGOC_ERROR_CURSOR_INVALID_CURSOR,
                      "Cannot advance a completed or failed cursor.");
      RETURN (false);
   }

   /*
    * If we reached our limit, make sure we mark this as done and do not try to
    * make further progress.
    */
   if (cursor->limit && cursor->count >= labs(cursor->limit)) {
      cursor->done = true;
      RETURN (false);
   }

   /*
    * Try to read the next document from the reader if it exists, we might
    * get NULL back and EOF, in which case we need to submit a getmore.
    */
   if (cursor->reader) {
      _mongoc_read_from_buffer (cursor, &b);
      if (b) {
         GOTO (complete);
      }
   }

   /*
    * Check to see if we need to send a GET_MORE for more results.
    */
   if (!cursor->sent) {
      b = _mongoc_cursor_initial_query (cursor);
   } else if (BSON_UNLIKELY (cursor->end_of_event) && cursor->rpc.reply.cursor_id) {
      b = _mongoc_cursor_get_more (cursor);
   }

complete:
   cursor->done = (cursor->end_of_event &&
                   ((cursor->in_exhaust && !cursor->rpc.reply.cursor_id) ||
                    (!b && !(cursor->flags & MONGOC_QUERY_TAILABLE_CURSOR))));

   if (bson) {
      *bson = b;
   }

   RETURN (!!b);
}


bool
mongoc_cursor_more (mongoc_cursor_t *cursor)
{
   bool ret;

   ENTRY;

   BSON_ASSERT(cursor);

   if (cursor->iface.more) {
      ret = cursor->iface.more(cursor);
   } else {
      ret = _mongoc_cursor_more(cursor);
   }

   RETURN(ret);
}


bool
_mongoc_cursor_more (mongoc_cursor_t *cursor)
{
   BSON_ASSERT (cursor);

   if (CURSOR_FAILED (cursor)) {
      return false;
   }

   return (!cursor->sent ||
           cursor->rpc.reply.cursor_id ||
           !cursor->end_of_event);
}


void
mongoc_cursor_get_host (mongoc_cursor_t    *cursor,
                        mongoc_host_list_t *host)
{
   BSON_ASSERT(cursor);
   BSON_ASSERT(host);

   if (cursor->iface.get_host) {
      cursor->iface.get_host(cursor, host);
   } else {
      _mongoc_cursor_get_host(cursor, host);
   }

   EXIT;
}

void
_mongoc_cursor_get_host (mongoc_cursor_t    *cursor,
                         mongoc_host_list_t *host)
{
   mongoc_server_description_t *description;

   BSON_ASSERT (cursor);
   BSON_ASSERT (host);

   memset(host, 0, sizeof *host);

   if (!cursor->hint) {
      MONGOC_WARNING("%s(): Must send query before fetching peer.",
                     BSON_FUNC);
      return;
   }

   description = mongoc_topology_server_by_id(cursor->client->topology,
                                              cursor->hint,
                                              &cursor->error);
   if (!description) {
      return;
   }

   *host = description->host;

   mongoc_server_description_destroy (description);

   return;
}

mongoc_cursor_t *
mongoc_cursor_clone (const mongoc_cursor_t *cursor)
{
   mongoc_cursor_t *ret;

   BSON_ASSERT(cursor);

   if (cursor->iface.clone) {
      ret = cursor->iface.clone(cursor);
   } else {
      ret = _mongoc_cursor_clone(cursor);
   }

   RETURN(ret);
}


mongoc_cursor_t *
_mongoc_cursor_clone (const mongoc_cursor_t *cursor)
{
   mongoc_cursor_t *_clone;

   ENTRY;

   BSON_ASSERT (cursor);

   _clone = (mongoc_cursor_t *)bson_malloc0 (sizeof *_clone);

   _clone->client = cursor->client;
   _clone->is_command = cursor->is_command;
   _clone->flags = cursor->flags;
   _clone->skip = cursor->skip;
   _clone->batch_size = cursor->batch_size;
   _clone->limit = cursor->limit;
   _clone->nslen = cursor->nslen;
   _clone->dblen = cursor->dblen;
   _clone->has_fields = cursor->has_fields;

   if (cursor->read_prefs) {
      _clone->read_prefs = mongoc_read_prefs_copy (cursor->read_prefs);
   }

   if (cursor->read_concern) {
      _clone->read_concern = mongoc_read_concern_copy (cursor->read_concern);
   }


   bson_copy_to (&cursor->query, &_clone->query);
   bson_copy_to (&cursor->fields, &_clone->fields);

   bson_strncpy (_clone->ns, cursor->ns, sizeof _clone->ns);

   _mongoc_buffer_init (&_clone->buffer, NULL, 0, NULL, NULL);

   mongoc_counter_cursors_active_inc ();

   RETURN (_clone);
}


/*
 *--------------------------------------------------------------------------
 *
 * mongoc_cursor_is_alive --
 *
 *       Checks to see if a cursor is alive.
 *
 *       This is primarily useful with tailable cursors.
 *
 * Returns:
 *       true if the cursor is alive.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

bool
mongoc_cursor_is_alive (const mongoc_cursor_t *cursor) /* IN */
{
   BSON_ASSERT (cursor);

   return (!cursor->sent ||
           (!CURSOR_FAILED (cursor) &&
            !cursor->done &&
            (cursor->rpc.header.opcode == MONGOC_OPCODE_REPLY) &&
            cursor->rpc.reply.cursor_id));
}


const bson_t *
mongoc_cursor_current (const mongoc_cursor_t *cursor) /* IN */
{
   BSON_ASSERT (cursor);

   return cursor->current;
}


void
mongoc_cursor_set_batch_size (mongoc_cursor_t *cursor,
                              uint32_t         batch_size)
{
   BSON_ASSERT (cursor);
   cursor->batch_size = batch_size;
}

uint32_t
mongoc_cursor_get_batch_size (const mongoc_cursor_t *cursor)
{
   BSON_ASSERT (cursor);

   return cursor->batch_size;
}

uint32_t
mongoc_cursor_get_hint (const mongoc_cursor_t *cursor)
{
   BSON_ASSERT (cursor);

   return cursor->hint;
}

int64_t
mongoc_cursor_get_id (const mongoc_cursor_t  *cursor)
{
   BSON_ASSERT(cursor);

   return cursor->rpc.reply.cursor_id;
}

void
mongoc_cursor_set_max_await_time_ms (mongoc_cursor_t *cursor,
                                     uint32_t         max_await_time_ms)
{
   BSON_ASSERT (cursor);

   if (!cursor->sent) {
      cursor->max_await_time_ms = max_await_time_ms;
   }
}

uint32_t
mongoc_cursor_get_max_await_time_ms (const mongoc_cursor_t *cursor)
{
   BSON_ASSERT (cursor);

   return cursor->max_await_time_ms;
}
