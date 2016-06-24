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


#include <bson.h>

#include "mongoc-array-private.h"
#include "mongoc-util-private.h"

#include "test-conveniences.h"

static bool gConveniencesInitialized = false;
static mongoc_array_t gTmpBsonArray;

static void test_conveniences_cleanup ();


static void
test_conveniences_init ()
{
   if (!gConveniencesInitialized) {
      _mongoc_array_init (&gTmpBsonArray, sizeof (bson_t *));
      atexit (test_conveniences_cleanup);
      gConveniencesInitialized = true;
   }
}


static void
test_conveniences_cleanup ()
{
   int i;
   bson_t *doc;

   if (gConveniencesInitialized) {
      for (i = 0; i < gTmpBsonArray.len; i++) {
         doc = _mongoc_array_index (&gTmpBsonArray, bson_t *, i);
         bson_destroy (doc);
      }

      _mongoc_array_destroy (&gTmpBsonArray);
   }
}


bson_t *
tmp_bson (const char *json)
{
   bson_error_t error;
   char *double_quoted;
   bson_t *doc;

   test_conveniences_init ();

   if (json) {
      double_quoted = single_quotes_to_double (json);
      doc = bson_new_from_json ((const uint8_t *) double_quoted,
                                -1, &error);

      if (!doc) {
         fprintf (stderr, "%s\n", error.message);
         abort ();
      }

      bson_free (double_quoted);

   } else {
      doc = bson_new ();
   }

   _mongoc_array_append_val (&gTmpBsonArray, doc);

   return doc;
}


/*--------------------------------------------------------------------------
 *
 * bson_iter_bson --
 *
 *       Statically init a bson_t from an iter at an array or document.
 *
 *--------------------------------------------------------------------------
 */
void
bson_iter_bson (const bson_iter_t *iter,
                bson_t *bson)
{
   uint32_t len;
   const uint8_t *data;

   assert (BSON_ITER_HOLDS_DOCUMENT (iter) || BSON_ITER_HOLDS_ARRAY (iter));

   if (BSON_ITER_HOLDS_DOCUMENT (iter)) {
      bson_iter_document (iter, &len, &data);
   } else {
      bson_iter_array (iter, &len, &data);
   }

   assert (bson_init_static (bson, data, len));
}


static bool
get_exists_operator (const bson_value_t *value,
                     bool               *exists);

static bool
get_empty_operator (const bson_value_t *value,
                    bool               *exists);

static bool
is_empty_doc_or_array (const bson_value_t *value);

static bool
find (bson_value_t *value,
      const bson_iter_t *iter,
      const char *key,
      bool is_command,
      bool is_first);

static bool
match_bson_value (const bson_value_t *doc,
                  const bson_value_t *pattern);

/*--------------------------------------------------------------------------
 *
 * single_quotes_to_double --
 *
 *       Copy str with single-quotes replaced by double.
 *
 * Returns:
 *       A string you must bson_free.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

char *
single_quotes_to_double (const char *str)
{
   char *result = bson_strdup (str);
   char *p;

   for (p = result; *p; p++) {
      if (*p == '\'') {
         *p = '"';
      }
   }

   return result;
}


/*--------------------------------------------------------------------------
 *
 * match_json --
 *
 *       Call match_bson on "doc" and "json_pattern".
 *       For convenience, single-quotes are synonymous with double-quotes.
 *
 *       A NULL doc or NULL json_pattern means "{}".
 *
 * Returns:
 *       True or false.
 *
 * Side effects:
 *       Logs if no match.
 *
 *--------------------------------------------------------------------------
 */

