#include <mongoc.h>

#include "mongoc-client-private.h"
#include "mongoc-uri-private.h"
#include "mongoc-host-list-private.h"

#include "TestSuite.h"

#include "test-libmongoc.h"

#define ASSERT_SUPPRESS(x) \
   do { \
      suppress_one_message (); \
      ASSERT (x); \
   } while (0)

static void
test_mongoc_uri_new (void)
{
   const mongoc_host_list_t *hosts;
   const bson_t *options;
   const bson_t *credentials;
   const bson_t *read_prefs_tags;
   const mongoc_read_prefs_t *read_prefs;
   bson_t properties;
   mongoc_uri_t *uri;
   bson_iter_t iter;
   bson_iter_t child;

   /* bad uris */
   ASSERT(!mongoc_uri_new("mongodb://"));
   ASSERT_SUPPRESS(!mongoc_uri_new("mongodb://\x80"));
   ASSERT_SUPPRESS(!mongoc_uri_new("mongodb://localhost/\x80"));
   ASSERT_SUPPRESS(!mongoc_uri_new("mongodb://localhost:\x80/"));
   ASSERT_SUPPRESS(!mongoc_uri_new("mongodb://localhost/?ipv6=\x80"));
   ASSERT_SUPPRESS(!mongoc_uri_new("mongodb://localhost/?foo=\x80"));
   ASSERT_SUPPRESS(!mongoc_uri_new("mongodb://localhost/?\x80=bar"));
   ASSERT_SUPPRESS(!mongoc_uri_new("mongodb://\x80:pass@localhost"));
   ASSERT_SUPPRESS(!mongoc_uri_new("mongodb://user:\x80@localhost"));
   ASSERT_SUPPRESS(!mongoc_uri_new("mongodb://user%40DOMAIN.COM:password@localhost/?"
                                   "authMechanism=\x80"));
   ASSERT_SUPPRESS(!mongoc_uri_new("mongodb://user%40DOMAIN.COM:password@localhost/?"
                                   "authMechanism=GSSAPI"
                                   "&authMechanismProperties=SERVICE_NAME:\x80"));
   ASSERT_SUPPRESS(!mongoc_uri_new("mongodb://user%40DOMAIN.COM:password@localhost/?"
                                   "authMechanism=GSSAPI"
                                   "&authMechanismProperties=\x80:mongodb"));
   ASSERT(!mongoc_uri_new("mongodb://::"));
   ASSERT(!mongoc_uri_new("mongodb://localhost::27017"));
   ASSERT(!mongoc_uri_new("mongodb://localhost,localhost::"));
   ASSERT(!mongoc_uri_new("mongodb://local1,local2,local3/d?k"));
   ASSERT(!mongoc_uri_new(""));
   ASSERT(!mongoc_uri_new("mongo://localhost:27017"));
   ASSERT(!mongoc_uri_new("mongodb://localhost::27017"));
   ASSERT(!mongoc_uri_new("mongodb://localhost::27017/"));
   ASSERT(!mongoc_uri_new("mongodb://localhost::27017,abc"));
   ASSERT(!mongoc_uri_new("mongodb://localhost:-1"));
   ASSERT(!mongoc_uri_new("mongodb://localhost:65536"));
   ASSERT(!mongoc_uri_new("mongodb://localhost:foo"));
   ASSERT(!mongoc_uri_new("mongodb://localhost:65536/"));
   ASSERT(!mongoc_uri_new("mongodb://localhost:0/"));
   ASSERT(!mongoc_uri_new("mongodb://[::1]:-1"));
   ASSERT(!mongoc_uri_new("mongodb://[::1]:foo"));
   ASSERT(!mongoc_uri_new("mongodb://[::1]:65536"));
   ASSERT(!mongoc_uri_new("mongodb://[::1]:65536/"));
   ASSERT(!mongoc_uri_new("mongodb://[::1]:0/"));

   uri = mongoc_uri_new("mongodb://[::1]:27888,[::2]:27999/?ipv6=true&safe=true");
   assert (uri);
   hosts = mongoc_uri_get_hosts(uri);
   assert (hosts);
   ASSERT_CMPSTR (hosts->host, "::1");
   assert (hosts->port == 27888);
   ASSERT_CMPSTR (hosts->host_and_port, "[::1]:27888");
   mongoc_uri_destroy (uri);

   uri = mongoc_uri_new("mongodb:///tmp/mongodb-27017.sock/?");
   ASSERT(uri);
   mongoc_uri_destroy(uri);

   /* should normalize to lowercase */
   uri = mongoc_uri_new ("mongodb://cRaZyHoStNaMe");
   assert (uri);
   hosts = mongoc_uri_get_hosts (uri);
   assert (hosts);
   ASSERT_CMPSTR (hosts->host, "crazyhostname");
   mongoc_uri_destroy (uri);

   uri = mongoc_uri_new("mongodb://localhost/?");
   ASSERT(uri);
   mongoc_uri_destroy(uri);

   uri = mongoc_uri_new("mongodb://localhost:27017/test?q=1");
   ASSERT(uri);
   hosts = mongoc_uri_get_hosts(uri);
   ASSERT(hosts);
   ASSERT(!hosts->next);
   ASSERT_CMPSTR(hosts->host, "localhost");
   ASSERT_CMPINT(hosts->port, ==, 27017);
   ASSERT_CMPSTR(hosts->host_and_port, "localhost:27017");
   ASSERT_CMPSTR(mongoc_uri_get_database(uri), "test");
   options = mongoc_uri_get_options(uri);
   ASSERT(options);
   ASSERT(bson_iter_init_find(&iter, options, "q"));
   ASSERT_CMPSTR(bson_iter_utf8(&iter, NULL), "1");
   mongoc_uri_destroy(uri);

   uri = mongoc_uri_new("mongodb://local1,local2:999,local3?q=1");
   ASSERT(uri);
   hosts = mongoc_uri_get_hosts(uri);
   ASSERT(hosts);
   ASSERT(hosts->next);
   ASSERT(hosts->next->next);
   ASSERT(!hosts->next->next->next);
   ASSERT_CMPSTR(hosts->host, "local1");
   ASSERT_CMPINT(hosts->port, ==, 27017);
   ASSERT_CMPSTR(hosts->next->host, "local2");
   ASSERT_CMPINT(hosts->next->port, ==, 999);
   ASSERT_CMPSTR(hosts->next->next->host, "local3");
   ASSERT_CMPINT(hosts->next->next->port, ==, 27017);
   options = mongoc_uri_get_options(uri);
   ASSERT(options);
   ASSERT(bson_iter_init_find(&iter, options, "q"));
   ASSERT_CMPSTR(bson_iter_utf8(&iter, NULL), "1");
   mongoc_uri_destroy(uri);

   uri = mongoc_uri_new("mongodb://localhost:27017/?readPreference=secondaryPreferred&readPreferenceTags=dc:ny&readPreferenceTags=");
   ASSERT(uri);
   read_prefs = mongoc_uri_get_read_prefs_t(uri);
   ASSERT(mongoc_read_prefs_get_mode(read_prefs) == MONGOC_READ_SECONDARY_PREFERRED);
   ASSERT(read_prefs);
   read_prefs_tags = mongoc_read_prefs_get_tags(read_prefs);
   ASSERT(read_prefs_tags);
   ASSERT_CMPINT(bson_count_keys(read_prefs_tags), ==, 2);
   ASSERT(bson_iter_init_find(&iter, read_prefs_tags, "0"));
   ASSERT(BSON_ITER_HOLDS_DOCUMENT(&iter));
   ASSERT(bson_iter_recurse(&iter, &child));
   ASSERT(bson_iter_next(&child));
   ASSERT_CMPSTR(bson_iter_key(&child), "dc");
   ASSERT_CMPSTR(bson_iter_utf8(&child, NULL), "ny");
   ASSERT(!bson_iter_next(&child));
   ASSERT(bson_iter_next(&iter));
   ASSERT(BSON_ITER_HOLDS_DOCUMENT(&iter));
   ASSERT(bson_iter_recurse(&iter, &child));
   ASSERT(!bson_iter_next(&child));
   ASSERT(!bson_iter_next(&iter));
   mongoc_uri_destroy(uri);

   uri = mongoc_uri_new("mongodb://localhost/a?slaveok=true&ssl=false&journal=true");
   options = mongoc_uri_get_options(uri);
   ASSERT(options);
   ASSERT_CMPINT(bson_count_keys(options), ==, 3);
   ASSERT(bson_iter_init(&iter, options));
   ASSERT(bson_iter_find_case(&iter, "slaveok"));
   ASSERT(BSON_ITER_HOLDS_BOOL(&iter));
   ASSERT(bson_iter_bool(&iter));
   ASSERT(bson_iter_find_case(&iter, "ssl"));
   ASSERT(BSON_ITER_HOLDS_BOOL(&iter));
   ASSERT(!bson_iter_bool(&iter));
   ASSERT(bson_iter_find_case(&iter, "journal"));
   ASSERT(BSON_ITER_HOLDS_BOOL(&iter));
   ASSERT(bson_iter_bool(&iter));
   ASSERT(!bson_iter_next(&iter));
   mongoc_uri_destroy(uri);

   uri = mongoc_uri_new("mongodb://localhost/?safe=false&journal=false");
   options = mongoc_uri_get_options(uri);
   ASSERT(options);
   ASSERT_CMPINT(bson_count_keys(options), ==, 2);
   ASSERT(bson_iter_init(&iter, options));
   ASSERT(bson_iter_find_case(&iter, "safe"));
   ASSERT(BSON_ITER_HOLDS_BOOL(&iter));
   ASSERT(!bson_iter_bool(&iter));
   ASSERT(bson_iter_find_case(&iter, "journal"));
   ASSERT(BSON_ITER_HOLDS_BOOL(&iter));
   ASSERT(!bson_iter_bool(&iter));
   ASSERT(!bson_iter_next(&iter));
   mongoc_uri_destroy(uri);

   uri = mongoc_uri_new("mongodb:///tmp/mongodb-27017.sock/?ssl=false");
   ASSERT(uri);
   ASSERT_CMPSTR(mongoc_uri_get_hosts(uri)->host, "/tmp/mongodb-27017.sock");
   mongoc_uri_destroy(uri);

   uri = mongoc_uri_new("mongodb:///tmp/mongodb-27017.sock,localhost:27017/?ssl=false");
   ASSERT(uri);
   ASSERT_CMPSTR(mongoc_uri_get_hosts(uri)->host, "/tmp/mongodb-27017.sock");
   ASSERT_CMPSTR(mongoc_uri_get_hosts(uri)->next->host_and_port, "localhost:27017");
   ASSERT(!mongoc_uri_get_hosts(uri)->next->next);
   mongoc_uri_destroy(uri);

   /* should assign port numbers to correct hosts */
   uri = mongoc_uri_new("mongodb://host1,host2:30000/foo/");
   ASSERT(uri);
   ASSERT_CMPSTR(mongoc_uri_get_hosts(uri)->host_and_port, "host1:27017");
   ASSERT_CMPSTR(mongoc_uri_get_hosts(uri)->next->host_and_port, "host2:30000");
   mongoc_uri_destroy(uri);

   uri = mongoc_uri_new("mongodb://localhost:27017,/tmp/mongodb-27017.sock/?ssl=false");
   ASSERT(uri);
   ASSERT_CMPSTR(mongoc_uri_get_hosts(uri)->host_and_port, "localhost:27017");
   ASSERT_CMPSTR(mongoc_uri_get_hosts(uri)->next->host, "/tmp/mongodb-27017.sock");
   ASSERT(!mongoc_uri_get_hosts(uri)->next->next);
   mongoc_uri_destroy(uri);

   /* should use the authSource over db when both are specified */
   uri = mongoc_uri_new("mongodb://christian:secret@localhost:27017/foo/?authSource=abcd");
   ASSERT(uri);
   ASSERT_CMPSTR(mongoc_uri_get_username(uri), "christian");
   ASSERT_CMPSTR(mongoc_uri_get_password(uri), "secret");
   ASSERT_CMPSTR(mongoc_uri_get_auth_source(uri), "abcd");
   mongoc_uri_destroy(uri);

   /* should use the default auth source and mechanism */
   uri = mongoc_uri_new("mongodb://christian:secret@localhost:27017");
   ASSERT(uri);
   ASSERT_CMPSTR(mongoc_uri_get_auth_source(uri), "admin");
   ASSERT(!mongoc_uri_get_auth_mechanism(uri));
   mongoc_uri_destroy(uri);

   /* should use the db when no authSource is specified */
   uri = mongoc_uri_new("mongodb://user:password@localhost/foo");
   ASSERT(uri);
   ASSERT_CMPSTR(mongoc_uri_get_auth_source(uri), "foo");
   mongoc_uri_destroy(uri);

   /* should recognize an empty password */
   uri = mongoc_uri_new("mongodb://samantha:@localhost");
   ASSERT(uri);
   ASSERT_CMPSTR(mongoc_uri_get_username(uri), "samantha");
   ASSERT_CMPSTR(mongoc_uri_get_password(uri), "");
   mongoc_uri_destroy(uri);

   /* should recognize no password */
   uri = mongoc_uri_new("mongodb://christian@localhost:27017");
   ASSERT(uri);
   ASSERT_CMPSTR(mongoc_uri_get_username(uri), "christian");
   ASSERT(!mongoc_uri_get_password(uri));
   mongoc_uri_destroy(uri);

   /* should recognize a url escaped character in the username */
   uri = mongoc_uri_new("mongodb://christian%40realm:pwd@localhost:27017");
   ASSERT(uri);
   ASSERT_CMPSTR(mongoc_uri_get_username(uri), "christian@realm");
   mongoc_uri_destroy(uri);

   /* while you shouldn't do this, lets test for it */
   uri = mongoc_uri_new("mongodb://christian%40realm@localhost:27017/db%2ename");
   ASSERT(uri);
   ASSERT_CMPSTR(mongoc_uri_get_database(uri), "db.name");
   mongoc_uri_destroy(uri);
   uri = mongoc_uri_new("mongodb://christian%40realm@localhost:27017/db%2Ename");
   ASSERT(uri);
   ASSERT_CMPSTR(mongoc_uri_get_database(uri), "db.name");
   mongoc_uri_destroy(uri);

   uri = mongoc_uri_new("mongodb://christian%40realm@localhost:27017/?abcd=%20");
   ASSERT(uri);
   options = mongoc_uri_get_options(uri);
   ASSERT(options);
   ASSERT(bson_iter_init_find(&iter, options, "abcd"));
   ASSERT(BSON_ITER_HOLDS_UTF8(&iter));
   ASSERT_CMPSTR(bson_iter_utf8(&iter, NULL), " ");
   mongoc_uri_destroy(uri);

   uri = mongoc_uri_new("mongodb://christian%40realm@[::6]:27017/?abcd=%20");
   ASSERT(uri);
   options = mongoc_uri_get_options(uri);
   ASSERT(options);
   ASSERT(bson_iter_init_find(&iter, options, "abcd"));
   ASSERT(BSON_ITER_HOLDS_UTF8(&iter));
   ASSERT_CMPSTR(bson_iter_utf8(&iter, NULL), " ");
   mongoc_uri_destroy(uri);

   /* GSSAPI-specific options */

   /* should recognize the GSSAPI mechanism, and use $external as source */
   uri = mongoc_uri_new("mongodb://user%40DOMAIN.COM:password@localhost/?authMechanism=GSSAPI");
   ASSERT(uri);
   ASSERT_CMPSTR(mongoc_uri_get_auth_mechanism(uri), "GSSAPI");
   /*ASSERT_CMPSTR(mongoc_uri_get_auth_source(uri), "$external");*/
   mongoc_uri_destroy(uri);

   /* use $external as source when db is specified */
   uri = mongoc_uri_new("mongodb://user%40DOMAIN.COM:password@localhost/foo/?authMechanism=GSSAPI");
   ASSERT(uri);
   ASSERT_CMPSTR(mongoc_uri_get_auth_source(uri), "$external");
   mongoc_uri_destroy(uri);

   /* should not accept authSource other than $external */
   ASSERT(!mongoc_uri_new("mongodb://user%40DOMAIN.COM:password@localhost/foo/?authMechanism=GSSAPI&authSource=bar"));

   /* should accept authMechanismProperties */
   uri = mongoc_uri_new("mongodb://user%40DOMAIN.COM:password@localhost/?authMechanism=GSSAPI"
                        "&authMechanismProperties=SERVICE_NAME:other,CANONICALIZE_HOST_NAME:true");
   ASSERT(uri);
   credentials = mongoc_uri_get_credentials(uri);
   ASSERT(credentials);
   ASSERT(mongoc_uri_get_mechanism_properties(uri, &properties));
   assert (bson_iter_init_find_case (&iter, &properties, "SERVICE_NAME") &&
           BSON_ITER_HOLDS_UTF8 (&iter) &&
           (0 == strcmp (bson_iter_utf8 (&iter, NULL), "other")));
   assert (bson_iter_init_find_case (&iter, &properties, "CANONICALIZE_HOST_NAME") &&
           BSON_ITER_HOLDS_UTF8 (&iter) &&
           (0 == strcmp (bson_iter_utf8 (&iter, NULL), "true")));
   mongoc_uri_destroy(uri);

   /* reverse order of arguments to ensure parsing still succeeds */
   uri = mongoc_uri_new("mongodb://user@localhost/"
                        "?authMechanismProperties=SERVICE_NAME:other"
                        "&authMechanism=GSSAPI");
   ASSERT(uri);
   mongoc_uri_destroy(uri);

   /* deprecated gssapiServiceName option */
   uri = mongoc_uri_new("mongodb://christian%40realm.cc@localhost:27017/?authMechanism=GSSAPI&gssapiServiceName=blah");
   ASSERT(uri);
   options = mongoc_uri_get_options(uri);
   ASSERT(options);
   assert (0 == strcmp (mongoc_uri_get_auth_mechanism (uri), "GSSAPI"));
   assert (0 == strcmp (mongoc_uri_get_username (uri), "christian@realm.cc"));
   assert (bson_iter_init_find_case (&iter, options, "gssapiServiceName") &&
           BSON_ITER_HOLDS_UTF8 (&iter) &&
           (0 == strcmp (bson_iter_utf8 (&iter, NULL), "blah")));
   mongoc_uri_destroy(uri);

   /* MONGODB-CR */

   /* should recognize this mechanism */
   uri = mongoc_uri_new("mongodb://user@localhost/?authMechanism=MONGODB-CR");
   ASSERT(uri);
   ASSERT_CMPSTR(mongoc_uri_get_auth_mechanism(uri), "MONGODB-CR");
   mongoc_uri_destroy(uri);

   /* X509 */

   /* should recognize this mechanism, and use $external as the source */
   uri = mongoc_uri_new("mongodb://user@localhost/?authMechanism=MONGODB-X509");
   ASSERT(uri);
   ASSERT_CMPSTR(mongoc_uri_get_auth_mechanism(uri), "MONGODB-X509");
   /*ASSERT_CMPSTR(mongoc_uri_get_auth_source(uri), "$external");*/
   mongoc_uri_destroy(uri);

   /* use $external as source when db is specified */
   uri = mongoc_uri_new("mongodb://CN%3DmyName%2COU%3DmyOrgUnit%2CO%3DmyOrg%2CL%3DmyLocality"
                        "%2CST%3DmyState%2CC%3DmyCountry@localhost/foo/?authMechanism=MONGODB-X509");
   ASSERT(uri);
   ASSERT_CMPSTR(mongoc_uri_get_auth_source(uri), "$external");
   mongoc_uri_destroy(uri);

   /* should not accept authSource other than $external */
   ASSERT(!mongoc_uri_new("mongodb://CN%3DmyName%2COU%3DmyOrgUnit%2CO%3DmyOrg%2CL%3DmyLocality"
                          "%2CST%3DmyState%2CC%3DmyCountry@localhost/foo/?authMechanism=MONGODB-X509&authSource=bar"));

   /* should recognize the encoded username */
   uri = mongoc_uri_new("mongodb://CN%3DmyName%2COU%3DmyOrgUnit%2CO%3DmyOrg%2CL%3DmyLocality"
                        "%2CST%3DmyState%2CC%3DmyCountry@localhost/?authMechanism=MONGODB-X509");
   ASSERT(uri);
   ASSERT_CMPSTR(mongoc_uri_get_username(uri), "CN=myName,OU=myOrgUnit,O=myOrg,L=myLocality,ST=myState,C=myCountry");
   mongoc_uri_destroy(uri);

   /* PLAIN */

   /* should recognize this mechanism */
   uri = mongoc_uri_new("mongodb://user@localhost/?authMechanism=PLAIN");
   ASSERT(uri);
   ASSERT_CMPSTR(mongoc_uri_get_auth_mechanism(uri), "PLAIN");
   mongoc_uri_destroy(uri);

   /* SCRAM-SHA1 */

   /* should recognize this mechanism */
   uri = mongoc_uri_new("mongodb://user@localhost/?authMechanism=SCRAM-SHA1");
   ASSERT(uri);
   ASSERT_CMPSTR(mongoc_uri_get_auth_mechanism(uri), "SCRAM-SHA1");
   mongoc_uri_destroy(uri);
}


