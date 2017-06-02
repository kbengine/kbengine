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


#include <bson.h>

#include "mongoc-async-private.h"
#include "mongoc-async-cmd-private.h"
#include "utlist.h"

#undef MONGOC_LOG_DOMAIN
#define MONGOC_LOG_DOMAIN "async"

mongoc_async_cmd_t *
mongoc_async_cmd (mongoc_async_t           *async,
                  mongoc_stream_t          *stream,
                  mongoc_async_cmd_setup_t  setup,
                  void                     *setup_ctx,
                  const char               *dbname,
                  const bson_t             *cmd,
                  mongoc_async_cmd_cb_t     cb,
                  void                     *cb_data,
                  int32_t                   timeout_msec)
{
   return mongoc_async_cmd_new (async, stream, setup, setup_ctx, dbname, cmd, cb,
                                cb_data, timeout_msec);
}

mongoc_async_t *
mongoc_async_new ()
{
   mongoc_async_t *async = (mongoc_async_t *)bson_malloc0 (sizeof (*async));

   return async;
}

void
mongoc_async_destroy (mongoc_async_t *async)
{
   mongoc_async_cmd_t *acmd, *tmp;

   DL_FOREACH_SAFE (async->cmds, acmd, tmp)
   {
      mongoc_async_cmd_destroy (acmd);
   }

   bson_free (async);
}

bool
mongoc_async_run (mongoc_async_t *async,
                  int32_t         timeout_msec)
{
   mongoc_async_cmd_t *acmd, *tmp;
   mongoc_stream_poll_t *poller = NULL;
   int i;
   ssize_t nactive = 0;
   int64_t now;
   int64_t expire_at = 0;

   size_t poll_size = 0;

   for (;;) {
      now = bson_get_monotonic_time ();

      if (expire_at == 0) {
         if (timeout_msec >= 0) {
            expire_at = now + ((int64_t) timeout_msec * 1000);
         } else {
            expire_at = -1;
         }
      } else if (timeout_msec >= 0) {
         timeout_msec = (expire_at - now) / 1000;
      }

      if (now > expire_at) {
         break;
      }

      DL_FOREACH_SAFE (async->cmds, acmd, tmp)
      {
         /* async commands are sorted by expire_at */
         if (now > acmd->expire_at) {
            acmd->cb (MONGOC_ASYNC_CMD_TIMEOUT, NULL, (now - acmd->start_time), acmd->data,
                      &acmd->error);
            mongoc_async_cmd_destroy (acmd);
         } else {
            break;
         }
      }

      if (!async->ncmds) {
         break;
      }

      if (poll_size < async->ncmds) {
         poller = (mongoc_stream_poll_t *)bson_realloc (poller, sizeof (*poller) * async->ncmds);
         poll_size = async->ncmds;
      }

      i = 0;
      DL_FOREACH (async->cmds, acmd)
      {
         poller[i].stream = acmd->stream;
         poller[i].events = acmd->events;
         poller[i].revents = 0;
         i++;
      }

      if (timeout_msec >= 0) {
         timeout_msec = BSON_MIN (timeout_msec, (async->cmds->expire_at - now) / 1000);
      } else {
         timeout_msec = (async->cmds->expire_at - now) / 1000;
      }

      nactive = mongoc_stream_poll (poller, async->ncmds, timeout_msec);

      if (nactive) {
         i = 0;

         DL_FOREACH_SAFE (async->cmds, acmd, tmp)
         {
            if (poller[i].revents & (POLLERR | POLLHUP)) {
               acmd->state = MONGOC_ASYNC_CMD_ERROR_STATE;
            }

            if (acmd->state == MONGOC_ASYNC_CMD_ERROR_STATE
                || (poller[i].revents & poller[i].events)) {

               mongoc_async_cmd_run (acmd);
               nactive--;

               if (!nactive) {
                  break;
               }
            }

            i++;
         }
      }
   }

   if (poll_size) {
      bson_free (poller);
   }

   return async->ncmds;
}
