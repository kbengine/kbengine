/* ssl/d1_clnt.c */
/* 
 * DTLS implementation written by Nagendra Modadugu
 * (nagendra@cs.stanford.edu) for the OpenSSL project 2005.  
 */
/* ====================================================================
 * Copyright (c) 1999-2005 The OpenSSL Project.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer. 
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. All advertising materials mentioning features or use of this
 *    software must display the following acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit. (http://www.OpenSSL.org/)"
 *
 * 4. The names "OpenSSL Toolkit" and "OpenSSL Project" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. For written permission, please contact
 *    openssl-core@OpenSSL.org.
 *
 * 5. Products derived from this software may not be called "OpenSSL"
 *    nor may "OpenSSL" appear in their names without prior written
 *    permission of the OpenSSL Project.
 *
 * 6. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit (http://www.OpenSSL.org/)"
 *
 * THIS SOFTWARE IS PROVIDED BY THE OpenSSL PROJECT ``AS IS'' AND ANY
 * EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE OpenSSL PROJECT OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 * ====================================================================
 *
 * This product includes cryptographic software written by Eric Young
 * (eay@cryptsoft.com).  This product includes software written by Tim
 * Hudson (tjh@cryptsoft.com).
 *
 */
/* Copyright (C) 1995-1998 Eric Young (eay@cryptsoft.com)
 * All rights reserved.
 *
 * This package is an SSL implementation written
 * by Eric Young (eay@cryptsoft.com).
 * The implementation was written so as to conform with Netscapes SSL.
 * 
 * This library is free for commercial and non-commercial use as long as
 * the following conditions are aheared to.  The following conditions
 * apply to all code found in this distribution, be it the RC4, RSA,
 * lhash, DES, etc., code; not just the SSL code.  The SSL documentation
 * included with this distribution is covered by the same copyright terms
 * except that the holder is Tim Hudson (tjh@cryptsoft.com).
 * 
 * Copyright remains Eric Young's, and as such any Copyright notices in
 * the code are not to be removed.
 * If this package is used in a product, Eric Young should be given attribution
 * as the author of the parts of the library used.
 * This can be in the form of a textual message at program startup or
 * in documentation (online or textual) provided with the package.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    "This product includes cryptographic software written by
 *     Eric Young (eay@cryptsoft.com)"
 *    The word 'cryptographic' can be left out if the rouines from the library
 *    being used are not cryptographic related :-).
 * 4. If you include any Windows specific code (or a derivative thereof) from 
 *    the apps directory (application code) you must include an acknowledgement:
 *    "This product includes software written by Tim Hudson (tjh@cryptsoft.com)"
 * 
 * THIS SOFTWARE IS PROVIDED BY ERIC YOUNG ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * 
 * The licence and distribution terms for any publically available version or
 * derivative of this code cannot be changed.  i.e. this code cannot simply be
 * copied and put under another distribution licence
 * [including the GNU Public Licence.]
 */

#include <stdio.h>
#include "ssl_locl.h"
#include "kssl_lcl.h"
#include <openssl/buffer.h>
#include <openssl/rand.h>
#include <openssl/objects.h>
#include <openssl/evp.h>
#include <openssl/md5.h>
#ifndef OPENSSL_NO_DH
#include <openssl/dh.h>
#endif

static SSL_METHOD *dtls1_get_client_method(int ver);
static int dtls1_get_hello_verify(SSL *s);

static SSL_METHOD *dtls1_get_client_method(int ver)
	{
	if (ver == DTLS1_VERSION)
		return(DTLSv1_client_method());
	else
		return(NULL);
	}

IMPLEMENT_dtls1_meth_func(DTLSv1_client_method,
			ssl_undefined_function,
			dtls1_connect,
			dtls1_get_client_method)