static void
test_mongoc_uri_functions (void)
{
   mongoc_client_t *client;
   mongoc_uri_t *uri;
   mongoc_database_t *db;

   uri = mongoc_uri_new("mongodb://foo:bar@localhost:27017/baz?authSource=source");

   ASSERT_CMPSTR(mongoc_uri_get_username(uri), "foo");
   ASSERT_CMPSTR(mongoc_uri_get_password(uri), "bar");
   ASSERT_CMPSTR(mongoc_uri_get_database(uri), "baz");
   ASSERT_CMPSTR(mongoc_uri_get_auth_source(uri), "source");

   mongoc_uri_set_username (uri, "longer username that should work");
   ASSERT_CMPSTR(mongoc_uri_get_username(uri), "longer username that should work");

   mongoc_uri_set_password (uri, "longer password that should also work");
   ASSERT_CMPSTR(mongoc_uri_get_password(uri), "longer password that should also work");

   mongoc_uri_set_database (uri, "longer database that should work");
   ASSERT_CMPSTR(mongoc_uri_get_database(uri), "longer database that should work");
   ASSERT_CMPSTR(mongoc_uri_get_auth_source(uri), "source");

   mongoc_uri_set_auth_source (uri, "longer authsource that should work");
   ASSERT_CMPSTR(mongoc_uri_get_auth_source(uri), "longer authsource that should work");
   ASSERT_CMPSTR(mongoc_uri_get_database(uri), "longer database that should work");

   client = mongoc_client_new_from_uri (uri);
   mongoc_uri_destroy(uri);

   ASSERT_CMPSTR(mongoc_uri_get_username(client->uri), "longer username that should work");
   ASSERT_CMPSTR(mongoc_uri_get_password(client->uri), "longer password that should also work");
   ASSERT_CMPSTR(mongoc_uri_get_database(client->uri), "longer database that should work");
   ASSERT_CMPSTR(mongoc_uri_get_auth_source(client->uri), "longer authsource that should work");
   mongoc_client_destroy (client);


   uri = mongoc_uri_new("mongodb://localhost/?serverselectiontimeoutms=3&journal=true&wtimeoutms=42&canonicalizeHostname=false");

   ASSERT_CMPINT(mongoc_uri_get_option_as_int32(uri, "serverselectiontimeoutms", 18), ==, 3);
   ASSERT(mongoc_uri_set_option_as_int32(uri, "serverselectiontimeoutms", 18));
   ASSERT_CMPINT(mongoc_uri_get_option_as_int32(uri, "serverselectiontimeoutms", 19), ==, 18);

   ASSERT_CMPINT(mongoc_uri_get_option_as_int32(uri, "wtimeoutms", 18), ==, 42);
   ASSERT(mongoc_uri_set_option_as_int32(uri, "wtimeoutms", 18));
   ASSERT_CMPINT(mongoc_uri_get_option_as_int32(uri, "wtimeoutms", 19), ==, 18);

   /* socketcheckintervalms isn't set, return our fallback */
   ASSERT_CMPINT(mongoc_uri_get_option_as_int32(uri, "socketcheckintervalms", 123), ==, 123);
   ASSERT(mongoc_uri_set_option_as_int32(uri, "socketcheckintervalms", 18));
   ASSERT_CMPINT(mongoc_uri_get_option_as_int32(uri, "socketcheckintervalms", 19), ==, 18);

   ASSERT(mongoc_uri_get_option_as_bool(uri, "journal", false));
   ASSERT(!mongoc_uri_get_option_as_bool(uri, "canonicalizeHostname", true));
   /* ssl isn't set, return out fallback */
   ASSERT(mongoc_uri_get_option_as_bool(uri, "ssl", true));

   client = mongoc_client_new_from_uri (uri);
   mongoc_uri_destroy(uri);

   ASSERT(mongoc_uri_get_option_as_bool(client->uri, "journal", false));
   ASSERT(!mongoc_uri_get_option_as_bool(client->uri, "canonicalizeHostname", true));
   /* ssl isn't set, return out fallback */
   ASSERT(mongoc_uri_get_option_as_bool(client->uri, "ssl", true));
   mongoc_client_destroy (client);

   uri = mongoc_uri_new("mongodb://localhost/");
   ASSERT_CMPSTR(mongoc_uri_get_option_as_utf8(uri, "random", "default"), "default");
   ASSERT(mongoc_uri_set_option_as_utf8(uri, "random", "value"));
   ASSERT_CMPSTR(mongoc_uri_get_option_as_utf8(uri, "random", "default"), "value");

   mongoc_uri_destroy(uri);


   uri = mongoc_uri_new("mongodb://localhost/?sockettimeoutms=1&socketcheckintervalms=200");
   ASSERT_CMPINT (1, ==, mongoc_uri_get_option_as_int32 (uri, "sockettimeoutms", 0));
   ASSERT_CMPINT (200, ==, mongoc_uri_get_option_as_int32 (uri, "socketcheckintervalms", 0));

   mongoc_uri_set_option_as_int32 (uri, "sockettimeoutms", 2);
   ASSERT_CMPINT (2, ==, mongoc_uri_get_option_as_int32 (uri, "sockettimeoutms", 0));

   mongoc_uri_set_option_as_int32 (uri, "socketcheckintervalms", 202);
   ASSERT_CMPINT (202, ==, mongoc_uri_get_option_as_int32 (uri, "socketcheckintervalms", 0));


   client = mongoc_client_new_from_uri (uri);
   ASSERT_CMPINT (2, ==, client->cluster.sockettimeoutms);
   ASSERT_CMPINT (202, ==, client->cluster.socketcheckintervalms);

   mongoc_client_destroy (client);
   mongoc_uri_destroy(uri);


   uri = mongoc_uri_new ("mongodb://host/dbname0");
   ASSERT_CMPSTR (mongoc_uri_get_database (uri), "dbname0");
   mongoc_uri_set_database (uri, "dbname1");
   client = mongoc_client_new_from_uri (uri);
   db = mongoc_client_get_default_database (client);
   ASSERT_CMPSTR (mongoc_database_get_name (db), "dbname1");

   mongoc_database_destroy (db);
   mongoc_client_destroy (client);
   mongoc_uri_destroy(uri);
}


