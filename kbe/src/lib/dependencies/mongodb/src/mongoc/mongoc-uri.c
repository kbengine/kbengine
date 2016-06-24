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


#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <math.h>

/* strcasecmp on windows */
#include "mongoc-util-private.h"

#include "mongoc-host-list.h"
#include "mongoc-host-list-private.h"
#include "mongoc-log.h"
#include "mongoc-socket.h"
#include "mongoc-uri-private.h"
#include "mongoc-read-concern-private.h"
#include "mongoc-write-concern-private.h"


struct _mongoc_uri_t
{
   char                   *str;
   mongoc_host_list_t     *hosts;
   char                   *username;
   char                   *password;
   char                   *database;
   bson_t                  options;
   bson_t                  credentials;
   mongoc_read_prefs_t    *read_prefs;
   mongoc_read_concern_t  *read_concern;
   mongoc_write_concern_t *write_concern;
};

static void
mongoc_uri_do_unescape (char **str)
{
   char *tmp;

   if ((tmp = *str)) {
      *str = mongoc_uri_unescape(tmp);
      bson_free(tmp);
   }
}

void
mongoc_uri_lowercase_hostname (const char *src,
                               char *buf /* OUT */,
                               int len)
{
   bson_unichar_t c;
   const char *iter;
   char *buf_iter;

   /* TODO: this code only accepts ascii, and assumes that lowercased
      chars are the same width as originals */
   for (iter = src, buf_iter = buf;
        iter && *iter && (c = bson_utf8_get_char(iter)) && buf_iter - buf < len;
        iter = bson_utf8_next_char(iter), buf_iter++) {
      assert(c < 128);
      *buf_iter = tolower(c);
   }
}

void
mongoc_uri_append_host (mongoc_uri_t  *uri,
                        const char    *host,
                        uint16_t       port)
{
   mongoc_host_list_t *iter;
   mongoc_host_list_t *link_;

   link_ = (mongoc_host_list_t *)bson_malloc0(sizeof *link_);
   mongoc_uri_lowercase_hostname(host, link_->host, sizeof link_->host);
   if (strchr (host, ':')) {
      bson_snprintf (link_->host_and_port, sizeof link_->host_and_port,
                     "[%s]:%hu", host, port);
      link_->family = AF_INET6;
   } else {
      bson_snprintf (link_->host_and_port, sizeof link_->host_and_port,
                     "%s:%hu", host, port);
      link_->family = strstr (host, ".sock") ? AF_UNIX : AF_INET;
   }
   link_->host_and_port[sizeof link_->host_and_port - 1] = '\0';
   link_->port = port;

   if ((iter = uri->hosts)) {
      for (; iter && iter->next; iter = iter->next) {}
      iter->next = link_;
   } else {
      uri->hosts = link_;
   }
}

/*
 *--------------------------------------------------------------------------
 *
 * scan_to_unichar --
 *
 *       Scans 'str' until either a character matching 'match' is found,
 *       until one of the characters in 'terminators' is encountered, or
 *       until we reach the end of 'str'.
 *
 *       NOTE: 'terminators' may not include multibyte UTF-8 characters.
 *
 * Returns:
 *       If 'match' is found, returns a copy of the section of 'str' before
 *       that character.  Otherwise, returns NULL.
 *
 * Side Effects:
 *       If 'match' is found, sets 'end' to begin at the matching character
 *       in 'str'.
 *
 *--------------------------------------------------------------------------
 */

static char *
scan_to_unichar (const char      *str,
                 bson_unichar_t   match,
                 const char      *terminators,
                 const char     **end)
{
   bson_unichar_t c;
   const char *iter;

   for (iter = str;
        iter && *iter && (c = bson_utf8_get_char(iter));
        iter = bson_utf8_next_char(iter)) {
      if (c == match) {
         *end = iter;
         return bson_strndup(str, iter - str);
      } else if (c == '\\') {
         iter = bson_utf8_next_char(iter);
         if (!bson_utf8_get_char(iter)) {
            break;
         }
      } else {
         const char *term_iter;
         for (term_iter = terminators; *term_iter; term_iter++) {
            if (c == *term_iter) {
               return NULL;
            }
         }
      }
   }

   return NULL;
}


static bool
mongoc_uri_parse_scheme (const char    *str,
                         const char   **end)
{
   if (!!strncmp(str, "mongodb://", 10)) {
      return false;
   }

   *end = str + 10;

   return true;
}


static bool
mongoc_uri_parse_userpass (mongoc_uri_t  *uri,
                           const char    *str,
                           const char   **end)
{
   bool ret = false;
   const char *end_userpass;
   const char *end_user;
   char *s;

   if ((s = scan_to_unichar(str, '@', "", &end_userpass))) {
      if ((uri->username = scan_to_unichar(s, ':', "", &end_user))) {
         uri->password = bson_strdup(end_user + 1);
      } else {
         uri->username = bson_strndup(str, end_userpass - str);
         uri->password = NULL;
      }
      mongoc_uri_do_unescape(&uri->username);
      mongoc_uri_do_unescape(&uri->password);
      *end = end_userpass + 1;
      bson_free(s);
      ret = true;
   } else {
      ret = true;
   }

   return ret;
}

