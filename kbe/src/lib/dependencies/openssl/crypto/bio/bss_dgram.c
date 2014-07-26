/* crypto/bio/bio_dgram.c */
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

#ifndef OPENSSL_NO_DGRAM

#include <stdio.h>
#include <errno.h>
#define USE_SOCKETS
#include "cryptlib.h"

#include <openssl/bio.h>

#define IP_MTU      14 /* linux is lame */

#ifdef WATT32
#define sock_write SockWrite  /* Watt-32 uses same names */
#define sock_read  SockRead
#define sock_puts  SockPuts
#endif

static int dgram_write(BIO *h, const char *buf, int num);
static int dgram_read(BIO *h, char *buf, int size);
static int dgram_puts(BIO *h, const char *str);
static long dgram_ctrl(BIO *h, int cmd, long arg1, void *arg2);
static int dgram_new(BIO *h);
static int dgram_free(BIO *data);
static int dgram_clear(BIO *bio);

int BIO_dgram_should_retry(int s);

static BIO_METHOD methods_dgramp=
	{
	BIO_TYPE_DGRAM,
	"datagram socket",
	dgram_write,
	dgram_read,
	dgram_puts,
	NULL, /* dgram_gets, */
	dgram_ctrl,
	dgram_new,
	dgram_free,
	NULL,
	};

typedef struct bio_dgram_data_st
	{
	struct sockaddr peer;
	unsigned int connected;
	unsigned int _errno;
	unsigned int mtu;
	} bio_dgram_data;

BIO_METHOD *BIO_s_datagram(void)
	{
	return(&methods_dgramp);
	}

BIO *BIO_new_dgram(int fd, int close_flag)
	{
	BIO *ret;

	ret=BIO_new(BIO_s_datagram());
	if (ret == NULL) return(NULL);
	BIO_set_fd(ret,fd,close_flag);
	return(ret);
	}

static int dgram_new(BIO *bi)
	{
	bio_dgram_data *data = NULL;

	bi->init=0;
	bi->num=0;
	data = OPENSSL_malloc(sizeof(bio_dgram_data));
	if (data == NULL)
		return 0;
	memset(data, 0x00, sizeof(bio_dgram_data));
    bi->ptr = data;

	bi->flags=0;
	return(1);
	}

static int dgram_free(BIO *a)
	{
	bio_dgram_data *data;

	if (a == NULL) return(0);
	if ( ! dgram_clear(a))
		return 0;

	data = (bio_dgram_data *)a->ptr;
	if(data != NULL) OPENSSL_free(data);

	return(1);
	}

static int dgram_clear(BIO *a)
	{
	if (a == NULL) return(0);
	if (a->shutdown)
		{
		if (a->init)
			{
			SHUTDOWN2(a->num);
			}
		a->init=0;
		a->flags=0;
		}
	return(1);
	}
	
static int dgram_read(BIO *b, char *out, int outl)
	{
	int ret=0;
	bio_dgram_data *data = (bio_dgram_data *)b->ptr;

	struct sockaddr peer;
	int peerlen = sizeof(peer);

	if (out != NULL)
		{
		clear_socket_error();
		memset(&peer, 0x00, peerlen);
		/* Last arg in recvfrom is signed on some platforms and
		 * unsigned on others. It is of type socklen_t on some
		 * but this is not universal. Cast to (void *) to avoid
		 * compiler warnings.
		 */
		ret=recvfrom(b->num,out,outl,0,&peer,(void *)&peerlen);

		if ( ! data->connected  && ret > 0)
			BIO_ctrl(b, BIO_CTRL_DGRAM_CONNECT, 0, &peer);

		BIO_clear_retry_flags(b);
		if (ret <= 0)
			{
			if (BIO_dgram_should_retry(ret))
				{
				BIO_set_retry_read(b);
				data->_errno = get_last_socket_error();
				}
			}
		}
	return(ret);
	}