#undef ASSERT_SUPPRESS

static void
test_mongoc_host_list_from_string (void)
{
   mongoc_host_list_t host_list = { 0 };

   ASSERT(_mongoc_host_list_from_string(&host_list, "localhost:27019"));
   ASSERT(!strcmp(host_list.host_and_port, "localhost:27019"));
   ASSERT(!strcmp(host_list.host, "localhost"));
   ASSERT(host_list.port == 27019);
   ASSERT(host_list.family == AF_INET);
   ASSERT(!host_list.next);
}


static void
test_mongoc_uri_new_for_host_port (void)
{
   mongoc_uri_t *uri;

   uri = mongoc_uri_new_for_host_port("uber", 555);
   ASSERT(uri);
   ASSERT(!strcmp("uber", mongoc_uri_get_hosts(uri)->host));
   ASSERT(!strcmp("uber:555", mongoc_uri_get_hosts(uri)->host_and_port));
   ASSERT(555 == mongoc_uri_get_hosts(uri)->port);
   mongoc_uri_destroy(uri);
}


static void
test_mongoc_uri_unescape (void)
{
#define ASSERT_URIDECODE_STR(_s, _e) \
   do { \
      char *str = mongoc_uri_unescape(_s); \
      ASSERT(!strcmp(str, _e)); \
      bson_free(str); \
   } while (0)
#define ASSERT_URIDECODE_FAIL(_s) \
   do { \
      char *str = mongoc_uri_unescape(_s); \
      ASSERT(!str); \
   } while (0)

   ASSERT_URIDECODE_STR("", "");
   ASSERT_URIDECODE_STR("%40", "@");
   ASSERT_URIDECODE_STR("me%40localhost@localhost", "me@localhost@localhost");
   ASSERT_URIDECODE_STR("%20", " ");
   ASSERT_URIDECODE_STR("%24%21%40%2A%26%5E%21%40%2A%23%26%5E%21%40%23%2A%26"
                        "%5E%21%40%2A%23%26%5E%21%40%2A%26%23%5E%7D%7B%7D%7B"
                        "%22%22%27%7D%7B%5B%5D%3C%3E%3F",
                        "$!@*&^!@*#&^!@#*&^!@*#&^!@*&#^}{}{\"\"'}{[]<>?");

   ASSERT_URIDECODE_FAIL("%");
   ASSERT_URIDECODE_FAIL("%%");
   ASSERT_URIDECODE_FAIL("%%%");
   ASSERT_URIDECODE_FAIL("%FF");
   ASSERT_URIDECODE_FAIL("%CC");
   ASSERT_URIDECODE_FAIL("%00");

#undef ASSERT_URIDECODE_STR
#undef ASSERT_URIDECODE_FAIL
}


