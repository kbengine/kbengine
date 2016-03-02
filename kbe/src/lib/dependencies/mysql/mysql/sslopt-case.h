/* Copyright (c) 2000, 2015, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#if defined(HAVE_OPENSSL) && !defined(EMBEDDED_LIBRARY)
#ifdef MYSQL_CLIENT
    case OPT_SSL_SSL:
      /*
        A client side --ssl option handling.
        --ssl=1 means enforce (use=1, enforce=1)
	--ssl=0 means can't enforce (use=0, enforce=0)
	no --ssl means default : no enforce (use=1), just try (enforce=1)
      */
      opt_ssl_enforce= opt_use_ssl;
      break;
#endif
    case OPT_SSL_KEY:
    case OPT_SSL_CERT:
    case OPT_SSL_CA:
    case OPT_SSL_CAPATH:
    case OPT_SSL_CIPHER:
    case OPT_SSL_CRL:
    case OPT_SSL_CRLPATH:
    /*
      Enable use of SSL if we are using any ssl option
      One can disable SSL later by using --skip-ssl or --ssl=0
    */
      opt_use_ssl= TRUE;
    /* crl has no effect in yaSSL */
#ifdef HAVE_YASSL
      opt_ssl_crl= NULL;
      opt_ssl_crlpath= NULL;
#endif
      break;
    case OPT_TLS_VERSION:
#ifdef MYSQL_CLIENT
      opt_ssl_enforce= TRUE;
#endif
      opt_use_ssl= TRUE;
      break;
#endif
