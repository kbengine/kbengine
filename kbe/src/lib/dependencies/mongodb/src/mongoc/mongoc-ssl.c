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

#include "mongoc-config.h"

#ifdef MONGOC_ENABLE_SSL

#include <bson.h>
#include <limits.h>
#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/x509v3.h>
#include <openssl/crypto.h>

#include <string.h>

#include "mongoc-init.h"
#include "mongoc-socket.h"
#include "mongoc-ssl.h"
#include "mongoc-ssl-private.h"
#include "mongoc-trace.h"
#include "mongoc-thread-private.h"
#include "mongoc-util-private.h"

/* TODO: we could populate these from a config or something further down the
 * road for providing defaults */
#ifndef MONGOC_SSL_DEFAULT_TRUST_FILE
#define MONGOC_SSL_DEFAULT_TRUST_FILE NULL
#endif
#ifndef MONGOC_SSL_DEFAULT_TRUST_DIR
#define MONGOC_SSL_DEFAULT_TRUST_DIR NULL
#endif

static
mongoc_ssl_opt_t gMongocSslOptDefault = {
   NULL,
   NULL,
   MONGOC_SSL_DEFAULT_TRUST_FILE,
   MONGOC_SSL_DEFAULT_TRUST_DIR,
};

static mongoc_mutex_t * gMongocSslThreadLocks;

static void _mongoc_ssl_thread_startup(void);
static void _mongoc_ssl_thread_cleanup(void);

const mongoc_ssl_opt_t *
mongoc_ssl_opt_get_default (void)
{
   return &gMongocSslOptDefault;
}

/**
 * _mongoc_ssl_init:
 *
 * initialization function for SSL
 *
 * This needs to get called early on and is not threadsafe.  Called by
 * mongoc_init.
 */
void
_mongoc_ssl_init (void)
{
   SSL_CTX *ctx;

   SSL_library_init ();
   SSL_load_error_strings ();
   ERR_load_BIO_strings ();
   OpenSSL_add_all_algorithms ();
   _mongoc_ssl_thread_startup ();

   /*
    * Ensure we also load the ciphers now from the primary thread
    * or we can run into some weirdness on 64-bit Solaris 10 on
    * SPARC with openssl 0.9.7.
    */
   ctx = SSL_CTX_new (SSLv23_method ());
   if (!ctx) {
      MONGOC_ERROR ("Failed to initialize OpenSSL.");
   }

   SSL_CTX_free (ctx);
}

void
_mongoc_ssl_cleanup (void)
{
   _mongoc_ssl_thread_cleanup ();
}

static int
_mongoc_ssl_password_cb (char *buf,
                         int   num,
                         int   rwflag,
                         void *user_data)
{
   char *pass = (char *)user_data;
   int pass_len = (int)strlen (pass);

   if (num < pass_len + 1) {
      return 0;
   }

   bson_strncpy (buf, pass, num);
   return pass_len;
}


/** mongoc_ssl_hostcheck
 *
 * rfc 6125 match a given hostname against a given pattern
 *
 * Patterns come from DNS common names or subjectAltNames.
 *
 * This code is meant to implement RFC 6125 6.4.[1-3]
 *
 */