static bool
mongoc_uri_parse_port (uint16_t   *port,
                       const char *str)
{
   unsigned long ul_port;

   ul_port = strtoul (str, NULL, 10);

   if (ul_port == 0 || ul_port > UINT16_MAX) {
      /* Parse error or port number out of range. mongod prohibits port 0. */
      return false;
   }

   *port = (uint16_t)ul_port;
   return true;
}


static bool
mongoc_uri_parse_host6 (mongoc_uri_t  *uri,
                        const char    *str)
{
   uint16_t port = MONGOC_DEFAULT_PORT;
   const char *portstr;
   const char *end_host;
   char *hostname;

   if ((portstr = strrchr (str, ':')) && !strstr (portstr, "]")) {
      if (!mongoc_uri_parse_port(&port, portstr + 1)) {
         return false;
      }
   }

   hostname = scan_to_unichar (str + 1, ']', "", &end_host);

   mongoc_uri_do_unescape (&hostname);
   if (!hostname) {
      return false;
   }

   mongoc_uri_append_host (uri, hostname, port);
   bson_free (hostname);

   return true;
}


bool
mongoc_uri_parse_host (mongoc_uri_t  *uri,
                       const char    *str)
{
   uint16_t port;
   const char *end_host;
   char *hostname;

   if (*str == '[' && strchr (str, ']')) {
      return mongoc_uri_parse_host6 (uri, str);
   }

   if ((hostname = scan_to_unichar(str, ':', "?/,", &end_host))) {
      end_host++;
      if (!mongoc_uri_parse_port(&port, end_host)) {
         bson_free (hostname);
         return false;
      }
   } else {
      hostname = bson_strdup(str);
      port = MONGOC_DEFAULT_PORT;
   }

   mongoc_uri_do_unescape(&hostname);
   if (!hostname) {
      /* invalid */
      bson_free (hostname);
      return false;
   }

   mongoc_uri_append_host(uri, hostname, port);
   bson_free(hostname);

   return true;
}


bool
_mongoc_host_list_from_string (mongoc_host_list_t *host_list,
                               const char         *host_and_port)
{
   bool rval = false;
   char *uri_str = NULL;
   mongoc_uri_t *uri = NULL;
   const mongoc_host_list_t *uri_hl;

   BSON_ASSERT (host_list);
   BSON_ASSERT (host_and_port);

   uri_str = bson_strdup_printf("mongodb://%s/", host_and_port);
   if (! uri_str) goto CLEANUP;

   uri = mongoc_uri_new(uri_str);
   if (! uri) goto CLEANUP;

   uri_hl = mongoc_uri_get_hosts(uri);
   if (uri_hl->next) goto CLEANUP;

   memcpy(host_list, uri_hl, sizeof(*uri_hl));

   rval = true;

CLEANUP:

   bson_free(uri_str);
   if (uri) mongoc_uri_destroy(uri);

   return rval;
}


static bool
mongoc_uri_parse_hosts (mongoc_uri_t  *uri,
                        const char    *str,
                        const char   **end)
{
   bool ret = false;
   const char *end_hostport;
   const char *sock;
   const char *tmp;
   char *s;

   /*
    * Parsing the series of hosts is a lot more complicated than you might
    * imagine. This is due to some characters being both separators as well as
    * valid characters within the "hostname". In particularly, we can have file
    * paths to specify paths to UNIX domain sockets. We impose the restriction
    * that they must be suffixed with ".sock" to simplify the parsing.
    *
    * You can separate hosts and file system paths to UNIX domain sockets with
    * ",".
    *
    * When you reach a "/" or "?" that is not part of a file-system path, we
    * have completed our parsing of hosts.
    */

again:
   if (((*str == '/') && (sock = strstr(str, ".sock"))) &&
       (!(tmp = strstr(str, ",")) || (tmp > sock)) &&
       (!(tmp = strstr(str, "?")) || (tmp > sock))) {
      s = bson_strndup(str, sock + 5 - str);
      if (!mongoc_uri_parse_host(uri, s)) {
         bson_free(s);
         return false;
      }
      bson_free(s);
      str = sock + 5;
      ret = true;
      if (*str == ',') {
         str++;
         goto again;
      }
   } else if ((s = scan_to_unichar(str, ',', "/", &end_hostport))) {
      if (!mongoc_uri_parse_host(uri, s)) {
         bson_free(s);
         return false;
      }
      bson_free(s);
      str = end_hostport + 1;
      ret = true;
      goto again;
   } else if ((s = scan_to_unichar(str, '/', "", &end_hostport)) ||
              (s = scan_to_unichar(str, '?', "", &end_hostport))) {
      if (!mongoc_uri_parse_host(uri, s)) {
         bson_free(s);
         return false;
      }
      bson_free(s);
      *end = end_hostport;
      return true;
   } else if (*str) {
      if (!mongoc_uri_parse_host(uri, str)) {
         return false;
      }
      *end = str + strlen(str);
      return true;
   }

   return ret;
}


static bool
mongoc_uri_parse_database (mongoc_uri_t  *uri,
                           const char    *str,
                           const char   **end)
{
   const char *end_database;

   if ((uri->database = scan_to_unichar(str, '?', "", &end_database))) {
      *end = end_database;
   } else if (*str) {
      uri->database = bson_strdup(str);
      *end = str + strlen(str);
   }

   mongoc_uri_do_unescape(&uri->database);
   if (!uri->database) {
      /* invalid */
      return false;
   }

   return true;
}


