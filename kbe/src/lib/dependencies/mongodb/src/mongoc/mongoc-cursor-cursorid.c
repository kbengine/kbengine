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
#include "mongoc-cursor-cursorid-private.h"
#include "mongoc-log.h"
#include "mongoc-trace.h"
#include "mongoc-error.h"
#include "mongoc-util-private.h"


#undef MONGOC_LOG_DOMAIN
#define MONGOC_LOG_DOMAIN "cursor-cursorid"


static void *
_mongoc_cursor_cursorid_new (void)
{
   mongoc_cursor_cursorid_t *cid;

   ENTRY;

   cid = (mongoc_cursor_cursorid_t *)bson_malloc0 (sizeof *cid);

   RETURN (cid);
}


static void
_mongoc_cursor_cursorid_destroy (mongoc_cursor_t *cursor)
{
   ENTRY;

   bson_free (cursor->iface_data);
   _mongoc_cursor_destroy (cursor);

   EXIT;
}


static bool
_mongoc_cursor_cursorid_refresh_from_command (mongoc_cursor_t *cursor,
                                              const bson_t    *command)
{
   mongoc_cursor_cursorid_t *cid;
   const bson_t *bson;
   bson_iter_t iter, child;
   const char *ns;

   ENTRY;

   cid = (mongoc_cursor_cursorid_t *)cursor->iface_data;
   BSON_ASSERT (cid);

   /* server replies to find / aggregate with {cursor: {id: N, firstBatch: []}},
    * to getMore command with {cursor: {id: N, nextBatch: []}}. */
   if (_mongoc_cursor_run_command (cursor, command) &&
       _mongoc_read_from_buffer (cursor, &bson) &&
       bson_iter_init_find (&iter, bson, "cursor") &&
       BSON_ITER_HOLDS_DOCUMENT (&iter) &&
       bson_iter_recurse (&iter, &child)) {

      while (bson_iter_next (&child)) {
         if (BSON_ITER_IS_KEY (&child, "id")) {
            cursor->rpc.reply.cursor_id = bson_iter_as_int64 (&child);
         } else if (BSON_ITER_IS_KEY (&child, "ns")) {
            ns = bson_iter_utf8 (&child, &cursor->nslen);
            bson_strncpy (cursor->ns, ns, sizeof cursor->ns);
         } else if (BSON_ITER_IS_KEY (&child, "firstBatch") ||
                    BSON_ITER_IS_KEY (&child, "nextBatch")) {
            if (BSON_ITER_HOLDS_ARRAY (&child) &&
                bson_iter_recurse (&child, &cid->batch_iter)) {
               cid->in_batch = true;
            }
         }
      }

      RETURN (true);
   } else {
      if (!cursor->error.domain) {
         bson_set_error (&cursor->error,
                         MONGOC_ERROR_PROTOCOL,
                         MONGOC_ERROR_PROTOCOL_INVALID_REPLY,
                         "Invalid reply to %s command.",
                         _mongoc_get_command_name (command));
      }

      RETURN (false);
   }
}


static void
_mongoc_cursor_cursorid_read_from_batch (mongoc_cursor_t *cursor,
                                         const bson_t   **bson)
{
   mongoc_cursor_cursorid_t *cid;
   const uint8_t *data = NULL;
   uint32_t data_len = 0;

   ENTRY;

   cid = (mongoc_cursor_cursorid_t *)cursor->iface_data;
   BSON_ASSERT (cid);

   if (bson_iter_next (&cid->batch_iter) &&
       BSON_ITER_HOLDS_DOCUMENT (&cid->batch_iter)) {
      bson_iter_document (&cid->batch_iter, &data_len, &data);

      if (bson_init_static (&cid->current_doc, data, data_len)) {
         *bson = &cid->current_doc;
      }
   }
}


bool
_mongoc_cursor_cursorid_prime (mongoc_cursor_t *cursor)
{
   cursor->sent = true;
   return _mongoc_cursor_cursorid_refresh_from_command (cursor, &cursor->query);
}


