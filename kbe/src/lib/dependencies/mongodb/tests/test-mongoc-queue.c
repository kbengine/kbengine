#include <mongoc.h>
#include <mongoc-queue-private.h>

#include "TestSuite.h"


static void
test_mongoc_queue_basic (void)
{
   mongoc_queue_t q = MONGOC_QUEUE_INITIALIZER;

   _mongoc_queue_push_head(&q, (void *)1);
   _mongoc_queue_push_tail(&q, (void *)2);
   _mongoc_queue_push_head(&q, (void *)3);
   _mongoc_queue_push_tail(&q, (void *)4);
   _mongoc_queue_push_head(&q, (void *)5);

   ASSERT_CMPINT(_mongoc_queue_get_length(&q), ==, 5);

   ASSERT(_mongoc_queue_pop_head(&q) == (void *)5);
   ASSERT(_mongoc_queue_pop_head(&q) == (void *)3);
   ASSERT(_mongoc_queue_pop_head(&q) == (void *)1);
   ASSERT(_mongoc_queue_pop_head(&q) == (void *)2);
   ASSERT(_mongoc_queue_pop_head(&q) == (void *)4);
   ASSERT(!_mongoc_queue_pop_head(&q));
}


void
test_queue_install (TestSuite *suite)
{
   TestSuite_Add (suite, "/Queue/basic", test_mongoc_queue_basic);
}