int dtls1_connect(SSL *s)
	{
	BUF_MEM *buf=NULL;
	unsigned long Time=(unsigned long)time(NULL),l;
	long num1;
	void (*cb)(const SSL *ssl,int type,int val)=NULL;
	int ret= -1;
	int new_state,state,skip=0;;

	RAND_add(&Time,sizeof(Time),0);
	ERR_clear_error();
	clear_sys_error();

	if (s->info_callback != NULL)
		cb=s->info_callback;
	else if (s->ctx->info_callback != NULL)
		cb=s->ctx->info_callback;
	
	s->in_handshake++;
	if (!SSL_in_init(s) || SSL_in_before(s)) SSL_clear(s); 

	for (;;)
		{
		state=s->state;

		switch(s->state)
			{
		case SSL_ST_RENEGOTIATE:
			s->new_session=1;
			s->state=SSL_ST_CONNECT;
			s->ctx->stats.sess_connect_renegotiate++;
			/* break */
		case SSL_ST_BEFORE:
		case SSL_ST_CONNECT:
		case SSL_ST_BEFORE|SSL_ST_CONNECT:
		case SSL_ST_OK|SSL_ST_CONNECT:

			s->server=0;
			if (cb != NULL) cb(s,SSL_CB_HANDSHAKE_START,1);

			if ((s->version & 0xff00 ) != (DTLS1_VERSION & 0xff00))
				{
				SSLerr(SSL_F_DTLS1_CONNECT, ERR_R_INTERNAL_ERROR);
				ret = -1;
				goto end;
				}
				
			/* s->version=SSL3_VERSION; */
			s->type=SSL_ST_CONNECT;

			if (s->init_buf == NULL)
				{
				if ((buf=BUF_MEM_new()) == NULL)
					{
					ret= -1;
					goto end;
					}
				if (!BUF_MEM_grow(buf,SSL3_RT_MAX_PLAIN_LENGTH))
					{
					ret= -1;
					goto end;
					}
				s->init_buf=buf;
				buf=NULL;
				}

			if (!ssl3_setup_buffers(s)) { ret= -1; goto end; }

			/* setup buffing BIO */
			if (!ssl_init_wbio_buffer(s,0)) { ret= -1; goto end; }

			/* don't push the buffering BIO quite yet */

			s->state=SSL3_ST_CW_CLNT_HELLO_A;
			s->ctx->stats.sess_connect++;
			s->init_num=0;
			/* mark client_random uninitialized */
			memset(s->s3->client_random,0,sizeof(s->s3->client_random));
			break;

		case SSL3_ST_CW_CLNT_HELLO_A:
		case SSL3_ST_CW_CLNT_HELLO_B:

			s->shutdown=0;

			/* every DTLS ClientHello resets Finished MAC */
			ssl3_init_finished_mac(s);

			ret=dtls1_client_hello(s);
			if (ret <= 0) goto end;

			if ( s->d1->send_cookie)
				{
				s->state=SSL3_ST_CW_FLUSH;
				s->s3->tmp.next_state=SSL3_ST_CR_SRVR_HELLO_A;
				}
			else
				s->state=SSL3_ST_CR_SRVR_HELLO_A;

			s->init_num=0;

			/* turn on buffering for the next lot of output */
			if (s->bbio != s->wbio)
				s->wbio=BIO_push(s->bbio,s->wbio);

			break;

		case SSL3_ST_CR_SRVR_HELLO_A:
		case SSL3_ST_CR_SRVR_HELLO_B:
			ret=ssl3_get_server_hello(s);
			if (ret <= 0) goto end;
			else
				{
				if (s->hit)
					s->state=SSL3_ST_CR_FINISHED_A;
				else
					s->state=DTLS1_ST_CR_HELLO_VERIFY_REQUEST_A;
				}
			s->init_num=0;
			break;

		case DTLS1_ST_CR_HELLO_VERIFY_REQUEST_A:
		case DTLS1_ST_CR_HELLO_VERIFY_REQUEST_B:

			ret = dtls1_get_hello_verify(s);
			if ( ret <= 0)
				goto end;
			if ( s->d1->send_cookie) /* start again, with a cookie */
				s->state=SSL3_ST_CW_CLNT_HELLO_A;
			else
				s->state = SSL3_ST_CR_CERT_A;
			s->init_num = 0;
			break;

		case SSL3_ST_CR_CERT_A:
		case SSL3_ST_CR_CERT_B:
			/* Check if it is anon DH */
			if (!(s->s3->tmp.new_cipher->algorithms & SSL_aNULL))
				{
				ret=ssl3_get_server_certificate(s);
				if (ret <= 0) goto end;
				}
			else
				skip=1;
			s->state=SSL3_ST_CR_KEY_EXCH_A;
			s->init_num=0;
			break;

		case SSL3_ST_CR_KEY_EXCH_A:
		case SSL3_ST_CR_KEY_EXCH_B:
			ret=ssl3_get_key_exchange(s);
			if (ret <= 0) goto end;
			s->state=SSL3_ST_CR_CERT_REQ_A;
			s->init_num=0;

			/* at this point we check that we have the
			 * required stuff from the server */
			if (!ssl3_check_cert_and_algorithm(s))
				{
				ret= -1;
				goto end;
				}
			break;

		case SSL3_ST_CR_CERT_REQ_A:
		case SSL3_ST_CR_CERT_REQ_B:
			ret=ssl3_get_certificate_request(s);
			if (ret <= 0) goto end;
			s->state=SSL3_ST_CR_SRVR_DONE_A;
			s->init_num=0;
			break;

		case SSL3_ST_CR_SRVR_DONE_A:
		case SSL3_ST_CR_SRVR_DONE_B:
			ret=ssl3_get_server_done(s);
			if (ret <= 0) goto end;
			if (s->s3->tmp.cert_req)
				s->state=SSL3_ST_CW_CERT_A;
			else
				s->state=SSL3_ST_CW_KEY_EXCH_A;
			s->init_num=0;

			break;

		case SSL3_ST_CW_CERT_A:
		case SSL3_ST_CW_CERT_B:
		case SSL3_ST_CW_CERT_C:
		case SSL3_ST_CW_CERT_D:
			ret=dtls1_send_client_certificate(s);
			if (ret <= 0) goto end;
			s->state=SSL3_ST_CW_KEY_EXCH_A;
			s->init_num=0;
			break;

		case SSL3_ST_CW_KEY_EXCH_A:
		case SSL3_ST_CW_KEY_EXCH_B:
			ret=dtls1_send_client_key_exchange(s);
			if (ret <= 0) goto end;
			l=s->s3->tmp.new_cipher->algorithms;
			/* EAY EAY EAY need to check for DH fix cert
			 * sent back */
			/* For TLS, cert_req is set to 2, so a cert chain
			 * of nothing is sent, but no verify packet is sent */
			if (s->s3->tmp.cert_req == 1)
				{
				s->state=SSL3_ST_CW_CERT_VRFY_A;
				}
			else
				{
				s->state=SSL3_ST_CW_CHANGE_A;
				s->s3->change_cipher_spec=0;
				}

			s->init_num=0;
			break;

		case SSL3_ST_CW_CERT_VRFY_A:
		case SSL3_ST_CW_CERT_VRFY_B:
			ret=dtls1_send_client_verify(s);
			if (ret <= 0) goto end;
			s->state=SSL3_ST_CW_CHANGE_A;
			s->init_num=0;
			s->s3->change_cipher_spec=0;
			break;

		case SSL3_ST_CW_CHANGE_A:
		case SSL3_ST_CW_CHANGE_B:
			ret=dtls1_send_change_cipher_spec(s,
				SSL3_ST_CW_CHANGE_A,SSL3_ST_CW_CHANGE_B);
			if (ret <= 0) goto end;
			s->state=SSL3_ST_CW_FINISHED_A;
			s->init_num=0;

			s->session->cipher=s->s3->tmp.new_cipher;
#ifdef OPENSSL_NO_COMP
			s->session->compress_meth=0;
#else
			if (s->s3->tmp.new_compression == NULL)
				s->session->compress_meth=0;
			else
				s->session->compress_meth=
					s->s3->tmp.new_compression->id;
#endif
			if (!s->method->ssl3_enc->setup_key_block(s))
				{
				ret= -1;
				goto end;
				}

			if (!s->method->ssl3_enc->change_cipher_state(s,
				SSL3_CHANGE_CIPHER_CLIENT_WRITE))
				{
				ret= -1;
				goto end;
				}
			
			dtls1_reset_seq_numbers(s, SSL3_CC_WRITE);
			break;

		case SSL3_ST_CW_FINISHED_A:
		case SSL3_ST_CW_FINISHED_B:
			ret=dtls1_send_finished(s,
				SSL3_ST_CW_FINISHED_A,SSL3_ST_CW_FINISHED_B,
				s->method->ssl3_enc->client_finished_label,
				s->method->ssl3_enc->client_finished_label_len);
			if (ret <= 0) goto end;
			s->state=SSL3_ST_CW_FLUSH;

			/* clear flags */
			s->s3->flags&= ~SSL3_FLAGS_POP_BUFFER;
			if (s->hit)
				{
				s->s3->tmp.next_state=SSL_ST_OK;
				if (s->s3->flags & SSL3_FLAGS_DELAY_CLIENT_FINISHED)
					{
					s->state=SSL_ST_OK;
					s->s3->flags|=SSL3_FLAGS_POP_BUFFER;
					s->s3->delay_buf_pop_ret=0;
					}
				}
			else
				{
				s->s3->tmp.next_state=SSL3_ST_CR_FINISHED_A;
				}
			s->init_num=0;
			/* mark client_random uninitialized */
			memset (s->s3->client_random,0,sizeof(s->s3->client_random));

			break;

		case SSL3_ST_CR_FINISHED_A:
		case SSL3_ST_CR_FINISHED_B:

			ret=ssl3_get_finished(s,SSL3_ST_CR_FINISHED_A,
				SSL3_ST_CR_FINISHED_B);
			if (ret <= 0) goto end;

			if (s->hit)
				s->state=SSL3_ST_CW_CHANGE_A;
			else
				s->state=SSL_ST_OK;
			s->init_num=0;
			break;

		case SSL3_ST_CW_FLUSH:
			/* number of bytes to be flushed */
			num1=BIO_ctrl(s->wbio,BIO_CTRL_INFO,0,NULL);
			if (num1 > 0)
				{
				s->rwstate=SSL_WRITING;
				num1=BIO_flush(s->wbio);
				if (num1 <= 0) { ret= -1; goto end; }
				s->rwstate=SSL_NOTHING;
				}

			s->state=s->s3->tmp.next_state;
			break;

		case SSL_ST_OK:
			/* clean a few things up */
			ssl3_cleanup_key_block(s);

#if 0
			if (s->init_buf != NULL)
				{
				BUF_MEM_free(s->init_buf);
				s->init_buf=NULL;
				}
#endif

			/* If we are not 'joining' the last two packets,
			 * remove the buffering now */
			if (!(s->s3->flags & SSL3_FLAGS_POP_BUFFER))
				ssl_free_wbio_buffer(s);
			/* else do it later in ssl3_write */

			s->init_num=0;
			s->new_session=0;

			ssl_update_cache(s,SSL_SESS_CACHE_CLIENT);
			if (s->hit) s->ctx->stats.sess_hit++;

			ret=1;
			/* s->server=0; */
			s->handshake_func=dtls1_connect;
			s->ctx->stats.sess_connect_good++;

			if (cb != NULL) cb(s,SSL_CB_HANDSHAKE_DONE,1);

			/* done with handshaking */
			s->d1->handshake_read_seq  = 0;
			goto end;
			/* break; */
			
		default:
			SSLerr(SSL_F_DTLS1_CONNECT,SSL_R_UNKNOWN_STATE);
			ret= -1;
			goto end;
			/* break; */
			}

		/* did we do anything */
		if (!s->s3->tmp.reuse_message && !skip)
			{
			if (s->debug)
				{
				if ((ret=BIO_flush(s->wbio)) <= 0)
					goto end;
				}

			if ((cb != NULL) && (s->state != state))
				{
				new_state=s->state;
				s->state=state;
				cb(s,SSL_CB_CONNECT_LOOP,1);
				s->state=new_state;
				}
			}
		skip=0;
		}
end:
	s->in_handshake--;
	if (buf != NULL)
		BUF_MEM_free(buf);
	if (cb != NULL)
		cb(s,SSL_CB_CONNECT_EXIT,ret);
	return(ret);
	}