typedef struct
{
   const char         *uri;
   bool                parses;
   mongoc_read_mode_t  mode;
   bson_t             *tags;
} read_prefs_test;


static void
test_mongoc_uri_read_prefs (void)
{
   const mongoc_read_prefs_t *rp;
   mongoc_uri_t *uri;
   const read_prefs_test *t;
   int i;

   bson_t *tags_dcny = BCON_NEW(
      "0", "{", "dc", "ny", "}"
   );
   bson_t *tags_dcny_empty = BCON_NEW(
      "0", "{", "dc", "ny", "}",
      "1", "{", "}"
   );
   bson_t *tags_dcnyusessd_dcsf_empty = BCON_NEW(
      "0", "{", "dc", "ny", "use", "ssd", "}",
      "1", "{", "dc", "sf", "}",
      "2", "{", "}"
   );
   bson_t *tags_empty = BCON_NEW(
      "0", "{", "}"
   );

   const read_prefs_test tests [] = {
      { "mongodb://localhost/", true, MONGOC_READ_PRIMARY, NULL },
      { "mongodb://localhost/?slaveOk=false", true, MONGOC_READ_PRIMARY, NULL },
      { "mongodb://localhost/?slaveOk=true", true, MONGOC_READ_SECONDARY_PREFERRED, NULL },
      { "mongodb://localhost/?readPreference=primary", true, MONGOC_READ_PRIMARY, NULL },
      { "mongodb://localhost/?readPreference=primaryPreferred", true, MONGOC_READ_PRIMARY_PREFERRED, NULL },
      { "mongodb://localhost/?readPreference=secondary", true, MONGOC_READ_SECONDARY, NULL },
      { "mongodb://localhost/?readPreference=secondaryPreferred", true, MONGOC_READ_SECONDARY_PREFERRED, NULL },
      { "mongodb://localhost/?readPreference=nearest", true, MONGOC_READ_NEAREST, NULL },
      /* readPreference should take priority over slaveOk */
      { "mongodb://localhost/?slaveOk=false&readPreference=secondary", true, MONGOC_READ_SECONDARY, NULL },
      /* readPreferenceTags conflict with primary mode */
      { "mongodb://localhost/?readPreferenceTags=", false },
      { "mongodb://localhost/?readPreference=primary&readPreferenceTags=", false },
      { "mongodb://localhost/?slaveOk=false&readPreferenceTags=", false },
      { "mongodb://localhost/?readPreference=secondaryPreferred&readPreferenceTags=", true, MONGOC_READ_SECONDARY_PREFERRED, tags_empty },
      { "mongodb://localhost/?readPreference=secondaryPreferred&readPreferenceTags=dc:ny", true, MONGOC_READ_SECONDARY_PREFERRED, tags_dcny },
      { "mongodb://localhost/?readPreference=nearest&readPreferenceTags=dc:ny&readPreferenceTags=", true, MONGOC_READ_NEAREST, tags_dcny_empty },
      { "mongodb://localhost/?readPreference=nearest&readPreferenceTags=dc:ny,use:ssd&readPreferenceTags=dc:sf&readPreferenceTags=", true, MONGOC_READ_NEAREST, tags_dcnyusessd_dcsf_empty },
      { NULL }
   };

   for (i = 0; tests[i].uri; i++) {
      t = &tests[i];

      uri = mongoc_uri_new(t->uri);
      if (t->parses) {
         assert(uri);
      } else {
         assert(!uri);
         continue;
      }

      rp = mongoc_uri_get_read_prefs_t(uri);
      assert(rp);

      assert(t->mode == mongoc_read_prefs_get_mode(rp));

      if (t->tags) {
         assert(bson_equal(t->tags, mongoc_read_prefs_get_tags(rp)));
      }

      mongoc_uri_destroy(uri);
   }

   bson_destroy(tags_dcny);
   bson_destroy(tags_dcny_empty);
   bson_destroy(tags_dcnyusessd_dcsf_empty);
   bson_destroy(tags_empty);
}


