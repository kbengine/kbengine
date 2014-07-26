/* crypto/rc4/rc4_skey.c */
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

#include <openssl/rc4.h>
#include "rc4_locl.h"
#include <openssl/opensslv.h>

const char RC4_version[]="RC4" OPENSSL_VERSION_PTEXT;

const char *RC4_options(void)
	{
#ifdef RC4_INDEX
	if (sizeof(RC4_INT) == 1)
		return("rc4(idx,char)");
	else
		return("rc4(idx,int)");
#else
	if (sizeof(RC4_INT) == 1)
		return("rc4(ptr,char)");
	else
		return("rc4(ptr,int)");
#endif
	}

/* RC4 as implemented from a posting from
 * Newsgroups: sci.crypt
 * From: sterndark@netcom.com (David Sterndark)
 * Subject: RC4 Algorithm revealed.
 * Message-ID: <sternCvKL4B.Hyy@netcom.com>
 * Date: Wed, 14 Sep 1994 06:35:31 GMT
 */

void RC4_set_key(RC4_KEY *key, int len, const unsigned char *data)
	{
        register RC4_INT tmp;
        register int id1,id2;
        register RC4_INT *d;
        unsigned int i;
        
        d= &(key->data[0]);
        key->x = 0;     
        key->y = 0;     
        id1=id2=0;     

#define SK_LOOP(d,n) { \
		tmp=d[(n)]; \
		id2 = (data[id1] + tmp + id2) & 0xff; \
		if (++id1 == len) id1=0; \
		d[(n)]=d[id2]; \
		d[id2]=tmp; }

#if defined(OPENSSL_CPUID_OBJ) && !defined(OPENSSL_NO_ASM)
# if	defined(__i386)   || defined(__i386__)   || defined(_M_IX86) || \
	defined(__INTEL__) || \
	defined(__x86_64) || defined(__x86_64__) || defined(_M_AMD64)
	if (sizeof(RC4_INT) > 1) {
		/*
		 * Unlike all other x86 [and x86_64] implementations,
		 * Intel P4 core [including EM64T] was found to perform
		 * poorly with wider RC4_INT. Performance improvement
		 * for IA-32 hand-coded assembler turned out to be 2.8x
		 * if re-coded for RC4_CHAR! It's however inappropriate
		 * to just switch to RC4_CHAR for x86[_64], as non-P4
		 * implementations suffer from significant performance
		 * losses then, e.g. PIII exhibits >2x deterioration,
		 * and so does Opteron. In order to assure optimal
		 * all-round performance, let us [try to] detect P4 at
		 * run-time by checking upon HTT bit in CPU capability
		 * vector and set up compressed key schedule, which is
		 * recognized by correspondingly updated assembler
		 * module...
		 *				<appro@fy.chalmers.se>
		 */
		if (OPENSSL_ia32cap_P & (1<<28)) {
			unsigned char *cp=(unsigned char *)d;

			for (i=0;i<256;i++) cp[i]=i;
			for (i=0;i<256;i++) SK_LOOP(cp,i);
			/* mark schedule as compressed! */
			d[256/sizeof(RC4_INT)]=-1;
			return;
		}
	}
# endif
#endif
	for (i=0; i < 256; i++) d[i]=i;
	for (i=0; i < 256; i+=4)
		{
		SK_LOOP(d,i+0);
		SK_LOOP(d,i+1);
		SK_LOOP(d,i+2);
		SK_LOOP(d,i+3);
		}
	}
    