int dtls1_client_hello(SSL *s)
	{
	unsigned char *buf;
	unsigned char *p,*d;
	unsigned int i,j;
	unsigned long Time,l;
	SSL_COMP *comp;

	buf=(unsigned char *)s->init_buf->data;
	if (s->state == SSL3_ST_CW_CLNT_HELLO_A)
		{
		if ((s->session == NULL) ||
			(s->session->ssl_version != s->version) ||
			(s->session->not_resumable))
			{
			if (!ssl_get_new_session(s,0))
				goto err;
			}
		/* else use the pre-loaded session */

		p=s->s3->client_random;
		/* if client_random is initialized, reuse it, we are
		 * required to use same upon reply to HelloVerify */
		for (i=0;p[i]=='\0' && i<sizeof(s->s3->client_random);i++) ;
		if (i==sizeof(s->s3->client_random))
			{
			Time=(unsigned long)time(NULL);	/* Time */
			l2n(Time,p);
			RAND_pseudo_bytes(p,SSL3_RANDOM_SIZE-4);
			}

		/* Do the message type and length last */
		d=p= &(buf[DTLS1_HM_HEADER_LENGTH]);

		*(p++)=s->version>>8;
		*(p++)=s->version&0xff;
		s->client_version=s->version;

		/* Random stuff */
		memcpy(p,s->s3->client_random,SSL3_RANDOM_SIZE);
		p+=SSL3_RANDOM_SIZE;

		/* Session ID */
		if (s->new_session)
			i=0;
		else
			i=s->session->session_id_length;
		*(p++)=i;
		if (i != 0)
			{
			if (i > sizeof s->session->session_id)
				{
				SSLerr(SSL_F_DTLS1_CLIENT_HELLO, ERR_R_INTERNAL_ERROR);
				goto err;
				}
			memcpy(p,s->session->session_id,i);
			p+=i;
			}
		
		/* cookie stuff */
		if ( s->d1->cookie_len > sizeof(s->d1->cookie))
			{
			SSLerr(SSL_F_DTLS1_CLIENT_HELLO, ERR_R_INTERNAL_ERROR);
			goto err;
			}
		*(p++) = s->d1->cookie_len;
		memcpy(p, s->d1->cookie, s->d1->cookie_len);
		p += s->d1->cookie_len;

		/* Ciphers supported */
		i=ssl_cipher_list_to_bytes(s,SSL_get_ciphers(s),&(p[2]),0);
		if (i == 0)
			{
			SSLerr(SSL_F_DTLS1_CLIENT_HELLO,SSL_R_NO_CIPHERS_AVAILABLE);
			goto err;
			}
		s2n(i,p);
		p+=i;

		/* COMPRESSION */
		if (s->ctx->comp_methods == NULL)
			j=0;
		else
			j=sk_SSL_COMP_num(s->ctx->comp_methods);
		*(p++)=1+j;
		for (i=0; i<j; i++)
			{
			comp=sk_SSL_COMP_value(s->ctx->comp_methods,i);
			*(p++)=comp->id;
			}
		*(p++)=0; /* Add the NULL method */
		
		l=(p-d);
		d=buf;

		d = dtls1_set_message_header(s, d, SSL3_MT_CLIENT_HELLO, l, 0, l);

		s->state=SSL3_ST_CW_CLNT_HELLO_B;
		/* number of bytes to write */
		s->init_num=p-buf;
		s->init_off=0;

		/* buffer the message to handle re-xmits */
		dtls1_buffer_message(s, 0);
		}

	/* SSL3_ST_CW_CLNT_HELLO_B */
	return(dtls1_do_write(s,SSL3_RT_HANDSHAKE));
err:
	return(-1);
	}

