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


#include "mongoc-bulk-operation.h"
#include "mongoc-bulk-operation-private.h"
#include "mongoc-client-private.h"
#include "mongoc-error.h"
#include "mongoc-trace.h"
#include "mongoc-write-concern-private.h"


/*
 * This is the implementation of both write commands and bulk write commands.
 * They are all implemented as one contiguous set since we'd like to cut down
 * on code duplication here.
 *
 * This implementation is currently naive.
 *
 * Some interesting optimizations might be:
 *
 *   - If unordered mode, send operations as we get them instead of waiting
 *     for execute() to be called. This could save us memcpy()'s too.
 *   - If there is no acknowledgement desired, keep a count of how many
 *     replies we need and ask the socket layer to skip that many bytes
 *     when reading.
 *   - Try to use iovec to send write commands with subdocuments rather than
 *     copying them into the write command document.
 */


mongoc_bulk_operation_t *
mongoc_bulk_operation_new (bool ordered)
{
   mongoc_bulk_operation_t *bulk;

   bulk = (mongoc_bulk_operation_t *)bson_malloc0 (sizeof *bulk);
   bulk->flags.bypass_document_validation = MONGOC_BYPASS_DOCUMENT_VALIDATION_DEFAULT;
   bulk->flags.ordered = ordered;
   bulk->hint = 0;

   _mongoc_array_init (&bulk->commands, sizeof (mongoc_write_command_t));

   return bulk;
}


mongoc_bulk_operation_t *
_mongoc_bulk_operation_new (mongoc_client_t              *client,        /* IN */
                            const char                   *database,      /* IN */
                            const char                   *collection,    /* IN */
                            mongoc_bulk_write_flags_t     flags,         /* IN */
                            const mongoc_write_concern_t *write_concern) /* IN */
{
   mongoc_bulk_operation_t *bulk;

   BSON_ASSERT (client);
   BSON_ASSERT (collection);

   bulk = mongoc_bulk_operation_new (flags.ordered);
   bulk->client = client;
   bulk->database = bson_strdup (database);
   bulk->collection = bson_strdup (collection);
   bulk->write_concern = mongoc_write_concern_copy (write_concern);
   bulk->executed = false;
   bulk->flags = flags;

   return bulk;
}


void
mongoc_bulk_operation_destroy (mongoc_bulk_operation_t *bulk) /* IN */
{
   mongoc_write_command_t *command;
   int i;

   if (bulk) {
      for (i = 0; i < bulk->commands.len; i++) {
         command = &_mongoc_array_index (&bulk->commands,
                                         mongoc_write_command_t, i);
         _mongoc_write_command_destroy (command);
      }

      bson_free (bulk->database);
      bson_free (bulk->collection);
      mongoc_write_concern_destroy (bulk->write_concern);
      _mongoc_array_destroy (&bulk->commands);

      if (bulk->executed) {
         _mongoc_write_result_destroy (&bulk->result);
      }

      bson_free (bulk);
   }
}


void
mongoc_bulk_operation_remove (mongoc_bulk_operation_t *bulk,     /* IN */
                              const bson_t            *selector) /* IN */
{
   mongoc_write_command_t command = { 0 };
   mongoc_write_command_t *last;

   ENTRY;

   BSON_ASSERT (bulk);
   BSON_ASSERT (selector);

   if (bulk->commands.len) {
      last = &_mongoc_array_index (&bulk->commands,
                                   mongoc_write_command_t,
                                   bulk->commands.len - 1);
      if ((last->type == MONGOC_WRITE_COMMAND_DELETE) &&
          last->u.delete_.multi) {
         _mongoc_write_command_delete_append (last, selector);
         EXIT;
      }
   }

   _mongoc_write_command_init_delete (&command, selector, true, bulk->flags);

   _mongoc_array_append_val (&bulk->commands, command);

   EXIT;
}


void
mongoc_bulk_operation_remove_one (mongoc_bulk_operation_t *bulk,     /* IN */
                                  const bson_t            *selector) /* IN */
{
   mongoc_write_command_t command = { 0 };
   mongoc_write_command_t *last;

   ENTRY;

   BSON_ASSERT (bulk);
   BSON_ASSERT (selector);

   if (bulk->commands.len) {
      last = &_mongoc_array_index (&bulk->commands,
                                   mongoc_write_command_t,
                                   bulk->commands.len - 1);
      if ((last->type == MONGOC_WRITE_COMMAND_DELETE) &&
          !last->u.delete_.multi) {
         _mongoc_write_command_delete_append (last, selector);
         EXIT;
      }
   }

   _mongoc_write_command_init_delete (&command, selector, false, bulk->flags);

   _mongoc_array_append_val (&bulk->commands, command);

   EXIT;
}


void
mongoc_bulk_operation_delete (mongoc_bulk_operation_t *bulk,
                              const bson_t            *selector)
{
   ENTRY;

   mongoc_bulk_operation_remove (bulk, selector);

   EXIT;
}