static bool
mongoc_uri_parse_auth_mechanism_properties (mongoc_uri_t *uri,
                                            const char   *str)
{
   char *field;
   char *value;
   const char *end_scan;
   bson_t properties;

   bson_init(&properties);

   /* build up the properties document */
   while ((field = scan_to_unichar(str, ':', "&", &end_scan))) {
      str = end_scan + 1;
      if (!(value = scan_to_unichar(str, ',', ":&", &end_scan))) {
         value = bson_strdup(str);
         str = "";
      } else {
         str = end_scan + 1;
      }
      bson_append_utf8(&properties, field, -1, value, -1);
      bson_free(field);
      bson_free(value);
   }

   /* append our auth properties to our credentials */
   bson_append_document(&uri->credentials, "mechanismProperties",
                        -1, (const bson_t *)&properties);
   return true;
}

static void
mongoc_uri_parse_tags (mongoc_uri_t *uri, /* IN */
                       const char   *str) /* IN */
{
   const char *end_keyval;
   const char *end_key;
   bson_t b;
   char *keyval;
   char *key;

   bson_init(&b);

again:
   if ((keyval = scan_to_unichar(str, ',', "", &end_keyval))) {
      if ((key = scan_to_unichar(keyval, ':', "", &end_key))) {
         bson_append_utf8(&b, key, -1, end_key + 1, -1);
         bson_free(key);
      }
      bson_free(keyval);
      str = end_keyval + 1;
      goto again;
   } else {
       if ((key = scan_to_unichar(str, ':', "", &end_key))) {
         bson_append_utf8(&b, key, -1, end_key + 1, -1);
         bson_free(key);
      }
   }

   mongoc_read_prefs_add_tag(uri->read_prefs, &b);
   bson_destroy(&b);
}


/*
 *--------------------------------------------------------------------------
 *
 * mongoc_uri_bson_append_or_replace_key --
 *
 *
 *       Appends 'option' to the end of 'options' if not already set.
 *
 *       Since we cannot grow utf8 strings inline, we have to allocate a temporary
 *       bson variable and splice in the new value if the key is already set.
 *
 *       NOTE: This function keeps the order of the BSON keys.
 *
 *       NOTE: 'option' is case*in*sensitive.
 *
 *
 *--------------------------------------------------------------------------
 */

static void
mongoc_uri_bson_append_or_replace_key (bson_t *options, const char *option, const char *value)
{
   bson_iter_t iter;
   bool found = false;

   if (bson_iter_init (&iter, options)) {
      bson_t tmp = BSON_INITIALIZER;

      while (bson_iter_next (&iter)) {
         const bson_value_t *bvalue;

         if (!strcasecmp(bson_iter_key (&iter), option)) {
            bson_append_utf8(&tmp, option, -1, value, -1);
            found = true;
            continue;
         }

         bvalue = bson_iter_value (&iter);
         BSON_APPEND_VALUE (&tmp, bson_iter_key (&iter), bvalue);
      }

      if (! found) {
         bson_append_utf8(&tmp, option, -1, value, -1);
      }

      bson_destroy (options);
      bson_copy_to (&tmp, options);
      bson_destroy (&tmp);
   }
}


bool
mongoc_uri_option_is_int32 (const char *key)
{
   return !strcasecmp(key, "connecttimeoutms") ||
       !strcasecmp(key, "heartbeatfrequencyms") ||
       !strcasecmp(key, "serverselectiontimeoutms") ||
       !strcasecmp(key, "socketcheckintervalms") ||
       !strcasecmp(key, "sockettimeoutms") ||
       !strcasecmp(key, "maxpoolsize") ||
       !strcasecmp(key, "minpoolsize") ||
       !strcasecmp(key, "maxidletimems") ||
       !strcasecmp(key, "waitqueuemultiple") ||
       !strcasecmp(key, "waitqueuetimeoutms") ||
       !strcasecmp(key, "wtimeoutms");
}

bool
mongoc_uri_option_is_bool (const char *key)
{
   return !strcasecmp(key, "canonicalizeHostname") ||
              !strcasecmp(key, "journal") ||
              !strcasecmp(key, "safe") ||
              !strcasecmp(key, "serverSelectionTryOnce") ||
              !strcasecmp(key, "slaveok") ||
              !strcasecmp(key, "ssl");
}

bool
mongoc_uri_option_is_utf8 (const char *key)
{
   if (mongoc_uri_option_is_bool(key) || mongoc_uri_option_is_int32(key)) {
      return false;
   }

   if (!strcasecmp(key, "readpreferencetags") ||
         !strcasecmp(key, "authmechanismproperties")) {
      return false;
   }

   if (!strcasecmp(key, "username") || !strcasecmp(key, "password")
         || !strcasecmp(key, "authsource") || !strcasecmp(key, "database")) {
      return false;
   }

   return true;
}