static int dgram_write(BIO *b, const char *in, int inl)
	{
	int ret;
	bio_dgram_data *data = (bio_dgram_data *)b->ptr;
	clear_socket_error();

    if ( data->connected )
        ret=send(b->num,in,inl,0);
    else
        ret=sendto(b->num, in, inl, 0, &data->peer, sizeof(data->peer));

	BIO_clear_retry_flags(b);
	if (ret <= 0)
		{
		if (BIO_sock_should_retry(ret))
			{
			BIO_set_retry_write(b);  
			data->_errno = get_last_socket_error();

#if 0 /* higher layers are responsible for querying MTU, if necessary */
			if ( data->_errno == EMSGSIZE)
				/* retrieve the new MTU */
				BIO_ctrl(b, BIO_CTRL_DGRAM_QUERY_MTU, 0, NULL);
#endif
			}
		}
	return(ret);
	}

static long dgram_ctrl(BIO *b, int cmd, long num, void *ptr)
	{
	long ret=1;
	int *ip;
	struct sockaddr *to = NULL;
	bio_dgram_data *data = NULL;
	long sockopt_val = 0;
	unsigned int sockopt_len = 0;

	data = (bio_dgram_data *)b->ptr;

	switch (cmd)
		{
	case BIO_CTRL_RESET:
		num=0;
	case BIO_C_FILE_SEEK:
		ret=0;
		break;
	case BIO_C_FILE_TELL:
	case BIO_CTRL_INFO:
		ret=0;
		break;
	case BIO_C_SET_FD:
		dgram_clear(b);
		b->num= *((int *)ptr);
		b->shutdown=(int)num;
		b->init=1;
		break;
	case BIO_C_GET_FD:
		if (b->init)
			{
			ip=(int *)ptr;
			if (ip != NULL) *ip=b->num;
			ret=b->num;
			}
		else
			ret= -1;
		break;
	case BIO_CTRL_GET_CLOSE:
		ret=b->shutdown;
		break;
	case BIO_CTRL_SET_CLOSE:
		b->shutdown=(int)num;
		break;
	case BIO_CTRL_PENDING:
	case BIO_CTRL_WPENDING:
		ret=0;
		break;
	case BIO_CTRL_DUP:
	case BIO_CTRL_FLUSH:
		ret=1;
		break;
	case BIO_CTRL_DGRAM_CONNECT:
		to = (struct sockaddr *)ptr;
#if 0
		if (connect(b->num, to, sizeof(struct sockaddr)) < 0)
			{ perror("connect"); ret = 0; }
		else
			{
#endif
			memcpy(&(data->peer),to, sizeof(struct sockaddr));
#if 0
			}
#endif
		break;
		/* (Linux)kernel sets DF bit on outgoing IP packets */
#ifdef IP_MTU_DISCOVER
	case BIO_CTRL_DGRAM_MTU_DISCOVER:
		sockopt_val = IP_PMTUDISC_DO;
		if ((ret = setsockopt(b->num, IPPROTO_IP, IP_MTU_DISCOVER,
			&sockopt_val, sizeof(sockopt_val))) < 0)
			perror("setsockopt");
		break;
#endif
	case BIO_CTRL_DGRAM_QUERY_MTU:
         sockopt_len = sizeof(sockopt_val);
		if ((ret = getsockopt(b->num, IPPROTO_IP, IP_MTU, (void *)&sockopt_val,
			&sockopt_len)) < 0 || sockopt_val < 0)
			{ ret = 0; }
		else
			{
			data->mtu = sockopt_val;
			ret = data->mtu;
			}
		break;
	case BIO_CTRL_DGRAM_GET_MTU:
		return data->mtu;
		break;
	case BIO_CTRL_DGRAM_SET_MTU:
		data->mtu = num;
		ret = num;
		break;
	case BIO_CTRL_DGRAM_SET_CONNECTED:
		to = (struct sockaddr *)ptr;

		if ( to != NULL)
			{
			data->connected = 1;
			memcpy(&(data->peer),to, sizeof(struct sockaddr));
			}
		else
			{
			data->connected = 0;
			memset(&(data->peer), 0x00, sizeof(struct sockaddr));
			}
		break;
    case BIO_CTRL_DGRAM_SET_PEER:
        to = (struct sockaddr *) ptr;

        memcpy(&(data->peer), to, sizeof(struct sockaddr));
        break;
	case BIO_CTRL_DGRAM_SET_RECV_TIMEOUT:
		if ( setsockopt(b->num, SOL_SOCKET, SO_RCVTIMEO, ptr,
			sizeof(struct timeval)) < 0)
			{ perror("setsockopt");	ret = -1; }
		break;
	case BIO_CTRL_DGRAM_GET_RECV_TIMEOUT:
		if ( getsockopt(b->num, SOL_SOCKET, SO_RCVTIMEO, 
			ptr, (void *)&ret) < 0)
			{ perror("getsockopt"); ret = -1; }
		break;
	case BIO_CTRL_DGRAM_SET_SEND_TIMEOUT:
		if ( setsockopt(b->num, SOL_SOCKET, SO_SNDTIMEO, ptr,
			sizeof(struct timeval)) < 0)
			{ perror("setsockopt");	ret = -1; }
		break;
	case BIO_CTRL_DGRAM_GET_SEND_TIMEOUT:
		if ( getsockopt(b->num, SOL_SOCKET, SO_SNDTIMEO, 
			ptr, (void *)&ret) < 0)
			{ perror("getsockopt"); ret = -1; }
		break;
	case BIO_CTRL_DGRAM_GET_SEND_TIMER_EXP:
		/* fall-through */
	case BIO_CTRL_DGRAM_GET_RECV_TIMER_EXP:
		if ( data->_errno == EAGAIN)
			{
			ret = 1;
			data->_errno = 0;
			}
		else
			ret = 0;
		break;
#ifdef EMSGSIZE
	case BIO_CTRL_DGRAM_MTU_EXCEEDED:
		if ( data->_errno == EMSGSIZE)
			{
			ret = 1;
			data->_errno = 0;
			}
		else
			ret = 0;
		break;
#endif
	default:
		ret=0;
		break;
		}
	return(ret);
	}

