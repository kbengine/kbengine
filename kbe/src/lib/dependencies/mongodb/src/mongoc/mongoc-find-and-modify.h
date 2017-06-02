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

#ifndef MONGOC_FIND_AND_MODIFY_H
#define MONGOC_FIND_AND_MODIFY_H

#if !defined (MONGOC_INSIDE) && !defined (MONGOC_COMPILATION)
# error "Only <mongoc.h> can be included directly."
#endif

#include <bson.h>

BSON_BEGIN_DECLS

typedef enum
{
   MONGOC_FIND_AND_MODIFY_NONE   = 0,
   MONGOC_FIND_AND_MODIFY_REMOVE = 1 << 0,
   MONGOC_FIND_AND_MODIFY_UPSERT = 1 << 1,
   MONGOC_FIND_AND_MODIFY_RETURN_NEW = 1 << 2,
} mongoc_find_and_modify_flags_t;

typedef struct _mongoc_find_and_modify_opts_t mongoc_find_and_modify_opts_t ;

mongoc_find_and_modify_opts_t*
mongoc_find_and_modify_opts_new               (void);

bool
mongoc_find_and_modify_opts_set_sort          (mongoc_find_and_modify_opts_t        *opts,
                                               const bson_t                         *sort);
bool
mongoc_find_and_modify_opts_set_update        (mongoc_find_and_modify_opts_t        *opts,
                                               const bson_t                         *update);
bool
mongoc_find_and_modify_opts_set_fields        (mongoc_find_and_modify_opts_t        *opts,
                                               const bson_t                         *fields);
bool
mongoc_find_and_modify_opts_set_flags         (mongoc_find_and_modify_opts_t        *opts,
                                               const mongoc_find_and_modify_flags_t  flags);
bool
mongoc_find_and_modify_opts_set_bypass_document_validation (mongoc_find_and_modify_opts_t *opts,
                                                            bool                           bypass);
void
mongoc_find_and_modify_opts_destroy           (mongoc_find_and_modify_opts_t *opts);

BSON_END_DECLS


#endif /* MONGOC_FIND_AND_MODIFY_H */