static int dtls1_get_hello_verify(SSL *s)
	{
	int n, al, ok = 0;
	unsigned char *data;
	unsigned int cookie_len;

	n=s->method->ssl_get_message(s,
		DTLS1_ST_CR_HELLO_VERIFY_REQUEST_A,
		DTLS1_ST_CR_HELLO_VERIFY_REQUEST_B,
		-1,
		s->max_cert_list,
		&ok);

	if (!ok) return((int)n);

	if (s->s3->tmp.message_type != DTLS1_MT_HELLO_VERIFY_REQUEST)
		{
		s->d1->send_cookie = 0;
		s->s3->tmp.reuse_message=1;
		return(1);
		}

	data = (unsigned char *)s->init_msg;

	if ((data[0] != (s->version>>8)) || (data[1] != (s->version&0xff)))
		{
		SSLerr(SSL_F_DTLS1_GET_HELLO_VERIFY,SSL_R_WRONG_SSL_VERSION);
		s->version=(s->version&0xff00)|data[1];
		al = SSL_AD_PROTOCOL_VERSION;
		goto f_err;
		}
	data+=2;

	cookie_len = *(data++);
	if ( cookie_len > sizeof(s->d1->cookie))
		{
		al=SSL_AD_ILLEGAL_PARAMETER;
		goto f_err;
		}

	memcpy(s->d1->cookie, data, cookie_len);
	s->d1->cookie_len = cookie_len;

	s->d1->send_cookie = 1;
	return 1;

f_err:
	ssl3_send_alert(s, SSL3_AL_FATAL, al);
	return -1;
	}