static void
_mongoc_cursor_prepare_getmore_command (mongoc_cursor_t *cursor,
                                        bson_t          *command)
{
   const char *collection;
   int collection_len;
   mongoc_cursor_cursorid_t *cid;

   cid = (mongoc_cursor_cursorid_t *)cursor->iface_data;
   BSON_ASSERT (cid);

   _mongoc_cursor_collection (cursor, &collection, &collection_len);

   bson_init (command);
   bson_append_int64 (command, "getMore", 7, mongoc_cursor_get_id (cursor));
   bson_append_utf8 (command, "collection", 10, collection, collection_len);

   if (cursor->batch_size) {
      bson_append_int32 (command, "batchSize", 9, cursor->batch_size);
   }

   /* Find, getMore And killCursors Commands Spec: "In the case of a tailable
      cursor with awaitData == true the driver MUST provide a Cursor level
      option named maxAwaitTimeMS (See CRUD specification for details). The
      maxTimeMS option on the getMore command MUST be set to the value of the
      option maxAwaitTimeMS. If no maxAwaitTimeMS is specified, the driver
      SHOULD not set maxTimeMS on the getMore command."
    */
   if (cursor->flags & MONGOC_QUERY_TAILABLE_CURSOR &&
       cursor->flags & MONGOC_QUERY_AWAIT_DATA &&
       cursor->max_await_time_ms) {
      bson_append_int32 (command, "maxTimeMS", 9, cursor->max_await_time_ms);
   }
}


static bool
_mongoc_cursor_cursorid_get_more (mongoc_cursor_t *cursor)
{
   mongoc_cursor_cursorid_t *cid;
   mongoc_server_stream_t *server_stream;
   bson_t command;
   bool ret;

   ENTRY;

   cid = (mongoc_cursor_cursorid_t *)cursor->iface_data;
   BSON_ASSERT (cid);

   server_stream = _mongoc_cursor_fetch_stream (cursor);

   if (!server_stream) {
      RETURN (false);
   }

   if (_use_find_command (cursor, server_stream)) {
      _mongoc_cursor_prepare_getmore_command (cursor, &command);
      ret = _mongoc_cursor_cursorid_refresh_from_command (cursor, &command);
      bson_destroy (&command);
   } else {
      ret = _mongoc_cursor_op_getmore (cursor, server_stream);
      cid->in_reader = ret;
   }

   mongoc_server_stream_cleanup (server_stream);
   RETURN (ret);
}


bool
_mongoc_cursor_cursorid_next (mongoc_cursor_t *cursor,
                              const bson_t   **bson)
{
   mongoc_cursor_cursorid_t *cid;
   bool refreshed = false;

   ENTRY;

   *bson = NULL;

   cid = (mongoc_cursor_cursorid_t *)cursor->iface_data;
   BSON_ASSERT (cid);

   if (!cursor->sent) {
      if (!_mongoc_cursor_cursorid_prime (cursor)) {
         GOTO (done);
      }
   }

again:

   if (cid->in_batch) {
      _mongoc_cursor_cursorid_read_from_batch (cursor, bson);

      if (*bson) {
         GOTO (done);
      }

      cid->in_batch = false;
   } else if (cid->in_reader) {
      _mongoc_read_from_buffer (cursor, bson);

      if (*bson) {
         GOTO (done);
      }

      cid->in_reader = false;
   }

   if (!refreshed && mongoc_cursor_get_id (cursor)) {
      if (!_mongoc_cursor_cursorid_get_more (cursor)) {
         GOTO (done);
      }

      refreshed = true;
      GOTO (again);
   }

done:
   RETURN (*bson ? true : false);
}


static mongoc_cursor_t *
_mongoc_cursor_cursorid_clone (const mongoc_cursor_t *cursor)
{
   mongoc_cursor_t *clone_;

   ENTRY;

   clone_ = _mongoc_cursor_clone (cursor);
   _mongoc_cursor_cursorid_init (clone_, &cursor->query);

   RETURN (clone_);
}


static mongoc_cursor_interface_t gMongocCursorCursorid = {
   _mongoc_cursor_cursorid_clone,
   _mongoc_cursor_cursorid_destroy,
   NULL,
   _mongoc_cursor_cursorid_next,
};


void
_mongoc_cursor_cursorid_init (mongoc_cursor_t *cursor,
                              const bson_t    *command)
{
   ENTRY;

   bson_destroy (&cursor->query);
   bson_copy_to (command, &cursor->query);

   cursor->iface_data = _mongoc_cursor_cursorid_new ();

   memcpy (&cursor->iface, &gMongocCursorCursorid,
           sizeof (mongoc_cursor_interface_t));

   EXIT;
}
