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

#include "json-test.h"

#ifdef _MSC_VER
#include <io.h>
#else
#include <dirent.h>
#endif


mongoc_topology_description_type_t
topology_type_from_test(const char *type)
{
   if (strcmp(type, "ReplicaSetWithPrimary") == 0) {
      return MONGOC_TOPOLOGY_RS_WITH_PRIMARY;
   } else if (strcmp(type, "ReplicaSetNoPrimary") == 0) {
      return MONGOC_TOPOLOGY_RS_NO_PRIMARY;
   } else if (strcmp(type, "Unknown") == 0) {
      return MONGOC_TOPOLOGY_UNKNOWN;
   } else if (strcmp(type, "Single") == 0) {
      return MONGOC_TOPOLOGY_SINGLE;
   } else if (strcmp(type, "Sharded") == 0) {
      return MONGOC_TOPOLOGY_SHARDED;
   }

   fprintf(stderr, "can't parse this: %s", type);
   assert(0);
   return 0;
}

mongoc_server_description_type_t
server_type_from_test(const char *type)
{
   if (strcmp(type, "RSPrimary") == 0) {
      return MONGOC_SERVER_RS_PRIMARY;
   } else if (strcmp(type, "RSSecondary") == 0) {
      return MONGOC_SERVER_RS_SECONDARY;
   } else if (strcmp(type, "Standalone") == 0) {
      return MONGOC_SERVER_STANDALONE;
   } else if (strcmp(type, "Mongos") == 0) {
      return MONGOC_SERVER_MONGOS;
   } else if (strcmp(type, "PossiblePrimary") == 0) {
      return MONGOC_SERVER_POSSIBLE_PRIMARY;
   } else if (strcmp(type, "RSArbiter") == 0) {
      return MONGOC_SERVER_RS_ARBITER;
   } else if (strcmp(type, "RSOther") == 0) {
      return MONGOC_SERVER_RS_OTHER;
   } else if (strcmp(type, "RSGhost") == 0) {
      return MONGOC_SERVER_RS_GHOST;
   } else if (strcmp(type, "Unknown") == 0) {
      return MONGOC_SERVER_UNKNOWN;
   }
   fprintf(stderr, "ERROR: Unknown server type %s\n", type);
   assert(0);
   return 0;
}

const char *
topology_type_to_string(mongoc_topology_description_type_t type)
{
   switch(type) {
   case MONGOC_TOPOLOGY_UNKNOWN:
      return "Unknown";
   case MONGOC_TOPOLOGY_SHARDED:
      return "Sharded";
   case MONGOC_TOPOLOGY_RS_NO_PRIMARY:
      return "ReplicaSetNoPrimary";
   case MONGOC_TOPOLOGY_RS_WITH_PRIMARY:
      return "ReplicaSetWithPrimary";
   case MONGOC_TOPOLOGY_SINGLE:
      return "Single";
   case MONGOC_TOPOLOGY_DESCRIPTION_TYPES:
   default:
      fprintf(stderr, "ERROR: Unknown topology state\n");
      assert(0);
   }

   return NULL;
}

/*
 *-----------------------------------------------------------------------
 *
 * assemble_path --
 *
 *       Given a parent directory and filename, compile a full path to
 *       the child file.
 *
 *-----------------------------------------------------------------------
 */
void
assemble_path (const char *parent_path,
               const char *child_name,
               char       *dst /* OUT */)
{
   int path_len = (int)strlen(parent_path);
   int name_len = (int)strlen(child_name);

   assert(path_len + name_len + 1 < MAX_TEST_NAME_LENGTH);

   memset(dst, '\0', MAX_TEST_NAME_LENGTH * sizeof(char));
   strncat(dst, parent_path, path_len);
   strncat(dst, "/", 1);
   strncat(dst, child_name, name_len);
}

/*
 *-----------------------------------------------------------------------
 *
 * collect_tests_from_dir --
 *
 *       Recursively search the directory at @dir_path for files with
 *       '.json' in their filenames. Append all found file paths to
 *       @paths, and return the number of files found.
 *
 *-----------------------------------------------------------------------
 */
