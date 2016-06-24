#include <mongoc.h>
#include <mongoc-write-concern-private.h>

#include "TestSuite.h"


static void
test_write_concern_basic (void)
{
   mongoc_write_concern_t *write_concern;
   const bson_t *gle;
   const bson_t *bson;
   bson_iter_t iter;

   write_concern = mongoc_write_concern_new();

   /*
    * Test defaults.
    */
   ASSERT(write_concern);
   ASSERT(!mongoc_write_concern_get_fsync(write_concern));
   ASSERT(!mongoc_write_concern_get_journal(write_concern));
   ASSERT(mongoc_write_concern_get_w(write_concern) == MONGOC_WRITE_CONCERN_W_DEFAULT);
   ASSERT(!mongoc_write_concern_get_wtimeout(write_concern));
   ASSERT(!mongoc_write_concern_get_wmajority(write_concern));

   mongoc_write_concern_set_fsync(write_concern, true);
   ASSERT(mongoc_write_concern_get_fsync(write_concern));
   mongoc_write_concern_set_fsync(write_concern, false);
   ASSERT(!mongoc_write_concern_get_fsync(write_concern));

   mongoc_write_concern_set_journal(write_concern, true);
   ASSERT(mongoc_write_concern_get_journal(write_concern));
   mongoc_write_concern_set_journal(write_concern, false);
   ASSERT(!mongoc_write_concern_get_journal(write_concern));

   /*
    * Test changes to w.
    */
   mongoc_write_concern_set_w(write_concern, MONGOC_WRITE_CONCERN_W_MAJORITY);
   ASSERT(mongoc_write_concern_get_wmajority(write_concern));
   mongoc_write_concern_set_w(write_concern, MONGOC_WRITE_CONCERN_W_DEFAULT);
   ASSERT(!mongoc_write_concern_get_wmajority(write_concern));
   mongoc_write_concern_set_wmajority(write_concern, 1000);
   ASSERT(mongoc_write_concern_get_wmajority(write_concern));
   ASSERT(mongoc_write_concern_get_wtimeout(write_concern) == 1000);
   mongoc_write_concern_set_wtimeout(write_concern, 0);
   ASSERT(!mongoc_write_concern_get_wtimeout(write_concern));
   mongoc_write_concern_set_w(write_concern, MONGOC_WRITE_CONCERN_W_DEFAULT);
   ASSERT(mongoc_write_concern_get_w(write_concern) == MONGOC_WRITE_CONCERN_W_DEFAULT);
   mongoc_write_concern_set_w(write_concern, 3);
   ASSERT(mongoc_write_concern_get_w(write_concern) == 3);

   /*
    * Check generated bson.
    */
   mongoc_write_concern_set_fsync(write_concern, true);
   mongoc_write_concern_set_journal(write_concern, true);
   gle = _mongoc_write_concern_get_gle(write_concern);
   ASSERT(bson_iter_init_find(&iter, gle, "getlasterror") && BSON_ITER_HOLDS_INT32(&iter) && bson_iter_int32(&iter) == 1);
   ASSERT(bson_iter_init_find(&iter, gle, "fsync") && BSON_ITER_HOLDS_BOOL(&iter) && bson_iter_bool(&iter));
   ASSERT(bson_iter_init_find(&iter, gle, "j") && BSON_ITER_HOLDS_BOOL(&iter) && bson_iter_bool(&iter));
   ASSERT(bson_iter_init_find(&iter, gle, "w") && BSON_ITER_HOLDS_INT32(&iter) && bson_iter_int32(&iter) == 3);
   ASSERT(gle);

   bson = _mongoc_write_concern_get_bson(write_concern);
   ASSERT(!bson_iter_init_find(&iter, bson, "getlasterror"));
   ASSERT(bson_iter_init_find(&iter, bson, "fsync") && BSON_ITER_HOLDS_BOOL(&iter) && bson_iter_bool(&iter));
   ASSERT(bson_iter_init_find(&iter, bson, "j") && BSON_ITER_HOLDS_BOOL(&iter) && bson_iter_bool(&iter));
   ASSERT(bson_iter_init_find(&iter, bson, "w") && BSON_ITER_HOLDS_INT32(&iter) && bson_iter_int32(&iter) == 3);
   ASSERT(bson);

   mongoc_write_concern_destroy(write_concern);
}