void
mongoc_bulk_operation_delete_one (mongoc_bulk_operation_t *bulk,
                                  const bson_t            *selector)
{
   ENTRY;

   mongoc_bulk_operation_remove_one (bulk, selector);

   EXIT;
}


void
mongoc_bulk_operation_insert (mongoc_bulk_operation_t *bulk,
                              const bson_t            *document)
{
   mongoc_write_command_t command = { 0 };
   mongoc_write_command_t *last;

   ENTRY;

   BSON_ASSERT (bulk);
   BSON_ASSERT (document);

   if (bulk->commands.len) {
      last = &_mongoc_array_index (&bulk->commands,
                                   mongoc_write_command_t,
                                   bulk->commands.len - 1);

      if (last->type == MONGOC_WRITE_COMMAND_INSERT) {
         _mongoc_write_command_insert_append (last, document);
         EXIT;
      }
   }

   _mongoc_write_command_init_insert (
      &command, document, bulk->flags,
      !_mongoc_write_concern_needs_gle (bulk->write_concern));

   _mongoc_array_append_val (&bulk->commands, command);

   EXIT;
}


void
mongoc_bulk_operation_replace_one (mongoc_bulk_operation_t *bulk,
                                   const bson_t            *selector,
                                   const bson_t            *document,
                                   bool                     upsert)
{
   mongoc_write_command_t command = { 0 };
   size_t err_off;
   mongoc_write_command_t *last;
   int flags = BSON_VALIDATE_DOT_KEYS|BSON_VALIDATE_DOLLAR_KEYS;

   BSON_ASSERT (bulk);
   BSON_ASSERT (selector);
   BSON_ASSERT (document);

   ENTRY;

   if (!bson_validate (document, (bson_validate_flags_t)flags, &err_off)) {
      MONGOC_WARNING ("%s(): replacement document may not contain "
                      "$ or . in keys. Ignoring document.",
                      BSON_FUNC);
      EXIT;
   }

   if (bulk->commands.len) {
      last = &_mongoc_array_index (&bulk->commands,
                                   mongoc_write_command_t,
                                   bulk->commands.len - 1);
      if (last->type == MONGOC_WRITE_COMMAND_UPDATE) {
         _mongoc_write_command_update_append (last, selector, document, upsert, false);
         EXIT;
      }
   }

   _mongoc_write_command_init_update (&command, selector, document, upsert,
                                      false, bulk->flags);
   _mongoc_array_append_val (&bulk->commands, command);

   EXIT;
}


void
mongoc_bulk_operation_update (mongoc_bulk_operation_t *bulk,
                              const bson_t            *selector,
                              const bson_t            *document,
                              bool                     upsert)
{
   bool multi = true;
   mongoc_write_command_t command = { 0 };
   bson_iter_t iter;
   mongoc_write_command_t *last;

   BSON_ASSERT (bulk);
   BSON_ASSERT (selector);
   BSON_ASSERT (document);

   ENTRY;

   if (bson_iter_init (&iter, document)) {
      while (bson_iter_next (&iter)) {
         if (!strchr (bson_iter_key (&iter), '$')) {
            MONGOC_WARNING ("%s(): update only works with $ operators.",
                            BSON_FUNC);
            EXIT;
         }
      }
   }

   if (bulk->commands.len) {
      last = &_mongoc_array_index (&bulk->commands,
                                   mongoc_write_command_t,
                                   bulk->commands.len - 1);
      if (last->type == MONGOC_WRITE_COMMAND_UPDATE) {
         _mongoc_write_command_update_append (last, selector, document, upsert, multi);
         EXIT;
      }
   }

   _mongoc_write_command_init_update (&command, selector, document, upsert,
                                      multi, bulk->flags);
   _mongoc_array_append_val (&bulk->commands, command);
   EXIT;
}


void
mongoc_bulk_operation_update_one (mongoc_bulk_operation_t *bulk,
                                  const bson_t            *selector,
                                  const bson_t            *document,
                                  bool                     upsert)
{
   mongoc_write_command_t command = { 0 };
   bson_iter_t iter;
   mongoc_write_command_t *last;

   BSON_ASSERT (bulk);
   BSON_ASSERT (selector);
   BSON_ASSERT (document);

   ENTRY;

   if (bson_iter_init (&iter, document)) {
      while (bson_iter_next (&iter)) {
         if (!strchr (bson_iter_key (&iter), '$')) {
            MONGOC_WARNING ("%s(): update_one only works with $ operators.",
                            BSON_FUNC);
            EXIT;
         }
      }
   }

   if (bulk->commands.len) {
      last = &_mongoc_array_index (&bulk->commands,
                                   mongoc_write_command_t,
                                   bulk->commands.len - 1);
      if (last->type == MONGOC_WRITE_COMMAND_UPDATE) {
         _mongoc_write_command_update_append (last, selector, document, upsert, false);
         EXIT;
      }
   }

   _mongoc_write_command_init_update (&command, selector, document, upsert,
                                      false, bulk->flags);
   _mongoc_array_append_val (&bulk->commands, command);
   EXIT;
}