int dtls1_send_client_key_exchange(SSL *s)
	{
	unsigned char *p,*d;
	int n;
	unsigned long l;
#ifndef OPENSSL_NO_RSA
	unsigned char *q;
	EVP_PKEY *pkey=NULL;
#endif
#ifndef OPENSSL_NO_KRB5
        KSSL_ERR kssl_err;
#endif /* OPENSSL_NO_KRB5 */

	if (s->state == SSL3_ST_CW_KEY_EXCH_A)
		{
		d=(unsigned char *)s->init_buf->data;
		p= &(d[DTLS1_HM_HEADER_LENGTH]);

		l=s->s3->tmp.new_cipher->algorithms;

                /* Fool emacs indentation */
                if (0) {}
#ifndef OPENSSL_NO_RSA
		else if (l & SSL_kRSA)
			{
			RSA *rsa;
			unsigned char tmp_buf[SSL_MAX_MASTER_KEY_LENGTH];

			if (s->session->sess_cert->peer_rsa_tmp != NULL)
				rsa=s->session->sess_cert->peer_rsa_tmp;
			else
				{
				pkey=X509_get_pubkey(s->session->sess_cert->peer_pkeys[SSL_PKEY_RSA_ENC].x509);
				if ((pkey == NULL) ||
					(pkey->type != EVP_PKEY_RSA) ||
					(pkey->pkey.rsa == NULL))
					{
					SSLerr(SSL_F_DTLS1_SEND_CLIENT_KEY_EXCHANGE,ERR_R_INTERNAL_ERROR);
					goto err;
					}
				rsa=pkey->pkey.rsa;
				EVP_PKEY_free(pkey);
				}
				
			tmp_buf[0]=s->client_version>>8;
			tmp_buf[1]=s->client_version&0xff;
			if (RAND_bytes(&(tmp_buf[2]),sizeof tmp_buf-2) <= 0)
					goto err;

			s->session->master_key_length=sizeof tmp_buf;

			q=p;
			/* Fix buf for TLS and [incidentally] DTLS */
			if (s->version > SSL3_VERSION)
				p+=2;
			n=RSA_public_encrypt(sizeof tmp_buf,
				tmp_buf,p,rsa,RSA_PKCS1_PADDING);
#ifdef PKCS1_CHECK
			if (s->options & SSL_OP_PKCS1_CHECK_1) p[1]++;
			if (s->options & SSL_OP_PKCS1_CHECK_2) tmp_buf[0]=0x70;
#endif
			if (n <= 0)
				{
				SSLerr(SSL_F_DTLS1_SEND_CLIENT_KEY_EXCHANGE,SSL_R_BAD_RSA_ENCRYPT);
				goto err;
				}

			/* Fix buf for TLS and [incidentally] DTLS */
			if (s->version > SSL3_VERSION)
				{
				s2n(n,q);
				n+=2;
				}

			s->session->master_key_length=
				s->method->ssl3_enc->generate_master_secret(s,
					s->session->master_key,
					tmp_buf,sizeof tmp_buf);
			OPENSSL_cleanse(tmp_buf,sizeof tmp_buf);
			}
#endif
#ifndef OPENSSL_NO_KRB5
		else if (l & SSL_kKRB5)
                        {
                        krb5_error_code	krb5rc;
                        KSSL_CTX	*kssl_ctx = s->kssl_ctx;
                        /*  krb5_data	krb5_ap_req;  */
                        krb5_data	*enc_ticket;
                        krb5_data	authenticator, *authp = NULL;
			EVP_CIPHER_CTX	ciph_ctx;
			EVP_CIPHER	*enc = NULL;
			unsigned char	iv[EVP_MAX_IV_LENGTH];
			unsigned char	tmp_buf[SSL_MAX_MASTER_KEY_LENGTH];
			unsigned char	epms[SSL_MAX_MASTER_KEY_LENGTH 
						+ EVP_MAX_IV_LENGTH];
			int 		padl, outl = sizeof(epms);

			EVP_CIPHER_CTX_init(&ciph_ctx);

#ifdef KSSL_DEBUG
                        printf("ssl3_send_client_key_exchange(%lx & %lx)\n",
                                l, SSL_kKRB5);
#endif	/* KSSL_DEBUG */

			authp = NULL;
#ifdef KRB5SENDAUTH
			if (KRB5SENDAUTH)  authp = &authenticator;
#endif	/* KRB5SENDAUTH */

                        krb5rc = kssl_cget_tkt(kssl_ctx, &enc_ticket, authp,
				&kssl_err);
			enc = kssl_map_enc(kssl_ctx->enctype);
                        if (enc == NULL)
                            goto err;
#ifdef KSSL_DEBUG
                        {
                        printf("kssl_cget_tkt rtn %d\n", krb5rc);
                        if (krb5rc && kssl_err.text)
			  printf("kssl_cget_tkt kssl_err=%s\n", kssl_err.text);
                        }
#endif	/* KSSL_DEBUG */

                        if (krb5rc)
                                {
                                ssl3_send_alert(s,SSL3_AL_FATAL,
						SSL_AD_HANDSHAKE_FAILURE);
                                SSLerr(SSL_F_DTLS1_SEND_CLIENT_KEY_EXCHANGE,
						kssl_err.reason);
                                goto err;
                                }

			/*  20010406 VRS - Earlier versions used KRB5 AP_REQ
			**  in place of RFC 2712 KerberosWrapper, as in:
			**
                        **  Send ticket (copy to *p, set n = length)
                        **  n = krb5_ap_req.length;
                        **  memcpy(p, krb5_ap_req.data, krb5_ap_req.length);
                        **  if (krb5_ap_req.data)  
                        **    kssl_krb5_free_data_contents(NULL,&krb5_ap_req);
                        **
			**  Now using real RFC 2712 KerberosWrapper
			**  (Thanks to Simon Wilkinson <sxw@sxw.org.uk>)
			**  Note: 2712 "opaque" types are here replaced
			**  with a 2-byte length followed by the value.
			**  Example:
			**  KerberosWrapper= xx xx asn1ticket 0 0 xx xx encpms
			**  Where "xx xx" = length bytes.  Shown here with
			**  optional authenticator omitted.
			*/

			/*  KerberosWrapper.Ticket		*/
			s2n(enc_ticket->length,p);
			memcpy(p, enc_ticket->data, enc_ticket->length);
			p+= enc_ticket->length;
			n = enc_ticket->length + 2;

			/*  KerberosWrapper.Authenticator	*/
			if (authp  &&  authp->length)  
				{
				s2n(authp->length,p);
				memcpy(p, authp->data, authp->length);
				p+= authp->length;
				n+= authp->length + 2;
				
				free(authp->data);
				authp->data = NULL;
				authp->length = 0;
				}
			else
				{
				s2n(0,p);/*  null authenticator length	*/
				n+=2;
				}
 
			if (RAND_bytes(tmp_buf,sizeof tmp_buf) <= 0)
			    goto err;

			/*  20010420 VRS.  Tried it this way; failed.
			**	EVP_EncryptInit_ex(&ciph_ctx,enc, NULL,NULL);
			**	EVP_CIPHER_CTX_set_key_length(&ciph_ctx,
			**				kssl_ctx->length);
			**	EVP_EncryptInit_ex(&ciph_ctx,NULL, key,iv);
			*/

			memset(iv, 0, sizeof iv);  /* per RFC 1510 */
			EVP_EncryptInit_ex(&ciph_ctx,enc, NULL,
				kssl_ctx->key,iv);
			EVP_EncryptUpdate(&ciph_ctx,epms,&outl,tmp_buf,
				sizeof tmp_buf);
			EVP_EncryptFinal_ex(&ciph_ctx,&(epms[outl]),&padl);
			outl += padl;
			if (outl > sizeof epms)
				{
				SSLerr(SSL_F_DTLS1_SEND_CLIENT_KEY_EXCHANGE, ERR_R_INTERNAL_ERROR);
				goto err;
				}
			EVP_CIPHER_CTX_cleanup(&ciph_ctx);

			/*  KerberosWrapper.EncryptedPreMasterSecret	*/
			s2n(outl,p);
			memcpy(p, epms, outl);
			p+=outl;
			n+=outl + 2;

                        s->session->master_key_length=
                                s->method->ssl3_enc->generate_master_secret(s,
					s->session->master_key,
					tmp_buf, sizeof tmp_buf);

			OPENSSL_cleanse(tmp_buf, sizeof tmp_buf);
			OPENSSL_cleanse(epms, outl);
                        }
#endif
#ifndef OPENSSL_NO_DH
		else if (l & (SSL_kEDH|SSL_kDHr|SSL_kDHd))
			{
			DH *dh_srvr,*dh_clnt;

			if (s->session->sess_cert->peer_dh_tmp != NULL)
				dh_srvr=s->session->sess_cert->peer_dh_tmp;
			else
				{
				/* we get them from the cert */
				ssl3_send_alert(s,SSL3_AL_FATAL,SSL_AD_HANDSHAKE_FAILURE);
				SSLerr(SSL_F_DTLS1_SEND_CLIENT_KEY_EXCHANGE,SSL_R_UNABLE_TO_FIND_DH_PARAMETERS);
				goto err;
				}
			
			/* generate a new random key */
			if ((dh_clnt=DHparams_dup(dh_srvr)) == NULL)
				{
				SSLerr(SSL_F_DTLS1_SEND_CLIENT_KEY_EXCHANGE,ERR_R_DH_LIB);
				goto err;
				}
			if (!DH_generate_key(dh_clnt))
				{
				SSLerr(SSL_F_DTLS1_SEND_CLIENT_KEY_EXCHANGE,ERR_R_DH_LIB);
				goto err;
				}

			/* use the 'p' output buffer for the DH key, but
			 * make sure to clear it out afterwards */

			n=DH_compute_key(p,dh_srvr->pub_key,dh_clnt);

			if (n <= 0)
				{
				SSLerr(SSL_F_DTLS1_SEND_CLIENT_KEY_EXCHANGE,ERR_R_DH_LIB);
				goto err;
				}

			/* generate master key from the result */
			s->session->master_key_length=
				s->method->ssl3_enc->generate_master_secret(s,
					s->session->master_key,p,n);
			/* clean up */
			memset(p,0,n);

			/* send off the data */
			n=BN_num_bytes(dh_clnt->pub_key);
			s2n(n,p);
			BN_bn2bin(dh_clnt->pub_key,p);
			n+=2;

			DH_free(dh_clnt);

			/* perhaps clean things up a bit EAY EAY EAY EAY*/
			}
#endif
		else
			{
			ssl3_send_alert(s,SSL3_AL_FATAL,SSL_AD_HANDSHAKE_FAILURE);
			SSLerr(SSL_F_DTLS1_SEND_CLIENT_KEY_EXCHANGE,ERR_R_INTERNAL_ERROR);
			goto err;
			}
		
		d = dtls1_set_message_header(s, d,
		SSL3_MT_CLIENT_KEY_EXCHANGE, n, 0, n);
		/*
		 *(d++)=SSL3_MT_CLIENT_KEY_EXCHANGE;
		 l2n3(n,d);
		 l2n(s->d1->handshake_write_seq,d);
		 s->d1->handshake_write_seq++;
		*/
		
		s->state=SSL3_ST_CW_KEY_EXCH_B;
		/* number of bytes to write */
		s->init_num=n+DTLS1_HM_HEADER_LENGTH;
		s->init_off=0;

		/* buffer the message to handle re-xmits */
		dtls1_buffer_message(s, 0);
		}
	
	/* SSL3_ST_CW_KEY_EXCH_B */
	return(dtls1_do_write(s,SSL3_RT_HANDSHAKE));
err:
	return(-1);
	}