int
collect_tests_from_dir (char (*paths)[MAX_TEST_NAME_LENGTH] /* OUT */,
                        const char *dir_path,
                        int paths_index,
                        int max_paths)
{
#ifdef _MSC_VER
   intptr_t handle;
   struct _finddata_t info;

   char child_path[MAX_TEST_NAME_LENGTH];

   handle = _findfirst(dir_path, &info);

   if (handle == -1) {
      return 0;
   }

   while (1) {
      assert(paths_index < max_paths);

      if (_findnext(handle, &info) == -1) {
         break;
      }

      if (info.attrib & _A_SUBDIR) {
         /* recursively call on child directories */
         if (strcmp (info.name, "..") != 0 &&
             strcmp (info.name, ".") != 0) {

            assemble_path(dir_path, info.name, child_path);
            paths_index = collect_tests_from_dir(paths, child_path, paths_index, max_paths);
         }
      } else if (strstr(info.name, ".json")) {
         /* if this is a JSON test, collect its path */
         assemble_path(dir_path, info.name, paths[paths_index++]);
      }
   }

   _findclose(handle);

   return paths_index;
#else
   struct dirent *entry;
   struct stat dir_stat;
   char child_path[MAX_TEST_NAME_LENGTH];
   DIR *dir;

   dir = opendir(dir_path);
   assert (dir);
   while ((entry = readdir(dir))) {
      assert(paths_index < max_paths);

      if (0 == stat(entry->d_name, &dir_stat) && dir_stat.st_mode & S_IFDIR) {
         /* recursively call on child directories */
         if (strcmp (entry->d_name, "..") != 0 &&
             strcmp (entry->d_name, ".") != 0) {

            assemble_path(dir_path, entry->d_name, child_path);
            paths_index = collect_tests_from_dir(paths, child_path, paths_index,
                                                 max_paths);
         }
      } else if (strstr(entry->d_name, ".json")) {
         /* if this is a JSON test, collect its path */
         assemble_path(dir_path, entry->d_name, paths[paths_index++]);
      }
   }

   closedir(dir);

   return paths_index;
#endif
}

/*
 *-----------------------------------------------------------------------
 *
 * get_bson_from_json_file --
 *
 *        Open the file at @filename and store its contents in a
 *        bson_t. This function assumes that @filename contains a
 *        single JSON object.
 *
 *        NOTE: caller owns returned bson_t and must free it.
 *
 *-----------------------------------------------------------------------
 */
bson_t *
get_bson_from_json_file(char *filename)
{
   FILE *file;
   long length;
   bson_t *data;
   bson_error_t error;
   const char *buffer;

   file = fopen(filename, "r");
   if (!file) {
      return NULL;
   }

   /* get file length */
   fseek(file, 0, SEEK_END);
   length = ftell(file);
   fseek(file, 0, SEEK_SET);
   if (length < 1) {
      return NULL;
   }

   /* read entire file into buffer */
   buffer = (const char *)bson_malloc0(length);
   if (fread((void *)buffer, 1, length, file) != length) {
      abort();
   }

   fclose(file);
   if (!buffer) {
      return NULL;
   }

   /* convert to bson */
   data = bson_new_from_json((const uint8_t*)buffer, length, &error);
   bson_free((void *)buffer);
   if (!data) {
      return NULL;
   }
   return data;
}

/*
 *-----------------------------------------------------------------------
 *
 * run_json_test_suite --
 *
 *      Given a path to a directory containing JSON tests, import each
 *      test into a BSON blob and call the provided callback for
 *      evaluation.
 *
 *      It is expected that the callback will assert on failure, so if
 *      callback returns quietly the test is considered to have passed.
 *
 *-----------------------------------------------------------------------
 */
void
install_json_test_suite(TestSuite *suite, const char *dir_path, test_hook callback)
{
   char test_paths[MAX_NUM_TESTS][MAX_TEST_NAME_LENGTH];
   int num_tests;
   int i;
   bson_t *test;
   char *skip_json;
   char *ext;

   num_tests = collect_tests_from_dir(&test_paths[0],
                                      dir_path,
                                      0, MAX_NUM_TESTS);

   for (i = 0; i < num_tests; i++) {
      test = get_bson_from_json_file(test_paths[i]);

      if (test) {
         skip_json = strstr(test_paths[i], "/json") + strlen("/json");
         assert(skip_json);
         ext = strstr (skip_json, ".json");
         assert(ext);
         ext[0] = '\0';

         TestSuite_AddWC(suite, skip_json, (void (*)(void *))callback, (void (*)(void*))bson_destroy, test);
      } else {
         fprintf(stderr, "NO DATA\n");
      }
   }
}
