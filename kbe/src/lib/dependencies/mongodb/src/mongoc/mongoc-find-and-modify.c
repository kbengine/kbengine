/*
 * Copyright 2015 MongoDB, Inc.
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


#include "mongoc-write-concern.h"
#include "mongoc-write-concern-private.h"
#include "mongoc-find-and-modify.h"
#include "mongoc-find-and-modify-private.h"
#include "mongoc-util-private.h"


/**
 * mongoc_find_and_modify_new:
 *
 * Create a new mongoc_find_and_modify_t.
 *
 * Returns: A newly allocated mongoc_find_and_modify_t. This should be freed
 *    with mongoc_find_and_modify_destroy().
 */
mongoc_find_and_modify_opts_t*
mongoc_find_and_modify_opts_new (void)
{
   mongoc_find_and_modify_opts_t *opts = NULL;

   opts = (mongoc_find_and_modify_opts_t *)bson_malloc0 (sizeof *opts);
   opts->bypass_document_validation = MONGOC_BYPASS_DOCUMENT_VALIDATION_DEFAULT;

   return opts;
}

bool
mongoc_find_and_modify_opts_set_sort (mongoc_find_and_modify_opts_t *opts,
                                      const bson_t                  *sort)
{
   BSON_ASSERT (opts);

   if (sort) {
      _mongoc_bson_destroy_if_set (opts->sort);
      opts->sort = bson_copy (sort);
      return true;
   }
   return false;
}

bool
mongoc_find_and_modify_opts_set_update (mongoc_find_and_modify_opts_t *opts,
                                        const bson_t                  *update)
{
   BSON_ASSERT (opts);

   if (update) {
      _mongoc_bson_destroy_if_set (opts->update);
      opts->update = bson_copy (update);
      return true;
   }
   return false;
}

bool
mongoc_find_and_modify_opts_set_fields (mongoc_find_and_modify_opts_t *opts,
                                        const bson_t                  *fields)
{
   BSON_ASSERT (opts);

   if (fields) {
      _mongoc_bson_destroy_if_set (opts->fields);
      opts->fields = bson_copy (fields);
      return true;
   }
   return false;
}

bool
mongoc_find_and_modify_opts_set_flags (mongoc_find_and_modify_opts_t        *opts,
                                       const mongoc_find_and_modify_flags_t  flags)
{
   BSON_ASSERT (opts);

   opts->flags = flags;
   return true;
}

bool
mongoc_find_and_modify_opts_set_bypass_document_validation (mongoc_find_and_modify_opts_t *opts,
                                                            bool                           bypass)
{
   BSON_ASSERT (opts);

   opts->bypass_document_validation = bypass ?
      MONGOC_BYPASS_DOCUMENT_VALIDATION_TRUE :
      MONGOC_BYPASS_DOCUMENT_VALIDATION_FALSE;
   return true;
}

/**
 * mongoc_find_and_modify_opts_destroy:
 * @opts: A mongoc_find_and_modify_opts_t.
 *
 * Releases a mongoc_find_and_modify_opts_t and all associated memory.
 */
void
mongoc_find_and_modify_opts_destroy (mongoc_find_and_modify_opts_t *opts)
{
   if (opts) {
      _mongoc_bson_destroy_if_set (opts->sort);
      _mongoc_bson_destroy_if_set (opts->update);
      _mongoc_bson_destroy_if_set (opts->fields);
      bson_free (opts);
   }
}