typedef struct
{
   const char *uri;
   bool        parses;
   int32_t     w;
   const char *wtag;
   int32_t     wtimeoutms;
} write_concern_test;


static void
test_mongoc_uri_write_concern (void)
{
   const mongoc_write_concern_t *wr;
   mongoc_uri_t *uri;
   const write_concern_test *t;
   int i;
   static const write_concern_test tests [] = {
      { "mongodb://localhost/?safe=false", true, MONGOC_WRITE_CONCERN_W_UNACKNOWLEDGED },
      { "mongodb://localhost/?safe=true", true, 1 },
      { "mongodb://localhost/?w=-1", true, MONGOC_WRITE_CONCERN_W_ERRORS_IGNORED },
      { "mongodb://localhost/?w=0", true, MONGOC_WRITE_CONCERN_W_UNACKNOWLEDGED },
      { "mongodb://localhost/?w=1", true, 1 },
      { "mongodb://localhost/?w=2", true, 2 },
      { "mongodb://localhost/?w=majority", true, MONGOC_WRITE_CONCERN_W_MAJORITY },
      { "mongodb://localhost/?w=10", true, 10 },
      { "mongodb://localhost/?w=", true, MONGOC_WRITE_CONCERN_W_DEFAULT },
      { "mongodb://localhost/?w=mytag", true, MONGOC_WRITE_CONCERN_W_TAG, "mytag" },
      { "mongodb://localhost/?w=mytag&safe=false", true, MONGOC_WRITE_CONCERN_W_TAG, "mytag" },
      { "mongodb://localhost/?w=1&safe=false", true, 1 },
      { "mongodb://localhost/?journal=true", true, MONGOC_WRITE_CONCERN_W_DEFAULT },
      { "mongodb://localhost/?w=0&journal=true", false, MONGOC_WRITE_CONCERN_W_UNACKNOWLEDGED },
      { "mongodb://localhost/?w=-1&journal=true", false, MONGOC_WRITE_CONCERN_W_ERRORS_IGNORED },
      { "mongodb://localhost/?w=1&journal=true", true, 1 },
      { "mongodb://localhost/?w=2&wtimeoutms=1000", true, 2, NULL, 1000 },
      { "mongodb://localhost/?w=majority&wtimeoutms=1000", true, MONGOC_WRITE_CONCERN_W_MAJORITY, NULL, 1000 },
      { "mongodb://localhost/?w=mytag&wtimeoutms=1000", true, MONGOC_WRITE_CONCERN_W_TAG, "mytag", 1000 },
      { NULL }
   };

   /* Suppress warnings from two invalid URIs ("journal" and "w" conflict) */
   suppress_one_message();
   suppress_one_message();

   for (i = 0; tests [i].uri; i++) {
      t = &tests [i];

      uri = mongoc_uri_new (t->uri);
      if (t->parses) {
         assert (uri);
      } else {
         assert (!uri);
         continue;
      }

      wr = mongoc_uri_get_write_concern (uri);
      assert (wr);

      assert (t->w == mongoc_write_concern_get_w (wr));

      if (t->wtag) {
         assert (0 == strcmp (t->wtag, mongoc_write_concern_get_wtag (wr)));
      }

      if (t->wtimeoutms) {
         assert (t->wtimeoutms == mongoc_write_concern_get_wtimeout (wr));
      }

      mongoc_uri_destroy (uri);
   }
}