static bool
mongoc_uri_parse_option (mongoc_uri_t *uri,
                         const char   *str)
{
   int32_t v_int;
   const char *end_key;
   char *key = NULL;
   char *value = NULL;
   bool ret = false;

   if (!(key = scan_to_unichar(str, '=', "", &end_key))) {
      goto CLEANUP;
   }

   value = bson_strdup(end_key + 1);
   mongoc_uri_do_unescape(&value);
   if (!value) {
      /* do_unescape detected invalid UTF-8 and freed value */
      goto CLEANUP;
   }

   if (mongoc_uri_option_is_int32(key)) {
      v_int = (int) strtol (value, NULL, 10);
      BSON_APPEND_INT32 (&uri->options, key, v_int);
   } else if (!strcasecmp(key, "w")) {
      if (*value == '-' || isdigit(*value)) {
         v_int = (int) strtol (value, NULL, 10);
         BSON_APPEND_INT32 (&uri->options, "w", v_int);
      } else if (0 == strcasecmp (value, "majority")) {
         BSON_APPEND_UTF8 (&uri->options, "w", "majority");
      } else if (*value) {
         BSON_APPEND_UTF8 (&uri->options, "w", value);
      }
   } else if (mongoc_uri_option_is_bool(key)) {
      bson_append_bool (&uri->options, key, -1,
                        (0 == strcasecmp (value, "true")) ||
                        (0 == strcasecmp (value, "t")) ||
                        (0 == strcmp (value, "1")));
   } else if (!strcasecmp(key, "readpreferencetags")) {
      mongoc_uri_parse_tags(uri, value);
   } else if (!strcasecmp(key, "authmechanism") ||
              !strcasecmp(key, "authsource")) {
      bson_append_utf8(&uri->credentials, key, -1, value, -1);
   } else if (!strcasecmp(key, "readconcernlevel")) {
      mongoc_read_concern_set_level (uri->read_concern, value);
   } else if (!strcasecmp(key, "authmechanismproperties")) {
      if (!mongoc_uri_parse_auth_mechanism_properties(uri, value)) {
         bson_free(key);
         bson_free(value);
         return false;
      }
   } else {
      bson_append_utf8(&uri->options, key, -1, value, -1);
   }

   ret = true;

CLEANUP:
   bson_free(key);
   bson_free(value);

   return ret;
}


static bool
mongoc_uri_parse_options (mongoc_uri_t *uri,
                          const char   *str)
{
   const char *end_option;
   char *option;

again:
   if ((option = scan_to_unichar(str, '&', "", &end_option))) {
      if (!mongoc_uri_parse_option(uri, option)) {
         bson_free(option);
         return false;
      }
      bson_free(option);
      str = end_option + 1;
      goto again;
   } else if (*str) {
      if (!mongoc_uri_parse_option(uri, str)) {
         return false;
      }
   }

   return true;
}


static bool
mongoc_uri_finalize_auth (mongoc_uri_t *uri) {
   bson_iter_t iter;
   const char *source = NULL;
   const char *mechanism = mongoc_uri_get_auth_mechanism(uri);

   if (bson_iter_init_find_case(&iter, &uri->credentials, "authSource")) {
      source = bson_iter_utf8(&iter, NULL);
   }

   /* authSource with GSSAPI or X509 should always be external */
   if (mechanism) {
      if (!strcasecmp(mechanism, "GSSAPI") ||
          !strcasecmp(mechanism, "MONGODB-X509")) {
         if (source) {
            if (strcasecmp(source, "$external")) {
               return false;
            }
         } else {
            bson_append_utf8(&uri->credentials, "authsource", -1, "$external", -1);
         }
      }
   }
   return true;
}

static bool
mongoc_uri_parse (mongoc_uri_t *uri,
                  const char   *str)
{
   if (!mongoc_uri_parse_scheme(str, &str)) {
      return false;
   }

   if (!*str || !mongoc_uri_parse_userpass(uri, str, &str)) {
      return false;
   }

   if (!*str || !mongoc_uri_parse_hosts(uri, str, &str)) {
      return false;
   }

   switch (*str) {
   case '/':
      str++;
      if (*str && !mongoc_uri_parse_database(uri, str, &str)) {
         return false;
      }
      if (!*str) {
         break;
      }
      /* Fall through */
   case '?':
      str++;
      if (*str && !mongoc_uri_parse_options(uri, str)) {
         return false;
      }
      break;
   default:
      break;
   }

   return mongoc_uri_finalize_auth(uri);
}


const mongoc_host_list_t *
mongoc_uri_get_hosts (const mongoc_uri_t *uri)
{
   BSON_ASSERT (uri);
   return uri->hosts;
}


const char *
mongoc_uri_get_replica_set (const mongoc_uri_t *uri)
{
   bson_iter_t iter;

   BSON_ASSERT (uri);

   if (bson_iter_init_find_case(&iter, &uri->options, "replicaSet") &&
       BSON_ITER_HOLDS_UTF8(&iter)) {
      return bson_iter_utf8(&iter, NULL);
   }

   return NULL;
}


const bson_t *
mongoc_uri_get_credentials (const mongoc_uri_t *uri)
{
   BSON_ASSERT (uri);
   return &uri->credentials;
}


const char *
mongoc_uri_get_auth_mechanism (const mongoc_uri_t *uri)
{
   bson_iter_t iter;

   BSON_ASSERT (uri);

   if (bson_iter_init_find_case (&iter, &uri->credentials, "authMechanism") &&
       BSON_ITER_HOLDS_UTF8 (&iter)) {
      return bson_iter_utf8 (&iter, NULL);
   }

   return NULL;
}