static bool
_mongoc_ssl_hostcheck (const char *pattern,
                       const char *hostname)
{
   const char *pattern_label_end;
   const char *pattern_wildcard;
   const char *hostname_label_end;
   size_t prefixlen;
   size_t suffixlen;

   pattern_wildcard = strchr (pattern, '*');

   if (pattern_wildcard == NULL) {
      return strcasecmp (pattern, hostname) == 0;
   }

   pattern_label_end = strchr (pattern, '.');

   /* Bail out on wildcarding in a couple of situations:
    * o we don't have 2 dots - we're not going to wildcard root tlds
    * o the wildcard isn't in the left most group (separated by dots)
    * o the pattern is embedded in an A-label or U-label
    */
   if (pattern_label_end == NULL ||
       strchr (pattern_label_end + 1, '.') == NULL ||
       pattern_wildcard > pattern_label_end ||
       strncasecmp (pattern, "xn--", 4) == 0) {
      return strcasecmp (pattern, hostname) == 0;
   }

   hostname_label_end = strchr (hostname, '.');

   /* we know we have a dot in the pattern, we need one in the hostname */
   if (hostname_label_end == NULL ||
       strcasecmp (pattern_label_end, hostname_label_end)) {
      return 0;
   }

   /* The wildcard must match at least one character, so the left part of the
    * hostname is at least as large as the left part of the pattern. */
   if ((hostname_label_end - hostname) < (pattern_label_end - pattern)) {
      return 0;
   }

   /* If the left prefix group before the star matches and right of the star
    * matches... we have a wildcard match */
   prefixlen = pattern_wildcard - pattern;
   suffixlen = pattern_label_end - (pattern_wildcard + 1);
   return strncasecmp (pattern, hostname, prefixlen) == 0 &&
          strncasecmp (pattern_wildcard + 1, hostname_label_end - suffixlen,
                       suffixlen) == 0;
}


/** check if a provided cert matches a passed hostname
 */
bool
_mongoc_ssl_check_cert (SSL        *ssl,
                        const char *host,
                        bool weak_cert_validation)
{
   X509 *peer;
   X509_NAME *subject_name;
   X509_NAME_ENTRY *entry;
   ASN1_STRING *entry_data;
   char *check;
   int length;
   int idx;
   int r = 0;
   long verify_status;

   size_t addrlen = 0;
   struct in_addr addr;
   int i;
   int n_sans = -1;
   int target = GEN_DNS;

   STACK_OF (GENERAL_NAME) * sans = NULL;

   BSON_ASSERT (ssl);
   BSON_ASSERT (host);

   if (weak_cert_validation) {
      return true;
   }

   /** if the host looks like an IP address, match that, otherwise we assume we
    * have a DNS name */
   if (inet_pton (AF_INET, host, &addr)) {
      target = GEN_IPADD;
      addrlen = sizeof (struct in_addr);
   }

   peer = SSL_get_peer_certificate (ssl);

   if (!peer) {
      return false;
   }

   verify_status = SSL_get_verify_result (ssl);

   /** TODO: should we return this somehow? */

   if (verify_status == X509_V_OK) {
      /* get's a stack of alt names that we can iterate through */
      sans = (STACK_OF (GENERAL_NAME) *) X509_get_ext_d2i (
         (X509 *)peer, NID_subject_alt_name, NULL, NULL);

      if (sans) {
         n_sans = sk_GENERAL_NAME_num (sans);

         /* loop through the stack, or until we find a match */
         for (i = 0; i < n_sans && !r; i++) {
            const GENERAL_NAME *name = sk_GENERAL_NAME_value (sans, i);

            /* skip entries that can't apply, I.e. IP entries if we've got a
             * DNS host */
            if (name->type == target) {
               check = (char *)ASN1_STRING_data (name->d.ia5);
               length = ASN1_STRING_length (name->d.ia5);

               switch (target) {
               case GEN_DNS:

                  /* check that we don't have an embedded null byte */
                  if ((length == bson_strnlen (check, length)) &&
                      _mongoc_ssl_hostcheck (check, host)) {
                     r = 1;
                  }

                  break;
               case GEN_IPADD:

                  if ((length == addrlen) && !memcmp (check, &addr, length)) {
                     r = 1;
                  }

                  break;
               default:
                  BSON_ASSERT (0);
                  break;
               }
            }
         }
         GENERAL_NAMES_free (sans);
      } else {
         subject_name = X509_get_subject_name (peer);

         if (subject_name) {
            idx = -1;
            i = -1;

            /* skip to the last common name */
            while ((idx =
                       X509_NAME_get_index_by_NID (subject_name, NID_commonName, i)) >= 0) {
               i = idx;
            }

            if (i >= 0) {
               entry = X509_NAME_get_entry (subject_name, i);
               entry_data = X509_NAME_ENTRY_get_data (entry);

               if (entry_data) {
                  /* TODO: I've heard tell that old versions of SSL crap out
                   * when calling ASN1_STRING_to_UTF8 on already utf8 data.
                   * Check up on that */
                  length = ASN1_STRING_to_UTF8 ((unsigned char **)&check,
                                                entry_data);

                  if (length >= 0) {
                     /* check for embedded nulls */
                     if ((length == bson_strnlen (check, length)) &&
                         _mongoc_ssl_hostcheck (check, host)) {
                        r = 1;
                     }

                     OPENSSL_free (check);
                  }
               }
            }
         }
      }
   }

   X509_free (peer);
   return r;
}