uint32_t
mongoc_bulk_operation_execute (mongoc_bulk_operation_t *bulk,  /* IN */
                               bson_t                  *reply, /* OUT */
                               bson_error_t            *error) /* OUT */
{
   mongoc_cluster_t *cluster;
   mongoc_write_command_t *command;
   mongoc_server_stream_t *server_stream;
   bool ret;
   uint32_t offset = 0;
   int i;

   ENTRY;

   BSON_ASSERT (bulk);

   cluster = &bulk->client->cluster;

   if (bulk->executed) {
      _mongoc_write_result_destroy (&bulk->result);
   }

   _mongoc_write_result_init (&bulk->result);

   bulk->executed = true;

   if (!bulk->client) {
      bson_set_error (error,
                      MONGOC_ERROR_COMMAND,
                      MONGOC_ERROR_COMMAND_INVALID_ARG,
                      "mongoc_bulk_operation_execute() requires a client "
                      "and one has not been set.");
      RETURN (false);
   } else if (!bulk->database) {
      bson_set_error (error,
                      MONGOC_ERROR_COMMAND,
                      MONGOC_ERROR_COMMAND_INVALID_ARG,
                      "mongoc_bulk_operation_execute() requires a database "
                      "and one has not been set.");
      RETURN (false);
   } else if (!bulk->collection) {
      bson_set_error (error,
                      MONGOC_ERROR_COMMAND,
                      MONGOC_ERROR_COMMAND_INVALID_ARG,
                      "mongoc_bulk_operation_execute() requires a collection "
                      "and one has not been set.");
      RETURN (false);
   }

   if (reply) {
      bson_init (reply);
   }

   if (!bulk->commands.len) {
      bson_set_error (error,
                      MONGOC_ERROR_COMMAND,
                      MONGOC_ERROR_COMMAND_INVALID_ARG,
                      "Cannot do an empty bulk write");
      RETURN (false);
   }

   if (bulk->hint) {
      server_stream = mongoc_cluster_stream_for_server (cluster,
                                                        bulk->hint,
                                                        true /* reconnect_ok */,
                                                        error);
   } else {
      server_stream = mongoc_cluster_stream_for_writes (cluster, error);
   }

   if (!server_stream) {
      RETURN (false);
   }

   for (i = 0; i < bulk->commands.len; i++) {
      command = &_mongoc_array_index (&bulk->commands,
                                      mongoc_write_command_t, i);

      _mongoc_write_command_execute (command, bulk->client, server_stream,
                                     bulk->database, bulk->collection,
                                     bulk->write_concern, offset,
                                     &bulk->result);

      bulk->hint = command->hint;

      if (bulk->result.failed && bulk->flags.ordered) {
         GOTO (cleanup);
      }

      offset += command->n_documents;
   }

cleanup:
   ret = _mongoc_write_result_complete (&bulk->result, reply, error);
   mongoc_server_stream_cleanup (server_stream);

   RETURN (ret ? bulk->hint : 0);
}

void
mongoc_bulk_operation_set_write_concern (mongoc_bulk_operation_t      *bulk,
                                         const mongoc_write_concern_t *write_concern)
{
   BSON_ASSERT (bulk);

   if (bulk->write_concern) {
      mongoc_write_concern_destroy (bulk->write_concern);
   }

   if (write_concern) {
      bulk->write_concern = mongoc_write_concern_copy (write_concern);
   } else {
      bulk->write_concern = mongoc_write_concern_new ();
   }
}

const mongoc_write_concern_t *
mongoc_bulk_operation_get_write_concern (const mongoc_bulk_operation_t *bulk)
{
   BSON_ASSERT (bulk);

   return bulk->write_concern;
}


void
mongoc_bulk_operation_set_database (mongoc_bulk_operation_t *bulk,
                                    const char              *database)
{
   BSON_ASSERT (bulk);

   if (bulk->database) {
      bson_free (bulk->database);
   }

   bulk->database = bson_strdup (database);
}


void
mongoc_bulk_operation_set_collection (mongoc_bulk_operation_t *bulk,
                                      const char              *collection)
{
   BSON_ASSERT (bulk);

   if (bulk->collection) {
      bson_free (bulk->collection);
   }

   bulk->collection = bson_strdup (collection);
}


void
mongoc_bulk_operation_set_client (mongoc_bulk_operation_t *bulk,
                                  void                    *client)
{
   BSON_ASSERT (bulk);

   bulk->client = (mongoc_client_t *)client;
}


void
mongoc_bulk_operation_set_hint (mongoc_bulk_operation_t *bulk,
                                uint32_t                 hint)
{
   BSON_ASSERT (bulk);

   bulk->hint = hint;
}


void
mongoc_bulk_operation_set_bypass_document_validation (mongoc_bulk_operation_t *bulk,
                                                      bool                     bypass)
{
   BSON_ASSERT (bulk);

   bulk->flags.bypass_document_validation = bypass ?
      MONGOC_BYPASS_DOCUMENT_VALIDATION_TRUE :
      MONGOC_BYPASS_DOCUMENT_VALIDATION_FALSE;
}