bool
mongoc_uri_get_mechanism_properties (const mongoc_uri_t *uri, bson_t *properties)
{
   bson_iter_t iter;

   if (!uri) {
      return false;
   }

   if (bson_iter_init_find_case (&iter, &uri->credentials, "mechanismProperties") &&
      BSON_ITER_HOLDS_DOCUMENT (&iter)) {
      uint32_t len = 0;
      const uint8_t *data = NULL;

      bson_iter_document (&iter, &len, &data);
      bson_init_static (properties, data, len);

      return true;
   }

   return false;
}


static void
_mongoc_uri_assign_read_prefs_mode (mongoc_uri_t *uri) /* IN */
{
   const char *str;
   bson_iter_t iter;

   BSON_ASSERT(uri);

   if (mongoc_uri_get_option_as_bool (uri, "slaveok", false)) {
      mongoc_read_prefs_set_mode(uri->read_prefs, MONGOC_READ_SECONDARY_PREFERRED);
   }

   if (bson_iter_init_find_case(&iter, &uri->options, "readpreference") &&
       BSON_ITER_HOLDS_UTF8(&iter)) {
      str = bson_iter_utf8(&iter, NULL);

      if (0 == strcasecmp("primary", str)) {
         mongoc_read_prefs_set_mode(uri->read_prefs, MONGOC_READ_PRIMARY);
      } else if (0 == strcasecmp("primarypreferred", str)) {
         mongoc_read_prefs_set_mode(uri->read_prefs, MONGOC_READ_PRIMARY_PREFERRED);
      } else if (0 == strcasecmp("secondary", str)) {
         mongoc_read_prefs_set_mode(uri->read_prefs, MONGOC_READ_SECONDARY);
      } else if (0 == strcasecmp("secondarypreferred", str)) {
         mongoc_read_prefs_set_mode(uri->read_prefs, MONGOC_READ_SECONDARY_PREFERRED);
      } else if (0 == strcasecmp("nearest", str)) {
         mongoc_read_prefs_set_mode(uri->read_prefs, MONGOC_READ_NEAREST);
      } else {
         MONGOC_WARNING("Unsupported readPreference value [readPreference=%s].", str);
      }
   }

   /* Warn on conflict, since read preference will be validated later */
   if (mongoc_read_prefs_get_mode(uri->read_prefs) == MONGOC_READ_PRIMARY &&
       !bson_empty(mongoc_read_prefs_get_tags(uri->read_prefs))) {
      MONGOC_WARNING("Primary read preference mode conflicts with tags.");
   }
}


static void
_mongoc_uri_build_write_concern (mongoc_uri_t *uri) /* IN */
{
   mongoc_write_concern_t *write_concern;
   const char *str;
   bson_iter_t iter;
   int32_t wtimeoutms;
   int value;

   BSON_ASSERT (uri);

   write_concern = mongoc_write_concern_new ();

   if (bson_iter_init_find_case (&iter, &uri->options, "safe") &&
       BSON_ITER_HOLDS_BOOL (&iter)) {
      mongoc_write_concern_set_w (write_concern,
                                  bson_iter_bool (&iter) ? 1 : MONGOC_WRITE_CONCERN_W_UNACKNOWLEDGED);
   }

   wtimeoutms = mongoc_uri_get_option_as_int32(uri, "wtimeoutms", 0);

   if (bson_iter_init_find_case (&iter, &uri->options, "journal") &&
       BSON_ITER_HOLDS_BOOL (&iter)) {
      mongoc_write_concern_set_journal (write_concern, bson_iter_bool (&iter));
   }

   if (bson_iter_init_find_case (&iter, &uri->options, "w")) {
      if (BSON_ITER_HOLDS_INT32 (&iter)) {
         value = bson_iter_int32 (&iter);

         switch (value) {
         case MONGOC_WRITE_CONCERN_W_ERRORS_IGNORED:
         case MONGOC_WRITE_CONCERN_W_UNACKNOWLEDGED:
            /* Warn on conflict, since write concern will be validated later */
            if (mongoc_write_concern_get_journal(write_concern)) {
               MONGOC_WARNING("Journal conflicts with w value [w=%d].", value);
            }
            mongoc_write_concern_set_w(write_concern, value);
            break;
         default:
            if (value > 0) {
               mongoc_write_concern_set_w (write_concern, value);
               if (value > 1) {
                  mongoc_write_concern_set_wtimeout (write_concern, wtimeoutms);
               }
               break;
            }
            MONGOC_WARNING ("Unsupported w value [w=%d].", value);
            break;
         }
      } else if (BSON_ITER_HOLDS_UTF8 (&iter)) {
         str = bson_iter_utf8 (&iter, NULL);

         if (0 == strcasecmp ("majority", str)) {
            mongoc_write_concern_set_wmajority (write_concern, wtimeoutms);
         } else {
            mongoc_write_concern_set_wtag (write_concern, str);
            mongoc_write_concern_set_wtimeout (write_concern, wtimeoutms);
         }
      } else {
         BSON_ASSERT (false);
      }
   }

   uri->write_concern = write_concern;
}