static bool
_mongoc_ssl_setup_ca (SSL_CTX    *ctx,
                      const char *cert,
                      const char *cert_dir)
{
   BSON_ASSERT(ctx);
   BSON_ASSERT(cert || cert_dir);

   if (!SSL_CTX_load_verify_locations (ctx, cert, cert_dir)) {
      return 0;
   }

   return 1;
}


static bool
_mongoc_ssl_setup_crl (SSL_CTX    *ctx,
                       const char *crlfile)
{
   X509_STORE *store;
   X509_LOOKUP *lookup;
   int status;

   store = SSL_CTX_get_cert_store (ctx);
   X509_STORE_set_flags (store, X509_V_FLAG_CRL_CHECK);

   lookup = X509_STORE_add_lookup (store, X509_LOOKUP_file ());

   status = X509_load_crl_file (lookup, crlfile, X509_FILETYPE_PEM);

   return status != 0;
}


static bool
_mongoc_ssl_setup_pem_file (SSL_CTX    *ctx,
                            const char *pem_file,
                            const char *password)
{
   if (!SSL_CTX_use_certificate_chain_file (ctx, pem_file)) {
      return 0;
   }

   if (password) {
      SSL_CTX_set_default_passwd_cb_userdata (ctx, (void *)password);
      SSL_CTX_set_default_passwd_cb (ctx, _mongoc_ssl_password_cb);
   }

   if (!(SSL_CTX_use_PrivateKey_file (ctx, pem_file, SSL_FILETYPE_PEM))) {
      return 0;
   }

   if (!(SSL_CTX_check_private_key (ctx))) {
      return 0;
   }

   return 1;
}


/**
 * _mongoc_ssl_ctx_new:
 *
 * Create a new ssl context declaratively
 *
 * The opt.pem_pwd parameter, if passed, must exist for the life of this
 * context object (for storing and loading the associated pem file)
 */
SSL_CTX *
_mongoc_ssl_ctx_new (mongoc_ssl_opt_t *opt)
{
   SSL_CTX *ctx = NULL;

   /*
    * Ensure we are initialized. This is safe to call multiple times.
    */
   mongoc_init ();

   ctx = SSL_CTX_new (SSLv23_method ());

   BSON_ASSERT (ctx);

   /* SSL_OP_ALL - Activate all bug workaround options, to support buggy client SSL's.
    * SSL_OP_NO_SSLv2 - Disable SSL v2 support */
   SSL_CTX_set_options (ctx, (SSL_OP_ALL | SSL_OP_NO_SSLv2));

   /* HIGH - Enable strong ciphers
    * !EXPORT - Disable export ciphers (40/56 bit)
    * !aNULL - Disable anonymous auth ciphers
    * @STRENGTH - Sort ciphers based on strength */
   SSL_CTX_set_cipher_list (ctx, "HIGH:!EXPORT:!aNULL@STRENGTH");

   /* If renegotiation is needed, don't return from recv() or send() until it's successful.
    * Note: this is for blocking sockets only. */
   SSL_CTX_set_mode (ctx, SSL_MODE_AUTO_RETRY);

   /* TODO: does this cargo cult actually matter?
    * Disable session caching (see SERVER-10261) */
   SSL_CTX_set_session_cache_mode (ctx, SSL_SESS_CACHE_OFF);

   /* Load in verification certs, private keys and revocation lists */
   if ((!opt->pem_file ||
        _mongoc_ssl_setup_pem_file (ctx, opt->pem_file, opt->pem_pwd))
       && (!(opt->ca_file ||
             opt->ca_dir) ||
           _mongoc_ssl_setup_ca (ctx, opt->ca_file, opt->ca_dir))
       && (!opt->crl_file || _mongoc_ssl_setup_crl (ctx, opt->crl_file))
       ) {
      return ctx;
   } else {
      SSL_CTX_free (ctx);
      return NULL;
   }
}


