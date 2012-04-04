#
# OpenSSL/crypto/evp/Makefile
#

DIR=	evp
TOP=	../..
CC=	cc
INCLUDES= -I.. -I$(TOP) -I../../include
CFLAG=-g
MAKEFILE=	Makefile
AR=		ar r

CFLAGS= $(INCLUDES) $(CFLAG)

GENERAL=Makefile
TEST=evp_test.c
TESTDATA=evptests.txt
APPS=

LIB=$(BWLIBDIR)/libcrypto.a
LIBSRC= encode.c digest.c evp_enc.c evp_key.c evp_acnf.c \
	e_des.c e_bf.c e_idea.c e_des3.c e_camellia.c\
	e_rc4.c e_aes.c names.c e_seed.c \
	e_xcbc_d.c e_rc2.c e_cast.c e_rc5.c \
	m_null.c m_md2.c m_md4.c m_md5.c m_sha.c m_sha1.c \
	m_dss.c m_dss1.c m_mdc2.c m_ripemd.c m_ecdsa.c\
	p_open.c p_seal.c p_sign.c p_verify.c p_lib.c p_enc.c p_dec.c \
	bio_md.c bio_b64.c bio_enc.c evp_err.c e_null.c \
	c_all.c c_allc.c c_alld.c evp_lib.c bio_ok.c \
	evp_pkey.c evp_pbe.c p5_crpt.c p5_crpt2.c \
	e_old.c

LIBOBJ=	$(MF_CONFIG)/encode.o $(MF_CONFIG)/digest.o $(MF_CONFIG)/evp_enc.o \
	$(MF_CONFIG)/evp_key.o $(MF_CONFIG)/evp_acnf.o $(MF_CONFIG)/e_des.o \
	$(MF_CONFIG)/e_bf.o $(MF_CONFIG)/e_idea.o $(MF_CONFIG)/e_des3.o \
	$(MF_CONFIG)/e_camellia.o $(MF_CONFIG)/e_rc4.o $(MF_CONFIG)/e_aes.o \
	$(MF_CONFIG)/names.o $(MF_CONFIG)/e_seed.o $(MF_CONFIG)/e_xcbc_d.o \
	$(MF_CONFIG)/e_rc2.o $(MF_CONFIG)/e_cast.o $(MF_CONFIG)/e_rc5.o \
	$(MF_CONFIG)/m_null.o $(MF_CONFIG)/m_md2.o $(MF_CONFIG)/m_md4.o \
	$(MF_CONFIG)/m_md5.o $(MF_CONFIG)/m_sha.o $(MF_CONFIG)/m_sha1.o \
	$(MF_CONFIG)/m_dss.o $(MF_CONFIG)/m_dss1.o $(MF_CONFIG)/m_mdc2.o \
	$(MF_CONFIG)/m_ripemd.o $(MF_CONFIG)/m_ecdsa.o $(MF_CONFIG)/p_open.o \
	$(MF_CONFIG)/p_seal.o $(MF_CONFIG)/p_sign.o $(MF_CONFIG)/p_verify.o \
	$(MF_CONFIG)/p_lib.o $(MF_CONFIG)/p_enc.o $(MF_CONFIG)/p_dec.o \
	$(MF_CONFIG)/bio_md.o $(MF_CONFIG)/bio_b64.o $(MF_CONFIG)/bio_enc.o \
	$(MF_CONFIG)/evp_err.o $(MF_CONFIG)/e_null.o $(MF_CONFIG)/c_all.o \
	$(MF_CONFIG)/c_allc.o $(MF_CONFIG)/c_alld.o $(MF_CONFIG)/evp_lib.o \
	$(MF_CONFIG)/bio_ok.o $(MF_CONFIG)/evp_pkey.o $(MF_CONFIG)/evp_pbe.o \
	$(MF_CONFIG)/p5_crpt.o $(MF_CONFIG)/p5_crpt2.o $(MF_CONFIG)/e_old.o

SRC= $(LIBSRC)

EXHEADER= evp.h
HEADER=	$(EXHEADER)

ALL=    $(GENERAL) $(SRC) $(HEADER)

top:
	(cd ../..; $(MAKE) DIRS=crypto SDIRS=$(DIR) sub_all)

all:	lib

$(MF_CONFIG):
	@-mkdir $(MF_CONFIG)

lib:	$(MF_CONFIG) $(LIBOBJ)
	$(AR) $(LIB) $(LIBOBJ)
	$(RANLIB) $(LIB) || echo Never mind.
	@touch lib

$(MF_CONFIG)/%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

files:
	$(PERL) $(TOP)/util/files.pl Makefile >> $(TOP)/MINFO

links:
	@$(PERL) $(TOP)/util/mklink.pl ../../include/openssl $(EXHEADER)
	@$(PERL) $(TOP)/util/mklink.pl ../../test $(TEST)
	cp $(TESTDATA) ../../test
	@$(PERL) $(TOP)/util/mklink.pl ../../apps $(APPS)

install:
	@[ -n "$(INSTALLTOP)" ] # should be set by top Makefile...
	@headerlist="$(EXHEADER)"; for i in $$headerlist ; \
	do  \
	(cp $$i $(INSTALL_PREFIX)$(INSTALLTOP)/include/openssl/$$i; \
	chmod 644 $(INSTALL_PREFIX)$(INSTALLTOP)/include/openssl/$$i ); \
	done;

tags:
	ctags $(SRC)

tests:

lint:
	lint -DLINT $(INCLUDES) $(SRC)>fluff

depend:
	@[ -n "$(MAKEDEPEND)" ] # should be set by upper Makefile...
	$(MAKEDEPEND) -- $(CFLAG) $(INCLUDES) $(DEPFLAG) -- $(LIBSRC)

dclean:
	$(PERL) -pe 'if (/^# DO NOT DELETE THIS LINE/) {print; exit(0);}' $(MAKEFILE) >Makefile.new
	mv -f Makefile.new $(MAKEFILE)