mongoc_uri_t *
mongoc_uri_new (const char *uri_string)
{
   mongoc_uri_t *uri;

   uri = (mongoc_uri_t *)bson_malloc0(sizeof *uri);
   bson_init(&uri->options);
   bson_init(&uri->credentials);

   /* Initialize read_prefs since tag parsing may add to it */
   uri->read_prefs = mongoc_read_prefs_new(MONGOC_READ_PRIMARY);

   /* Initialize empty read_concern */
   uri->read_concern = mongoc_read_concern_new ();

   if (!uri_string) {
      uri_string = "mongodb://127.0.0.1/";
   }

   if (!mongoc_uri_parse(uri, uri_string)) {
      mongoc_uri_destroy(uri);
      return NULL;
   }

   uri->str = bson_strdup(uri_string);

   _mongoc_uri_assign_read_prefs_mode(uri);

   if (!mongoc_read_prefs_is_valid(uri->read_prefs)) {
      mongoc_uri_destroy(uri);
      return NULL;
   }

   _mongoc_uri_build_write_concern (uri);

   if (!_mongoc_write_concern_is_valid(uri->write_concern)) {
      mongoc_uri_destroy(uri);
      return NULL;
   }

   return uri;
}


mongoc_uri_t *
mongoc_uri_new_for_host_port (const char *hostname,
                              uint16_t    port)
{
   mongoc_uri_t *uri;
   char *str;

   BSON_ASSERT (hostname);
   BSON_ASSERT (port);

   str = bson_strdup_printf("mongodb://%s:%hu/", hostname, port);
   uri = mongoc_uri_new(str);
   bson_free(str);

   return uri;
}


const char *
mongoc_uri_get_username (const mongoc_uri_t *uri)
{
   BSON_ASSERT (uri);

   return uri->username;
}

bool
mongoc_uri_set_username (mongoc_uri_t *uri, const char *username)
{
   size_t len;

   BSON_ASSERT (username);

   len = strlen(username);

   if (!bson_utf8_validate (username, len, false)) {
      return false;
   }

   if (uri->username) {
      bson_free (uri->username);
   }

   uri->username = bson_strdup (username);
   return true;
}


const char *
mongoc_uri_get_password (const mongoc_uri_t *uri)
{
   BSON_ASSERT (uri);

   return uri->password;
}

bool
mongoc_uri_set_password (mongoc_uri_t *uri, const char *password)
{
   size_t len;

   BSON_ASSERT (password);

   len = strlen(password);

   if (!bson_utf8_validate (password, len, false)) {
      return false;
   }

   if (uri->password) {
      bson_free (uri->password);
   }

   uri->password = bson_strdup (password);
   return true;
}


const char *
mongoc_uri_get_database (const mongoc_uri_t *uri)
{
   BSON_ASSERT (uri);
   return uri->database;
}

bool
mongoc_uri_set_database (mongoc_uri_t *uri, const char *database)
{
   size_t len;

   BSON_ASSERT (database);

   len = strlen(database);

   if (!bson_utf8_validate (database, len, false)) {
      return false;
   }

   if (uri->database) {
      bson_free (uri->database);
   }

   uri->database = bson_strdup(database);
   return true;
}


const char *
mongoc_uri_get_auth_source (const mongoc_uri_t *uri)
{
   bson_iter_t iter;

   BSON_ASSERT (uri);

   if (bson_iter_init_find_case(&iter, &uri->credentials, "authSource")) {
      return bson_iter_utf8(&iter, NULL);
   }

   return uri->database ? uri->database : "admin";
}


bool
mongoc_uri_set_auth_source (mongoc_uri_t *uri, const char *value)
{
   size_t len;

   BSON_ASSERT (value);

   len = strlen(value);

   if (!bson_utf8_validate (value, len, false)) {
      return false;
   }

   mongoc_uri_bson_append_or_replace_key (&uri->credentials, "authSource", value);

   return true;
}

const bson_t *
mongoc_uri_get_options (const mongoc_uri_t *uri)
{
   BSON_ASSERT (uri);
   return &uri->options;
}


void
mongoc_uri_destroy (mongoc_uri_t *uri)
{
   if (uri) {
      _mongoc_host_list_destroy_all (uri->hosts);
      bson_free(uri->str);
      bson_free(uri->database);
      bson_free(uri->username);
      bson_destroy(&uri->options);
      bson_destroy(&uri->credentials);
      mongoc_read_prefs_destroy(uri->read_prefs);
      mongoc_read_concern_destroy(uri->read_concern);
      mongoc_write_concern_destroy(uri->write_concern);

      if (uri->password) {
         bson_zero_free(uri->password, strlen(uri->password));
      }

      bson_free(uri);
   }
}


mongoc_uri_t *
mongoc_uri_copy (const mongoc_uri_t *uri)
{
   mongoc_uri_t       *copy;
   mongoc_host_list_t *iter;

   BSON_ASSERT (uri);

   copy = (mongoc_uri_t *)bson_malloc0(sizeof (*copy));

   copy->str      = bson_strdup (uri->str);
   copy->username = bson_strdup (uri->username);
   copy->password = bson_strdup (uri->password);
   copy->database = bson_strdup (uri->database);

   copy->read_prefs    = mongoc_read_prefs_copy (uri->read_prefs);
   copy->read_concern  = mongoc_read_concern_copy (uri->read_concern);
   copy->write_concern = mongoc_write_concern_copy (uri->write_concern);

   for (iter = uri->hosts; iter; iter = iter->next) {
      mongoc_uri_append_host (copy, iter->host, iter->port);
   }

   bson_copy_to (&uri->options, &copy->options);
   bson_copy_to (&uri->credentials, &copy->credentials);

   return copy;
}