bool
match_json (const bson_t *doc,
            bool          is_command,
            const char   *filename,
            int           lineno,
            const char   *funcname,
            const char   *json_pattern,
            ...)
{
   va_list args;
   char *json_pattern_formatted;
   char *double_quoted;
   bson_error_t error;
   bson_t *pattern;
   bool matches;

   va_start (args, json_pattern);
   json_pattern_formatted = bson_strdupv_printf (
      json_pattern ? json_pattern : "{}", args);
   va_end (args);

   double_quoted = single_quotes_to_double (json_pattern_formatted);


   pattern = bson_new_from_json ((const uint8_t *) double_quoted, -1, &error);

   if (!pattern) {
      fprintf (stderr, "couldn't parse JSON: %s\n", error.message);
      abort ();
   }

   matches = match_bson (doc, pattern, is_command);

   if (!matches) {
      fprintf (stderr,
            "ASSERT_MATCH failed with document:\n\n"
                  "%s\n"
                  "pattern:\n%s\n\n"
                  "%s:%d %s()\n",
            doc ? bson_as_json (doc, NULL) : "{}",
            double_quoted,
            filename, lineno, funcname);
   }

   bson_destroy (pattern);
   bson_free (json_pattern_formatted);
   bson_free (double_quoted);

   return matches;
}


/*--------------------------------------------------------------------------
 *
 * match_bson --
 *
 *       Does "doc" match "pattern"?
 *
 *       mongoc_matcher_t prohibits $-prefixed keys, which is something
 *       we need to test in e.g. test_mongoc_client_read_prefs, so this
 *       does *not* use mongoc_matcher_t. Instead, "doc" matches "pattern"
 *       if its key-value pairs are a simple superset of pattern's. Order
 *       matters.
 *       
 *       The only special pattern syntaxes are "field": {"$exists": true/false}
 *       and "field": {"$empty": true/false}.
 *
 *       The first key matches case-insensitively if is_command.
 *
 *       A NULL doc or NULL pattern means "{}".
 *
 * Returns:
 *       True or false.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

bool
match_bson (const bson_t *doc,
            const bson_t *pattern,
            bool          is_command)
{
   bson_iter_t pattern_iter;
   const char *key;
   const bson_value_t *value;
   bson_iter_t doc_iter;
   bool is_first = true;
   bool is_exists_operator;
   bool is_empty_operator;
   bool exists;
   bool empty;
   bool found;
   bson_value_t doc_value;

   if (bson_empty0 (pattern)) {
      /* matches anything */
      return true;
   }

   if (bson_empty0 (doc)) {
      /* non-empty pattern can't match doc */
      return false;
   }

   assert (bson_iter_init (&pattern_iter, pattern));
   assert (bson_iter_init (&doc_iter, doc));

   while (bson_iter_next (&pattern_iter)) {
      key = bson_iter_key (&pattern_iter);
      value = bson_iter_value (&pattern_iter);
      found = find (&doc_value, &doc_iter, key, is_command, is_first);

      /* is value {"$exists": true} or {"$exists": false} ? */
      is_exists_operator = get_exists_operator (value, &exists);

      /* is value {"$empty": true} or {"$empty": false} ? */
      is_empty_operator = get_empty_operator (value, &empty);

      if (is_exists_operator) {
         if (exists != found) {
            return false;
         }
      } else if (!found) {
         return false;
      } else if (is_empty_operator) {
         if (empty != is_empty_doc_or_array (&doc_value)) {
            return false;
         }
      } else if (!match_bson_value (&doc_value, value)) {
         return false;
      }

      /* don't advance if next call may be for another key in the same subdoc,
       * or if we're skipping a pattern key that was {$exists: false}. */
      if (!strchr (key, '.') && !(is_exists_operator && !exists)) {
         bson_iter_next (&doc_iter);
      }

      is_first = false;
      if (found) {
         bson_value_destroy (&doc_value);
      }
   }

   return true;
}


/*--------------------------------------------------------------------------
 *
 * find --
 *
 *       Find the value for a key.
 *
 * Returns:
 *       Whether the key was found.
 *
 * Side effects:
 *       Copies the found value into "value".
 *
 *--------------------------------------------------------------------------
 */