clean:
	rm -f $(MF_CONFIG)/*.o *.o *.obj lib tags core .pure .nfs* *.old *.bak fluff

# DO NOT DELETE THIS LINE -- make depend depends on it.

$(MF_CONFIG)/bio_b64.o: ../../e_os.h ../../include/openssl/asn1.h
$(MF_CONFIG)/bio_b64.o: ../../include/openssl/bio.h ../../include/openssl/buffer.h
$(MF_CONFIG)/bio_b64.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(MF_CONFIG)/bio_b64.o: ../../include/openssl/err.h ../../include/openssl/evp.h
$(MF_CONFIG)/bio_b64.o: ../../include/openssl/lhash.h ../../include/openssl/obj_mac.h
$(MF_CONFIG)/bio_b64.o: ../../include/openssl/objects.h ../../include/openssl/opensslconf.h
$(MF_CONFIG)/bio_b64.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(MF_CONFIG)/bio_b64.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(MF_CONFIG)/bio_b64.o: ../../include/openssl/symhacks.h ../cryptlib.h bio_b64.c
$(MF_CONFIG)/bio_enc.o: ../../e_os.h ../../include/openssl/asn1.h
$(MF_CONFIG)/bio_enc.o: ../../include/openssl/bio.h ../../include/openssl/buffer.h
$(MF_CONFIG)/bio_enc.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(MF_CONFIG)/bio_enc.o: ../../include/openssl/err.h ../../include/openssl/evp.h
$(MF_CONFIG)/bio_enc.o: ../../include/openssl/lhash.h ../../include/openssl/obj_mac.h
$(MF_CONFIG)/bio_enc.o: ../../include/openssl/objects.h ../../include/openssl/opensslconf.h
$(MF_CONFIG)/bio_enc.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(MF_CONFIG)/bio_enc.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(MF_CONFIG)/bio_enc.o: ../../include/openssl/symhacks.h ../cryptlib.h bio_enc.c
$(MF_CONFIG)/bio_md.o: ../../e_os.h ../../include/openssl/asn1.h ../../include/openssl/bio.h
$(MF_CONFIG)/bio_md.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(MF_CONFIG)/bio_md.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(MF_CONFIG)/bio_md.o: ../../include/openssl/evp.h ../../include/openssl/lhash.h
$(MF_CONFIG)/bio_md.o: ../../include/openssl/obj_mac.h ../../include/openssl/objects.h
$(MF_CONFIG)/bio_md.o: ../../include/openssl/opensslconf.h ../../include/openssl/opensslv.h
$(MF_CONFIG)/bio_md.o: ../../include/openssl/ossl_typ.h ../../include/openssl/safestack.h
$(MF_CONFIG)/bio_md.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
$(MF_CONFIG)/bio_md.o: ../cryptlib.h bio_md.c
$(MF_CONFIG)/bio_ok.o: ../../e_os.h ../../include/openssl/asn1.h ../../include/openssl/bio.h
$(MF_CONFIG)/bio_ok.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(MF_CONFIG)/bio_ok.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(MF_CONFIG)/bio_ok.o: ../../include/openssl/evp.h ../../include/openssl/lhash.h
$(MF_CONFIG)/bio_ok.o: ../../include/openssl/obj_mac.h ../../include/openssl/objects.h
$(MF_CONFIG)/bio_ok.o: ../../include/openssl/opensslconf.h ../../include/openssl/opensslv.h
$(MF_CONFIG)/bio_ok.o: ../../include/openssl/ossl_typ.h ../../include/openssl/rand.h
$(MF_CONFIG)/bio_ok.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(MF_CONFIG)/bio_ok.o: ../../include/openssl/symhacks.h ../cryptlib.h bio_ok.c
$(MF_CONFIG)/c_all.o: ../../e_os.h ../../include/openssl/asn1.h ../../include/openssl/bio.h
$(MF_CONFIG)/c_all.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(MF_CONFIG)/c_all.o: ../../include/openssl/e_os2.h ../../include/openssl/engine.h
$(MF_CONFIG)/c_all.o: ../../include/openssl/err.h ../../include/openssl/evp.h
$(MF_CONFIG)/c_all.o: ../../include/openssl/lhash.h ../../include/openssl/obj_mac.h
$(MF_CONFIG)/c_all.o: ../../include/openssl/objects.h ../../include/openssl/opensslconf.h
$(MF_CONFIG)/c_all.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(MF_CONFIG)/c_all.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(MF_CONFIG)/c_all.o: ../../include/openssl/symhacks.h ../cryptlib.h c_all.c
$(MF_CONFIG)/c_allc.o: ../../e_os.h ../../include/openssl/asn1.h ../../include/openssl/bio.h
$(MF_CONFIG)/c_allc.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(MF_CONFIG)/c_allc.o: ../../include/openssl/e_os2.h ../../include/openssl/ec.h
$(MF_CONFIG)/c_allc.o: ../../include/openssl/ecdh.h ../../include/openssl/ecdsa.h
$(MF_CONFIG)/c_allc.o: ../../include/openssl/err.h ../../include/openssl/evp.h
$(MF_CONFIG)/c_allc.o: ../../include/openssl/lhash.h ../../include/openssl/obj_mac.h
$(MF_CONFIG)/c_allc.o: ../../include/openssl/objects.h ../../include/openssl/opensslconf.h
$(MF_CONFIG)/c_allc.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(MF_CONFIG)/c_allc.o: ../../include/openssl/pkcs12.h ../../include/openssl/pkcs7.h
$(MF_CONFIG)/c_allc.o: ../../include/openssl/safestack.h ../../include/openssl/sha.h
$(MF_CONFIG)/c_allc.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
$(MF_CONFIG)/c_allc.o: ../../include/openssl/x509.h ../../include/openssl/x509_vfy.h
$(MF_CONFIG)/c_allc.o: ../cryptlib.h c_allc.c
$(MF_CONFIG)/c_alld.o: ../../e_os.h ../../include/openssl/asn1.h ../../include/openssl/bio.h
$(MF_CONFIG)/c_alld.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(MF_CONFIG)/c_alld.o: ../../include/openssl/e_os2.h ../../include/openssl/ec.h
$(MF_CONFIG)/c_alld.o: ../../include/openssl/ecdh.h ../../include/openssl/ecdsa.h
$(MF_CONFIG)/c_alld.o: ../../include/openssl/err.h ../../include/openssl/evp.h
$(MF_CONFIG)/c_alld.o: ../../include/openssl/lhash.h ../../include/openssl/obj_mac.h
$(MF_CONFIG)/c_alld.o: ../../include/openssl/objects.h ../../include/openssl/opensslconf.h
$(MF_CONFIG)/c_alld.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(MF_CONFIG)/c_alld.o: ../../include/openssl/pkcs12.h ../../include/openssl/pkcs7.h
$(MF_CONFIG)/c_alld.o: ../../include/openssl/safestack.h ../../include/openssl/sha.h
$(MF_CONFIG)/c_alld.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
$(MF_CONFIG)/c_alld.o: ../../include/openssl/x509.h ../../include/openssl/x509_vfy.h
$(MF_CONFIG)/c_alld.o: ../cryptlib.h c_alld.c
$(MF_CONFIG)/digest.o: ../../e_os.h ../../include/openssl/asn1.h ../../include/openssl/bio.h
$(MF_CONFIG)/digest.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(MF_CONFIG)/digest.o: ../../include/openssl/e_os2.h ../../include/openssl/engine.h
$(MF_CONFIG)/digest.o: ../../include/openssl/err.h ../../include/openssl/evp.h
$(MF_CONFIG)/digest.o: ../../include/openssl/lhash.h ../../include/openssl/obj_mac.h
$(MF_CONFIG)/digest.o: ../../include/openssl/objects.h ../../include/openssl/opensslconf.h
$(MF_CONFIG)/digest.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(MF_CONFIG)/digest.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(MF_CONFIG)/digest.o: ../../include/openssl/symhacks.h ../cryptlib.h digest.c
$(MF_CONFIG)/e_aes.o: ../../include/openssl/aes.h ../../include/openssl/asn1.h
$(MF_CONFIG)/e_aes.o: ../../include/openssl/bio.h ../../include/openssl/crypto.h
$(MF_CONFIG)/e_aes.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(MF_CONFIG)/e_aes.o: ../../include/openssl/evp.h ../../include/openssl/lhash.h
$(MF_CONFIG)/e_aes.o: ../../include/openssl/obj_mac.h ../../include/openssl/objects.h
$(MF_CONFIG)/e_aes.o: ../../include/openssl/opensslconf.h ../../include/openssl/opensslv.h
$(MF_CONFIG)/e_aes.o: ../../include/openssl/ossl_typ.h ../../include/openssl/safestack.h
$(MF_CONFIG)/e_aes.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h e_aes.c
$(MF_CONFIG)/e_aes.o: evp_locl.h
$(MF_CONFIG)/e_bf.o: ../../e_os.h ../../include/openssl/asn1.h ../../include/openssl/bio.h
$(MF_CONFIG)/e_bf.o: ../../include/openssl/blowfish.h ../../include/openssl/buffer.h
$(MF_CONFIG)/e_bf.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(MF_CONFIG)/e_bf.o: ../../include/openssl/err.h ../../include/openssl/evp.h
$(MF_CONFIG)/e_bf.o: ../../include/openssl/lhash.h ../../include/openssl/obj_mac.h
$(MF_CONFIG)/e_bf.o: ../../include/openssl/objects.h ../../include/openssl/opensslconf.h
$(MF_CONFIG)/e_bf.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(MF_CONFIG)/e_bf.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(MF_CONFIG)/e_bf.o: ../../include/openssl/symhacks.h ../cryptlib.h e_bf.c evp_locl.h
$(MF_CONFIG)/e_camellia.o: ../../include/openssl/opensslconf.h e_camellia.c
$(MF_CONFIG)/e_cast.o: ../../e_os.h ../../include/openssl/bio.h
$(MF_CONFIG)/e_cast.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(MF_CONFIG)/e_cast.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(MF_CONFIG)/e_cast.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(MF_CONFIG)/e_cast.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(MF_CONFIG)/e_cast.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(MF_CONFIG)/e_cast.o: ../../include/openssl/symhacks.h ../cryptlib.h e_cast.c
$(MF_CONFIG)/e_des.o: ../../e_os.h ../../include/openssl/bio.h
$(MF_CONFIG)/e_des.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(MF_CONFIG)/e_des.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(MF_CONFIG)/e_des.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(MF_CONFIG)/e_des.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(MF_CONFIG)/e_des.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(MF_CONFIG)/e_des.o: ../../include/openssl/symhacks.h ../cryptlib.h e_des.c
$(MF_CONFIG)/e_des3.o: ../../e_os.h ../../include/openssl/bio.h
$(MF_CONFIG)/e_des3.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(MF_CONFIG)/e_des3.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(MF_CONFIG)/e_des3.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(MF_CONFIG)/e_des3.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(MF_CONFIG)/e_des3.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(MF_CONFIG)/e_des3.o: ../../include/openssl/symhacks.h ../cryptlib.h e_des3.c
$(MF_CONFIG)/e_idea.o: ../../e_os.h ../../include/openssl/asn1.h ../../include/openssl/bio.h
$(MF_CONFIG)/e_idea.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(MF_CONFIG)/e_idea.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(MF_CONFIG)/e_idea.o: ../../include/openssl/evp.h ../../include/openssl/idea.h
$(MF_CONFIG)/e_idea.o: ../../include/openssl/lhash.h ../../include/openssl/obj_mac.h
$(MF_CONFIG)/e_idea.o: ../../include/openssl/objects.h ../../include/openssl/opensslconf.h
$(MF_CONFIG)/e_idea.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(MF_CONFIG)/e_idea.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(MF_CONFIG)/e_idea.o: ../../include/openssl/symhacks.h ../cryptlib.h e_idea.c evp_locl.h
$(MF_CONFIG)/e_null.o: ../../e_os.h ../../include/openssl/asn1.h ../../include/openssl/bio.h
$(MF_CONFIG)/e_null.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(MF_CONFIG)/e_null.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(MF_CONFIG)/e_null.o: ../../include/openssl/evp.h ../../include/openssl/lhash.h
$(MF_CONFIG)/e_null.o: ../../include/openssl/obj_mac.h ../../include/openssl/objects.h
$(MF_CONFIG)/e_null.o: ../../include/openssl/opensslconf.h ../../include/openssl/opensslv.h
$(MF_CONFIG)/e_null.o: ../../include/openssl/ossl_typ.h ../../include/openssl/safestack.h
$(MF_CONFIG)/e_null.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
$(MF_CONFIG)/e_null.o: ../cryptlib.h e_null.c
$(MF_CONFIG)/e_old.o: e_old.c
$(MF_CONFIG)/e_rc2.o: ../../e_os.h ../../include/openssl/bio.h
$(MF_CONFIG)/e_rc2.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(MF_CONFIG)/e_rc2.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(MF_CONFIG)/e_rc2.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(MF_CONFIG)/e_rc2.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(MF_CONFIG)/e_rc2.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(MF_CONFIG)/e_rc2.o: ../../include/openssl/symhacks.h ../cryptlib.h e_rc2.c
$(MF_CONFIG)/e_rc4.o: ../../e_os.h ../../include/openssl/bio.h
$(MF_CONFIG)/e_rc4.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(MF_CONFIG)/e_rc4.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(MF_CONFIG)/e_rc4.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(MF_CONFIG)/e_rc4.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(MF_CONFIG)/e_rc4.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(MF_CONFIG)/e_rc4.o: ../../include/openssl/symhacks.h ../cryptlib.h e_rc4.c
$(MF_CONFIG)/e_rc5.o: ../../e_os.h ../../include/openssl/bio.h
$(MF_CONFIG)/e_rc5.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(MF_CONFIG)/e_rc5.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(MF_CONFIG)/e_rc5.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(MF_CONFIG)/e_rc5.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(MF_CONFIG)/e_rc5.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(MF_CONFIG)/e_rc5.o: ../../include/openssl/symhacks.h ../cryptlib.h e_rc5.c
$(MF_CONFIG)/e_seed.o: ../../include/openssl/asn1.h ../../include/openssl/bio.h
$(MF_CONFIG)/e_seed.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(MF_CONFIG)/e_seed.o: ../../include/openssl/err.h ../../include/openssl/evp.h
$(MF_CONFIG)/e_seed.o: ../../include/openssl/lhash.h ../../include/openssl/obj_mac.h
$(MF_CONFIG)/e_seed.o: ../../include/openssl/objects.h ../../include/openssl/opensslconf.h
$(MF_CONFIG)/e_seed.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(MF_CONFIG)/e_seed.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(MF_CONFIG)/e_seed.o: ../../include/openssl/symhacks.h e_seed.c
$(MF_CONFIG)/e_xcbc_d.o: ../../e_os.h ../../include/openssl/bio.h
$(MF_CONFIG)/e_xcbc_d.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(MF_CONFIG)/e_xcbc_d.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(MF_CONFIG)/e_xcbc_d.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(MF_CONFIG)/e_xcbc_d.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(MF_CONFIG)/e_xcbc_d.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(MF_CONFIG)/e_xcbc_d.o: ../../include/openssl/symhacks.h ../cryptlib.h e_xcbc_d.c
$(MF_CONFIG)/encode.o: ../../e_os.h ../../include/openssl/asn1.h ../../include/openssl/bio.h
$(MF_CONFIG)/encode.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(MF_CONFIG)/encode.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(MF_CONFIG)/encode.o: ../../include/openssl/evp.h ../../include/openssl/lhash.h
$(MF_CONFIG)/encode.o: ../../include/openssl/obj_mac.h ../../include/openssl/objects.h
$(MF_CONFIG)/encode.o: ../../include/openssl/opensslconf.h ../../include/openssl/opensslv.h
$(MF_CONFIG)/encode.o: ../../include/openssl/ossl_typ.h ../../include/openssl/safestack.h
$(MF_CONFIG)/encode.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
$(MF_CONFIG)/encode.o: ../cryptlib.h encode.c
$(MF_CONFIG)/evp_acnf.o: ../../e_os.h ../../include/openssl/asn1.h
$(MF_CONFIG)/evp_acnf.o: ../../include/openssl/bio.h ../../include/openssl/buffer.h
$(MF_CONFIG)/evp_acnf.o: ../../include/openssl/conf.h ../../include/openssl/crypto.h
$(MF_CONFIG)/evp_acnf.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(MF_CONFIG)/evp_acnf.o: ../../include/openssl/evp.h ../../include/openssl/lhash.h
$(MF_CONFIG)/evp_acnf.o: ../../include/openssl/obj_mac.h ../../include/openssl/objects.h
$(MF_CONFIG)/evp_acnf.o: ../../include/openssl/opensslconf.h
$(MF_CONFIG)/evp_acnf.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(MF_CONFIG)/evp_acnf.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(MF_CONFIG)/evp_acnf.o: ../../include/openssl/symhacks.h ../cryptlib.h evp_acnf.c
$(MF_CONFIG)/evp_enc.o: ../../e_os.h ../../include/openssl/asn1.h
$(MF_CONFIG)/evp_enc.o: ../../include/openssl/bio.h ../../include/openssl/buffer.h
$(MF_CONFIG)/evp_enc.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(MF_CONFIG)/evp_enc.o: ../../include/openssl/engine.h ../../include/openssl/err.h
$(MF_CONFIG)/evp_enc.o: ../../include/openssl/evp.h ../../include/openssl/lhash.h
$(MF_CONFIG)/evp_enc.o: ../../include/openssl/obj_mac.h ../../include/openssl/objects.h
$(MF_CONFIG)/evp_enc.o: ../../include/openssl/opensslconf.h ../../include/openssl/opensslv.h
$(MF_CONFIG)/evp_enc.o: ../../include/openssl/ossl_typ.h ../../include/openssl/rand.h
$(MF_CONFIG)/evp_enc.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(MF_CONFIG)/evp_enc.o: ../../include/openssl/symhacks.h ../cryptlib.h evp_enc.c evp_locl.h
$(MF_CONFIG)/evp_err.o: ../../include/openssl/asn1.h ../../include/openssl/bio.h
$(MF_CONFIG)/evp_err.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(MF_CONFIG)/evp_err.o: ../../include/openssl/err.h ../../include/openssl/evp.h
$(MF_CONFIG)/evp_err.o: ../../include/openssl/lhash.h ../../include/openssl/obj_mac.h
$(MF_CONFIG)/evp_err.o: ../../include/openssl/objects.h ../../include/openssl/opensslconf.h
$(MF_CONFIG)/evp_err.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(MF_CONFIG)/evp_err.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(MF_CONFIG)/evp_err.o: ../../include/openssl/symhacks.h evp_err.c
$(MF_CONFIG)/evp_key.o: ../../e_os.h ../../include/openssl/asn1.h
$(MF_CONFIG)/evp_key.o: ../../include/openssl/bio.h ../../include/openssl/buffer.h
$(MF_CONFIG)/evp_key.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(MF_CONFIG)/evp_key.o: ../../include/openssl/ec.h ../../include/openssl/ecdh.h
$(MF_CONFIG)/evp_key.o: ../../include/openssl/ecdsa.h ../../include/openssl/err.h
$(MF_CONFIG)/evp_key.o: ../../include/openssl/evp.h ../../include/openssl/lhash.h
$(MF_CONFIG)/evp_key.o: ../../include/openssl/obj_mac.h ../../include/openssl/objects.h
$(MF_CONFIG)/evp_key.o: ../../include/openssl/opensslconf.h ../../include/openssl/opensslv.h
$(MF_CONFIG)/evp_key.o: ../../include/openssl/ossl_typ.h ../../include/openssl/pkcs7.h
$(MF_CONFIG)/evp_key.o: ../../include/openssl/safestack.h ../../include/openssl/sha.h
$(MF_CONFIG)/evp_key.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
$(MF_CONFIG)/evp_key.o: ../../include/openssl/ui.h ../../include/openssl/x509.h
$(MF_CONFIG)/evp_key.o: ../../include/openssl/x509_vfy.h ../cryptlib.h evp_key.c
$(MF_CONFIG)/evp_lib.o: ../../e_os.h ../../include/openssl/asn1.h
$(MF_CONFIG)/evp_lib.o: ../../include/openssl/bio.h ../../include/openssl/buffer.h
$(MF_CONFIG)/evp_lib.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(MF_CONFIG)/evp_lib.o: ../../include/openssl/err.h ../../include/openssl/evp.h
$(MF_CONFIG)/evp_lib.o: ../../include/openssl/lhash.h ../../include/openssl/obj_mac.h
$(MF_CONFIG)/evp_lib.o: ../../include/openssl/objects.h ../../include/openssl/opensslconf.h
$(MF_CONFIG)/evp_lib.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(MF_CONFIG)/evp_lib.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(MF_CONFIG)/evp_lib.o: ../../include/openssl/symhacks.h ../cryptlib.h evp_lib.c
$(MF_CONFIG)/evp_pbe.o: ../../e_os.h ../../include/openssl/asn1.h
$(MF_CONFIG)/evp_pbe.o: ../../include/openssl/bio.h ../../include/openssl/buffer.h
$(MF_CONFIG)/evp_pbe.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(MF_CONFIG)/evp_pbe.o: ../../include/openssl/ec.h ../../include/openssl/ecdh.h
$(MF_CONFIG)/evp_pbe.o: ../../include/openssl/ecdsa.h ../../include/openssl/err.h
$(MF_CONFIG)/evp_pbe.o: ../../include/openssl/evp.h ../../include/openssl/lhash.h
$(MF_CONFIG)/evp_pbe.o: ../../include/openssl/obj_mac.h ../../include/openssl/objects.h
$(MF_CONFIG)/evp_pbe.o: ../../include/openssl/opensslconf.h ../../include/openssl/opensslv.h
$(MF_CONFIG)/evp_pbe.o: ../../include/openssl/ossl_typ.h ../../include/openssl/pkcs7.h
$(MF_CONFIG)/evp_pbe.o: ../../include/openssl/safestack.h ../../include/openssl/sha.h
$(MF_CONFIG)/evp_pbe.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
$(MF_CONFIG)/evp_pbe.o: ../../include/openssl/x509.h ../../include/openssl/x509_vfy.h
$(MF_CONFIG)/evp_pbe.o: ../cryptlib.h evp_pbe.c
$(MF_CONFIG)/evp_pkey.o: ../../e_os.h ../../include/openssl/asn1.h
$(MF_CONFIG)/evp_pkey.o: ../../include/openssl/bio.h ../../include/openssl/bn.h
$(MF_CONFIG)/evp_pkey.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(MF_CONFIG)/evp_pkey.o: ../../include/openssl/e_os2.h ../../include/openssl/ec.h
$(MF_CONFIG)/evp_pkey.o: ../../include/openssl/ecdh.h ../../include/openssl/ecdsa.h
$(MF_CONFIG)/evp_pkey.o: ../../include/openssl/err.h ../../include/openssl/evp.h
$(MF_CONFIG)/evp_pkey.o: ../../include/openssl/lhash.h ../../include/openssl/obj_mac.h
$(MF_CONFIG)/evp_pkey.o: ../../include/openssl/objects.h ../../include/openssl/opensslconf.h
$(MF_CONFIG)/evp_pkey.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(MF_CONFIG)/evp_pkey.o: ../../include/openssl/pkcs7.h ../../include/openssl/rand.h
$(MF_CONFIG)/evp_pkey.o: ../../include/openssl/rsa.h ../../include/openssl/safestack.h
$(MF_CONFIG)/evp_pkey.o: ../../include/openssl/sha.h ../../include/openssl/stack.h
$(MF_CONFIG)/evp_pkey.o: ../../include/openssl/symhacks.h ../../include/openssl/x509.h
$(MF_CONFIG)/evp_pkey.o: ../../include/openssl/x509_vfy.h ../cryptlib.h evp_pkey.c
$(MF_CONFIG)/m_dss.o: ../../e_os.h ../../include/openssl/asn1.h ../../include/openssl/bio.h
$(MF_CONFIG)/m_dss.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(MF_CONFIG)/m_dss.o: ../../include/openssl/e_os2.h ../../include/openssl/ec.h
$(MF_CONFIG)/m_dss.o: ../../include/openssl/ecdh.h ../../include/openssl/ecdsa.h
$(MF_CONFIG)/m_dss.o: ../../include/openssl/err.h ../../include/openssl/evp.h
$(MF_CONFIG)/m_dss.o: ../../include/openssl/lhash.h ../../include/openssl/obj_mac.h
$(MF_CONFIG)/m_dss.o: ../../include/openssl/objects.h ../../include/openssl/opensslconf.h
$(MF_CONFIG)/m_dss.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(MF_CONFIG)/m_dss.o: ../../include/openssl/pkcs7.h ../../include/openssl/safestack.h
$(MF_CONFIG)/m_dss.o: ../../include/openssl/sha.h ../../include/openssl/stack.h
$(MF_CONFIG)/m_dss.o: ../../include/openssl/symhacks.h ../../include/openssl/x509.h
$(MF_CONFIG)/m_dss.o: ../../include/openssl/x509_vfy.h ../cryptlib.h m_dss.c
$(MF_CONFIG)/m_dss1.o: ../../e_os.h ../../include/openssl/asn1.h ../../include/openssl/bio.h
$(MF_CONFIG)/m_dss1.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(MF_CONFIG)/m_dss1.o: ../../include/openssl/e_os2.h ../../include/openssl/ec.h
$(MF_CONFIG)/m_dss1.o: ../../include/openssl/ecdh.h ../../include/openssl/ecdsa.h
$(MF_CONFIG)/m_dss1.o: ../../include/openssl/err.h ../../include/openssl/evp.h
$(MF_CONFIG)/m_dss1.o: ../../include/openssl/lhash.h ../../include/openssl/obj_mac.h
$(MF_CONFIG)/m_dss1.o: ../../include/openssl/objects.h ../../include/openssl/opensslconf.h
$(MF_CONFIG)/m_dss1.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(MF_CONFIG)/m_dss1.o: ../../include/openssl/pkcs7.h ../../include/openssl/safestack.h
$(MF_CONFIG)/m_dss1.o: ../../include/openssl/sha.h ../../include/openssl/stack.h
$(MF_CONFIG)/m_dss1.o: ../../include/openssl/symhacks.h ../../include/openssl/x509.h
$(MF_CONFIG)/m_dss1.o: ../../include/openssl/x509_vfy.h ../cryptlib.h m_dss1.c
$(MF_CONFIG)/m_ecdsa.o: ../../e_os.h ../../include/openssl/asn1.h
$(MF_CONFIG)/m_ecdsa.o: ../../include/openssl/bio.h ../../include/openssl/buffer.h
$(MF_CONFIG)/m_ecdsa.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(MF_CONFIG)/m_ecdsa.o: ../../include/openssl/ec.h ../../include/openssl/ecdh.h
$(MF_CONFIG)/m_ecdsa.o: ../../include/openssl/ecdsa.h ../../include/openssl/err.h
$(MF_CONFIG)/m_ecdsa.o: ../../include/openssl/evp.h ../../include/openssl/lhash.h
$(MF_CONFIG)/m_ecdsa.o: ../../include/openssl/obj_mac.h ../../include/openssl/objects.h
$(MF_CONFIG)/m_ecdsa.o: ../../include/openssl/opensslconf.h ../../include/openssl/opensslv.h
$(MF_CONFIG)/m_ecdsa.o: ../../include/openssl/ossl_typ.h ../../include/openssl/pkcs7.h
$(MF_CONFIG)/m_ecdsa.o: ../../include/openssl/safestack.h ../../include/openssl/sha.h
$(MF_CONFIG)/m_ecdsa.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
$(MF_CONFIG)/m_ecdsa.o: ../../include/openssl/x509.h ../../include/openssl/x509_vfy.h
$(MF_CONFIG)/m_ecdsa.o: ../cryptlib.h m_ecdsa.c
$(MF_CONFIG)/m_md2.o: ../../e_os.h ../../include/openssl/bio.h
$(MF_CONFIG)/m_md2.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(MF_CONFIG)/m_md2.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(MF_CONFIG)/m_md2.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(MF_CONFIG)/m_md2.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(MF_CONFIG)/m_md2.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(MF_CONFIG)/m_md2.o: ../../include/openssl/symhacks.h ../cryptlib.h m_md2.c
$(MF_CONFIG)/m_md4.o: ../../e_os.h ../../include/openssl/asn1.h ../../include/openssl/bio.h
$(MF_CONFIG)/m_md4.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(MF_CONFIG)/m_md4.o: ../../include/openssl/e_os2.h ../../include/openssl/ec.h
$(MF_CONFIG)/m_md4.o: ../../include/openssl/ecdh.h ../../include/openssl/ecdsa.h
$(MF_CONFIG)/m_md4.o: ../../include/openssl/err.h ../../include/openssl/evp.h
$(MF_CONFIG)/m_md4.o: ../../include/openssl/lhash.h ../../include/openssl/md4.h
$(MF_CONFIG)/m_md4.o: ../../include/openssl/obj_mac.h ../../include/openssl/objects.h
$(MF_CONFIG)/m_md4.o: ../../include/openssl/opensslconf.h ../../include/openssl/opensslv.h
$(MF_CONFIG)/m_md4.o: ../../include/openssl/ossl_typ.h ../../include/openssl/pkcs7.h
$(MF_CONFIG)/m_md4.o: ../../include/openssl/rsa.h ../../include/openssl/safestack.h
$(MF_CONFIG)/m_md4.o: ../../include/openssl/sha.h ../../include/openssl/stack.h
$(MF_CONFIG)/m_md4.o: ../../include/openssl/symhacks.h ../../include/openssl/x509.h
$(MF_CONFIG)/m_md4.o: ../../include/openssl/x509_vfy.h ../cryptlib.h m_md4.c
$(MF_CONFIG)/m_md5.o: ../../e_os.h ../../include/openssl/asn1.h ../../include/openssl/bio.h
$(MF_CONFIG)/m_md5.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(MF_CONFIG)/m_md5.o: ../../include/openssl/e_os2.h ../../include/openssl/ec.h
$(MF_CONFIG)/m_md5.o: ../../include/openssl/ecdh.h ../../include/openssl/ecdsa.h
$(MF_CONFIG)/m_md5.o: ../../include/openssl/err.h ../../include/openssl/evp.h
$(MF_CONFIG)/m_md5.o: ../../include/openssl/lhash.h ../../include/openssl/md5.h
$(MF_CONFIG)/m_md5.o: ../../include/openssl/obj_mac.h ../../include/openssl/objects.h
$(MF_CONFIG)/m_md5.o: ../../include/openssl/opensslconf.h ../../include/openssl/opensslv.h
$(MF_CONFIG)/m_md5.o: ../../include/openssl/ossl_typ.h ../../include/openssl/pkcs7.h
$(MF_CONFIG)/m_md5.o: ../../include/openssl/rsa.h ../../include/openssl/safestack.h
$(MF_CONFIG)/m_md5.o: ../../include/openssl/sha.h ../../include/openssl/stack.h
$(MF_CONFIG)/m_md5.o: ../../include/openssl/symhacks.h ../../include/openssl/x509.h
$(MF_CONFIG)/m_md5.o: ../../include/openssl/x509_vfy.h ../cryptlib.h m_md5.c
$(MF_CONFIG)/m_mdc2.o: ../../e_os.h ../../include/openssl/bio.h
$(MF_CONFIG)/m_mdc2.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(MF_CONFIG)/m_mdc2.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(MF_CONFIG)/m_mdc2.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(MF_CONFIG)/m_mdc2.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(MF_CONFIG)/m_mdc2.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(MF_CONFIG)/m_mdc2.o: ../../include/openssl/symhacks.h ../cryptlib.h m_mdc2.c
$(MF_CONFIG)/m_null.o: ../../e_os.h ../../include/openssl/asn1.h ../../include/openssl/bio.h
$(MF_CONFIG)/m_null.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(MF_CONFIG)/m_null.o: ../../include/openssl/e_os2.h ../../include/openssl/ec.h
$(MF_CONFIG)/m_null.o: ../../include/openssl/ecdh.h ../../include/openssl/ecdsa.h
$(MF_CONFIG)/m_null.o: ../../include/openssl/err.h ../../include/openssl/evp.h
$(MF_CONFIG)/m_null.o: ../../include/openssl/lhash.h ../../include/openssl/obj_mac.h
$(MF_CONFIG)/m_null.o: ../../include/openssl/objects.h ../../include/openssl/opensslconf.h
$(MF_CONFIG)/m_null.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(MF_CONFIG)/m_null.o: ../../include/openssl/pkcs7.h ../../include/openssl/safestack.h
$(MF_CONFIG)/m_null.o: ../../include/openssl/sha.h ../../include/openssl/stack.h
$(MF_CONFIG)/m_null.o: ../../include/openssl/symhacks.h ../../include/openssl/x509.h
$(MF_CONFIG)/m_null.o: ../../include/openssl/x509_vfy.h ../cryptlib.h m_null.c
$(MF_CONFIG)/m_ripemd.o: ../../e_os.h ../../include/openssl/asn1.h
$(MF_CONFIG)/m_ripemd.o: ../../include/openssl/bio.h ../../include/openssl/buffer.h
$(MF_CONFIG)/m_ripemd.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(MF_CONFIG)/m_ripemd.o: ../../include/openssl/ec.h ../../include/openssl/ecdh.h
$(MF_CONFIG)/m_ripemd.o: ../../include/openssl/ecdsa.h ../../include/openssl/err.h
$(MF_CONFIG)/m_ripemd.o: ../../include/openssl/evp.h ../../include/openssl/lhash.h
$(MF_CONFIG)/m_ripemd.o: ../../include/openssl/obj_mac.h ../../include/openssl/objects.h
$(MF_CONFIG)/m_ripemd.o: ../../include/openssl/opensslconf.h
$(MF_CONFIG)/m_ripemd.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(MF_CONFIG)/m_ripemd.o: ../../include/openssl/pkcs7.h ../../include/openssl/ripemd.h
$(MF_CONFIG)/m_ripemd.o: ../../include/openssl/rsa.h ../../include/openssl/safestack.h
$(MF_CONFIG)/m_ripemd.o: ../../include/openssl/sha.h ../../include/openssl/stack.h
$(MF_CONFIG)/m_ripemd.o: ../../include/openssl/symhacks.h ../../include/openssl/x509.h
$(MF_CONFIG)/m_ripemd.o: ../../include/openssl/x509_vfy.h ../cryptlib.h m_ripemd.c
$(MF_CONFIG)/m_sha.o: ../../e_os.h ../../include/openssl/asn1.h ../../include/openssl/bio.h
$(MF_CONFIG)/m_sha.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(MF_CONFIG)/m_sha.o: ../../include/openssl/e_os2.h ../../include/openssl/ec.h
$(MF_CONFIG)/m_sha.o: ../../include/openssl/ecdh.h ../../include/openssl/ecdsa.h
$(MF_CONFIG)/m_sha.o: ../../include/openssl/err.h ../../include/openssl/evp.h
$(MF_CONFIG)/m_sha.o: ../../include/openssl/lhash.h ../../include/openssl/obj_mac.h
$(MF_CONFIG)/m_sha.o: ../../include/openssl/objects.h ../../include/openssl/opensslconf.h
$(MF_CONFIG)/m_sha.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(MF_CONFIG)/m_sha.o: ../../include/openssl/pkcs7.h ../../include/openssl/rsa.h
$(MF_CONFIG)/m_sha.o: ../../include/openssl/safestack.h ../../include/openssl/sha.h
$(MF_CONFIG)/m_sha.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
$(MF_CONFIG)/m_sha.o: ../../include/openssl/x509.h ../../include/openssl/x509_vfy.h
$(MF_CONFIG)/m_sha.o: ../cryptlib.h m_sha.c
$(MF_CONFIG)/m_sha1.o: ../../e_os.h ../../include/openssl/asn1.h ../../include/openssl/bio.h
$(MF_CONFIG)/m_sha1.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(MF_CONFIG)/m_sha1.o: ../../include/openssl/e_os2.h ../../include/openssl/ec.h
$(MF_CONFIG)/m_sha1.o: ../../include/openssl/ecdh.h ../../include/openssl/ecdsa.h
$(MF_CONFIG)/m_sha1.o: ../../include/openssl/err.h ../../include/openssl/evp.h
$(MF_CONFIG)/m_sha1.o: ../../include/openssl/lhash.h ../../include/openssl/obj_mac.h
$(MF_CONFIG)/m_sha1.o: ../../include/openssl/objects.h ../../include/openssl/opensslconf.h
$(MF_CONFIG)/m_sha1.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(MF_CONFIG)/m_sha1.o: ../../include/openssl/pkcs7.h ../../include/openssl/rsa.h
$(MF_CONFIG)/m_sha1.o: ../../include/openssl/safestack.h ../../include/openssl/sha.h
$(MF_CONFIG)/m_sha1.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
$(MF_CONFIG)/m_sha1.o: ../../include/openssl/x509.h ../../include/openssl/x509_vfy.h
$(MF_CONFIG)/m_sha1.o: ../cryptlib.h m_sha1.c
$(MF_CONFIG)/names.o: ../../e_os.h ../../include/openssl/asn1.h ../../include/openssl/bio.h
$(MF_CONFIG)/names.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(MF_CONFIG)/names.o: ../../include/openssl/e_os2.h ../../include/openssl/ec.h
$(MF_CONFIG)/names.o: ../../include/openssl/ecdh.h ../../include/openssl/ecdsa.h
$(MF_CONFIG)/names.o: ../../include/openssl/err.h ../../include/openssl/evp.h
$(MF_CONFIG)/names.o: ../../include/openssl/lhash.h ../../include/openssl/obj_mac.h
$(MF_CONFIG)/names.o: ../../include/openssl/objects.h ../../include/openssl/opensslconf.h
$(MF_CONFIG)/names.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(MF_CONFIG)/names.o: ../../include/openssl/pkcs7.h ../../include/openssl/safestack.h
$(MF_CONFIG)/names.o: ../../include/openssl/sha.h ../../include/openssl/stack.h
$(MF_CONFIG)/names.o: ../../include/openssl/symhacks.h ../../include/openssl/x509.h
$(MF_CONFIG)/names.o: ../../include/openssl/x509_vfy.h ../cryptlib.h names.c
$(MF_CONFIG)/p5_crpt.o: ../../e_os.h ../../include/openssl/asn1.h
$(MF_CONFIG)/p5_crpt.o: ../../include/openssl/bio.h ../../include/openssl/buffer.h
$(MF_CONFIG)/p5_crpt.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(MF_CONFIG)/p5_crpt.o: ../../include/openssl/ec.h ../../include/openssl/ecdh.h
$(MF_CONFIG)/p5_crpt.o: ../../include/openssl/ecdsa.h ../../include/openssl/err.h
$(MF_CONFIG)/p5_crpt.o: ../../include/openssl/evp.h ../../include/openssl/lhash.h
$(MF_CONFIG)/p5_crpt.o: ../../include/openssl/obj_mac.h ../../include/openssl/objects.h
$(MF_CONFIG)/p5_crpt.o: ../../include/openssl/opensslconf.h ../../include/openssl/opensslv.h
$(MF_CONFIG)/p5_crpt.o: ../../include/openssl/ossl_typ.h ../../include/openssl/pkcs7.h
$(MF_CONFIG)/p5_crpt.o: ../../include/openssl/safestack.h ../../include/openssl/sha.h
$(MF_CONFIG)/p5_crpt.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
$(MF_CONFIG)/p5_crpt.o: ../../include/openssl/x509.h ../../include/openssl/x509_vfy.h
$(MF_CONFIG)/p5_crpt.o: ../cryptlib.h p5_crpt.c
$(MF_CONFIG)/p5_crpt2.o: ../../e_os.h ../../include/openssl/asn1.h
$(MF_CONFIG)/p5_crpt2.o: ../../include/openssl/bio.h ../../include/openssl/buffer.h
$(MF_CONFIG)/p5_crpt2.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(MF_CONFIG)/p5_crpt2.o: ../../include/openssl/ec.h ../../include/openssl/ecdh.h
$(MF_CONFIG)/p5_crpt2.o: ../../include/openssl/ecdsa.h ../../include/openssl/err.h
$(MF_CONFIG)/p5_crpt2.o: ../../include/openssl/evp.h ../../include/openssl/hmac.h
$(MF_CONFIG)/p5_crpt2.o: ../../include/openssl/lhash.h ../../include/openssl/obj_mac.h
$(MF_CONFIG)/p5_crpt2.o: ../../include/openssl/objects.h ../../include/openssl/opensslconf.h
$(MF_CONFIG)/p5_crpt2.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(MF_CONFIG)/p5_crpt2.o: ../../include/openssl/pkcs7.h ../../include/openssl/safestack.h
$(MF_CONFIG)/p5_crpt2.o: ../../include/openssl/sha.h ../../include/openssl/stack.h
$(MF_CONFIG)/p5_crpt2.o: ../../include/openssl/symhacks.h ../../include/openssl/x509.h
$(MF_CONFIG)/p5_crpt2.o: ../../include/openssl/x509_vfy.h ../cryptlib.h p5_crpt2.c
$(MF_CONFIG)/p_dec.o: ../../e_os.h ../../include/openssl/asn1.h ../../include/openssl/bio.h
$(MF_CONFIG)/p_dec.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(MF_CONFIG)/p_dec.o: ../../include/openssl/e_os2.h ../../include/openssl/ec.h
$(MF_CONFIG)/p_dec.o: ../../include/openssl/ecdh.h ../../include/openssl/ecdsa.h
$(MF_CONFIG)/p_dec.o: ../../include/openssl/err.h ../../include/openssl/evp.h
$(MF_CONFIG)/p_dec.o: ../../include/openssl/lhash.h ../../include/openssl/obj_mac.h
$(MF_CONFIG)/p_dec.o: ../../include/openssl/objects.h ../../include/openssl/opensslconf.h
$(MF_CONFIG)/p_dec.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(MF_CONFIG)/p_dec.o: ../../include/openssl/pkcs7.h ../../include/openssl/rand.h
$(MF_CONFIG)/p_dec.o: ../../include/openssl/rsa.h ../../include/openssl/safestack.h
$(MF_CONFIG)/p_dec.o: ../../include/openssl/sha.h ../../include/openssl/stack.h
$(MF_CONFIG)/p_dec.o: ../../include/openssl/symhacks.h ../../include/openssl/x509.h
$(MF_CONFIG)/p_dec.o: ../../include/openssl/x509_vfy.h ../cryptlib.h p_dec.c
$(MF_CONFIG)/p_enc.o: ../../e_os.h ../../include/openssl/asn1.h ../../include/openssl/bio.h
$(MF_CONFIG)/p_enc.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(MF_CONFIG)/p_enc.o: ../../include/openssl/e_os2.h ../../include/openssl/ec.h
$(MF_CONFIG)/p_enc.o: ../../include/openssl/ecdh.h ../../include/openssl/ecdsa.h
$(MF_CONFIG)/p_enc.o: ../../include/openssl/err.h ../../include/openssl/evp.h
$(MF_CONFIG)/p_enc.o: ../../include/openssl/lhash.h ../../include/openssl/obj_mac.h
$(MF_CONFIG)/p_enc.o: ../../include/openssl/objects.h ../../include/openssl/opensslconf.h
$(MF_CONFIG)/p_enc.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(MF_CONFIG)/p_enc.o: ../../include/openssl/pkcs7.h ../../include/openssl/rand.h
$(MF_CONFIG)/p_enc.o: ../../include/openssl/rsa.h ../../include/openssl/safestack.h
$(MF_CONFIG)/p_enc.o: ../../include/openssl/sha.h ../../include/openssl/stack.h
$(MF_CONFIG)/p_enc.o: ../../include/openssl/symhacks.h ../../include/openssl/x509.h
$(MF_CONFIG)/p_enc.o: ../../include/openssl/x509_vfy.h ../cryptlib.h p_enc.c
$(MF_CONFIG)/p_lib.o: ../../e_os.h ../../include/openssl/asn1.h
$(MF_CONFIG)/p_lib.o: ../../include/openssl/asn1_mac.h ../../include/openssl/bio.h
$(MF_CONFIG)/p_lib.o: ../../include/openssl/bn.h ../../include/openssl/buffer.h
$(MF_CONFIG)/p_lib.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(MF_CONFIG)/p_lib.o: ../../include/openssl/ec.h ../../include/openssl/ecdh.h
$(MF_CONFIG)/p_lib.o: ../../include/openssl/ecdsa.h ../../include/openssl/err.h
$(MF_CONFIG)/p_lib.o: ../../include/openssl/evp.h ../../include/openssl/lhash.h
$(MF_CONFIG)/p_lib.o: ../../include/openssl/obj_mac.h ../../include/openssl/objects.h
$(MF_CONFIG)/p_lib.o: ../../include/openssl/opensslconf.h ../../include/openssl/opensslv.h
$(MF_CONFIG)/p_lib.o: ../../include/openssl/ossl_typ.h ../../include/openssl/pkcs7.h
$(MF_CONFIG)/p_lib.o: ../../include/openssl/rsa.h ../../include/openssl/safestack.h
$(MF_CONFIG)/p_lib.o: ../../include/openssl/sha.h ../../include/openssl/stack.h
$(MF_CONFIG)/p_lib.o: ../../include/openssl/symhacks.h ../../include/openssl/x509.h
$(MF_CONFIG)/p_lib.o: ../../include/openssl/x509_vfy.h ../cryptlib.h p_lib.c
$(MF_CONFIG)/p_open.o: ../../e_os.h ../../include/openssl/asn1.h ../../include/openssl/bio.h
$(MF_CONFIG)/p_open.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(MF_CONFIG)/p_open.o: ../../include/openssl/e_os2.h ../../include/openssl/ec.h
$(MF_CONFIG)/p_open.o: ../../include/openssl/ecdh.h ../../include/openssl/ecdsa.h
$(MF_CONFIG)/p_open.o: ../../include/openssl/err.h ../../include/openssl/evp.h
$(MF_CONFIG)/p_open.o: ../../include/openssl/lhash.h ../../include/openssl/obj_mac.h
$(MF_CONFIG)/p_open.o: ../../include/openssl/objects.h ../../include/openssl/opensslconf.h
$(MF_CONFIG)/p_open.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(MF_CONFIG)/p_open.o: ../../include/openssl/pkcs7.h ../../include/openssl/rsa.h
$(MF_CONFIG)/p_open.o: ../../include/openssl/safestack.h ../../include/openssl/sha.h
$(MF_CONFIG)/p_open.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
$(MF_CONFIG)/p_open.o: ../../include/openssl/x509.h ../../include/openssl/x509_vfy.h
$(MF_CONFIG)/p_open.o: ../cryptlib.h p_open.c
$(MF_CONFIG)/p_seal.o: ../../e_os.h ../../include/openssl/asn1.h ../../include/openssl/bio.h
$(MF_CONFIG)/p_seal.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(MF_CONFIG)/p_seal.o: ../../include/openssl/e_os2.h ../../include/openssl/ec.h
$(MF_CONFIG)/p_seal.o: ../../include/openssl/ecdh.h ../../include/openssl/ecdsa.h
$(MF_CONFIG)/p_seal.o: ../../include/openssl/err.h ../../include/openssl/evp.h
$(MF_CONFIG)/p_seal.o: ../../include/openssl/lhash.h ../../include/openssl/obj_mac.h
$(MF_CONFIG)/p_seal.o: ../../include/openssl/objects.h ../../include/openssl/opensslconf.h
$(MF_CONFIG)/p_seal.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(MF_CONFIG)/p_seal.o: ../../include/openssl/pkcs7.h ../../include/openssl/rand.h
$(MF_CONFIG)/p_seal.o: ../../include/openssl/rsa.h ../../include/openssl/safestack.h
$(MF_CONFIG)/p_seal.o: ../../include/openssl/sha.h ../../include/openssl/stack.h
$(MF_CONFIG)/p_seal.o: ../../include/openssl/symhacks.h ../../include/openssl/x509.h
$(MF_CONFIG)/p_seal.o: ../../include/openssl/x509_vfy.h ../cryptlib.h p_seal.c
$(MF_CONFIG)/p_sign.o: ../../e_os.h ../../include/openssl/asn1.h ../../include/openssl/bio.h
$(MF_CONFIG)/p_sign.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(MF_CONFIG)/p_sign.o: ../../include/openssl/e_os2.h ../../include/openssl/ec.h
$(MF_CONFIG)/p_sign.o: ../../include/openssl/ecdh.h ../../include/openssl/ecdsa.h
$(MF_CONFIG)/p_sign.o: ../../include/openssl/err.h ../../include/openssl/evp.h
$(MF_CONFIG)/p_sign.o: ../../include/openssl/lhash.h ../../include/openssl/obj_mac.h
$(MF_CONFIG)/p_sign.o: ../../include/openssl/objects.h ../../include/openssl/opensslconf.h
$(MF_CONFIG)/p_sign.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(MF_CONFIG)/p_sign.o: ../../include/openssl/pkcs7.h ../../include/openssl/safestack.h
$(MF_CONFIG)/p_sign.o: ../../include/openssl/sha.h ../../include/openssl/stack.h
$(MF_CONFIG)/p_sign.o: ../../include/openssl/symhacks.h ../../include/openssl/x509.h
$(MF_CONFIG)/p_sign.o: ../../include/openssl/x509_vfy.h ../cryptlib.h p_sign.c
$(MF_CONFIG)/p_verify.o: ../../e_os.h ../../include/openssl/asn1.h
$(MF_CONFIG)/p_verify.o: ../../include/openssl/bio.h ../../include/openssl/buffer.h
$(MF_CONFIG)/p_verify.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(MF_CONFIG)/p_verify.o: ../../include/openssl/ec.h ../../include/openssl/ecdh.h
$(MF_CONFIG)/p_verify.o: ../../include/openssl/ecdsa.h ../../include/openssl/err.h
$(MF_CONFIG)/p_verify.o: ../../include/openssl/evp.h ../../include/openssl/lhash.h
$(MF_CONFIG)/p_verify.o: ../../include/openssl/obj_mac.h ../../include/openssl/objects.h
$(MF_CONFIG)/p_verify.o: ../../include/openssl/opensslconf.h
$(MF_CONFIG)/p_verify.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(MF_CONFIG)/p_verify.o: ../../include/openssl/pkcs7.h ../../include/openssl/safestack.h
$(MF_CONFIG)/p_verify.o: ../../include/openssl/sha.h ../../include/openssl/stack.h
$(MF_CONFIG)/p_verify.o: ../../include/openssl/symhacks.h ../../include/openssl/x509.h
$(MF_CONFIG)/p_verify.o: ../../include/openssl/x509_vfy.h ../cryptlib.h p_verify.c