static void
test_write_concern_bson_omits_defaults (void)
{
   mongoc_write_concern_t *write_concern;
   const bson_t *gle;
   const bson_t *bson;
   bson_iter_t iter;

   write_concern = mongoc_write_concern_new();

   /*
    * Check generated bson.
    */
   ASSERT(write_concern);

   gle = _mongoc_write_concern_get_gle(write_concern);
   ASSERT(bson_iter_init_find(&iter, gle, "getlasterror") && BSON_ITER_HOLDS_INT32(&iter) && bson_iter_int32(&iter) == 1);
   ASSERT(!bson_iter_init_find(&iter, gle, "fsync"));
   ASSERT(!bson_iter_init_find(&iter, gle, "j"));
   ASSERT(!bson_iter_init_find(&iter, gle, "w"));
   ASSERT(gle);

   bson = _mongoc_write_concern_get_bson(write_concern);
   ASSERT(!bson_iter_init_find(&iter, bson, "getlasterror"));
   ASSERT(!bson_iter_init_find(&iter, bson, "fsync"));
   ASSERT(!bson_iter_init_find(&iter, bson, "j"));
   ASSERT(!bson_iter_init_find(&iter, gle, "w"));
   ASSERT(bson);

   mongoc_write_concern_destroy(write_concern);
}


static void
test_write_concern_bson_includes_false_fsync_and_journal (void)
{
   mongoc_write_concern_t *write_concern;
   const bson_t *gle;
   const bson_t *bson;
   bson_iter_t iter;

   write_concern = mongoc_write_concern_new();

   /*
    * Check generated bson.
    */
   ASSERT(write_concern);
   mongoc_write_concern_set_fsync(write_concern, false);
   mongoc_write_concern_set_journal(write_concern, false);

   gle = _mongoc_write_concern_get_gle(write_concern);
   ASSERT(bson_iter_init_find(&iter, gle, "getlasterror") && BSON_ITER_HOLDS_INT32(&iter) && bson_iter_int32(&iter) == 1);
   ASSERT(bson_iter_init_find(&iter, gle, "fsync") && BSON_ITER_HOLDS_BOOL(&iter) && !bson_iter_bool(&iter));
   ASSERT(bson_iter_init_find(&iter, gle, "j") && BSON_ITER_HOLDS_BOOL(&iter) && !bson_iter_bool(&iter));
   ASSERT(!bson_iter_init_find(&iter, gle, "w"));
   ASSERT(gle);

   bson = _mongoc_write_concern_get_bson(write_concern);
   ASSERT(!bson_iter_init_find(&iter, bson, "getlasterror"));
   ASSERT(bson_iter_init_find(&iter, bson, "fsync") && BSON_ITER_HOLDS_BOOL(&iter) && !bson_iter_bool(&iter));
   ASSERT(bson_iter_init_find(&iter, bson, "j") && BSON_ITER_HOLDS_BOOL(&iter) && !bson_iter_bool(&iter));
   ASSERT(!bson_iter_init_find(&iter, bson, "w"));
   ASSERT(bson);

   mongoc_write_concern_destroy(write_concern);
}


