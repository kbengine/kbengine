/* crypto/rand/randfile.c */
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

/* We need to define this to get macros like S_IFBLK and S_IFCHR */
#define _XOPEN_SOURCE 500

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "e_os.h"
#include <openssl/crypto.h>
#include <openssl/rand.h>
#include <openssl/buffer.h>

#ifdef OPENSSL_SYS_VMS
#include <unixio.h>
#endif
#ifndef NO_SYS_TYPES_H
# include <sys/types.h>
#endif
#ifdef MAC_OS_pre_X
# include <stat.h>
#else
# include <sys/stat.h>
#endif

#undef BUFSIZE
#define BUFSIZE	1024
#define RAND_DATA 1024

/* #define RFILE ".rnd" - defined in ../../e_os.h */

/* Note that these functions are intended for seed files only.
 * Entropy devices and EGD sockets are handled in rand_unix.c */

int RAND_load_file(const char *file, long bytes)
	{
	/* If bytes >= 0, read up to 'bytes' bytes.
	 * if bytes == -1, read complete file. */

	MS_STATIC unsigned char buf[BUFSIZE];
	struct stat sb;
	int i,ret=0,n;
	FILE *in;

	if (file == NULL) return(0);

	if (stat(file,&sb) < 0) return(0);
	RAND_add(&sb,sizeof(sb),0.0);
	if (bytes == 0) return(ret);

	in=fopen(file,"rb");
	if (in == NULL) goto err;
#if defined(S_IFBLK) && defined(S_IFCHR)
	if (sb.st_mode & (S_IFBLK | S_IFCHR)) {
	  /* this file is a device. we don't want read an infinite number
	   * of bytes from a random device, nor do we want to use buffered
	   * I/O because we will waste system entropy. 
	   */
	  bytes = (bytes == -1) ? 2048 : bytes; /* ok, is 2048 enough? */
	  setvbuf(in, NULL, _IONBF, 0); /* don't do buffered reads */
	}
#endif
	for (;;)
		{
		if (bytes > 0)
			n = (bytes < BUFSIZE)?(int)bytes:BUFSIZE;
		else
			n = BUFSIZE;
		i=fread(buf,1,n,in);
		if (i <= 0) break;
		/* even if n != i, use the full array */
		RAND_add(buf,n,(double)i);
		ret+=i;
		if (bytes > 0)
			{
			bytes-=n;
			if (bytes <= 0) break;
			}
		}
	fclose(in);
	OPENSSL_cleanse(buf,BUFSIZE);
err:
	return(ret);
	}

int RAND_write_file(const char *file)
	{
	unsigned char buf[BUFSIZE];
	int i,ret=0,rand_err=0;
	FILE *out = NULL;
	int n;
	struct stat sb;
	
	i=stat(file,&sb);
	if (i != -1) { 
#if defined(S_IFBLK) && defined(S_IFCHR)
	  if (sb.st_mode & (S_IFBLK | S_IFCHR)) {
	    /* this file is a device. we don't write back to it. 
	     * we "succeed" on the assumption this is some sort 
	     * of random device. Otherwise attempting to write to 
	     * and chmod the device causes problems.
	     */
	    return(1); 
	  }
#endif
	}

#if defined(O_CREAT) && !defined(OPENSSL_SYS_WIN32)
	{
	/* For some reason Win32 can't write to files created this way */
	
	/* chmod(..., 0600) is too late to protect the file,
	 * permissions should be restrictive from the start */
	int fd = open(file, O_CREAT, 0600);
	if (fd != -1)
		out = fdopen(fd, "wb");
	}
#endif
	if (out == NULL)
		out = fopen(file,"wb");
	if (out == NULL) goto err;

#ifndef NO_CHMOD
	chmod(file,0600);
#endif
	n=RAND_DATA;
	for (;;)
		{
		i=(n > BUFSIZE)?BUFSIZE:n;
		n-=BUFSIZE;
		if (RAND_bytes(buf,i) <= 0)
			rand_err=1;
		i=fwrite(buf,1,i,out);
		if (i <= 0)
			{
			ret=0;
			break;
			}
		ret+=i;
		if (n <= 0) break;
                }
#ifdef OPENSSL_SYS_VMS
	/* Try to delete older versions of the file, until there aren't
	   any */
	{
	char *tmpf;

	tmpf = OPENSSL_malloc(strlen(file) + 4);  /* to add ";-1" and a nul */
	if (tmpf)
		{
		strcpy(tmpf, file);
		strcat(tmpf, ";-1");
		while(delete(tmpf) == 0)
			;
		rename(file,";1"); /* Make sure it's version 1, or we
				      will reach the limit (32767) at
				      some point... */
		}
	}
#endif /* OPENSSL_SYS_VMS */

	fclose(out);
	OPENSSL_cleanse(buf,BUFSIZE);
err:
	return (rand_err ? -1 : ret);
	}

const char *RAND_file_name(char *buf, size_t size)
	{
	char *s=NULL;
	int ok = 0;
#ifdef __OpenBSD__
	struct stat sb;
#endif

	if (OPENSSL_issetugid() == 0)
		s=getenv("RANDFILE");
	if (s != NULL && *s && strlen(s) + 1 < size)
		{
		if (BUF_strlcpy(buf,s,size) >= size)
			return NULL;
		}
	else
		{
		if (OPENSSL_issetugid() == 0)
			s=getenv("HOME");
#ifdef DEFAULT_HOME
		if (s == NULL)
			{
			s = DEFAULT_HOME;
			}
#endif
		if (s && *s && strlen(s)+strlen(RFILE)+2 < size)
			{
			BUF_strlcpy(buf,s,size);
#ifndef OPENSSL_SYS_VMS
			BUF_strlcat(buf,"/",size);
#endif
			BUF_strlcat(buf,RFILE,size);
			ok = 1;
			}
		else
		  	buf[0] = '\0'; /* no file name */
		}

#ifdef __OpenBSD__
	/* given that all random loads just fail if the file can't be 
	 * seen on a stat, we stat the file we're returning, if it
	 * fails, use /dev/arandom instead. this allows the user to 
	 * use their own source for good random data, but defaults
	 * to something hopefully decent if that isn't available. 
	 */

	if (!ok)
		if (BUF_strlcpy(buf,"/dev/arandom",size) >= size) {
			return(NULL);
		}	
	if (stat(buf,&sb) == -1)
		if (BUF_strlcpy(buf,"/dev/arandom",size) >= size) {
			return(NULL);
		}	

#endif
	return(buf);
	}
