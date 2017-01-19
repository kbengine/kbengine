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

#ifndef MONGOC_CLUSTER_PRIVATE_H
#define MONGOC_CLUSTER_PRIVATE_H

#if !defined (MONGOC_I_AM_A_DRIVER) && !defined (MONGOC_COMPILATION)
#error "Only <mongoc.h> can be included directly."
#endif

#include <bson.h>

#include "mongoc-array-private.h"
#include "mongoc-buffer-private.h"
#include "mongoc-config.h"
#include "mongoc-client.h"
#include "mongoc-list-private.h"
#include "mongoc-opcode.h"
#include "mongoc-read-prefs.h"
#include "mongoc-rpc-private.h"
#include "mongoc-server-stream-private.h"
#include "mongoc-set-private.h"
#include "mongoc-stream.h"
#include "mongoc-topology-description-private.h"
#include "mongoc-uri.h"
#include "mongoc-write-concern.h"


BSON_BEGIN_DECLS


typedef struct _mongoc_cluster_node_t
{
   mongoc_stream_t *stream;

   int32_t          max_wire_version;
   int32_t          min_wire_version;
   int32_t          max_write_batch_size;
   int32_t          max_bson_obj_size;
   int32_t          max_msg_size;

   int64_t          timestamp;
} mongoc_cluster_node_t;

typedef struct _mongoc_cluster_t
{
   uint32_t         request_id;
   uint32_t         sockettimeoutms;
   uint32_t         socketcheckintervalms;
   mongoc_uri_t    *uri;
   unsigned         requires_auth : 1;

   mongoc_client_t *client;

   mongoc_set_t    *nodes;
   mongoc_array_t   iov;
} mongoc_cluster_t;

void
mongoc_cluster_init (mongoc_cluster_t   *cluster,
                     const mongoc_uri_t *uri,
                     void               *client);

void
mongoc_cluster_destroy (mongoc_cluster_t *cluster);

void
mongoc_cluster_disconnect_node (mongoc_cluster_t *cluster,
                                uint32_t          id);

int32_t
mongoc_cluster_get_max_bson_obj_size (mongoc_cluster_t *cluster);

int32_t
mongoc_cluster_get_max_msg_size (mongoc_cluster_t *cluster);

int32_t
mongoc_cluster_node_max_wire_version (mongoc_cluster_t *cluster,
                                      uint32_t          server_id);

int32_t
mongoc_cluster_node_min_wire_version (mongoc_cluster_t *cluster,
                                      uint32_t          server_id);

bool
mongoc_cluster_sendv_to_server (mongoc_cluster_t             *cluster,
                                mongoc_rpc_t                 *rpcs,
                                size_t                        rpcs_len,
                                mongoc_server_stream_t       *server_stream,
                                const mongoc_write_concern_t *write_concern,
                                bson_error_t                 *error);

bool
mongoc_cluster_try_recv (mongoc_cluster_t       *cluster,
                         mongoc_rpc_t           *rpc,
                         mongoc_buffer_t        *buffer,
                         mongoc_server_stream_t *server_stream,
                         bson_error_t           *error);

mongoc_server_stream_t *
mongoc_cluster_stream_for_reads (mongoc_cluster_t *cluster,
                                 const mongoc_read_prefs_t *read_prefs,
                                 bson_error_t *error);

mongoc_server_stream_t *
mongoc_cluster_stream_for_writes (mongoc_cluster_t *cluster,
                                  bson_error_t *error);

mongoc_server_stream_t *
mongoc_cluster_stream_for_server (mongoc_cluster_t *cluster,
                                  uint32_t server_id,
                                  bool reconnect_ok,
                                  bson_error_t *error);

bool
mongoc_cluster_run_command_rpc (mongoc_cluster_t *cluster,
                                mongoc_stream_t  *stream,
                                uint32_t          server_id,
                                const char       *command_name,
                                mongoc_rpc_t     *rpc,
                                mongoc_rpc_t     *reply_rpc,
                                mongoc_buffer_t  *buffer,
                                bson_error_t     *error);

bool
mongoc_cluster_run_command (mongoc_cluster_t    *cluster,
                            mongoc_stream_t     *stream,
                            uint32_t             server_id,
                            mongoc_query_flags_t flags,
                            const char          *db_name,
                            const bson_t        *command,
                            bson_t              *reply,
                            bson_error_t        *error);


BSON_END_DECLS


#endif /* MONGOC_CLUSTER_PRIVATE_H */