static bool
find (bson_value_t *value,
      const bson_iter_t *iter,
      const char *key,
      bool is_command,
      bool is_first)
{
   bson_iter_t i2;
   bson_iter_t descendent;

   /* don't advance iter. */
   memcpy (&i2, iter, sizeof *iter);

   if (strchr (key, '.')) {
      if (!bson_iter_find_descendant (&i2, key, &descendent)) {
         return false;
      }

      bson_value_copy(bson_iter_value (&descendent), value);
      return true;
   } else if (is_command && is_first) {
      if (!bson_iter_find_case (&i2, key)) {
         return false;
      }
   } else if (!bson_iter_find (&i2, key)) {
      return false;
   }

   bson_value_copy (bson_iter_value (&i2), value);
   return true;
}


static bool
bson_init_from_value (bson_t             *b,
                      const bson_value_t *v)
{
   assert (v->value_type == BSON_TYPE_ARRAY ||
           v->value_type == BSON_TYPE_DOCUMENT);

   return bson_init_static (b, v->value.v_doc.data, v->value.v_doc.data_len);
}


static bool
_is_operator (const char         *op_name,
              const bson_value_t *value,
              bool               *op_val)
{
   bson_t bson;
   bson_iter_t iter;

   if (value->value_type == BSON_TYPE_DOCUMENT &&
       bson_init_from_value (&bson, value) &&
       bson_iter_init_find (&iter, &bson, op_name)) {
      *op_val = bson_iter_as_bool (&iter);
      return true;
   }

   return false;
}


/*--------------------------------------------------------------------------
 *
 * get_exists_operator --
 *
 *       Is value a subdocument like {"$exists": bool}?
 *
 * Returns:
 *       True if the value is a subdocument with the first key "$exists".
 *
 * Side effects:
 *       If the function returns true, *exists is set to true or false,
 *       the value of the bool.
 *
 *--------------------------------------------------------------------------
 */

static bool
get_exists_operator (const bson_value_t *value,
                     bool               *exists)
{
   return _is_operator ("$exists", value, exists);
}


/*--------------------------------------------------------------------------
 *
 * get_empty_operator --
 *
 *       Is value a subdocument like {"$empty": bool}?
 *
 * Returns:
 *       True if the value is a subdocument with the first key "$empty".
 *
 * Side effects:
 *       If the function returns true, *empty is set to true or false,
 *       the value of the bool.
 *
 *--------------------------------------------------------------------------
 */

bool
get_empty_operator (const bson_value_t *value,
                    bool               *empty)
{
   return _is_operator ("$empty", value, empty);
}


/*--------------------------------------------------------------------------
 *
 * is_empty_doc_or_array --
 *
 *       Is value the subdocument {} or the array []?
 *
 *--------------------------------------------------------------------------
 */

static bool
is_empty_doc_or_array (const bson_value_t *value)
{
   bson_t doc;

   if (!(value->value_type == BSON_TYPE_ARRAY ||
         value->value_type == BSON_TYPE_DOCUMENT)) {
      return false;
   }
   assert (bson_init_static (&doc,
                             value->value.v_doc.data,
                             value->value.v_doc.data_len));

   return bson_count_keys (&doc) == 0;
}


static bool
match_bson_arrays (const bson_t *array,
                   const bson_t *pattern)
{
   /* an array is just a document with keys "0", "1", ...
    * so match_bson suffices if the number of keys is equal. */
   if (bson_count_keys (array) != bson_count_keys (pattern)) {
      return false;
   }
   
   return match_bson (array, pattern, false);
}


