/* crypto/objects/obj_dat.h */

/* THIS FILE IS GENERATED FROM objects.h by obj_dat.pl via the
 * following command:
 * perl obj_dat.pl obj_mac.h obj_dat.h
 */

/* Copyright (C) 1995-1997 Eric Young (eay@cryptsoft.com)
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

#define NUM_NID 780
#define NUM_SN 773
#define NUM_LN 773
#define NUM_OBJ 729

static unsigned char lvalues[5154]={
0x00,                                        /* [  0] OBJ_undef */
0x2A,0x86,0x48,0x86,0xF7,0x0D,               /* [  1] OBJ_rsadsi */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,          /* [  7] OBJ_pkcs */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x02,0x02,     /* [ 14] OBJ_md2 */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x02,0x05,     /* [ 22] OBJ_md5 */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x03,0x04,     /* [ 30] OBJ_rc4 */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x01,0x01,/* [ 38] OBJ_rsaEncryption */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x01,0x02,/* [ 47] OBJ_md2WithRSAEncryption */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x01,0x04,/* [ 56] OBJ_md5WithRSAEncryption */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x05,0x01,/* [ 65] OBJ_pbeWithMD2AndDES_CBC */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x05,0x03,/* [ 74] OBJ_pbeWithMD5AndDES_CBC */
0x55,                                        /* [ 83] OBJ_X500 */
0x55,0x04,                                   /* [ 84] OBJ_X509 */
0x55,0x04,0x03,                              /* [ 86] OBJ_commonName */
0x55,0x04,0x06,                              /* [ 89] OBJ_countryName */
0x55,0x04,0x07,                              /* [ 92] OBJ_localityName */
0x55,0x04,0x08,                              /* [ 95] OBJ_stateOrProvinceName */
0x55,0x04,0x0A,                              /* [ 98] OBJ_organizationName */
0x55,0x04,0x0B,                              /* [101] OBJ_organizationalUnitName */
0x55,0x08,0x01,0x01,                         /* [104] OBJ_rsa */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x07,     /* [108] OBJ_pkcs7 */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x07,0x01,/* [116] OBJ_pkcs7_data */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x07,0x02,/* [125] OBJ_pkcs7_signed */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x07,0x03,/* [134] OBJ_pkcs7_enveloped */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x07,0x04,/* [143] OBJ_pkcs7_signedAndEnveloped */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x07,0x05,/* [152] OBJ_pkcs7_digest */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x07,0x06,/* [161] OBJ_pkcs7_encrypted */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x03,     /* [170] OBJ_pkcs3 */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x03,0x01,/* [178] OBJ_dhKeyAgreement */
0x2B,0x0E,0x03,0x02,0x06,                    /* [187] OBJ_des_ecb */
0x2B,0x0E,0x03,0x02,0x09,                    /* [192] OBJ_des_cfb64 */
0x2B,0x0E,0x03,0x02,0x07,                    /* [197] OBJ_des_cbc */
0x2B,0x0E,0x03,0x02,0x11,                    /* [202] OBJ_des_ede_ecb */
0x2B,0x06,0x01,0x04,0x01,0x81,0x3C,0x07,0x01,0x01,0x02,/* [207] OBJ_idea_cbc */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x03,0x02,     /* [218] OBJ_rc2_cbc */
0x2B,0x0E,0x03,0x02,0x12,                    /* [226] OBJ_sha */
0x2B,0x0E,0x03,0x02,0x0F,                    /* [231] OBJ_shaWithRSAEncryption */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x03,0x07,     /* [236] OBJ_des_ede3_cbc */
0x2B,0x0E,0x03,0x02,0x08,                    /* [244] OBJ_des_ofb64 */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,     /* [249] OBJ_pkcs9 */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,0x01,/* [257] OBJ_pkcs9_emailAddress */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,0x02,/* [266] OBJ_pkcs9_unstructuredName */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,0x03,/* [275] OBJ_pkcs9_contentType */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,0x04,/* [284] OBJ_pkcs9_messageDigest */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,0x05,/* [293] OBJ_pkcs9_signingTime */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,0x06,/* [302] OBJ_pkcs9_countersignature */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,0x07,/* [311] OBJ_pkcs9_challengePassword */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,0x08,/* [320] OBJ_pkcs9_unstructuredAddress */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,0x09,/* [329] OBJ_pkcs9_extCertAttributes */
0x60,0x86,0x48,0x01,0x86,0xF8,0x42,          /* [338] OBJ_netscape */
0x60,0x86,0x48,0x01,0x86,0xF8,0x42,0x01,     /* [345] OBJ_netscape_cert_extension */
0x60,0x86,0x48,0x01,0x86,0xF8,0x42,0x02,     /* [353] OBJ_netscape_data_type */
0x2B,0x0E,0x03,0x02,0x1A,                    /* [361] OBJ_sha1 */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x01,0x05,/* [366] OBJ_sha1WithRSAEncryption */
0x2B,0x0E,0x03,0x02,0x0D,                    /* [375] OBJ_dsaWithSHA */
0x2B,0x0E,0x03,0x02,0x0C,                    /* [380] OBJ_dsa_2 */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x05,0x0B,/* [385] OBJ_pbeWithSHA1AndRC2_CBC */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x05,0x0C,/* [394] OBJ_id_pbkdf2 */
0x2B,0x0E,0x03,0x02,0x1B,                    /* [403] OBJ_dsaWithSHA1_2 */
0x60,0x86,0x48,0x01,0x86,0xF8,0x42,0x01,0x01,/* [408] OBJ_netscape_cert_type */
0x60,0x86,0x48,0x01,0x86,0xF8,0x42,0x01,0x02,/* [417] OBJ_netscape_base_url */
0x60,0x86,0x48,0x01,0x86,0xF8,0x42,0x01,0x03,/* [426] OBJ_netscape_revocation_url */
0x60,0x86,0x48,0x01,0x86,0xF8,0x42,0x01,0x04,/* [435] OBJ_netscape_ca_revocation_url */
0x60,0x86,0x48,0x01,0x86,0xF8,0x42,0x01,0x07,/* [444] OBJ_netscape_renewal_url */
0x60,0x86,0x48,0x01,0x86,0xF8,0x42,0x01,0x08,/* [453] OBJ_netscape_ca_policy_url */
0x60,0x86,0x48,0x01,0x86,0xF8,0x42,0x01,0x0C,/* [462] OBJ_netscape_ssl_server_name */
0x60,0x86,0x48,0x01,0x86,0xF8,0x42,0x01,0x0D,/* [471] OBJ_netscape_comment */
0x60,0x86,0x48,0x01,0x86,0xF8,0x42,0x02,0x05,/* [480] OBJ_netscape_cert_sequence */
0x55,0x1D,                                   /* [489] OBJ_id_ce */
0x55,0x1D,0x0E,                              /* [491] OBJ_subject_key_identifier */
0x55,0x1D,0x0F,                              /* [494] OBJ_key_usage */
0x55,0x1D,0x10,                              /* [497] OBJ_private_key_usage_period */
0x55,0x1D,0x11,                              /* [500] OBJ_subject_alt_name */
0x55,0x1D,0x12,                              /* [503] OBJ_issuer_alt_name */
0x55,0x1D,0x13,                              /* [506] OBJ_basic_constraints */
0x55,0x1D,0x14,                              /* [509] OBJ_crl_number */
0x55,0x1D,0x20,                              /* [512] OBJ_certificate_policies */
0x55,0x1D,0x23,                              /* [515] OBJ_authority_key_identifier */
0x2B,0x06,0x01,0x04,0x01,0x97,0x55,0x01,0x02,/* [518] OBJ_bf_cbc */
0x55,0x08,0x03,0x65,                         /* [527] OBJ_mdc2 */
0x55,0x08,0x03,0x64,                         /* [531] OBJ_mdc2WithRSA */
0x55,0x04,0x2A,                              /* [535] OBJ_givenName */
0x55,0x04,0x04,                              /* [538] OBJ_surname */
0x55,0x04,0x2B,                              /* [541] OBJ_initials */
0x55,0x1D,0x1F,                              /* [544] OBJ_crl_distribution_points */
0x2B,0x0E,0x03,0x02,0x03,                    /* [547] OBJ_md5WithRSA */
0x55,0x04,0x05,                              /* [552] OBJ_serialNumber */
0x55,0x04,0x0C,                              /* [555] OBJ_title */
0x55,0x04,0x0D,                              /* [558] OBJ_description */
0x2A,0x86,0x48,0x86,0xF6,0x7D,0x07,0x42,0x0A,/* [561] OBJ_cast5_cbc */
0x2A,0x86,0x48,0x86,0xF6,0x7D,0x07,0x42,0x0C,/* [570] OBJ_pbeWithMD5AndCast5_CBC */
0x2A,0x86,0x48,0xCE,0x38,0x04,0x03,          /* [579] OBJ_dsaWithSHA1 */
0x2B,0x0E,0x03,0x02,0x1D,                    /* [586] OBJ_sha1WithRSA */
0x2A,0x86,0x48,0xCE,0x38,0x04,0x01,          /* [591] OBJ_dsa */
0x2B,0x24,0x03,0x02,0x01,                    /* [598] OBJ_ripemd160 */
0x2B,0x24,0x03,0x03,0x01,0x02,               /* [603] OBJ_ripemd160WithRSA */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x03,0x08,     /* [609] OBJ_rc5_cbc */
0x29,0x01,0x01,0x85,0x1A,0x01,               /* [617] OBJ_rle_compression */
0x29,0x01,0x01,0x85,0x1A,0x02,               /* [623] OBJ_zlib_compression */
0x55,0x1D,0x25,                              /* [629] OBJ_ext_key_usage */
0x2B,0x06,0x01,0x05,0x05,0x07,               /* [632] OBJ_id_pkix */
0x2B,0x06,0x01,0x05,0x05,0x07,0x03,          /* [638] OBJ_id_kp */
0x2B,0x06,0x01,0x05,0x05,0x07,0x03,0x01,     /* [645] OBJ_server_auth */
0x2B,0x06,0x01,0x05,0x05,0x07,0x03,0x02,     /* [653] OBJ_client_auth */
0x2B,0x06,0x01,0x05,0x05,0x07,0x03,0x03,     /* [661] OBJ_code_sign */
0x2B,0x06,0x01,0x05,0x05,0x07,0x03,0x04,     /* [669] OBJ_email_protect */
0x2B,0x06,0x01,0x05,0x05,0x07,0x03,0x08,     /* [677] OBJ_time_stamp */
0x2B,0x06,0x01,0x04,0x01,0x82,0x37,0x02,0x01,0x15,/* [685] OBJ_ms_code_ind */
0x2B,0x06,0x01,0x04,0x01,0x82,0x37,0x02,0x01,0x16,/* [695] OBJ_ms_code_com */
0x2B,0x06,0x01,0x04,0x01,0x82,0x37,0x0A,0x03,0x01,/* [705] OBJ_ms_ctl_sign */
0x2B,0x06,0x01,0x04,0x01,0x82,0x37,0x0A,0x03,0x03,/* [715] OBJ_ms_sgc */
0x2B,0x06,0x01,0x04,0x01,0x82,0x37,0x0A,0x03,0x04,/* [725] OBJ_ms_efs */
0x60,0x86,0x48,0x01,0x86,0xF8,0x42,0x04,0x01,/* [735] OBJ_ns_sgc */
0x55,0x1D,0x1B,                              /* [744] OBJ_delta_crl */
0x55,0x1D,0x15,                              /* [747] OBJ_crl_reason */
0x55,0x1D,0x18,                              /* [750] OBJ_invalidity_date */
0x2B,0x65,0x01,0x04,0x01,                    /* [753] OBJ_sxnet */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x0C,0x01,0x01,/* [758] OBJ_pbe_WithSHA1And128BitRC4 */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x0C,0x01,0x02,/* [768] OBJ_pbe_WithSHA1And40BitRC4 */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x0C,0x01,0x03,/* [778] OBJ_pbe_WithSHA1And3_Key_TripleDES_CBC */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x0C,0x01,0x04,/* [788] OBJ_pbe_WithSHA1And2_Key_TripleDES_CBC */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x0C,0x01,0x05,/* [798] OBJ_pbe_WithSHA1And128BitRC2_CBC */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x0C,0x01,0x06,/* [808] OBJ_pbe_WithSHA1And40BitRC2_CBC */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x0C,0x0A,0x01,0x01,/* [818] OBJ_keyBag */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x0C,0x0A,0x01,0x02,/* [829] OBJ_pkcs8ShroudedKeyBag */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x0C,0x0A,0x01,0x03,/* [840] OBJ_certBag */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x0C,0x0A,0x01,0x04,/* [851] OBJ_crlBag */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x0C,0x0A,0x01,0x05,/* [862] OBJ_secretBag */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x0C,0x0A,0x01,0x06,/* [873] OBJ_safeContentsBag */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,0x14,/* [884] OBJ_friendlyName */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,0x15,/* [893] OBJ_localKeyID */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,0x16,0x01,/* [902] OBJ_x509Certificate */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,0x16,0x02,/* [912] OBJ_sdsiCertificate */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,0x17,0x01,/* [922] OBJ_x509Crl */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x05,0x0D,/* [932] OBJ_pbes2 */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x05,0x0E,/* [941] OBJ_pbmac1 */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x02,0x07,     /* [950] OBJ_hmacWithSHA1 */
0x2B,0x06,0x01,0x05,0x05,0x07,0x02,0x01,     /* [958] OBJ_id_qt_cps */
0x2B,0x06,0x01,0x05,0x05,0x07,0x02,0x02,     /* [966] OBJ_id_qt_unotice */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,0x0F,/* [974] OBJ_SMIMECapabilities */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x05,0x04,/* [983] OBJ_pbeWithMD2AndRC2_CBC */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x05,0x06,/* [992] OBJ_pbeWithMD5AndRC2_CBC */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x05,0x0A,/* [1001] OBJ_pbeWithSHA1AndDES_CBC */
0x2B,0x06,0x01,0x04,0x01,0x82,0x37,0x02,0x01,0x0E,/* [1010] OBJ_ms_ext_req */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,0x0E,/* [1020] OBJ_ext_req */
0x55,0x04,0x29,                              /* [1029] OBJ_name */
0x55,0x04,0x2E,                              /* [1032] OBJ_dnQualifier */
0x2B,0x06,0x01,0x05,0x05,0x07,0x01,          /* [1035] OBJ_id_pe */
0x2B,0x06,0x01,0x05,0x05,0x07,0x30,          /* [1042] OBJ_id_ad */
0x2B,0x06,0x01,0x05,0x05,0x07,0x01,0x01,     /* [1049] OBJ_info_access */
0x2B,0x06,0x01,0x05,0x05,0x07,0x30,0x01,     /* [1057] OBJ_ad_OCSP */
0x2B,0x06,0x01,0x05,0x05,0x07,0x30,0x02,     /* [1065] OBJ_ad_ca_issuers */
0x2B,0x06,0x01,0x05,0x05,0x07,0x03,0x09,     /* [1073] OBJ_OCSP_sign */
0x28,                                        /* [1081] OBJ_iso */
0x2A,                                        /* [1082] OBJ_member_body */
0x2A,0x86,0x48,                              /* [1083] OBJ_ISO_US */
0x2A,0x86,0x48,0xCE,0x38,                    /* [1086] OBJ_X9_57 */
0x2A,0x86,0x48,0xCE,0x38,0x04,               /* [1091] OBJ_X9cm */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x01,     /* [1097] OBJ_pkcs1 */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x05,     /* [1105] OBJ_pkcs5 */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,0x10,/* [1113] OBJ_SMIME */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,0x10,0x00,/* [1122] OBJ_id_smime_mod */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,0x10,0x01,/* [1132] OBJ_id_smime_ct */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,0x10,0x02,/* [1142] OBJ_id_smime_aa */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,0x10,0x03,/* [1152] OBJ_id_smime_alg */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,0x10,0x04,/* [1162] OBJ_id_smime_cd */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,0x10,0x05,/* [1172] OBJ_id_smime_spq */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,0x10,0x06,/* [1182] OBJ_id_smime_cti */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,0x10,0x00,0x01,/* [1192] OBJ_id_smime_mod_cms */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,0x10,0x00,0x02,/* [1203] OBJ_id_smime_mod_ess */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,0x10,0x00,0x03,/* [1214] OBJ_id_smime_mod_oid */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,0x10,0x00,0x04,/* [1225] OBJ_id_smime_mod_msg_v3 */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,0x10,0x00,0x05,/* [1236] OBJ_id_smime_mod_ets_eSignature_88 */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,0x10,0x00,0x06,/* [1247] OBJ_id_smime_mod_ets_eSignature_97 */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,0x10,0x00,0x07,/* [1258] OBJ_id_smime_mod_ets_eSigPolicy_88 */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,0x10,0x00,0x08,/* [1269] OBJ_id_smime_mod_ets_eSigPolicy_97 */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,0x10,0x01,0x01,/* [1280] OBJ_id_smime_ct_receipt */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,0x10,0x01,0x02,/* [1291] OBJ_id_smime_ct_authData */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,0x10,0x01,0x03,/* [1302] OBJ_id_smime_ct_publishCert */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,0x10,0x01,0x04,/* [1313] OBJ_id_smime_ct_TSTInfo */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,0x10,0x01,0x05,/* [1324] OBJ_id_smime_ct_TDTInfo */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,0x10,0x01,0x06,/* [1335] OBJ_id_smime_ct_contentInfo */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,0x10,0x01,0x07,/* [1346] OBJ_id_smime_ct_DVCSRequestData */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,0x10,0x01,0x08,/* [1357] OBJ_id_smime_ct_DVCSResponseData */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,0x10,0x02,0x01,/* [1368] OBJ_id_smime_aa_receiptRequest */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,0x10,0x02,0x02,/* [1379] OBJ_id_smime_aa_securityLabel */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,0x10,0x02,0x03,/* [1390] OBJ_id_smime_aa_mlExpandHistory */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,0x10,0x02,0x04,/* [1401] OBJ_id_smime_aa_contentHint */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,0x10,0x02,0x05,/* [1412] OBJ_id_smime_aa_msgSigDigest */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,0x10,0x02,0x06,/* [1423] OBJ_id_smime_aa_encapContentType */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,0x10,0x02,0x07,/* [1434] OBJ_id_smime_aa_contentIdentifier */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,0x10,0x02,0x08,/* [1445] OBJ_id_smime_aa_macValue */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,0x10,0x02,0x09,/* [1456] OBJ_id_smime_aa_equivalentLabels */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,0x10,0x02,0x0A,/* [1467] OBJ_id_smime_aa_contentReference */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,0x10,0x02,0x0B,/* [1478] OBJ_id_smime_aa_encrypKeyPref */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,0x10,0x02,0x0C,/* [1489] OBJ_id_smime_aa_signingCertificate */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,0x10,0x02,0x0D,/* [1500] OBJ_id_smime_aa_smimeEncryptCerts */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,0x10,0x02,0x0E,/* [1511] OBJ_id_smime_aa_timeStampToken */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,0x10,0x02,0x0F,/* [1522] OBJ_id_smime_aa_ets_sigPolicyId */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,0x10,0x02,0x10,/* [1533] OBJ_id_smime_aa_ets_commitmentType */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,0x10,0x02,0x11,/* [1544] OBJ_id_smime_aa_ets_signerLocation */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,0x10,0x02,0x12,/* [1555] OBJ_id_smime_aa_ets_signerAttr */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,0x10,0x02,0x13,/* [1566] OBJ_id_smime_aa_ets_otherSigCert */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,0x10,0x02,0x14,/* [1577] OBJ_id_smime_aa_ets_contentTimestamp */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,0x10,0x02,0x15,/* [1588] OBJ_id_smime_aa_ets_CertificateRefs */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,0x10,0x02,0x16,/* [1599] OBJ_id_smime_aa_ets_RevocationRefs */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,0x10,0x02,0x17,/* [1610] OBJ_id_smime_aa_ets_certValues */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,0x10,0x02,0x18,/* [1621] OBJ_id_smime_aa_ets_revocationValues */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,0x10,0x02,0x19,/* [1632] OBJ_id_smime_aa_ets_escTimeStamp */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,0x10,0x02,0x1A,/* [1643] OBJ_id_smime_aa_ets_certCRLTimestamp */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,0x10,0x02,0x1B,/* [1654] OBJ_id_smime_aa_ets_archiveTimeStamp */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,0x10,0x02,0x1C,/* [1665] OBJ_id_smime_aa_signatureType */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,0x10,0x02,0x1D,/* [1676] OBJ_id_smime_aa_dvcs_dvc */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,0x10,0x03,0x01,/* [1687] OBJ_id_smime_alg_ESDHwith3DES */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,0x10,0x03,0x02,/* [1698] OBJ_id_smime_alg_ESDHwithRC2 */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,0x10,0x03,0x03,/* [1709] OBJ_id_smime_alg_3DESwrap */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,0x10,0x03,0x04,/* [1720] OBJ_id_smime_alg_RC2wrap */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,0x10,0x03,0x05,/* [1731] OBJ_id_smime_alg_ESDH */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,0x10,0x03,0x06,/* [1742] OBJ_id_smime_alg_CMS3DESwrap */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,0x10,0x03,0x07,/* [1753] OBJ_id_smime_alg_CMSRC2wrap */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,0x10,0x04,0x01,/* [1764] OBJ_id_smime_cd_ldap */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,0x10,0x05,0x01,/* [1775] OBJ_id_smime_spq_ets_sqt_uri */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,0x10,0x05,0x02,/* [1786] OBJ_id_smime_spq_ets_sqt_unotice */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,0x10,0x06,0x01,/* [1797] OBJ_id_smime_cti_ets_proofOfOrigin */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,0x10,0x06,0x02,/* [1808] OBJ_id_smime_cti_ets_proofOfReceipt */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,0x10,0x06,0x03,/* [1819] OBJ_id_smime_cti_ets_proofOfDelivery */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,0x10,0x06,0x04,/* [1830] OBJ_id_smime_cti_ets_proofOfSender */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,0x10,0x06,0x05,/* [1841] OBJ_id_smime_cti_ets_proofOfApproval */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,0x10,0x06,0x06,/* [1852] OBJ_id_smime_cti_ets_proofOfCreation */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x02,0x04,     /* [1863] OBJ_md4 */
0x2B,0x06,0x01,0x05,0x05,0x07,0x00,          /* [1871] OBJ_id_pkix_mod */
0x2B,0x06,0x01,0x05,0x05,0x07,0x02,          /* [1878] OBJ_id_qt */
0x2B,0x06,0x01,0x05,0x05,0x07,0x04,          /* [1885] OBJ_id_it */
0x2B,0x06,0x01,0x05,0x05,0x07,0x05,          /* [1892] OBJ_id_pkip */
0x2B,0x06,0x01,0x05,0x05,0x07,0x06,          /* [1899] OBJ_id_alg */
0x2B,0x06,0x01,0x05,0x05,0x07,0x07,          /* [1906] OBJ_id_cmc */
0x2B,0x06,0x01,0x05,0x05,0x07,0x08,          /* [1913] OBJ_id_on */
0x2B,0x06,0x01,0x05,0x05,0x07,0x09,          /* [1920] OBJ_id_pda */
0x2B,0x06,0x01,0x05,0x05,0x07,0x0A,          /* [1927] OBJ_id_aca */
0x2B,0x06,0x01,0x05,0x05,0x07,0x0B,          /* [1934] OBJ_id_qcs */
0x2B,0x06,0x01,0x05,0x05,0x07,0x0C,          /* [1941] OBJ_id_cct */
0x2B,0x06,0x01,0x05,0x05,0x07,0x00,0x01,     /* [1948] OBJ_id_pkix1_explicit_88 */
0x2B,0x06,0x01,0x05,0x05,0x07,0x00,0x02,     /* [1956] OBJ_id_pkix1_implicit_88 */
0x2B,0x06,0x01,0x05,0x05,0x07,0x00,0x03,     /* [1964] OBJ_id_pkix1_explicit_93 */
0x2B,0x06,0x01,0x05,0x05,0x07,0x00,0x04,     /* [1972] OBJ_id_pkix1_implicit_93 */
0x2B,0x06,0x01,0x05,0x05,0x07,0x00,0x05,     /* [1980] OBJ_id_mod_crmf */
0x2B,0x06,0x01,0x05,0x05,0x07,0x00,0x06,     /* [1988] OBJ_id_mod_cmc */
0x2B,0x06,0x01,0x05,0x05,0x07,0x00,0x07,     /* [1996] OBJ_id_mod_kea_profile_88 */
0x2B,0x06,0x01,0x05,0x05,0x07,0x00,0x08,     /* [2004] OBJ_id_mod_kea_profile_93 */
0x2B,0x06,0x01,0x05,0x05,0x07,0x00,0x09,     /* [2012] OBJ_id_mod_cmp */
0x2B,0x06,0x01,0x05,0x05,0x07,0x00,0x0A,     /* [2020] OBJ_id_mod_qualified_cert_88 */
0x2B,0x06,0x01,0x05,0x05,0x07,0x00,0x0B,     /* [2028] OBJ_id_mod_qualified_cert_93 */
0x2B,0x06,0x01,0x05,0x05,0x07,0x00,0x0C,     /* [2036] OBJ_id_mod_attribute_cert */
0x2B,0x06,0x01,0x05,0x05,0x07,0x00,0x0D,     /* [2044] OBJ_id_mod_timestamp_protocol */
0x2B,0x06,0x01,0x05,0x05,0x07,0x00,0x0E,     /* [2052] OBJ_id_mod_ocsp */
0x2B,0x06,0x01,0x05,0x05,0x07,0x00,0x0F,     /* [2060] OBJ_id_mod_dvcs */
0x2B,0x06,0x01,0x05,0x05,0x07,0x00,0x10,     /* [2068] OBJ_id_mod_cmp2000 */
0x2B,0x06,0x01,0x05,0x05,0x07,0x01,0x02,     /* [2076] OBJ_biometricInfo */
0x2B,0x06,0x01,0x05,0x05,0x07,0x01,0x03,     /* [2084] OBJ_qcStatements */
0x2B,0x06,0x01,0x05,0x05,0x07,0x01,0x04,     /* [2092] OBJ_ac_auditEntity */
0x2B,0x06,0x01,0x05,0x05,0x07,0x01,0x05,     /* [2100] OBJ_ac_targeting */
0x2B,0x06,0x01,0x05,0x05,0x07,0x01,0x06,     /* [2108] OBJ_aaControls */
0x2B,0x06,0x01,0x05,0x05,0x07,0x01,0x07,     /* [2116] OBJ_sbgp_ipAddrBlock */
0x2B,0x06,0x01,0x05,0x05,0x07,0x01,0x08,     /* [2124] OBJ_sbgp_autonomousSysNum */
0x2B,0x06,0x01,0x05,0x05,0x07,0x01,0x09,     /* [2132] OBJ_sbgp_routerIdentifier */
0x2B,0x06,0x01,0x05,0x05,0x07,0x02,0x03,     /* [2140] OBJ_textNotice */
0x2B,0x06,0x01,0x05,0x05,0x07,0x03,0x05,     /* [2148] OBJ_ipsecEndSystem */
0x2B,0x06,0x01,0x05,0x05,0x07,0x03,0x06,     /* [2156] OBJ_ipsecTunnel */
0x2B,0x06,0x01,0x05,0x05,0x07,0x03,0x07,     /* [2164] OBJ_ipsecUser */
0x2B,0x06,0x01,0x05,0x05,0x07,0x03,0x0A,     /* [2172] OBJ_dvcs */
0x2B,0x06,0x01,0x05,0x05,0x07,0x04,0x01,     /* [2180] OBJ_id_it_caProtEncCert */
0x2B,0x06,0x01,0x05,0x05,0x07,0x04,0x02,     /* [2188] OBJ_id_it_signKeyPairTypes */
0x2B,0x06,0x01,0x05,0x05,0x07,0x04,0x03,     /* [2196] OBJ_id_it_encKeyPairTypes */
0x2B,0x06,0x01,0x05,0x05,0x07,0x04,0x04,     /* [2204] OBJ_id_it_preferredSymmAlg */
0x2B,0x06,0x01,0x05,0x05,0x07,0x04,0x05,     /* [2212] OBJ_id_it_caKeyUpdateInfo */
0x2B,0x06,0x01,0x05,0x05,0x07,0x04,0x06,     /* [2220] OBJ_id_it_currentCRL */
0x2B,0x06,0x01,0x05,0x05,0x07,0x04,0x07,     /* [2228] OBJ_id_it_unsupportedOIDs */
0x2B,0x06,0x01,0x05,0x05,0x07,0x04,0x08,     /* [2236] OBJ_id_it_subscriptionRequest */
0x2B,0x06,0x01,0x05,0x05,0x07,0x04,0x09,     /* [2244] OBJ_id_it_subscriptionResponse */
0x2B,0x06,0x01,0x05,0x05,0x07,0x04,0x0A,     /* [2252] OBJ_id_it_keyPairParamReq */
0x2B,0x06,0x01,0x05,0x05,0x07,0x04,0x0B,     /* [2260] OBJ_id_it_keyPairParamRep */
0x2B,0x06,0x01,0x05,0x05,0x07,0x04,0x0C,     /* [2268] OBJ_id_it_revPassphrase */
0x2B,0x06,0x01,0x05,0x05,0x07,0x04,0x0D,     /* [2276] OBJ_id_it_implicitConfirm */
0x2B,0x06,0x01,0x05,0x05,0x07,0x04,0x0E,     /* [2284] OBJ_id_it_confirmWaitTime */
0x2B,0x06,0x01,0x05,0x05,0x07,0x04,0x0F,     /* [2292] OBJ_id_it_origPKIMessage */
0x2B,0x06,0x01,0x05,0x05,0x07,0x05,0x01,     /* [2300] OBJ_id_regCtrl */
0x2B,0x06,0x01,0x05,0x05,0x07,0x05,0x02,     /* [2308] OBJ_id_regInfo */
0x2B,0x06,0x01,0x05,0x05,0x07,0x05,0x01,0x01,/* [2316] OBJ_id_regCtrl_regToken */
0x2B,0x06,0x01,0x05,0x05,0x07,0x05,0x01,0x02,/* [2325] OBJ_id_regCtrl_authenticator */
0x2B,0x06,0x01,0x05,0x05,0x07,0x05,0x01,0x03,/* [2334] OBJ_id_regCtrl_pkiPublicationInfo */
0x2B,0x06,0x01,0x05,0x05,0x07,0x05,0x01,0x04,/* [2343] OBJ_id_regCtrl_pkiArchiveOptions */
0x2B,0x06,0x01,0x05,0x05,0x07,0x05,0x01,0x05,/* [2352] OBJ_id_regCtrl_oldCertID */
0x2B,0x06,0x01,0x05,0x05,0x07,0x05,0x01,0x06,/* [2361] OBJ_id_regCtrl_protocolEncrKey */
0x2B,0x06,0x01,0x05,0x05,0x07,0x05,0x02,0x01,/* [2370] OBJ_id_regInfo_utf8Pairs */
0x2B,0x06,0x01,0x05,0x05,0x07,0x05,0x02,0x02,/* [2379] OBJ_id_regInfo_certReq */
0x2B,0x06,0x01,0x05,0x05,0x07,0x06,0x01,     /* [2388] OBJ_id_alg_des40 */
0x2B,0x06,0x01,0x05,0x05,0x07,0x06,0x02,     /* [2396] OBJ_id_alg_noSignature */
0x2B,0x06,0x01,0x05,0x05,0x07,0x06,0x03,     /* [2404] OBJ_id_alg_dh_sig_hmac_sha1 */
0x2B,0x06,0x01,0x05,0x05,0x07,0x06,0x04,     /* [2412] OBJ_id_alg_dh_pop */
0x2B,0x06,0x01,0x05,0x05,0x07,0x07,0x01,     /* [2420] OBJ_id_cmc_statusInfo */
0x2B,0x06,0x01,0x05,0x05,0x07,0x07,0x02,     /* [2428] OBJ_id_cmc_identification */
0x2B,0x06,0x01,0x05,0x05,0x07,0x07,0x03,     /* [2436] OBJ_id_cmc_identityProof */
0x2B,0x06,0x01,0x05,0x05,0x07,0x07,0x04,     /* [2444] OBJ_id_cmc_dataReturn */
0x2B,0x06,0x01,0x05,0x05,0x07,0x07,0x05,     /* [2452] OBJ_id_cmc_transactionId */
0x2B,0x06,0x01,0x05,0x05,0x07,0x07,0x06,     /* [2460] OBJ_id_cmc_senderNonce */
0x2B,0x06,0x01,0x05,0x05,0x07,0x07,0x07,     /* [2468] OBJ_id_cmc_recipientNonce */
0x2B,0x06,0x01,0x05,0x05,0x07,0x07,0x08,     /* [2476] OBJ_id_cmc_addExtensions */
0x2B,0x06,0x01,0x05,0x05,0x07,0x07,0x09,     /* [2484] OBJ_id_cmc_encryptedPOP */
0x2B,0x06,0x01,0x05,0x05,0x07,0x07,0x0A,     /* [2492] OBJ_id_cmc_decryptedPOP */
0x2B,0x06,0x01,0x05,0x05,0x07,0x07,0x0B,     /* [2500] OBJ_id_cmc_lraPOPWitness */
0x2B,0x06,0x01,0x05,0x05,0x07,0x07,0x0F,     /* [2508] OBJ_id_cmc_getCert */
0x2B,0x06,0x01,0x05,0x05,0x07,0x07,0x10,     /* [2516] OBJ_id_cmc_getCRL */
0x2B,0x06,0x01,0x05,0x05,0x07,0x07,0x11,     /* [2524] OBJ_id_cmc_revokeRequest */
0x2B,0x06,0x01,0x05,0x05,0x07,0x07,0x12,     /* [2532] OBJ_id_cmc_regInfo */
0x2B,0x06,0x01,0x05,0x05,0x07,0x07,0x13,     /* [2540] OBJ_id_cmc_responseInfo */
0x2B,0x06,0x01,0x05,0x05,0x07,0x07,0x15,     /* [2548] OBJ_id_cmc_queryPending */
0x2B,0x06,0x01,0x05,0x05,0x07,0x07,0x16,     /* [2556] OBJ_id_cmc_popLinkRandom */
0x2B,0x06,0x01,0x05,0x05,0x07,0x07,0x17,     /* [2564] OBJ_id_cmc_popLinkWitness */
0x2B,0x06,0x01,0x05,0x05,0x07,0x07,0x18,     /* [2572] OBJ_id_cmc_confirmCertAcceptance */
0x2B,0x06,0x01,0x05,0x05,0x07,0x08,0x01,     /* [2580] OBJ_id_on_personalData */
0x2B,0x06,0x01,0x05,0x05,0x07,0x09,0x01,     /* [2588] OBJ_id_pda_dateOfBirth */
0x2B,0x06,0x01,0x05,0x05,0x07,0x09,0x02,     /* [2596] OBJ_id_pda_placeOfBirth */
0x2B,0x06,0x01,0x05,0x05,0x07,0x09,0x03,     /* [2604] OBJ_id_pda_gender */
0x2B,0x06,0x01,0x05,0x05,0x07,0x09,0x04,     /* [2612] OBJ_id_pda_countryOfCitizenship */
0x2B,0x06,0x01,0x05,0x05,0x07,0x09,0x05,     /* [2620] OBJ_id_pda_countryOfResidence */
0x2B,0x06,0x01,0x05,0x05,0x07,0x0A,0x01,     /* [2628] OBJ_id_aca_authenticationInfo */
0x2B,0x06,0x01,0x05,0x05,0x07,0x0A,0x02,     /* [2636] OBJ_id_aca_accessIdentity */
0x2B,0x06,0x01,0x05,0x05,0x07,0x0A,0x03,     /* [2644] OBJ_id_aca_chargingIdentity */
0x2B,0x06,0x01,0x05,0x05,0x07,0x0A,0x04,     /* [2652] OBJ_id_aca_group */
0x2B,0x06,0x01,0x05,0x05,0x07,0x0A,0x05,     /* [2660] OBJ_id_aca_role */
0x2B,0x06,0x01,0x05,0x05,0x07,0x0B,0x01,     /* [2668] OBJ_id_qcs_pkixQCSyntax_v1 */
0x2B,0x06,0x01,0x05,0x05,0x07,0x0C,0x01,     /* [2676] OBJ_id_cct_crs */
0x2B,0x06,0x01,0x05,0x05,0x07,0x0C,0x02,     /* [2684] OBJ_id_cct_PKIData */
0x2B,0x06,0x01,0x05,0x05,0x07,0x0C,0x03,     /* [2692] OBJ_id_cct_PKIResponse */
0x2B,0x06,0x01,0x05,0x05,0x07,0x30,0x03,     /* [2700] OBJ_ad_timeStamping */
0x2B,0x06,0x01,0x05,0x05,0x07,0x30,0x04,     /* [2708] OBJ_ad_dvcs */
0x2B,0x06,0x01,0x05,0x05,0x07,0x30,0x01,0x01,/* [2716] OBJ_id_pkix_OCSP_basic */
0x2B,0x06,0x01,0x05,0x05,0x07,0x30,0x01,0x02,/* [2725] OBJ_id_pkix_OCSP_Nonce */
0x2B,0x06,0x01,0x05,0x05,0x07,0x30,0x01,0x03,/* [2734] OBJ_id_pkix_OCSP_CrlID */
0x2B,0x06,0x01,0x05,0x05,0x07,0x30,0x01,0x04,/* [2743] OBJ_id_pkix_OCSP_acceptableResponses */
0x2B,0x06,0x01,0x05,0x05,0x07,0x30,0x01,0x05,/* [2752] OBJ_id_pkix_OCSP_noCheck */
0x2B,0x06,0x01,0x05,0x05,0x07,0x30,0x01,0x06,/* [2761] OBJ_id_pkix_OCSP_archiveCutoff */
0x2B,0x06,0x01,0x05,0x05,0x07,0x30,0x01,0x07,/* [2770] OBJ_id_pkix_OCSP_serviceLocator */
0x2B,0x06,0x01,0x05,0x05,0x07,0x30,0x01,0x08,/* [2779] OBJ_id_pkix_OCSP_extendedStatus */
0x2B,0x06,0x01,0x05,0x05,0x07,0x30,0x01,0x09,/* [2788] OBJ_id_pkix_OCSP_valid */
0x2B,0x06,0x01,0x05,0x05,0x07,0x30,0x01,0x0A,/* [2797] OBJ_id_pkix_OCSP_path */
0x2B,0x06,0x01,0x05,0x05,0x07,0x30,0x01,0x0B,/* [2806] OBJ_id_pkix_OCSP_trustRoot */
0x2B,0x0E,0x03,0x02,                         /* [2815] OBJ_algorithm */
0x2B,0x0E,0x03,0x02,0x0B,                    /* [2819] OBJ_rsaSignature */
0x55,0x08,                                   /* [2824] OBJ_X500algorithms */
0x2B,                                        /* [2826] OBJ_org */
0x2B,0x06,                                   /* [2827] OBJ_dod */
0x2B,0x06,0x01,                              /* [2829] OBJ_iana */
0x2B,0x06,0x01,0x01,                         /* [2832] OBJ_Directory */
0x2B,0x06,0x01,0x02,                         /* [2836] OBJ_Management */
0x2B,0x06,0x01,0x03,                         /* [2840] OBJ_Experimental */
0x2B,0x06,0x01,0x04,                         /* [2844] OBJ_Private */
0x2B,0x06,0x01,0x05,                         /* [2848] OBJ_Security */
0x2B,0x06,0x01,0x06,                         /* [2852] OBJ_SNMPv2 */
0x2B,0x06,0x01,0x07,                         /* [2856] OBJ_Mail */
0x2B,0x06,0x01,0x04,0x01,                    /* [2860] OBJ_Enterprises */
0x2B,0x06,0x01,0x04,0x01,0x8B,0x3A,0x82,0x58,/* [2865] OBJ_dcObject */
0x09,0x92,0x26,0x89,0x93,0xF2,0x2C,0x64,0x01,0x19,/* [2874] OBJ_domainComponent */
0x09,0x92,0x26,0x89,0x93,0xF2,0x2C,0x64,0x04,0x0D,/* [2884] OBJ_Domain */
0x00,                                        /* [2894] OBJ_joint_iso_ccitt */
0x55,0x01,0x05,                              /* [2895] OBJ_selected_attribute_types */
0x55,0x01,0x05,0x37,                         /* [2898] OBJ_clearance */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x01,0x03,/* [2902] OBJ_md4WithRSAEncryption */
0x2B,0x06,0x01,0x05,0x05,0x07,0x01,0x0A,     /* [2911] OBJ_ac_proxying */
0x2B,0x06,0x01,0x05,0x05,0x07,0x01,0x0B,     /* [2919] OBJ_sinfo_access */
0x2B,0x06,0x01,0x05,0x05,0x07,0x0A,0x06,     /* [2927] OBJ_id_aca_encAttrs */
0x55,0x04,0x48,                              /* [2935] OBJ_role */
0x55,0x1D,0x24,                              /* [2938] OBJ_policy_constraints */
0x55,0x1D,0x37,                              /* [2941] OBJ_target_information */
0x55,0x1D,0x38,                              /* [2944] OBJ_no_rev_avail */
0x00,                                        /* [2947] OBJ_ccitt */
0x2A,0x86,0x48,0xCE,0x3D,                    /* [2948] OBJ_ansi_X9_62 */
0x2A,0x86,0x48,0xCE,0x3D,0x01,0x01,          /* [2953] OBJ_X9_62_prime_field */
0x2A,0x86,0x48,0xCE,0x3D,0x01,0x02,          /* [2960] OBJ_X9_62_characteristic_two_field */
0x2A,0x86,0x48,0xCE,0x3D,0x02,0x01,          /* [2967] OBJ_X9_62_id_ecPublicKey */
0x2A,0x86,0x48,0xCE,0x3D,0x03,0x01,0x01,     /* [2974] OBJ_X9_62_prime192v1 */
0x2A,0x86,0x48,0xCE,0x3D,0x03,0x01,0x02,     /* [2982] OBJ_X9_62_prime192v2 */
0x2A,0x86,0x48,0xCE,0x3D,0x03,0x01,0x03,     /* [2990] OBJ_X9_62_prime192v3 */
0x2A,0x86,0x48,0xCE,0x3D,0x03,0x01,0x04,     /* [2998] OBJ_X9_62_prime239v1 */
0x2A,0x86,0x48,0xCE,0x3D,0x03,0x01,0x05,     /* [3006] OBJ_X9_62_prime239v2 */
0x2A,0x86,0x48,0xCE,0x3D,0x03,0x01,0x06,     /* [3014] OBJ_X9_62_prime239v3 */
0x2A,0x86,0x48,0xCE,0x3D,0x03,0x01,0x07,     /* [3022] OBJ_X9_62_prime256v1 */
0x2A,0x86,0x48,0xCE,0x3D,0x04,0x01,          /* [3030] OBJ_ecdsa_with_SHA1 */
0x2B,0x06,0x01,0x04,0x01,0x82,0x37,0x11,0x01,/* [3037] OBJ_ms_csp_name */
0x60,0x86,0x48,0x01,0x65,0x03,0x04,0x01,0x01,/* [3046] OBJ_aes_128_ecb */
0x60,0x86,0x48,0x01,0x65,0x03,0x04,0x01,0x02,/* [3055] OBJ_aes_128_cbc */
0x60,0x86,0x48,0x01,0x65,0x03,0x04,0x01,0x03,/* [3064] OBJ_aes_128_ofb128 */
0x60,0x86,0x48,0x01,0x65,0x03,0x04,0x01,0x04,/* [3073] OBJ_aes_128_cfb128 */
0x60,0x86,0x48,0x01,0x65,0x03,0x04,0x01,0x15,/* [3082] OBJ_aes_192_ecb */
0x60,0x86,0x48,0x01,0x65,0x03,0x04,0x01,0x16,/* [3091] OBJ_aes_192_cbc */
0x60,0x86,0x48,0x01,0x65,0x03,0x04,0x01,0x17,/* [3100] OBJ_aes_192_ofb128 */
0x60,0x86,0x48,0x01,0x65,0x03,0x04,0x01,0x18,/* [3109] OBJ_aes_192_cfb128 */
0x60,0x86,0x48,0x01,0x65,0x03,0x04,0x01,0x29,/* [3118] OBJ_aes_256_ecb */
0x60,0x86,0x48,0x01,0x65,0x03,0x04,0x01,0x2A,/* [3127] OBJ_aes_256_cbc */
0x60,0x86,0x48,0x01,0x65,0x03,0x04,0x01,0x2B,/* [3136] OBJ_aes_256_ofb128 */
0x60,0x86,0x48,0x01,0x65,0x03,0x04,0x01,0x2C,/* [3145] OBJ_aes_256_cfb128 */
0x55,0x1D,0x17,                              /* [3154] OBJ_hold_instruction_code */
0x2A,0x86,0x48,0xCE,0x38,0x02,0x01,          /* [3157] OBJ_hold_instruction_none */
0x2A,0x86,0x48,0xCE,0x38,0x02,0x02,          /* [3164] OBJ_hold_instruction_call_issuer */
0x2A,0x86,0x48,0xCE,0x38,0x02,0x03,          /* [3171] OBJ_hold_instruction_reject */
0x09,                                        /* [3178] OBJ_data */
0x09,0x92,0x26,                              /* [3179] OBJ_pss */
0x09,0x92,0x26,0x89,0x93,0xF2,0x2C,          /* [3182] OBJ_ucl */
0x09,0x92,0x26,0x89,0x93,0xF2,0x2C,0x64,     /* [3189] OBJ_pilot */
0x09,0x92,0x26,0x89,0x93,0xF2,0x2C,0x64,0x01,/* [3197] OBJ_pilotAttributeType */
0x09,0x92,0x26,0x89,0x93,0xF2,0x2C,0x64,0x03,/* [3206] OBJ_pilotAttributeSyntax */
0x09,0x92,0x26,0x89,0x93,0xF2,0x2C,0x64,0x04,/* [3215] OBJ_pilotObjectClass */
0x09,0x92,0x26,0x89,0x93,0xF2,0x2C,0x64,0x0A,/* [3224] OBJ_pilotGroups */
0x09,0x92,0x26,0x89,0x93,0xF2,0x2C,0x64,0x03,0x04,/* [3233] OBJ_iA5StringSyntax */
0x09,0x92,0x26,0x89,0x93,0xF2,0x2C,0x64,0x03,0x05,/* [3243] OBJ_caseIgnoreIA5StringSyntax */
0x09,0x92,0x26,0x89,0x93,0xF2,0x2C,0x64,0x04,0x03,/* [3253] OBJ_pilotObject */
0x09,0x92,0x26,0x89,0x93,0xF2,0x2C,0x64,0x04,0x04,/* [3263] OBJ_pilotPerson */
0x09,0x92,0x26,0x89,0x93,0xF2,0x2C,0x64,0x04,0x05,/* [3273] OBJ_account */
0x09,0x92,0x26,0x89,0x93,0xF2,0x2C,0x64,0x04,0x06,/* [3283] OBJ_document */
0x09,0x92,0x26,0x89,0x93,0xF2,0x2C,0x64,0x04,0x07,/* [3293] OBJ_room */
0x09,0x92,0x26,0x89,0x93,0xF2,0x2C,0x64,0x04,0x09,/* [3303] OBJ_documentSeries */
0x09,0x92,0x26,0x89,0x93,0xF2,0x2C,0x64,0x04,0x0E,/* [3313] OBJ_rFC822localPart */
0x09,0x92,0x26,0x89,0x93,0xF2,0x2C,0x64,0x04,0x0F,/* [3323] OBJ_dNSDomain */
0x09,0x92,0x26,0x89,0x93,0xF2,0x2C,0x64,0x04,0x11,/* [3333] OBJ_domainRelatedObject */
0x09,0x92,0x26,0x89,0x93,0xF2,0x2C,0x64,0x04,0x12,/* [3343] OBJ_friendlyCountry */
0x09,0x92,0x26,0x89,0x93,0xF2,0x2C,0x64,0x04,0x13,/* [3353] OBJ_simpleSecurityObject */
0x09,0x92,0x26,0x89,0x93,0xF2,0x2C,0x64,0x04,0x14,/* [3363] OBJ_pilotOrganization */
0x09,0x92,0x26,0x89,0x93,0xF2,0x2C,0x64,0x04,0x15,/* [3373] OBJ_pilotDSA */
0x09,0x92,0x26,0x89,0x93,0xF2,0x2C,0x64,0x04,0x16,/* [3383] OBJ_qualityLabelledData */
0x09,0x92,0x26,0x89,0x93,0xF2,0x2C,0x64,0x01,0x01,/* [3393] OBJ_userId */
0x09,0x92,0x26,0x89,0x93,0xF2,0x2C,0x64,0x01,0x02,/* [3403] OBJ_textEncodedORAddress */
0x09,0x92,0x26,0x89,0x93,0xF2,0x2C,0x64,0x01,0x03,/* [3413] OBJ_rfc822Mailbox */
0x09,0x92,0x26,0x89,0x93,0xF2,0x2C,0x64,0x01,0x04,/* [3423] OBJ_info */
0x09,0x92,0x26,0x89,0x93,0xF2,0x2C,0x64,0x01,0x05,/* [3433] OBJ_favouriteDrink */
0x09,0x92,0x26,0x89,0x93,0xF2,0x2C,0x64,0x01,0x06,/* [3443] OBJ_roomNumber */
0x09,0x92,0x26,0x89,0x93,0xF2,0x2C,0x64,0x01,0x07,/* [3453] OBJ_photo */
0x09,0x92,0x26,0x89,0x93,0xF2,0x2C,0x64,0x01,0x08,/* [3463] OBJ_userClass */
0x09,0x92,0x26,0x89,0x93,0xF2,0x2C,0x64,0x01,0x09,/* [3473] OBJ_host */
0x09,0x92,0x26,0x89,0x93,0xF2,0x2C,0x64,0x01,0x0A,/* [3483] OBJ_manager */
0x09,0x92,0x26,0x89,0x93,0xF2,0x2C,0x64,0x01,0x0B,/* [3493] OBJ_documentIdentifier */
0x09,0x92,0x26,0x89,0x93,0xF2,0x2C,0x64,0x01,0x0C,/* [3503] OBJ_documentTitle */
0x09,0x92,0x26,0x89,0x93,0xF2,0x2C,0x64,0x01,0x0D,/* [3513] OBJ_documentVersion */
0x09,0x92,0x26,0x89,0x93,0xF2,0x2C,0x64,0x01,0x0E,/* [3523] OBJ_documentAuthor */
0x09,0x92,0x26,0x89,0x93,0xF2,0x2C,0x64,0x01,0x0F,/* [3533] OBJ_documentLocation */
0x09,0x92,0x26,0x89,0x93,0xF2,0x2C,0x64,0x01,0x14,/* [3543] OBJ_homeTelephoneNumber */
0x09,0x92,0x26,0x89,0x93,0xF2,0x2C,0x64,0x01,0x15,/* [3553] OBJ_secretary */
0x09,0x92,0x26,0x89,0x93,0xF2,0x2C,0x64,0x01,0x16,/* [3563] OBJ_otherMailbox */
0x09,0x92,0x26,0x89,0x93,0xF2,0x2C,0x64,0x01,0x17,/* [3573] OBJ_lastModifiedTime */
0x09,0x92,0x26,0x89,0x93,0xF2,0x2C,0x64,0x01,0x18,/* [3583] OBJ_lastModifiedBy */
0x09,0x92,0x26,0x89,0x93,0xF2,0x2C,0x64,0x01,0x1A,/* [3593] OBJ_aRecord */
0x09,0x92,0x26,0x89,0x93,0xF2,0x2C,0x64,0x01,0x1B,/* [3603] OBJ_pilotAttributeType27 */
0x09,0x92,0x26,0x89,0x93,0xF2,0x2C,0x64,0x01,0x1C,/* [3613] OBJ_mXRecord */
0x09,0x92,0x26,0x89,0x93,0xF2,0x2C,0x64,0x01,0x1D,/* [3623] OBJ_nSRecord */
0x09,0x92,0x26,0x89,0x93,0xF2,0x2C,0x64,0x01,0x1E,/* [3633] OBJ_sOARecord */
0x09,0x92,0x26,0x89,0x93,0xF2,0x2C,0x64,0x01,0x1F,/* [3643] OBJ_cNAMERecord */
0x09,0x92,0x26,0x89,0x93,0xF2,0x2C,0x64,0x01,0x25,/* [3653] OBJ_associatedDomain */
0x09,0x92,0x26,0x89,0x93,0xF2,0x2C,0x64,0x01,0x26,/* [3663] OBJ_associatedName */
0x09,0x92,0x26,0x89,0x93,0xF2,0x2C,0x64,0x01,0x27,/* [3673] OBJ_homePostalAddress */
0x09,0x92,0x26,0x89,0x93,0xF2,0x2C,0x64,0x01,0x28,/* [3683] OBJ_personalTitle */
0x09,0x92,0x26,0x89,0x93,0xF2,0x2C,0x64,0x01,0x29,/* [3693] OBJ_mobileTelephoneNumber */
0x09,0x92,0x26,0x89,0x93,0xF2,0x2C,0x64,0x01,0x2A,/* [3703] OBJ_pagerTelephoneNumber */
0x09,0x92,0x26,0x89,0x93,0xF2,0x2C,0x64,0x01,0x2B,/* [3713] OBJ_friendlyCountryName */
0x09,0x92,0x26,0x89,0x93,0xF2,0x2C,0x64,0x01,0x2D,/* [3723] OBJ_organizationalStatus */
0x09,0x92,0x26,0x89,0x93,0xF2,0x2C,0x64,0x01,0x2E,/* [3733] OBJ_janetMailbox */
0x09,0x92,0x26,0x89,0x93,0xF2,0x2C,0x64,0x01,0x2F,/* [3743] OBJ_mailPreferenceOption */
0x09,0x92,0x26,0x89,0x93,0xF2,0x2C,0x64,0x01,0x30,/* [3753] OBJ_buildingName */
0x09,0x92,0x26,0x89,0x93,0xF2,0x2C,0x64,0x01,0x31,/* [3763] OBJ_dSAQuality */
0x09,0x92,0x26,0x89,0x93,0xF2,0x2C,0x64,0x01,0x32,/* [3773] OBJ_singleLevelQuality */
0x09,0x92,0x26,0x89,0x93,0xF2,0x2C,0x64,0x01,0x33,/* [3783] OBJ_subtreeMinimumQuality */
0x09,0x92,0x26,0x89,0x93,0xF2,0x2C,0x64,0x01,0x34,/* [3793] OBJ_subtreeMaximumQuality */
0x09,0x92,0x26,0x89,0x93,0xF2,0x2C,0x64,0x01,0x35,/* [3803] OBJ_personalSignature */
0x09,0x92,0x26,0x89,0x93,0xF2,0x2C,0x64,0x01,0x36,/* [3813] OBJ_dITRedirect */
0x09,0x92,0x26,0x89,0x93,0xF2,0x2C,0x64,0x01,0x37,/* [3823] OBJ_audio */
0x09,0x92,0x26,0x89,0x93,0xF2,0x2C,0x64,0x01,0x38,/* [3833] OBJ_documentPublisher */
0x55,0x04,0x2D,                              /* [3843] OBJ_x500UniqueIdentifier */
0x2B,0x06,0x01,0x07,0x01,                    /* [3846] OBJ_mime_mhs */
0x2B,0x06,0x01,0x07,0x01,0x01,               /* [3851] OBJ_mime_mhs_headings */
0x2B,0x06,0x01,0x07,0x01,0x02,               /* [3857] OBJ_mime_mhs_bodies */
0x2B,0x06,0x01,0x07,0x01,0x01,0x01,          /* [3863] OBJ_id_hex_partial_message */
0x2B,0x06,0x01,0x07,0x01,0x01,0x02,          /* [3870] OBJ_id_hex_multipart_message */
0x55,0x04,0x2C,                              /* [3877] OBJ_generationQualifier */
0x55,0x04,0x41,                              /* [3880] OBJ_pseudonym */
0x67,0x2A,                                   /* [3883] OBJ_id_set */
0x67,0x2A,0x00,                              /* [3885] OBJ_set_ctype */
0x67,0x2A,0x01,                              /* [3888] OBJ_set_msgExt */
0x67,0x2A,0x03,                              /* [3891] OBJ_set_attr */
0x67,0x2A,0x05,                              /* [3894] OBJ_set_policy */
0x67,0x2A,0x07,                              /* [3897] OBJ_set_certExt */
0x67,0x2A,0x08,                              /* [3900] OBJ_set_brand */
0x67,0x2A,0x00,0x00,                         /* [3903] OBJ_setct_PANData */
0x67,0x2A,0x00,0x01,                         /* [3907] OBJ_setct_PANToken */
0x67,0x2A,0x00,0x02,                         /* [3911] OBJ_setct_PANOnly */
0x67,0x2A,0x00,0x03,                         /* [3915] OBJ_setct_OIData */
0x67,0x2A,0x00,0x04,                         /* [3919] OBJ_setct_PI */
0x67,0x2A,0x00,0x05,                         /* [3923] OBJ_setct_PIData */
0x67,0x2A,0x00,0x06,                         /* [3927] OBJ_setct_PIDataUnsigned */
0x67,0x2A,0x00,0x07,                         /* [3931] OBJ_setct_HODInput */
0x67,0x2A,0x00,0x08,                         /* [3935] OBJ_setct_AuthResBaggage */
0x67,0x2A,0x00,0x09,                         /* [3939] OBJ_setct_AuthRevReqBaggage */
0x67,0x2A,0x00,0x0A,                         /* [3943] OBJ_setct_AuthRevResBaggage */
0x67,0x2A,0x00,0x0B,                         /* [3947] OBJ_setct_CapTokenSeq */
0x67,0x2A,0x00,0x0C,                         /* [3951] OBJ_setct_PInitResData */
0x67,0x2A,0x00,0x0D,                         /* [3955] OBJ_setct_PI_TBS */
0x67,0x2A,0x00,0x0E,                         /* [3959] OBJ_setct_PResData */
0x67,0x2A,0x00,0x10,                         /* [3963] OBJ_setct_AuthReqTBS */
0x67,0x2A,0x00,0x11,                         /* [3967] OBJ_setct_AuthResTBS */
0x67,0x2A,0x00,0x12,                         /* [3971] OBJ_setct_AuthResTBSX */
0x67,0x2A,0x00,0x13,                         /* [3975] OBJ_setct_AuthTokenTBS */
0x67,0x2A,0x00,0x14,                         /* [3979] OBJ_setct_CapTokenData */
0x67,0x2A,0x00,0x15,                         /* [3983] OBJ_setct_CapTokenTBS */
0x67,0x2A,0x00,0x16,                         /* [3987] OBJ_setct_AcqCardCodeMsg */
0x67,0x2A,0x00,0x17,                         /* [3991] OBJ_setct_AuthRevReqTBS */
0x67,0x2A,0x00,0x18,                         /* [3995] OBJ_setct_AuthRevResData */
0x67,0x2A,0x00,0x19,                         /* [3999] OBJ_setct_AuthRevResTBS */
0x67,0x2A,0x00,0x1A,                         /* [4003] OBJ_setct_CapReqTBS */
0x67,0x2A,0x00,0x1B,                         /* [4007] OBJ_setct_CapReqTBSX */
0x67,0x2A,0x00,0x1C,                         /* [4011] OBJ_setct_CapResData */
0x67,0x2A,0x00,0x1D,                         /* [4015] OBJ_setct_CapRevReqTBS */
0x67,0x2A,0x00,0x1E,                         /* [4019] OBJ_setct_CapRevReqTBSX */
0x67,0x2A,0x00,0x1F,                         /* [4023] OBJ_setct_CapRevResData */
0x67,0x2A,0x00,0x20,                         /* [4027] OBJ_setct_CredReqTBS */
0x67,0x2A,0x00,0x21,                         /* [4031] OBJ_setct_CredReqTBSX */
0x67,0x2A,0x00,0x22,                         /* [4035] OBJ_setct_CredResData */
0x67,0x2A,0x00,0x23,                         /* [4039] OBJ_setct_CredRevReqTBS */
0x67,0x2A,0x00,0x24,                         /* [4043] OBJ_setct_CredRevReqTBSX */
0x67,0x2A,0x00,0x25,                         /* [4047] OBJ_setct_CredRevResData */
0x67,0x2A,0x00,0x26,                         /* [4051] OBJ_setct_PCertReqData */
0x67,0x2A,0x00,0x27,                         /* [4055] OBJ_setct_PCertResTBS */
0x67,0x2A,0x00,0x28,                         /* [4059] OBJ_setct_BatchAdminReqData */
0x67,0x2A,0x00,0x29,                         /* [4063] OBJ_setct_BatchAdminResData */
0x67,0x2A,0x00,0x2A,                         /* [4067] OBJ_setct_CardCInitResTBS */
0x67,0x2A,0x00,0x2B,                         /* [4071] OBJ_setct_MeAqCInitResTBS */
0x67,0x2A,0x00,0x2C,                         /* [4075] OBJ_setct_RegFormResTBS */
0x67,0x2A,0x00,0x2D,                         /* [4079] OBJ_setct_CertReqData */
0x67,0x2A,0x00,0x2E,                         /* [4083] OBJ_setct_CertReqTBS */
0x67,0x2A,0x00,0x2F,                         /* [4087] OBJ_setct_CertResData */
0x67,0x2A,0x00,0x30,                         /* [4091] OBJ_setct_CertInqReqTBS */
0x67,0x2A,0x00,0x31,                         /* [4095] OBJ_setct_ErrorTBS */
0x67,0x2A,0x00,0x32,                         /* [4099] OBJ_setct_PIDualSignedTBE */
0x67,0x2A,0x00,0x33,                         /* [4103] OBJ_setct_PIUnsignedTBE */
0x67,0x2A,0x00,0x34,                         /* [4107] OBJ_setct_AuthReqTBE */
0x67,0x2A,0x00,0x35,                         /* [4111] OBJ_setct_AuthResTBE */
0x67,0x2A,0x00,0x36,                         /* [4115] OBJ_setct_AuthResTBEX */
0x67,0x2A,0x00,0x37,                         /* [4119] OBJ_setct_AuthTokenTBE */
0x67,0x2A,0x00,0x38,                         /* [4123] OBJ_setct_CapTokenTBE */
0x67,0x2A,0x00,0x39,                         /* [4127] OBJ_setct_CapTokenTBEX */
0x67,0x2A,0x00,0x3A,                         /* [4131] OBJ_setct_AcqCardCodeMsgTBE */
0x67,0x2A,0x00,0x3B,                         /* [4135] OBJ_setct_AuthRevReqTBE */
0x67,0x2A,0x00,0x3C,                         /* [4139] OBJ_setct_AuthRevResTBE */
0x67,0x2A,0x00,0x3D,                         /* [4143] OBJ_setct_AuthRevResTBEB */
0x67,0x2A,0x00,0x3E,                         /* [4147] OBJ_setct_CapReqTBE */
0x67,0x2A,0x00,0x3F,                         /* [4151] OBJ_setct_CapReqTBEX */
0x67,0x2A,0x00,0x40,                         /* [4155] OBJ_setct_CapResTBE */
0x67,0x2A,0x00,0x41,                         /* [4159] OBJ_setct_CapRevReqTBE */
0x67,0x2A,0x00,0x42,                         /* [4163] OBJ_setct_CapRevReqTBEX */
0x67,0x2A,0x00,0x43,                         /* [4167] OBJ_setct_CapRevResTBE */
0x67,0x2A,0x00,0x44,                         /* [4171] OBJ_setct_CredReqTBE */
0x67,0x2A,0x00,0x45,                         /* [4175] OBJ_setct_CredReqTBEX */
0x67,0x2A,0x00,0x46,                         /* [4179] OBJ_setct_CredResTBE */
0x67,0x2A,0x00,0x47,                         /* [4183] OBJ_setct_CredRevReqTBE */
0x67,0x2A,0x00,0x48,                         /* [4187] OBJ_setct_CredRevReqTBEX */
0x67,0x2A,0x00,0x49,                         /* [4191] OBJ_setct_CredRevResTBE */
0x67,0x2A,0x00,0x4A,                         /* [4195] OBJ_setct_BatchAdminReqTBE */
0x67,0x2A,0x00,0x4B,                         /* [4199] OBJ_setct_BatchAdminResTBE */
0x67,0x2A,0x00,0x4C,                         /* [4203] OBJ_setct_RegFormReqTBE */
0x67,0x2A,0x00,0x4D,                         /* [4207] OBJ_setct_CertReqTBE */
0x67,0x2A,0x00,0x4E,                         /* [4211] OBJ_setct_CertReqTBEX */
0x67,0x2A,0x00,0x4F,                         /* [4215] OBJ_setct_CertResTBE */
0x67,0x2A,0x00,0x50,                         /* [4219] OBJ_setct_CRLNotificationTBS */
0x67,0x2A,0x00,0x51,                         /* [4223] OBJ_setct_CRLNotificationResTBS */
0x67,0x2A,0x00,0x52,                         /* [4227] OBJ_setct_BCIDistributionTBS */
0x67,0x2A,0x01,0x01,                         /* [4231] OBJ_setext_genCrypt */
0x67,0x2A,0x01,0x03,                         /* [4235] OBJ_setext_miAuth */
0x67,0x2A,0x01,0x04,                         /* [4239] OBJ_setext_pinSecure */
0x67,0x2A,0x01,0x05,                         /* [4243] OBJ_setext_pinAny */
0x67,0x2A,0x01,0x07,                         /* [4247] OBJ_setext_track2 */
0x67,0x2A,0x01,0x08,                         /* [4251] OBJ_setext_cv */
0x67,0x2A,0x05,0x00,                         /* [4255] OBJ_set_policy_root */
0x67,0x2A,0x07,0x00,                         /* [4259] OBJ_setCext_hashedRoot */
0x67,0x2A,0x07,0x01,                         /* [4263] OBJ_setCext_certType */
0x67,0x2A,0x07,0x02,                         /* [4267] OBJ_setCext_merchData */
0x67,0x2A,0x07,0x03,                         /* [4271] OBJ_setCext_cCertRequired */
0x67,0x2A,0x07,0x04,                         /* [4275] OBJ_setCext_tunneling */
0x67,0x2A,0x07,0x05,                         /* [4279] OBJ_setCext_setExt */
0x67,0x2A,0x07,0x06,                         /* [4283] OBJ_setCext_setQualf */
0x67,0x2A,0x07,0x07,                         /* [4287] OBJ_setCext_PGWYcapabilities */
0x67,0x2A,0x07,0x08,                         /* [4291] OBJ_setCext_TokenIdentifier */
0x67,0x2A,0x07,0x09,                         /* [4295] OBJ_setCext_Track2Data */
0x67,0x2A,0x07,0x0A,                         /* [4299] OBJ_setCext_TokenType */
0x67,0x2A,0x07,0x0B,                         /* [4303] OBJ_setCext_IssuerCapabilities */
0x67,0x2A,0x03,0x00,                         /* [4307] OBJ_setAttr_Cert */
0x67,0x2A,0x03,0x01,                         /* [4311] OBJ_setAttr_PGWYcap */
0x67,0x2A,0x03,0x02,                         /* [4315] OBJ_setAttr_TokenType */
0x67,0x2A,0x03,0x03,                         /* [4319] OBJ_setAttr_IssCap */
0x67,0x2A,0x03,0x00,0x00,                    /* [4323] OBJ_set_rootKeyThumb */
0x67,0x2A,0x03,0x00,0x01,                    /* [4328] OBJ_set_addPolicy */
0x67,0x2A,0x03,0x02,0x01,                    /* [4333] OBJ_setAttr_Token_EMV */
0x67,0x2A,0x03,0x02,0x02,                    /* [4338] OBJ_setAttr_Token_B0Prime */
0x67,0x2A,0x03,0x03,0x03,                    /* [4343] OBJ_setAttr_IssCap_CVM */
0x67,0x2A,0x03,0x03,0x04,                    /* [4348] OBJ_setAttr_IssCap_T2 */
0x67,0x2A,0x03,0x03,0x05,                    /* [4353] OBJ_setAttr_IssCap_Sig */
0x67,0x2A,0x03,0x03,0x03,0x01,               /* [4358] OBJ_setAttr_GenCryptgrm */
0x67,0x2A,0x03,0x03,0x04,0x01,               /* [4364] OBJ_setAttr_T2Enc */
0x67,0x2A,0x03,0x03,0x04,0x02,               /* [4370] OBJ_setAttr_T2cleartxt */
0x67,0x2A,0x03,0x03,0x05,0x01,               /* [4376] OBJ_setAttr_TokICCsig */
0x67,0x2A,0x03,0x03,0x05,0x02,               /* [4382] OBJ_setAttr_SecDevSig */
0x67,0x2A,0x08,0x01,                         /* [4388] OBJ_set_brand_IATA_ATA */
0x67,0x2A,0x08,0x1E,                         /* [4392] OBJ_set_brand_Diners */
0x67,0x2A,0x08,0x22,                         /* [4396] OBJ_set_brand_AmericanExpress */
0x67,0x2A,0x08,0x23,                         /* [4400] OBJ_set_brand_JCB */
0x67,0x2A,0x08,0x04,                         /* [4404] OBJ_set_brand_Visa */
0x67,0x2A,0x08,0x05,                         /* [4408] OBJ_set_brand_MasterCard */
0x67,0x2A,0x08,0xAE,0x7B,                    /* [4412] OBJ_set_brand_Novus */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x03,0x0A,     /* [4417] OBJ_des_cdmf */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x01,0x06,/* [4425] OBJ_rsaOAEPEncryptionSET */
0x00,                                        /* [4434] OBJ_itu_t */
0x50,                                        /* [4435] OBJ_joint_iso_itu_t */
0x67,                                        /* [4436] OBJ_international_organizations */
0x2B,0x06,0x01,0x04,0x01,0x82,0x37,0x14,0x02,0x02,/* [4437] OBJ_ms_smartcard_login */
0x2B,0x06,0x01,0x04,0x01,0x82,0x37,0x14,0x02,0x03,/* [4447] OBJ_ms_upn */
0x55,0x04,0x09,                              /* [4457] OBJ_streetAddress */
0x55,0x04,0x11,                              /* [4460] OBJ_postalCode */
0x2B,0x06,0x01,0x05,0x05,0x07,0x15,          /* [4463] OBJ_id_ppl */
0x2B,0x06,0x01,0x05,0x05,0x07,0x01,0x0E,     /* [4470] OBJ_proxyCertInfo */
0x2B,0x06,0x01,0x05,0x05,0x07,0x15,0x00,     /* [4478] OBJ_id_ppl_anyLanguage */
0x2B,0x06,0x01,0x05,0x05,0x07,0x15,0x01,     /* [4486] OBJ_id_ppl_inheritAll */
0x55,0x1D,0x1E,                              /* [4494] OBJ_name_constraints */
0x2B,0x06,0x01,0x05,0x05,0x07,0x15,0x02,     /* [4497] OBJ_Independent */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x01,0x0B,/* [4505] OBJ_sha256WithRSAEncryption */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x01,0x0C,/* [4514] OBJ_sha384WithRSAEncryption */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x01,0x0D,/* [4523] OBJ_sha512WithRSAEncryption */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x01,0x0E,/* [4532] OBJ_sha224WithRSAEncryption */
0x60,0x86,0x48,0x01,0x65,0x03,0x04,0x02,0x01,/* [4541] OBJ_sha256 */
0x60,0x86,0x48,0x01,0x65,0x03,0x04,0x02,0x02,/* [4550] OBJ_sha384 */
0x60,0x86,0x48,0x01,0x65,0x03,0x04,0x02,0x03,/* [4559] OBJ_sha512 */
0x60,0x86,0x48,0x01,0x65,0x03,0x04,0x02,0x04,/* [4568] OBJ_sha224 */
0x2B,                                        /* [4577] OBJ_identified_organization */
0x2B,0x81,0x04,                              /* [4578] OBJ_certicom_arc */
0x67,0x2B,                                   /* [4581] OBJ_wap */
0x67,0x2B,0x0D,                              /* [4583] OBJ_wap_wsg */
0x2A,0x86,0x48,0xCE,0x3D,0x01,0x02,0x03,     /* [4586] OBJ_X9_62_id_characteristic_two_basis */
0x2A,0x86,0x48,0xCE,0x3D,0x01,0x02,0x03,0x01,/* [4594] OBJ_X9_62_onBasis */
0x2A,0x86,0x48,0xCE,0x3D,0x01,0x02,0x03,0x02,/* [4603] OBJ_X9_62_tpBasis */
0x2A,0x86,0x48,0xCE,0x3D,0x01,0x02,0x03,0x03,/* [4612] OBJ_X9_62_ppBasis */
0x2A,0x86,0x48,0xCE,0x3D,0x03,0x00,0x01,     /* [4621] OBJ_X9_62_c2pnb163v1 */
0x2A,0x86,0x48,0xCE,0x3D,0x03,0x00,0x02,     /* [4629] OBJ_X9_62_c2pnb163v2 */
0x2A,0x86,0x48,0xCE,0x3D,0x03,0x00,0x03,     /* [4637] OBJ_X9_62_c2pnb163v3 */
0x2A,0x86,0x48,0xCE,0x3D,0x03,0x00,0x04,     /* [4645] OBJ_X9_62_c2pnb176v1 */
0x2A,0x86,0x48,0xCE,0x3D,0x03,0x00,0x05,     /* [4653] OBJ_X9_62_c2tnb191v1 */
0x2A,0x86,0x48,0xCE,0x3D,0x03,0x00,0x06,     /* [4661] OBJ_X9_62_c2tnb191v2 */
0x2A,0x86,0x48,0xCE,0x3D,0x03,0x00,0x07,     /* [4669] OBJ_X9_62_c2tnb191v3 */
0x2A,0x86,0x48,0xCE,0x3D,0x03,0x00,0x08,     /* [4677] OBJ_X9_62_c2onb191v4 */
0x2A,0x86,0x48,0xCE,0x3D,0x03,0x00,0x09,     /* [4685] OBJ_X9_62_c2onb191v5 */
0x2A,0x86,0x48,0xCE,0x3D,0x03,0x00,0x0A,     /* [4693] OBJ_X9_62_c2pnb208w1 */
0x2A,0x86,0x48,0xCE,0x3D,0x03,0x00,0x0B,     /* [4701] OBJ_X9_62_c2tnb239v1 */
0x2A,0x86,0x48,0xCE,0x3D,0x03,0x00,0x0C,     /* [4709] OBJ_X9_62_c2tnb239v2 */
0x2A,0x86,0x48,0xCE,0x3D,0x03,0x00,0x0D,     /* [4717] OBJ_X9_62_c2tnb239v3 */
0x2A,0x86,0x48,0xCE,0x3D,0x03,0x00,0x0E,     /* [4725] OBJ_X9_62_c2onb239v4 */
0x2A,0x86,0x48,0xCE,0x3D,0x03,0x00,0x0F,     /* [4733] OBJ_X9_62_c2onb239v5 */
0x2A,0x86,0x48,0xCE,0x3D,0x03,0x00,0x10,     /* [4741] OBJ_X9_62_c2pnb272w1 */
0x2A,0x86,0x48,0xCE,0x3D,0x03,0x00,0x11,     /* [4749] OBJ_X9_62_c2pnb304w1 */
0x2A,0x86,0x48,0xCE,0x3D,0x03,0x00,0x12,     /* [4757] OBJ_X9_62_c2tnb359v1 */
0x2A,0x86,0x48,0xCE,0x3D,0x03,0x00,0x13,     /* [4765] OBJ_X9_62_c2pnb368w1 */
0x2A,0x86,0x48,0xCE,0x3D,0x03,0x00,0x14,     /* [4773] OBJ_X9_62_c2tnb431r1 */
0x2B,0x81,0x04,0x00,0x06,                    /* [4781] OBJ_secp112r1 */
0x2B,0x81,0x04,0x00,0x07,                    /* [4786] OBJ_secp112r2 */
0x2B,0x81,0x04,0x00,0x1C,                    /* [4791] OBJ_secp128r1 */
0x2B,0x81,0x04,0x00,0x1D,                    /* [4796] OBJ_secp128r2 */
0x2B,0x81,0x04,0x00,0x09,                    /* [4801] OBJ_secp160k1 */
0x2B,0x81,0x04,0x00,0x08,                    /* [4806] OBJ_secp160r1 */
0x2B,0x81,0x04,0x00,0x1E,                    /* [4811] OBJ_secp160r2 */
0x2B,0x81,0x04,0x00,0x1F,                    /* [4816] OBJ_secp192k1 */
0x2B,0x81,0x04,0x00,0x20,                    /* [4821] OBJ_secp224k1 */
0x2B,0x81,0x04,0x00,0x21,                    /* [4826] OBJ_secp224r1 */
0x2B,0x81,0x04,0x00,0x0A,                    /* [4831] OBJ_secp256k1 */
0x2B,0x81,0x04,0x00,0x22,                    /* [4836] OBJ_secp384r1 */
0x2B,0x81,0x04,0x00,0x23,                    /* [4841] OBJ_secp521r1 */
0x2B,0x81,0x04,0x00,0x04,                    /* [4846] OBJ_sect113r1 */
0x2B,0x81,0x04,0x00,0x05,                    /* [4851] OBJ_sect113r2 */
0x2B,0x81,0x04,0x00,0x16,                    /* [4856] OBJ_sect131r1 */
0x2B,0x81,0x04,0x00,0x17,                    /* [4861] OBJ_sect131r2 */
0x2B,0x81,0x04,0x00,0x01,                    /* [4866] OBJ_sect163k1 */
0x2B,0x81,0x04,0x00,0x02,                    /* [4871] OBJ_sect163r1 */
0x2B,0x81,0x04,0x00,0x0F,                    /* [4876] OBJ_sect163r2 */
0x2B,0x81,0x04,0x00,0x18,                    /* [4881] OBJ_sect193r1 */
0x2B,0x81,0x04,0x00,0x19,                    /* [4886] OBJ_sect193r2 */
0x2B,0x81,0x04,0x00,0x1A,                    /* [4891] OBJ_sect233k1 */
0x2B,0x81,0x04,0x00,0x1B,                    /* [4896] OBJ_sect233r1 */
0x2B,0x81,0x04,0x00,0x03,                    /* [4901] OBJ_sect239k1 */
0x2B,0x81,0x04,0x00,0x10,                    /* [4906] OBJ_sect283k1 */
0x2B,0x81,0x04,0x00,0x11,                    /* [4911] OBJ_sect283r1 */
0x2B,0x81,0x04,0x00,0x24,                    /* [4916] OBJ_sect409k1 */
0x2B,0x81,0x04,0x00,0x25,                    /* [4921] OBJ_sect409r1 */
0x2B,0x81,0x04,0x00,0x26,                    /* [4926] OBJ_sect571k1 */
0x2B,0x81,0x04,0x00,0x27,                    /* [4931] OBJ_sect571r1 */
0x67,0x2B,0x0D,0x04,0x01,                    /* [4936] OBJ_wap_wsg_idm_ecid_wtls1 */
0x67,0x2B,0x0D,0x04,0x03,                    /* [4941] OBJ_wap_wsg_idm_ecid_wtls3 */
0x67,0x2B,0x0D,0x04,0x04,                    /* [4946] OBJ_wap_wsg_idm_ecid_wtls4 */
0x67,0x2B,0x0D,0x04,0x05,                    /* [4951] OBJ_wap_wsg_idm_ecid_wtls5 */
0x67,0x2B,0x0D,0x04,0x06,                    /* [4956] OBJ_wap_wsg_idm_ecid_wtls6 */
0x67,0x2B,0x0D,0x04,0x07,                    /* [4961] OBJ_wap_wsg_idm_ecid_wtls7 */
0x67,0x2B,0x0D,0x04,0x08,                    /* [4966] OBJ_wap_wsg_idm_ecid_wtls8 */
0x67,0x2B,0x0D,0x04,0x09,                    /* [4971] OBJ_wap_wsg_idm_ecid_wtls9 */
0x67,0x2B,0x0D,0x04,0x0A,                    /* [4976] OBJ_wap_wsg_idm_ecid_wtls10 */
0x67,0x2B,0x0D,0x04,0x0B,                    /* [4981] OBJ_wap_wsg_idm_ecid_wtls11 */
0x67,0x2B,0x0D,0x04,0x0C,                    /* [4986] OBJ_wap_wsg_idm_ecid_wtls12 */
0x55,0x1D,0x20,0x00,                         /* [4991] OBJ_any_policy */
0x55,0x1D,0x21,                              /* [4995] OBJ_policy_mappings */
0x55,0x1D,0x36,                              /* [4998] OBJ_inhibit_any_policy */
0x2A,0x83,0x08,0x8C,0x9A,0x4B,0x3D,0x01,0x01,0x01,0x02,/* [5001] OBJ_camellia_128_cbc */
0x2A,0x83,0x08,0x8C,0x9A,0x4B,0x3D,0x01,0x01,0x01,0x03,/* [5012] OBJ_camellia_192_cbc */
0x2A,0x83,0x08,0x8C,0x9A,0x4B,0x3D,0x01,0x01,0x01,0x04,/* [5023] OBJ_camellia_256_cbc */
0x03,0xA2,0x31,0x05,0x03,0x01,0x09,0x01,     /* [5034] OBJ_camellia_128_ecb */
0x03,0xA2,0x31,0x05,0x03,0x01,0x09,0x15,     /* [5042] OBJ_camellia_192_ecb */
0x03,0xA2,0x31,0x05,0x03,0x01,0x09,0x29,     /* [5050] OBJ_camellia_256_ecb */
0x03,0xA2,0x31,0x05,0x03,0x01,0x09,0x04,     /* [5058] OBJ_camellia_128_cfb128 */
0x03,0xA2,0x31,0x05,0x03,0x01,0x09,0x18,     /* [5066] OBJ_camellia_192_cfb128 */
0x03,0xA2,0x31,0x05,0x03,0x01,0x09,0x2C,     /* [5074] OBJ_camellia_256_cfb128 */
0x03,0xA2,0x31,0x05,0x03,0x01,0x09,0x03,     /* [5082] OBJ_camellia_128_ofb128 */
0x03,0xA2,0x31,0x05,0x03,0x01,0x09,0x17,     /* [5090] OBJ_camellia_192_ofb128 */
0x03,0xA2,0x31,0x05,0x03,0x01,0x09,0x2B,     /* [5098] OBJ_camellia_256_ofb128 */
0x55,0x1D,0x09,                              /* [5106] OBJ_subject_directory_attributes */
0x55,0x1D,0x1C,                              /* [5109] OBJ_issuing_distribution_point */
0x55,0x1D,0x1D,                              /* [5112] OBJ_certificate_issuer */
0x2A,0x83,0x1A,0x8C,0x9A,0x44,               /* [5115] OBJ_kisa */
0x2A,0x83,0x1A,0x8C,0x9A,0x44,0x01,0x03,     /* [5121] OBJ_seed_ecb */
0x2A,0x83,0x1A,0x8C,0x9A,0x44,0x01,0x04,     /* [5129] OBJ_seed_cbc */
0x2A,0x83,0x1A,0x8C,0x9A,0x44,0x01,0x06,     /* [5137] OBJ_seed_ofb128 */
0x2A,0x83,0x1A,0x8C,0x9A,0x44,0x01,0x05,     /* [5145] OBJ_seed_cfb128 */
};

static ASN1_OBJECT nid_objs[NUM_NID]={
{"UNDEF","undefined",NID_undef,1,&(lvalues[0]),0},
{"rsadsi","RSA Data Security, Inc.",NID_rsadsi,6,&(lvalues[1]),0},
{"pkcs","RSA Data Security, Inc. PKCS",NID_pkcs,7,&(lvalues[7]),0},
{"MD2","md2",NID_md2,8,&(lvalues[14]),0},
{"MD5","md5",NID_md5,8,&(lvalues[22]),0},
{"RC4","rc4",NID_rc4,8,&(lvalues[30]),0},
{"rsaEncryption","rsaEncryption",NID_rsaEncryption,9,&(lvalues[38]),0},
{"RSA-MD2","md2WithRSAEncryption",NID_md2WithRSAEncryption,9,
	&(lvalues[47]),0},
{"RSA-MD5","md5WithRSAEncryption",NID_md5WithRSAEncryption,9,
	&(lvalues[56]),0},
{"PBE-MD2-DES","pbeWithMD2AndDES-CBC",NID_pbeWithMD2AndDES_CBC,9,
	&(lvalues[65]),0},
{"PBE-MD5-DES","pbeWithMD5AndDES-CBC",NID_pbeWithMD5AndDES_CBC,9,
	&(lvalues[74]),0},
{"X500","directory services (X.500)",NID_X500,1,&(lvalues[83]),0},
{"X509","X509",NID_X509,2,&(lvalues[84]),0},
{"CN","commonName",NID_commonName,3,&(lvalues[86]),0},
{"C","countryName",NID_countryName,3,&(lvalues[89]),0},
{"L","localityName",NID_localityName,3,&(lvalues[92]),0},
{"ST","stateOrProvinceName",NID_stateOrProvinceName,3,&(lvalues[95]),0},
{"O","organizationName",NID_organizationName,3,&(lvalues[98]),0},
{"OU","organizationalUnitName",NID_organizationalUnitName,3,
	&(lvalues[101]),0},
{"RSA","rsa",NID_rsa,4,&(lvalues[104]),0},
{"pkcs7","pkcs7",NID_pkcs7,8,&(lvalues[108]),0},
{"pkcs7-data","pkcs7-data",NID_pkcs7_data,9,&(lvalues[116]),0},
{"pkcs7-signedData","pkcs7-signedData",NID_pkcs7_signed,9,
	&(lvalues[125]),0},
{"pkcs7-envelopedData","pkcs7-envelopedData",NID_pkcs7_enveloped,9,
	&(lvalues[134]),0},
{"pkcs7-signedAndEnvelopedData","pkcs7-signedAndEnvelopedData",
	NID_pkcs7_signedAndEnveloped,9,&(lvalues[143]),0},
{"pkcs7-digestData","pkcs7-digestData",NID_pkcs7_digest,9,
	&(lvalues[152]),0},
{"pkcs7-encryptedData","pkcs7-encryptedData",NID_pkcs7_encrypted,9,
	&(lvalues[161]),0},
{"pkcs3","pkcs3",NID_pkcs3,8,&(lvalues[170]),0},
{"dhKeyAgreement","dhKeyAgreement",NID_dhKeyAgreement,9,
	&(lvalues[178]),0},
{"DES-ECB","des-ecb",NID_des_ecb,5,&(lvalues[187]),0},
{"DES-CFB","des-cfb",NID_des_cfb64,5,&(lvalues[192]),0},
{"DES-CBC","des-cbc",NID_des_cbc,5,&(lvalues[197]),0},
{"DES-EDE","des-ede",NID_des_ede_ecb,5,&(lvalues[202]),0},
{"DES-EDE3","des-ede3",NID_des_ede3_ecb,0,NULL,0},
{"IDEA-CBC","idea-cbc",NID_idea_cbc,11,&(lvalues[207]),0},
{"IDEA-CFB","idea-cfb",NID_idea_cfb64,0,NULL,0},
{"IDEA-ECB","idea-ecb",NID_idea_ecb,0,NULL,0},
{"RC2-CBC","rc2-cbc",NID_rc2_cbc,8,&(lvalues[218]),0},
{"RC2-ECB","rc2-ecb",NID_rc2_ecb,0,NULL,0},
{"RC2-CFB","rc2-cfb",NID_rc2_cfb64,0,NULL,0},
{"RC2-OFB","rc2-ofb",NID_rc2_ofb64,0,NULL,0},
{"SHA","sha",NID_sha,5,&(lvalues[226]),0},
{"RSA-SHA","shaWithRSAEncryption",NID_shaWithRSAEncryption,5,
	&(lvalues[231]),0},
{"DES-EDE-CBC","des-ede-cbc",NID_des_ede_cbc,0,NULL,0},
{"DES-EDE3-CBC","des-ede3-cbc",NID_des_ede3_cbc,8,&(lvalues[236]),0},
{"DES-OFB","des-ofb",NID_des_ofb64,5,&(lvalues[244]),0},
{"IDEA-OFB","idea-ofb",NID_idea_ofb64,0,NULL,0},
{"pkcs9","pkcs9",NID_pkcs9,8,&(lvalues[249]),0},
{"emailAddress","emailAddress",NID_pkcs9_emailAddress,9,
	&(lvalues[257]),0},
{"unstructuredName","unstructuredName",NID_pkcs9_unstructuredName,9,
	&(lvalues[266]),0},
{"contentType","contentType",NID_pkcs9_contentType,9,&(lvalues[275]),0},
{"messageDigest","messageDigest",NID_pkcs9_messageDigest,9,
	&(lvalues[284]),0},
{"signingTime","signingTime",NID_pkcs9_signingTime,9,&(lvalues[293]),0},
{"countersignature","countersignature",NID_pkcs9_countersignature,9,
	&(lvalues[302]),0},
{"challengePassword","challengePassword",NID_pkcs9_challengePassword,
	9,&(lvalues[311]),0},
{"unstructuredAddress","unstructuredAddress",
	NID_pkcs9_unstructuredAddress,9,&(lvalues[320]),0},
{"extendedCertificateAttributes","extendedCertificateAttributes",
	NID_pkcs9_extCertAttributes,9,&(lvalues[329]),0},
{"Netscape","Netscape Communications Corp.",NID_netscape,7,
	&(lvalues[338]),0},
{"nsCertExt","Netscape Certificate Extension",
	NID_netscape_cert_extension,8,&(lvalues[345]),0},
{"nsDataType","Netscape Data Type",NID_netscape_data_type,8,
	&(lvalues[353]),0},
{"DES-EDE-CFB","des-ede-cfb",NID_des_ede_cfb64,0,NULL,0},
{"DES-EDE3-CFB","des-ede3-cfb",NID_des_ede3_cfb64,0,NULL,0},
{"DES-EDE-OFB","des-ede-ofb",NID_des_ede_ofb64,0,NULL,0},
{"DES-EDE3-OFB","des-ede3-ofb",NID_des_ede3_ofb64,0,NULL,0},
{"SHA1","sha1",NID_sha1,5,&(lvalues[361]),0},
{"RSA-SHA1","sha1WithRSAEncryption",NID_sha1WithRSAEncryption,9,
	&(lvalues[366]),0},
{"DSA-SHA","dsaWithSHA",NID_dsaWithSHA,5,&(lvalues[375]),0},
{"DSA-old","dsaEncryption-old",NID_dsa_2,5,&(lvalues[380]),0},
{"PBE-SHA1-RC2-64","pbeWithSHA1AndRC2-CBC",NID_pbeWithSHA1AndRC2_CBC,
	9,&(lvalues[385]),0},
{"PBKDF2","PBKDF2",NID_id_pbkdf2,9,&(lvalues[394]),0},
{"DSA-SHA1-old","dsaWithSHA1-old",NID_dsaWithSHA1_2,5,&(lvalues[403]),0},
{"nsCertType","Netscape Cert Type",NID_netscape_cert_type,9,
	&(lvalues[408]),0},
{"nsBaseUrl","Netscape Base Url",NID_netscape_base_url,9,
	&(lvalues[417]),0},
{"nsRevocationUrl","Netscape Revocation Url",
	NID_netscape_revocation_url,9,&(lvalues[426]),0},
{"nsCaRevocationUrl","Netscape CA Revocation Url",
	NID_netscape_ca_revocation_url,9,&(lvalues[435]),0},
{"nsRenewalUrl","Netscape Renewal Url",NID_netscape_renewal_url,9,
	&(lvalues[444]),0},
{"nsCaPolicyUrl","Netscape CA Policy Url",NID_netscape_ca_policy_url,
	9,&(lvalues[453]),0},
{"nsSslServerName","Netscape SSL Server Name",
	NID_netscape_ssl_server_name,9,&(lvalues[462]),0},
{"nsComment","Netscape Comment",NID_netscape_comment,9,&(lvalues[471]),0},
{"nsCertSequence","Netscape Certificate Sequence",
	NID_netscape_cert_sequence,9,&(lvalues[480]),0},
{"DESX-CBC","desx-cbc",NID_desx_cbc,0,NULL,0},
{"id-ce","id-ce",NID_id_ce,2,&(lvalues[489]),0},
{"subjectKeyIdentifier","X509v3 Subject Key Identifier",
	NID_subject_key_identifier,3,&(lvalues[491]),0},
{"keyUsage","X509v3 Key Usage",NID_key_usage,3,&(lvalues[494]),0},
{"privateKeyUsagePeriod","X509v3 Private Key Usage Period",
	NID_private_key_usage_period,3,&(lvalues[497]),0},
{"subjectAltName","X509v3 Subject Alternative Name",
	NID_subject_alt_name,3,&(lvalues[500]),0},
{"issuerAltName","X509v3 Issuer Alternative Name",NID_issuer_alt_name,
	3,&(lvalues[503]),0},
{"basicConstraints","X509v3 Basic Constraints",NID_basic_constraints,
	3,&(lvalues[506]),0},
{"crlNumber","X509v3 CRL Number",NID_crl_number,3,&(lvalues[509]),0},
{"certificatePolicies","X509v3 Certificate Policies",
	NID_certificate_policies,3,&(lvalues[512]),0},
{"authorityKeyIdentifier","X509v3 Authority Key Identifier",
	NID_authority_key_identifier,3,&(lvalues[515]),0},
{"BF-CBC","bf-cbc",NID_bf_cbc,9,&(lvalues[518]),0},
{"BF-ECB","bf-ecb",NID_bf_ecb,0,NULL,0},
{"BF-CFB","bf-cfb",NID_bf_cfb64,0,NULL,0},
{"BF-OFB","bf-ofb",NID_bf_ofb64,0,NULL,0},
{"MDC2","mdc2",NID_mdc2,4,&(lvalues[527]),0},
{"RSA-MDC2","mdc2WithRSA",NID_mdc2WithRSA,4,&(lvalues[531]),0},
{"RC4-40","rc4-40",NID_rc4_40,0,NULL,0},
{"RC2-40-CBC","rc2-40-cbc",NID_rc2_40_cbc,0,NULL,0},
{"GN","givenName",NID_givenName,3,&(lvalues[535]),0},
{"SN","surname",NID_surname,3,&(lvalues[538]),0},
{"initials","initials",NID_initials,3,&(lvalues[541]),0},
{NULL,NULL,NID_undef,0,NULL,0},
{"crlDistributionPoints","X509v3 CRL Distribution Points",
	NID_crl_distribution_points,3,&(lvalues[544]),0},
{"RSA-NP-MD5","md5WithRSA",NID_md5WithRSA,5,&(lvalues[547]),0},
{"serialNumber","serialNumber",NID_serialNumber,3,&(lvalues[552]),0},
{"title","title",NID_title,3,&(lvalues[555]),0},
{"description","description",NID_description,3,&(lvalues[558]),0},
{"CAST5-CBC","cast5-cbc",NID_cast5_cbc,9,&(lvalues[561]),0},
{"CAST5-ECB","cast5-ecb",NID_cast5_ecb,0,NULL,0},
{"CAST5-CFB","cast5-cfb",NID_cast5_cfb64,0,NULL,0},
{"CAST5-OFB","cast5-ofb",NID_cast5_ofb64,0,NULL,0},
{"pbeWithMD5AndCast5CBC","pbeWithMD5AndCast5CBC",
	NID_pbeWithMD5AndCast5_CBC,9,&(lvalues[570]),0},
{"DSA-SHA1","dsaWithSHA1",NID_dsaWithSHA1,7,&(lvalues[579]),0},
{"MD5-SHA1","md5-sha1",NID_md5_sha1,0,NULL,0},
{"RSA-SHA1-2","sha1WithRSA",NID_sha1WithRSA,5,&(lvalues[586]),0},
{"DSA","dsaEncryption",NID_dsa,7,&(lvalues[591]),0},
{"RIPEMD160","ripemd160",NID_ripemd160,5,&(lvalues[598]),0},
{NULL,NULL,NID_undef,0,NULL,0},
{"RSA-RIPEMD160","ripemd160WithRSA",NID_ripemd160WithRSA,6,
	&(lvalues[603]),0},
{"RC5-CBC","rc5-cbc",NID_rc5_cbc,8,&(lvalues[609]),0},
{"RC5-ECB","rc5-ecb",NID_rc5_ecb,0,NULL,0},
{"RC5-CFB","rc5-cfb",NID_rc5_cfb64,0,NULL,0},
{"RC5-OFB","rc5-ofb",NID_rc5_ofb64,0,NULL,0},
{"RLE","run length compression",NID_rle_compression,6,&(lvalues[617]),0},
{"ZLIB","zlib compression",NID_zlib_compression,6,&(lvalues[623]),0},
{"extendedKeyUsage","X509v3 Extended Key Usage",NID_ext_key_usage,3,
	&(lvalues[629]),0},
{"PKIX","PKIX",NID_id_pkix,6,&(lvalues[632]),0},
{"id-kp","id-kp",NID_id_kp,7,&(lvalues[638]),0},
{"serverAuth","TLS Web Server Authentication",NID_server_auth,8,
	&(lvalues[645]),0},
{"clientAuth","TLS Web Client Authentication",NID_client_auth,8,
	&(lvalues[653]),0},
{"codeSigning","Code Signing",NID_code_sign,8,&(lvalues[661]),0},
{"emailProtection","E-mail Protection",NID_email_protect,8,
	&(lvalues[669]),0},
{"timeStamping","Time Stamping",NID_time_stamp,8,&(lvalues[677]),0},
{"msCodeInd","Microsoft Individual Code Signing",NID_ms_code_ind,10,
	&(lvalues[685]),0},
{"msCodeCom","Microsoft Commercial Code Signing",NID_ms_code_com,10,
	&(lvalues[695]),0},
{"msCTLSign","Microsoft Trust List Signing",NID_ms_ctl_sign,10,
	&(lvalues[705]),0},
{"msSGC","Microsoft Server Gated Crypto",NID_ms_sgc,10,&(lvalues[715]),0},
{"msEFS","Microsoft Encrypted File System",NID_ms_efs,10,
	&(lvalues[725]),0},
{"nsSGC","Netscape Server Gated Crypto",NID_ns_sgc,9,&(lvalues[735]),0},
{"deltaCRL","X509v3 Delta CRL Indicator",NID_delta_crl,3,
	&(lvalues[744]),0},
{"CRLReason","X509v3 CRL Reason Code",NID_crl_reason,3,&(lvalues[747]),0},
{"invalidityDate","Invalidity Date",NID_invalidity_date,3,
	&(lvalues[750]),0},
{"SXNetID","Strong Extranet ID",NID_sxnet,5,&(lvalues[753]),0},
{"PBE-SHA1-RC4-128","pbeWithSHA1And128BitRC4",
	NID_pbe_WithSHA1And128BitRC4,10,&(lvalues[758]),0},
{"PBE-SHA1-RC4-40","pbeWithSHA1And40BitRC4",
	NID_pbe_WithSHA1And40BitRC4,10,&(lvalues[768]),0},
{"PBE-SHA1-3DES","pbeWithSHA1And3-KeyTripleDES-CBC",
	NID_pbe_WithSHA1And3_Key_TripleDES_CBC,10,&(lvalues[778]),0},
{"PBE-SHA1-2DES","pbeWithSHA1And2-KeyTripleDES-CBC",
	NID_pbe_WithSHA1And2_Key_TripleDES_CBC,10,&(lvalues[788]),0},
{"PBE-SHA1-RC2-128","pbeWithSHA1And128BitRC2-CBC",
	NID_pbe_WithSHA1And128BitRC2_CBC,10,&(lvalues[798]),0},
{"PBE-SHA1-RC2-40","pbeWithSHA1And40BitRC2-CBC",
	NID_pbe_WithSHA1And40BitRC2_CBC,10,&(lvalues[808]),0},
{"keyBag","keyBag",NID_keyBag,11,&(lvalues[818]),0},
{"pkcs8ShroudedKeyBag","pkcs8ShroudedKeyBag",NID_pkcs8ShroudedKeyBag,
	11,&(lvalues[829]),0},
{"certBag","certBag",NID_certBag,11,&(lvalues[840]),0},
{"crlBag","crlBag",NID_crlBag,11,&(lvalues[851]),0},
{"secretBag","secretBag",NID_secretBag,11,&(lvalues[862]),0},
{"safeContentsBag","safeContentsBag",NID_safeContentsBag,11,
	&(lvalues[873]),0},
{"friendlyName","friendlyName",NID_friendlyName,9,&(lvalues[884]),0},
{"localKeyID","localKeyID",NID_localKeyID,9,&(lvalues[893]),0},
{"x509Certificate","x509Certificate",NID_x509Certificate,10,
	&(lvalues[902]),0},
{"sdsiCertificate","sdsiCertificate",NID_sdsiCertificate,10,
	&(lvalues[912]),0},
{"x509Crl","x509Crl",NID_x509Crl,10,&(lvalues[922]),0},
{"PBES2","PBES2",NID_pbes2,9,&(lvalues[932]),0},
{"PBMAC1","PBMAC1",NID_pbmac1,9,&(lvalues[941]),0},
{"hmacWithSHA1","hmacWithSHA1",NID_hmacWithSHA1,8,&(lvalues[950]),0},
{"id-qt-cps","Policy Qualifier CPS",NID_id_qt_cps,8,&(lvalues[958]),0},
{"id-qt-unotice","Policy Qualifier User Notice",NID_id_qt_unotice,8,
	&(lvalues[966]),0},
{"RC2-64-CBC","rc2-64-cbc",NID_rc2_64_cbc,0,NULL,0},
{"SMIME-CAPS","S/MIME Capabilities",NID_SMIMECapabilities,9,
	&(lvalues[974]),0},
{"PBE-MD2-RC2-64","pbeWithMD2AndRC2-CBC",NID_pbeWithMD2AndRC2_CBC,9,
	&(lvalues[983]),0},
{"PBE-MD5-RC2-64","pbeWithMD5AndRC2-CBC",NID_pbeWithMD5AndRC2_CBC,9,
	&(lvalues[992]),0},
{"PBE-SHA1-DES","pbeWithSHA1AndDES-CBC",NID_pbeWithSHA1AndDES_CBC,9,
	&(lvalues[1001]),0},
{"msExtReq","Microsoft Extension Request",NID_ms_ext_req,10,
	&(lvalues[1010]),0},
{"extReq","Extension Request",NID_ext_req,9,&(lvalues[1020]),0},
{"name","name",NID_name,3,&(lvalues[1029]),0},
{"dnQualifier","dnQualifier",NID_dnQualifier,3,&(lvalues[1032]),0},
{"id-pe","id-pe",NID_id_pe,7,&(lvalues[1035]),0},
{"id-ad","id-ad",NID_id_ad,7,&(lvalues[1042]),0},
{"authorityInfoAccess","Authority Information Access",NID_info_access,
	8,&(lvalues[1049]),0},
{"OCSP","OCSP",NID_ad_OCSP,8,&(lvalues[1057]),0},
{"caIssuers","CA Issuers",NID_ad_ca_issuers,8,&(lvalues[1065]),0},
{"OCSPSigning","OCSP Signing",NID_OCSP_sign,8,&(lvalues[1073]),0},
{"ISO","iso",NID_iso,1,&(lvalues[1081]),0},
{"member-body","ISO Member Body",NID_member_body,1,&(lvalues[1082]),0},
{"ISO-US","ISO US Member Body",NID_ISO_US,3,&(lvalues[1083]),0},
{"X9-57","X9.57",NID_X9_57,5,&(lvalues[1086]),0},
{"X9cm","X9.57 CM ?",NID_X9cm,6,&(lvalues[1091]),0},
{"pkcs1","pkcs1",NID_pkcs1,8,&(lvalues[1097]),0},
{"pkcs5","pkcs5",NID_pkcs5,8,&(lvalues[1105]),0},
{"SMIME","S/MIME",NID_SMIME,9,&(lvalues[1113]),0},
{"id-smime-mod","id-smime-mod",NID_id_smime_mod,10,&(lvalues[1122]),0},
{"id-smime-ct","id-smime-ct",NID_id_smime_ct,10,&(lvalues[1132]),0},
{"id-smime-aa","id-smime-aa",NID_id_smime_aa,10,&(lvalues[1142]),0},
{"id-smime-alg","id-smime-alg",NID_id_smime_alg,10,&(lvalues[1152]),0},
{"id-smime-cd","id-smime-cd",NID_id_smime_cd,10,&(lvalues[1162]),0},
{"id-smime-spq","id-smime-spq",NID_id_smime_spq,10,&(lvalues[1172]),0},
{"id-smime-cti","id-smime-cti",NID_id_smime_cti,10,&(lvalues[1182]),0},
{"id-smime-mod-cms","id-smime-mod-cms",NID_id_smime_mod_cms,11,
	&(lvalues[1192]),0},
{"id-smime-mod-ess","id-smime-mod-ess",NID_id_smime_mod_ess,11,
	&(lvalues[1203]),0},
{"id-smime-mod-oid","id-smime-mod-oid",NID_id_smime_mod_oid,11,
	&(lvalues[1214]),0},
{"id-smime-mod-msg-v3","id-smime-mod-msg-v3",NID_id_smime_mod_msg_v3,
	11,&(lvalues[1225]),0},
{"id-smime-mod-ets-eSignature-88","id-smime-mod-ets-eSignature-88",
	NID_id_smime_mod_ets_eSignature_88,11,&(lvalues[1236]),0},
{"id-smime-mod-ets-eSignature-97","id-smime-mod-ets-eSignature-97",
	NID_id_smime_mod_ets_eSignature_97,11,&(lvalues[1247]),0},
{"id-smime-mod-ets-eSigPolicy-88","id-smime-mod-ets-eSigPolicy-88",
	NID_id_smime_mod_ets_eSigPolicy_88,11,&(lvalues[1258]),0},
{"id-smime-mod-ets-eSigPolicy-97","id-smime-mod-ets-eSigPolicy-97",
	NID_id_smime_mod_ets_eSigPolicy_97,11,&(lvalues[1269]),0},
{"id-smime-ct-receipt","id-smime-ct-receipt",NID_id_smime_ct_receipt,
	11,&(lvalues[1280]),0},
{"id-smime-ct-authData","id-smime-ct-authData",
	NID_id_smime_ct_authData,11,&(lvalues[1291]),0},
{"id-smime-ct-publishCert","id-smime-ct-publishCert",
	NID_id_smime_ct_publishCert,11,&(lvalues[1302]),0},
{"id-smime-ct-TSTInfo","id-smime-ct-TSTInfo",NID_id_smime_ct_TSTInfo,
	11,&(lvalues[1313]),0},
{"id-smime-ct-TDTInfo","id-smime-ct-TDTInfo",NID_id_smime_ct_TDTInfo,
	11,&(lvalues[1324]),0},
{"id-smime-ct-contentInfo","id-smime-ct-contentInfo",
	NID_id_smime_ct_contentInfo,11,&(lvalues[1335]),0},
{"id-smime-ct-DVCSRequestData","id-smime-ct-DVCSRequestData",
	NID_id_smime_ct_DVCSRequestData,11,&(lvalues[1346]),0},
{"id-smime-ct-DVCSResponseData","id-smime-ct-DVCSResponseData",
	NID_id_smime_ct_DVCSResponseData,11,&(lvalues[1357]),0},
{"id-smime-aa-receiptRequest","id-smime-aa-receiptRequest",
	NID_id_smime_aa_receiptRequest,11,&(lvalues[1368]),0},
{"id-smime-aa-securityLabel","id-smime-aa-securityLabel",
	NID_id_smime_aa_securityLabel,11,&(lvalues[1379]),0},
{"id-smime-aa-mlExpandHistory","id-smime-aa-mlExpandHistory",
	NID_id_smime_aa_mlExpandHistory,11,&(lvalues[1390]),0},
{"id-smime-aa-contentHint","id-smime-aa-contentHint",
	NID_id_smime_aa_contentHint,11,&(lvalues[1401]),0},
{"id-smime-aa-msgSigDigest","id-smime-aa-msgSigDigest",
	NID_id_smime_aa_msgSigDigest,11,&(lvalues[1412]),0},
{"id-smime-aa-encapContentType","id-smime-aa-encapContentType",
	NID_id_smime_aa_encapContentType,11,&(lvalues[1423]),0},
{"id-smime-aa-contentIdentifier","id-smime-aa-contentIdentifier",
	NID_id_smime_aa_contentIdentifier,11,&(lvalues[1434]),0},
{"id-smime-aa-macValue","id-smime-aa-macValue",
	NID_id_smime_aa_macValue,11,&(lvalues[1445]),0},
{"id-smime-aa-equivalentLabels","id-smime-aa-equivalentLabels",
	NID_id_smime_aa_equivalentLabels,11,&(lvalues[1456]),0},
{"id-smime-aa-contentReference","id-smime-aa-contentReference",
	NID_id_smime_aa_contentReference,11,&(lvalues[1467]),0},
{"id-smime-aa-encrypKeyPref","id-smime-aa-encrypKeyPref",
	NID_id_smime_aa_encrypKeyPref,11,&(lvalues[1478]),0},
{"id-smime-aa-signingCertificate","id-smime-aa-signingCertificate",
	NID_id_smime_aa_signingCertificate,11,&(lvalues[1489]),0},
{"id-smime-aa-smimeEncryptCerts","id-smime-aa-smimeEncryptCerts",
	NID_id_smime_aa_smimeEncryptCerts,11,&(lvalues[1500]),0},
{"id-smime-aa-timeStampToken","id-smime-aa-timeStampToken",
	NID_id_smime_aa_timeStampToken,11,&(lvalues[1511]),0},
{"id-smime-aa-ets-sigPolicyId","id-smime-aa-ets-sigPolicyId",
	NID_id_smime_aa_ets_sigPolicyId,11,&(lvalues[1522]),0},
{"id-smime-aa-ets-commitmentType","id-smime-aa-ets-commitmentType",
	NID_id_smime_aa_ets_commitmentType,11,&(lvalues[1533]),0},
{"id-smime-aa-ets-signerLocation","id-smime-aa-ets-signerLocation",
	NID_id_smime_aa_ets_signerLocation,11,&(lvalues[1544]),0},
{"id-smime-aa-ets-signerAttr","id-smime-aa-ets-signerAttr",
	NID_id_smime_aa_ets_signerAttr,11,&(lvalues[1555]),0},
{"id-smime-aa-ets-otherSigCert","id-smime-aa-ets-otherSigCert",
	NID_id_smime_aa_ets_otherSigCert,11,&(lvalues[1566]),0},
{"id-smime-aa-ets-contentTimestamp",
	"id-smime-aa-ets-contentTimestamp",
	NID_id_smime_aa_ets_contentTimestamp,11,&(lvalues[1577]),0},
{"id-smime-aa-ets-CertificateRefs","id-smime-aa-ets-CertificateRefs",
	NID_id_smime_aa_ets_CertificateRefs,11,&(lvalues[1588]),0},
{"id-smime-aa-ets-RevocationRefs","id-smime-aa-ets-RevocationRefs",
	NID_id_smime_aa_ets_RevocationRefs,11,&(lvalues[1599]),0},
{"id-smime-aa-ets-certValues","id-smime-aa-ets-certValues",
	NID_id_smime_aa_ets_certValues,11,&(lvalues[1610]),0},
{"id-smime-aa-ets-revocationValues",
	"id-smime-aa-ets-revocationValues",
	NID_id_smime_aa_ets_revocationValues,11,&(lvalues[1621]),0},
{"id-smime-aa-ets-escTimeStamp","id-smime-aa-ets-escTimeStamp",
	NID_id_smime_aa_ets_escTimeStamp,11,&(lvalues[1632]),0},
{"id-smime-aa-ets-certCRLTimestamp",
	"id-smime-aa-ets-certCRLTimestamp",
	NID_id_smime_aa_ets_certCRLTimestamp,11,&(lvalues[1643]),0},
{"id-smime-aa-ets-archiveTimeStamp",
	"id-smime-aa-ets-archiveTimeStamp",
	NID_id_smime_aa_ets_archiveTimeStamp,11,&(lvalues[1654]),0},
{"id-smime-aa-signatureType","id-smime-aa-signatureType",
	NID_id_smime_aa_signatureType,11,&(lvalues[1665]),0},
{"id-smime-aa-dvcs-dvc","id-smime-aa-dvcs-dvc",
	NID_id_smime_aa_dvcs_dvc,11,&(lvalues[1676]),0},
{"id-smime-alg-ESDHwith3DES","id-smime-alg-ESDHwith3DES",
	NID_id_smime_alg_ESDHwith3DES,11,&(lvalues[1687]),0},
{"id-smime-alg-ESDHwithRC2","id-smime-alg-ESDHwithRC2",
	NID_id_smime_alg_ESDHwithRC2,11,&(lvalues[1698]),0},
{"id-smime-alg-3DESwrap","id-smime-alg-3DESwrap",
	NID_id_smime_alg_3DESwrap,11,&(lvalues[1709]),0},
{"id-smime-alg-RC2wrap","id-smime-alg-RC2wrap",
	NID_id_smime_alg_RC2wrap,11,&(lvalues[1720]),0},
{"id-smime-alg-ESDH","id-smime-alg-ESDH",NID_id_smime_alg_ESDH,11,
	&(lvalues[1731]),0},
{"id-smime-alg-CMS3DESwrap","id-smime-alg-CMS3DESwrap",
	NID_id_smime_alg_CMS3DESwrap,11,&(lvalues[1742]),0},
{"id-smime-alg-CMSRC2wrap","id-smime-alg-CMSRC2wrap",
	NID_id_smime_alg_CMSRC2wrap,11,&(lvalues[1753]),0},
{"id-smime-cd-ldap","id-smime-cd-ldap",NID_id_smime_cd_ldap,11,
	&(lvalues[1764]),0},
{"id-smime-spq-ets-sqt-uri","id-smime-spq-ets-sqt-uri",
	NID_id_smime_spq_ets_sqt_uri,11,&(lvalues[1775]),0},
{"id-smime-spq-ets-sqt-unotice","id-smime-spq-ets-sqt-unotice",
	NID_id_smime_spq_ets_sqt_unotice,11,&(lvalues[1786]),0},
{"id-smime-cti-ets-proofOfOrigin","id-smime-cti-ets-proofOfOrigin",
	NID_id_smime_cti_ets_proofOfOrigin,11,&(lvalues[1797]),0},
{"id-smime-cti-ets-proofOfReceipt","id-smime-cti-ets-proofOfReceipt",
	NID_id_smime_cti_ets_proofOfReceipt,11,&(lvalues[1808]),0},
{"id-smime-cti-ets-proofOfDelivery",
	"id-smime-cti-ets-proofOfDelivery",
	NID_id_smime_cti_ets_proofOfDelivery,11,&(lvalues[1819]),0},
{"id-smime-cti-ets-proofOfSender","id-smime-cti-ets-proofOfSender",
	NID_id_smime_cti_ets_proofOfSender,11,&(lvalues[1830]),0},
{"id-smime-cti-ets-proofOfApproval",
	"id-smime-cti-ets-proofOfApproval",
	NID_id_smime_cti_ets_proofOfApproval,11,&(lvalues[1841]),0},
{"id-smime-cti-ets-proofOfCreation",
	"id-smime-cti-ets-proofOfCreation",
	NID_id_smime_cti_ets_proofOfCreation,11,&(lvalues[1852]),0},
{"MD4","md4",NID_md4,8,&(lvalues[1863]),0},
{"id-pkix-mod","id-pkix-mod",NID_id_pkix_mod,7,&(lvalues[1871]),0},
{"id-qt","id-qt",NID_id_qt,7,&(lvalues[1878]),0},
{"id-it","id-it",NID_id_it,7,&(lvalues[1885]),0},
{"id-pkip","id-pkip",NID_id_pkip,7,&(lvalues[1892]),0},
{"id-alg","id-alg",NID_id_alg,7,&(lvalues[1899]),0},
{"id-cmc","id-cmc",NID_id_cmc,7,&(lvalues[1906]),0},
{"id-on","id-on",NID_id_on,7,&(lvalues[1913]),0},
{"id-pda","id-pda",NID_id_pda,7,&(lvalues[1920]),0},
{"id-aca","id-aca",NID_id_aca,7,&(lvalues[1927]),0},
{"id-qcs","id-qcs",NID_id_qcs,7,&(lvalues[1934]),0},
{"id-cct","id-cct",NID_id_cct,7,&(lvalues[1941]),0},
{"id-pkix1-explicit-88","id-pkix1-explicit-88",
	NID_id_pkix1_explicit_88,8,&(lvalues[1948]),0},
{"id-pkix1-implicit-88","id-pkix1-implicit-88",
	NID_id_pkix1_implicit_88,8,&(lvalues[1956]),0},
{"id-pkix1-explicit-93","id-pkix1-explicit-93",
	NID_id_pkix1_explicit_93,8,&(lvalues[1964]),0},
{"id-pkix1-implicit-93","id-pkix1-implicit-93",
	NID_id_pkix1_implicit_93,8,&(lvalues[1972]),0},
{"id-mod-crmf","id-mod-crmf",NID_id_mod_crmf,8,&(lvalues[1980]),0},
{"id-mod-cmc","id-mod-cmc",NID_id_mod_cmc,8,&(lvalues[1988]),0},
{"id-mod-kea-profile-88","id-mod-kea-profile-88",
	NID_id_mod_kea_profile_88,8,&(lvalues[1996]),0},
{"id-mod-kea-profile-93","id-mod-kea-profile-93",
	NID_id_mod_kea_profile_93,8,&(lvalues[2004]),0},
{"id-mod-cmp","id-mod-cmp",NID_id_mod_cmp,8,&(lvalues[2012]),0},
{"id-mod-qualified-cert-88","id-mod-qualified-cert-88",
	NID_id_mod_qualified_cert_88,8,&(lvalues[2020]),0},
{"id-mod-qualified-cert-93","id-mod-qualified-cert-93",
	NID_id_mod_qualified_cert_93,8,&(lvalues[2028]),0},
{"id-mod-attribute-cert","id-mod-attribute-cert",
	NID_id_mod_attribute_cert,8,&(lvalues[2036]),0},
{"id-mod-timestamp-protocol","id-mod-timestamp-protocol",
	NID_id_mod_timestamp_protocol,8,&(lvalues[2044]),0},
{"id-mod-ocsp","id-mod-ocsp",NID_id_mod_ocsp,8,&(lvalues[2052]),0},
{"id-mod-dvcs","id-mod-dvcs",NID_id_mod_dvcs,8,&(lvalues[2060]),0},
{"id-mod-cmp2000","id-mod-cmp2000",NID_id_mod_cmp2000,8,
	&(lvalues[2068]),0},
{"biometricInfo","Biometric Info",NID_biometricInfo,8,&(lvalues[2076]),0},
{"qcStatements","qcStatements",NID_qcStatements,8,&(lvalues[2084]),0},
{"ac-auditEntity","ac-auditEntity",NID_ac_auditEntity,8,
	&(lvalues[2092]),0},
{"ac-targeting","ac-targeting",NID_ac_targeting,8,&(lvalues[2100]),0},
{"aaControls","aaControls",NID_aaControls,8,&(lvalues[2108]),0},
{"sbgp-ipAddrBlock","sbgp-ipAddrBlock",NID_sbgp_ipAddrBlock,8,
	&(lvalues[2116]),0},
{"sbgp-autonomousSysNum","sbgp-autonomousSysNum",
	NID_sbgp_autonomousSysNum,8,&(lvalues[2124]),0},
{"sbgp-routerIdentifier","sbgp-routerIdentifier",
	NID_sbgp_routerIdentifier,8,&(lvalues[2132]),0},
{"textNotice","textNotice",NID_textNotice,8,&(lvalues[2140]),0},
{"ipsecEndSystem","IPSec End System",NID_ipsecEndSystem,8,
	&(lvalues[2148]),0},
{"ipsecTunnel","IPSec Tunnel",NID_ipsecTunnel,8,&(lvalues[2156]),0},
{"ipsecUser","IPSec User",NID_ipsecUser,8,&(lvalues[2164]),0},
{"DVCS","dvcs",NID_dvcs,8,&(lvalues[2172]),0},
{"id-it-caProtEncCert","id-it-caProtEncCert",NID_id_it_caProtEncCert,
	8,&(lvalues[2180]),0},
{"id-it-signKeyPairTypes","id-it-signKeyPairTypes",
	NID_id_it_signKeyPairTypes,8,&(lvalues[2188]),0},
{"id-it-encKeyPairTypes","id-it-encKeyPairTypes",
	NID_id_it_encKeyPairTypes,8,&(lvalues[2196]),0},
{"id-it-preferredSymmAlg","id-it-preferredSymmAlg",
	NID_id_it_preferredSymmAlg,8,&(lvalues[2204]),0},
{"id-it-caKeyUpdateInfo","id-it-caKeyUpdateInfo",
	NID_id_it_caKeyUpdateInfo,8,&(lvalues[2212]),0},
{"id-it-currentCRL","id-it-currentCRL",NID_id_it_currentCRL,8,
	&(lvalues[2220]),0},
{"id-it-unsupportedOIDs","id-it-unsupportedOIDs",
	NID_id_it_unsupportedOIDs,8,&(lvalues[2228]),0},
{"id-it-subscriptionRequest","id-it-subscriptionRequest",
	NID_id_it_subscriptionRequest,8,&(lvalues[2236]),0},
{"id-it-subscriptionResponse","id-it-subscriptionResponse",
	NID_id_it_subscriptionResponse,8,&(lvalues[2244]),0},
{"id-it-keyPairParamReq","id-it-keyPairParamReq",
	NID_id_it_keyPairParamReq,8,&(lvalues[2252]),0},
{"id-it-keyPairParamRep","id-it-keyPairParamRep",
	NID_id_it_keyPairParamRep,8,&(lvalues[2260]),0},
{"id-it-revPassphrase","id-it-revPassphrase",NID_id_it_revPassphrase,
	8,&(lvalues[2268]),0},
{"id-it-implicitConfirm","id-it-implicitConfirm",
	NID_id_it_implicitConfirm,8,&(lvalues[2276]),0},
{"id-it-confirmWaitTime","id-it-confirmWaitTime",
	NID_id_it_confirmWaitTime,8,&(lvalues[2284]),0},
{"id-it-origPKIMessage","id-it-origPKIMessage",
	NID_id_it_origPKIMessage,8,&(lvalues[2292]),0},
{"id-regCtrl","id-regCtrl",NID_id_regCtrl,8,&(lvalues[2300]),0},
{"id-regInfo","id-regInfo",NID_id_regInfo,8,&(lvalues[2308]),0},
{"id-regCtrl-regToken","id-regCtrl-regToken",NID_id_regCtrl_regToken,
	9,&(lvalues[2316]),0},
{"id-regCtrl-authenticator","id-regCtrl-authenticator",
	NID_id_regCtrl_authenticator,9,&(lvalues[2325]),0},
{"id-regCtrl-pkiPublicationInfo","id-regCtrl-pkiPublicationInfo",
	NID_id_regCtrl_pkiPublicationInfo,9,&(lvalues[2334]),0},
{"id-regCtrl-pkiArchiveOptions","id-regCtrl-pkiArchiveOptions",
	NID_id_regCtrl_pkiArchiveOptions,9,&(lvalues[2343]),0},
{"id-regCtrl-oldCertID","id-regCtrl-oldCertID",
	NID_id_regCtrl_oldCertID,9,&(lvalues[2352]),0},
{"id-regCtrl-protocolEncrKey","id-regCtrl-protocolEncrKey",
	NID_id_regCtrl_protocolEncrKey,9,&(lvalues[2361]),0},
{"id-regInfo-utf8Pairs","id-regInfo-utf8Pairs",
	NID_id_regInfo_utf8Pairs,9,&(lvalues[2370]),0},
{"id-regInfo-certReq","id-regInfo-certReq",NID_id_regInfo_certReq,9,
	&(lvalues[2379]),0},
{"id-alg-des40","id-alg-des40",NID_id_alg_des40,8,&(lvalues[2388]),0},
{"id-alg-noSignature","id-alg-noSignature",NID_id_alg_noSignature,8,
	&(lvalues[2396]),0},
{"id-alg-dh-sig-hmac-sha1","id-alg-dh-sig-hmac-sha1",
	NID_id_alg_dh_sig_hmac_sha1,8,&(lvalues[2404]),0},
{"id-alg-dh-pop","id-alg-dh-pop",NID_id_alg_dh_pop,8,&(lvalues[2412]),0},
{"id-cmc-statusInfo","id-cmc-statusInfo",NID_id_cmc_statusInfo,8,
	&(lvalues[2420]),0},
{"id-cmc-identification","id-cmc-identification",
	NID_id_cmc_identification,8,&(lvalues[2428]),0},
{"id-cmc-identityProof","id-cmc-identityProof",
	NID_id_cmc_identityProof,8,&(lvalues[2436]),0},
{"id-cmc-dataReturn","id-cmc-dataReturn",NID_id_cmc_dataReturn,8,
	&(lvalues[2444]),0},
{"id-cmc-transactionId","id-cmc-transactionId",
	NID_id_cmc_transactionId,8,&(lvalues[2452]),0},
{"id-cmc-senderNonce","id-cmc-senderNonce",NID_id_cmc_senderNonce,8,
	&(lvalues[2460]),0},
{"id-cmc-recipientNonce","id-cmc-recipientNonce",
	NID_id_cmc_recipientNonce,8,&(lvalues[2468]),0},
{"id-cmc-addExtensions","id-cmc-addExtensions",
	NID_id_cmc_addExtensions,8,&(lvalues[2476]),0},
{"id-cmc-encryptedPOP","id-cmc-encryptedPOP",NID_id_cmc_encryptedPOP,
	8,&(lvalues[2484]),0},
{"id-cmc-decryptedPOP","id-cmc-decryptedPOP",NID_id_cmc_decryptedPOP,
	8,&(lvalues[2492]),0},
{"id-cmc-lraPOPWitness","id-cmc-lraPOPWitness",
	NID_id_cmc_lraPOPWitness,8,&(lvalues[2500]),0},
{"id-cmc-getCert","id-cmc-getCert",NID_id_cmc_getCert,8,
	&(lvalues[2508]),0},
{"id-cmc-getCRL","id-cmc-getCRL",NID_id_cmc_getCRL,8,&(lvalues[2516]),0},
{"id-cmc-revokeRequest","id-cmc-revokeRequest",
	NID_id_cmc_revokeRequest,8,&(lvalues[2524]),0},
{"id-cmc-regInfo","id-cmc-regInfo",NID_id_cmc_regInfo,8,
	&(lvalues[2532]),0},
{"id-cmc-responseInfo","id-cmc-responseInfo",NID_id_cmc_responseInfo,
	8,&(lvalues[2540]),0},
{"id-cmc-queryPending","id-cmc-queryPending",NID_id_cmc_queryPending,
	8,&(lvalues[2548]),0},
{"id-cmc-popLinkRandom","id-cmc-popLinkRandom",
	NID_id_cmc_popLinkRandom,8,&(lvalues[2556]),0},
{"id-cmc-popLinkWitness","id-cmc-popLinkWitness",
	NID_id_cmc_popLinkWitness,8,&(lvalues[2564]),0},
{"id-cmc-confirmCertAcceptance","id-cmc-confirmCertAcceptance",
	NID_id_cmc_confirmCertAcceptance,8,&(lvalues[2572]),0},
{"id-on-personalData","id-on-personalData",NID_id_on_personalData,8,
	&(lvalues[2580]),0},
{"id-pda-dateOfBirth","id-pda-dateOfBirth",NID_id_pda_dateOfBirth,8,
	&(lvalues[2588]),0},
{"id-pda-placeOfBirth","id-pda-placeOfBirth",NID_id_pda_placeOfBirth,
	8,&(lvalues[2596]),0},
{NULL,NULL,NID_undef,0,NULL,0},
{"id-pda-gender","id-pda-gender",NID_id_pda_gender,8,&(lvalues[2604]),0},
{"id-pda-countryOfCitizenship","id-pda-countryOfCitizenship",
	NID_id_pda_countryOfCitizenship,8,&(lvalues[2612]),0},
{"id-pda-countryOfResidence","id-pda-countryOfResidence",
	NID_id_pda_countryOfResidence,8,&(lvalues[2620]),0},
{"id-aca-authenticationInfo","id-aca-authenticationInfo",
	NID_id_aca_authenticationInfo,8,&(lvalues[2628]),0},
{"id-aca-accessIdentity","id-aca-accessIdentity",
	NID_id_aca_accessIdentity,8,&(lvalues[2636]),0},
{"id-aca-chargingIdentity","id-aca-chargingIdentity",
	NID_id_aca_chargingIdentity,8,&(lvalues[2644]),0},
{"id-aca-group","id-aca-group",NID_id_aca_group,8,&(lvalues[2652]),0},
{"id-aca-role","id-aca-role",NID_id_aca_role,8,&(lvalues[2660]),0},
{"id-qcs-pkixQCSyntax-v1","id-qcs-pkixQCSyntax-v1",
	NID_id_qcs_pkixQCSyntax_v1,8,&(lvalues[2668]),0},
{"id-cct-crs","id-cct-crs",NID_id_cct_crs,8,&(lvalues[2676]),0},
{"id-cct-PKIData","id-cct-PKIData",NID_id_cct_PKIData,8,
	&(lvalues[2684]),0},
{"id-cct-PKIResponse","id-cct-PKIResponse",NID_id_cct_PKIResponse,8,
	&(lvalues[2692]),0},
{"ad_timestamping","AD Time Stamping",NID_ad_timeStamping,8,
	&(lvalues[2700]),0},
{"AD_DVCS","ad dvcs",NID_ad_dvcs,8,&(lvalues[2708]),0},
{"basicOCSPResponse","Basic OCSP Response",NID_id_pkix_OCSP_basic,9,
	&(lvalues[2716]),0},
{"Nonce","OCSP Nonce",NID_id_pkix_OCSP_Nonce,9,&(lvalues[2725]),0},
{"CrlID","OCSP CRL ID",NID_id_pkix_OCSP_CrlID,9,&(lvalues[2734]),0},
{"acceptableResponses","Acceptable OCSP Responses",
	NID_id_pkix_OCSP_acceptableResponses,9,&(lvalues[2743]),0},
{"noCheck","OCSP No Check",NID_id_pkix_OCSP_noCheck,9,&(lvalues[2752]),0},
{"archiveCutoff","OCSP Archive Cutoff",NID_id_pkix_OCSP_archiveCutoff,
	9,&(lvalues[2761]),0},
{"serviceLocator","OCSP Service Locator",
	NID_id_pkix_OCSP_serviceLocator,9,&(lvalues[2770]),0},
{"extendedStatus","Extended OCSP Status",
	NID_id_pkix_OCSP_extendedStatus,9,&(lvalues[2779]),0},
{"valid","valid",NID_id_pkix_OCSP_valid,9,&(lvalues[2788]),0},
{"path","path",NID_id_pkix_OCSP_path,9,&(lvalues[2797]),0},
{"trustRoot","Trust Root",NID_id_pkix_OCSP_trustRoot,9,
	&(lvalues[2806]),0},
{"algorithm","algorithm",NID_algorithm,4,&(lvalues[2815]),0},
{"rsaSignature","rsaSignature",NID_rsaSignature,5,&(lvalues[2819]),0},
{"X500algorithms","directory services - algorithms",
	NID_X500algorithms,2,&(lvalues[2824]),0},
{"ORG","org",NID_org,1,&(lvalues[2826]),0},
{"DOD","dod",NID_dod,2,&(lvalues[2827]),0},
{"IANA","iana",NID_iana,3,&(lvalues[2829]),0},
{"directory","Directory",NID_Directory,4,&(lvalues[2832]),0},
{"mgmt","Management",NID_Management,4,&(lvalues[2836]),0},
{"experimental","Experimental",NID_Experimental,4,&(lvalues[2840]),0},
{"private","Private",NID_Private,4,&(lvalues[2844]),0},
{"security","Security",NID_Security,4,&(lvalues[2848]),0},
{"snmpv2","SNMPv2",NID_SNMPv2,4,&(lvalues[2852]),0},
{"Mail","Mail",NID_Mail,4,&(lvalues[2856]),0},
{"enterprises","Enterprises",NID_Enterprises,5,&(lvalues[2860]),0},
{"dcobject","dcObject",NID_dcObject,9,&(lvalues[2865]),0},
{"DC","domainComponent",NID_domainComponent,10,&(lvalues[2874]),0},
{"domain","Domain",NID_Domain,10,&(lvalues[2884]),0},
{"NULL","NULL",NID_joint_iso_ccitt,1,&(lvalues[2894]),0},
{"selected-attribute-types","Selected Attribute Types",
	NID_selected_attribute_types,3,&(lvalues[2895]),0},
{"clearance","clearance",NID_clearance,4,&(lvalues[2898]),0},
{"RSA-MD4","md4WithRSAEncryption",NID_md4WithRSAEncryption,9,
	&(lvalues[2902]),0},
{"ac-proxying","ac-proxying",NID_ac_proxying,8,&(lvalues[2911]),0},
{"subjectInfoAccess","Subject Information Access",NID_sinfo_access,8,
	&(lvalues[2919]),0},
{"id-aca-encAttrs","id-aca-encAttrs",NID_id_aca_encAttrs,8,
	&(lvalues[2927]),0},
{"role","role",NID_role,3,&(lvalues[2935]),0},
{"policyConstraints","X509v3 Policy Constraints",
	NID_policy_constraints,3,&(lvalues[2938]),0},
{"targetInformation","X509v3 AC Targeting",NID_target_information,3,
	&(lvalues[2941]),0},
{"noRevAvail","X509v3 No Revocation Available",NID_no_rev_avail,3,
	&(lvalues[2944]),0},
{"NULL","NULL",NID_ccitt,1,&(lvalues[2947]),0},
{"ansi-X9-62","ANSI X9.62",NID_ansi_X9_62,5,&(lvalues[2948]),0},
{"prime-field","prime-field",NID_X9_62_prime_field,7,&(lvalues[2953]),0},
{"characteristic-two-field","characteristic-two-field",
	NID_X9_62_characteristic_two_field,7,&(lvalues[2960]),0},
{"id-ecPublicKey","id-ecPublicKey",NID_X9_62_id_ecPublicKey,7,
	&(lvalues[2967]),0},
{"prime192v1","prime192v1",NID_X9_62_prime192v1,8,&(lvalues[2974]),0},
{"prime192v2","prime192v2",NID_X9_62_prime192v2,8,&(lvalues[2982]),0},
{"prime192v3","prime192v3",NID_X9_62_prime192v3,8,&(lvalues[2990]),0},
{"prime239v1","prime239v1",NID_X9_62_prime239v1,8,&(lvalues[2998]),0},
{"prime239v2","prime239v2",NID_X9_62_prime239v2,8,&(lvalues[3006]),0},
{"prime239v3","prime239v3",NID_X9_62_prime239v3,8,&(lvalues[3014]),0},
{"prime256v1","prime256v1",NID_X9_62_prime256v1,8,&(lvalues[3022]),0},
{"ecdsa-with-SHA1","ecdsa-with-SHA1",NID_ecdsa_with_SHA1,7,
	&(lvalues[3030]),0},
{"CSPName","Microsoft CSP Name",NID_ms_csp_name,9,&(lvalues[3037]),0},
{"AES-128-ECB","aes-128-ecb",NID_aes_128_ecb,9,&(lvalues[3046]),0},
{"AES-128-CBC","aes-128-cbc",NID_aes_128_cbc,9,&(lvalues[3055]),0},
{"AES-128-OFB","aes-128-ofb",NID_aes_128_ofb128,9,&(lvalues[3064]),0},
{"AES-128-CFB","aes-128-cfb",NID_aes_128_cfb128,9,&(lvalues[3073]),0},
{"AES-192-ECB","aes-192-ecb",NID_aes_192_ecb,9,&(lvalues[3082]),0},
{"AES-192-CBC","aes-192-cbc",NID_aes_192_cbc,9,&(lvalues[3091]),0},
{"AES-192-OFB","aes-192-ofb",NID_aes_192_ofb128,9,&(lvalues[3100]),0},
{"AES-192-CFB","aes-192-cfb",NID_aes_192_cfb128,9,&(lvalues[3109]),0},
{"AES-256-ECB","aes-256-ecb",NID_aes_256_ecb,9,&(lvalues[3118]),0},
{"AES-256-CBC","aes-256-cbc",NID_aes_256_cbc,9,&(lvalues[3127]),0},
{"AES-256-OFB","aes-256-ofb",NID_aes_256_ofb128,9,&(lvalues[3136]),0},
{"AES-256-CFB","aes-256-cfb",NID_aes_256_cfb128,9,&(lvalues[3145]),0},
{"holdInstructionCode","Hold Instruction Code",
	NID_hold_instruction_code,3,&(lvalues[3154]),0},
{"holdInstructionNone","Hold Instruction None",
	NID_hold_instruction_none,7,&(lvalues[3157]),0},
{"holdInstructionCallIssuer","Hold Instruction Call Issuer",
	NID_hold_instruction_call_issuer,7,&(lvalues[3164]),0},
{"holdInstructionReject","Hold Instruction Reject",
	NID_hold_instruction_reject,7,&(lvalues[3171]),0},
{"data","data",NID_data,1,&(lvalues[3178]),0},
{"pss","pss",NID_pss,3,&(lvalues[3179]),0},
{"ucl","ucl",NID_ucl,7,&(lvalues[3182]),0},
{"pilot","pilot",NID_pilot,8,&(lvalues[3189]),0},
{"pilotAttributeType","pilotAttributeType",NID_pilotAttributeType,9,
	&(lvalues[3197]),0},
{"pilotAttributeSyntax","pilotAttributeSyntax",
	NID_pilotAttributeSyntax,9,&(lvalues[3206]),0},
{"pilotObjectClass","pilotObjectClass",NID_pilotObjectClass,9,
	&(lvalues[3215]),0},
{"pilotGroups","pilotGroups",NID_pilotGroups,9,&(lvalues[3224]),0},
{"iA5StringSyntax","iA5StringSyntax",NID_iA5StringSyntax,10,
	&(lvalues[3233]),0},
{"caseIgnoreIA5StringSyntax","caseIgnoreIA5StringSyntax",
	NID_caseIgnoreIA5StringSyntax,10,&(lvalues[3243]),0},
{"pilotObject","pilotObject",NID_pilotObject,10,&(lvalues[3253]),0},
{"pilotPerson","pilotPerson",NID_pilotPerson,10,&(lvalues[3263]),0},
{"account","account",NID_account,10,&(lvalues[3273]),0},
{"document","document",NID_document,10,&(lvalues[3283]),0},
{"room","room",NID_room,10,&(lvalues[3293]),0},
{"documentSeries","documentSeries",NID_documentSeries,10,
	&(lvalues[3303]),0},
{"rFC822localPart","rFC822localPart",NID_rFC822localPart,10,
	&(lvalues[3313]),0},
{"dNSDomain","dNSDomain",NID_dNSDomain,10,&(lvalues[3323]),0},
{"domainRelatedObject","domainRelatedObject",NID_domainRelatedObject,
	10,&(lvalues[3333]),0},
{"friendlyCountry","friendlyCountry",NID_friendlyCountry,10,
	&(lvalues[3343]),0},
{"simpleSecurityObject","simpleSecurityObject",
	NID_simpleSecurityObject,10,&(lvalues[3353]),0},
{"pilotOrganization","pilotOrganization",NID_pilotOrganization,10,
	&(lvalues[3363]),0},
{"pilotDSA","pilotDSA",NID_pilotDSA,10,&(lvalues[3373]),0},
{"qualityLabelledData","qualityLabelledData",NID_qualityLabelledData,
	10,&(lvalues[3383]),0},
{"UID","userId",NID_userId,10,&(lvalues[3393]),0},
{"textEncodedORAddress","textEncodedORAddress",
	NID_textEncodedORAddress,10,&(lvalues[3403]),0},
{"mail","rfc822Mailbox",NID_rfc822Mailbox,10,&(lvalues[3413]),0},
{"info","info",NID_info,10,&(lvalues[3423]),0},
{"favouriteDrink","favouriteDrink",NID_favouriteDrink,10,
	&(lvalues[3433]),0},
{"roomNumber","roomNumber",NID_roomNumber,10,&(lvalues[3443]),0},
{"photo","photo",NID_photo,10,&(lvalues[3453]),0},
{"userClass","userClass",NID_userClass,10,&(lvalues[3463]),0},
{"host","host",NID_host,10,&(lvalues[3473]),0},
{"manager","manager",NID_manager,10,&(lvalues[3483]),0},
{"documentIdentifier","documentIdentifier",NID_documentIdentifier,10,
	&(lvalues[3493]),0},
{"documentTitle","documentTitle",NID_documentTitle,10,&(lvalues[3503]),0},
{"documentVersion","documentVersion",NID_documentVersion,10,
	&(lvalues[3513]),0},
{"documentAuthor","documentAuthor",NID_documentAuthor,10,
	&(lvalues[3523]),0},
{"documentLocation","documentLocation",NID_documentLocation,10,
	&(lvalues[3533]),0},
{"homeTelephoneNumber","homeTelephoneNumber",NID_homeTelephoneNumber,
	10,&(lvalues[3543]),0},
{"secretary","secretary",NID_secretary,10,&(lvalues[3553]),0},
{"otherMailbox","otherMailbox",NID_otherMailbox,10,&(lvalues[3563]),0},
{"lastModifiedTime","lastModifiedTime",NID_lastModifiedTime,10,
	&(lvalues[3573]),0},
{"lastModifiedBy","lastModifiedBy",NID_lastModifiedBy,10,
	&(lvalues[3583]),0},
{"aRecord","aRecord",NID_aRecord,10,&(lvalues[3593]),0},
{"pilotAttributeType27","pilotAttributeType27",
	NID_pilotAttributeType27,10,&(lvalues[3603]),0},
{"mXRecord","mXRecord",NID_mXRecord,10,&(lvalues[3613]),0},
{"nSRecord","nSRecord",NID_nSRecord,10,&(lvalues[3623]),0},
{"sOARecord","sOARecord",NID_sOARecord,10,&(lvalues[3633]),0},
{"cNAMERecord","cNAMERecord",NID_cNAMERecord,10,&(lvalues[3643]),0},
{"associatedDomain","associatedDomain",NID_associatedDomain,10,
	&(lvalues[3653]),0},
{"associatedName","associatedName",NID_associatedName,10,
	&(lvalues[3663]),0},
{"homePostalAddress","homePostalAddress",NID_homePostalAddress,10,
	&(lvalues[3673]),0},
{"personalTitle","personalTitle",NID_personalTitle,10,&(lvalues[3683]),0},
{"mobileTelephoneNumber","mobileTelephoneNumber",
	NID_mobileTelephoneNumber,10,&(lvalues[3693]),0},
{"pagerTelephoneNumber","pagerTelephoneNumber",
	NID_pagerTelephoneNumber,10,&(lvalues[3703]),0},
{"friendlyCountryName","friendlyCountryName",NID_friendlyCountryName,
	10,&(lvalues[3713]),0},
{"organizationalStatus","organizationalStatus",
	NID_organizationalStatus,10,&(lvalues[3723]),0},
{"janetMailbox","janetMailbox",NID_janetMailbox,10,&(lvalues[3733]),0},
{"mailPreferenceOption","mailPreferenceOption",
	NID_mailPreferenceOption,10,&(lvalues[3743]),0},
{"buildingName","buildingName",NID_buildingName,10,&(lvalues[3753]),0},
{"dSAQuality","dSAQuality",NID_dSAQuality,10,&(lvalues[3763]),0},
{"singleLevelQuality","singleLevelQuality",NID_singleLevelQuality,10,
	&(lvalues[3773]),0},
{"subtreeMinimumQuality","subtreeMinimumQuality",
	NID_subtreeMinimumQuality,10,&(lvalues[3783]),0},
{"subtreeMaximumQuality","subtreeMaximumQuality",
	NID_subtreeMaximumQuality,10,&(lvalues[3793]),0},
{"personalSignature","personalSignature",NID_personalSignature,10,
	&(lvalues[3803]),0},
{"dITRedirect","dITRedirect",NID_dITRedirect,10,&(lvalues[3813]),0},
{"audio","audio",NID_audio,10,&(lvalues[3823]),0},
{"documentPublisher","documentPublisher",NID_documentPublisher,10,
	&(lvalues[3833]),0},
{"x500UniqueIdentifier","x500UniqueIdentifier",
	NID_x500UniqueIdentifier,3,&(lvalues[3843]),0},
{"mime-mhs","MIME MHS",NID_mime_mhs,5,&(lvalues[3846]),0},
{"mime-mhs-headings","mime-mhs-headings",NID_mime_mhs_headings,6,
	&(lvalues[3851]),0},
{"mime-mhs-bodies","mime-mhs-bodies",NID_mime_mhs_bodies,6,
	&(lvalues[3857]),0},
{"id-hex-partial-message","id-hex-partial-message",
	NID_id_hex_partial_message,7,&(lvalues[3863]),0},
{"id-hex-multipart-message","id-hex-multipart-message",
	NID_id_hex_multipart_message,7,&(lvalues[3870]),0},
{"generationQualifier","generationQualifier",NID_generationQualifier,
	3,&(lvalues[3877]),0},
{"pseudonym","pseudonym",NID_pseudonym,3,&(lvalues[3880]),0},
{NULL,NULL,NID_undef,0,NULL,0},
{"id-set","Secure Electronic Transactions",NID_id_set,2,
	&(lvalues[3883]),0},
{"set-ctype","content types",NID_set_ctype,3,&(lvalues[3885]),0},
{"set-msgExt","message extensions",NID_set_msgExt,3,&(lvalues[3888]),0},
{"set-attr","set-attr",NID_set_attr,3,&(lvalues[3891]),0},
{"set-policy","set-policy",NID_set_policy,3,&(lvalues[3894]),0},
{"set-certExt","certificate extensions",NID_set_certExt,3,
	&(lvalues[3897]),0},
{"set-brand","set-brand",NID_set_brand,3,&(lvalues[3900]),0},
{"setct-PANData","setct-PANData",NID_setct_PANData,4,&(lvalues[3903]),0},
{"setct-PANToken","setct-PANToken",NID_setct_PANToken,4,
	&(lvalues[3907]),0},
{"setct-PANOnly","setct-PANOnly",NID_setct_PANOnly,4,&(lvalues[3911]),0},
{"setct-OIData","setct-OIData",NID_setct_OIData,4,&(lvalues[3915]),0},
{"setct-PI","setct-PI",NID_setct_PI,4,&(lvalues[3919]),0},
{"setct-PIData","setct-PIData",NID_setct_PIData,4,&(lvalues[3923]),0},
{"setct-PIDataUnsigned","setct-PIDataUnsigned",
	NID_setct_PIDataUnsigned,4,&(lvalues[3927]),0},
{"setct-HODInput","setct-HODInput",NID_setct_HODInput,4,
	&(lvalues[3931]),0},
{"setct-AuthResBaggage","setct-AuthResBaggage",
	NID_setct_AuthResBaggage,4,&(lvalues[3935]),0},
{"setct-AuthRevReqBaggage","setct-AuthRevReqBaggage",
	NID_setct_AuthRevReqBaggage,4,&(lvalues[3939]),0},
{"setct-AuthRevResBaggage","setct-AuthRevResBaggage",
	NID_setct_AuthRevResBaggage,4,&(lvalues[3943]),0},
{"setct-CapTokenSeq","setct-CapTokenSeq",NID_setct_CapTokenSeq,4,
	&(lvalues[3947]),0},
{"setct-PInitResData","setct-PInitResData",NID_setct_PInitResData,4,
	&(lvalues[3951]),0},
{"setct-PI-TBS","setct-PI-TBS",NID_setct_PI_TBS,4,&(lvalues[3955]),0},
{"setct-PResData","setct-PResData",NID_setct_PResData,4,
	&(lvalues[3959]),0},
{"setct-AuthReqTBS","setct-AuthReqTBS",NID_setct_AuthReqTBS,4,
	&(lvalues[3963]),0},
{"setct-AuthResTBS","setct-AuthResTBS",NID_setct_AuthResTBS,4,
	&(lvalues[3967]),0},
{"setct-AuthResTBSX","setct-AuthResTBSX",NID_setct_AuthResTBSX,4,
	&(lvalues[3971]),0},
{"setct-AuthTokenTBS","setct-AuthTokenTBS",NID_setct_AuthTokenTBS,4,
	&(lvalues[3975]),0},
{"setct-CapTokenData","setct-CapTokenData",NID_setct_CapTokenData,4,
	&(lvalues[3979]),0},
{"setct-CapTokenTBS","setct-CapTokenTBS",NID_setct_CapTokenTBS,4,
	&(lvalues[3983]),0},
{"setct-AcqCardCodeMsg","setct-AcqCardCodeMsg",
	NID_setct_AcqCardCodeMsg,4,&(lvalues[3987]),0},
{"setct-AuthRevReqTBS","setct-AuthRevReqTBS",NID_setct_AuthRevReqTBS,
	4,&(lvalues[3991]),0},
{"setct-AuthRevResData","setct-AuthRevResData",
	NID_setct_AuthRevResData,4,&(lvalues[3995]),0},
{"setct-AuthRevResTBS","setct-AuthRevResTBS",NID_setct_AuthRevResTBS,
	4,&(lvalues[3999]),0},
{"setct-CapReqTBS","setct-CapReqTBS",NID_setct_CapReqTBS,4,
	&(lvalues[4003]),0},
{"setct-CapReqTBSX","setct-CapReqTBSX",NID_setct_CapReqTBSX,4,
	&(lvalues[4007]),0},
{"setct-CapResData","setct-CapResData",NID_setct_CapResData,4,
	&(lvalues[4011]),0},
{"setct-CapRevReqTBS","setct-CapRevReqTBS",NID_setct_CapRevReqTBS,4,
	&(lvalues[4015]),0},
{"setct-CapRevReqTBSX","setct-CapRevReqTBSX",NID_setct_CapRevReqTBSX,
	4,&(lvalues[4019]),0},
{"setct-CapRevResData","setct-CapRevResData",NID_setct_CapRevResData,
	4,&(lvalues[4023]),0},
{"setct-CredReqTBS","setct-CredReqTBS",NID_setct_CredReqTBS,4,
	&(lvalues[4027]),0},
{"setct-CredReqTBSX","setct-CredReqTBSX",NID_setct_CredReqTBSX,4,
	&(lvalues[4031]),0},
{"setct-CredResData","setct-CredResData",NID_setct_CredResData,4,
	&(lvalues[4035]),0},
{"setct-CredRevReqTBS","setct-CredRevReqTBS",NID_setct_CredRevReqTBS,
	4,&(lvalues[4039]),0},
{"setct-CredRevReqTBSX","setct-CredRevReqTBSX",
	NID_setct_CredRevReqTBSX,4,&(lvalues[4043]),0},
{"setct-CredRevResData","setct-CredRevResData",
	NID_setct_CredRevResData,4,&(lvalues[4047]),0},
{"setct-PCertReqData","setct-PCertReqData",NID_setct_PCertReqData,4,
	&(lvalues[4051]),0},
{"setct-PCertResTBS","setct-PCertResTBS",NID_setct_PCertResTBS,4,
	&(lvalues[4055]),0},
{"setct-BatchAdminReqData","setct-BatchAdminReqData",
	NID_setct_BatchAdminReqData,4,&(lvalues[4059]),0},
{"setct-BatchAdminResData","setct-BatchAdminResData",
	NID_setct_BatchAdminResData,4,&(lvalues[4063]),0},
{"setct-CardCInitResTBS","setct-CardCInitResTBS",
	NID_setct_CardCInitResTBS,4,&(lvalues[4067]),0},
{"setct-MeAqCInitResTBS","setct-MeAqCInitResTBS",
	NID_setct_MeAqCInitResTBS,4,&(lvalues[4071]),0},
{"setct-RegFormResTBS","setct-RegFormResTBS",NID_setct_RegFormResTBS,
	4,&(lvalues[4075]),0},
{"setct-CertReqData","setct-CertReqData",NID_setct_CertReqData,4,
	&(lvalues[4079]),0},
{"setct-CertReqTBS","setct-CertReqTBS",NID_setct_CertReqTBS,4,
	&(lvalues[4083]),0},
{"setct-CertResData","setct-CertResData",NID_setct_CertResData,4,
	&(lvalues[4087]),0},
{"setct-CertInqReqTBS","setct-CertInqReqTBS",NID_setct_CertInqReqTBS,
	4,&(lvalues[4091]),0},
{"setct-ErrorTBS","setct-ErrorTBS",NID_setct_ErrorTBS,4,
	&(lvalues[4095]),0},
{"setct-PIDualSignedTBE","setct-PIDualSignedTBE",
	NID_setct_PIDualSignedTBE,4,&(lvalues[4099]),0},
{"setct-PIUnsignedTBE","setct-PIUnsignedTBE",NID_setct_PIUnsignedTBE,
	4,&(lvalues[4103]),0},
{"setct-AuthReqTBE","setct-AuthReqTBE",NID_setct_AuthReqTBE,4,
	&(lvalues[4107]),0},
{"setct-AuthResTBE","setct-AuthResTBE",NID_setct_AuthResTBE,4,
	&(lvalues[4111]),0},
{"setct-AuthResTBEX","setct-AuthResTBEX",NID_setct_AuthResTBEX,4,
	&(lvalues[4115]),0},
{"setct-AuthTokenTBE","setct-AuthTokenTBE",NID_setct_AuthTokenTBE,4,
	&(lvalues[4119]),0},
{"setct-CapTokenTBE","setct-CapTokenTBE",NID_setct_CapTokenTBE,4,
	&(lvalues[4123]),0},
{"setct-CapTokenTBEX","setct-CapTokenTBEX",NID_setct_CapTokenTBEX,4,
	&(lvalues[4127]),0},
{"setct-AcqCardCodeMsgTBE","setct-AcqCardCodeMsgTBE",
	NID_setct_AcqCardCodeMsgTBE,4,&(lvalues[4131]),0},
{"setct-AuthRevReqTBE","setct-AuthRevReqTBE",NID_setct_AuthRevReqTBE,
	4,&(lvalues[4135]),0},
{"setct-AuthRevResTBE","setct-AuthRevResTBE",NID_setct_AuthRevResTBE,
	4,&(lvalues[4139]),0},
{"setct-AuthRevResTBEB","setct-AuthRevResTBEB",
	NID_setct_AuthRevResTBEB,4,&(lvalues[4143]),0},
{"setct-CapReqTBE","setct-CapReqTBE",NID_setct_CapReqTBE,4,
	&(lvalues[4147]),0},
{"setct-CapReqTBEX","setct-CapReqTBEX",NID_setct_CapReqTBEX,4,
	&(lvalues[4151]),0},
{"setct-CapResTBE","setct-CapResTBE",NID_setct_CapResTBE,4,
	&(lvalues[4155]),0},
{"setct-CapRevReqTBE","setct-CapRevReqTBE",NID_setct_CapRevReqTBE,4,
	&(lvalues[4159]),0},
{"setct-CapRevReqTBEX","setct-CapRevReqTBEX",NID_setct_CapRevReqTBEX,
	4,&(lvalues[4163]),0},
{"setct-CapRevResTBE","setct-CapRevResTBE",NID_setct_CapRevResTBE,4,
	&(lvalues[4167]),0},
{"setct-CredReqTBE","setct-CredReqTBE",NID_setct_CredReqTBE,4,
	&(lvalues[4171]),0},
{"setct-CredReqTBEX","setct-CredReqTBEX",NID_setct_CredReqTBEX,4,
	&(lvalues[4175]),0},
{"setct-CredResTBE","setct-CredResTBE",NID_setct_CredResTBE,4,
	&(lvalues[4179]),0},
{"setct-CredRevReqTBE","setct-CredRevReqTBE",NID_setct_CredRevReqTBE,
	4,&(lvalues[4183]),0},
{"setct-CredRevReqTBEX","setct-CredRevReqTBEX",
	NID_setct_CredRevReqTBEX,4,&(lvalues[4187]),0},
{"setct-CredRevResTBE","setct-CredRevResTBE",NID_setct_CredRevResTBE,
	4,&(lvalues[4191]),0},
{"setct-BatchAdminReqTBE","setct-BatchAdminReqTBE",
	NID_setct_BatchAdminReqTBE,4,&(lvalues[4195]),0},
{"setct-BatchAdminResTBE","setct-BatchAdminResTBE",
	NID_setct_BatchAdminResTBE,4,&(lvalues[4199]),0},
{"setct-RegFormReqTBE","setct-RegFormReqTBE",NID_setct_RegFormReqTBE,
	4,&(lvalues[4203]),0},
{"setct-CertReqTBE","setct-CertReqTBE",NID_setct_CertReqTBE,4,
	&(lvalues[4207]),0},
{"setct-CertReqTBEX","setct-CertReqTBEX",NID_setct_CertReqTBEX,4,
	&(lvalues[4211]),0},
{"setct-CertResTBE","setct-CertResTBE",NID_setct_CertResTBE,4,
	&(lvalues[4215]),0},
{"setct-CRLNotificationTBS","setct-CRLNotificationTBS",
	NID_setct_CRLNotificationTBS,4,&(lvalues[4219]),0},
{"setct-CRLNotificationResTBS","setct-CRLNotificationResTBS",
	NID_setct_CRLNotificationResTBS,4,&(lvalues[4223]),0},
{"setct-BCIDistributionTBS","setct-BCIDistributionTBS",
	NID_setct_BCIDistributionTBS,4,&(lvalues[4227]),0},
{"setext-genCrypt","generic cryptogram",NID_setext_genCrypt,4,
	&(lvalues[4231]),0},
{"setext-miAuth","merchant initiated auth",NID_setext_miAuth,4,
	&(lvalues[4235]),0},
{"setext-pinSecure","setext-pinSecure",NID_setext_pinSecure,4,
	&(lvalues[4239]),0},
{"setext-pinAny","setext-pinAny",NID_setext_pinAny,4,&(lvalues[4243]),0},
{"setext-track2","setext-track2",NID_setext_track2,4,&(lvalues[4247]),0},
{"setext-cv","additional verification",NID_setext_cv,4,
	&(lvalues[4251]),0},
{"set-policy-root","set-policy-root",NID_set_policy_root,4,
	&(lvalues[4255]),0},
{"setCext-hashedRoot","setCext-hashedRoot",NID_setCext_hashedRoot,4,
	&(lvalues[4259]),0},
{"setCext-certType","setCext-certType",NID_setCext_certType,4,
	&(lvalues[4263]),0},
{"setCext-merchData","setCext-merchData",NID_setCext_merchData,4,
	&(lvalues[4267]),0},
{"setCext-cCertRequired","setCext-cCertRequired",
	NID_setCext_cCertRequired,4,&(lvalues[4271]),0},
{"setCext-tunneling","setCext-tunneling",NID_setCext_tunneling,4,
	&(lvalues[4275]),0},
{"setCext-setExt","setCext-setExt",NID_setCext_setExt,4,
	&(lvalues[4279]),0},
{"setCext-setQualf","setCext-setQualf",NID_setCext_setQualf,4,
	&(lvalues[4283]),0},
{"setCext-PGWYcapabilities","setCext-PGWYcapabilities",
	NID_setCext_PGWYcapabilities,4,&(lvalues[4287]),0},
{"setCext-TokenIdentifier","setCext-TokenIdentifier",
	NID_setCext_TokenIdentifier,4,&(lvalues[4291]),0},
{"setCext-Track2Data","setCext-Track2Data",NID_setCext_Track2Data,4,
	&(lvalues[4295]),0},
{"setCext-TokenType","setCext-TokenType",NID_setCext_TokenType,4,
	&(lvalues[4299]),0},
{"setCext-IssuerCapabilities","setCext-IssuerCapabilities",
	NID_setCext_IssuerCapabilities,4,&(lvalues[4303]),0},
{"setAttr-Cert","setAttr-Cert",NID_setAttr_Cert,4,&(lvalues[4307]),0},
{"setAttr-PGWYcap","payment gateway capabilities",NID_setAttr_PGWYcap,
	4,&(lvalues[4311]),0},
{"setAttr-TokenType","setAttr-TokenType",NID_setAttr_TokenType,4,
	&(lvalues[4315]),0},
{"setAttr-IssCap","issuer capabilities",NID_setAttr_IssCap,4,
	&(lvalues[4319]),0},
{"set-rootKeyThumb","set-rootKeyThumb",NID_set_rootKeyThumb,5,
	&(lvalues[4323]),0},
{"set-addPolicy","set-addPolicy",NID_set_addPolicy,5,&(lvalues[4328]),0},
{"setAttr-Token-EMV","setAttr-Token-EMV",NID_setAttr_Token_EMV,5,
	&(lvalues[4333]),0},
{"setAttr-Token-B0Prime","setAttr-Token-B0Prime",
	NID_setAttr_Token_B0Prime,5,&(lvalues[4338]),0},
{"setAttr-IssCap-CVM","setAttr-IssCap-CVM",NID_setAttr_IssCap_CVM,5,
	&(lvalues[4343]),0},
{"setAttr-IssCap-T2","setAttr-IssCap-T2",NID_setAttr_IssCap_T2,5,
	&(lvalues[4348]),0},
{"setAttr-IssCap-Sig","setAttr-IssCap-Sig",NID_setAttr_IssCap_Sig,5,
	&(lvalues[4353]),0},
{"setAttr-GenCryptgrm","generate cryptogram",NID_setAttr_GenCryptgrm,
	6,&(lvalues[4358]),0},
{"setAttr-T2Enc","encrypted track 2",NID_setAttr_T2Enc,6,
	&(lvalues[4364]),0},
{"setAttr-T2cleartxt","cleartext track 2",NID_setAttr_T2cleartxt,6,
	&(lvalues[4370]),0},
{"setAttr-TokICCsig","ICC or token signature",NID_setAttr_TokICCsig,6,
	&(lvalues[4376]),0},
{"setAttr-SecDevSig","secure device signature",NID_setAttr_SecDevSig,
	6,&(lvalues[4382]),0},
{"set-brand-IATA-ATA","set-brand-IATA-ATA",NID_set_brand_IATA_ATA,4,
	&(lvalues[4388]),0},
{"set-brand-Diners","set-brand-Diners",NID_set_brand_Diners,4,
	&(lvalues[4392]),0},
{"set-brand-AmericanExpress","set-brand-AmericanExpress",
	NID_set_brand_AmericanExpress,4,&(lvalues[4396]),0},
{"set-brand-JCB","set-brand-JCB",NID_set_brand_JCB,4,&(lvalues[4400]),0},
{"set-brand-Visa","set-brand-Visa",NID_set_brand_Visa,4,
	&(lvalues[4404]),0},
{"set-brand-MasterCard","set-brand-MasterCard",
	NID_set_brand_MasterCard,4,&(lvalues[4408]),0},
{"set-brand-Novus","set-brand-Novus",NID_set_brand_Novus,5,
	&(lvalues[4412]),0},
{"DES-CDMF","des-cdmf",NID_des_cdmf,8,&(lvalues[4417]),0},
{"rsaOAEPEncryptionSET","rsaOAEPEncryptionSET",
	NID_rsaOAEPEncryptionSET,9,&(lvalues[4425]),0},
{"ITU-T","itu-t",NID_itu_t,1,&(lvalues[4434]),0},
{"JOINT-ISO-ITU-T","joint-iso-itu-t",NID_joint_iso_itu_t,1,
	&(lvalues[4435]),0},
{"international-organizations","International Organizations",
	NID_international_organizations,1,&(lvalues[4436]),0},
{"msSmartcardLogin","Microsoft Smartcardlogin",NID_ms_smartcard_login,
	10,&(lvalues[4437]),0},
{"msUPN","Microsoft Universal Principal Name",NID_ms_upn,10,
	&(lvalues[4447]),0},
{"AES-128-CFB1","aes-128-cfb1",NID_aes_128_cfb1,0,NULL,0},
{"AES-192-CFB1","aes-192-cfb1",NID_aes_192_cfb1,0,NULL,0},
{"AES-256-CFB1","aes-256-cfb1",NID_aes_256_cfb1,0,NULL,0},
{"AES-128-CFB8","aes-128-cfb8",NID_aes_128_cfb8,0,NULL,0},
{"AES-192-CFB8","aes-192-cfb8",NID_aes_192_cfb8,0,NULL,0},
{"AES-256-CFB8","aes-256-cfb8",NID_aes_256_cfb8,0,NULL,0},
{"DES-CFB1","des-cfb1",NID_des_cfb1,0,NULL,0},
{"DES-CFB8","des-cfb8",NID_des_cfb8,0,NULL,0},
{"DES-EDE3-CFB1","des-ede3-cfb1",NID_des_ede3_cfb1,0,NULL,0},
{"DES-EDE3-CFB8","des-ede3-cfb8",NID_des_ede3_cfb8,0,NULL,0},
{"streetAddress","streetAddress",NID_streetAddress,3,&(lvalues[4457]),0},
{"postalCode","postalCode",NID_postalCode,3,&(lvalues[4460]),0},
{"id-ppl","id-ppl",NID_id_ppl,7,&(lvalues[4463]),0},
{"proxyCertInfo","Proxy Certificate Information",NID_proxyCertInfo,8,
	&(lvalues[4470]),0},
{"id-ppl-anyLanguage","Any language",NID_id_ppl_anyLanguage,8,
	&(lvalues[4478]),0},
{"id-ppl-inheritAll","Inherit all",NID_id_ppl_inheritAll,8,
	&(lvalues[4486]),0},
{"nameConstraints","X509v3 Name Constraints",NID_name_constraints,3,
	&(lvalues[4494]),0},
{"id-ppl-independent","Independent",NID_Independent,8,&(lvalues[4497]),0},
{"RSA-SHA256","sha256WithRSAEncryption",NID_sha256WithRSAEncryption,9,
	&(lvalues[4505]),0},
{"RSA-SHA384","sha384WithRSAEncryption",NID_sha384WithRSAEncryption,9,
	&(lvalues[4514]),0},
{"RSA-SHA512","sha512WithRSAEncryption",NID_sha512WithRSAEncryption,9,
	&(lvalues[4523]),0},
{"RSA-SHA224","sha224WithRSAEncryption",NID_sha224WithRSAEncryption,9,
	&(lvalues[4532]),0},
{"SHA256","sha256",NID_sha256,9,&(lvalues[4541]),0},
{"SHA384","sha384",NID_sha384,9,&(lvalues[4550]),0},
{"SHA512","sha512",NID_sha512,9,&(lvalues[4559]),0},
{"SHA224","sha224",NID_sha224,9,&(lvalues[4568]),0},
{"identified-organization","identified-organization",
	NID_identified_organization,1,&(lvalues[4577]),0},
{"certicom-arc","certicom-arc",NID_certicom_arc,3,&(lvalues[4578]),0},
{"wap","wap",NID_wap,2,&(lvalues[4581]),0},
{"wap-wsg","wap-wsg",NID_wap_wsg,3,&(lvalues[4583]),0},
{"id-characteristic-two-basis","id-characteristic-two-basis",
	NID_X9_62_id_characteristic_two_basis,8,&(lvalues[4586]),0},
{"onBasis","onBasis",NID_X9_62_onBasis,9,&(lvalues[4594]),0},
{"tpBasis","tpBasis",NID_X9_62_tpBasis,9,&(lvalues[4603]),0},
{"ppBasis","ppBasis",NID_X9_62_ppBasis,9,&(lvalues[4612]),0},
{"c2pnb163v1","c2pnb163v1",NID_X9_62_c2pnb163v1,8,&(lvalues[4621]),0},
{"c2pnb163v2","c2pnb163v2",NID_X9_62_c2pnb163v2,8,&(lvalues[4629]),0},
{"c2pnb163v3","c2pnb163v3",NID_X9_62_c2pnb163v3,8,&(lvalues[4637]),0},
{"c2pnb176v1","c2pnb176v1",NID_X9_62_c2pnb176v1,8,&(lvalues[4645]),0},
{"c2tnb191v1","c2tnb191v1",NID_X9_62_c2tnb191v1,8,&(lvalues[4653]),0},
{"c2tnb191v2","c2tnb191v2",NID_X9_62_c2tnb191v2,8,&(lvalues[4661]),0},
{"c2tnb191v3","c2tnb191v3",NID_X9_62_c2tnb191v3,8,&(lvalues[4669]),0},
{"c2onb191v4","c2onb191v4",NID_X9_62_c2onb191v4,8,&(lvalues[4677]),0},
{"c2onb191v5","c2onb191v5",NID_X9_62_c2onb191v5,8,&(lvalues[4685]),0},
{"c2pnb208w1","c2pnb208w1",NID_X9_62_c2pnb208w1,8,&(lvalues[4693]),0},
{"c2tnb239v1","c2tnb239v1",NID_X9_62_c2tnb239v1,8,&(lvalues[4701]),0},
{"c2tnb239v2","c2tnb239v2",NID_X9_62_c2tnb239v2,8,&(lvalues[4709]),0},
{"c2tnb239v3","c2tnb239v3",NID_X9_62_c2tnb239v3,8,&(lvalues[4717]),0},
{"c2onb239v4","c2onb239v4",NID_X9_62_c2onb239v4,8,&(lvalues[4725]),0},
{"c2onb239v5","c2onb239v5",NID_X9_62_c2onb239v5,8,&(lvalues[4733]),0},
{"c2pnb272w1","c2pnb272w1",NID_X9_62_c2pnb272w1,8,&(lvalues[4741]),0},
{"c2pnb304w1","c2pnb304w1",NID_X9_62_c2pnb304w1,8,&(lvalues[4749]),0},
{"c2tnb359v1","c2tnb359v1",NID_X9_62_c2tnb359v1,8,&(lvalues[4757]),0},
{"c2pnb368w1","c2pnb368w1",NID_X9_62_c2pnb368w1,8,&(lvalues[4765]),0},
{"c2tnb431r1","c2tnb431r1",NID_X9_62_c2tnb431r1,8,&(lvalues[4773]),0},
{"secp112r1","secp112r1",NID_secp112r1,5,&(lvalues[4781]),0},
{"secp112r2","secp112r2",NID_secp112r2,5,&(lvalues[4786]),0},
{"secp128r1","secp128r1",NID_secp128r1,5,&(lvalues[4791]),0},
{"secp128r2","secp128r2",NID_secp128r2,5,&(lvalues[4796]),0},
{"secp160k1","secp160k1",NID_secp160k1,5,&(lvalues[4801]),0},
{"secp160r1","secp160r1",NID_secp160r1,5,&(lvalues[4806]),0},
{"secp160r2","secp160r2",NID_secp160r2,5,&(lvalues[4811]),0},
{"secp192k1","secp192k1",NID_secp192k1,5,&(lvalues[4816]),0},
{"secp224k1","secp224k1",NID_secp224k1,5,&(lvalues[4821]),0},
{"secp224r1","secp224r1",NID_secp224r1,5,&(lvalues[4826]),0},
{"secp256k1","secp256k1",NID_secp256k1,5,&(lvalues[4831]),0},
{"secp384r1","secp384r1",NID_secp384r1,5,&(lvalues[4836]),0},
{"secp521r1","secp521r1",NID_secp521r1,5,&(lvalues[4841]),0},
{"sect113r1","sect113r1",NID_sect113r1,5,&(lvalues[4846]),0},
{"sect113r2","sect113r2",NID_sect113r2,5,&(lvalues[4851]),0},
{"sect131r1","sect131r1",NID_sect131r1,5,&(lvalues[4856]),0},
{"sect131r2","sect131r2",NID_sect131r2,5,&(lvalues[4861]),0},
{"sect163k1","sect163k1",NID_sect163k1,5,&(lvalues[4866]),0},
{"sect163r1","sect163r1",NID_sect163r1,5,&(lvalues[4871]),0},
{"sect163r2","sect163r2",NID_sect163r2,5,&(lvalues[4876]),0},
{"sect193r1","sect193r1",NID_sect193r1,5,&(lvalues[4881]),0},
{"sect193r2","sect193r2",NID_sect193r2,5,&(lvalues[4886]),0},
{"sect233k1","sect233k1",NID_sect233k1,5,&(lvalues[4891]),0},
{"sect233r1","sect233r1",NID_sect233r1,5,&(lvalues[4896]),0},
{"sect239k1","sect239k1",NID_sect239k1,5,&(lvalues[4901]),0},
{"sect283k1","sect283k1",NID_sect283k1,5,&(lvalues[4906]),0},
{"sect283r1","sect283r1",NID_sect283r1,5,&(lvalues[4911]),0},
{"sect409k1","sect409k1",NID_sect409k1,5,&(lvalues[4916]),0},
{"sect409r1","sect409r1",NID_sect409r1,5,&(lvalues[4921]),0},
{"sect571k1","sect571k1",NID_sect571k1,5,&(lvalues[4926]),0},
{"sect571r1","sect571r1",NID_sect571r1,5,&(lvalues[4931]),0},
{"wap-wsg-idm-ecid-wtls1","wap-wsg-idm-ecid-wtls1",
	NID_wap_wsg_idm_ecid_wtls1,5,&(lvalues[4936]),0},
{"wap-wsg-idm-ecid-wtls3","wap-wsg-idm-ecid-wtls3",
	NID_wap_wsg_idm_ecid_wtls3,5,&(lvalues[4941]),0},
{"wap-wsg-idm-ecid-wtls4","wap-wsg-idm-ecid-wtls4",
	NID_wap_wsg_idm_ecid_wtls4,5,&(lvalues[4946]),0},
{"wap-wsg-idm-ecid-wtls5","wap-wsg-idm-ecid-wtls5",
	NID_wap_wsg_idm_ecid_wtls5,5,&(lvalues[4951]),0},
{"wap-wsg-idm-ecid-wtls6","wap-wsg-idm-ecid-wtls6",
	NID_wap_wsg_idm_ecid_wtls6,5,&(lvalues[4956]),0},
{"wap-wsg-idm-ecid-wtls7","wap-wsg-idm-ecid-wtls7",
	NID_wap_wsg_idm_ecid_wtls7,5,&(lvalues[4961]),0},
{"wap-wsg-idm-ecid-wtls8","wap-wsg-idm-ecid-wtls8",
	NID_wap_wsg_idm_ecid_wtls8,5,&(lvalues[4966]),0},
{"wap-wsg-idm-ecid-wtls9","wap-wsg-idm-ecid-wtls9",
	NID_wap_wsg_idm_ecid_wtls9,5,&(lvalues[4971]),0},
{"wap-wsg-idm-ecid-wtls10","wap-wsg-idm-ecid-wtls10",
	NID_wap_wsg_idm_ecid_wtls10,5,&(lvalues[4976]),0},
{"wap-wsg-idm-ecid-wtls11","wap-wsg-idm-ecid-wtls11",
	NID_wap_wsg_idm_ecid_wtls11,5,&(lvalues[4981]),0},
{"wap-wsg-idm-ecid-wtls12","wap-wsg-idm-ecid-wtls12",
	NID_wap_wsg_idm_ecid_wtls12,5,&(lvalues[4986]),0},
{"anyPolicy","X509v3 Any Policy",NID_any_policy,4,&(lvalues[4991]),0},
{"policyMappings","X509v3 Policy Mappings",NID_policy_mappings,3,
	&(lvalues[4995]),0},
{"inhibitAnyPolicy","X509v3 Inhibit Any Policy",
	NID_inhibit_any_policy,3,&(lvalues[4998]),0},
{"Oakley-EC2N-3","ipsec3",NID_ipsec3,0,NULL,0},
{"Oakley-EC2N-4","ipsec4",NID_ipsec4,0,NULL,0},
{"CAMELLIA-128-CBC","camellia-128-cbc",NID_camellia_128_cbc,11,
	&(lvalues[5001]),0},
{"CAMELLIA-192-CBC","camellia-192-cbc",NID_camellia_192_cbc,11,
	&(lvalues[5012]),0},
{"CAMELLIA-256-CBC","camellia-256-cbc",NID_camellia_256_cbc,11,
	&(lvalues[5023]),0},
{"CAMELLIA-128-ECB","camellia-128-ecb",NID_camellia_128_ecb,8,
	&(lvalues[5034]),0},
{"CAMELLIA-192-ECB","camellia-192-ecb",NID_camellia_192_ecb,8,
	&(lvalues[5042]),0},
{"CAMELLIA-256-ECB","camellia-256-ecb",NID_camellia_256_ecb,8,
	&(lvalues[5050]),0},
{"CAMELLIA-128-CFB","camellia-128-cfb",NID_camellia_128_cfb128,8,
	&(lvalues[5058]),0},
{"CAMELLIA-192-CFB","camellia-192-cfb",NID_camellia_192_cfb128,8,
	&(lvalues[5066]),0},
{"CAMELLIA-256-CFB","camellia-256-cfb",NID_camellia_256_cfb128,8,
	&(lvalues[5074]),0},
{"CAMELLIA-128-CFB1","camellia-128-cfb1",NID_camellia_128_cfb1,0,NULL,0},
{"CAMELLIA-192-CFB1","camellia-192-cfb1",NID_camellia_192_cfb1,0,NULL,0},
{"CAMELLIA-256-CFB1","camellia-256-cfb1",NID_camellia_256_cfb1,0,NULL,0},
{"CAMELLIA-128-CFB8","camellia-128-cfb8",NID_camellia_128_cfb8,0,NULL,0},
{"CAMELLIA-192-CFB8","camellia-192-cfb8",NID_camellia_192_cfb8,0,NULL,0},
{"CAMELLIA-256-CFB8","camellia-256-cfb8",NID_camellia_256_cfb8,0,NULL,0},
{"CAMELLIA-128-OFB","camellia-128-ofb",NID_camellia_128_ofb128,8,
	&(lvalues[5082]),0},
{"CAMELLIA-192-OFB","camellia-192-ofb",NID_camellia_192_ofb128,8,
	&(lvalues[5090]),0},
{"CAMELLIA-256-OFB","camellia-256-ofb",NID_camellia_256_ofb128,8,
	&(lvalues[5098]),0},
{"subjectDirectoryAttributes","X509v3 Subject Directory Attributes",
	NID_subject_directory_attributes,3,&(lvalues[5106]),0},
{"issuingDistributionPoint","X509v3 Issuing Distrubution Point",
	NID_issuing_distribution_point,3,&(lvalues[5109]),0},
{"certificateIssuer","X509v3 Certificate Issuer",
	NID_certificate_issuer,3,&(lvalues[5112]),0},
{NULL,NULL,NID_undef,0,NULL,0},
{"KISA","kisa",NID_kisa,6,&(lvalues[5115]),0},
{NULL,NULL,NID_undef,0,NULL,0},
{NULL,NULL,NID_undef,0,NULL,0},
{"SEED-ECB","seed-ecb",NID_seed_ecb,8,&(lvalues[5121]),0},
{"SEED-CBC","seed-cbc",NID_seed_cbc,8,&(lvalues[5129]),0},
{"SEED-OFB","seed-ofb",NID_seed_ofb128,8,&(lvalues[5137]),0},
{"SEED-CFB","seed-cfb",NID_seed_cfb128,8,&(lvalues[5145]),0},
};

static ASN1_OBJECT *sn_objs[NUM_SN]={
&(nid_objs[364]),/* "AD_DVCS" */
&(nid_objs[419]),/* "AES-128-CBC" */
&(nid_objs[421]),/* "AES-128-CFB" */
&(nid_objs[650]),/* "AES-128-CFB1" */
&(nid_objs[653]),/* "AES-128-CFB8" */
&(nid_objs[418]),/* "AES-128-ECB" */
&(nid_objs[420]),/* "AES-128-OFB" */
&(nid_objs[423]),/* "AES-192-CBC" */
&(nid_objs[425]),/* "AES-192-CFB" */
&(nid_objs[651]),/* "AES-192-CFB1" */
&(nid_objs[654]),/* "AES-192-CFB8" */
&(nid_objs[422]),/* "AES-192-ECB" */
&(nid_objs[424]),/* "AES-192-OFB" */
&(nid_objs[427]),/* "AES-256-CBC" */
&(nid_objs[429]),/* "AES-256-CFB" */
&(nid_objs[652]),/* "AES-256-CFB1" */
&(nid_objs[655]),/* "AES-256-CFB8" */
&(nid_objs[426]),/* "AES-256-ECB" */
&(nid_objs[428]),/* "AES-256-OFB" */
&(nid_objs[91]),/* "BF-CBC" */
&(nid_objs[93]),/* "BF-CFB" */
&(nid_objs[92]),/* "BF-ECB" */
&(nid_objs[94]),/* "BF-OFB" */
&(nid_objs[14]),/* "C" */
&(nid_objs[751]),/* "CAMELLIA-128-CBC" */
&(nid_objs[757]),/* "CAMELLIA-128-CFB" */
&(nid_objs[760]),/* "CAMELLIA-128-CFB1" */
&(nid_objs[763]),/* "CAMELLIA-128-CFB8" */
&(nid_objs[754]),/* "CAMELLIA-128-ECB" */
&(nid_objs[766]),/* "CAMELLIA-128-OFB" */
&(nid_objs[752]),/* "CAMELLIA-192-CBC" */
&(nid_objs[758]),/* "CAMELLIA-192-CFB" */
&(nid_objs[761]),/* "CAMELLIA-192-CFB1" */
&(nid_objs[764]),/* "CAMELLIA-192-CFB8" */
&(nid_objs[755]),/* "CAMELLIA-192-ECB" */
&(nid_objs[767]),/* "CAMELLIA-192-OFB" */
&(nid_objs[753]),/* "CAMELLIA-256-CBC" */
&(nid_objs[759]),/* "CAMELLIA-256-CFB" */
&(nid_objs[762]),/* "CAMELLIA-256-CFB1" */
&(nid_objs[765]),/* "CAMELLIA-256-CFB8" */
&(nid_objs[756]),/* "CAMELLIA-256-ECB" */
&(nid_objs[768]),/* "CAMELLIA-256-OFB" */
&(nid_objs[108]),/* "CAST5-CBC" */
&(nid_objs[110]),/* "CAST5-CFB" */
&(nid_objs[109]),/* "CAST5-ECB" */
&(nid_objs[111]),/* "CAST5-OFB" */
&(nid_objs[13]),/* "CN" */
&(nid_objs[141]),/* "CRLReason" */
&(nid_objs[417]),/* "CSPName" */
&(nid_objs[367]),/* "CrlID" */
&(nid_objs[391]),/* "DC" */
&(nid_objs[31]),/* "DES-CBC" */
&(nid_objs[643]),/* "DES-CDMF" */
&(nid_objs[30]),/* "DES-CFB" */
&(nid_objs[656]),/* "DES-CFB1" */
&(nid_objs[657]),/* "DES-CFB8" */
&(nid_objs[29]),/* "DES-ECB" */
&(nid_objs[32]),/* "DES-EDE" */
&(nid_objs[43]),/* "DES-EDE-CBC" */
&(nid_objs[60]),/* "DES-EDE-CFB" */
&(nid_objs[62]),/* "DES-EDE-OFB" */
&(nid_objs[33]),/* "DES-EDE3" */
&(nid_objs[44]),/* "DES-EDE3-CBC" */
&(nid_objs[61]),/* "DES-EDE3-CFB" */
&(nid_objs[658]),/* "DES-EDE3-CFB1" */
&(nid_objs[659]),/* "DES-EDE3-CFB8" */
&(nid_objs[63]),/* "DES-EDE3-OFB" */
&(nid_objs[45]),/* "DES-OFB" */
&(nid_objs[80]),/* "DESX-CBC" */
&(nid_objs[380]),/* "DOD" */
&(nid_objs[116]),/* "DSA" */
&(nid_objs[66]),/* "DSA-SHA" */
&(nid_objs[113]),/* "DSA-SHA1" */
&(nid_objs[70]),/* "DSA-SHA1-old" */
&(nid_objs[67]),/* "DSA-old" */
&(nid_objs[297]),/* "DVCS" */
&(nid_objs[99]),/* "GN" */
&(nid_objs[381]),/* "IANA" */
&(nid_objs[34]),/* "IDEA-CBC" */
&(nid_objs[35]),/* "IDEA-CFB" */
&(nid_objs[36]),/* "IDEA-ECB" */
&(nid_objs[46]),/* "IDEA-OFB" */
&(nid_objs[181]),/* "ISO" */
&(nid_objs[183]),/* "ISO-US" */
&(nid_objs[645]),/* "ITU-T" */
&(nid_objs[646]),/* "JOINT-ISO-ITU-T" */
&(nid_objs[773]),/* "KISA" */
&(nid_objs[15]),/* "L" */
&(nid_objs[ 3]),/* "MD2" */
&(nid_objs[257]),/* "MD4" */
&(nid_objs[ 4]),/* "MD5" */
&(nid_objs[114]),/* "MD5-SHA1" */
&(nid_objs[95]),/* "MDC2" */
&(nid_objs[388]),/* "Mail" */
&(nid_objs[393]),/* "NULL" */
&(nid_objs[404]),/* "NULL" */
&(nid_objs[57]),/* "Netscape" */
&(nid_objs[366]),/* "Nonce" */
&(nid_objs[17]),/* "O" */
&(nid_objs[178]),/* "OCSP" */
&(nid_objs[180]),/* "OCSPSigning" */
&(nid_objs[379]),/* "ORG" */
&(nid_objs[18]),/* "OU" */
&(nid_objs[749]),/* "Oakley-EC2N-3" */
&(nid_objs[750]),/* "Oakley-EC2N-4" */
&(nid_objs[ 9]),/* "PBE-MD2-DES" */
&(nid_objs[168]),/* "PBE-MD2-RC2-64" */
&(nid_objs[10]),/* "PBE-MD5-DES" */
&(nid_objs[169]),/* "PBE-MD5-RC2-64" */
&(nid_objs[147]),/* "PBE-SHA1-2DES" */
&(nid_objs[146]),/* "PBE-SHA1-3DES" */
&(nid_objs[170]),/* "PBE-SHA1-DES" */
&(nid_objs[148]),/* "PBE-SHA1-RC2-128" */
&(nid_objs[149]),/* "PBE-SHA1-RC2-40" */
&(nid_objs[68]),/* "PBE-SHA1-RC2-64" */
&(nid_objs[144]),/* "PBE-SHA1-RC4-128" */
&(nid_objs[145]),/* "PBE-SHA1-RC4-40" */
&(nid_objs[161]),/* "PBES2" */
&(nid_objs[69]),/* "PBKDF2" */
&(nid_objs[162]),/* "PBMAC1" */
&(nid_objs[127]),/* "PKIX" */
&(nid_objs[98]),/* "RC2-40-CBC" */
&(nid_objs[166]),/* "RC2-64-CBC" */
&(nid_objs[37]),/* "RC2-CBC" */
&(nid_objs[39]),/* "RC2-CFB" */
&(nid_objs[38]),/* "RC2-ECB" */
&(nid_objs[40]),/* "RC2-OFB" */
&(nid_objs[ 5]),/* "RC4" */
&(nid_objs[97]),/* "RC4-40" */
&(nid_objs[120]),/* "RC5-CBC" */
&(nid_objs[122]),/* "RC5-CFB" */
&(nid_objs[121]),/* "RC5-ECB" */
&(nid_objs[123]),/* "RC5-OFB" */
&(nid_objs[117]),/* "RIPEMD160" */
&(nid_objs[124]),/* "RLE" */
&(nid_objs[19]),/* "RSA" */
&(nid_objs[ 7]),/* "RSA-MD2" */
&(nid_objs[396]),/* "RSA-MD4" */
&(nid_objs[ 8]),/* "RSA-MD5" */
&(nid_objs[96]),/* "RSA-MDC2" */
&(nid_objs[104]),/* "RSA-NP-MD5" */
&(nid_objs[119]),/* "RSA-RIPEMD160" */
&(nid_objs[42]),/* "RSA-SHA" */
&(nid_objs[65]),/* "RSA-SHA1" */
&(nid_objs[115]),/* "RSA-SHA1-2" */
&(nid_objs[671]),/* "RSA-SHA224" */
&(nid_objs[668]),/* "RSA-SHA256" */
&(nid_objs[669]),/* "RSA-SHA384" */
&(nid_objs[670]),/* "RSA-SHA512" */
&(nid_objs[777]),/* "SEED-CBC" */
&(nid_objs[779]),/* "SEED-CFB" */
&(nid_objs[776]),/* "SEED-ECB" */
&(nid_objs[778]),/* "SEED-OFB" */
&(nid_objs[41]),/* "SHA" */
&(nid_objs[64]),/* "SHA1" */
&(nid_objs[675]),/* "SHA224" */
&(nid_objs[672]),/* "SHA256" */
&(nid_objs[673]),/* "SHA384" */
&(nid_objs[674]),/* "SHA512" */
&(nid_objs[188]),/* "SMIME" */
&(nid_objs[167]),/* "SMIME-CAPS" */
&(nid_objs[100]),/* "SN" */
&(nid_objs[16]),/* "ST" */
&(nid_objs[143]),/* "SXNetID" */
&(nid_objs[458]),/* "UID" */
&(nid_objs[ 0]),/* "UNDEF" */
&(nid_objs[11]),/* "X500" */
&(nid_objs[378]),/* "X500algorithms" */
&(nid_objs[12]),/* "X509" */
&(nid_objs[184]),/* "X9-57" */
&(nid_objs[185]),/* "X9cm" */
&(nid_objs[125]),/* "ZLIB" */
&(nid_objs[478]),/* "aRecord" */
&(nid_objs[289]),/* "aaControls" */
&(nid_objs[287]),/* "ac-auditEntity" */
&(nid_objs[397]),/* "ac-proxying" */
&(nid_objs[288]),/* "ac-targeting" */
&(nid_objs[368]),/* "acceptableResponses" */
&(nid_objs[446]),/* "account" */
&(nid_objs[363]),/* "ad_timestamping" */
&(nid_objs[376]),/* "algorithm" */
&(nid_objs[405]),/* "ansi-X9-62" */
&(nid_objs[746]),/* "anyPolicy" */
&(nid_objs[370]),/* "archiveCutoff" */
&(nid_objs[484]),/* "associatedDomain" */
&(nid_objs[485]),/* "associatedName" */
&(nid_objs[501]),/* "audio" */
&(nid_objs[177]),/* "authorityInfoAccess" */
&(nid_objs[90]),/* "authorityKeyIdentifier" */
&(nid_objs[87]),/* "basicConstraints" */
&(nid_objs[365]),/* "basicOCSPResponse" */
&(nid_objs[285]),/* "biometricInfo" */
&(nid_objs[494]),/* "buildingName" */
&(nid_objs[691]),/* "c2onb191v4" */
&(nid_objs[692]),/* "c2onb191v5" */
&(nid_objs[697]),/* "c2onb239v4" */
&(nid_objs[698]),/* "c2onb239v5" */
&(nid_objs[684]),/* "c2pnb163v1" */
&(nid_objs[685]),/* "c2pnb163v2" */
&(nid_objs[686]),/* "c2pnb163v3" */
&(nid_objs[687]),/* "c2pnb176v1" */
&(nid_objs[693]),/* "c2pnb208w1" */
&(nid_objs[699]),/* "c2pnb272w1" */
&(nid_objs[700]),/* "c2pnb304w1" */
&(nid_objs[702]),/* "c2pnb368w1" */
&(nid_objs[688]),/* "c2tnb191v1" */
&(nid_objs[689]),/* "c2tnb191v2" */
&(nid_objs[690]),/* "c2tnb191v3" */
&(nid_objs[694]),/* "c2tnb239v1" */
&(nid_objs[695]),/* "c2tnb239v2" */
&(nid_objs[696]),/* "c2tnb239v3" */
&(nid_objs[701]),/* "c2tnb359v1" */
&(nid_objs[703]),/* "c2tnb431r1" */
&(nid_objs[483]),/* "cNAMERecord" */
&(nid_objs[179]),/* "caIssuers" */
&(nid_objs[443]),/* "caseIgnoreIA5StringSyntax" */
&(nid_objs[152]),/* "certBag" */
&(nid_objs[677]),/* "certicom-arc" */
&(nid_objs[771]),/* "certificateIssuer" */
&(nid_objs[89]),/* "certificatePolicies" */
&(nid_objs[54]),/* "challengePassword" */
&(nid_objs[407]),/* "characteristic-two-field" */
&(nid_objs[395]),/* "clearance" */
&(nid_objs[130]),/* "clientAuth" */
&(nid_objs[131]),/* "codeSigning" */
&(nid_objs[50]),/* "contentType" */
&(nid_objs[53]),/* "countersignature" */
&(nid_objs[153]),/* "crlBag" */
&(nid_objs[103]),/* "crlDistributionPoints" */
&(nid_objs[88]),/* "crlNumber" */
&(nid_objs[500]),/* "dITRedirect" */
&(nid_objs[451]),/* "dNSDomain" */
&(nid_objs[495]),/* "dSAQuality" */
&(nid_objs[434]),/* "data" */
&(nid_objs[390]),/* "dcobject" */
&(nid_objs[140]),/* "deltaCRL" */
&(nid_objs[107]),/* "description" */
&(nid_objs[28]),/* "dhKeyAgreement" */
&(nid_objs[382]),/* "directory" */
&(nid_objs[174]),/* "dnQualifier" */
&(nid_objs[447]),/* "document" */
&(nid_objs[471]),/* "documentAuthor" */
&(nid_objs[468]),/* "documentIdentifier" */
&(nid_objs[472]),/* "documentLocation" */
&(nid_objs[502]),/* "documentPublisher" */
&(nid_objs[449]),/* "documentSeries" */
&(nid_objs[469]),/* "documentTitle" */
&(nid_objs[470]),/* "documentVersion" */
&(nid_objs[392]),/* "domain" */
&(nid_objs[452]),/* "domainRelatedObject" */
&(nid_objs[416]),/* "ecdsa-with-SHA1" */
&(nid_objs[48]),/* "emailAddress" */
&(nid_objs[132]),/* "emailProtection" */
&(nid_objs[389]),/* "enterprises" */
&(nid_objs[384]),/* "experimental" */
&(nid_objs[172]),/* "extReq" */
&(nid_objs[56]),/* "extendedCertificateAttributes" */
&(nid_objs[126]),/* "extendedKeyUsage" */
&(nid_objs[372]),/* "extendedStatus" */
&(nid_objs[462]),/* "favouriteDrink" */
&(nid_objs[453]),/* "friendlyCountry" */
&(nid_objs[490]),/* "friendlyCountryName" */
&(nid_objs[156]),/* "friendlyName" */
&(nid_objs[509]),/* "generationQualifier" */
&(nid_objs[163]),/* "hmacWithSHA1" */
&(nid_objs[432]),/* "holdInstructionCallIssuer" */
&(nid_objs[430]),/* "holdInstructionCode" */
&(nid_objs[431]),/* "holdInstructionNone" */
&(nid_objs[433]),/* "holdInstructionReject" */
&(nid_objs[486]),/* "homePostalAddress" */
&(nid_objs[473]),/* "homeTelephoneNumber" */
&(nid_objs[466]),/* "host" */
&(nid_objs[442]),/* "iA5StringSyntax" */
&(nid_objs[266]),/* "id-aca" */
&(nid_objs[355]),/* "id-aca-accessIdentity" */
&(nid_objs[354]),/* "id-aca-authenticationInfo" */
&(nid_objs[356]),/* "id-aca-chargingIdentity" */
&(nid_objs[399]),/* "id-aca-encAttrs" */
&(nid_objs[357]),/* "id-aca-group" */
&(nid_objs[358]),/* "id-aca-role" */
&(nid_objs[176]),/* "id-ad" */
&(nid_objs[262]),/* "id-alg" */
&(nid_objs[323]),/* "id-alg-des40" */
&(nid_objs[326]),/* "id-alg-dh-pop" */
&(nid_objs[325]),/* "id-alg-dh-sig-hmac-sha1" */
&(nid_objs[324]),/* "id-alg-noSignature" */
&(nid_objs[268]),/* "id-cct" */
&(nid_objs[361]),/* "id-cct-PKIData" */
&(nid_objs[362]),/* "id-cct-PKIResponse" */
&(nid_objs[360]),/* "id-cct-crs" */
&(nid_objs[81]),/* "id-ce" */
&(nid_objs[680]),/* "id-characteristic-two-basis" */
&(nid_objs[263]),/* "id-cmc" */
&(nid_objs[334]),/* "id-cmc-addExtensions" */
&(nid_objs[346]),/* "id-cmc-confirmCertAcceptance" */
&(nid_objs[330]),/* "id-cmc-dataReturn" */
&(nid_objs[336]),/* "id-cmc-decryptedPOP" */
&(nid_objs[335]),/* "id-cmc-encryptedPOP" */
&(nid_objs[339]),/* "id-cmc-getCRL" */
&(nid_objs[338]),/* "id-cmc-getCert" */
&(nid_objs[328]),/* "id-cmc-identification" */
&(nid_objs[329]),/* "id-cmc-identityProof" */
&(nid_objs[337]),/* "id-cmc-lraPOPWitness" */
&(nid_objs[344]),/* "id-cmc-popLinkRandom" */
&(nid_objs[345]),/* "id-cmc-popLinkWitness" */
&(nid_objs[343]),/* "id-cmc-queryPending" */
&(nid_objs[333]),/* "id-cmc-recipientNonce" */
&(nid_objs[341]),/* "id-cmc-regInfo" */
&(nid_objs[342]),/* "id-cmc-responseInfo" */
&(nid_objs[340]),/* "id-cmc-revokeRequest" */
&(nid_objs[332]),/* "id-cmc-senderNonce" */
&(nid_objs[327]),/* "id-cmc-statusInfo" */
&(nid_objs[331]),/* "id-cmc-transactionId" */
&(nid_objs[408]),/* "id-ecPublicKey" */
&(nid_objs[508]),/* "id-hex-multipart-message" */
&(nid_objs[507]),/* "id-hex-partial-message" */
&(nid_objs[260]),/* "id-it" */
&(nid_objs[302]),/* "id-it-caKeyUpdateInfo" */
&(nid_objs[298]),/* "id-it-caProtEncCert" */
&(nid_objs[311]),/* "id-it-confirmWaitTime" */
&(nid_objs[303]),/* "id-it-currentCRL" */
&(nid_objs[300]),/* "id-it-encKeyPairTypes" */
&(nid_objs[310]),/* "id-it-implicitConfirm" */
&(nid_objs[308]),/* "id-it-keyPairParamRep" */
&(nid_objs[307]),/* "id-it-keyPairParamReq" */
&(nid_objs[312]),/* "id-it-origPKIMessage" */
&(nid_objs[301]),/* "id-it-preferredSymmAlg" */
&(nid_objs[309]),/* "id-it-revPassphrase" */
&(nid_objs[299]),/* "id-it-signKeyPairTypes" */
&(nid_objs[305]),/* "id-it-subscriptionRequest" */
&(nid_objs[306]),/* "id-it-subscriptionResponse" */
&(nid_objs[304]),/* "id-it-unsupportedOIDs" */
&(nid_objs[128]),/* "id-kp" */
&(nid_objs[280]),/* "id-mod-attribute-cert" */
&(nid_objs[274]),/* "id-mod-cmc" */
&(nid_objs[277]),/* "id-mod-cmp" */
&(nid_objs[284]),/* "id-mod-cmp2000" */
&(nid_objs[273]),/* "id-mod-crmf" */
&(nid_objs[283]),/* "id-mod-dvcs" */
&(nid_objs[275]),/* "id-mod-kea-profile-88" */
&(nid_objs[276]),/* "id-mod-kea-profile-93" */
&(nid_objs[282]),/* "id-mod-ocsp" */
&(nid_objs[278]),/* "id-mod-qualified-cert-88" */
&(nid_objs[279]),/* "id-mod-qualified-cert-93" */
&(nid_objs[281]),/* "id-mod-timestamp-protocol" */
&(nid_objs[264]),/* "id-on" */
&(nid_objs[347]),/* "id-on-personalData" */
&(nid_objs[265]),/* "id-pda" */
&(nid_objs[352]),/* "id-pda-countryOfCitizenship" */
&(nid_objs[353]),/* "id-pda-countryOfResidence" */
&(nid_objs[348]),/* "id-pda-dateOfBirth" */
&(nid_objs[351]),/* "id-pda-gender" */
&(nid_objs[349]),/* "id-pda-placeOfBirth" */
&(nid_objs[175]),/* "id-pe" */
&(nid_objs[261]),/* "id-pkip" */
&(nid_objs[258]),/* "id-pkix-mod" */
&(nid_objs[269]),/* "id-pkix1-explicit-88" */
&(nid_objs[271]),/* "id-pkix1-explicit-93" */
&(nid_objs[270]),/* "id-pkix1-implicit-88" */
&(nid_objs[272]),/* "id-pkix1-implicit-93" */
&(nid_objs[662]),/* "id-ppl" */
&(nid_objs[664]),/* "id-ppl-anyLanguage" */
&(nid_objs[667]),/* "id-ppl-independent" */
&(nid_objs[665]),/* "id-ppl-inheritAll" */
&(nid_objs[267]),/* "id-qcs" */
&(nid_objs[359]),/* "id-qcs-pkixQCSyntax-v1" */
&(nid_objs[259]),/* "id-qt" */
&(nid_objs[164]),/* "id-qt-cps" */
&(nid_objs[165]),/* "id-qt-unotice" */
&(nid_objs[313]),/* "id-regCtrl" */
&(nid_objs[316]),/* "id-regCtrl-authenticator" */
&(nid_objs[319]),/* "id-regCtrl-oldCertID" */
&(nid_objs[318]),/* "id-regCtrl-pkiArchiveOptions" */
&(nid_objs[317]),/* "id-regCtrl-pkiPublicationInfo" */
&(nid_objs[320]),/* "id-regCtrl-protocolEncrKey" */
&(nid_objs[315]),/* "id-regCtrl-regToken" */
&(nid_objs[314]),/* "id-regInfo" */
&(nid_objs[322]),/* "id-regInfo-certReq" */
&(nid_objs[321]),/* "id-regInfo-utf8Pairs" */
&(nid_objs[512]),/* "id-set" */
&(nid_objs[191]),/* "id-smime-aa" */
&(nid_objs[215]),/* "id-smime-aa-contentHint" */
&(nid_objs[218]),/* "id-smime-aa-contentIdentifier" */
&(nid_objs[221]),/* "id-smime-aa-contentReference" */
&(nid_objs[240]),/* "id-smime-aa-dvcs-dvc" */
&(nid_objs[217]),/* "id-smime-aa-encapContentType" */
&(nid_objs[222]),/* "id-smime-aa-encrypKeyPref" */
&(nid_objs[220]),/* "id-smime-aa-equivalentLabels" */
&(nid_objs[232]),/* "id-smime-aa-ets-CertificateRefs" */
&(nid_objs[233]),/* "id-smime-aa-ets-RevocationRefs" */
&(nid_objs[238]),/* "id-smime-aa-ets-archiveTimeStamp" */
&(nid_objs[237]),/* "id-smime-aa-ets-certCRLTimestamp" */
&(nid_objs[234]),/* "id-smime-aa-ets-certValues" */
&(nid_objs[227]),/* "id-smime-aa-ets-commitmentType" */
&(nid_objs[231]),/* "id-smime-aa-ets-contentTimestamp" */
&(nid_objs[236]),/* "id-smime-aa-ets-escTimeStamp" */
&(nid_objs[230]),/* "id-smime-aa-ets-otherSigCert" */
&(nid_objs[235]),/* "id-smime-aa-ets-revocationValues" */
&(nid_objs[226]),/* "id-smime-aa-ets-sigPolicyId" */
&(nid_objs[229]),/* "id-smime-aa-ets-signerAttr" */
&(nid_objs[228]),/* "id-smime-aa-ets-signerLocation" */
&(nid_objs[219]),/* "id-smime-aa-macValue" */
&(nid_objs[214]),/* "id-smime-aa-mlExpandHistory" */
&(nid_objs[216]),/* "id-smime-aa-msgSigDigest" */
&(nid_objs[212]),/* "id-smime-aa-receiptRequest" */
&(nid_objs[213]),/* "id-smime-aa-securityLabel" */
&(nid_objs[239]),/* "id-smime-aa-signatureType" */
&(nid_objs[223]),/* "id-smime-aa-signingCertificate" */
&(nid_objs[224]),/* "id-smime-aa-smimeEncryptCerts" */
&(nid_objs[225]),/* "id-smime-aa-timeStampToken" */
&(nid_objs[192]),/* "id-smime-alg" */
&(nid_objs[243]),/* "id-smime-alg-3DESwrap" */
&(nid_objs[246]),/* "id-smime-alg-CMS3DESwrap" */
&(nid_objs[247]),/* "id-smime-alg-CMSRC2wrap" */
&(nid_objs[245]),/* "id-smime-alg-ESDH" */
&(nid_objs[241]),/* "id-smime-alg-ESDHwith3DES" */
&(nid_objs[242]),/* "id-smime-alg-ESDHwithRC2" */
&(nid_objs[244]),/* "id-smime-alg-RC2wrap" */
&(nid_objs[193]),/* "id-smime-cd" */
&(nid_objs[248]),/* "id-smime-cd-ldap" */
&(nid_objs[190]),/* "id-smime-ct" */
&(nid_objs[210]),/* "id-smime-ct-DVCSRequestData" */
&(nid_objs[211]),/* "id-smime-ct-DVCSResponseData" */
&(nid_objs[208]),/* "id-smime-ct-TDTInfo" */
&(nid_objs[207]),/* "id-smime-ct-TSTInfo" */
&(nid_objs[205]),/* "id-smime-ct-authData" */
&(nid_objs[209]),/* "id-smime-ct-contentInfo" */
&(nid_objs[206]),/* "id-smime-ct-publishCert" */
&(nid_objs[204]),/* "id-smime-ct-receipt" */
&(nid_objs[195]),/* "id-smime-cti" */
&(nid_objs[255]),/* "id-smime-cti-ets-proofOfApproval" */
&(nid_objs[256]),/* "id-smime-cti-ets-proofOfCreation" */
&(nid_objs[253]),/* "id-smime-cti-ets-proofOfDelivery" */
&(nid_objs[251]),/* "id-smime-cti-ets-proofOfOrigin" */
&(nid_objs[252]),/* "id-smime-cti-ets-proofOfReceipt" */
&(nid_objs[254]),/* "id-smime-cti-ets-proofOfSender" */
&(nid_objs[189]),/* "id-smime-mod" */
&(nid_objs[196]),/* "id-smime-mod-cms" */
&(nid_objs[197]),/* "id-smime-mod-ess" */
&(nid_objs[202]),/* "id-smime-mod-ets-eSigPolicy-88" */
&(nid_objs[203]),/* "id-smime-mod-ets-eSigPolicy-97" */
&(nid_objs[200]),/* "id-smime-mod-ets-eSignature-88" */
&(nid_objs[201]),/* "id-smime-mod-ets-eSignature-97" */
&(nid_objs[199]),/* "id-smime-mod-msg-v3" */
&(nid_objs[198]),/* "id-smime-mod-oid" */
&(nid_objs[194]),/* "id-smime-spq" */
&(nid_objs[250]),/* "id-smime-spq-ets-sqt-unotice" */
&(nid_objs[249]),/* "id-smime-spq-ets-sqt-uri" */
&(nid_objs[676]),/* "identified-organization" */
&(nid_objs[461]),/* "info" */
&(nid_objs[748]),/* "inhibitAnyPolicy" */
&(nid_objs[101]),/* "initials" */
&(nid_objs[647]),/* "international-organizations" */
&(nid_objs[142]),/* "invalidityDate" */
&(nid_objs[294]),/* "ipsecEndSystem" */
&(nid_objs[295]),/* "ipsecTunnel" */
&(nid_objs[296]),/* "ipsecUser" */
&(nid_objs[86]),/* "issuerAltName" */
&(nid_objs[770]),/* "issuingDistributionPoint" */
&(nid_objs[492]),/* "janetMailbox" */
&(nid_objs[150]),/* "keyBag" */
&(nid_objs[83]),/* "keyUsage" */
&(nid_objs[477]),/* "lastModifiedBy" */
&(nid_objs[476]),/* "lastModifiedTime" */
&(nid_objs[157]),/* "localKeyID" */
&(nid_objs[480]),/* "mXRecord" */
&(nid_objs[460]),/* "mail" */
&(nid_objs[493]),/* "mailPreferenceOption" */
&(nid_objs[467]),/* "manager" */
&(nid_objs[182]),/* "member-body" */
&(nid_objs[51]),/* "messageDigest" */
&(nid_objs[383]),/* "mgmt" */
&(nid_objs[504]),/* "mime-mhs" */
&(nid_objs[506]),/* "mime-mhs-bodies" */
&(nid_objs[505]),/* "mime-mhs-headings" */
&(nid_objs[488]),/* "mobileTelephoneNumber" */
&(nid_objs[136]),/* "msCTLSign" */
&(nid_objs[135]),/* "msCodeCom" */
&(nid_objs[134]),/* "msCodeInd" */
&(nid_objs[138]),/* "msEFS" */
&(nid_objs[171]),/* "msExtReq" */
&(nid_objs[137]),/* "msSGC" */
&(nid_objs[648]),/* "msSmartcardLogin" */
&(nid_objs[649]),/* "msUPN" */
&(nid_objs[481]),/* "nSRecord" */
&(nid_objs[173]),/* "name" */
&(nid_objs[666]),/* "nameConstraints" */
&(nid_objs[369]),/* "noCheck" */
&(nid_objs[403]),/* "noRevAvail" */
&(nid_objs[72]),/* "nsBaseUrl" */
&(nid_objs[76]),/* "nsCaPolicyUrl" */
&(nid_objs[74]),/* "nsCaRevocationUrl" */
&(nid_objs[58]),/* "nsCertExt" */
&(nid_objs[79]),/* "nsCertSequence" */
&(nid_objs[71]),/* "nsCertType" */
&(nid_objs[78]),/* "nsComment" */
&(nid_objs[59]),/* "nsDataType" */
&(nid_objs[75]),/* "nsRenewalUrl" */
&(nid_objs[73]),/* "nsRevocationUrl" */
&(nid_objs[139]),/* "nsSGC" */
&(nid_objs[77]),/* "nsSslServerName" */
&(nid_objs[681]),/* "onBasis" */
&(nid_objs[491]),/* "organizationalStatus" */
&(nid_objs[475]),/* "otherMailbox" */
&(nid_objs[489]),/* "pagerTelephoneNumber" */
&(nid_objs[374]),/* "path" */
&(nid_objs[112]),/* "pbeWithMD5AndCast5CBC" */
&(nid_objs[499]),/* "personalSignature" */
&(nid_objs[487]),/* "personalTitle" */
&(nid_objs[464]),/* "photo" */
&(nid_objs[437]),/* "pilot" */
&(nid_objs[439]),/* "pilotAttributeSyntax" */
&(nid_objs[438]),/* "pilotAttributeType" */
&(nid_objs[479]),/* "pilotAttributeType27" */
&(nid_objs[456]),/* "pilotDSA" */
&(nid_objs[441]),/* "pilotGroups" */
&(nid_objs[444]),/* "pilotObject" */
&(nid_objs[440]),/* "pilotObjectClass" */
&(nid_objs[455]),/* "pilotOrganization" */
&(nid_objs[445]),/* "pilotPerson" */
&(nid_objs[ 2]),/* "pkcs" */
&(nid_objs[186]),/* "pkcs1" */
&(nid_objs[27]),/* "pkcs3" */
&(nid_objs[187]),/* "pkcs5" */
&(nid_objs[20]),/* "pkcs7" */
&(nid_objs[21]),/* "pkcs7-data" */
&(nid_objs[25]),/* "pkcs7-digestData" */
&(nid_objs[26]),/* "pkcs7-encryptedData" */
&(nid_objs[23]),/* "pkcs7-envelopedData" */
&(nid_objs[24]),/* "pkcs7-signedAndEnvelopedData" */
&(nid_objs[22]),/* "pkcs7-signedData" */
&(nid_objs[151]),/* "pkcs8ShroudedKeyBag" */
&(nid_objs[47]),/* "pkcs9" */
&(nid_objs[401]),/* "policyConstraints" */
&(nid_objs[747]),/* "policyMappings" */
&(nid_objs[661]),/* "postalCode" */
&(nid_objs[683]),/* "ppBasis" */
&(nid_objs[406]),/* "prime-field" */
&(nid_objs[409]),/* "prime192v1" */
&(nid_objs[410]),/* "prime192v2" */
&(nid_objs[411]),/* "prime192v3" */
&(nid_objs[412]),/* "prime239v1" */
&(nid_objs[413]),/* "prime239v2" */
&(nid_objs[414]),/* "prime239v3" */
&(nid_objs[415]),/* "prime256v1" */
&(nid_objs[385]),/* "private" */
&(nid_objs[84]),/* "privateKeyUsagePeriod" */
&(nid_objs[663]),/* "proxyCertInfo" */
&(nid_objs[510]),/* "pseudonym" */
&(nid_objs[435]),/* "pss" */
&(nid_objs[286]),/* "qcStatements" */
&(nid_objs[457]),/* "qualityLabelledData" */
&(nid_objs[450]),/* "rFC822localPart" */
&(nid_objs[400]),/* "role" */
&(nid_objs[448]),/* "room" */
&(nid_objs[463]),/* "roomNumber" */
&(nid_objs[ 6]),/* "rsaEncryption" */
&(nid_objs[644]),/* "rsaOAEPEncryptionSET" */
&(nid_objs[377]),/* "rsaSignature" */
&(nid_objs[ 1]),/* "rsadsi" */
&(nid_objs[482]),/* "sOARecord" */
&(nid_objs[155]),/* "safeContentsBag" */
&(nid_objs[291]),/* "sbgp-autonomousSysNum" */
&(nid_objs[290]),/* "sbgp-ipAddrBlock" */
&(nid_objs[292]),/* "sbgp-routerIdentifier" */
&(nid_objs[159]),/* "sdsiCertificate" */
&(nid_objs[704]),/* "secp112r1" */
&(nid_objs[705]),/* "secp112r2" */
&(nid_objs[706]),/* "secp128r1" */
&(nid_objs[707]),/* "secp128r2" */
&(nid_objs[708]),/* "secp160k1" */
&(nid_objs[709]),/* "secp160r1" */
&(nid_objs[710]),/* "secp160r2" */
&(nid_objs[711]),/* "secp192k1" */
&(nid_objs[712]),/* "secp224k1" */
&(nid_objs[713]),/* "secp224r1" */
&(nid_objs[714]),/* "secp256k1" */
&(nid_objs[715]),/* "secp384r1" */
&(nid_objs[716]),/* "secp521r1" */
&(nid_objs[154]),/* "secretBag" */
&(nid_objs[474]),/* "secretary" */
&(nid_objs[717]),/* "sect113r1" */
&(nid_objs[718]),/* "sect113r2" */
&(nid_objs[719]),/* "sect131r1" */
&(nid_objs[720]),/* "sect131r2" */
&(nid_objs[721]),/* "sect163k1" */
&(nid_objs[722]),/* "sect163r1" */
&(nid_objs[723]),/* "sect163r2" */
&(nid_objs[724]),/* "sect193r1" */
&(nid_objs[725]),/* "sect193r2" */
&(nid_objs[726]),/* "sect233k1" */
&(nid_objs[727]),/* "sect233r1" */
&(nid_objs[728]),/* "sect239k1" */
&(nid_objs[729]),/* "sect283k1" */
&(nid_objs[730]),/* "sect283r1" */
&(nid_objs[731]),/* "sect409k1" */
&(nid_objs[732]),/* "sect409r1" */
&(nid_objs[733]),/* "sect571k1" */
&(nid_objs[734]),/* "sect571r1" */
&(nid_objs[386]),/* "security" */
&(nid_objs[394]),/* "selected-attribute-types" */
&(nid_objs[105]),/* "serialNumber" */
&(nid_objs[129]),/* "serverAuth" */
&(nid_objs[371]),/* "serviceLocator" */
&(nid_objs[625]),/* "set-addPolicy" */
&(nid_objs[515]),/* "set-attr" */
&(nid_objs[518]),/* "set-brand" */
&(nid_objs[638]),/* "set-brand-AmericanExpress" */
&(nid_objs[637]),/* "set-brand-Diners" */
&(nid_objs[636]),/* "set-brand-IATA-ATA" */
&(nid_objs[639]),/* "set-brand-JCB" */
&(nid_objs[641]),/* "set-brand-MasterCard" */
&(nid_objs[642]),/* "set-brand-Novus" */
&(nid_objs[640]),/* "set-brand-Visa" */
&(nid_objs[517]),/* "set-certExt" */
&(nid_objs[513]),/* "set-ctype" */
&(nid_objs[514]),/* "set-msgExt" */
&(nid_objs[516]),/* "set-policy" */
&(nid_objs[607]),/* "set-policy-root" */
&(nid_objs[624]),/* "set-rootKeyThumb" */
&(nid_objs[620]),/* "setAttr-Cert" */
&(nid_objs[631]),/* "setAttr-GenCryptgrm" */
&(nid_objs[623]),/* "setAttr-IssCap" */
&(nid_objs[628]),/* "setAttr-IssCap-CVM" */
&(nid_objs[630]),/* "setAttr-IssCap-Sig" */
&(nid_objs[629]),/* "setAttr-IssCap-T2" */
&(nid_objs[621]),/* "setAttr-PGWYcap" */
&(nid_objs[635]),/* "setAttr-SecDevSig" */
&(nid_objs[632]),/* "setAttr-T2Enc" */
&(nid_objs[633]),/* "setAttr-T2cleartxt" */
&(nid_objs[634]),/* "setAttr-TokICCsig" */
&(nid_objs[627]),/* "setAttr-Token-B0Prime" */
&(nid_objs[626]),/* "setAttr-Token-EMV" */
&(nid_objs[622]),/* "setAttr-TokenType" */
&(nid_objs[619]),/* "setCext-IssuerCapabilities" */
&(nid_objs[615]),/* "setCext-PGWYcapabilities" */
&(nid_objs[616]),/* "setCext-TokenIdentifier" */
&(nid_objs[618]),/* "setCext-TokenType" */
&(nid_objs[617]),/* "setCext-Track2Data" */
&(nid_objs[611]),/* "setCext-cCertRequired" */
&(nid_objs[609]),/* "setCext-certType" */
&(nid_objs[608]),/* "setCext-hashedRoot" */
&(nid_objs[610]),/* "setCext-merchData" */
&(nid_objs[613]),/* "setCext-setExt" */
&(nid_objs[614]),/* "setCext-setQualf" */
&(nid_objs[612]),/* "setCext-tunneling" */
&(nid_objs[540]),/* "setct-AcqCardCodeMsg" */
&(nid_objs[576]),/* "setct-AcqCardCodeMsgTBE" */
&(nid_objs[570]),/* "setct-AuthReqTBE" */
&(nid_objs[534]),/* "setct-AuthReqTBS" */
&(nid_objs[527]),/* "setct-AuthResBaggage" */
&(nid_objs[571]),/* "setct-AuthResTBE" */
&(nid_objs[572]),/* "setct-AuthResTBEX" */
&(nid_objs[535]),/* "setct-AuthResTBS" */
&(nid_objs[536]),/* "setct-AuthResTBSX" */
&(nid_objs[528]),/* "setct-AuthRevReqBaggage" */
&(nid_objs[577]),/* "setct-AuthRevReqTBE" */
&(nid_objs[541]),/* "setct-AuthRevReqTBS" */
&(nid_objs[529]),/* "setct-AuthRevResBaggage" */
&(nid_objs[542]),/* "setct-AuthRevResData" */
&(nid_objs[578]),/* "setct-AuthRevResTBE" */
&(nid_objs[579]),/* "setct-AuthRevResTBEB" */
&(nid_objs[543]),/* "setct-AuthRevResTBS" */
&(nid_objs[573]),/* "setct-AuthTokenTBE" */
&(nid_objs[537]),/* "setct-AuthTokenTBS" */
&(nid_objs[600]),/* "setct-BCIDistributionTBS" */
&(nid_objs[558]),/* "setct-BatchAdminReqData" */
&(nid_objs[592]),/* "setct-BatchAdminReqTBE" */
&(nid_objs[559]),/* "setct-BatchAdminResData" */
&(nid_objs[593]),/* "setct-BatchAdminResTBE" */
&(nid_objs[599]),/* "setct-CRLNotificationResTBS" */
&(nid_objs[598]),/* "setct-CRLNotificationTBS" */
&(nid_objs[580]),/* "setct-CapReqTBE" */
&(nid_objs[581]),/* "setct-CapReqTBEX" */
&(nid_objs[544]),/* "setct-CapReqTBS" */
&(nid_objs[545]),/* "setct-CapReqTBSX" */
&(nid_objs[546]),/* "setct-CapResData" */
&(nid_objs[582]),/* "setct-CapResTBE" */
&(nid_objs[583]),/* "setct-CapRevReqTBE" */
&(nid_objs[584]),/* "setct-CapRevReqTBEX" */
&(nid_objs[547]),/* "setct-CapRevReqTBS" */
&(nid_objs[548]),/* "setct-CapRevReqTBSX" */
&(nid_objs[549]),/* "setct-CapRevResData" */
&(nid_objs[585]),/* "setct-CapRevResTBE" */
&(nid_objs[538]),/* "setct-CapTokenData" */
&(nid_objs[530]),/* "setct-CapTokenSeq" */
&(nid_objs[574]),/* "setct-CapTokenTBE" */
&(nid_objs[575]),/* "setct-CapTokenTBEX" */
&(nid_objs[539]),/* "setct-CapTokenTBS" */
&(nid_objs[560]),/* "setct-CardCInitResTBS" */
&(nid_objs[566]),/* "setct-CertInqReqTBS" */
&(nid_objs[563]),/* "setct-CertReqData" */
&(nid_objs[595]),/* "setct-CertReqTBE" */
&(nid_objs[596]),/* "setct-CertReqTBEX" */
&(nid_objs[564]),/* "setct-CertReqTBS" */
&(nid_objs[565]),/* "setct-CertResData" */
&(nid_objs[597]),/* "setct-CertResTBE" */
&(nid_objs[586]),/* "setct-CredReqTBE" */
&(nid_objs[587]),/* "setct-CredReqTBEX" */
&(nid_objs[550]),/* "setct-CredReqTBS" */
&(nid_objs[551]),/* "setct-CredReqTBSX" */
&(nid_objs[552]),/* "setct-CredResData" */
&(nid_objs[588]),/* "setct-CredResTBE" */
&(nid_objs[589]),/* "setct-CredRevReqTBE" */
&(nid_objs[590]),/* "setct-CredRevReqTBEX" */
&(nid_objs[553]),/* "setct-CredRevReqTBS" */
&(nid_objs[554]),/* "setct-CredRevReqTBSX" */
&(nid_objs[555]),/* "setct-CredRevResData" */
&(nid_objs[591]),/* "setct-CredRevResTBE" */
&(nid_objs[567]),/* "setct-ErrorTBS" */
&(nid_objs[526]),/* "setct-HODInput" */
&(nid_objs[561]),/* "setct-MeAqCInitResTBS" */
&(nid_objs[522]),/* "setct-OIData" */
&(nid_objs[519]),/* "setct-PANData" */
&(nid_objs[521]),/* "setct-PANOnly" */
&(nid_objs[520]),/* "setct-PANToken" */
&(nid_objs[556]),/* "setct-PCertReqData" */
&(nid_objs[557]),/* "setct-PCertResTBS" */
&(nid_objs[523]),/* "setct-PI" */
&(nid_objs[532]),/* "setct-PI-TBS" */
&(nid_objs[524]),/* "setct-PIData" */
&(nid_objs[525]),/* "setct-PIDataUnsigned" */
&(nid_objs[568]),/* "setct-PIDualSignedTBE" */
&(nid_objs[569]),/* "setct-PIUnsignedTBE" */
&(nid_objs[531]),/* "setct-PInitResData" */
&(nid_objs[533]),/* "setct-PResData" */
&(nid_objs[594]),/* "setct-RegFormReqTBE" */
&(nid_objs[562]),/* "setct-RegFormResTBS" */
&(nid_objs[606]),/* "setext-cv" */
&(nid_objs[601]),/* "setext-genCrypt" */
&(nid_objs[602]),/* "setext-miAuth" */
&(nid_objs[604]),/* "setext-pinAny" */
&(nid_objs[603]),/* "setext-pinSecure" */
&(nid_objs[605]),/* "setext-track2" */
&(nid_objs[52]),/* "signingTime" */
&(nid_objs[454]),/* "simpleSecurityObject" */
&(nid_objs[496]),/* "singleLevelQuality" */
&(nid_objs[387]),/* "snmpv2" */
&(nid_objs[660]),/* "streetAddress" */
&(nid_objs[85]),/* "subjectAltName" */
&(nid_objs[769]),/* "subjectDirectoryAttributes" */
&(nid_objs[398]),/* "subjectInfoAccess" */
&(nid_objs[82]),/* "subjectKeyIdentifier" */
&(nid_objs[498]),/* "subtreeMaximumQuality" */
&(nid_objs[497]),/* "subtreeMinimumQuality" */
&(nid_objs[402]),/* "targetInformation" */
&(nid_objs[459]),/* "textEncodedORAddress" */
&(nid_objs[293]),/* "textNotice" */
&(nid_objs[133]),/* "timeStamping" */
&(nid_objs[106]),/* "title" */
&(nid_objs[682]),/* "tpBasis" */
&(nid_objs[375]),/* "trustRoot" */
&(nid_objs[436]),/* "ucl" */
&(nid_objs[55]),/* "unstructuredAddress" */
&(nid_objs[49]),/* "unstructuredName" */
&(nid_objs[465]),/* "userClass" */
&(nid_objs[373]),/* "valid" */
&(nid_objs[678]),/* "wap" */
&(nid_objs[679]),/* "wap-wsg" */
&(nid_objs[735]),/* "wap-wsg-idm-ecid-wtls1" */
&(nid_objs[743]),/* "wap-wsg-idm-ecid-wtls10" */
&(nid_objs[744]),/* "wap-wsg-idm-ecid-wtls11" */
&(nid_objs[745]),/* "wap-wsg-idm-ecid-wtls12" */
&(nid_objs[736]),/* "wap-wsg-idm-ecid-wtls3" */
&(nid_objs[737]),/* "wap-wsg-idm-ecid-wtls4" */
&(nid_objs[738]),/* "wap-wsg-idm-ecid-wtls5" */
&(nid_objs[739]),/* "wap-wsg-idm-ecid-wtls6" */
&(nid_objs[740]),/* "wap-wsg-idm-ecid-wtls7" */
&(nid_objs[741]),/* "wap-wsg-idm-ecid-wtls8" */
&(nid_objs[742]),/* "wap-wsg-idm-ecid-wtls9" */
&(nid_objs[503]),/* "x500UniqueIdentifier" */
&(nid_objs[158]),/* "x509Certificate" */
&(nid_objs[160]),/* "x509Crl" */
};

static ASN1_OBJECT *ln_objs[NUM_LN]={
&(nid_objs[363]),/* "AD Time Stamping" */
&(nid_objs[405]),/* "ANSI X9.62" */
&(nid_objs[368]),/* "Acceptable OCSP Responses" */
&(nid_objs[664]),/* "Any language" */
&(nid_objs[177]),/* "Authority Information Access" */
&(nid_objs[365]),/* "Basic OCSP Response" */
&(nid_objs[285]),/* "Biometric Info" */
&(nid_objs[179]),/* "CA Issuers" */
&(nid_objs[131]),/* "Code Signing" */
&(nid_objs[382]),/* "Directory" */
&(nid_objs[392]),/* "Domain" */
&(nid_objs[132]),/* "E-mail Protection" */
&(nid_objs[389]),/* "Enterprises" */
&(nid_objs[384]),/* "Experimental" */
&(nid_objs[372]),/* "Extended OCSP Status" */
&(nid_objs[172]),/* "Extension Request" */
&(nid_objs[432]),/* "Hold Instruction Call Issuer" */
&(nid_objs[430]),/* "Hold Instruction Code" */
&(nid_objs[431]),/* "Hold Instruction None" */
&(nid_objs[433]),/* "Hold Instruction Reject" */
&(nid_objs[634]),/* "ICC or token signature" */
&(nid_objs[294]),/* "IPSec End System" */
&(nid_objs[295]),/* "IPSec Tunnel" */
&(nid_objs[296]),/* "IPSec User" */
&(nid_objs[182]),/* "ISO Member Body" */
&(nid_objs[183]),/* "ISO US Member Body" */
&(nid_objs[667]),/* "Independent" */
&(nid_objs[665]),/* "Inherit all" */
&(nid_objs[647]),/* "International Organizations" */
&(nid_objs[142]),/* "Invalidity Date" */
&(nid_objs[504]),/* "MIME MHS" */
&(nid_objs[388]),/* "Mail" */
&(nid_objs[383]),/* "Management" */
&(nid_objs[417]),/* "Microsoft CSP Name" */
&(nid_objs[135]),/* "Microsoft Commercial Code Signing" */
&(nid_objs[138]),/* "Microsoft Encrypted File System" */
&(nid_objs[171]),/* "Microsoft Extension Request" */
&(nid_objs[134]),/* "Microsoft Individual Code Signing" */
&(nid_objs[137]),/* "Microsoft Server Gated Crypto" */
&(nid_objs[648]),/* "Microsoft Smartcardlogin" */
&(nid_objs[136]),/* "Microsoft Trust List Signing" */
&(nid_objs[649]),/* "Microsoft Universal Principal Name" */
&(nid_objs[393]),/* "NULL" */
&(nid_objs[404]),/* "NULL" */
&(nid_objs[72]),/* "Netscape Base Url" */
&(nid_objs[76]),/* "Netscape CA Policy Url" */
&(nid_objs[74]),/* "Netscape CA Revocation Url" */
&(nid_objs[71]),/* "Netscape Cert Type" */
&(nid_objs[58]),/* "Netscape Certificate Extension" */
&(nid_objs[79]),/* "Netscape Certificate Sequence" */
&(nid_objs[78]),/* "Netscape Comment" */
&(nid_objs[57]),/* "Netscape Communications Corp." */
&(nid_objs[59]),/* "Netscape Data Type" */
&(nid_objs[75]),/* "Netscape Renewal Url" */
&(nid_objs[73]),/* "Netscape Revocation Url" */
&(nid_objs[77]),/* "Netscape SSL Server Name" */
&(nid_objs[139]),/* "Netscape Server Gated Crypto" */
&(nid_objs[178]),/* "OCSP" */
&(nid_objs[370]),/* "OCSP Archive Cutoff" */
&(nid_objs[367]),/* "OCSP CRL ID" */
&(nid_objs[369]),/* "OCSP No Check" */
&(nid_objs[366]),/* "OCSP Nonce" */
&(nid_objs[371]),/* "OCSP Service Locator" */
&(nid_objs[180]),/* "OCSP Signing" */
&(nid_objs[161]),/* "PBES2" */
&(nid_objs[69]),/* "PBKDF2" */
&(nid_objs[162]),/* "PBMAC1" */
&(nid_objs[127]),/* "PKIX" */
&(nid_objs[164]),/* "Policy Qualifier CPS" */
&(nid_objs[165]),/* "Policy Qualifier User Notice" */
&(nid_objs[385]),/* "Private" */
&(nid_objs[663]),/* "Proxy Certificate Information" */
&(nid_objs[ 1]),/* "RSA Data Security, Inc." */
&(nid_objs[ 2]),/* "RSA Data Security, Inc. PKCS" */
&(nid_objs[188]),/* "S/MIME" */
&(nid_objs[167]),/* "S/MIME Capabilities" */
&(nid_objs[387]),/* "SNMPv2" */
&(nid_objs[512]),/* "Secure Electronic Transactions" */
&(nid_objs[386]),/* "Security" */
&(nid_objs[394]),/* "Selected Attribute Types" */
&(nid_objs[143]),/* "Strong Extranet ID" */
&(nid_objs[398]),/* "Subject Information Access" */
&(nid_objs[130]),/* "TLS Web Client Authentication" */
&(nid_objs[129]),/* "TLS Web Server Authentication" */
&(nid_objs[133]),/* "Time Stamping" */
&(nid_objs[375]),/* "Trust Root" */
&(nid_objs[12]),/* "X509" */
&(nid_objs[402]),/* "X509v3 AC Targeting" */
&(nid_objs[746]),/* "X509v3 Any Policy" */
&(nid_objs[90]),/* "X509v3 Authority Key Identifier" */
&(nid_objs[87]),/* "X509v3 Basic Constraints" */
&(nid_objs[103]),/* "X509v3 CRL Distribution Points" */
&(nid_objs[88]),/* "X509v3 CRL Number" */
&(nid_objs[141]),/* "X509v3 CRL Reason Code" */
&(nid_objs[771]),/* "X509v3 Certificate Issuer" */
&(nid_objs[89]),/* "X509v3 Certificate Policies" */
&(nid_objs[140]),/* "X509v3 Delta CRL Indicator" */
&(nid_objs[126]),/* "X509v3 Extended Key Usage" */
&(nid_objs[748]),/* "X509v3 Inhibit Any Policy" */
&(nid_objs[86]),/* "X509v3 Issuer Alternative Name" */
&(nid_objs[770]),/* "X509v3 Issuing Distrubution Point" */
&(nid_objs[83]),/* "X509v3 Key Usage" */
&(nid_objs[666]),/* "X509v3 Name Constraints" */
&(nid_objs[403]),/* "X509v3 No Revocation Available" */
&(nid_objs[401]),/* "X509v3 Policy Constraints" */
&(nid_objs[747]),/* "X509v3 Policy Mappings" */
&(nid_objs[84]),/* "X509v3 Private Key Usage Period" */
&(nid_objs[85]),/* "X509v3 Subject Alternative Name" */
&(nid_objs[769]),/* "X509v3 Subject Directory Attributes" */
&(nid_objs[82]),/* "X509v3 Subject Key Identifier" */
&(nid_objs[184]),/* "X9.57" */
&(nid_objs[185]),/* "X9.57 CM ?" */
&(nid_objs[478]),/* "aRecord" */
&(nid_objs[289]),/* "aaControls" */
&(nid_objs[287]),/* "ac-auditEntity" */
&(nid_objs[397]),/* "ac-proxying" */
&(nid_objs[288]),/* "ac-targeting" */
&(nid_objs[446]),/* "account" */
&(nid_objs[364]),/* "ad dvcs" */
&(nid_objs[606]),/* "additional verification" */
&(nid_objs[419]),/* "aes-128-cbc" */
&(nid_objs[421]),/* "aes-128-cfb" */
&(nid_objs[650]),/* "aes-128-cfb1" */
&(nid_objs[653]),/* "aes-128-cfb8" */
&(nid_objs[418]),/* "aes-128-ecb" */
&(nid_objs[420]),/* "aes-128-ofb" */
&(nid_objs[423]),/* "aes-192-cbc" */
&(nid_objs[425]),/* "aes-192-cfb" */
&(nid_objs[651]),/* "aes-192-cfb1" */
&(nid_objs[654]),/* "aes-192-cfb8" */
&(nid_objs[422]),/* "aes-192-ecb" */
&(nid_objs[424]),/* "aes-192-ofb" */
&(nid_objs[427]),/* "aes-256-cbc" */
&(nid_objs[429]),/* "aes-256-cfb" */
&(nid_objs[652]),/* "aes-256-cfb1" */
&(nid_objs[655]),/* "aes-256-cfb8" */
&(nid_objs[426]),/* "aes-256-ecb" */
&(nid_objs[428]),/* "aes-256-ofb" */
&(nid_objs[376]),/* "algorithm" */
&(nid_objs[484]),/* "associatedDomain" */
&(nid_objs[485]),/* "associatedName" */
&(nid_objs[501]),/* "audio" */
&(nid_objs[91]),/* "bf-cbc" */
&(nid_objs[93]),/* "bf-cfb" */
&(nid_objs[92]),/* "bf-ecb" */
&(nid_objs[94]),/* "bf-ofb" */
&(nid_objs[494]),/* "buildingName" */
&(nid_objs[691]),/* "c2onb191v4" */
&(nid_objs[692]),/* "c2onb191v5" */
&(nid_objs[697]),/* "c2onb239v4" */
&(nid_objs[698]),/* "c2onb239v5" */
&(nid_objs[684]),/* "c2pnb163v1" */
&(nid_objs[685]),/* "c2pnb163v2" */
&(nid_objs[686]),/* "c2pnb163v3" */
&(nid_objs[687]),/* "c2pnb176v1" */
&(nid_objs[693]),/* "c2pnb208w1" */
&(nid_objs[699]),/* "c2pnb272w1" */
&(nid_objs[700]),/* "c2pnb304w1" */
&(nid_objs[702]),/* "c2pnb368w1" */
&(nid_objs[688]),/* "c2tnb191v1" */
&(nid_objs[689]),/* "c2tnb191v2" */
&(nid_objs[690]),/* "c2tnb191v3" */
&(nid_objs[694]),/* "c2tnb239v1" */
&(nid_objs[695]),/* "c2tnb239v2" */
&(nid_objs[696]),/* "c2tnb239v3" */
&(nid_objs[701]),/* "c2tnb359v1" */
&(nid_objs[703]),/* "c2tnb431r1" */
&(nid_objs[483]),/* "cNAMERecord" */
&(nid_objs[751]),/* "camellia-128-cbc" */
&(nid_objs[757]),/* "camellia-128-cfb" */
&(nid_objs[760]),/* "camellia-128-cfb1" */
&(nid_objs[763]),/* "camellia-128-cfb8" */
&(nid_objs[754]),/* "camellia-128-ecb" */
&(nid_objs[766]),/* "camellia-128-ofb" */
&(nid_objs[752]),/* "camellia-192-cbc" */
&(nid_objs[758]),/* "camellia-192-cfb" */
&(nid_objs[761]),/* "camellia-192-cfb1" */
&(nid_objs[764]),/* "camellia-192-cfb8" */
&(nid_objs[755]),/* "camellia-192-ecb" */
&(nid_objs[767]),/* "camellia-192-ofb" */
&(nid_objs[753]),/* "camellia-256-cbc" */
&(nid_objs[759]),/* "camellia-256-cfb" */
&(nid_objs[762]),/* "camellia-256-cfb1" */
&(nid_objs[765]),/* "camellia-256-cfb8" */
&(nid_objs[756]),/* "camellia-256-ecb" */
&(nid_objs[768]),/* "camellia-256-ofb" */
&(nid_objs[443]),/* "caseIgnoreIA5StringSyntax" */
&(nid_objs[108]),/* "cast5-cbc" */
&(nid_objs[110]),/* "cast5-cfb" */
&(nid_objs[109]),/* "cast5-ecb" */
&(nid_objs[111]),/* "cast5-ofb" */
&(nid_objs[152]),/* "certBag" */
&(nid_objs[677]),/* "certicom-arc" */
&(nid_objs[517]),/* "certificate extensions" */
&(nid_objs[54]),/* "challengePassword" */
&(nid_objs[407]),/* "characteristic-two-field" */
&(nid_objs[395]),/* "clearance" */
&(nid_objs[633]),/* "cleartext track 2" */
&(nid_objs[13]),/* "commonName" */
&(nid_objs[513]),/* "content types" */
&(nid_objs[50]),/* "contentType" */
&(nid_objs[53]),/* "countersignature" */
&(nid_objs[14]),/* "countryName" */
&(nid_objs[153]),/* "crlBag" */
&(nid_objs[500]),/* "dITRedirect" */
&(nid_objs[451]),/* "dNSDomain" */
&(nid_objs[495]),/* "dSAQuality" */
&(nid_objs[434]),/* "data" */
&(nid_objs[390]),/* "dcObject" */
&(nid_objs[31]),/* "des-cbc" */
&(nid_objs[643]),/* "des-cdmf" */
&(nid_objs[30]),/* "des-cfb" */
&(nid_objs[656]),/* "des-cfb1" */
&(nid_objs[657]),/* "des-cfb8" */
&(nid_objs[29]),/* "des-ecb" */
&(nid_objs[32]),/* "des-ede" */
&(nid_objs[43]),/* "des-ede-cbc" */
&(nid_objs[60]),/* "des-ede-cfb" */
&(nid_objs[62]),/* "des-ede-ofb" */
&(nid_objs[33]),/* "des-ede3" */
&(nid_objs[44]),/* "des-ede3-cbc" */
&(nid_objs[61]),/* "des-ede3-cfb" */
&(nid_objs[658]),/* "des-ede3-cfb1" */
&(nid_objs[659]),/* "des-ede3-cfb8" */
&(nid_objs[63]),/* "des-ede3-ofb" */
&(nid_objs[45]),/* "des-ofb" */
&(nid_objs[107]),/* "description" */
&(nid_objs[80]),/* "desx-cbc" */
&(nid_objs[28]),/* "dhKeyAgreement" */
&(nid_objs[11]),/* "directory services (X.500)" */
&(nid_objs[378]),/* "directory services - algorithms" */
&(nid_objs[174]),/* "dnQualifier" */
&(nid_objs[447]),/* "document" */
&(nid_objs[471]),/* "documentAuthor" */
&(nid_objs[468]),/* "documentIdentifier" */
&(nid_objs[472]),/* "documentLocation" */
&(nid_objs[502]),/* "documentPublisher" */
&(nid_objs[449]),/* "documentSeries" */
&(nid_objs[469]),/* "documentTitle" */
&(nid_objs[470]),/* "documentVersion" */
&(nid_objs[380]),/* "dod" */
&(nid_objs[391]),/* "domainComponent" */
&(nid_objs[452]),/* "domainRelatedObject" */
&(nid_objs[116]),/* "dsaEncryption" */
&(nid_objs[67]),/* "dsaEncryption-old" */
&(nid_objs[66]),/* "dsaWithSHA" */
&(nid_objs[113]),/* "dsaWithSHA1" */
&(nid_objs[70]),/* "dsaWithSHA1-old" */
&(nid_objs[297]),/* "dvcs" */
&(nid_objs[416]),/* "ecdsa-with-SHA1" */
&(nid_objs[48]),/* "emailAddress" */
&(nid_objs[632]),/* "encrypted track 2" */
&(nid_objs[56]),/* "extendedCertificateAttributes" */
&(nid_objs[462]),/* "favouriteDrink" */
&(nid_objs[453]),/* "friendlyCountry" */
&(nid_objs[490]),/* "friendlyCountryName" */
&(nid_objs[156]),/* "friendlyName" */
&(nid_objs[631]),/* "generate cryptogram" */
&(nid_objs[509]),/* "generationQualifier" */
&(nid_objs[601]),/* "generic cryptogram" */
&(nid_objs[99]),/* "givenName" */
&(nid_objs[163]),/* "hmacWithSHA1" */
&(nid_objs[486]),/* "homePostalAddress" */
&(nid_objs[473]),/* "homeTelephoneNumber" */
&(nid_objs[466]),/* "host" */
&(nid_objs[442]),/* "iA5StringSyntax" */
&(nid_objs[381]),/* "iana" */
&(nid_objs[266]),/* "id-aca" */
&(nid_objs[355]),/* "id-aca-accessIdentity" */
&(nid_objs[354]),/* "id-aca-authenticationInfo" */
&(nid_objs[356]),/* "id-aca-chargingIdentity" */
&(nid_objs[399]),/* "id-aca-encAttrs" */
&(nid_objs[357]),/* "id-aca-group" */
&(nid_objs[358]),/* "id-aca-role" */
&(nid_objs[176]),/* "id-ad" */
&(nid_objs[262]),/* "id-alg" */
&(nid_objs[323]),/* "id-alg-des40" */
&(nid_objs[326]),/* "id-alg-dh-pop" */
&(nid_objs[325]),/* "id-alg-dh-sig-hmac-sha1" */
&(nid_objs[324]),/* "id-alg-noSignature" */
&(nid_objs[268]),/* "id-cct" */
&(nid_objs[361]),/* "id-cct-PKIData" */
&(nid_objs[362]),/* "id-cct-PKIResponse" */
&(nid_objs[360]),/* "id-cct-crs" */
&(nid_objs[81]),/* "id-ce" */
&(nid_objs[680]),/* "id-characteristic-two-basis" */
&(nid_objs[263]),/* "id-cmc" */
&(nid_objs[334]),/* "id-cmc-addExtensions" */
&(nid_objs[346]),/* "id-cmc-confirmCertAcceptance" */
&(nid_objs[330]),/* "id-cmc-dataReturn" */
&(nid_objs[336]),/* "id-cmc-decryptedPOP" */
&(nid_objs[335]),/* "id-cmc-encryptedPOP" */
&(nid_objs[339]),/* "id-cmc-getCRL" */
&(nid_objs[338]),/* "id-cmc-getCert" */
&(nid_objs[328]),/* "id-cmc-identification" */
&(nid_objs[329]),/* "id-cmc-identityProof" */
&(nid_objs[337]),/* "id-cmc-lraPOPWitness" */
&(nid_objs[344]),/* "id-cmc-popLinkRandom" */
&(nid_objs[345]),/* "id-cmc-popLinkWitness" */
&(nid_objs[343]),/* "id-cmc-queryPending" */
&(nid_objs[333]),/* "id-cmc-recipientNonce" */
&(nid_objs[341]),/* "id-cmc-regInfo" */
&(nid_objs[342]),/* "id-cmc-responseInfo" */
&(nid_objs[340]),/* "id-cmc-revokeRequest" */
&(nid_objs[332]),/* "id-cmc-senderNonce" */
&(nid_objs[327]),/* "id-cmc-statusInfo" */
&(nid_objs[331]),/* "id-cmc-transactionId" */
&(nid_objs[408]),/* "id-ecPublicKey" */
&(nid_objs[508]),/* "id-hex-multipart-message" */
&(nid_objs[507]),/* "id-hex-partial-message" */
&(nid_objs[260]),/* "id-it" */
&(nid_objs[302]),/* "id-it-caKeyUpdateInfo" */
&(nid_objs[298]),/* "id-it-caProtEncCert" */
&(nid_objs[311]),/* "id-it-confirmWaitTime" */
&(nid_objs[303]),/* "id-it-currentCRL" */
&(nid_objs[300]),/* "id-it-encKeyPairTypes" */
&(nid_objs[310]),/* "id-it-implicitConfirm" */
&(nid_objs[308]),/* "id-it-keyPairParamRep" */
&(nid_objs[307]),/* "id-it-keyPairParamReq" */
&(nid_objs[312]),/* "id-it-origPKIMessage" */
&(nid_objs[301]),/* "id-it-preferredSymmAlg" */
&(nid_objs[309]),/* "id-it-revPassphrase" */
&(nid_objs[299]),/* "id-it-signKeyPairTypes" */
&(nid_objs[305]),/* "id-it-subscriptionRequest" */
&(nid_objs[306]),/* "id-it-subscriptionResponse" */
&(nid_objs[304]),/* "id-it-unsupportedOIDs" */
&(nid_objs[128]),/* "id-kp" */
&(nid_objs[280]),/* "id-mod-attribute-cert" */
&(nid_objs[274]),/* "id-mod-cmc" */
&(nid_objs[277]),/* "id-mod-cmp" */
&(nid_objs[284]),/* "id-mod-cmp2000" */
&(nid_objs[273]),/* "id-mod-crmf" */
&(nid_objs[283]),/* "id-mod-dvcs" */
&(nid_objs[275]),/* "id-mod-kea-profile-88" */
&(nid_objs[276]),/* "id-mod-kea-profile-93" */
&(nid_objs[282]),/* "id-mod-ocsp" */
&(nid_objs[278]),/* "id-mod-qualified-cert-88" */
&(nid_objs[279]),/* "id-mod-qualified-cert-93" */
&(nid_objs[281]),/* "id-mod-timestamp-protocol" */
&(nid_objs[264]),/* "id-on" */
&(nid_objs[347]),/* "id-on-personalData" */
&(nid_objs[265]),/* "id-pda" */
&(nid_objs[352]),/* "id-pda-countryOfCitizenship" */
&(nid_objs[353]),/* "id-pda-countryOfResidence" */
&(nid_objs[348]),/* "id-pda-dateOfBirth" */
&(nid_objs[351]),/* "id-pda-gender" */
&(nid_objs[349]),/* "id-pda-placeOfBirth" */
&(nid_objs[175]),/* "id-pe" */
&(nid_objs[261]),/* "id-pkip" */
&(nid_objs[258]),/* "id-pkix-mod" */
&(nid_objs[269]),/* "id-pkix1-explicit-88" */
&(nid_objs[271]),/* "id-pkix1-explicit-93" */
&(nid_objs[270]),/* "id-pkix1-implicit-88" */
&(nid_objs[272]),/* "id-pkix1-implicit-93" */
&(nid_objs[662]),/* "id-ppl" */
&(nid_objs[267]),/* "id-qcs" */
&(nid_objs[359]),/* "id-qcs-pkixQCSyntax-v1" */
&(nid_objs[259]),/* "id-qt" */
&(nid_objs[313]),/* "id-regCtrl" */
&(nid_objs[316]),/* "id-regCtrl-authenticator" */
&(nid_objs[319]),/* "id-regCtrl-oldCertID" */
&(nid_objs[318]),/* "id-regCtrl-pkiArchiveOptions" */
&(nid_objs[317]),/* "id-regCtrl-pkiPublicationInfo" */
&(nid_objs[320]),/* "id-regCtrl-protocolEncrKey" */
&(nid_objs[315]),/* "id-regCtrl-regToken" */
&(nid_objs[314]),/* "id-regInfo" */
&(nid_objs[322]),/* "id-regInfo-certReq" */
&(nid_objs[321]),/* "id-regInfo-utf8Pairs" */
&(nid_objs[191]),/* "id-smime-aa" */
&(nid_objs[215]),/* "id-smime-aa-contentHint" */
&(nid_objs[218]),/* "id-smime-aa-contentIdentifier" */
&(nid_objs[221]),/* "id-smime-aa-contentReference" */
&(nid_objs[240]),/* "id-smime-aa-dvcs-dvc" */
&(nid_objs[217]),/* "id-smime-aa-encapContentType" */
&(nid_objs[222]),/* "id-smime-aa-encrypKeyPref" */
&(nid_objs[220]),/* "id-smime-aa-equivalentLabels" */
&(nid_objs[232]),/* "id-smime-aa-ets-CertificateRefs" */
&(nid_objs[233]),/* "id-smime-aa-ets-RevocationRefs" */
&(nid_objs[238]),/* "id-smime-aa-ets-archiveTimeStamp" */
&(nid_objs[237]),/* "id-smime-aa-ets-certCRLTimestamp" */
&(nid_objs[234]),/* "id-smime-aa-ets-certValues" */
&(nid_objs[227]),/* "id-smime-aa-ets-commitmentType" */
&(nid_objs[231]),/* "id-smime-aa-ets-contentTimestamp" */
&(nid_objs[236]),/* "id-smime-aa-ets-escTimeStamp" */
&(nid_objs[230]),/* "id-smime-aa-ets-otherSigCert" */
&(nid_objs[235]),/* "id-smime-aa-ets-revocationValues" */
&(nid_objs[226]),/* "id-smime-aa-ets-sigPolicyId" */
&(nid_objs[229]),/* "id-smime-aa-ets-signerAttr" */
&(nid_objs[228]),/* "id-smime-aa-ets-signerLocation" */
&(nid_objs[219]),/* "id-smime-aa-macValue" */
&(nid_objs[214]),/* "id-smime-aa-mlExpandHistory" */
&(nid_objs[216]),/* "id-smime-aa-msgSigDigest" */
&(nid_objs[212]),/* "id-smime-aa-receiptRequest" */
&(nid_objs[213]),/* "id-smime-aa-securityLabel" */
&(nid_objs[239]),/* "id-smime-aa-signatureType" */
&(nid_objs[223]),/* "id-smime-aa-signingCertificate" */
&(nid_objs[224]),/* "id-smime-aa-smimeEncryptCerts" */
&(nid_objs[225]),/* "id-smime-aa-timeStampToken" */
&(nid_objs[192]),/* "id-smime-alg" */
&(nid_objs[243]),/* "id-smime-alg-3DESwrap" */
&(nid_objs[246]),/* "id-smime-alg-CMS3DESwrap" */
&(nid_objs[247]),/* "id-smime-alg-CMSRC2wrap" */
&(nid_objs[245]),/* "id-smime-alg-ESDH" */
&(nid_objs[241]),/* "id-smime-alg-ESDHwith3DES" */
&(nid_objs[242]),/* "id-smime-alg-ESDHwithRC2" */
&(nid_objs[244]),/* "id-smime-alg-RC2wrap" */
&(nid_objs[193]),/* "id-smime-cd" */
&(nid_objs[248]),/* "id-smime-cd-ldap" */
&(nid_objs[190]),/* "id-smime-ct" */
&(nid_objs[210]),/* "id-smime-ct-DVCSRequestData" */
&(nid_objs[211]),/* "id-smime-ct-DVCSResponseData" */
&(nid_objs[208]),/* "id-smime-ct-TDTInfo" */
&(nid_objs[207]),/* "id-smime-ct-TSTInfo" */
&(nid_objs[205]),/* "id-smime-ct-authData" */
&(nid_objs[209]),/* "id-smime-ct-contentInfo" */
&(nid_objs[206]),/* "id-smime-ct-publishCert" */
&(nid_objs[204]),/* "id-smime-ct-receipt" */
&(nid_objs[195]),/* "id-smime-cti" */
&(nid_objs[255]),/* "id-smime-cti-ets-proofOfApproval" */
&(nid_objs[256]),/* "id-smime-cti-ets-proofOfCreation" */
&(nid_objs[253]),/* "id-smime-cti-ets-proofOfDelivery" */
&(nid_objs[251]),/* "id-smime-cti-ets-proofOfOrigin" */
&(nid_objs[252]),/* "id-smime-cti-ets-proofOfReceipt" */
&(nid_objs[254]),/* "id-smime-cti-ets-proofOfSender" */
&(nid_objs[189]),/* "id-smime-mod" */
&(nid_objs[196]),/* "id-smime-mod-cms" */
&(nid_objs[197]),/* "id-smime-mod-ess" */
&(nid_objs[202]),/* "id-smime-mod-ets-eSigPolicy-88" */
&(nid_objs[203]),/* "id-smime-mod-ets-eSigPolicy-97" */
&(nid_objs[200]),/* "id-smime-mod-ets-eSignature-88" */
&(nid_objs[201]),/* "id-smime-mod-ets-eSignature-97" */
&(nid_objs[199]),/* "id-smime-mod-msg-v3" */
&(nid_objs[198]),/* "id-smime-mod-oid" */
&(nid_objs[194]),/* "id-smime-spq" */
&(nid_objs[250]),/* "id-smime-spq-ets-sqt-unotice" */
&(nid_objs[249]),/* "id-smime-spq-ets-sqt-uri" */
&(nid_objs[34]),/* "idea-cbc" */
&(nid_objs[35]),/* "idea-cfb" */
&(nid_objs[36]),/* "idea-ecb" */
&(nid_objs[46]),/* "idea-ofb" */
&(nid_objs[676]),/* "identified-organization" */
&(nid_objs[461]),/* "info" */
&(nid_objs[101]),/* "initials" */
&(nid_objs[749]),/* "ipsec3" */
&(nid_objs[750]),/* "ipsec4" */
&(nid_objs[181]),/* "iso" */
&(nid_objs[623]),/* "issuer capabilities" */
&(nid_objs[645]),/* "itu-t" */
&(nid_objs[492]),/* "janetMailbox" */
&(nid_objs[646]),/* "joint-iso-itu-t" */
&(nid_objs[150]),/* "keyBag" */
&(nid_objs[773]),/* "kisa" */
&(nid_objs[477]),/* "lastModifiedBy" */
&(nid_objs[476]),/* "lastModifiedTime" */
&(nid_objs[157]),/* "localKeyID" */
&(nid_objs[15]),/* "localityName" */
&(nid_objs[480]),/* "mXRecord" */
&(nid_objs[493]),/* "mailPreferenceOption" */
&(nid_objs[467]),/* "manager" */
&(nid_objs[ 3]),/* "md2" */
&(nid_objs[ 7]),/* "md2WithRSAEncryption" */
&(nid_objs[257]),/* "md4" */
&(nid_objs[396]),/* "md4WithRSAEncryption" */
&(nid_objs[ 4]),/* "md5" */
&(nid_objs[114]),/* "md5-sha1" */
&(nid_objs[104]),/* "md5WithRSA" */
&(nid_objs[ 8]),/* "md5WithRSAEncryption" */
&(nid_objs[95]),/* "mdc2" */
&(nid_objs[96]),/* "mdc2WithRSA" */
&(nid_objs[602]),/* "merchant initiated auth" */
&(nid_objs[514]),/* "message extensions" */
&(nid_objs[51]),/* "messageDigest" */
&(nid_objs[506]),/* "mime-mhs-bodies" */
&(nid_objs[505]),/* "mime-mhs-headings" */
&(nid_objs[488]),/* "mobileTelephoneNumber" */
&(nid_objs[481]),/* "nSRecord" */
&(nid_objs[173]),/* "name" */
&(nid_objs[681]),/* "onBasis" */
&(nid_objs[379]),/* "org" */
&(nid_objs[17]),/* "organizationName" */
&(nid_objs[491]),/* "organizationalStatus" */
&(nid_objs[18]),/* "organizationalUnitName" */
&(nid_objs[475]),/* "otherMailbox" */
&(nid_objs[489]),/* "pagerTelephoneNumber" */
&(nid_objs[374]),/* "path" */
&(nid_objs[621]),/* "payment gateway capabilities" */
&(nid_objs[ 9]),/* "pbeWithMD2AndDES-CBC" */
&(nid_objs[168]),/* "pbeWithMD2AndRC2-CBC" */
&(nid_objs[112]),/* "pbeWithMD5AndCast5CBC" */
&(nid_objs[10]),/* "pbeWithMD5AndDES-CBC" */
&(nid_objs[169]),/* "pbeWithMD5AndRC2-CBC" */
&(nid_objs[148]),/* "pbeWithSHA1And128BitRC2-CBC" */
&(nid_objs[144]),/* "pbeWithSHA1And128BitRC4" */
&(nid_objs[147]),/* "pbeWithSHA1And2-KeyTripleDES-CBC" */
&(nid_objs[146]),/* "pbeWithSHA1And3-KeyTripleDES-CBC" */
&(nid_objs[149]),/* "pbeWithSHA1And40BitRC2-CBC" */
&(nid_objs[145]),/* "pbeWithSHA1And40BitRC4" */
&(nid_objs[170]),/* "pbeWithSHA1AndDES-CBC" */
&(nid_objs[68]),/* "pbeWithSHA1AndRC2-CBC" */
&(nid_objs[499]),/* "personalSignature" */
&(nid_objs[487]),/* "personalTitle" */
&(nid_objs[464]),/* "photo" */
&(nid_objs[437]),/* "pilot" */
&(nid_objs[439]),/* "pilotAttributeSyntax" */
&(nid_objs[438]),/* "pilotAttributeType" */
&(nid_objs[479]),/* "pilotAttributeType27" */
&(nid_objs[456]),/* "pilotDSA" */
&(nid_objs[441]),/* "pilotGroups" */
&(nid_objs[444]),/* "pilotObject" */
&(nid_objs[440]),/* "pilotObjectClass" */
&(nid_objs[455]),/* "pilotOrganization" */
&(nid_objs[445]),/* "pilotPerson" */
&(nid_objs[186]),/* "pkcs1" */
&(nid_objs[27]),/* "pkcs3" */
&(nid_objs[187]),/* "pkcs5" */
&(nid_objs[20]),/* "pkcs7" */
&(nid_objs[21]),/* "pkcs7-data" */
&(nid_objs[25]),/* "pkcs7-digestData" */
&(nid_objs[26]),/* "pkcs7-encryptedData" */
&(nid_objs[23]),/* "pkcs7-envelopedData" */
&(nid_objs[24]),/* "pkcs7-signedAndEnvelopedData" */
&(nid_objs[22]),/* "pkcs7-signedData" */
&(nid_objs[151]),/* "pkcs8ShroudedKeyBag" */
&(nid_objs[47]),/* "pkcs9" */
&(nid_objs[661]),/* "postalCode" */
&(nid_objs[683]),/* "ppBasis" */
&(nid_objs[406]),/* "prime-field" */
&(nid_objs[409]),/* "prime192v1" */
&(nid_objs[410]),/* "prime192v2" */
&(nid_objs[411]),/* "prime192v3" */
&(nid_objs[412]),/* "prime239v1" */
&(nid_objs[413]),/* "prime239v2" */
&(nid_objs[414]),/* "prime239v3" */
&(nid_objs[415]),/* "prime256v1" */
&(nid_objs[510]),/* "pseudonym" */
&(nid_objs[435]),/* "pss" */
&(nid_objs[286]),/* "qcStatements" */
&(nid_objs[457]),/* "qualityLabelledData" */
&(nid_objs[450]),/* "rFC822localPart" */
&(nid_objs[98]),/* "rc2-40-cbc" */
&(nid_objs[166]),/* "rc2-64-cbc" */
&(nid_objs[37]),/* "rc2-cbc" */
&(nid_objs[39]),/* "rc2-cfb" */
&(nid_objs[38]),/* "rc2-ecb" */
&(nid_objs[40]),/* "rc2-ofb" */
&(nid_objs[ 5]),/* "rc4" */
&(nid_objs[97]),/* "rc4-40" */
&(nid_objs[120]),/* "rc5-cbc" */
&(nid_objs[122]),/* "rc5-cfb" */
&(nid_objs[121]),/* "rc5-ecb" */
&(nid_objs[123]),/* "rc5-ofb" */
&(nid_objs[460]),/* "rfc822Mailbox" */
&(nid_objs[117]),/* "ripemd160" */
&(nid_objs[119]),/* "ripemd160WithRSA" */
&(nid_objs[400]),/* "role" */
&(nid_objs[448]),/* "room" */
&(nid_objs[463]),/* "roomNumber" */
&(nid_objs[19]),/* "rsa" */
&(nid_objs[ 6]),/* "rsaEncryption" */
&(nid_objs[644]),/* "rsaOAEPEncryptionSET" */
&(nid_objs[377]),/* "rsaSignature" */
&(nid_objs[124]),/* "run length compression" */
&(nid_objs[482]),/* "sOARecord" */
&(nid_objs[155]),/* "safeContentsBag" */
&(nid_objs[291]),/* "sbgp-autonomousSysNum" */
&(nid_objs[290]),/* "sbgp-ipAddrBlock" */
&(nid_objs[292]),/* "sbgp-routerIdentifier" */
&(nid_objs[159]),/* "sdsiCertificate" */
&(nid_objs[704]),/* "secp112r1" */
&(nid_objs[705]),/* "secp112r2" */
&(nid_objs[706]),/* "secp128r1" */
&(nid_objs[707]),/* "secp128r2" */
&(nid_objs[708]),/* "secp160k1" */
&(nid_objs[709]),/* "secp160r1" */
&(nid_objs[710]),/* "secp160r2" */
&(nid_objs[711]),/* "secp192k1" */
&(nid_objs[712]),/* "secp224k1" */
&(nid_objs[713]),/* "secp224r1" */
&(nid_objs[714]),/* "secp256k1" */
&(nid_objs[715]),/* "secp384r1" */
&(nid_objs[716]),/* "secp521r1" */
&(nid_objs[154]),/* "secretBag" */
&(nid_objs[474]),/* "secretary" */
&(nid_objs[717]),/* "sect113r1" */
&(nid_objs[718]),/* "sect113r2" */
&(nid_objs[719]),/* "sect131r1" */
&(nid_objs[720]),/* "sect131r2" */
&(nid_objs[721]),/* "sect163k1" */
&(nid_objs[722]),/* "sect163r1" */
&(nid_objs[723]),/* "sect163r2" */
&(nid_objs[724]),/* "sect193r1" */
&(nid_objs[725]),/* "sect193r2" */
&(nid_objs[726]),/* "sect233k1" */
&(nid_objs[727]),/* "sect233r1" */
&(nid_objs[728]),/* "sect239k1" */
&(nid_objs[729]),/* "sect283k1" */
&(nid_objs[730]),/* "sect283r1" */
&(nid_objs[731]),/* "sect409k1" */
&(nid_objs[732]),/* "sect409r1" */
&(nid_objs[733]),/* "sect571k1" */
&(nid_objs[734]),/* "sect571r1" */
&(nid_objs[635]),/* "secure device signature" */
&(nid_objs[777]),/* "seed-cbc" */
&(nid_objs[779]),/* "seed-cfb" */
&(nid_objs[776]),/* "seed-ecb" */
&(nid_objs[778]),/* "seed-ofb" */
&(nid_objs[105]),/* "serialNumber" */
&(nid_objs[625]),/* "set-addPolicy" */
&(nid_objs[515]),/* "set-attr" */
&(nid_objs[518]),/* "set-brand" */
&(nid_objs[638]),/* "set-brand-AmericanExpress" */
&(nid_objs[637]),/* "set-brand-Diners" */
&(nid_objs[636]),/* "set-brand-IATA-ATA" */
&(nid_objs[639]),/* "set-brand-JCB" */
&(nid_objs[641]),/* "set-brand-MasterCard" */
&(nid_objs[642]),/* "set-brand-Novus" */
&(nid_objs[640]),/* "set-brand-Visa" */
&(nid_objs[516]),/* "set-policy" */
&(nid_objs[607]),/* "set-policy-root" */
&(nid_objs[624]),/* "set-rootKeyThumb" */
&(nid_objs[620]),/* "setAttr-Cert" */
&(nid_objs[628]),/* "setAttr-IssCap-CVM" */
&(nid_objs[630]),/* "setAttr-IssCap-Sig" */
&(nid_objs[629]),/* "setAttr-IssCap-T2" */
&(nid_objs[627]),/* "setAttr-Token-B0Prime" */
&(nid_objs[626]),/* "setAttr-Token-EMV" */
&(nid_objs[622]),/* "setAttr-TokenType" */
&(nid_objs[619]),/* "setCext-IssuerCapabilities" */
&(nid_objs[615]),/* "setCext-PGWYcapabilities" */
&(nid_objs[616]),/* "setCext-TokenIdentifier" */
&(nid_objs[618]),/* "setCext-TokenType" */
&(nid_objs[617]),/* "setCext-Track2Data" */
&(nid_objs[611]),/* "setCext-cCertRequired" */
&(nid_objs[609]),/* "setCext-certType" */
&(nid_objs[608]),/* "setCext-hashedRoot" */
&(nid_objs[610]),/* "setCext-merchData" */
&(nid_objs[613]),/* "setCext-setExt" */
&(nid_objs[614]),/* "setCext-setQualf" */
&(nid_objs[612]),/* "setCext-tunneling" */
&(nid_objs[540]),/* "setct-AcqCardCodeMsg" */
&(nid_objs[576]),/* "setct-AcqCardCodeMsgTBE" */
&(nid_objs[570]),/* "setct-AuthReqTBE" */
&(nid_objs[534]),/* "setct-AuthReqTBS" */
&(nid_objs[527]),/* "setct-AuthResBaggage" */
&(nid_objs[571]),/* "setct-AuthResTBE" */
&(nid_objs[572]),/* "setct-AuthResTBEX" */
&(nid_objs[535]),/* "setct-AuthResTBS" */
&(nid_objs[536]),/* "setct-AuthResTBSX" */
&(nid_objs[528]),/* "setct-AuthRevReqBaggage" */
&(nid_objs[577]),/* "setct-AuthRevReqTBE" */
&(nid_objs[541]),/* "setct-AuthRevReqTBS" */
&(nid_objs[529]),/* "setct-AuthRevResBaggage" */
&(nid_objs[542]),/* "setct-AuthRevResData" */
&(nid_objs[578]),/* "setct-AuthRevResTBE" */
&(nid_objs[579]),/* "setct-AuthRevResTBEB" */
&(nid_objs[543]),/* "setct-AuthRevResTBS" */
&(nid_objs[573]),/* "setct-AuthTokenTBE" */
&(nid_objs[537]),/* "setct-AuthTokenTBS" */
&(nid_objs[600]),/* "setct-BCIDistributionTBS" */
&(nid_objs[558]),/* "setct-BatchAdminReqData" */
&(nid_objs[592]),/* "setct-BatchAdminReqTBE" */
&(nid_objs[559]),/* "setct-BatchAdminResData" */
&(nid_objs[593]),/* "setct-BatchAdminResTBE" */
&(nid_objs[599]),/* "setct-CRLNotificationResTBS" */
&(nid_objs[598]),/* "setct-CRLNotificationTBS" */
&(nid_objs[580]),/* "setct-CapReqTBE" */
&(nid_objs[581]),/* "setct-CapReqTBEX" */
&(nid_objs[544]),/* "setct-CapReqTBS" */
&(nid_objs[545]),/* "setct-CapReqTBSX" */
&(nid_objs[546]),/* "setct-CapResData" */
&(nid_objs[582]),/* "setct-CapResTBE" */
&(nid_objs[583]),/* "setct-CapRevReqTBE" */
&(nid_objs[584]),/* "setct-CapRevReqTBEX" */
&(nid_objs[547]),/* "setct-CapRevReqTBS" */
&(nid_objs[548]),/* "setct-CapRevReqTBSX" */
&(nid_objs[549]),/* "setct-CapRevResData" */
&(nid_objs[585]),/* "setct-CapRevResTBE" */
&(nid_objs[538]),/* "setct-CapTokenData" */
&(nid_objs[530]),/* "setct-CapTokenSeq" */
&(nid_objs[574]),/* "setct-CapTokenTBE" */
&(nid_objs[575]),/* "setct-CapTokenTBEX" */
&(nid_objs[539]),/* "setct-CapTokenTBS" */
&(nid_objs[560]),/* "setct-CardCInitResTBS" */
&(nid_objs[566]),/* "setct-CertInqReqTBS" */
&(nid_objs[563]),/* "setct-CertReqData" */
&(nid_objs[595]),/* "setct-CertReqTBE" */
&(nid_objs[596]),/* "setct-CertReqTBEX" */
&(nid_objs[564]),/* "setct-CertReqTBS" */
&(nid_objs[565]),/* "setct-CertResData" */
&(nid_objs[597]),/* "setct-CertResTBE" */
&(nid_objs[586]),/* "setct-CredReqTBE" */
&(nid_objs[587]),/* "setct-CredReqTBEX" */
&(nid_objs[550]),/* "setct-CredReqTBS" */
&(nid_objs[551]),/* "setct-CredReqTBSX" */
&(nid_objs[552]),/* "setct-CredResData" */
&(nid_objs[588]),/* "setct-CredResTBE" */
&(nid_objs[589]),/* "setct-CredRevReqTBE" */
&(nid_objs[590]),/* "setct-CredRevReqTBEX" */
&(nid_objs[553]),/* "setct-CredRevReqTBS" */
&(nid_objs[554]),/* "setct-CredRevReqTBSX" */
&(nid_objs[555]),/* "setct-CredRevResData" */
&(nid_objs[591]),/* "setct-CredRevResTBE" */
&(nid_objs[567]),/* "setct-ErrorTBS" */
&(nid_objs[526]),/* "setct-HODInput" */
&(nid_objs[561]),/* "setct-MeAqCInitResTBS" */
&(nid_objs[522]),/* "setct-OIData" */
&(nid_objs[519]),/* "setct-PANData" */
&(nid_objs[521]),/* "setct-PANOnly" */
&(nid_objs[520]),/* "setct-PANToken" */
&(nid_objs[556]),/* "setct-PCertReqData" */
&(nid_objs[557]),/* "setct-PCertResTBS" */
&(nid_objs[523]),/* "setct-PI" */
&(nid_objs[532]),/* "setct-PI-TBS" */
&(nid_objs[524]),/* "setct-PIData" */
&(nid_objs[525]),/* "setct-PIDataUnsigned" */
&(nid_objs[568]),/* "setct-PIDualSignedTBE" */
&(nid_objs[569]),/* "setct-PIUnsignedTBE" */
&(nid_objs[531]),/* "setct-PInitResData" */
&(nid_objs[533]),/* "setct-PResData" */
&(nid_objs[594]),/* "setct-RegFormReqTBE" */
&(nid_objs[562]),/* "setct-RegFormResTBS" */
&(nid_objs[604]),/* "setext-pinAny" */
&(nid_objs[603]),/* "setext-pinSecure" */
&(nid_objs[605]),/* "setext-track2" */
&(nid_objs[41]),/* "sha" */
&(nid_objs[64]),/* "sha1" */
&(nid_objs[115]),/* "sha1WithRSA" */
&(nid_objs[65]),/* "sha1WithRSAEncryption" */
&(nid_objs[675]),/* "sha224" */
&(nid_objs[671]),/* "sha224WithRSAEncryption" */
&(nid_objs[672]),/* "sha256" */
&(nid_objs[668]),/* "sha256WithRSAEncryption" */
&(nid_objs[673]),/* "sha384" */
&(nid_objs[669]),/* "sha384WithRSAEncryption" */
&(nid_objs[674]),/* "sha512" */
&(nid_objs[670]),/* "sha512WithRSAEncryption" */
&(nid_objs[42]),/* "shaWithRSAEncryption" */
&(nid_objs[52]),/* "signingTime" */
&(nid_objs[454]),/* "simpleSecurityObject" */
&(nid_objs[496]),/* "singleLevelQuality" */
&(nid_objs[16]),/* "stateOrProvinceName" */
&(nid_objs[660]),/* "streetAddress" */
&(nid_objs[498]),/* "subtreeMaximumQuality" */
&(nid_objs[497]),/* "subtreeMinimumQuality" */
&(nid_objs[100]),/* "surname" */
&(nid_objs[459]),/* "textEncodedORAddress" */
&(nid_objs[293]),/* "textNotice" */
&(nid_objs[106]),/* "title" */
&(nid_objs[682]),/* "tpBasis" */
&(nid_objs[436]),/* "ucl" */
&(nid_objs[ 0]),/* "undefined" */
&(nid_objs[55]),/* "unstructuredAddress" */
&(nid_objs[49]),/* "unstructuredName" */
&(nid_objs[465]),/* "userClass" */
&(nid_objs[458]),/* "userId" */
&(nid_objs[373]),/* "valid" */
&(nid_objs[678]),/* "wap" */
&(nid_objs[679]),/* "wap-wsg" */
&(nid_objs[735]),/* "wap-wsg-idm-ecid-wtls1" */
&(nid_objs[743]),/* "wap-wsg-idm-ecid-wtls10" */
&(nid_objs[744]),/* "wap-wsg-idm-ecid-wtls11" */
&(nid_objs[745]),/* "wap-wsg-idm-ecid-wtls12" */
&(nid_objs[736]),/* "wap-wsg-idm-ecid-wtls3" */
&(nid_objs[737]),/* "wap-wsg-idm-ecid-wtls4" */
&(nid_objs[738]),/* "wap-wsg-idm-ecid-wtls5" */
&(nid_objs[739]),/* "wap-wsg-idm-ecid-wtls6" */
&(nid_objs[740]),/* "wap-wsg-idm-ecid-wtls7" */
&(nid_objs[741]),/* "wap-wsg-idm-ecid-wtls8" */
&(nid_objs[742]),/* "wap-wsg-idm-ecid-wtls9" */
&(nid_objs[503]),/* "x500UniqueIdentifier" */
&(nid_objs[158]),/* "x509Certificate" */
&(nid_objs[160]),/* "x509Crl" */
&(nid_objs[125]),/* "zlib compression" */
};

static ASN1_OBJECT *obj_objs[NUM_OBJ]={
&(nid_objs[ 0]),/* OBJ_undef                        0 */
&(nid_objs[393]),/* OBJ_joint_iso_ccitt              OBJ_joint_iso_itu_t */
&(nid_objs[404]),/* OBJ_ccitt                        OBJ_itu_t */
&(nid_objs[645]),/* OBJ_itu_t                        0 */
&(nid_objs[434]),/* OBJ_data                         0 9 */
&(nid_objs[181]),/* OBJ_iso                          1 */
&(nid_objs[182]),/* OBJ_member_body                  1 2 */
&(nid_objs[379]),/* OBJ_org                          1 3 */
&(nid_objs[676]),/* OBJ_identified_organization      1 3 */
&(nid_objs[646]),/* OBJ_joint_iso_itu_t              2 */
&(nid_objs[11]),/* OBJ_X500                         2 5 */
&(nid_objs[647]),/* OBJ_international_organizations  2 23 */
&(nid_objs[380]),/* OBJ_dod                          1 3 6 */
&(nid_objs[12]),/* OBJ_X509                         2 5 4 */
&(nid_objs[378]),/* OBJ_X500algorithms               2 5 8 */
&(nid_objs[81]),/* OBJ_id_ce                        2 5 29 */
&(nid_objs[512]),/* OBJ_id_set                       2 23 42 */
&(nid_objs[678]),/* OBJ_wap                          2 23 43 */
&(nid_objs[435]),/* OBJ_pss                          0 9 2342 */
&(nid_objs[183]),/* OBJ_ISO_US                       1 2 840 */
&(nid_objs[381]),/* OBJ_iana                         1 3 6 1 */
&(nid_objs[677]),/* OBJ_certicom_arc                 1 3 132 */
&(nid_objs[394]),/* OBJ_selected_attribute_types     2 5 1 5 */
&(nid_objs[13]),/* OBJ_commonName                   2 5 4 3 */
&(nid_objs[100]),/* OBJ_surname                      2 5 4 4 */
&(nid_objs[105]),/* OBJ_serialNumber                 2 5 4 5 */
&(nid_objs[14]),/* OBJ_countryName                  2 5 4 6 */
&(nid_objs[15]),/* OBJ_localityName                 2 5 4 7 */
&(nid_objs[16]),/* OBJ_stateOrProvinceName          2 5 4 8 */
&(nid_objs[660]),/* OBJ_streetAddress                2 5 4 9 */
&(nid_objs[17]),/* OBJ_organizationName             2 5 4 10 */
&(nid_objs[18]),/* OBJ_organizationalUnitName       2 5 4 11 */
&(nid_objs[106]),/* OBJ_title                        2 5 4 12 */
&(nid_objs[107]),/* OBJ_description                  2 5 4 13 */
&(nid_objs[661]),/* OBJ_postalCode                   2 5 4 17 */
&(nid_objs[173]),/* OBJ_name                         2 5 4 41 */
&(nid_objs[99]),/* OBJ_givenName                    2 5 4 42 */
&(nid_objs[101]),/* OBJ_initials                     2 5 4 43 */
&(nid_objs[509]),/* OBJ_generationQualifier          2 5 4 44 */
&(nid_objs[503]),/* OBJ_x500UniqueIdentifier         2 5 4 45 */
&(nid_objs[174]),/* OBJ_dnQualifier                  2 5 4 46 */
&(nid_objs[510]),/* OBJ_pseudonym                    2 5 4 65 */
&(nid_objs[400]),/* OBJ_role                         2 5 4 72 */
&(nid_objs[769]),/* OBJ_subject_directory_attributes 2 5 29 9 */
&(nid_objs[82]),/* OBJ_subject_key_identifier       2 5 29 14 */
&(nid_objs[83]),/* OBJ_key_usage                    2 5 29 15 */
&(nid_objs[84]),/* OBJ_private_key_usage_period     2 5 29 16 */
&(nid_objs[85]),/* OBJ_subject_alt_name             2 5 29 17 */
&(nid_objs[86]),/* OBJ_issuer_alt_name              2 5 29 18 */
&(nid_objs[87]),/* OBJ_basic_constraints            2 5 29 19 */
&(nid_objs[88]),/* OBJ_crl_number                   2 5 29 20 */
&(nid_objs[141]),/* OBJ_crl_reason                   2 5 29 21 */
&(nid_objs[430]),/* OBJ_hold_instruction_code        2 5 29 23 */
&(nid_objs[142]),/* OBJ_invalidity_date              2 5 29 24 */
&(nid_objs[140]),/* OBJ_delta_crl                    2 5 29 27 */
&(nid_objs[770]),/* OBJ_issuing_distribution_point   2 5 29 28 */
&(nid_objs[771]),/* OBJ_certificate_issuer           2 5 29 29 */
&(nid_objs[666]),/* OBJ_name_constraints             2 5 29 30 */
&(nid_objs[103]),/* OBJ_crl_distribution_points      2 5 29 31 */
&(nid_objs[89]),/* OBJ_certificate_policies         2 5 29 32 */
&(nid_objs[747]),/* OBJ_policy_mappings              2 5 29 33 */
&(nid_objs[90]),/* OBJ_authority_key_identifier     2 5 29 35 */
&(nid_objs[401]),/* OBJ_policy_constraints           2 5 29 36 */
&(nid_objs[126]),/* OBJ_ext_key_usage                2 5 29 37 */
&(nid_objs[748]),/* OBJ_inhibit_any_policy           2 5 29 54 */
&(nid_objs[402]),/* OBJ_target_information           2 5 29 55 */
&(nid_objs[403]),/* OBJ_no_rev_avail                 2 5 29 56 */
&(nid_objs[513]),/* OBJ_set_ctype                    2 23 42 0 */
&(nid_objs[514]),/* OBJ_set_msgExt                   2 23 42 1 */
&(nid_objs[515]),/* OBJ_set_attr                     2 23 42 3 */
&(nid_objs[516]),/* OBJ_set_policy                   2 23 42 5 */
&(nid_objs[517]),/* OBJ_set_certExt                  2 23 42 7 */
&(nid_objs[518]),/* OBJ_set_brand                    2 23 42 8 */
&(nid_objs[679]),/* OBJ_wap_wsg                      2 23 43 13 */
&(nid_objs[382]),/* OBJ_Directory                    1 3 6 1 1 */
&(nid_objs[383]),/* OBJ_Management                   1 3 6 1 2 */
&(nid_objs[384]),/* OBJ_Experimental                 1 3 6 1 3 */
&(nid_objs[385]),/* OBJ_Private                      1 3 6 1 4 */
&(nid_objs[386]),/* OBJ_Security                     1 3 6 1 5 */
&(nid_objs[387]),/* OBJ_SNMPv2                       1 3 6 1 6 */
&(nid_objs[388]),/* OBJ_Mail                         1 3 6 1 7 */
&(nid_objs[376]),/* OBJ_algorithm                    1 3 14 3 2 */
&(nid_objs[395]),/* OBJ_clearance                    2 5 1 5 55 */
&(nid_objs[19]),/* OBJ_rsa                          2 5 8 1 1 */
&(nid_objs[96]),/* OBJ_mdc2WithRSA                  2 5 8 3 100 */
&(nid_objs[95]),/* OBJ_mdc2                         2 5 8 3 101 */
&(nid_objs[746]),/* OBJ_any_policy                   2 5 29 32 0 */
&(nid_objs[519]),/* OBJ_setct_PANData                2 23 42 0 0 */
&(nid_objs[520]),/* OBJ_setct_PANToken               2 23 42 0 1 */
&(nid_objs[521]),/* OBJ_setct_PANOnly                2 23 42 0 2 */
&(nid_objs[522]),/* OBJ_setct_OIData                 2 23 42 0 3 */
&(nid_objs[523]),/* OBJ_setct_PI                     2 23 42 0 4 */
&(nid_objs[524]),/* OBJ_setct_PIData                 2 23 42 0 5 */
&(nid_objs[525]),/* OBJ_setct_PIDataUnsigned         2 23 42 0 6 */
&(nid_objs[526]),/* OBJ_setct_HODInput               2 23 42 0 7 */
&(nid_objs[527]),/* OBJ_setct_AuthResBaggage         2 23 42 0 8 */
&(nid_objs[528]),/* OBJ_setct_AuthRevReqBaggage      2 23 42 0 9 */
&(nid_objs[529]),/* OBJ_setct_AuthRevResBaggage      2 23 42 0 10 */
&(nid_objs[530]),/* OBJ_setct_CapTokenSeq            2 23 42 0 11 */
&(nid_objs[531]),/* OBJ_setct_PInitResData           2 23 42 0 12 */
&(nid_objs[532]),/* OBJ_setct_PI_TBS                 2 23 42 0 13 */
&(nid_objs[533]),/* OBJ_setct_PResData               2 23 42 0 14 */
&(nid_objs[534]),/* OBJ_setct_AuthReqTBS             2 23 42 0 16 */
&(nid_objs[535]),/* OBJ_setct_AuthResTBS             2 23 42 0 17 */
&(nid_objs[536]),/* OBJ_setct_AuthResTBSX            2 23 42 0 18 */
&(nid_objs[537]),/* OBJ_setct_AuthTokenTBS           2 23 42 0 19 */
&(nid_objs[538]),/* OBJ_setct_CapTokenData           2 23 42 0 20 */
&(nid_objs[539]),/* OBJ_setct_CapTokenTBS            2 23 42 0 21 */
&(nid_objs[540]),/* OBJ_setct_AcqCardCodeMsg         2 23 42 0 22 */
&(nid_objs[541]),/* OBJ_setct_AuthRevReqTBS          2 23 42 0 23 */
&(nid_objs[542]),/* OBJ_setct_AuthRevResData         2 23 42 0 24 */
&(nid_objs[543]),/* OBJ_setct_AuthRevResTBS          2 23 42 0 25 */
&(nid_objs[544]),/* OBJ_setct_CapReqTBS              2 23 42 0 26 */
&(nid_objs[545]),/* OBJ_setct_CapReqTBSX             2 23 42 0 27 */
&(nid_objs[546]),/* OBJ_setct_CapResData             2 23 42 0 28 */
&(nid_objs[547]),/* OBJ_setct_CapRevReqTBS           2 23 42 0 29 */
&(nid_objs[548]),/* OBJ_setct_CapRevReqTBSX          2 23 42 0 30 */
&(nid_objs[549]),/* OBJ_setct_CapRevResData          2 23 42 0 31 */
&(nid_objs[550]),/* OBJ_setct_CredReqTBS             2 23 42 0 32 */
&(nid_objs[551]),/* OBJ_setct_CredReqTBSX            2 23 42 0 33 */
&(nid_objs[552]),/* OBJ_setct_CredResData            2 23 42 0 34 */
&(nid_objs[553]),/* OBJ_setct_CredRevReqTBS          2 23 42 0 35 */
&(nid_objs[554]),/* OBJ_setct_CredRevReqTBSX         2 23 42 0 36 */
&(nid_objs[555]),/* OBJ_setct_CredRevResData         2 23 42 0 37 */
&(nid_objs[556]),/* OBJ_setct_PCertReqData           2 23 42 0 38 */
&(nid_objs[557]),/* OBJ_setct_PCertResTBS            2 23 42 0 39 */
&(nid_objs[558]),/* OBJ_setct_BatchAdminReqData      2 23 42 0 40 */
&(nid_objs[559]),/* OBJ_setct_BatchAdminResData      2 23 42 0 41 */
&(nid_objs[560]),/* OBJ_setct_CardCInitResTBS        2 23 42 0 42 */
&(nid_objs[561]),/* OBJ_setct_MeAqCInitResTBS        2 23 42 0 43 */
&(nid_objs[562]),/* OBJ_setct_RegFormResTBS          2 23 42 0 44 */
&(nid_objs[563]),/* OBJ_setct_CertReqData            2 23 42 0 45 */
&(nid_objs[564]),/* OBJ_setct_CertReqTBS             2 23 42 0 46 */
&(nid_objs[565]),/* OBJ_setct_CertResData            2 23 42 0 47 */
&(nid_objs[566]),/* OBJ_setct_CertInqReqTBS          2 23 42 0 48 */
&(nid_objs[567]),/* OBJ_setct_ErrorTBS               2 23 42 0 49 */
&(nid_objs[568]),/* OBJ_setct_PIDualSignedTBE        2 23 42 0 50 */
&(nid_objs[569]),/* OBJ_setct_PIUnsignedTBE          2 23 42 0 51 */
&(nid_objs[570]),/* OBJ_setct_AuthReqTBE             2 23 42 0 52 */
&(nid_objs[571]),/* OBJ_setct_AuthResTBE             2 23 42 0 53 */
&(nid_objs[572]),/* OBJ_setct_AuthResTBEX            2 23 42 0 54 */
&(nid_objs[573]),/* OBJ_setct_AuthTokenTBE           2 23 42 0 55 */
&(nid_objs[574]),/* OBJ_setct_CapTokenTBE            2 23 42 0 56 */
&(nid_objs[575]),/* OBJ_setct_CapTokenTBEX           2 23 42 0 57 */
&(nid_objs[576]),/* OBJ_setct_AcqCardCodeMsgTBE      2 23 42 0 58 */
&(nid_objs[577]),/* OBJ_setct_AuthRevReqTBE          2 23 42 0 59 */
&(nid_objs[578]),/* OBJ_setct_AuthRevResTBE          2 23 42 0 60 */
&(nid_objs[579]),/* OBJ_setct_AuthRevResTBEB         2 23 42 0 61 */
&(nid_objs[580]),/* OBJ_setct_CapReqTBE              2 23 42 0 62 */
&(nid_objs[581]),/* OBJ_setct_CapReqTBEX             2 23 42 0 63 */
&(nid_objs[582]),/* OBJ_setct_CapResTBE              2 23 42 0 64 */
&(nid_objs[583]),/* OBJ_setct_CapRevReqTBE           2 23 42 0 65 */
&(nid_objs[584]),/* OBJ_setct_CapRevReqTBEX          2 23 42 0 66 */
&(nid_objs[585]),/* OBJ_setct_CapRevResTBE           2 23 42 0 67 */
&(nid_objs[586]),/* OBJ_setct_CredReqTBE             2 23 42 0 68 */
&(nid_objs[587]),/* OBJ_setct_CredReqTBEX            2 23 42 0 69 */
&(nid_objs[588]),/* OBJ_setct_CredResTBE             2 23 42 0 70 */
&(nid_objs[589]),/* OBJ_setct_CredRevReqTBE          2 23 42 0 71 */
&(nid_objs[590]),/* OBJ_setct_CredRevReqTBEX         2 23 42 0 72 */
&(nid_objs[591]),/* OBJ_setct_CredRevResTBE          2 23 42 0 73 */
&(nid_objs[592]),/* OBJ_setct_BatchAdminReqTBE       2 23 42 0 74 */
&(nid_objs[593]),/* OBJ_setct_BatchAdminResTBE       2 23 42 0 75 */
&(nid_objs[594]),/* OBJ_setct_RegFormReqTBE          2 23 42 0 76 */
&(nid_objs[595]),/* OBJ_setct_CertReqTBE             2 23 42 0 77 */
&(nid_objs[596]),/* OBJ_setct_CertReqTBEX            2 23 42 0 78 */
&(nid_objs[597]),/* OBJ_setct_CertResTBE             2 23 42 0 79 */
&(nid_objs[598]),/* OBJ_setct_CRLNotificationTBS     2 23 42 0 80 */
&(nid_objs[599]),/* OBJ_setct_CRLNotificationResTBS  2 23 42 0 81 */
&(nid_objs[600]),/* OBJ_setct_BCIDistributionTBS     2 23 42 0 82 */
&(nid_objs[601]),/* OBJ_setext_genCrypt              2 23 42 1 1 */
&(nid_objs[602]),/* OBJ_setext_miAuth                2 23 42 1 3 */
&(nid_objs[603]),/* OBJ_setext_pinSecure             2 23 42 1 4 */
&(nid_objs[604]),/* OBJ_setext_pinAny                2 23 42 1 5 */
&(nid_objs[605]),/* OBJ_setext_track2                2 23 42 1 7 */
&(nid_objs[606]),/* OBJ_setext_cv                    2 23 42 1 8 */
&(nid_objs[620]),/* OBJ_setAttr_Cert                 2 23 42 3 0 */
&(nid_objs[621]),/* OBJ_setAttr_PGWYcap              2 23 42 3 1 */
&(nid_objs[622]),/* OBJ_setAttr_TokenType            2 23 42 3 2 */
&(nid_objs[623]),/* OBJ_setAttr_IssCap               2 23 42 3 3 */
&(nid_objs[607]),/* OBJ_set_policy_root              2 23 42 5 0 */
&(nid_objs[608]),/* OBJ_setCext_hashedRoot           2 23 42 7 0 */
&(nid_objs[609]),/* OBJ_setCext_certType             2 23 42 7 1 */
&(nid_objs[610]),/* OBJ_setCext_merchData            2 23 42 7 2 */
&(nid_objs[611]),/* OBJ_setCext_cCertRequired        2 23 42 7 3 */
&(nid_objs[612]),/* OBJ_setCext_tunneling            2 23 42 7 4 */
&(nid_objs[613]),/* OBJ_setCext_setExt               2 23 42 7 5 */
&(nid_objs[614]),/* OBJ_setCext_setQualf             2 23 42 7 6 */
&(nid_objs[615]),/* OBJ_setCext_PGWYcapabilities     2 23 42 7 7 */
&(nid_objs[616]),/* OBJ_setCext_TokenIdentifier      2 23 42 7 8 */
&(nid_objs[617]),/* OBJ_setCext_Track2Data           2 23 42 7 9 */
&(nid_objs[618]),/* OBJ_setCext_TokenType            2 23 42 7 10 */
&(nid_objs[619]),/* OBJ_setCext_IssuerCapabilities   2 23 42 7 11 */
&(nid_objs[636]),/* OBJ_set_brand_IATA_ATA           2 23 42 8 1 */
&(nid_objs[640]),/* OBJ_set_brand_Visa               2 23 42 8 4 */
&(nid_objs[641]),/* OBJ_set_brand_MasterCard         2 23 42 8 5 */
&(nid_objs[637]),/* OBJ_set_brand_Diners             2 23 42 8 30 */
&(nid_objs[638]),/* OBJ_set_brand_AmericanExpress    2 23 42 8 34 */
&(nid_objs[639]),/* OBJ_set_brand_JCB                2 23 42 8 35 */
&(nid_objs[184]),/* OBJ_X9_57                        1 2 840 10040 */
&(nid_objs[405]),/* OBJ_ansi_X9_62                   1 2 840 10045 */
&(nid_objs[389]),/* OBJ_Enterprises                  1 3 6 1 4 1 */
&(nid_objs[504]),/* OBJ_mime_mhs                     1 3 6 1 7 1 */
&(nid_objs[104]),/* OBJ_md5WithRSA                   1 3 14 3 2 3 */
&(nid_objs[29]),/* OBJ_des_ecb                      1 3 14 3 2 6 */
&(nid_objs[31]),/* OBJ_des_cbc                      1 3 14 3 2 7 */
&(nid_objs[45]),/* OBJ_des_ofb64                    1 3 14 3 2 8 */
&(nid_objs[30]),/* OBJ_des_cfb64                    1 3 14 3 2 9 */
&(nid_objs[377]),/* OBJ_rsaSignature                 1 3 14 3 2 11 */
&(nid_objs[67]),/* OBJ_dsa_2                        1 3 14 3 2 12 */
&(nid_objs[66]),/* OBJ_dsaWithSHA                   1 3 14 3 2 13 */
&(nid_objs[42]),/* OBJ_shaWithRSAEncryption         1 3 14 3 2 15 */
&(nid_objs[32]),/* OBJ_des_ede_ecb                  1 3 14 3 2 17 */
&(nid_objs[41]),/* OBJ_sha                          1 3 14 3 2 18 */
&(nid_objs[64]),/* OBJ_sha1                         1 3 14 3 2 26 */
&(nid_objs[70]),/* OBJ_dsaWithSHA1_2                1 3 14 3 2 27 */
&(nid_objs[115]),/* OBJ_sha1WithRSA                  1 3 14 3 2 29 */
&(nid_objs[117]),/* OBJ_ripemd160                    1 3 36 3 2 1 */
&(nid_objs[143]),/* OBJ_sxnet                        1 3 101 1 4 1 */
&(nid_objs[721]),/* OBJ_sect163k1                    1 3 132 0 1 */
&(nid_objs[722]),/* OBJ_sect163r1                    1 3 132 0 2 */
&(nid_objs[728]),/* OBJ_sect239k1                    1 3 132 0 3 */
&(nid_objs[717]),/* OBJ_sect113r1                    1 3 132 0 4 */
&(nid_objs[718]),/* OBJ_sect113r2                    1 3 132 0 5 */
&(nid_objs[704]),/* OBJ_secp112r1                    1 3 132 0 6 */
&(nid_objs[705]),/* OBJ_secp112r2                    1 3 132 0 7 */
&(nid_objs[709]),/* OBJ_secp160r1                    1 3 132 0 8 */
&(nid_objs[708]),/* OBJ_secp160k1                    1 3 132 0 9 */
&(nid_objs[714]),/* OBJ_secp256k1                    1 3 132 0 10 */
&(nid_objs[723]),/* OBJ_sect163r2                    1 3 132 0 15 */
&(nid_objs[729]),/* OBJ_sect283k1                    1 3 132 0 16 */
&(nid_objs[730]),/* OBJ_sect283r1                    1 3 132 0 17 */
&(nid_objs[719]),/* OBJ_sect131r1                    1 3 132 0 22 */
&(nid_objs[720]),/* OBJ_sect131r2                    1 3 132 0 23 */
&(nid_objs[724]),/* OBJ_sect193r1                    1 3 132 0 24 */
&(nid_objs[725]),/* OBJ_sect193r2                    1 3 132 0 25 */
&(nid_objs[726]),/* OBJ_sect233k1                    1 3 132 0 26 */
&(nid_objs[727]),/* OBJ_sect233r1                    1 3 132 0 27 */
&(nid_objs[706]),/* OBJ_secp128r1                    1 3 132 0 28 */
&(nid_objs[707]),/* OBJ_secp128r2                    1 3 132 0 29 */
&(nid_objs[710]),/* OBJ_secp160r2                    1 3 132 0 30 */
&(nid_objs[711]),/* OBJ_secp192k1                    1 3 132 0 31 */
&(nid_objs[712]),/* OBJ_secp224k1                    1 3 132 0 32 */
&(nid_objs[713]),/* OBJ_secp224r1                    1 3 132 0 33 */
&(nid_objs[715]),/* OBJ_secp384r1                    1 3 132 0 34 */
&(nid_objs[716]),/* OBJ_secp521r1                    1 3 132 0 35 */
&(nid_objs[731]),/* OBJ_sect409k1                    1 3 132 0 36 */
&(nid_objs[732]),/* OBJ_sect409r1                    1 3 132 0 37 */
&(nid_objs[733]),/* OBJ_sect571k1                    1 3 132 0 38 */
&(nid_objs[734]),/* OBJ_sect571r1                    1 3 132 0 39 */
&(nid_objs[624]),/* OBJ_set_rootKeyThumb             2 23 42 3 0 0 */
&(nid_objs[625]),/* OBJ_set_addPolicy                2 23 42 3 0 1 */
&(nid_objs[626]),/* OBJ_setAttr_Token_EMV            2 23 42 3 2 1 */
&(nid_objs[627]),/* OBJ_setAttr_Token_B0Prime        2 23 42 3 2 2 */
&(nid_objs[628]),/* OBJ_setAttr_IssCap_CVM           2 23 42 3 3 3 */
&(nid_objs[629]),/* OBJ_setAttr_IssCap_T2            2 23 42 3 3 4 */
&(nid_objs[630]),/* OBJ_setAttr_IssCap_Sig           2 23 42 3 3 5 */
&(nid_objs[642]),/* OBJ_set_brand_Novus              2 23 42 8 6011 */
&(nid_objs[735]),/* OBJ_wap_wsg_idm_ecid_wtls1       2 23 43 13 4 1 */
&(nid_objs[736]),/* OBJ_wap_wsg_idm_ecid_wtls3       2 23 43 13 4 3 */
&(nid_objs[737]),/* OBJ_wap_wsg_idm_ecid_wtls4       2 23 43 13 4 4 */
&(nid_objs[738]),/* OBJ_wap_wsg_idm_ecid_wtls5       2 23 43 13 4 5 */
&(nid_objs[739]),/* OBJ_wap_wsg_idm_ecid_wtls6       2 23 43 13 4 6 */
&(nid_objs[740]),/* OBJ_wap_wsg_idm_ecid_wtls7       2 23 43 13 4 7 */
&(nid_objs[741]),/* OBJ_wap_wsg_idm_ecid_wtls8       2 23 43 13 4 8 */
&(nid_objs[742]),/* OBJ_wap_wsg_idm_ecid_wtls9       2 23 43 13 4 9 */
&(nid_objs[743]),/* OBJ_wap_wsg_idm_ecid_wtls10      2 23 43 13 4 10 */
&(nid_objs[744]),/* OBJ_wap_wsg_idm_ecid_wtls11      2 23 43 13 4 11 */
&(nid_objs[745]),/* OBJ_wap_wsg_idm_ecid_wtls12      2 23 43 13 4 12 */
&(nid_objs[124]),/* OBJ_rle_compression              1 1 1 1 666 1 */
&(nid_objs[125]),/* OBJ_zlib_compression             1 1 1 1 666 2 */
&(nid_objs[773]),/* OBJ_kisa                         1 2 410 200004 */
&(nid_objs[ 1]),/* OBJ_rsadsi                       1 2 840 113549 */
&(nid_objs[185]),/* OBJ_X9cm                         1 2 840 10040 4 */
&(nid_objs[127]),/* OBJ_id_pkix                      1 3 6 1 5 5 7 */
&(nid_objs[505]),/* OBJ_mime_mhs_headings            1 3 6 1 7 1 1 */
&(nid_objs[506]),/* OBJ_mime_mhs_bodies              1 3 6 1 7 1 2 */
&(nid_objs[119]),/* OBJ_ripemd160WithRSA             1 3 36 3 3 1 2 */
&(nid_objs[631]),/* OBJ_setAttr_GenCryptgrm          2 23 42 3 3 3 1 */
&(nid_objs[632]),/* OBJ_setAttr_T2Enc                2 23 42 3 3 4 1 */
&(nid_objs[633]),/* OBJ_setAttr_T2cleartxt           2 23 42 3 3 4 2 */
&(nid_objs[634]),/* OBJ_setAttr_TokICCsig            2 23 42 3 3 5 1 */
&(nid_objs[635]),/* OBJ_setAttr_SecDevSig            2 23 42 3 3 5 2 */
&(nid_objs[436]),/* OBJ_ucl                          0 9 2342 19200300 */
&(nid_objs[ 2]),/* OBJ_pkcs                         1 2 840 113549 1 */
&(nid_objs[431]),/* OBJ_hold_instruction_none        1 2 840 10040 2 1 */
&(nid_objs[432]),/* OBJ_hold_instruction_call_issuer 1 2 840 10040 2 2 */
&(nid_objs[433]),/* OBJ_hold_instruction_reject      1 2 840 10040 2 3 */
&(nid_objs[116]),/* OBJ_dsa                          1 2 840 10040 4 1 */
&(nid_objs[113]),/* OBJ_dsaWithSHA1                  1 2 840 10040 4 3 */
&(nid_objs[406]),/* OBJ_X9_62_prime_field            1 2 840 10045 1 1 */
&(nid_objs[407]),/* OBJ_X9_62_characteristic_two_field 1 2 840 10045 1 2 */
&(nid_objs[408]),/* OBJ_X9_62_id_ecPublicKey         1 2 840 10045 2 1 */
&(nid_objs[416]),/* OBJ_ecdsa_with_SHA1              1 2 840 10045 4 1 */
&(nid_objs[258]),/* OBJ_id_pkix_mod                  1 3 6 1 5 5 7 0 */
&(nid_objs[175]),/* OBJ_id_pe                        1 3 6 1 5 5 7 1 */
&(nid_objs[259]),/* OBJ_id_qt                        1 3 6 1 5 5 7 2 */
&(nid_objs[128]),/* OBJ_id_kp                        1 3 6 1 5 5 7 3 */
&(nid_objs[260]),/* OBJ_id_it                        1 3 6 1 5 5 7 4 */
&(nid_objs[261]),/* OBJ_id_pkip                      1 3 6 1 5 5 7 5 */
&(nid_objs[262]),/* OBJ_id_alg                       1 3 6 1 5 5 7 6 */
&(nid_objs[263]),/* OBJ_id_cmc                       1 3 6 1 5 5 7 7 */
&(nid_objs[264]),/* OBJ_id_on                        1 3 6 1 5 5 7 8 */
&(nid_objs[265]),/* OBJ_id_pda                       1 3 6 1 5 5 7 9 */
&(nid_objs[266]),/* OBJ_id_aca                       1 3 6 1 5 5 7 10 */
&(nid_objs[267]),/* OBJ_id_qcs                       1 3 6 1 5 5 7 11 */
&(nid_objs[268]),/* OBJ_id_cct                       1 3 6 1 5 5 7 12 */
&(nid_objs[662]),/* OBJ_id_ppl                       1 3 6 1 5 5 7 21 */
&(nid_objs[176]),/* OBJ_id_ad                        1 3 6 1 5 5 7 48 */
&(nid_objs[507]),/* OBJ_id_hex_partial_message       1 3 6 1 7 1 1 1 */
&(nid_objs[508]),/* OBJ_id_hex_multipart_message     1 3 6 1 7 1 1 2 */
&(nid_objs[57]),/* OBJ_netscape                     2 16 840 1 113730 */
&(nid_objs[754]),/* OBJ_camellia_128_ecb             0 3 4401 5 3 1 9 1 */
&(nid_objs[766]),/* OBJ_camellia_128_ofb128          0 3 4401 5 3 1 9 3 */
&(nid_objs[757]),/* OBJ_camellia_128_cfb128          0 3 4401 5 3 1 9 4 */
&(nid_objs[755]),/* OBJ_camellia_192_ecb             0 3 4401 5 3 1 9 21 */
&(nid_objs[767]),/* OBJ_camellia_192_ofb128          0 3 4401 5 3 1 9 23 */
&(nid_objs[758]),/* OBJ_camellia_192_cfb128          0 3 4401 5 3 1 9 24 */
&(nid_objs[756]),/* OBJ_camellia_256_ecb             0 3 4401 5 3 1 9 41 */
&(nid_objs[768]),/* OBJ_camellia_256_ofb128          0 3 4401 5 3 1 9 43 */
&(nid_objs[759]),/* OBJ_camellia_256_cfb128          0 3 4401 5 3 1 9 44 */
&(nid_objs[437]),/* OBJ_pilot                        0 9 2342 19200300 100 */
&(nid_objs[776]),/* OBJ_seed_ecb                     1 2 410 200004 1 3 */
&(nid_objs[777]),/* OBJ_seed_cbc                     1 2 410 200004 1 4 */
&(nid_objs[779]),/* OBJ_seed_cfb128                  1 2 410 200004 1 5 */
&(nid_objs[778]),/* OBJ_seed_ofb128                  1 2 410 200004 1 6 */
&(nid_objs[186]),/* OBJ_pkcs1                        1 2 840 113549 1 1 */
&(nid_objs[27]),/* OBJ_pkcs3                        1 2 840 113549 1 3 */
&(nid_objs[187]),/* OBJ_pkcs5                        1 2 840 113549 1 5 */
&(nid_objs[20]),/* OBJ_pkcs7                        1 2 840 113549 1 7 */
&(nid_objs[47]),/* OBJ_pkcs9                        1 2 840 113549 1 9 */
&(nid_objs[ 3]),/* OBJ_md2                          1 2 840 113549 2 2 */
&(nid_objs[257]),/* OBJ_md4                          1 2 840 113549 2 4 */
&(nid_objs[ 4]),/* OBJ_md5                          1 2 840 113549 2 5 */
&(nid_objs[163]),/* OBJ_hmacWithSHA1                 1 2 840 113549 2 7 */
&(nid_objs[37]),/* OBJ_rc2_cbc                      1 2 840 113549 3 2 */
&(nid_objs[ 5]),/* OBJ_rc4                          1 2 840 113549 3 4 */
&(nid_objs[44]),/* OBJ_des_ede3_cbc                 1 2 840 113549 3 7 */
&(nid_objs[120]),/* OBJ_rc5_cbc                      1 2 840 113549 3 8 */
&(nid_objs[643]),/* OBJ_des_cdmf                     1 2 840 113549 3 10 */
&(nid_objs[680]),/* OBJ_X9_62_id_characteristic_two_basis 1 2 840 10045 1 2 3 */
&(nid_objs[684]),/* OBJ_X9_62_c2pnb163v1             1 2 840 10045 3 0 1 */
&(nid_objs[685]),/* OBJ_X9_62_c2pnb163v2             1 2 840 10045 3 0 2 */
&(nid_objs[686]),/* OBJ_X9_62_c2pnb163v3             1 2 840 10045 3 0 3 */
&(nid_objs[687]),/* OBJ_X9_62_c2pnb176v1             1 2 840 10045 3 0 4 */
&(nid_objs[688]),/* OBJ_X9_62_c2tnb191v1             1 2 840 10045 3 0 5 */
&(nid_objs[689]),/* OBJ_X9_62_c2tnb191v2             1 2 840 10045 3 0 6 */
&(nid_objs[690]),/* OBJ_X9_62_c2tnb191v3             1 2 840 10045 3 0 7 */
&(nid_objs[691]),/* OBJ_X9_62_c2onb191v4             1 2 840 10045 3 0 8 */
&(nid_objs[692]),/* OBJ_X9_62_c2onb191v5             1 2 840 10045 3 0 9 */
&(nid_objs[693]),/* OBJ_X9_62_c2pnb208w1             1 2 840 10045 3 0 10 */
&(nid_objs[694]),/* OBJ_X9_62_c2tnb239v1             1 2 840 10045 3 0 11 */
&(nid_objs[695]),/* OBJ_X9_62_c2tnb239v2             1 2 840 10045 3 0 12 */
&(nid_objs[696]),/* OBJ_X9_62_c2tnb239v3             1 2 840 10045 3 0 13 */
&(nid_objs[697]),/* OBJ_X9_62_c2onb239v4             1 2 840 10045 3 0 14 */
&(nid_objs[698]),/* OBJ_X9_62_c2onb239v5             1 2 840 10045 3 0 15 */
&(nid_objs[699]),/* OBJ_X9_62_c2pnb272w1             1 2 840 10045 3 0 16 */
&(nid_objs[700]),/* OBJ_X9_62_c2pnb304w1             1 2 840 10045 3 0 17 */
&(nid_objs[701]),/* OBJ_X9_62_c2tnb359v1             1 2 840 10045 3 0 18 */
&(nid_objs[702]),/* OBJ_X9_62_c2pnb368w1             1 2 840 10045 3 0 19 */
&(nid_objs[703]),/* OBJ_X9_62_c2tnb431r1             1 2 840 10045 3 0 20 */
&(nid_objs[409]),/* OBJ_X9_62_prime192v1             1 2 840 10045 3 1 1 */
&(nid_objs[410]),/* OBJ_X9_62_prime192v2             1 2 840 10045 3 1 2 */
&(nid_objs[411]),/* OBJ_X9_62_prime192v3             1 2 840 10045 3 1 3 */
&(nid_objs[412]),/* OBJ_X9_62_prime239v1             1 2 840 10045 3 1 4 */
&(nid_objs[413]),/* OBJ_X9_62_prime239v2             1 2 840 10045 3 1 5 */
&(nid_objs[414]),/* OBJ_X9_62_prime239v3             1 2 840 10045 3 1 6 */
&(nid_objs[415]),/* OBJ_X9_62_prime256v1             1 2 840 10045 3 1 7 */
&(nid_objs[269]),/* OBJ_id_pkix1_explicit_88         1 3 6 1 5 5 7 0 1 */
&(nid_objs[270]),/* OBJ_id_pkix1_implicit_88         1 3 6 1 5 5 7 0 2 */
&(nid_objs[271]),/* OBJ_id_pkix1_explicit_93         1 3 6 1 5 5 7 0 3 */
&(nid_objs[272]),/* OBJ_id_pkix1_implicit_93         1 3 6 1 5 5 7 0 4 */
&(nid_objs[273]),/* OBJ_id_mod_crmf                  1 3 6 1 5 5 7 0 5 */
&(nid_objs[274]),/* OBJ_id_mod_cmc                   1 3 6 1 5 5 7 0 6 */
&(nid_objs[275]),/* OBJ_id_mod_kea_profile_88        1 3 6 1 5 5 7 0 7 */
&(nid_objs[276]),/* OBJ_id_mod_kea_profile_93        1 3 6 1 5 5 7 0 8 */
&(nid_objs[277]),/* OBJ_id_mod_cmp                   1 3 6 1 5 5 7 0 9 */
&(nid_objs[278]),/* OBJ_id_mod_qualified_cert_88     1 3 6 1 5 5 7 0 10 */
&(nid_objs[279]),/* OBJ_id_mod_qualified_cert_93     1 3 6 1 5 5 7 0 11 */
&(nid_objs[280]),/* OBJ_id_mod_attribute_cert        1 3 6 1 5 5 7 0 12 */
&(nid_objs[281]),/* OBJ_id_mod_timestamp_protocol    1 3 6 1 5 5 7 0 13 */
&(nid_objs[282]),/* OBJ_id_mod_ocsp                  1 3 6 1 5 5 7 0 14 */
&(nid_objs[283]),/* OBJ_id_mod_dvcs                  1 3 6 1 5 5 7 0 15 */
&(nid_objs[284]),/* OBJ_id_mod_cmp2000               1 3 6 1 5 5 7 0 16 */
&(nid_objs[177]),/* OBJ_info_access                  1 3 6 1 5 5 7 1 1 */
&(nid_objs[285]),/* OBJ_biometricInfo                1 3 6 1 5 5 7 1 2 */
&(nid_objs[286]),/* OBJ_qcStatements                 1 3 6 1 5 5 7 1 3 */
&(nid_objs[287]),/* OBJ_ac_auditEntity               1 3 6 1 5 5 7 1 4 */
&(nid_objs[288]),/* OBJ_ac_targeting                 1 3 6 1 5 5 7 1 5 */
&(nid_objs[289]),/* OBJ_aaControls                   1 3 6 1 5 5 7 1 6 */
&(nid_objs[290]),/* OBJ_sbgp_ipAddrBlock             1 3 6 1 5 5 7 1 7 */
&(nid_objs[291]),/* OBJ_sbgp_autonomousSysNum        1 3 6 1 5 5 7 1 8 */
&(nid_objs[292]),/* OBJ_sbgp_routerIdentifier        1 3 6 1 5 5 7 1 9 */
&(nid_objs[397]),/* OBJ_ac_proxying                  1 3 6 1 5 5 7 1 10 */
&(nid_objs[398]),/* OBJ_sinfo_access                 1 3 6 1 5 5 7 1 11 */
&(nid_objs[663]),/* OBJ_proxyCertInfo                1 3 6 1 5 5 7 1 14 */
&(nid_objs[164]),/* OBJ_id_qt_cps                    1 3 6 1 5 5 7 2 1 */
&(nid_objs[165]),/* OBJ_id_qt_unotice                1 3 6 1 5 5 7 2 2 */
&(nid_objs[293]),/* OBJ_textNotice                   1 3 6 1 5 5 7 2 3 */
&(nid_objs[129]),/* OBJ_server_auth                  1 3 6 1 5 5 7 3 1 */
&(nid_objs[130]),/* OBJ_client_auth                  1 3 6 1 5 5 7 3 2 */
&(nid_objs[131]),/* OBJ_code_sign                    1 3 6 1 5 5 7 3 3 */
&(nid_objs[132]),/* OBJ_email_protect                1 3 6 1 5 5 7 3 4 */
&(nid_objs[294]),/* OBJ_ipsecEndSystem               1 3 6 1 5 5 7 3 5 */
&(nid_objs[295]),/* OBJ_ipsecTunnel                  1 3 6 1 5 5 7 3 6 */
&(nid_objs[296]),/* OBJ_ipsecUser                    1 3 6 1 5 5 7 3 7 */
&(nid_objs[133]),/* OBJ_time_stamp                   1 3 6 1 5 5 7 3 8 */
&(nid_objs[180]),/* OBJ_OCSP_sign                    1 3 6 1 5 5 7 3 9 */
&(nid_objs[297]),/* OBJ_dvcs                         1 3 6 1 5 5 7 3 10 */
&(nid_objs[298]),/* OBJ_id_it_caProtEncCert          1 3 6 1 5 5 7 4 1 */
&(nid_objs[299]),/* OBJ_id_it_signKeyPairTypes       1 3 6 1 5 5 7 4 2 */
&(nid_objs[300]),/* OBJ_id_it_encKeyPairTypes        1 3 6 1 5 5 7 4 3 */
&(nid_objs[301]),/* OBJ_id_it_preferredSymmAlg       1 3 6 1 5 5 7 4 4 */
&(nid_objs[302]),/* OBJ_id_it_caKeyUpdateInfo        1 3 6 1 5 5 7 4 5 */
&(nid_objs[303]),/* OBJ_id_it_currentCRL             1 3 6 1 5 5 7 4 6 */
&(nid_objs[304]),/* OBJ_id_it_unsupportedOIDs        1 3 6 1 5 5 7 4 7 */
&(nid_objs[305]),/* OBJ_id_it_subscriptionRequest    1 3 6 1 5 5 7 4 8 */
&(nid_objs[306]),/* OBJ_id_it_subscriptionResponse   1 3 6 1 5 5 7 4 9 */
&(nid_objs[307]),/* OBJ_id_it_keyPairParamReq        1 3 6 1 5 5 7 4 10 */
&(nid_objs[308]),/* OBJ_id_it_keyPairParamRep        1 3 6 1 5 5 7 4 11 */
&(nid_objs[309]),/* OBJ_id_it_revPassphrase          1 3 6 1 5 5 7 4 12 */
&(nid_objs[310]),/* OBJ_id_it_implicitConfirm        1 3 6 1 5 5 7 4 13 */
&(nid_objs[311]),/* OBJ_id_it_confirmWaitTime        1 3 6 1 5 5 7 4 14 */
&(nid_objs[312]),/* OBJ_id_it_origPKIMessage         1 3 6 1 5 5 7 4 15 */
&(nid_objs[313]),/* OBJ_id_regCtrl                   1 3 6 1 5 5 7 5 1 */
&(nid_objs[314]),/* OBJ_id_regInfo                   1 3 6 1 5 5 7 5 2 */
&(nid_objs[323]),/* OBJ_id_alg_des40                 1 3 6 1 5 5 7 6 1 */
&(nid_objs[324]),/* OBJ_id_alg_noSignature           1 3 6 1 5 5 7 6 2 */
&(nid_objs[325]),/* OBJ_id_alg_dh_sig_hmac_sha1      1 3 6 1 5 5 7 6 3 */
&(nid_objs[326]),/* OBJ_id_alg_dh_pop                1 3 6 1 5 5 7 6 4 */
&(nid_objs[327]),/* OBJ_id_cmc_statusInfo            1 3 6 1 5 5 7 7 1 */
&(nid_objs[328]),/* OBJ_id_cmc_identification        1 3 6 1 5 5 7 7 2 */
&(nid_objs[329]),/* OBJ_id_cmc_identityProof         1 3 6 1 5 5 7 7 3 */
&(nid_objs[330]),/* OBJ_id_cmc_dataReturn            1 3 6 1 5 5 7 7 4 */
&(nid_objs[331]),/* OBJ_id_cmc_transactionId         1 3 6 1 5 5 7 7 5 */
&(nid_objs[332]),/* OBJ_id_cmc_senderNonce           1 3 6 1 5 5 7 7 6 */
&(nid_objs[333]),/* OBJ_id_cmc_recipientNonce        1 3 6 1 5 5 7 7 7 */
&(nid_objs[334]),/* OBJ_id_cmc_addExtensions         1 3 6 1 5 5 7 7 8 */
&(nid_objs[335]),/* OBJ_id_cmc_encryptedPOP          1 3 6 1 5 5 7 7 9 */
&(nid_objs[336]),/* OBJ_id_cmc_decryptedPOP          1 3 6 1 5 5 7 7 10 */
&(nid_objs[337]),/* OBJ_id_cmc_lraPOPWitness         1 3 6 1 5 5 7 7 11 */
&(nid_objs[338]),/* OBJ_id_cmc_getCert               1 3 6 1 5 5 7 7 15 */
&(nid_objs[339]),/* OBJ_id_cmc_getCRL                1 3 6 1 5 5 7 7 16 */
&(nid_objs[340]),/* OBJ_id_cmc_revokeRequest         1 3 6 1 5 5 7 7 17 */
&(nid_objs[341]),/* OBJ_id_cmc_regInfo               1 3 6 1 5 5 7 7 18 */
&(nid_objs[342]),/* OBJ_id_cmc_responseInfo          1 3 6 1 5 5 7 7 19 */
&(nid_objs[343]),/* OBJ_id_cmc_queryPending          1 3 6 1 5 5 7 7 21 */
&(nid_objs[344]),/* OBJ_id_cmc_popLinkRandom         1 3 6 1 5 5 7 7 22 */
&(nid_objs[345]),/* OBJ_id_cmc_popLinkWitness        1 3 6 1 5 5 7 7 23 */
&(nid_objs[346]),/* OBJ_id_cmc_confirmCertAcceptance 1 3 6 1 5 5 7 7 24 */
&(nid_objs[347]),/* OBJ_id_on_personalData           1 3 6 1 5 5 7 8 1 */
&(nid_objs[348]),/* OBJ_id_pda_dateOfBirth           1 3 6 1 5 5 7 9 1 */
&(nid_objs[349]),/* OBJ_id_pda_placeOfBirth          1 3 6 1 5 5 7 9 2 */
&(nid_objs[351]),/* OBJ_id_pda_gender                1 3 6 1 5 5 7 9 3 */
&(nid_objs[352]),/* OBJ_id_pda_countryOfCitizenship  1 3 6 1 5 5 7 9 4 */
&(nid_objs[353]),/* OBJ_id_pda_countryOfResidence    1 3 6 1 5 5 7 9 5 */
&(nid_objs[354]),/* OBJ_id_aca_authenticationInfo    1 3 6 1 5 5 7 10 1 */
&(nid_objs[355]),/* OBJ_id_aca_accessIdentity        1 3 6 1 5 5 7 10 2 */
&(nid_objs[356]),/* OBJ_id_aca_chargingIdentity      1 3 6 1 5 5 7 10 3 */
&(nid_objs[357]),/* OBJ_id_aca_group                 1 3 6 1 5 5 7 10 4 */
&(nid_objs[358]),/* OBJ_id_aca_role                  1 3 6 1 5 5 7 10 5 */
&(nid_objs[399]),/* OBJ_id_aca_encAttrs              1 3 6 1 5 5 7 10 6 */
&(nid_objs[359]),/* OBJ_id_qcs_pkixQCSyntax_v1       1 3 6 1 5 5 7 11 1 */
&(nid_objs[360]),/* OBJ_id_cct_crs                   1 3 6 1 5 5 7 12 1 */
&(nid_objs[361]),/* OBJ_id_cct_PKIData               1 3 6 1 5 5 7 12 2 */
&(nid_objs[362]),/* OBJ_id_cct_PKIResponse           1 3 6 1 5 5 7 12 3 */
&(nid_objs[664]),/* OBJ_id_ppl_anyLanguage           1 3 6 1 5 5 7 21 0 */
&(nid_objs[665]),/* OBJ_id_ppl_inheritAll            1 3 6 1 5 5 7 21 1 */
&(nid_objs[667]),/* OBJ_Independent                  1 3 6 1 5 5 7 21 2 */
&(nid_objs[178]),/* OBJ_ad_OCSP                      1 3 6 1 5 5 7 48 1 */
&(nid_objs[179]),/* OBJ_ad_ca_issuers                1 3 6 1 5 5 7 48 2 */
&(nid_objs[363]),/* OBJ_ad_timeStamping              1 3 6 1 5 5 7 48 3 */
&(nid_objs[364]),/* OBJ_ad_dvcs                      1 3 6 1 5 5 7 48 4 */
&(nid_objs[58]),/* OBJ_netscape_cert_extension      2 16 840 1 113730 1 */
&(nid_objs[59]),/* OBJ_netscape_data_type           2 16 840 1 113730 2 */
&(nid_objs[438]),/* OBJ_pilotAttributeType           0 9 2342 19200300 100 1 */
&(nid_objs[439]),/* OBJ_pilotAttributeSyntax         0 9 2342 19200300 100 3 */
&(nid_objs[440]),/* OBJ_pilotObjectClass             0 9 2342 19200300 100 4 */
&(nid_objs[441]),/* OBJ_pilotGroups                  0 9 2342 19200300 100 10 */
&(nid_objs[108]),/* OBJ_cast5_cbc                    1 2 840 113533 7 66 10 */
&(nid_objs[112]),/* OBJ_pbeWithMD5AndCast5_CBC       1 2 840 113533 7 66 12 */
&(nid_objs[ 6]),/* OBJ_rsaEncryption                1 2 840 113549 1 1 1 */
&(nid_objs[ 7]),/* OBJ_md2WithRSAEncryption         1 2 840 113549 1 1 2 */
&(nid_objs[396]),/* OBJ_md4WithRSAEncryption         1 2 840 113549 1 1 3 */
&(nid_objs[ 8]),/* OBJ_md5WithRSAEncryption         1 2 840 113549 1 1 4 */
&(nid_objs[65]),/* OBJ_sha1WithRSAEncryption        1 2 840 113549 1 1 5 */
&(nid_objs[644]),/* OBJ_rsaOAEPEncryptionSET         1 2 840 113549 1 1 6 */
&(nid_objs[668]),/* OBJ_sha256WithRSAEncryption      1 2 840 113549 1 1 11 */
&(nid_objs[669]),/* OBJ_sha384WithRSAEncryption      1 2 840 113549 1 1 12 */
&(nid_objs[670]),/* OBJ_sha512WithRSAEncryption      1 2 840 113549 1 1 13 */
&(nid_objs[671]),/* OBJ_sha224WithRSAEncryption      1 2 840 113549 1 1 14 */
&(nid_objs[28]),/* OBJ_dhKeyAgreement               1 2 840 113549 1 3 1 */
&(nid_objs[ 9]),/* OBJ_pbeWithMD2AndDES_CBC         1 2 840 113549 1 5 1 */
&(nid_objs[10]),/* OBJ_pbeWithMD5AndDES_CBC         1 2 840 113549 1 5 3 */
&(nid_objs[168]),/* OBJ_pbeWithMD2AndRC2_CBC         1 2 840 113549 1 5 4 */
&(nid_objs[169]),/* OBJ_pbeWithMD5AndRC2_CBC         1 2 840 113549 1 5 6 */
&(nid_objs[170]),/* OBJ_pbeWithSHA1AndDES_CBC        1 2 840 113549 1 5 10 */
&(nid_objs[68]),/* OBJ_pbeWithSHA1AndRC2_CBC        1 2 840 113549 1 5 11 */
&(nid_objs[69]),/* OBJ_id_pbkdf2                    1 2 840 113549 1 5 12 */
&(nid_objs[161]),/* OBJ_pbes2                        1 2 840 113549 1 5 13 */
&(nid_objs[162]),/* OBJ_pbmac1                       1 2 840 113549 1 5 14 */
&(nid_objs[21]),/* OBJ_pkcs7_data                   1 2 840 113549 1 7 1 */
&(nid_objs[22]),/* OBJ_pkcs7_signed                 1 2 840 113549 1 7 2 */
&(nid_objs[23]),/* OBJ_pkcs7_enveloped              1 2 840 113549 1 7 3 */
&(nid_objs[24]),/* OBJ_pkcs7_signedAndEnveloped     1 2 840 113549 1 7 4 */
&(nid_objs[25]),/* OBJ_pkcs7_digest                 1 2 840 113549 1 7 5 */
&(nid_objs[26]),/* OBJ_pkcs7_encrypted              1 2 840 113549 1 7 6 */
&(nid_objs[48]),/* OBJ_pkcs9_emailAddress           1 2 840 113549 1 9 1 */
&(nid_objs[49]),/* OBJ_pkcs9_unstructuredName       1 2 840 113549 1 9 2 */
&(nid_objs[50]),/* OBJ_pkcs9_contentType            1 2 840 113549 1 9 3 */
&(nid_objs[51]),/* OBJ_pkcs9_messageDigest          1 2 840 113549 1 9 4 */
&(nid_objs[52]),/* OBJ_pkcs9_signingTime            1 2 840 113549 1 9 5 */
&(nid_objs[53]),/* OBJ_pkcs9_countersignature       1 2 840 113549 1 9 6 */
&(nid_objs[54]),/* OBJ_pkcs9_challengePassword      1 2 840 113549 1 9 7 */
&(nid_objs[55]),/* OBJ_pkcs9_unstructuredAddress    1 2 840 113549 1 9 8 */
&(nid_objs[56]),/* OBJ_pkcs9_extCertAttributes      1 2 840 113549 1 9 9 */
&(nid_objs[172]),/* OBJ_ext_req                      1 2 840 113549 1 9 14 */
&(nid_objs[167]),/* OBJ_SMIMECapabilities            1 2 840 113549 1 9 15 */
&(nid_objs[188]),/* OBJ_SMIME                        1 2 840 113549 1 9 16 */
&(nid_objs[156]),/* OBJ_friendlyName                 1 2 840 113549 1 9 20 */
&(nid_objs[157]),/* OBJ_localKeyID                   1 2 840 113549 1 9 21 */
&(nid_objs[681]),/* OBJ_X9_62_onBasis                1 2 840 10045 1 2 3 1 */
&(nid_objs[682]),/* OBJ_X9_62_tpBasis                1 2 840 10045 1 2 3 2 */
&(nid_objs[683]),/* OBJ_X9_62_ppBasis                1 2 840 10045 1 2 3 3 */
&(nid_objs[417]),/* OBJ_ms_csp_name                  1 3 6 1 4 1 311 17 1 */
&(nid_objs[390]),/* OBJ_dcObject                     1 3 6 1 4 1 1466 344 */
&(nid_objs[91]),/* OBJ_bf_cbc                       1 3 6 1 4 1 3029 1 2 */
&(nid_objs[315]),/* OBJ_id_regCtrl_regToken          1 3 6 1 5 5 7 5 1 1 */
&(nid_objs[316]),/* OBJ_id_regCtrl_authenticator     1 3 6 1 5 5 7 5 1 2 */
&(nid_objs[317]),/* OBJ_id_regCtrl_pkiPublicationInfo 1 3 6 1 5 5 7 5 1 3 */
&(nid_objs[318]),/* OBJ_id_regCtrl_pkiArchiveOptions 1 3 6 1 5 5 7 5 1 4 */
&(nid_objs[319]),/* OBJ_id_regCtrl_oldCertID         1 3 6 1 5 5 7 5 1 5 */
&(nid_objs[320]),/* OBJ_id_regCtrl_protocolEncrKey   1 3 6 1 5 5 7 5 1 6 */
&(nid_objs[321]),/* OBJ_id_regInfo_utf8Pairs         1 3 6 1 5 5 7 5 2 1 */
&(nid_objs[322]),/* OBJ_id_regInfo_certReq           1 3 6 1 5 5 7 5 2 2 */
&(nid_objs[365]),/* OBJ_id_pkix_OCSP_basic           1 3 6 1 5 5 7 48 1 1 */
&(nid_objs[366]),/* OBJ_id_pkix_OCSP_Nonce           1 3 6 1 5 5 7 48 1 2 */
&(nid_objs[367]),/* OBJ_id_pkix_OCSP_CrlID           1 3 6 1 5 5 7 48 1 3 */
&(nid_objs[368]),/* OBJ_id_pkix_OCSP_acceptableResponses 1 3 6 1 5 5 7 48 1 4 */
&(nid_objs[369]),/* OBJ_id_pkix_OCSP_noCheck         1 3 6 1 5 5 7 48 1 5 */
&(nid_objs[370]),/* OBJ_id_pkix_OCSP_archiveCutoff   1 3 6 1 5 5 7 48 1 6 */
&(nid_objs[371]),/* OBJ_id_pkix_OCSP_serviceLocator  1 3 6 1 5 5 7 48 1 7 */
&(nid_objs[372]),/* OBJ_id_pkix_OCSP_extendedStatus  1 3 6 1 5 5 7 48 1 8 */
&(nid_objs[373]),/* OBJ_id_pkix_OCSP_valid           1 3 6 1 5 5 7 48 1 9 */
&(nid_objs[374]),/* OBJ_id_pkix_OCSP_path            1 3 6 1 5 5 7 48 1 10 */
&(nid_objs[375]),/* OBJ_id_pkix_OCSP_trustRoot       1 3 6 1 5 5 7 48 1 11 */
&(nid_objs[418]),/* OBJ_aes_128_ecb                  2 16 840 1 101 3 4 1 1 */
&(nid_objs[419]),/* OBJ_aes_128_cbc                  2 16 840 1 101 3 4 1 2 */
&(nid_objs[420]),/* OBJ_aes_128_ofb128               2 16 840 1 101 3 4 1 3 */
&(nid_objs[421]),/* OBJ_aes_128_cfb128               2 16 840 1 101 3 4 1 4 */
&(nid_objs[422]),/* OBJ_aes_192_ecb                  2 16 840 1 101 3 4 1 21 */
&(nid_objs[423]),/* OBJ_aes_192_cbc                  2 16 840 1 101 3 4 1 22 */
&(nid_objs[424]),/* OBJ_aes_192_ofb128               2 16 840 1 101 3 4 1 23 */
&(nid_objs[425]),/* OBJ_aes_192_cfb128               2 16 840 1 101 3 4 1 24 */
&(nid_objs[426]),/* OBJ_aes_256_ecb                  2 16 840 1 101 3 4 1 41 */
&(nid_objs[427]),/* OBJ_aes_256_cbc                  2 16 840 1 101 3 4 1 42 */
&(nid_objs[428]),/* OBJ_aes_256_ofb128               2 16 840 1 101 3 4 1 43 */
&(nid_objs[429]),/* OBJ_aes_256_cfb128               2 16 840 1 101 3 4 1 44 */
&(nid_objs[672]),/* OBJ_sha256                       2 16 840 1 101 3 4 2 1 */
&(nid_objs[673]),/* OBJ_sha384                       2 16 840 1 101 3 4 2 2 */
&(nid_objs[674]),/* OBJ_sha512                       2 16 840 1 101 3 4 2 3 */
&(nid_objs[675]),/* OBJ_sha224                       2 16 840 1 101 3 4 2 4 */
&(nid_objs[71]),/* OBJ_netscape_cert_type           2 16 840 1 113730 1 1 */
&(nid_objs[72]),/* OBJ_netscape_base_url            2 16 840 1 113730 1 2 */
&(nid_objs[73]),/* OBJ_netscape_revocation_url      2 16 840 1 113730 1 3 */
&(nid_objs[74]),/* OBJ_netscape_ca_revocation_url   2 16 840 1 113730 1 4 */
&(nid_objs[75]),/* OBJ_netscape_renewal_url         2 16 840 1 113730 1 7 */
&(nid_objs[76]),/* OBJ_netscape_ca_policy_url       2 16 840 1 113730 1 8 */
&(nid_objs[77]),/* OBJ_netscape_ssl_server_name     2 16 840 1 113730 1 12 */
&(nid_objs[78]),/* OBJ_netscape_comment             2 16 840 1 113730 1 13 */
&(nid_objs[79]),/* OBJ_netscape_cert_sequence       2 16 840 1 113730 2 5 */
&(nid_objs[139]),/* OBJ_ns_sgc                       2 16 840 1 113730 4 1 */
&(nid_objs[458]),/* OBJ_userId                       0 9 2342 19200300 100 1 1 */
&(nid_objs[459]),/* OBJ_textEncodedORAddress         0 9 2342 19200300 100 1 2 */
&(nid_objs[460]),/* OBJ_rfc822Mailbox                0 9 2342 19200300 100 1 3 */
&(nid_objs[461]),/* OBJ_info                         0 9 2342 19200300 100 1 4 */
&(nid_objs[462]),/* OBJ_favouriteDrink               0 9 2342 19200300 100 1 5 */
&(nid_objs[463]),/* OBJ_roomNumber                   0 9 2342 19200300 100 1 6 */
&(nid_objs[464]),/* OBJ_photo                        0 9 2342 19200300 100 1 7 */
&(nid_objs[465]),/* OBJ_userClass                    0 9 2342 19200300 100 1 8 */
&(nid_objs[466]),/* OBJ_host                         0 9 2342 19200300 100 1 9 */
&(nid_objs[467]),/* OBJ_manager                      0 9 2342 19200300 100 1 10 */
&(nid_objs[468]),/* OBJ_documentIdentifier           0 9 2342 19200300 100 1 11 */
&(nid_objs[469]),/* OBJ_documentTitle                0 9 2342 19200300 100 1 12 */
&(nid_objs[470]),/* OBJ_documentVersion              0 9 2342 19200300 100 1 13 */
&(nid_objs[471]),/* OBJ_documentAuthor               0 9 2342 19200300 100 1 14 */
&(nid_objs[472]),/* OBJ_documentLocation             0 9 2342 19200300 100 1 15 */
&(nid_objs[473]),/* OBJ_homeTelephoneNumber          0 9 2342 19200300 100 1 20 */
&(nid_objs[474]),/* OBJ_secretary                    0 9 2342 19200300 100 1 21 */
&(nid_objs[475]),/* OBJ_otherMailbox                 0 9 2342 19200300 100 1 22 */
&(nid_objs[476]),/* OBJ_lastModifiedTime             0 9 2342 19200300 100 1 23 */
&(nid_objs[477]),/* OBJ_lastModifiedBy               0 9 2342 19200300 100 1 24 */
&(nid_objs[391]),/* OBJ_domainComponent              0 9 2342 19200300 100 1 25 */
&(nid_objs[478]),/* OBJ_aRecord                      0 9 2342 19200300 100 1 26 */
&(nid_objs[479]),/* OBJ_pilotAttributeType27         0 9 2342 19200300 100 1 27 */
&(nid_objs[480]),/* OBJ_mXRecord                     0 9 2342 19200300 100 1 28 */
&(nid_objs[481]),/* OBJ_nSRecord                     0 9 2342 19200300 100 1 29 */
&(nid_objs[482]),/* OBJ_sOARecord                    0 9 2342 19200300 100 1 30 */
&(nid_objs[483]),/* OBJ_cNAMERecord                  0 9 2342 19200300 100 1 31 */
&(nid_objs[484]),/* OBJ_associatedDomain             0 9 2342 19200300 100 1 37 */
&(nid_objs[485]),/* OBJ_associatedName               0 9 2342 19200300 100 1 38 */
&(nid_objs[486]),/* OBJ_homePostalAddress            0 9 2342 19200300 100 1 39 */
&(nid_objs[487]),/* OBJ_personalTitle                0 9 2342 19200300 100 1 40 */
&(nid_objs[488]),/* OBJ_mobileTelephoneNumber        0 9 2342 19200300 100 1 41 */
&(nid_objs[489]),/* OBJ_pagerTelephoneNumber         0 9 2342 19200300 100 1 42 */
&(nid_objs[490]),/* OBJ_friendlyCountryName          0 9 2342 19200300 100 1 43 */
&(nid_objs[491]),/* OBJ_organizationalStatus         0 9 2342 19200300 100 1 45 */
&(nid_objs[492]),/* OBJ_janetMailbox                 0 9 2342 19200300 100 1 46 */
&(nid_objs[493]),/* OBJ_mailPreferenceOption         0 9 2342 19200300 100 1 47 */
&(nid_objs[494]),/* OBJ_buildingName                 0 9 2342 19200300 100 1 48 */
&(nid_objs[495]),/* OBJ_dSAQuality                   0 9 2342 19200300 100 1 49 */
&(nid_objs[496]),/* OBJ_singleLevelQuality           0 9 2342 19200300 100 1 50 */
&(nid_objs[497]),/* OBJ_subtreeMinimumQuality        0 9 2342 19200300 100 1 51 */
&(nid_objs[498]),/* OBJ_subtreeMaximumQuality        0 9 2342 19200300 100 1 52 */
&(nid_objs[499]),/* OBJ_personalSignature            0 9 2342 19200300 100 1 53 */
&(nid_objs[500]),/* OBJ_dITRedirect                  0 9 2342 19200300 100 1 54 */
&(nid_objs[501]),/* OBJ_audio                        0 9 2342 19200300 100 1 55 */
&(nid_objs[502]),/* OBJ_documentPublisher            0 9 2342 19200300 100 1 56 */
&(nid_objs[442]),/* OBJ_iA5StringSyntax              0 9 2342 19200300 100 3 4 */
&(nid_objs[443]),/* OBJ_caseIgnoreIA5StringSyntax    0 9 2342 19200300 100 3 5 */
&(nid_objs[444]),/* OBJ_pilotObject                  0 9 2342 19200300 100 4 3 */
&(nid_objs[445]),/* OBJ_pilotPerson                  0 9 2342 19200300 100 4 4 */
&(nid_objs[446]),/* OBJ_account                      0 9 2342 19200300 100 4 5 */
&(nid_objs[447]),/* OBJ_document                     0 9 2342 19200300 100 4 6 */
&(nid_objs[448]),/* OBJ_room                         0 9 2342 19200300 100 4 7 */
&(nid_objs[449]),/* OBJ_documentSeries               0 9 2342 19200300 100 4 9 */
&(nid_objs[392]),/* OBJ_Domain                       0 9 2342 19200300 100 4 13 */
&(nid_objs[450]),/* OBJ_rFC822localPart              0 9 2342 19200300 100 4 14 */
&(nid_objs[451]),/* OBJ_dNSDomain                    0 9 2342 19200300 100 4 15 */
&(nid_objs[452]),/* OBJ_domainRelatedObject          0 9 2342 19200300 100 4 17 */
&(nid_objs[453]),/* OBJ_friendlyCountry              0 9 2342 19200300 100 4 18 */
&(nid_objs[454]),/* OBJ_simpleSecurityObject         0 9 2342 19200300 100 4 19 */
&(nid_objs[455]),/* OBJ_pilotOrganization            0 9 2342 19200300 100 4 20 */
&(nid_objs[456]),/* OBJ_pilotDSA                     0 9 2342 19200300 100 4 21 */
&(nid_objs[457]),/* OBJ_qualityLabelledData          0 9 2342 19200300 100 4 22 */
&(nid_objs[189]),/* OBJ_id_smime_mod                 1 2 840 113549 1 9 16 0 */
&(nid_objs[190]),/* OBJ_id_smime_ct                  1 2 840 113549 1 9 16 1 */
&(nid_objs[191]),/* OBJ_id_smime_aa                  1 2 840 113549 1 9 16 2 */
&(nid_objs[192]),/* OBJ_id_smime_alg                 1 2 840 113549 1 9 16 3 */
&(nid_objs[193]),/* OBJ_id_smime_cd                  1 2 840 113549 1 9 16 4 */
&(nid_objs[194]),/* OBJ_id_smime_spq                 1 2 840 113549 1 9 16 5 */
&(nid_objs[195]),/* OBJ_id_smime_cti                 1 2 840 113549 1 9 16 6 */
&(nid_objs[158]),/* OBJ_x509Certificate              1 2 840 113549 1 9 22 1 */
&(nid_objs[159]),/* OBJ_sdsiCertificate              1 2 840 113549 1 9 22 2 */
&(nid_objs[160]),/* OBJ_x509Crl                      1 2 840 113549 1 9 23 1 */
&(nid_objs[144]),/* OBJ_pbe_WithSHA1And128BitRC4     1 2 840 113549 1 12 1 1 */
&(nid_objs[145]),/* OBJ_pbe_WithSHA1And40BitRC4      1 2 840 113549 1 12 1 2 */
&(nid_objs[146]),/* OBJ_pbe_WithSHA1And3_Key_TripleDES_CBC 1 2 840 113549 1 12 1 3 */
&(nid_objs[147]),/* OBJ_pbe_WithSHA1And2_Key_TripleDES_CBC 1 2 840 113549 1 12 1 4 */
&(nid_objs[148]),/* OBJ_pbe_WithSHA1And128BitRC2_CBC 1 2 840 113549 1 12 1 5 */
&(nid_objs[149]),/* OBJ_pbe_WithSHA1And40BitRC2_CBC  1 2 840 113549 1 12 1 6 */
&(nid_objs[171]),/* OBJ_ms_ext_req                   1 3 6 1 4 1 311 2 1 14 */
&(nid_objs[134]),/* OBJ_ms_code_ind                  1 3 6 1 4 1 311 2 1 21 */
&(nid_objs[135]),/* OBJ_ms_code_com                  1 3 6 1 4 1 311 2 1 22 */
&(nid_objs[136]),/* OBJ_ms_ctl_sign                  1 3 6 1 4 1 311 10 3 1 */
&(nid_objs[137]),/* OBJ_ms_sgc                       1 3 6 1 4 1 311 10 3 3 */
&(nid_objs[138]),/* OBJ_ms_efs                       1 3 6 1 4 1 311 10 3 4 */
&(nid_objs[648]),/* OBJ_ms_smartcard_login           1 3 6 1 4 1 311 20 2 2 */
&(nid_objs[649]),/* OBJ_ms_upn                       1 3 6 1 4 1 311 20 2 3 */
&(nid_objs[751]),/* OBJ_camellia_128_cbc             1 2 392 200011 61 1 1 1 2 */
&(nid_objs[752]),/* OBJ_camellia_192_cbc             1 2 392 200011 61 1 1 1 3 */
&(nid_objs[753]),/* OBJ_camellia_256_cbc             1 2 392 200011 61 1 1 1 4 */
&(nid_objs[196]),/* OBJ_id_smime_mod_cms             1 2 840 113549 1 9 16 0 1 */
&(nid_objs[197]),/* OBJ_id_smime_mod_ess             1 2 840 113549 1 9 16 0 2 */
&(nid_objs[198]),/* OBJ_id_smime_mod_oid             1 2 840 113549 1 9 16 0 3 */
&(nid_objs[199]),/* OBJ_id_smime_mod_msg_v3          1 2 840 113549 1 9 16 0 4 */
&(nid_objs[200]),/* OBJ_id_smime_mod_ets_eSignature_88 1 2 840 113549 1 9 16 0 5 */
&(nid_objs[201]),/* OBJ_id_smime_mod_ets_eSignature_97 1 2 840 113549 1 9 16 0 6 */
&(nid_objs[202]),/* OBJ_id_smime_mod_ets_eSigPolicy_88 1 2 840 113549 1 9 16 0 7 */
&(nid_objs[203]),/* OBJ_id_smime_mod_ets_eSigPolicy_97 1 2 840 113549 1 9 16 0 8 */
&(nid_objs[204]),/* OBJ_id_smime_ct_receipt          1 2 840 113549 1 9 16 1 1 */
&(nid_objs[205]),/* OBJ_id_smime_ct_authData         1 2 840 113549 1 9 16 1 2 */
&(nid_objs[206]),/* OBJ_id_smime_ct_publishCert      1 2 840 113549 1 9 16 1 3 */
&(nid_objs[207]),/* OBJ_id_smime_ct_TSTInfo          1 2 840 113549 1 9 16 1 4 */
&(nid_objs[208]),/* OBJ_id_smime_ct_TDTInfo          1 2 840 113549 1 9 16 1 5 */
&(nid_objs[209]),/* OBJ_id_smime_ct_contentInfo      1 2 840 113549 1 9 16 1 6 */
&(nid_objs[210]),/* OBJ_id_smime_ct_DVCSRequestData  1 2 840 113549 1 9 16 1 7 */
&(nid_objs[211]),/* OBJ_id_smime_ct_DVCSResponseData 1 2 840 113549 1 9 16 1 8 */
&(nid_objs[212]),/* OBJ_id_smime_aa_receiptRequest   1 2 840 113549 1 9 16 2 1 */
&(nid_objs[213]),/* OBJ_id_smime_aa_securityLabel    1 2 840 113549 1 9 16 2 2 */
&(nid_objs[214]),/* OBJ_id_smime_aa_mlExpandHistory  1 2 840 113549 1 9 16 2 3 */
&(nid_objs[215]),/* OBJ_id_smime_aa_contentHint      1 2 840 113549 1 9 16 2 4 */
&(nid_objs[216]),/* OBJ_id_smime_aa_msgSigDigest     1 2 840 113549 1 9 16 2 5 */
&(nid_objs[217]),/* OBJ_id_smime_aa_encapContentType 1 2 840 113549 1 9 16 2 6 */
&(nid_objs[218]),/* OBJ_id_smime_aa_contentIdentifier 1 2 840 113549 1 9 16 2 7 */
&(nid_objs[219]),/* OBJ_id_smime_aa_macValue         1 2 840 113549 1 9 16 2 8 */
&(nid_objs[220]),/* OBJ_id_smime_aa_equivalentLabels 1 2 840 113549 1 9 16 2 9 */
&(nid_objs[221]),/* OBJ_id_smime_aa_contentReference 1 2 840 113549 1 9 16 2 10 */
&(nid_objs[222]),/* OBJ_id_smime_aa_encrypKeyPref    1 2 840 113549 1 9 16 2 11 */
&(nid_objs[223]),/* OBJ_id_smime_aa_signingCertificate 1 2 840 113549 1 9 16 2 12 */
&(nid_objs[224]),/* OBJ_id_smime_aa_smimeEncryptCerts 1 2 840 113549 1 9 16 2 13 */
&(nid_objs[225]),/* OBJ_id_smime_aa_timeStampToken   1 2 840 113549 1 9 16 2 14 */
&(nid_objs[226]),/* OBJ_id_smime_aa_ets_sigPolicyId  1 2 840 113549 1 9 16 2 15 */
&(nid_objs[227]),/* OBJ_id_smime_aa_ets_commitmentType 1 2 840 113549 1 9 16 2 16 */
&(nid_objs[228]),/* OBJ_id_smime_aa_ets_signerLocation 1 2 840 113549 1 9 16 2 17 */
&(nid_objs[229]),/* OBJ_id_smime_aa_ets_signerAttr   1 2 840 113549 1 9 16 2 18 */
&(nid_objs[230]),/* OBJ_id_smime_aa_ets_otherSigCert 1 2 840 113549 1 9 16 2 19 */
&(nid_objs[231]),/* OBJ_id_smime_aa_ets_contentTimestamp 1 2 840 113549 1 9 16 2 20 */
&(nid_objs[232]),/* OBJ_id_smime_aa_ets_CertificateRefs 1 2 840 113549 1 9 16 2 21 */
&(nid_objs[233]),/* OBJ_id_smime_aa_ets_RevocationRefs 1 2 840 113549 1 9 16 2 22 */
&(nid_objs[234]),/* OBJ_id_smime_aa_ets_certValues   1 2 840 113549 1 9 16 2 23 */
&(nid_objs[235]),/* OBJ_id_smime_aa_ets_revocationValues 1 2 840 113549 1 9 16 2 24 */
&(nid_objs[236]),/* OBJ_id_smime_aa_ets_escTimeStamp 1 2 840 113549 1 9 16 2 25 */
&(nid_objs[237]),/* OBJ_id_smime_aa_ets_certCRLTimestamp 1 2 840 113549 1 9 16 2 26 */
&(nid_objs[238]),/* OBJ_id_smime_aa_ets_archiveTimeStamp 1 2 840 113549 1 9 16 2 27 */
&(nid_objs[239]),/* OBJ_id_smime_aa_signatureType    1 2 840 113549 1 9 16 2 28 */
&(nid_objs[240]),/* OBJ_id_smime_aa_dvcs_dvc         1 2 840 113549 1 9 16 2 29 */
&(nid_objs[241]),/* OBJ_id_smime_alg_ESDHwith3DES    1 2 840 113549 1 9 16 3 1 */
&(nid_objs[242]),/* OBJ_id_smime_alg_ESDHwithRC2     1 2 840 113549 1 9 16 3 2 */
&(nid_objs[243]),/* OBJ_id_smime_alg_3DESwrap        1 2 840 113549 1 9 16 3 3 */
&(nid_objs[244]),/* OBJ_id_smime_alg_RC2wrap         1 2 840 113549 1 9 16 3 4 */
&(nid_objs[245]),/* OBJ_id_smime_alg_ESDH            1 2 840 113549 1 9 16 3 5 */
&(nid_objs[246]),/* OBJ_id_smime_alg_CMS3DESwrap     1 2 840 113549 1 9 16 3 6 */
&(nid_objs[247]),/* OBJ_id_smime_alg_CMSRC2wrap      1 2 840 113549 1 9 16 3 7 */
&(nid_objs[248]),/* OBJ_id_smime_cd_ldap             1 2 840 113549 1 9 16 4 1 */
&(nid_objs[249]),/* OBJ_id_smime_spq_ets_sqt_uri     1 2 840 113549 1 9 16 5 1 */
&(nid_objs[250]),/* OBJ_id_smime_spq_ets_sqt_unotice 1 2 840 113549 1 9 16 5 2 */
&(nid_objs[251]),/* OBJ_id_smime_cti_ets_proofOfOrigin 1 2 840 113549 1 9 16 6 1 */
&(nid_objs[252]),/* OBJ_id_smime_cti_ets_proofOfReceipt 1 2 840 113549 1 9 16 6 2 */
&(nid_objs[253]),/* OBJ_id_smime_cti_ets_proofOfDelivery 1 2 840 113549 1 9 16 6 3 */
&(nid_objs[254]),/* OBJ_id_smime_cti_ets_proofOfSender 1 2 840 113549 1 9 16 6 4 */
&(nid_objs[255]),/* OBJ_id_smime_cti_ets_proofOfApproval 1 2 840 113549 1 9 16 6 5 */
&(nid_objs[256]),/* OBJ_id_smime_cti_ets_proofOfCreation 1 2 840 113549 1 9 16 6 6 */
&(nid_objs[150]),/* OBJ_keyBag                       1 2 840 113549 1 12 10 1 1 */
&(nid_objs[151]),/* OBJ_pkcs8ShroudedKeyBag          1 2 840 113549 1 12 10 1 2 */
&(nid_objs[152]),/* OBJ_certBag                      1 2 840 113549 1 12 10 1 3 */
&(nid_objs[153]),/* OBJ_crlBag                       1 2 840 113549 1 12 10 1 4 */
&(nid_objs[154]),/* OBJ_secretBag                    1 2 840 113549 1 12 10 1 5 */
&(nid_objs[155]),/* OBJ_safeContentsBag              1 2 840 113549 1 12 10 1 6 */
&(nid_objs[34]),/* OBJ_idea_cbc                     1 3 6 1 4 1 188 7 1 1 2 */
};

