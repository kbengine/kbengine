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
#include "mongoc-cursor-array-private.h"
#include "mongoc-cursor-private.h"
#include "mongoc-client-private.h"
#include "mongoc-counters-private.h"
#include "mongoc-error.h"
#include "mongoc-log.h"
#include "mongoc-opcode.h"
#include "mongoc-trace.h"


#undef MONGOC_LOG_DOMAIN
#define MONGOC_LOG_DOMAIN "cursor-array"


typedef struct
{
   const bson_t       *result;
   bool         has_array;
   bool         has_synthetic_bson;
   bson_iter_t         iter;
   bson_t              bson;
   uint32_t       document_len;
   const uint8_t *document;
   const char    *field_name;
} mongoc_cursor_array_t;


static void *
_mongoc_cursor_array_new (const char *field_name)
{
   mongoc_cursor_array_t *arr;

   ENTRY;

   arr = (mongoc_cursor_array_t *)bson_malloc0 (sizeof *arr);
   arr->field_name = field_name;

   RETURN (arr);
}


static void
_mongoc_cursor_array_destroy (mongoc_cursor_t *cursor)
{
   mongoc_cursor_array_t *arr;

   ENTRY;

   arr = (mongoc_cursor_array_t *)cursor->iface_data;

   if (arr->has_synthetic_bson) {
      bson_destroy(&arr->bson);
   }

   bson_free (cursor->iface_data);
   _mongoc_cursor_destroy (cursor);

   EXIT;
}


bool
_mongoc_cursor_array_prime (mongoc_cursor_t *cursor)
{
   const bson_t *bson;
   mongoc_cursor_array_t *arr;
   bson_iter_t iter;

   ENTRY;

   arr = (mongoc_cursor_array_t *)cursor->iface_data;

   BSON_ASSERT (arr);

   if (_mongoc_cursor_run_command (cursor, &cursor->query) &&
       _mongoc_read_from_buffer (cursor, &bson) &&
       bson_iter_init_find (&iter, bson, arr->field_name) &&
       BSON_ITER_HOLDS_ARRAY (&iter) &&
       bson_iter_recurse (&iter, &arr->iter)) {
      arr->has_array = true;
   }

   return arr->has_array;
}


static bool
_mongoc_cursor_array_next (mongoc_cursor_t *cursor,
                           const bson_t   **bson)
{
   bool ret = true;
   mongoc_cursor_array_t *arr;

   ENTRY;

   arr = (mongoc_cursor_array_t *)cursor->iface_data;
   *bson = NULL;

   if (!arr->has_array) {
      ret = _mongoc_cursor_array_prime(cursor);
   }

   if (ret) {
      ret = bson_iter_next (&arr->iter);
   }

   if (ret) {
      bson_iter_document (&arr->iter, &arr->document_len, &arr->document);
      bson_init_static (&arr->bson, arr->document, arr->document_len);

      *bson = &arr->bson;
   }

   RETURN (ret);
}


static mongoc_cursor_t *
_mongoc_cursor_array_clone (const mongoc_cursor_t *cursor)
{
   mongoc_cursor_array_t *arr;
   mongoc_cursor_t *clone_;

   ENTRY;

   arr = (mongoc_cursor_array_t *)cursor->iface_data;

   clone_ = _mongoc_cursor_clone (cursor);
   _mongoc_cursor_array_init (clone_, &cursor->query, arr->field_name);

   RETURN (clone_);
}


static bool
_mongoc_cursor_array_more (mongoc_cursor_t *cursor)
{
   bool ret;
   mongoc_cursor_array_t *arr;
   bson_iter_t iter;

   ENTRY;

   arr = (mongoc_cursor_array_t *)cursor->iface_data;

   if (arr->has_array) {
      memcpy (&iter, &arr->iter, sizeof iter);

      ret = bson_iter_next (&iter);
   } else {
      ret = true;
   }

   RETURN (ret);
}


static bool
_mongoc_cursor_array_error  (mongoc_cursor_t *cursor,
                             bson_error_t    *error)
{
   mongoc_cursor_array_t *arr;

   ENTRY;

   arr = (mongoc_cursor_array_t *)cursor->iface_data;

   if (arr->has_synthetic_bson) {
      return false;
   } else {
      return _mongoc_cursor_error(cursor, error);
   }
}


static mongoc_cursor_interface_t gMongocCursorArray = {
   _mongoc_cursor_array_clone,
   _mongoc_cursor_array_destroy,
   _mongoc_cursor_array_more,
   _mongoc_cursor_array_next,
   _mongoc_cursor_array_error,
};


void
_mongoc_cursor_array_init (mongoc_cursor_t *cursor,
                           const bson_t    *command,
                           const char      *field_name)
{
   ENTRY;


   if (command) {
      bson_destroy (&cursor->query);
      bson_copy_to (command, &cursor->query);
   }

   cursor->iface_data = _mongoc_cursor_array_new (field_name);

   memcpy (&cursor->iface, &gMongocCursorArray,
           sizeof (mongoc_cursor_interface_t));

   EXIT;
}


void
_mongoc_cursor_array_set_bson (mongoc_cursor_t *cursor,
                               const bson_t    *bson)
{
   mongoc_cursor_array_t *arr;

   ENTRY;

   arr = (mongoc_cursor_array_t *)cursor->iface_data;

   bson_copy_to(bson, &arr->bson);

   arr->has_array = true;
   arr->has_synthetic_bson = true;

   bson_iter_init(&arr->iter, &arr->bson);
}
