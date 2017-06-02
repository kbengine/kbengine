#include <assert.h>
#include <bcon.h>
#include <mongoc.h>
#include <stdio.h>

static void
bulk2 (mongoc_collection_t *collection)
{
   mongoc_bulk_operation_t *bulk;
   bson_error_t error;
   bson_t *query;
   bson_t *doc;
   bson_t reply;
   char *str;
   bool ret;
   int i;

   bulk = mongoc_collection_create_bulk_operation (collection, true, NULL);

   /* Remove everything */
   query = bson_new ();
   mongoc_bulk_operation_remove (bulk, query);
   bson_destroy (query);

   /* Add a few documents */
   for (i = 1; i < 4; i++) {
      doc = BCON_NEW ("_id", BCON_INT32 (i));
      mongoc_bulk_operation_insert (bulk, doc);
      bson_destroy (doc);
   }

   /* {_id: 1} => {$set: {foo: "bar"}} */
   query = BCON_NEW ("_id", BCON_INT32 (1));
   doc = BCON_NEW ("$set", "{", "foo", BCON_UTF8 ("bar"), "}");
   mongoc_bulk_operation_update (bulk, query, doc, false);
   bson_destroy (query);
   bson_destroy (doc);

   /* {_id: 4} => {'$inc': {'j': 1}} (upsert) */
   query = BCON_NEW ("_id", BCON_INT32 (4));
   doc = BCON_NEW ("$inc", "{", "j", BCON_INT32 (1), "}");
   mongoc_bulk_operation_update (bulk, query, doc, true);
   bson_destroy (query);
   bson_destroy (doc);

   /* replace {j:1} with {j:2} */
   query = BCON_NEW ("j", BCON_INT32 (1));
   doc = BCON_NEW ("j", BCON_INT32 (2));
   mongoc_bulk_operation_replace_one (bulk, query, doc, false);
   bson_destroy (query);
   bson_destroy (doc);

   ret = mongoc_bulk_operation_execute (bulk, &reply, &error);

   str = bson_as_json (&reply, NULL);
   printf ("%s\n", str);
   bson_free (str);

   if (!ret) {
      printf ("Error: %s\n", error.message);
   }

   bson_destroy (&reply);
   mongoc_bulk_operation_destroy (bulk);
}

int
main (int argc,
      char *argv[])
{
   mongoc_client_t *client;
   mongoc_collection_t *collection;

   mongoc_init ();

   client = mongoc_client_new ("mongodb://localhost/");
   collection = mongoc_client_get_collection (client, "test", "test");

   bulk2 (collection);

   mongoc_collection_destroy (collection);
   mongoc_client_destroy (client);

   mongoc_cleanup ();

   return 0;
}