static int dgram_puts(BIO *bp, const char *str)
	{
	int n,ret;

	n=strlen(str);
	ret=dgram_write(bp,str,n);
	return(ret);
	}

int BIO_dgram_should_retry(int i)
	{
	int err;

	if ((i == 0) || (i == -1))
		{
		err=get_last_socket_error();

#if defined(OPENSSL_SYS_WINDOWS) && 0 /* more microsoft stupidity? perhaps not? Ben 4/1/99 */
		if ((i == -1) && (err == 0))
			return(1);
#endif

		return(BIO_dgram_non_fatal_error(err));
		}
	return(0);
	}

int BIO_dgram_non_fatal_error(int err)
	{
	switch (err)
		{
#if defined(OPENSSL_SYS_WINDOWS)
# if defined(WSAEWOULDBLOCK)
	case WSAEWOULDBLOCK:
# endif

# if 0 /* This appears to always be an error */
#  if defined(WSAENOTCONN)
	case WSAENOTCONN:
#  endif
# endif
#endif

#ifdef EWOULDBLOCK
# ifdef WSAEWOULDBLOCK
#  if WSAEWOULDBLOCK != EWOULDBLOCK
	case EWOULDBLOCK:
#  endif
# else
	case EWOULDBLOCK:
# endif
#endif

#if defined(ENOTCONN)
	case ENOTCONN:
#endif

#ifdef EINTR
	case EINTR:
#endif

#ifdef EAGAIN
#if EWOULDBLOCK != EAGAIN
	case EAGAIN:
# endif
#endif

#ifdef EPROTO
	case EPROTO:
#endif

#ifdef EINPROGRESS
	case EINPROGRESS:
#endif

#ifdef EALREADY
	case EALREADY:
#endif

/* DF bit set, and packet larger than MTU */
#ifdef EMSGSIZE
	case EMSGSIZE:
#endif

		return(1);
		/* break; */
	default:
		break;
		}
	return(0);
	}
#endif
