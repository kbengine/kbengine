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

#ifndef MONGOC_URI_PRIVATE_H
#define MONGOC_URI_PRIVATE_H

#if !defined (MONGOC_I_AM_A_DRIVER) && !defined (MONGOC_COMPILATION)
#error "Only <mongoc.h> can be included directly."
#endif

#include "mongoc-uri.h"


BSON_BEGIN_DECLS


void
mongoc_uri_lowercase_hostname    (      const char   *src,
                                        char         *buf /* OUT */,
                                        int           len);
void
mongoc_uri_append_host           (      mongoc_uri_t *uri,
                                  const char         *host,
                                        uint16_t      port);
bool
mongoc_uri_parse_host            (      mongoc_uri_t  *uri,
                                  const char          *str);
bool
mongoc_uri_set_username          (      mongoc_uri_t *uri,
                                  const char         *username);
bool
mongoc_uri_set_password          (      mongoc_uri_t *uri,
                                  const char         *password);
bool
mongoc_uri_set_database          (      mongoc_uri_t *uri,
                                  const char         *database);
bool
mongoc_uri_set_auth_source       (      mongoc_uri_t *uri,
                                  const char         *value);
bool
mongoc_uri_option_is_int32       (const char         *key);
bool
mongoc_uri_option_is_bool        (const char         *key);
bool
mongoc_uri_option_is_utf8        (const char         *key);
int32_t
mongoc_uri_get_option_as_int32   (const mongoc_uri_t *uri,
                                  const char         *option,
                                        int32_t       fallback);
bool
mongoc_uri_get_option_as_bool    (const mongoc_uri_t *uri,
                                  const char         *option,
                                        bool          fallback);
const char*
mongoc_uri_get_option_as_utf8    (const mongoc_uri_t *uri,
                                  const char         *option,
                                  const char         *fallback);
bool
mongoc_uri_set_option_as_int32   (      mongoc_uri_t *uri,
                                  const char         *option,
                                        int32_t       value);
bool
mongoc_uri_set_option_as_bool    (      mongoc_uri_t *uri,
                                  const char         *option,
                                        bool          value);
bool
mongoc_uri_set_option_as_utf8    (      mongoc_uri_t *uri,
                                  const char         *option,
                                  const char         *value);

BSON_END_DECLS


#endif /* MONGOC_URI_PRIVATE_H */