const char *
mongoc_uri_get_string (const mongoc_uri_t *uri)
{
   BSON_ASSERT (uri);
   return uri->str;
}


const bson_t *
mongoc_uri_get_read_prefs (const mongoc_uri_t *uri)
{
   BSON_ASSERT (uri);
   return mongoc_read_prefs_get_tags(uri->read_prefs);
}


/*
 *--------------------------------------------------------------------------
 *
 * mongoc_uri_unescape --
 *
 *       Escapes an UTF-8 encoded string containing URI escaped segments
 *       such as %20.
 *
 *       It is a programming error to call this function with a string
 *       that is not UTF-8 encoded!
 *
 * Returns:
 *       A newly allocated string that should be freed with bson_free().
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

char *
mongoc_uri_unescape (const char *escaped_string)
{
   bson_unichar_t c;
   bson_string_t *str;
   unsigned int hex = 0;
   const char *ptr;
   const char *end;
   size_t len;

   BSON_ASSERT (escaped_string);

   len = strlen(escaped_string);

   /*
    * Double check that this is a UTF-8 valid string. Bail out if necessary.
    */
   if (!bson_utf8_validate(escaped_string, len, false)) {
      MONGOC_WARNING("%s(): escaped_string contains invalid UTF-8",
                     BSON_FUNC);
      return NULL;
   }

   ptr = escaped_string;
   end = ptr + len;
   str = bson_string_new(NULL);

   for (; *ptr; ptr = bson_utf8_next_char(ptr)) {
      c = bson_utf8_get_char(ptr);
      switch (c) {
      case '%':
         if (((end - ptr) < 2) ||
             !isxdigit(ptr[1]) ||
             !isxdigit(ptr[2]) ||
#ifdef _MSC_VER
             (1 != sscanf_s(&ptr[1], "%02x", &hex)) ||
#else
             (1 != sscanf(&ptr[1], "%02x", &hex)) ||
#endif
             !isprint(hex)) {
            bson_string_free(str, true);
            return NULL;
         }
         bson_string_append_c(str, hex);
         ptr += 2;
         break;
      default:
         bson_string_append_unichar(str, c);
         break;
      }
   }

   return bson_string_free(str, false);
}


const mongoc_read_prefs_t *
mongoc_uri_get_read_prefs_t (const mongoc_uri_t *uri) /* IN */
{
   BSON_ASSERT (uri);

   return uri->read_prefs;
}


const mongoc_read_concern_t *
mongoc_uri_get_read_concern (const mongoc_uri_t *uri) /* IN */
{
   BSON_ASSERT (uri);

   return uri->read_concern;
}


const mongoc_write_concern_t *
mongoc_uri_get_write_concern (const mongoc_uri_t *uri) /* IN */
{
   BSON_ASSERT (uri);

   return uri->write_concern;
}


bool
mongoc_uri_get_ssl (const mongoc_uri_t *uri) /* IN */
{
   bson_iter_t iter;

   BSON_ASSERT (uri);

   return (bson_iter_init_find_case (&iter, &uri->options, "ssl") &&
           BSON_ITER_HOLDS_BOOL (&iter) &&
           bson_iter_bool (&iter));
}

/*
 *--------------------------------------------------------------------------
 *
 * mongoc_uri_get_option_as_int32 --
 *
 *       Checks if the URI 'option' is set and of correct type (int32).
 *       The special value '0' is considered as "unset".
 *       This is so users can provide
 *       sprintf(mongodb://localhost/?option=%d, myvalue) style connection strings,
 *       and still apply default values.
 *
 *       If not set, or set to invalid type, 'fallback' is returned.
 *
 *       NOTE: 'option' is case*in*sensitive.
 *
 * Returns:
 *       The value of 'option' if available as int32 (and not 0), or 'fallback'.
 *
 *--------------------------------------------------------------------------
 */

int32_t
mongoc_uri_get_option_as_int32(const mongoc_uri_t *uri, const char *option,
                               int32_t fallback)
{
   const bson_t *options;
   bson_iter_t iter;
   int32_t retval = fallback;

   if ((options = mongoc_uri_get_options (uri)) &&
        bson_iter_init_find_case (&iter, options, option) &&
        BSON_ITER_HOLDS_INT32 (&iter)) {

      if (!(retval = bson_iter_int32(&iter))) {
         retval = fallback;
      }
   }

   return retval;
}

/*
 *--------------------------------------------------------------------------
 *
 * mongoc_uri_set_option_as_int32 --
 *
 *       Sets a URI option 'after the fact'. Allows users to set individual
 *       URI options without passing them as a connection string.
 *
 *       Only allows a set of known options to be set.
 *       @see mongoc_uri_option_is_int32 ().
 *
 *       Does in-place-update of the option BSON if 'option' is already set.
 *       Appends the option to the end otherwise.
 *
 *       NOTE: If 'option' is already set, and is of invalid type, this
 *       function will return false.
 *
 *       NOTE: 'option' is case*in*sensitive.
 *
 * Returns:
 *       true on successfully setting the option, false on failure.
 *
 *--------------------------------------------------------------------------
 */