static bool
match_bson_value (const bson_value_t *doc,
                  const bson_value_t *pattern)
{
   bson_t subdoc;
   bson_t pattern_subdoc;
   bool ret;

   if (doc->value_type != pattern->value_type) {
      return false;
   }
   
   switch (doc->value_type) {
   case BSON_TYPE_ARRAY:
   case BSON_TYPE_DOCUMENT:

      if (!bson_init_from_value (&subdoc, doc)) {
         return false;
      }

      if (!bson_init_from_value (&pattern_subdoc, pattern)) {
         bson_destroy (&subdoc);
         return false;
      }

      if (doc->value_type == BSON_TYPE_ARRAY) {
         ret = match_bson_arrays (&subdoc, &pattern_subdoc);
      } else {
         ret = match_bson (&subdoc, &pattern_subdoc, false);
      }

      bson_destroy (&subdoc);
      bson_destroy (&pattern_subdoc);

      return ret;

   case BSON_TYPE_BINARY:
      return doc->value.v_binary.data_len == pattern->value.v_binary.data_len &&
             !memcmp (doc->value.v_binary.data,
                      pattern->value.v_binary.data,
                      doc->value.v_binary.data_len);

   case BSON_TYPE_BOOL:
      return doc->value.v_bool == pattern->value.v_bool;

   case BSON_TYPE_CODE:
      return doc->value.v_code.code_len == pattern->value.v_code.code_len &&
             !memcmp (doc->value.v_code.code,
                      pattern->value.v_code.code,
                      doc->value.v_code.code_len);

   case BSON_TYPE_CODEWSCOPE:
      return doc->value.v_codewscope.code_len ==
                pattern->value.v_codewscope.code_len &&
             !memcmp (doc->value.v_codewscope.code,
                      pattern->value.v_codewscope.code,
                      doc->value.v_codewscope.code_len) &&
             doc->value.v_codewscope.scope_len ==
                pattern->value.v_codewscope.scope_len
             && !memcmp (doc->value.v_codewscope.scope_data,
                         pattern->value.v_codewscope.scope_data,
                         doc->value.v_codewscope.scope_len);

   case BSON_TYPE_DATE_TIME:
      return doc->value.v_datetime == pattern->value.v_datetime;
   case BSON_TYPE_DOUBLE:
      return doc->value.v_double == pattern->value.v_double;
   case BSON_TYPE_INT32:
      return doc->value.v_int32 == pattern->value.v_int32;
   case BSON_TYPE_INT64:
      return doc->value.v_int64 == pattern->value.v_int64;
   case BSON_TYPE_OID:
      return bson_oid_equal (&doc->value.v_oid, &pattern->value.v_oid);
   case BSON_TYPE_REGEX:
      return !strcmp (doc->value.v_regex.regex,
                      pattern->value.v_regex.regex) &&
             !strcmp (doc->value.v_regex.options,
                      pattern->value.v_regex.options);
   case BSON_TYPE_SYMBOL:
      return doc->value.v_symbol.len == pattern->value.v_symbol.len &&
             !strncmp (doc->value.v_symbol.symbol,
                       pattern->value.v_symbol.symbol,
                       doc->value.v_symbol.len);
   case BSON_TYPE_TIMESTAMP:
      return doc->value.v_timestamp.timestamp ==
                pattern->value.v_timestamp.timestamp &&
             doc->value.v_timestamp.increment ==
                pattern->value.v_timestamp.increment;
   case BSON_TYPE_UTF8:
      return doc->value.v_utf8.len == pattern->value.v_utf8.len &&
             !strncmp (doc->value.v_utf8.str,
                       pattern->value.v_utf8.str,
                       doc->value.v_utf8.len);

   /* these are empty types, if "a" and "b" are the same type they're equal */
   case BSON_TYPE_EOD:
   case BSON_TYPE_MAXKEY:
   case BSON_TYPE_MINKEY:
   case BSON_TYPE_NULL:
   case BSON_TYPE_UNDEFINED:
      return true;

   case BSON_TYPE_DBPOINTER:
      fprintf (stderr, "DBPointer comparison not implemented");
      abort ();

   default:
      fprintf (stderr, "unexpected value type %d", doc->value_type);
      abort ();
   }
}
