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
 * This program will print each BSON document contained in the provided files
 * as a JSON string to STDOUT.
 */


#include <bson.h>
#include <stdio.h>


int
main (int   argc,
      char *argv[])
{
   bson_reader_t *reader;
   const bson_t *b;
   bson_error_t error;
   const char *filename;
   char *str;
   int i;

   /*
    * Print program usage if no arguments are provided.
    */
   if (argc == 1) {
      fprintf(stderr,
              "usage: %s [FILE | -]...\nUse - for STDIN.\n",
              argv[0]);
      return 1;
   }

   /*
    * Process command line arguments expecting each to be a filename.
    */
   for (i = 1; i < argc; i++) {
      filename = argv[i];

      if (strcmp (filename, "-") == 0) {
         reader = bson_reader_new_from_fd (STDIN_FILENO, false);
      } else {
         if (!(reader = bson_reader_new_from_file (filename, &error))) {
            fprintf (stderr, "Failed to open \"%s\": %s\n",
                     filename, error.message);
            continue;
         }
      }

      /*
       * Convert each incoming document to JSON and print to stdout.
       */
      while ((b = bson_reader_read (reader, NULL))) {
         str = bson_as_json(b, NULL);
         fprintf(stdout, "%s\n", str);
         bson_free(str);
      }

      /*
       * Cleanup after our reader, which closes the file descriptor.
       */
      bson_reader_destroy (reader);
   }

   return 0;
}
