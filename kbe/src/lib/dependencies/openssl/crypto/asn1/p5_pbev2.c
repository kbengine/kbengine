/* p5_pbev2.c */
/* Written by Dr Stephen N Henson (shenson@bigfoot.com) for the OpenSSL
 * project 1999-2004.
 */
/* ====================================================================
 * Copyright (c) 1999 The OpenSSL Project.  All rights reserved.
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
 *    licensing@OpenSSL.org.
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

#include <stdio.h>
#include "cryptlib.h"
#include <openssl/asn1t.h>
#include <openssl/x509.h>
#include <openssl/rand.h>

/* PKCS#5 v2.0 password based encryption structures */

ASN1_SEQUENCE(PBE2PARAM) = {
	ASN1_SIMPLE(PBE2PARAM, keyfunc, X509_ALGOR),
	ASN1_SIMPLE(PBE2PARAM, encryption, X509_ALGOR)
} ASN1_SEQUENCE_END(PBE2PARAM)

IMPLEMENT_ASN1_FUNCTIONS(PBE2PARAM)

ASN1_SEQUENCE(PBKDF2PARAM) = {
	ASN1_SIMPLE(PBKDF2PARAM, salt, ASN1_ANY),
	ASN1_SIMPLE(PBKDF2PARAM, iter, ASN1_INTEGER),
	ASN1_OPT(PBKDF2PARAM, keylength, ASN1_INTEGER),
	ASN1_OPT(PBKDF2PARAM, prf, X509_ALGOR)
} ASN1_SEQUENCE_END(PBKDF2PARAM)

IMPLEMENT_ASN1_FUNCTIONS(PBKDF2PARAM)

/* Return an algorithm identifier for a PKCS#5 v2.0 PBE algorithm:
 * yes I know this is horrible!
 */

X509_ALGOR *PKCS5_pbe2_set(const EVP_CIPHER *cipher, int iter,
				 unsigned char *salt, int saltlen)
{
	X509_ALGOR *scheme = NULL, *kalg = NULL, *ret = NULL;
	int alg_nid;
	EVP_CIPHER_CTX ctx;
	unsigned char iv[EVP_MAX_IV_LENGTH];
	PBKDF2PARAM *kdf = NULL;
	PBE2PARAM *pbe2 = NULL;
	ASN1_OCTET_STRING *osalt = NULL;
	ASN1_OBJECT *obj;

	alg_nid = EVP_CIPHER_type(cipher);
	if(alg_nid == NID_undef) {
		ASN1err(ASN1_F_PKCS5_PBE2_SET,
				ASN1_R_CIPHER_HAS_NO_OBJECT_IDENTIFIER);
		goto err;
	}
	obj = OBJ_nid2obj(alg_nid);

	if(!(pbe2 = PBE2PARAM_new())) goto merr;

	/* Setup the AlgorithmIdentifier for the encryption scheme */
	scheme = pbe2->encryption;

	scheme->algorithm = obj;
	if(!(scheme->parameter = ASN1_TYPE_new())) goto merr;

	/* Create random IV */
	if (EVP_CIPHER_iv_length(cipher) &&
		RAND_pseudo_bytes(iv, EVP_CIPHER_iv_length(cipher)) < 0)
  		goto err;

	EVP_CIPHER_CTX_init(&ctx);

	/* Dummy cipherinit to just setup the IV */
	EVP_CipherInit_ex(&ctx, cipher, NULL, NULL, iv, 0);
	if(EVP_CIPHER_param_to_asn1(&ctx, scheme->parameter) < 0) {
		ASN1err(ASN1_F_PKCS5_PBE2_SET,
					ASN1_R_ERROR_SETTING_CIPHER_PARAMS);
		EVP_CIPHER_CTX_cleanup(&ctx);
		goto err;
	}
	EVP_CIPHER_CTX_cleanup(&ctx);

	if(!(kdf = PBKDF2PARAM_new())) goto merr;
	if(!(osalt = M_ASN1_OCTET_STRING_new())) goto merr;

	if (!saltlen) saltlen = PKCS5_SALT_LEN;
	if (!(osalt->data = OPENSSL_malloc (saltlen))) goto merr;
	osalt->length = saltlen;
	if (salt) memcpy (osalt->data, salt, saltlen);
	else if (RAND_pseudo_bytes (osalt->data, saltlen) < 0) goto merr;

	if(iter <= 0) iter = PKCS5_DEFAULT_ITER;
	if(!ASN1_INTEGER_set(kdf->iter, iter)) goto merr;

	/* Now include salt in kdf structure */
	kdf->salt->value.octet_string = osalt;
	kdf->salt->type = V_ASN1_OCTET_STRING;
	osalt = NULL;

	/* If its RC2 then we'd better setup the key length */

	if(alg_nid == NID_rc2_cbc) {
		if(!(kdf->keylength = M_ASN1_INTEGER_new())) goto merr;
		if(!ASN1_INTEGER_set (kdf->keylength,
				 EVP_CIPHER_key_length(cipher))) goto merr;
	}

	/* prf can stay NULL because we are using hmacWithSHA1 */

	/* Now setup the PBE2PARAM keyfunc structure */

	pbe2->keyfunc->algorithm = OBJ_nid2obj(NID_id_pbkdf2);

	/* Encode PBKDF2PARAM into parameter of pbe2 */

	if(!(pbe2->keyfunc->parameter = ASN1_TYPE_new())) goto merr;

	if(!ASN1_pack_string_of(PBKDF2PARAM, kdf, i2d_PBKDF2PARAM,
			 &pbe2->keyfunc->parameter->value.sequence)) goto merr;
	pbe2->keyfunc->parameter->type = V_ASN1_SEQUENCE;

	PBKDF2PARAM_free(kdf);
	kdf = NULL;

	/* Now set up top level AlgorithmIdentifier */

	if(!(ret = X509_ALGOR_new())) goto merr;
	if(!(ret->parameter = ASN1_TYPE_new())) goto merr;

	ret->algorithm = OBJ_nid2obj(NID_pbes2);

	/* Encode PBE2PARAM into parameter */

	if(!ASN1_pack_string_of(PBE2PARAM, pbe2, i2d_PBE2PARAM,
				 &ret->parameter->value.sequence)) goto merr;
	ret->parameter->type = V_ASN1_SEQUENCE;

	PBE2PARAM_free(pbe2);
	pbe2 = NULL;

	return ret;

	merr:
	ASN1err(ASN1_F_PKCS5_PBE2_SET,ERR_R_MALLOC_FAILURE);

	err:
	PBE2PARAM_free(pbe2);
	/* Note 'scheme' is freed as part of pbe2 */
	M_ASN1_OCTET_STRING_free(osalt);
	PBKDF2PARAM_free(kdf);
	X509_ALGOR_free(kalg);
	X509_ALGOR_free(ret);

	return NULL;

}
