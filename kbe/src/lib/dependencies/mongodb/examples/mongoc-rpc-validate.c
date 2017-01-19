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


/*
 * This example can only be used internally to the library as it uses
 * private features that are not exported in the public ABI. It does,
 * however, illustrate some of the internals of the system.
 */


#include <fcntl.h>
#include <mongoc.h>
#include <mongoc-array-private.h>
#include <mongoc-rpc-private.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>


static void
validate (const char *name) /* IN */
{
   mongoc_stream_t *stream;
   mongoc_rpc_t rpc;
#ifdef _WIN32
   struct _stat st;
#else
   struct stat st;
#endif
   uint8_t *buf;
   int32_t len;
   int ret;

   stream = mongoc_stream_file_new_for_path (name, O_RDONLY, 0);
   if (!stream) {
      perror ("failed to open file");
      exit (EXIT_FAILURE);
   }

#ifdef _WIN32
   ret = _stat (name, &st);
#else
   ret = stat (name, &st);
#endif
   if (ret != 0) {
      perror ("failed to stat() file.");
      exit (EXIT_FAILURE);
   }

   if ((st.st_size > (100 * 1024 * 1024)) || (st.st_size < 16)) {
      fprintf (stderr, "%s: unreasonable message size\n", name);
      exit (EXIT_FAILURE);
   }

   buf = (uint8_t *)bson_malloc (st.st_size);
   if (buf == NULL) {
      fprintf (stderr, "%s: Failed to malloc %d bytes.\n",
               name, (int)st.st_size);
      exit (EXIT_FAILURE);
   }

   if (st.st_size != mongoc_stream_read (stream, buf, st.st_size, st.st_size, -1)) {
      fprintf (stderr, "%s: Failed to read %d bytes into buffer.\n",
               name, (int)st.st_size);
      exit (EXIT_FAILURE);
   }

   memcpy (&len, buf, 4);
   len = BSON_UINT32_FROM_LE (len);
   if (len != st.st_size) {
      fprintf (stderr, "%s is invalid. Invalid Length.\n", name);
      exit (EXIT_FAILURE);
   }

   if (!_mongoc_rpc_scatter (&rpc, buf, st.st_size)) {
      fprintf (stderr, "%s is invalid. Invalid Format.\n", name);
      exit (EXIT_FAILURE);
   }

   fprintf (stdout, "%s is valid.\n", name);

   bson_free (buf);
}


int
main (int   argc,
      char *argv[])
{
   int i;

   if (argc < 2) {
      fprintf (stderr, "usage: %s FILE...\n", argv[0]);
      return EXIT_FAILURE;
   }

   mongoc_init ();

   for (i = 1; i < argc; i++) {
      validate (argv [i]);
   }

   return EXIT_SUCCESS;
}