char *
_mongoc_ssl_extract_subject (const char *filename)
{
   X509_NAME *subject = NULL;
   X509 *cert = NULL;
   BIO *certbio = NULL;
   BIO *strbio = NULL;
   char *str = NULL;
   int ret;

   if (!filename) {
      return NULL;
   }

   certbio = BIO_new (BIO_s_file ());
   strbio = BIO_new (BIO_s_mem ());;

   BSON_ASSERT (certbio);
   BSON_ASSERT (strbio);

   BIO_read_filename (certbio, filename);

   if ((cert = PEM_read_bio_X509 (certbio, NULL, 0, NULL))) {
      if ((subject = X509_get_subject_name (cert))) {
         ret = X509_NAME_print_ex (strbio, subject, 0, XN_FLAG_RFC2253);

         if ((ret > 0) && (ret < INT_MAX)) {
            str = (char *)bson_malloc (ret + 2);
            BIO_gets (strbio, str, ret + 1);
            str [ret] = '\0';
         }
      }
   }

   if (cert) {
      X509_free (cert);
   }

   if (certbio) {
      BIO_free (certbio);
   }

   if (strbio) {
      BIO_free (strbio);
   }

   return str;
}

#ifdef _WIN32

static unsigned long
_mongoc_ssl_thread_id_callback (void)
{
   unsigned long ret;

   ret = (unsigned long)GetCurrentThreadId ();
   return ret;
}

#else

static unsigned long
_mongoc_ssl_thread_id_callback (void)
{
   unsigned long ret;

   ret = (unsigned long)pthread_self ();
   return ret;
}

#endif

static void
_mongoc_ssl_thread_locking_callback (int         mode,
                                     int         type,
                                     const char *file,
                                     int         line)
{
   if (mode & CRYPTO_LOCK) {
      mongoc_mutex_lock (&gMongocSslThreadLocks[type]);
   } else {
      mongoc_mutex_unlock (&gMongocSslThreadLocks[type]);
   }
}

static void
_mongoc_ssl_thread_startup (void)
{
   int i;

   gMongocSslThreadLocks = (mongoc_mutex_t *)OPENSSL_malloc (CRYPTO_num_locks () * sizeof (mongoc_mutex_t));

   for (i = 0; i < CRYPTO_num_locks (); i++) {
      mongoc_mutex_init(&gMongocSslThreadLocks[i]);
   }

   if (!CRYPTO_get_locking_callback ()) {
      CRYPTO_set_locking_callback (_mongoc_ssl_thread_locking_callback);
      CRYPTO_set_id_callback (_mongoc_ssl_thread_id_callback);
   }
}

static void
_mongoc_ssl_thread_cleanup (void)
{
   int i;

   if (CRYPTO_get_locking_callback () == _mongoc_ssl_thread_locking_callback) {
      CRYPTO_set_locking_callback (NULL);
      CRYPTO_set_id_callback (NULL);
   }

   for (i = 0; i < CRYPTO_num_locks (); i++) {
      mongoc_mutex_destroy (&gMongocSslThreadLocks[i]);
   }
   OPENSSL_free (gMongocSslThreadLocks);
}

#endif