static void
test_write_concern_fsync_and_journal_gle_and_validity (void)
{
   mongoc_write_concern_t *write_concern = mongoc_write_concern_new();

   /*
    * Journal and fsync should imply GLE regardless of w; however, journal and
    * fsync logically conflict with w=0 and w=-1, so a write concern with such
    * a combination of options will be considered invalid.
    */

   /* Default write concern needs GLE and is valid */
   ASSERT(write_concern);
   ASSERT(_mongoc_write_concern_needs_gle(write_concern));
   ASSERT(_mongoc_write_concern_is_valid(write_concern));

   /* w=0 does not need GLE and is valid */
   mongoc_write_concern_set_w(write_concern, MONGOC_WRITE_CONCERN_W_UNACKNOWLEDGED);
   ASSERT(!_mongoc_write_concern_needs_gle(write_concern));
   ASSERT(_mongoc_write_concern_is_valid(write_concern));

   /* fsync=true needs GLE, but it conflicts with w=0 */
   mongoc_write_concern_set_fsync(write_concern, true);
   ASSERT(_mongoc_write_concern_needs_gle(write_concern));
   ASSERT(!_mongoc_write_concern_is_valid(write_concern));
   mongoc_write_concern_set_fsync(write_concern, false);

   /* journal=true needs GLE, but it conflicts with w=0 */
   mongoc_write_concern_set_journal(write_concern, true);
   ASSERT(_mongoc_write_concern_needs_gle(write_concern));
   ASSERT(!_mongoc_write_concern_is_valid(write_concern));
   mongoc_write_concern_set_journal(write_concern, false);

   /* w=-1 does not need GLE and is valid */
   mongoc_write_concern_set_w(write_concern, MONGOC_WRITE_CONCERN_W_ERRORS_IGNORED);
   ASSERT(!_mongoc_write_concern_needs_gle(write_concern));
   ASSERT(_mongoc_write_concern_is_valid(write_concern));

   /* fsync=true needs GLE, but it conflicts with w=-1 */
   mongoc_write_concern_set_fsync(write_concern, true);
   ASSERT(_mongoc_write_concern_needs_gle(write_concern));
   ASSERT(!_mongoc_write_concern_is_valid(write_concern));

   /* journal=true needs GLE, but it conflicts with w=-1 */
   mongoc_write_concern_set_fsync(write_concern, false);
   mongoc_write_concern_set_journal(write_concern, true);
   ASSERT(_mongoc_write_concern_needs_gle(write_concern));

   /* fsync=true with w=default needs GLE and is valid */
   mongoc_write_concern_set_journal(write_concern, false);
   mongoc_write_concern_set_fsync(write_concern, true);
   mongoc_write_concern_set_w(write_concern, MONGOC_WRITE_CONCERN_W_DEFAULT);
   ASSERT(_mongoc_write_concern_needs_gle(write_concern));
   ASSERT(_mongoc_write_concern_is_valid(write_concern));

   /* journal=true with w=default needs GLE and is valid */
   mongoc_write_concern_set_journal(write_concern, false);
   mongoc_write_concern_set_fsync(write_concern, true);
   mongoc_write_concern_set_w(write_concern, MONGOC_WRITE_CONCERN_W_DEFAULT);
   ASSERT(_mongoc_write_concern_needs_gle(write_concern));
   ASSERT(_mongoc_write_concern_is_valid(write_concern));

   mongoc_write_concern_destroy(write_concern);
}

static void
test_write_concern_wtimeout_validity (void)
{
   mongoc_write_concern_t *write_concern = mongoc_write_concern_new();

   /* Test defaults */
   ASSERT(write_concern);
   ASSERT(mongoc_write_concern_get_w(write_concern) == MONGOC_WRITE_CONCERN_W_DEFAULT);
   ASSERT(mongoc_write_concern_get_wtimeout(write_concern) == 0);
   ASSERT(!mongoc_write_concern_get_wmajority(write_concern));

   /* mongoc_write_concern_set_wtimeout() ignores invalid wtimeout */
   mongoc_write_concern_set_wtimeout(write_concern, -1);
   ASSERT(mongoc_write_concern_get_w(write_concern) == MONGOC_WRITE_CONCERN_W_DEFAULT);
   ASSERT(mongoc_write_concern_get_wtimeout(write_concern) == 0);
   ASSERT(!mongoc_write_concern_get_wmajority(write_concern));
   ASSERT(_mongoc_write_concern_is_valid(write_concern));

   /* mongoc_write_concern_set_wmajority() ignores invalid wtimeout */
   mongoc_write_concern_set_wmajority(write_concern, -1);
   ASSERT(mongoc_write_concern_get_w(write_concern) == MONGOC_WRITE_CONCERN_W_MAJORITY);
   ASSERT(mongoc_write_concern_get_wtimeout(write_concern) == 0);
   ASSERT(mongoc_write_concern_get_wmajority(write_concern));
   ASSERT(_mongoc_write_concern_is_valid(write_concern));

   /* Manually assigning a negative wtimeout will make the write concern invalid */
   write_concern->wtimeout = -1;
   ASSERT(!_mongoc_write_concern_is_valid(write_concern));

   mongoc_write_concern_destroy(write_concern);
}


void
test_write_concern_install (TestSuite *suite)
{
   TestSuite_Add (suite, "/WriteConcern/basic", test_write_concern_basic);
   TestSuite_Add (suite, "/WriteConcern/bson_omits_defaults", test_write_concern_bson_omits_defaults);
   TestSuite_Add (suite, "/WriteConcern/bson_includes_false_fsync_and_journal", test_write_concern_bson_includes_false_fsync_and_journal);
   TestSuite_Add (suite, "/WriteConcern/fsync_and_journal_gle_and_validity", test_write_concern_fsync_and_journal_gle_and_validity);
   TestSuite_Add (suite, "/WriteConcern/wtimeout_validity", test_write_concern_wtimeout_validity);
}