int dtls1_send_client_verify(SSL *s)
	{
	unsigned char *p,*d;
	unsigned char data[MD5_DIGEST_LENGTH+SHA_DIGEST_LENGTH];
	EVP_PKEY *pkey;
#ifndef OPENSSL_NO_RSA
	unsigned u=0;
#endif
	unsigned long n;
#ifndef OPENSSL_NO_DSA
	int j;
#endif

	if (s->state == SSL3_ST_CW_CERT_VRFY_A)
		{
		d=(unsigned char *)s->init_buf->data;
		p= &(d[DTLS1_HM_HEADER_LENGTH]);
		pkey=s->cert->key->privatekey;

		s->method->ssl3_enc->cert_verify_mac(s,&(s->s3->finish_dgst2),
			&(data[MD5_DIGEST_LENGTH]));

#ifndef OPENSSL_NO_RSA
		if (pkey->type == EVP_PKEY_RSA)
			{
			s->method->ssl3_enc->cert_verify_mac(s,
				&(s->s3->finish_dgst1),&(data[0]));
			if (RSA_sign(NID_md5_sha1, data,
					 MD5_DIGEST_LENGTH+SHA_DIGEST_LENGTH,
					&(p[2]), &u, pkey->pkey.rsa) <= 0 )
				{
				SSLerr(SSL_F_DTLS1_SEND_CLIENT_VERIFY,ERR_R_RSA_LIB);
				goto err;
				}
			s2n(u,p);
			n=u+2;
			}
		else
#endif
#ifndef OPENSSL_NO_DSA
			if (pkey->type == EVP_PKEY_DSA)
			{
			if (!DSA_sign(pkey->save_type,
				&(data[MD5_DIGEST_LENGTH]),
				SHA_DIGEST_LENGTH,&(p[2]),
				(unsigned int *)&j,pkey->pkey.dsa))
				{
				SSLerr(SSL_F_DTLS1_SEND_CLIENT_VERIFY,ERR_R_DSA_LIB);
				goto err;
				}
			s2n(j,p);
			n=j+2;
			}
		else
#endif
			{
			SSLerr(SSL_F_DTLS1_SEND_CLIENT_VERIFY,ERR_R_INTERNAL_ERROR);
			goto err;
			}

		d = dtls1_set_message_header(s, d,
			SSL3_MT_CERTIFICATE_VERIFY, n, 0, n) ;

		s->init_num=(int)n+DTLS1_HM_HEADER_LENGTH;
		s->init_off=0;

		/* buffer the message to handle re-xmits */
		dtls1_buffer_message(s, 0);

		s->state = SSL3_ST_CW_CERT_VRFY_B;
		}

	/* s->state = SSL3_ST_CW_CERT_VRFY_B */
	return(dtls1_do_write(s,SSL3_RT_HANDSHAKE));
err:
	return(-1);
	}

