#include <openssl/err.h>
#include <mongoc.h>

#include "ssl-test.h"
#include "TestSuite.h"

#define HOST "mongodb.com"

#define TRUST_DIR "tests/trust_dir"
#define VERIFY_DIR TRUST_DIR "/verify"
#define CRLFILE TRUST_DIR "/crl/root.crl.pem"
#define CAFILE TRUST_DIR "/verify/mongo_root.pem"
#define PEMFILE_PASS TRUST_DIR "/keys/pass.mongodb.com.pem"
#define PEMFILE_ALT TRUST_DIR "/keys/alt.mongodb.com.pem"
#define PEMFILE_LOCALHOST TRUST_DIR "/keys/127.0.0.1.pem"
#define PEMFILE_NOPASS TRUST_DIR "/keys/mongodb.com.pem"
#define PEMFILE_REV TRUST_DIR "/keys/rev.mongodb.com.pem"
#define PASSWORD "testpass"

static void
test_mongoc_tls_no_certs (void)
{
   mongoc_ssl_opt_t sopt = { 0 };
   mongoc_ssl_opt_t copt = { 0 };
   ssl_test_result_t sr;
   ssl_test_result_t cr;

   ssl_test (&copt, &sopt, "doesnt_matter", &cr, &sr);

   ASSERT (cr.result == SSL_TEST_SSL_HANDSHAKE);
   ASSERT (sr.result == SSL_TEST_SSL_HANDSHAKE);
}


static void
test_mongoc_tls_password (void)
{
   mongoc_ssl_opt_t sopt = { 0 };
   mongoc_ssl_opt_t copt = { 0 };
   ssl_test_result_t sr;
   ssl_test_result_t cr;

   sopt.pem_file = PEMFILE_PASS;
   sopt.ca_file = CAFILE;
   sopt.pem_pwd = PASSWORD;

   copt.ca_file = CAFILE;

   ssl_test (&copt, &sopt, "pass.mongodb.com", &cr, &sr);

   ASSERT (cr.result == SSL_TEST_SUCCESS);
   ASSERT (sr.result == SSL_TEST_SUCCESS);
}

static void
test_mongoc_tls_bad_password (void)
{
   mongoc_ssl_opt_t sopt = { 0 };
   mongoc_ssl_opt_t copt = { 0 };
   ssl_test_result_t sr;
   ssl_test_result_t cr;

   sopt.pem_file = PEMFILE_PASS;
   sopt.ca_file = CAFILE;
   sopt.pem_pwd = "badpass";

   copt.ca_file = CAFILE;

   ssl_test (&copt, &sopt, "pass.mongodb.com", &cr, &sr);

   ASSERT (cr.result == SSL_TEST_SSL_HANDSHAKE);
   ASSERT (sr.result == SSL_TEST_SSL_INIT);
}


static void
test_mongoc_tls_no_verify (void)
{
   mongoc_ssl_opt_t sopt = { 0 };
   mongoc_ssl_opt_t copt = { 0 };
   ssl_test_result_t sr;
   ssl_test_result_t cr;

   sopt.pem_file = PEMFILE_NOPASS;
   sopt.ca_file = CAFILE;

   copt.ca_file = CAFILE;
   copt.weak_cert_validation = 1;

   ssl_test (&copt, &sopt, "bad_domain.com", &cr, &sr);

   ASSERT (cr.result == SSL_TEST_SUCCESS);
   ASSERT (sr.result == SSL_TEST_SUCCESS);
}


static void
test_mongoc_tls_bad_verify (void)
{
   mongoc_ssl_opt_t sopt = { 0 };
   mongoc_ssl_opt_t copt = { 0 };
   ssl_test_result_t sr;
   ssl_test_result_t cr;

   sopt.pem_file = PEMFILE_NOPASS;
   sopt.ca_file = CAFILE;

   copt.ca_file = CAFILE;

   ssl_test (&copt, &sopt, "bad_domain.com", &cr, &sr);

   ASSERT (cr.result == SSL_TEST_SSL_VERIFY);
   ASSERT (sr.result == SSL_TEST_TIMEOUT);
}


static void
test_mongoc_tls_basic (void)
{
   mongoc_ssl_opt_t sopt = { 0 };
   mongoc_ssl_opt_t copt = { 0 };
   ssl_test_result_t sr;
   ssl_test_result_t cr;

   sopt.pem_file = PEMFILE_NOPASS;
   sopt.ca_file = CAFILE;

   copt.ca_file = CAFILE;

   ssl_test (&copt, &sopt, HOST, &cr, &sr);

   ASSERT (cr.result == SSL_TEST_SUCCESS);
   ASSERT (sr.result == SSL_TEST_SUCCESS);
}


