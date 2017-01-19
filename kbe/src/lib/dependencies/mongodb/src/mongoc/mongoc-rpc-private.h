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

#ifndef MONGOC_RPC_PRIVATE_H
#define MONGOC_RPC_PRIVATE_H

#if !defined (MONGOC_I_AM_A_DRIVER) && !defined (MONGOC_COMPILATION)
#error "Only <mongoc.h> can be included directly."
#endif

#include <bson.h>
#include <stddef.h>

#include "mongoc-array-private.h"
#include "mongoc-iovec.h"
#include "mongoc-write-concern.h"
#include "mongoc-flags.h"


BSON_BEGIN_DECLS


#define RPC(_name, _code)                typedef struct { _code } mongoc_rpc_##_name##_t;
#define ENUM_FIELD(_name)                uint32_t _name;
#define INT32_FIELD(_name)               int32_t _name;
#define INT64_FIELD(_name)               int64_t _name;
#define INT64_ARRAY_FIELD(_len, _name)   int32_t _len; int64_t *_name;
#define CSTRING_FIELD(_name)             const char *_name;
#define BSON_FIELD(_name)                const uint8_t *_name;
#define BSON_ARRAY_FIELD(_name)          const uint8_t *_name; int32_t _name##_len;
#define IOVEC_ARRAY_FIELD(_name)         const mongoc_iovec_t *_name; int32_t n_##_name; mongoc_iovec_t _name##_recv;
#define RAW_BUFFER_FIELD(_name)          const uint8_t *_name; int32_t _name##_len;
#define BSON_OPTIONAL(_check, _code)     _code


#include "op-delete.def"
#include "op-get-more.def"
#include "op-header.def"
#include "op-insert.def"
#include "op-kill-cursors.def"
#include "op-msg.def"
#include "op-query.def"
#include "op-reply.def"
#include "op-update.def"


typedef union
{
   mongoc_rpc_delete_t       delete_;
   mongoc_rpc_get_more_t     get_more;
   mongoc_rpc_header_t       header;
   mongoc_rpc_insert_t       insert;
   mongoc_rpc_kill_cursors_t kill_cursors;
   mongoc_rpc_msg_t          msg;
   mongoc_rpc_query_t        query;
   mongoc_rpc_reply_t        reply;
   mongoc_rpc_update_t       update;
} mongoc_rpc_t;


BSON_STATIC_ASSERT (sizeof (mongoc_rpc_header_t) == 16);
BSON_STATIC_ASSERT (offsetof (mongoc_rpc_header_t, opcode) ==
                    offsetof (mongoc_rpc_reply_t, opcode));


#undef RPC
#undef ENUM_FIELD
#undef INT32_FIELD
#undef INT64_FIELD
#undef INT64_ARRAY_FIELD
#undef CSTRING_FIELD
#undef BSON_FIELD
#undef BSON_ARRAY_FIELD
#undef IOVEC_ARRAY_FIELD
#undef BSON_OPTIONAL
#undef RAW_BUFFER_FIELD


void _mongoc_rpc_gather             (mongoc_rpc_t                 *rpc,
                                     mongoc_array_t               *array);
bool _mongoc_rpc_needs_gle          (mongoc_rpc_t                 *rpc,
                                     const mongoc_write_concern_t *write_concern);
void _mongoc_rpc_swab_to_le         (mongoc_rpc_t                 *rpc);
void _mongoc_rpc_swab_from_le       (mongoc_rpc_t                 *rpc);
void _mongoc_rpc_printf             (mongoc_rpc_t                 *rpc);
bool _mongoc_rpc_scatter            (mongoc_rpc_t                 *rpc,
                                     const uint8_t                *buf,
                                     size_t                        buflen);
bool _mongoc_rpc_reply_get_first    (mongoc_rpc_reply_t           *reply,
                                     bson_t                       *bson);
void _mongoc_rpc_prep_command       (mongoc_rpc_t                 *rpc,
                                     const char                   *cmd_ns,
                                     const bson_t                 *command,
                                     mongoc_query_flags_t          flags);
bool _mongoc_rpc_parse_command_error(mongoc_rpc_t                 *rpc,
                                     bson_error_t                 *error);
bool _mongoc_rpc_parse_query_error  (mongoc_rpc_t                 *rpc,
                                     bson_error_t                 *error);


BSON_END_DECLS


#endif /* MONGOC_RPC_PRIVATE_H */