int dtls1_send_client_certificate(SSL *s)
	{
	X509 *x509=NULL;
	EVP_PKEY *pkey=NULL;
	int i;
	unsigned long l;

	if (s->state ==	SSL3_ST_CW_CERT_A)
		{
		if ((s->cert == NULL) ||
			(s->cert->key->x509 == NULL) ||
			(s->cert->key->privatekey == NULL))
			s->state=SSL3_ST_CW_CERT_B;
		else
			s->state=SSL3_ST_CW_CERT_C;
		}

	/* We need to get a client cert */
	if (s->state == SSL3_ST_CW_CERT_B)
		{
		/* If we get an error, we need to
		 * ssl->rwstate=SSL_X509_LOOKUP; return(-1);
		 * We then get retied later */
		i=0;
		if (s->ctx->client_cert_cb != NULL)
			i=s->ctx->client_cert_cb(s,&(x509),&(pkey));
		if (i < 0)
			{
			s->rwstate=SSL_X509_LOOKUP;
			return(-1);
			}
		s->rwstate=SSL_NOTHING;
		if ((i == 1) && (pkey != NULL) && (x509 != NULL))
			{
			s->state=SSL3_ST_CW_CERT_B;
			if (	!SSL_use_certificate(s,x509) ||
				!SSL_use_PrivateKey(s,pkey))
				i=0;
			}
		else if (i == 1)
			{
			i=0;
			SSLerr(SSL_F_DTLS1_SEND_CLIENT_CERTIFICATE,SSL_R_BAD_DATA_RETURNED_BY_CALLBACK);
			}

		if (x509 != NULL) X509_free(x509);
		if (pkey != NULL) EVP_PKEY_free(pkey);
		if (i == 0)
			{
			if (s->version == SSL3_VERSION)
				{
				s->s3->tmp.cert_req=0;
				ssl3_send_alert(s,SSL3_AL_WARNING,SSL_AD_NO_CERTIFICATE);
				return(1);
				}
			else
				{
				s->s3->tmp.cert_req=2;
				}
			}

		/* Ok, we have a cert */
		s->state=SSL3_ST_CW_CERT_C;
		}

	if (s->state == SSL3_ST_CW_CERT_C)
		{
		s->state=SSL3_ST_CW_CERT_D;
		l=dtls1_output_cert_chain(s,
			(s->s3->tmp.cert_req == 2)?NULL:s->cert->key->x509);
		s->init_num=(int)l;
		s->init_off=0;

		/* set header called by dtls1_output_cert_chain() */

		/* buffer the message to handle re-xmits */
		dtls1_buffer_message(s, 0);
		}
	/* SSL3_ST_CW_CERT_D */
	return(dtls1_do_write(s,SSL3_RT_HANDSHAKE));
	}