static void
test_mongoc_tls_crl (void)
{
   mongoc_ssl_opt_t sopt = { 0 };
   mongoc_ssl_opt_t copt = { 0 };
   ssl_test_result_t sr;
   ssl_test_result_t cr;

   sopt.pem_file = PEMFILE_REV;
   sopt.ca_file = CAFILE;

   copt.ca_file = CAFILE;
   copt.crl_file = CRLFILE;

   ssl_test (&copt, &sopt, "rev.mongodb.com", &cr, &sr);

   ASSERT (cr.result == SSL_TEST_SSL_HANDSHAKE);
   ASSERT (sr.result == SSL_TEST_SSL_HANDSHAKE);
}


static void
test_mongoc_tls_altname (void)
{
   mongoc_ssl_opt_t sopt = { 0 };
   mongoc_ssl_opt_t copt = { 0 };
   ssl_test_result_t sr;
   ssl_test_result_t cr;

   sopt.pem_file = PEMFILE_ALT;
   sopt.ca_file = CAFILE;

   copt.ca_file = CAFILE;

   ssl_test (&copt, &sopt, "alt2.mongodb.com", &cr, &sr);

   ASSERT (cr.result == SSL_TEST_SUCCESS);
   ASSERT (sr.result == SSL_TEST_SUCCESS);
}


static void
test_mongoc_tls_wild (void)
{
   mongoc_ssl_opt_t sopt = { 0 };
   mongoc_ssl_opt_t copt = { 0 };
   ssl_test_result_t sr;
   ssl_test_result_t cr;

   sopt.pem_file = PEMFILE_ALT;
   sopt.ca_file = CAFILE;

   copt.ca_file = CAFILE;

   ssl_test (&copt, &sopt, "unicorn.wild.mongodb.com", &cr, &sr);

   ASSERT (cr.result == SSL_TEST_SUCCESS);
   ASSERT (sr.result == SSL_TEST_SUCCESS);
}


static void
test_mongoc_tls_ip (void)
{
   mongoc_ssl_opt_t sopt = { 0 };
   mongoc_ssl_opt_t copt = { 0 };
   ssl_test_result_t sr;
   ssl_test_result_t cr;

   sopt.pem_file = PEMFILE_ALT;
   sopt.ca_file = CAFILE;

   copt.ca_file = CAFILE;

   ssl_test (&copt, &sopt, "10.0.0.1", &cr, &sr);

   ASSERT (cr.result == SSL_TEST_SUCCESS);
   ASSERT (sr.result == SSL_TEST_SUCCESS);
}


#ifndef _WIN32
static void
test_mongoc_tls_trust_dir (void)
{
   mongoc_ssl_opt_t sopt = { 0 };
   mongoc_ssl_opt_t copt = { 0 };
   ssl_test_result_t sr;
   ssl_test_result_t cr;

   sopt.pem_file = PEMFILE_NOPASS;
   sopt.ca_dir = VERIFY_DIR;

   copt.ca_dir = VERIFY_DIR;

   ssl_test (&copt, &sopt, HOST, &cr, &sr);

   ASSERT (cr.result == SSL_TEST_SUCCESS);
   ASSERT (sr.result == SSL_TEST_SUCCESS);
}
#endif


void
test_stream_tls_install (TestSuite *suite)
{
   TestSuite_Add (suite, "/TLS/altname", test_mongoc_tls_altname);
   TestSuite_Add (suite, "/TLS/bad_password", test_mongoc_tls_bad_password);
   TestSuite_Add (suite, "/TLS/bad_verify", test_mongoc_tls_bad_verify);
   TestSuite_Add (suite, "/TLS/basic", test_mongoc_tls_basic);
   TestSuite_Add (suite, "/TLS/crl", test_mongoc_tls_crl);
   TestSuite_Add (suite, "/TLS/ip", test_mongoc_tls_ip);
   TestSuite_Add (suite, "/TLS/no_certs", test_mongoc_tls_no_certs);
   TestSuite_Add (suite, "/TLS/no_verify", test_mongoc_tls_no_verify);
   TestSuite_Add (suite, "/TLS/password", test_mongoc_tls_password);
#ifndef _WIN32
   TestSuite_Add (suite, "/TLS/trust_dir", test_mongoc_tls_trust_dir);
#endif
   TestSuite_Add (suite, "/TLS/wild", test_mongoc_tls_wild);
}