static void
test_mongoc_uri_read_concern (void)
{
   const mongoc_read_concern_t *rc;
   mongoc_uri_t *uri;

   uri = mongoc_uri_new ("mongodb://localhost/?readConcernLevel=majority");
   rc = mongoc_uri_get_read_concern (uri);
   ASSERT_CMPSTR (mongoc_read_concern_get_level (rc), "majority");
   mongoc_uri_destroy (uri);

   uri = mongoc_uri_new ("mongodb://localhost/?readConcernLevel=" MONGOC_READ_CONCERN_LEVEL_MAJORITY);
   rc = mongoc_uri_get_read_concern (uri);
   ASSERT_CMPSTR (mongoc_read_concern_get_level (rc), "majority");
   mongoc_uri_destroy (uri);


   uri = mongoc_uri_new ("mongodb://localhost/?readConcernLevel=local");
   rc = mongoc_uri_get_read_concern (uri);
   ASSERT_CMPSTR (mongoc_read_concern_get_level (rc), "local");
   mongoc_uri_destroy (uri);

   uri = mongoc_uri_new ("mongodb://localhost/?readConcernLevel=" MONGOC_READ_CONCERN_LEVEL_LOCAL);
   rc = mongoc_uri_get_read_concern (uri);
   ASSERT_CMPSTR (mongoc_read_concern_get_level (rc), "local");
   mongoc_uri_destroy (uri);


   uri = mongoc_uri_new ("mongodb://localhost/?readConcernLevel=randomstuff");
   rc = mongoc_uri_get_read_concern (uri);
   ASSERT_CMPSTR (mongoc_read_concern_get_level (rc), "randomstuff");
   mongoc_uri_destroy (uri);


   uri = mongoc_uri_new ("mongodb://localhost/");
   rc = mongoc_uri_get_read_concern (uri);
   ASSERT (mongoc_read_concern_get_level (rc) == NULL);
   mongoc_uri_destroy (uri);


   uri = mongoc_uri_new ("mongodb://localhost/?readConcernLevel=");
   rc = mongoc_uri_get_read_concern (uri);
   ASSERT_CMPSTR (mongoc_read_concern_get_level (rc), "");
   mongoc_uri_destroy (uri);
}



void
test_uri_install (TestSuite *suite)
{
   TestSuite_Add (suite, "/Uri/new", test_mongoc_uri_new);
   TestSuite_Add (suite, "/Uri/new_for_host_port", test_mongoc_uri_new_for_host_port);
   TestSuite_Add (suite, "/Uri/unescape", test_mongoc_uri_unescape);
   TestSuite_Add (suite, "/Uri/read_prefs", test_mongoc_uri_read_prefs);
   TestSuite_Add (suite, "/Uri/read_concern", test_mongoc_uri_read_concern);
   TestSuite_Add (suite, "/Uri/write_concern", test_mongoc_uri_write_concern);
   TestSuite_Add (suite, "/HostList/from_string", test_mongoc_host_list_from_string);
   TestSuite_Add (suite, "/Uri/functions", test_mongoc_uri_functions);
}