bool
mongoc_uri_set_option_as_int32(mongoc_uri_t *uri, const char *option,
                               int32_t value)
{
   const bson_t *options;
   bson_iter_t iter;

   BSON_ASSERT (option);

   if (!mongoc_uri_option_is_int32 (option)) {
      return false;
   }

   if ((options = mongoc_uri_get_options (uri)) &&
         bson_iter_init_find_case (&iter, options, option)) {
      if (BSON_ITER_HOLDS_INT32 (&iter)) {
         bson_iter_overwrite_int32 (&iter, value);
         return true;
      } else {
         return false;
      }
   }

   bson_append_int32(&uri->options, option, -1, value);
   return true;
}

/*
 *--------------------------------------------------------------------------
 *
 * mongoc_uri_get_option_as_bool --
 *
 *       Checks if the URI 'option' is set and of correct type (bool).
 *
 *       If not set, or set to invalid type, 'fallback' is returned.
 *
 *       NOTE: 'option' is case*in*sensitive.
 *
 * Returns:
 *       The value of 'option' if available as bool, or 'fallback'.
 *
 *--------------------------------------------------------------------------
 */

bool
mongoc_uri_get_option_as_bool (const mongoc_uri_t *uri, const char *option,
                               bool fallback)
{
   const bson_t *options;
   bson_iter_t iter;

   if ((options = mongoc_uri_get_options (uri)) &&
       bson_iter_init_find_case (&iter, options, option) &&
       BSON_ITER_HOLDS_BOOL (&iter)) {
      return bson_iter_bool (&iter);
   }

   return fallback;
}

/*
 *--------------------------------------------------------------------------
 *
 * mongoc_uri_set_option_as_bool --
 *
 *       Sets a URI option 'after the fact'. Allows users to set individual
 *       URI options without passing them as a connection string.
 *
 *       Only allows a set of known options to be set.
 *       @see mongoc_uri_option_is_bool ().
 *
 *       Does in-place-update of the option BSON if 'option' is already set.
 *       Appends the option to the end otherwise.
 *
 *       NOTE: If 'option' is already set, and is of invalid type, this
 *       function will return false.
 *
 *       NOTE: 'option' is case*in*sensitive.
 *
 * Returns:
 *       true on successfully setting the option, false on failure.
 *
 *--------------------------------------------------------------------------
 */

bool
mongoc_uri_set_option_as_bool(mongoc_uri_t *uri, const char *option,
                              bool value)
{
   const bson_t *options;
   bson_iter_t iter;

   BSON_ASSERT (option);

   if (!mongoc_uri_option_is_bool (option)) {
      return false;
   }

   if ((options = mongoc_uri_get_options (uri)) &&
         bson_iter_init_find_case (&iter, options, option)) {
      if (BSON_ITER_HOLDS_BOOL (&iter)) {
         bson_iter_overwrite_bool (&iter, value);
         return true;
      } else {
         return false;
      }
   }
   bson_append_bool(&uri->options, option, -1, value);
   return true;

}

/*
 *--------------------------------------------------------------------------
 *
 * mongoc_uri_get_option_as_utf8 --
 *
 *       Checks if the URI 'option' is set and of correct type (utf8).
 *
 *       If not set, or set to invalid type, 'fallback' is returned.
 *
 *       NOTE: 'option' is case*in*sensitive.
 *
 * Returns:
 *       The value of 'option' if available as utf8, or 'fallback'.
 *
 *--------------------------------------------------------------------------
 */

const char*
mongoc_uri_get_option_as_utf8 (const mongoc_uri_t *uri, const char *option,
                               const char *fallback)
{
   const bson_t *options;
   bson_iter_t iter;

   if ((options = mongoc_uri_get_options (uri)) &&
       bson_iter_init_find_case (&iter, options, option) &&
       BSON_ITER_HOLDS_UTF8 (&iter)) {
      return bson_iter_utf8 (&iter, NULL);
   }

   return fallback;
}

/*
 *--------------------------------------------------------------------------
 *
 * mongoc_uri_set_option_as_utf8 --
 *
 *       Sets a URI option 'after the fact'. Allows users to set individual
 *       URI options without passing them as a connection string.
 *
 *       Only allows a set of known options to be set.
 *       @see mongoc_uri_option_is_utf8 ().
 *
 *       If the option is not already set, this function will append it to the end
 *       of the options bson.
 *       NOTE: If the option is already set the entire options bson will be
 *       overwritten, containing the new option=value (at the same position).
 *
 *       NOTE: If 'option' is already set, and is of invalid type, this
 *       function will return false.
 *
 *       NOTE: 'option' must be valid utf8.
 *
 *       NOTE: 'option' is case*in*sensitive.
 *
 * Returns:
 *       true on successfully setting the option, false on failure.
 *
 *--------------------------------------------------------------------------
 */

bool
mongoc_uri_set_option_as_utf8(mongoc_uri_t *uri, const char *option,
                              const char *value)
{
   size_t len;

   BSON_ASSERT (option);

   len = strlen(value);

   if (!bson_utf8_validate (value, len, false)) {
      return false;
   }

   if (!mongoc_uri_option_is_utf8 (option)) {
      return false;
   }

   mongoc_uri_bson_append_or_replace_key (&uri->options, option, value);

   return true;
}

