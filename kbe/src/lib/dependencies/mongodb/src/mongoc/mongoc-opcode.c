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

#include "mongoc-opcode-private.h"

/*
 *--------------------------------------------------------------------------
 *
 * _mongoc_opcode_needs_primary --
 *
 *       Returns true if this operation needs to run on a primary,
 *       false if it does not.
 *
 * Returns:
 *       true, false
 *
 * Side effects:
 *       None
 *
 *--------------------------------------------------------------------------
 */

bool
_mongoc_opcode_needs_primary (mongoc_opcode_t opcode)
{
   bool needs_primary = false;

   switch(opcode) {
   case MONGOC_OPCODE_KILL_CURSORS:
   case MONGOC_OPCODE_GET_MORE:
   case MONGOC_OPCODE_MSG:
   case MONGOC_OPCODE_REPLY:
      needs_primary = false;
      break;
   case MONGOC_OPCODE_QUERY:
      /* In some cases, queries may be run against secondaries.
         However, more information is needed to make that decision.
         Callers with access to read preferences and query flags may
         route queries to a secondary when appropriate */
   case MONGOC_OPCODE_DELETE:
   case MONGOC_OPCODE_INSERT:
   case MONGOC_OPCODE_UPDATE:
   default:
      needs_primary = true;
      break;
   }

   return needs_primary;
}
