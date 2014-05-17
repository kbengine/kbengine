#
# OpenSSL/crypto/asn1/Makefile
#

ifndef KBE_ROOT
export KBE_ROOT := $(subst /kbe/src/lib/dependencies/openssl/crypto/asn1,,$(CURDIR))
endif

LIBDIR= $(KBE_ROOT)/kbe/src/libs

DIR=	asn1
TOP=	../..
CC=	cc
INCLUDES= -I.. -I$(TOP) -I../../include
CFLAG=-g
MAKEFILE=	Makefile
AR=		ar r

CFLAGS= $(INCLUDES) $(CFLAG)

GENERAL=Makefile README
TEST=
APPS=

LIB=$(LIBDIR)/libcrypto.a
LIBSRC=	a_object.c a_bitstr.c a_utctm.c a_gentm.c a_time.c a_int.c a_octet.c \
	a_print.c a_type.c a_set.c a_dup.c a_d2i_fp.c a_i2d_fp.c \
	a_enum.c a_utf8.c a_sign.c a_digest.c a_verify.c a_mbstr.c a_strex.c \
	x_algor.c x_val.c x_pubkey.c x_sig.c x_req.c x_attrib.c x_bignum.c \
	x_long.c x_name.c x_x509.c x_x509a.c x_crl.c x_info.c x_spki.c nsseq.c \
	d2i_pu.c d2i_pr.c i2d_pu.c i2d_pr.c\
	t_req.c t_x509.c t_x509a.c t_crl.c t_pkey.c t_spki.c t_bitst.c \
	tasn_new.c tasn_fre.c tasn_enc.c tasn_dec.c tasn_utl.c tasn_typ.c \
	f_int.c f_string.c n_pkey.c \
	f_enum.c a_hdr.c x_pkey.c a_bool.c x_exten.c \
	asn1_gen.c asn1_par.c asn1_lib.c asn1_err.c a_meth.c a_bytes.c a_strnid.c \
	evp_asn1.c asn_pack.c p5_pbe.c p5_pbev2.c p8_pkey.c asn_moid.c
LIBOBJ= $(KBE_CONFIG)/a_object.o $(KBE_CONFIG)/a_bitstr.o $(KBE_CONFIG)/a_utctm.o \
	$(KBE_CONFIG)/a_gentm.o $(KBE_CONFIG)/a_time.o $(KBE_CONFIG)/a_int.o \
	$(KBE_CONFIG)/a_octet.o $(KBE_CONFIG)/a_print.o $(KBE_CONFIG)/a_type.o \
	$(KBE_CONFIG)/a_set.o $(KBE_CONFIG)/a_dup.o $(KBE_CONFIG)/a_d2i_fp.o \
	$(KBE_CONFIG)/a_i2d_fp.o $(KBE_CONFIG)/a_enum.o $(KBE_CONFIG)/a_utf8.o \
	$(KBE_CONFIG)/a_sign.o $(KBE_CONFIG)/a_digest.o $(KBE_CONFIG)/a_verify.o \
	$(KBE_CONFIG)/a_mbstr.o $(KBE_CONFIG)/a_strex.o $(KBE_CONFIG)/x_algor.o \
	$(KBE_CONFIG)/x_val.o $(KBE_CONFIG)/x_pubkey.o $(KBE_CONFIG)/x_sig.o \
	$(KBE_CONFIG)/x_req.o $(KBE_CONFIG)/x_attrib.o $(KBE_CONFIG)/x_bignum.o \
	$(KBE_CONFIG)/x_long.o $(KBE_CONFIG)/x_name.o $(KBE_CONFIG)/x_x509.o \
	$(KBE_CONFIG)/x_x509a.o $(KBE_CONFIG)/x_crl.o $(KBE_CONFIG)/x_info.o \
	$(KBE_CONFIG)/x_spki.o $(KBE_CONFIG)/nsseq.o $(KBE_CONFIG)/d2i_pu.o \
	$(KBE_CONFIG)/d2i_pr.o $(KBE_CONFIG)/i2d_pu.o $(KBE_CONFIG)/i2d_pr.o \
	$(KBE_CONFIG)/t_req.o $(KBE_CONFIG)/t_x509.o $(KBE_CONFIG)/t_x509a.o \
	$(KBE_CONFIG)/t_crl.o $(KBE_CONFIG)/t_pkey.o $(KBE_CONFIG)/t_spki.o \
	$(KBE_CONFIG)/t_bitst.o $(KBE_CONFIG)/tasn_new.o $(KBE_CONFIG)/tasn_fre.o \
	$(KBE_CONFIG)/tasn_enc.o $(KBE_CONFIG)/tasn_dec.o $(KBE_CONFIG)/tasn_utl.o \
	$(KBE_CONFIG)/tasn_typ.o $(KBE_CONFIG)/f_int.o $(KBE_CONFIG)/f_string.o \
	$(KBE_CONFIG)/n_pkey.o $(KBE_CONFIG)/f_enum.o $(KBE_CONFIG)/a_hdr.o \
	$(KBE_CONFIG)/x_pkey.o $(KBE_CONFIG)/a_bool.o $(KBE_CONFIG)/x_exten.o \
	$(KBE_CONFIG)/asn1_gen.o $(KBE_CONFIG)/asn1_par.o $(KBE_CONFIG)/asn1_lib.o \
	$(KBE_CONFIG)/asn1_err.o $(KBE_CONFIG)/a_meth.o $(KBE_CONFIG)/a_bytes.o \
	$(KBE_CONFIG)/a_strnid.o $(KBE_CONFIG)/evp_asn1.o $(KBE_CONFIG)/asn_pack.o \
	$(KBE_CONFIG)/p5_pbe.o $(KBE_CONFIG)/p5_pbev2.o $(KBE_CONFIG)/p8_pkey.o \
	$(KBE_CONFIG)/asn_moid.o

SRC= $(LIBSRC)

EXHEADER=  asn1.h asn1_mac.h asn1t.h
HEADER=	$(EXHEADER)

ALL=    $(GENERAL) $(SRC) $(HEADER)

top:
	(cd ../..; $(MAKE) DIRS=crypto SDIRS=$(DIR) sub_all)

test:	test.c
	cc -g -I../../include -c test.c
	cc -g -I../../include -o test test.o -L../.. -lcrypto

pk:	pk.c
	cc -g -I../../include -c pk.c
	cc -g -I../../include -o pk pk.o -L../.. -lcrypto

all:	lib

$(KBE_CONFIG):
	@-mkdir $(KBE_CONFIG)

lib:	$(KBE_CONFIG) $(LIBOBJ)
	$(AR) $(LIB) $(LIBOBJ)
	$(RANLIB) $(LIB) || echo Never mind.
	@touch lib

$(KBE_CONFIG)/%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

files:
	$(PERL) $(TOP)/util/files.pl Makefile >> $(TOP)/MINFO

links:
	@$(PERL) $(TOP)/util/mklink.pl ../../include/openssl $(EXHEADER)
	@$(PERL) $(TOP)/util/mklink.pl ../../test $(TEST)
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
	@[ -n "$(MAKEDEPEND)" ] # should be set by top Makefile...
	$(MAKEDEPEND) -- $(CFLAG) $(INCLUDES) $(DEPFLAG) -- $(PROGS) $(LIBSRC)

dclean:
	$(PERL) -pe 'if (/^# DO NOT DELETE THIS LINE/) {print; exit(0);}' $(MAKEFILE) >Makefile.new
	mv -f Makefile.new $(MAKEFILE)

clean:
	rm -f $(KBE_CONFIG)/*.o *.o *.obj lib tags core .pure .nfs* *.old *.bak fluff


# DO NOT DELETE THIS LINE -- make depend depends on it.

$(KBE_CONFIG)/a_bitstr.o: ../../e_os.h ../../include/openssl/asn1.h
$(KBE_CONFIG)/a_bitstr.o: ../../include/openssl/bio.h ../../include/openssl/buffer.h
$(KBE_CONFIG)/a_bitstr.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(KBE_CONFIG)/a_bitstr.o: ../../include/openssl/err.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/a_bitstr.o: ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/a_bitstr.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/a_bitstr.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/a_bitstr.o: ../../include/openssl/symhacks.h ../cryptlib.h a_bitstr.c
$(KBE_CONFIG)/a_bool.o: ../../e_os.h ../../include/openssl/asn1.h
$(KBE_CONFIG)/a_bool.o: ../../include/openssl/asn1t.h ../../include/openssl/bio.h
$(KBE_CONFIG)/a_bool.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/a_bool.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(KBE_CONFIG)/a_bool.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/a_bool.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/a_bool.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/a_bool.o: ../../include/openssl/symhacks.h ../cryptlib.h a_bool.c
$(KBE_CONFIG)/a_bytes.o: ../../e_os.h ../../include/openssl/asn1.h
$(KBE_CONFIG)/a_bytes.o: ../../include/openssl/bio.h ../../include/openssl/buffer.h
$(KBE_CONFIG)/a_bytes.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(KBE_CONFIG)/a_bytes.o: ../../include/openssl/err.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/a_bytes.o: ../../include/openssl/opensslconf.h ../../include/openssl/opensslv.h
$(KBE_CONFIG)/a_bytes.o: ../../include/openssl/ossl_typ.h ../../include/openssl/safestack.h
$(KBE_CONFIG)/a_bytes.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
$(KBE_CONFIG)/a_bytes.o: ../cryptlib.h a_bytes.c
$(KBE_CONFIG)/a_d2i_fp.o: ../../e_os.h ../../include/openssl/asn1.h
$(KBE_CONFIG)/a_d2i_fp.o: ../../include/openssl/asn1_mac.h ../../include/openssl/bio.h
$(KBE_CONFIG)/a_d2i_fp.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/a_d2i_fp.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(KBE_CONFIG)/a_d2i_fp.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/a_d2i_fp.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/a_d2i_fp.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/a_d2i_fp.o: ../../include/openssl/symhacks.h ../cryptlib.h a_d2i_fp.c
$(KBE_CONFIG)/a_digest.o: ../../e_os.h ../../include/openssl/asn1.h
$(KBE_CONFIG)/a_digest.o: ../../include/openssl/bio.h ../../include/openssl/buffer.h
$(KBE_CONFIG)/a_digest.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(KBE_CONFIG)/a_digest.o: ../../include/openssl/ec.h ../../include/openssl/ecdh.h
$(KBE_CONFIG)/a_digest.o: ../../include/openssl/ecdsa.h ../../include/openssl/err.h
$(KBE_CONFIG)/a_digest.o: ../../include/openssl/evp.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/a_digest.o: ../../include/openssl/obj_mac.h ../../include/openssl/objects.h
$(KBE_CONFIG)/a_digest.o: ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/a_digest.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/a_digest.o: ../../include/openssl/pkcs7.h ../../include/openssl/safestack.h
$(KBE_CONFIG)/a_digest.o: ../../include/openssl/sha.h ../../include/openssl/stack.h
$(KBE_CONFIG)/a_digest.o: ../../include/openssl/symhacks.h ../../include/openssl/x509.h
$(KBE_CONFIG)/a_digest.o: ../../include/openssl/x509_vfy.h ../cryptlib.h a_digest.c
$(KBE_CONFIG)/a_dup.o: ../../e_os.h ../../include/openssl/asn1.h ../../include/openssl/bio.h
$(KBE_CONFIG)/a_dup.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/a_dup.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(KBE_CONFIG)/a_dup.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/a_dup.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/a_dup.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/a_dup.o: ../../include/openssl/symhacks.h ../cryptlib.h a_dup.c
$(KBE_CONFIG)/a_enum.o: ../../e_os.h ../../include/openssl/asn1.h ../../include/openssl/bio.h
$(KBE_CONFIG)/a_enum.o: ../../include/openssl/bn.h ../../include/openssl/buffer.h
$(KBE_CONFIG)/a_enum.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(KBE_CONFIG)/a_enum.o: ../../include/openssl/err.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/a_enum.o: ../../include/openssl/opensslconf.h ../../include/openssl/opensslv.h
$(KBE_CONFIG)/a_enum.o: ../../include/openssl/ossl_typ.h ../../include/openssl/safestack.h
$(KBE_CONFIG)/a_enum.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
$(KBE_CONFIG)/a_enum.o: ../cryptlib.h a_enum.c
$(KBE_CONFIG)/a_gentm.o: ../../e_os.h ../../include/openssl/asn1.h
$(KBE_CONFIG)/a_gentm.o: ../../include/openssl/bio.h ../../include/openssl/buffer.h
$(KBE_CONFIG)/a_gentm.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(KBE_CONFIG)/a_gentm.o: ../../include/openssl/err.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/a_gentm.o: ../../include/openssl/opensslconf.h ../../include/openssl/opensslv.h
$(KBE_CONFIG)/a_gentm.o: ../../include/openssl/ossl_typ.h ../../include/openssl/safestack.h
$(KBE_CONFIG)/a_gentm.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
$(KBE_CONFIG)/a_gentm.o: ../cryptlib.h ../o_time.h a_gentm.c
$(KBE_CONFIG)/a_hdr.o: ../../e_os.h ../../include/openssl/asn1.h
$(KBE_CONFIG)/a_hdr.o: ../../include/openssl/asn1_mac.h ../../include/openssl/bio.h
$(KBE_CONFIG)/a_hdr.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/a_hdr.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(KBE_CONFIG)/a_hdr.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/a_hdr.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/a_hdr.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/a_hdr.o: ../../include/openssl/symhacks.h ../cryptlib.h a_hdr.c
$(KBE_CONFIG)/a_i2d_fp.o: ../../e_os.h ../../include/openssl/asn1.h
$(KBE_CONFIG)/a_i2d_fp.o: ../../include/openssl/bio.h ../../include/openssl/buffer.h
$(KBE_CONFIG)/a_i2d_fp.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(KBE_CONFIG)/a_i2d_fp.o: ../../include/openssl/err.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/a_i2d_fp.o: ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/a_i2d_fp.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/a_i2d_fp.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/a_i2d_fp.o: ../../include/openssl/symhacks.h ../cryptlib.h a_i2d_fp.c
$(KBE_CONFIG)/a_int.o: ../../e_os.h ../../include/openssl/asn1.h ../../include/openssl/bio.h
$(KBE_CONFIG)/a_int.o: ../../include/openssl/bn.h ../../include/openssl/buffer.h
$(KBE_CONFIG)/a_int.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(KBE_CONFIG)/a_int.o: ../../include/openssl/err.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/a_int.o: ../../include/openssl/opensslconf.h ../../include/openssl/opensslv.h
$(KBE_CONFIG)/a_int.o: ../../include/openssl/ossl_typ.h ../../include/openssl/safestack.h
$(KBE_CONFIG)/a_int.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
$(KBE_CONFIG)/a_int.o: ../cryptlib.h a_int.c
$(KBE_CONFIG)/a_mbstr.o: ../../e_os.h ../../include/openssl/asn1.h
$(KBE_CONFIG)/a_mbstr.o: ../../include/openssl/bio.h ../../include/openssl/buffer.h
$(KBE_CONFIG)/a_mbstr.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(KBE_CONFIG)/a_mbstr.o: ../../include/openssl/err.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/a_mbstr.o: ../../include/openssl/opensslconf.h ../../include/openssl/opensslv.h
$(KBE_CONFIG)/a_mbstr.o: ../../include/openssl/ossl_typ.h ../../include/openssl/safestack.h
$(KBE_CONFIG)/a_mbstr.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
$(KBE_CONFIG)/a_mbstr.o: ../cryptlib.h a_mbstr.c
$(KBE_CONFIG)/a_meth.o: ../../e_os.h ../../include/openssl/asn1.h ../../include/openssl/bio.h
$(KBE_CONFIG)/a_meth.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/a_meth.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(KBE_CONFIG)/a_meth.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/a_meth.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/a_meth.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/a_meth.o: ../../include/openssl/symhacks.h ../cryptlib.h a_meth.c
$(KBE_CONFIG)/a_object.o: ../../e_os.h ../../include/openssl/asn1.h
$(KBE_CONFIG)/a_object.o: ../../include/openssl/bio.h ../../include/openssl/buffer.h
$(KBE_CONFIG)/a_object.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(KBE_CONFIG)/a_object.o: ../../include/openssl/err.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/a_object.o: ../../include/openssl/obj_mac.h ../../include/openssl/objects.h
$(KBE_CONFIG)/a_object.o: ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/a_object.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/a_object.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/a_object.o: ../../include/openssl/symhacks.h ../cryptlib.h a_object.c
$(KBE_CONFIG)/a_octet.o: ../../e_os.h ../../include/openssl/asn1.h
$(KBE_CONFIG)/a_octet.o: ../../include/openssl/bio.h ../../include/openssl/buffer.h
$(KBE_CONFIG)/a_octet.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(KBE_CONFIG)/a_octet.o: ../../include/openssl/err.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/a_octet.o: ../../include/openssl/opensslconf.h ../../include/openssl/opensslv.h
$(KBE_CONFIG)/a_octet.o: ../../include/openssl/ossl_typ.h ../../include/openssl/safestack.h
$(KBE_CONFIG)/a_octet.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
$(KBE_CONFIG)/a_octet.o: ../cryptlib.h a_octet.c
$(KBE_CONFIG)/a_print.o: ../../e_os.h ../../include/openssl/asn1.h
$(KBE_CONFIG)/a_print.o: ../../include/openssl/bio.h ../../include/openssl/buffer.h
$(KBE_CONFIG)/a_print.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(KBE_CONFIG)/a_print.o: ../../include/openssl/err.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/a_print.o: ../../include/openssl/opensslconf.h ../../include/openssl/opensslv.h
$(KBE_CONFIG)/a_print.o: ../../include/openssl/ossl_typ.h ../../include/openssl/safestack.h
$(KBE_CONFIG)/a_print.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
$(KBE_CONFIG)/a_print.o: ../cryptlib.h a_print.c
$(KBE_CONFIG)/a_set.o: ../../e_os.h ../../include/openssl/asn1.h
$(KBE_CONFIG)/a_set.o: ../../include/openssl/asn1_mac.h ../../include/openssl/bio.h
$(KBE_CONFIG)/a_set.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/a_set.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(KBE_CONFIG)/a_set.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/a_set.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/a_set.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/a_set.o: ../../include/openssl/symhacks.h ../cryptlib.h a_set.c
$(KBE_CONFIG)/a_sign.o: ../../e_os.h ../../include/openssl/asn1.h ../../include/openssl/bio.h
$(KBE_CONFIG)/a_sign.o: ../../include/openssl/bn.h ../../include/openssl/buffer.h
$(KBE_CONFIG)/a_sign.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(KBE_CONFIG)/a_sign.o: ../../include/openssl/ec.h ../../include/openssl/ecdh.h
$(KBE_CONFIG)/a_sign.o: ../../include/openssl/ecdsa.h ../../include/openssl/err.h
$(KBE_CONFIG)/a_sign.o: ../../include/openssl/evp.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/a_sign.o: ../../include/openssl/obj_mac.h ../../include/openssl/objects.h
$(KBE_CONFIG)/a_sign.o: ../../include/openssl/opensslconf.h ../../include/openssl/opensslv.h
$(KBE_CONFIG)/a_sign.o: ../../include/openssl/ossl_typ.h ../../include/openssl/pkcs7.h
$(KBE_CONFIG)/a_sign.o: ../../include/openssl/safestack.h ../../include/openssl/sha.h
$(KBE_CONFIG)/a_sign.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
$(KBE_CONFIG)/a_sign.o: ../../include/openssl/x509.h ../../include/openssl/x509_vfy.h
$(KBE_CONFIG)/a_sign.o: ../cryptlib.h a_sign.c
$(KBE_CONFIG)/a_strex.o: ../../e_os.h ../../include/openssl/asn1.h
$(KBE_CONFIG)/a_strex.o: ../../include/openssl/bio.h ../../include/openssl/buffer.h
$(KBE_CONFIG)/a_strex.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(KBE_CONFIG)/a_strex.o: ../../include/openssl/ec.h ../../include/openssl/ecdh.h
$(KBE_CONFIG)/a_strex.o: ../../include/openssl/ecdsa.h ../../include/openssl/err.h
$(KBE_CONFIG)/a_strex.o: ../../include/openssl/evp.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/a_strex.o: ../../include/openssl/obj_mac.h ../../include/openssl/objects.h
$(KBE_CONFIG)/a_strex.o: ../../include/openssl/opensslconf.h ../../include/openssl/opensslv.h
$(KBE_CONFIG)/a_strex.o: ../../include/openssl/ossl_typ.h ../../include/openssl/pkcs7.h
$(KBE_CONFIG)/a_strex.o: ../../include/openssl/safestack.h ../../include/openssl/sha.h
$(KBE_CONFIG)/a_strex.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
$(KBE_CONFIG)/a_strex.o: ../../include/openssl/x509.h ../../include/openssl/x509_vfy.h
$(KBE_CONFIG)/a_strex.o: ../cryptlib.h a_strex.c charmap.h
$(KBE_CONFIG)/a_strnid.o: ../../e_os.h ../../include/openssl/asn1.h
$(KBE_CONFIG)/a_strnid.o: ../../include/openssl/bio.h ../../include/openssl/buffer.h
$(KBE_CONFIG)/a_strnid.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(KBE_CONFIG)/a_strnid.o: ../../include/openssl/err.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/a_strnid.o: ../../include/openssl/obj_mac.h ../../include/openssl/objects.h
$(KBE_CONFIG)/a_strnid.o: ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/a_strnid.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/a_strnid.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/a_strnid.o: ../../include/openssl/symhacks.h ../cryptlib.h a_strnid.c
$(KBE_CONFIG)/a_time.o: ../../e_os.h ../../include/openssl/asn1.h
$(KBE_CONFIG)/a_time.o: ../../include/openssl/asn1t.h ../../include/openssl/bio.h
$(KBE_CONFIG)/a_time.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/a_time.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(KBE_CONFIG)/a_time.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/a_time.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/a_time.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/a_time.o: ../../include/openssl/symhacks.h ../cryptlib.h ../o_time.h a_time.c
$(KBE_CONFIG)/a_type.o: ../../e_os.h ../../include/openssl/asn1.h
$(KBE_CONFIG)/a_type.o: ../../include/openssl/asn1t.h ../../include/openssl/bio.h
$(KBE_CONFIG)/a_type.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/a_type.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(KBE_CONFIG)/a_type.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/a_type.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/a_type.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/a_type.o: ../../include/openssl/symhacks.h ../cryptlib.h a_type.c
$(KBE_CONFIG)/a_utctm.o: ../../e_os.h ../../include/openssl/asn1.h
$(KBE_CONFIG)/a_utctm.o: ../../include/openssl/bio.h ../../include/openssl/buffer.h
$(KBE_CONFIG)/a_utctm.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(KBE_CONFIG)/a_utctm.o: ../../include/openssl/err.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/a_utctm.o: ../../include/openssl/opensslconf.h ../../include/openssl/opensslv.h
$(KBE_CONFIG)/a_utctm.o: ../../include/openssl/ossl_typ.h ../../include/openssl/safestack.h
$(KBE_CONFIG)/a_utctm.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
$(KBE_CONFIG)/a_utctm.o: ../cryptlib.h ../o_time.h a_utctm.c
$(KBE_CONFIG)/a_utf8.o: ../../e_os.h ../../include/openssl/asn1.h ../../include/openssl/bio.h
$(KBE_CONFIG)/a_utf8.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/a_utf8.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(KBE_CONFIG)/a_utf8.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/a_utf8.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/a_utf8.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/a_utf8.o: ../../include/openssl/symhacks.h ../cryptlib.h a_utf8.c
$(KBE_CONFIG)/a_verify.o: ../../e_os.h ../../include/openssl/asn1.h
$(KBE_CONFIG)/a_verify.o: ../../include/openssl/bio.h ../../include/openssl/bn.h
$(KBE_CONFIG)/a_verify.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/a_verify.o: ../../include/openssl/e_os2.h ../../include/openssl/ec.h
$(KBE_CONFIG)/a_verify.o: ../../include/openssl/ecdh.h ../../include/openssl/ecdsa.h
$(KBE_CONFIG)/a_verify.o: ../../include/openssl/err.h ../../include/openssl/evp.h
$(KBE_CONFIG)/a_verify.o: ../../include/openssl/lhash.h ../../include/openssl/obj_mac.h
$(KBE_CONFIG)/a_verify.o: ../../include/openssl/objects.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/a_verify.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/a_verify.o: ../../include/openssl/pkcs7.h ../../include/openssl/safestack.h
$(KBE_CONFIG)/a_verify.o: ../../include/openssl/sha.h ../../include/openssl/stack.h
$(KBE_CONFIG)/a_verify.o: ../../include/openssl/symhacks.h ../../include/openssl/x509.h
$(KBE_CONFIG)/a_verify.o: ../../include/openssl/x509_vfy.h ../cryptlib.h a_verify.c
$(KBE_CONFIG)/asn1_err.o: ../../include/openssl/asn1.h ../../include/openssl/bio.h
$(KBE_CONFIG)/asn1_err.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(KBE_CONFIG)/asn1_err.o: ../../include/openssl/err.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/asn1_err.o: ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/asn1_err.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/asn1_err.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/asn1_err.o: ../../include/openssl/symhacks.h asn1_err.c
$(KBE_CONFIG)/asn1_gen.o: ../../e_os.h ../../include/openssl/asn1.h
$(KBE_CONFIG)/asn1_gen.o: ../../include/openssl/bio.h ../../include/openssl/buffer.h
$(KBE_CONFIG)/asn1_gen.o: ../../include/openssl/conf.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/asn1_gen.o: ../../include/openssl/e_os2.h ../../include/openssl/ec.h
$(KBE_CONFIG)/asn1_gen.o: ../../include/openssl/ecdh.h ../../include/openssl/ecdsa.h
$(KBE_CONFIG)/asn1_gen.o: ../../include/openssl/err.h ../../include/openssl/evp.h
$(KBE_CONFIG)/asn1_gen.o: ../../include/openssl/lhash.h ../../include/openssl/obj_mac.h
$(KBE_CONFIG)/asn1_gen.o: ../../include/openssl/objects.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/asn1_gen.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/asn1_gen.o: ../../include/openssl/pkcs7.h ../../include/openssl/safestack.h
$(KBE_CONFIG)/asn1_gen.o: ../../include/openssl/sha.h ../../include/openssl/stack.h
$(KBE_CONFIG)/asn1_gen.o: ../../include/openssl/symhacks.h ../../include/openssl/x509.h
$(KBE_CONFIG)/asn1_gen.o: ../../include/openssl/x509_vfy.h ../../include/openssl/x509v3.h
$(KBE_CONFIG)/asn1_gen.o: ../cryptlib.h asn1_gen.c
$(KBE_CONFIG)/asn1_lib.o: ../../e_os.h ../../include/openssl/asn1.h
$(KBE_CONFIG)/asn1_lib.o: ../../include/openssl/asn1_mac.h ../../include/openssl/bio.h
$(KBE_CONFIG)/asn1_lib.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/asn1_lib.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(KBE_CONFIG)/asn1_lib.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/asn1_lib.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/asn1_lib.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/asn1_lib.o: ../../include/openssl/symhacks.h ../cryptlib.h asn1_lib.c
$(KBE_CONFIG)/asn1_par.o: ../../e_os.h ../../include/openssl/asn1.h
$(KBE_CONFIG)/asn1_par.o: ../../include/openssl/bio.h ../../include/openssl/buffer.h
$(KBE_CONFIG)/asn1_par.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(KBE_CONFIG)/asn1_par.o: ../../include/openssl/err.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/asn1_par.o: ../../include/openssl/obj_mac.h ../../include/openssl/objects.h
$(KBE_CONFIG)/asn1_par.o: ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/asn1_par.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/asn1_par.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/asn1_par.o: ../../include/openssl/symhacks.h ../cryptlib.h asn1_par.c
$(KBE_CONFIG)/asn_moid.o: ../../e_os.h ../../include/openssl/asn1.h
$(KBE_CONFIG)/asn_moid.o: ../../include/openssl/bio.h ../../include/openssl/buffer.h
$(KBE_CONFIG)/asn_moid.o: ../../include/openssl/conf.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/asn_moid.o: ../../include/openssl/dso.h ../../include/openssl/e_os2.h
$(KBE_CONFIG)/asn_moid.o: ../../include/openssl/ec.h ../../include/openssl/ecdh.h
$(KBE_CONFIG)/asn_moid.o: ../../include/openssl/ecdsa.h ../../include/openssl/err.h
$(KBE_CONFIG)/asn_moid.o: ../../include/openssl/evp.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/asn_moid.o: ../../include/openssl/obj_mac.h ../../include/openssl/objects.h
$(KBE_CONFIG)/asn_moid.o: ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/asn_moid.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/asn_moid.o: ../../include/openssl/pkcs7.h ../../include/openssl/safestack.h
$(KBE_CONFIG)/asn_moid.o: ../../include/openssl/sha.h ../../include/openssl/stack.h
$(KBE_CONFIG)/asn_moid.o: ../../include/openssl/symhacks.h ../../include/openssl/x509.h
$(KBE_CONFIG)/asn_moid.o: ../../include/openssl/x509_vfy.h ../cryptlib.h asn_moid.c
$(KBE_CONFIG)/asn_pack.o: ../../e_os.h ../../include/openssl/asn1.h
$(KBE_CONFIG)/asn_pack.o: ../../include/openssl/bio.h ../../include/openssl/buffer.h
$(KBE_CONFIG)/asn_pack.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(KBE_CONFIG)/asn_pack.o: ../../include/openssl/err.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/asn_pack.o: ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/asn_pack.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/asn_pack.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/asn_pack.o: ../../include/openssl/symhacks.h ../cryptlib.h asn_pack.c
$(KBE_CONFIG)/d2i_pr.o: ../../e_os.h ../../include/openssl/asn1.h ../../include/openssl/bio.h
$(KBE_CONFIG)/d2i_pr.o: ../../include/openssl/bn.h ../../include/openssl/buffer.h
$(KBE_CONFIG)/d2i_pr.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(KBE_CONFIG)/d2i_pr.o: ../../include/openssl/ec.h ../../include/openssl/err.h
$(KBE_CONFIG)/d2i_pr.o: ../../include/openssl/evp.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/d2i_pr.o: ../../include/openssl/obj_mac.h ../../include/openssl/objects.h
$(KBE_CONFIG)/d2i_pr.o: ../../include/openssl/opensslconf.h ../../include/openssl/opensslv.h
$(KBE_CONFIG)/d2i_pr.o: ../../include/openssl/ossl_typ.h ../../include/openssl/rsa.h
$(KBE_CONFIG)/d2i_pr.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/d2i_pr.o: ../../include/openssl/symhacks.h ../cryptlib.h d2i_pr.c
$(KBE_CONFIG)/d2i_pu.o: ../../e_os.h ../../include/openssl/asn1.h ../../include/openssl/bio.h
$(KBE_CONFIG)/d2i_pu.o: ../../include/openssl/bn.h ../../include/openssl/buffer.h
$(KBE_CONFIG)/d2i_pu.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(KBE_CONFIG)/d2i_pu.o: ../../include/openssl/ec.h ../../include/openssl/err.h
$(KBE_CONFIG)/d2i_pu.o: ../../include/openssl/evp.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/d2i_pu.o: ../../include/openssl/obj_mac.h ../../include/openssl/objects.h
$(KBE_CONFIG)/d2i_pu.o: ../../include/openssl/opensslconf.h ../../include/openssl/opensslv.h
$(KBE_CONFIG)/d2i_pu.o: ../../include/openssl/ossl_typ.h ../../include/openssl/rsa.h
$(KBE_CONFIG)/d2i_pu.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/d2i_pu.o: ../../include/openssl/symhacks.h ../cryptlib.h d2i_pu.c
$(KBE_CONFIG)/evp_asn1.o: ../../e_os.h ../../include/openssl/asn1.h
$(KBE_CONFIG)/evp_asn1.o: ../../include/openssl/asn1_mac.h ../../include/openssl/bio.h
$(KBE_CONFIG)/evp_asn1.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/evp_asn1.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(KBE_CONFIG)/evp_asn1.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/evp_asn1.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/evp_asn1.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/evp_asn1.o: ../../include/openssl/symhacks.h ../cryptlib.h evp_asn1.c
$(KBE_CONFIG)/f_enum.o: ../../e_os.h ../../include/openssl/asn1.h ../../include/openssl/bio.h
$(KBE_CONFIG)/f_enum.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/f_enum.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(KBE_CONFIG)/f_enum.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/f_enum.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/f_enum.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/f_enum.o: ../../include/openssl/symhacks.h ../cryptlib.h f_enum.c
$(KBE_CONFIG)/f_int.o: ../../e_os.h ../../include/openssl/asn1.h ../../include/openssl/bio.h
$(KBE_CONFIG)/f_int.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/f_int.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(KBE_CONFIG)/f_int.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/f_int.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/f_int.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/f_int.o: ../../include/openssl/symhacks.h ../cryptlib.h f_int.c
$(KBE_CONFIG)/f_string.o: ../../e_os.h ../../include/openssl/asn1.h
$(KBE_CONFIG)/f_string.o: ../../include/openssl/bio.h ../../include/openssl/buffer.h
$(KBE_CONFIG)/f_string.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(KBE_CONFIG)/f_string.o: ../../include/openssl/err.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/f_string.o: ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/f_string.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/f_string.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/f_string.o: ../../include/openssl/symhacks.h ../cryptlib.h f_string.c
$(KBE_CONFIG)/i2d_pr.o: ../../e_os.h ../../include/openssl/asn1.h ../../include/openssl/bio.h
$(KBE_CONFIG)/i2d_pr.o: ../../include/openssl/bn.h ../../include/openssl/buffer.h
$(KBE_CONFIG)/i2d_pr.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(KBE_CONFIG)/i2d_pr.o: ../../include/openssl/ec.h ../../include/openssl/err.h
$(KBE_CONFIG)/i2d_pr.o: ../../include/openssl/evp.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/i2d_pr.o: ../../include/openssl/obj_mac.h ../../include/openssl/objects.h
$(KBE_CONFIG)/i2d_pr.o: ../../include/openssl/opensslconf.h ../../include/openssl/opensslv.h
$(KBE_CONFIG)/i2d_pr.o: ../../include/openssl/ossl_typ.h ../../include/openssl/rsa.h
$(KBE_CONFIG)/i2d_pr.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/i2d_pr.o: ../../include/openssl/symhacks.h ../cryptlib.h i2d_pr.c
$(KBE_CONFIG)/i2d_pu.o: ../../e_os.h ../../include/openssl/asn1.h ../../include/openssl/bio.h
$(KBE_CONFIG)/i2d_pu.o: ../../include/openssl/bn.h ../../include/openssl/buffer.h
$(KBE_CONFIG)/i2d_pu.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(KBE_CONFIG)/i2d_pu.o: ../../include/openssl/ec.h ../../include/openssl/err.h
$(KBE_CONFIG)/i2d_pu.o: ../../include/openssl/evp.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/i2d_pu.o: ../../include/openssl/obj_mac.h ../../include/openssl/objects.h
$(KBE_CONFIG)/i2d_pu.o: ../../include/openssl/opensslconf.h ../../include/openssl/opensslv.h
$(KBE_CONFIG)/i2d_pu.o: ../../include/openssl/ossl_typ.h ../../include/openssl/rsa.h
$(KBE_CONFIG)/i2d_pu.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/i2d_pu.o: ../../include/openssl/symhacks.h ../cryptlib.h i2d_pu.c
$(KBE_CONFIG)/n_pkey.o: ../../e_os.h ../../include/openssl/asn1.h
$(KBE_CONFIG)/n_pkey.o: ../../include/openssl/asn1_mac.h ../../include/openssl/asn1t.h
$(KBE_CONFIG)/n_pkey.o: ../../include/openssl/bio.h ../../include/openssl/buffer.h
$(KBE_CONFIG)/n_pkey.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(KBE_CONFIG)/n_pkey.o: ../../include/openssl/ec.h ../../include/openssl/ecdh.h
$(KBE_CONFIG)/n_pkey.o: ../../include/openssl/ecdsa.h ../../include/openssl/err.h
$(KBE_CONFIG)/n_pkey.o: ../../include/openssl/evp.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/n_pkey.o: ../../include/openssl/obj_mac.h ../../include/openssl/objects.h
$(KBE_CONFIG)/n_pkey.o: ../../include/openssl/opensslconf.h ../../include/openssl/opensslv.h
$(KBE_CONFIG)/n_pkey.o: ../../include/openssl/ossl_typ.h ../../include/openssl/pkcs7.h
$(KBE_CONFIG)/n_pkey.o: ../../include/openssl/rsa.h ../../include/openssl/safestack.h
$(KBE_CONFIG)/n_pkey.o: ../../include/openssl/sha.h ../../include/openssl/stack.h
$(KBE_CONFIG)/n_pkey.o: ../../include/openssl/symhacks.h ../../include/openssl/x509.h
$(KBE_CONFIG)/n_pkey.o: ../../include/openssl/x509_vfy.h ../cryptlib.h n_pkey.c
$(KBE_CONFIG)/nsseq.o: ../../include/openssl/asn1.h ../../include/openssl/asn1t.h
$(KBE_CONFIG)/nsseq.o: ../../include/openssl/bio.h ../../include/openssl/buffer.h
$(KBE_CONFIG)/nsseq.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(KBE_CONFIG)/nsseq.o: ../../include/openssl/ec.h ../../include/openssl/ecdh.h
$(KBE_CONFIG)/nsseq.o: ../../include/openssl/ecdsa.h ../../include/openssl/evp.h
$(KBE_CONFIG)/nsseq.o: ../../include/openssl/lhash.h ../../include/openssl/obj_mac.h
$(KBE_CONFIG)/nsseq.o: ../../include/openssl/objects.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/nsseq.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/nsseq.o: ../../include/openssl/pkcs7.h ../../include/openssl/safestack.h
$(KBE_CONFIG)/nsseq.o: ../../include/openssl/sha.h ../../include/openssl/stack.h
$(KBE_CONFIG)/nsseq.o: ../../include/openssl/symhacks.h ../../include/openssl/x509.h
$(KBE_CONFIG)/nsseq.o: ../../include/openssl/x509_vfy.h nsseq.c
$(KBE_CONFIG)/p5_pbe.o: ../../e_os.h ../../include/openssl/asn1.h
$(KBE_CONFIG)/p5_pbe.o: ../../include/openssl/asn1t.h ../../include/openssl/bio.h
$(KBE_CONFIG)/p5_pbe.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/p5_pbe.o: ../../include/openssl/e_os2.h ../../include/openssl/ec.h
$(KBE_CONFIG)/p5_pbe.o: ../../include/openssl/ecdh.h ../../include/openssl/ecdsa.h
$(KBE_CONFIG)/p5_pbe.o: ../../include/openssl/err.h ../../include/openssl/evp.h
$(KBE_CONFIG)/p5_pbe.o: ../../include/openssl/lhash.h ../../include/openssl/obj_mac.h
$(KBE_CONFIG)/p5_pbe.o: ../../include/openssl/objects.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/p5_pbe.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/p5_pbe.o: ../../include/openssl/pkcs7.h ../../include/openssl/rand.h
$(KBE_CONFIG)/p5_pbe.o: ../../include/openssl/safestack.h ../../include/openssl/sha.h
$(KBE_CONFIG)/p5_pbe.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
$(KBE_CONFIG)/p5_pbe.o: ../../include/openssl/x509.h ../../include/openssl/x509_vfy.h
$(KBE_CONFIG)/p5_pbe.o: ../cryptlib.h p5_pbe.c
$(KBE_CONFIG)/p5_pbev2.o: ../../e_os.h ../../include/openssl/asn1.h
$(KBE_CONFIG)/p5_pbev2.o: ../../include/openssl/asn1t.h ../../include/openssl/bio.h
$(KBE_CONFIG)/p5_pbev2.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/p5_pbev2.o: ../../include/openssl/e_os2.h ../../include/openssl/ec.h
$(KBE_CONFIG)/p5_pbev2.o: ../../include/openssl/ecdh.h ../../include/openssl/ecdsa.h
$(KBE_CONFIG)/p5_pbev2.o: ../../include/openssl/err.h ../../include/openssl/evp.h
$(KBE_CONFIG)/p5_pbev2.o: ../../include/openssl/lhash.h ../../include/openssl/obj_mac.h
$(KBE_CONFIG)/p5_pbev2.o: ../../include/openssl/objects.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/p5_pbev2.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/p5_pbev2.o: ../../include/openssl/pkcs7.h ../../include/openssl/rand.h
$(KBE_CONFIG)/p5_pbev2.o: ../../include/openssl/safestack.h ../../include/openssl/sha.h
$(KBE_CONFIG)/p5_pbev2.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
$(KBE_CONFIG)/p5_pbev2.o: ../../include/openssl/x509.h ../../include/openssl/x509_vfy.h
$(KBE_CONFIG)/p5_pbev2.o: ../cryptlib.h p5_pbev2.c
$(KBE_CONFIG)/p8_pkey.o: ../../e_os.h ../../include/openssl/asn1.h
$(KBE_CONFIG)/p8_pkey.o: ../../include/openssl/asn1t.h ../../include/openssl/bio.h
$(KBE_CONFIG)/p8_pkey.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/p8_pkey.o: ../../include/openssl/e_os2.h ../../include/openssl/ec.h
$(KBE_CONFIG)/p8_pkey.o: ../../include/openssl/ecdh.h ../../include/openssl/ecdsa.h
$(KBE_CONFIG)/p8_pkey.o: ../../include/openssl/err.h ../../include/openssl/evp.h
$(KBE_CONFIG)/p8_pkey.o: ../../include/openssl/lhash.h ../../include/openssl/obj_mac.h
$(KBE_CONFIG)/p8_pkey.o: ../../include/openssl/objects.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/p8_pkey.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/p8_pkey.o: ../../include/openssl/pkcs7.h ../../include/openssl/safestack.h
$(KBE_CONFIG)/p8_pkey.o: ../../include/openssl/sha.h ../../include/openssl/stack.h
$(KBE_CONFIG)/p8_pkey.o: ../../include/openssl/symhacks.h ../../include/openssl/x509.h
$(KBE_CONFIG)/p8_pkey.o: ../../include/openssl/x509_vfy.h ../cryptlib.h p8_pkey.c
$(KBE_CONFIG)/t_bitst.o: ../../e_os.h ../../include/openssl/asn1.h
$(KBE_CONFIG)/t_bitst.o: ../../include/openssl/bio.h ../../include/openssl/buffer.h
$(KBE_CONFIG)/t_bitst.o: ../../include/openssl/conf.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/t_bitst.o: ../../include/openssl/e_os2.h ../../include/openssl/ec.h
$(KBE_CONFIG)/t_bitst.o: ../../include/openssl/ecdh.h ../../include/openssl/ecdsa.h
$(KBE_CONFIG)/t_bitst.o: ../../include/openssl/err.h ../../include/openssl/evp.h
$(KBE_CONFIG)/t_bitst.o: ../../include/openssl/lhash.h ../../include/openssl/obj_mac.h
$(KBE_CONFIG)/t_bitst.o: ../../include/openssl/objects.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/t_bitst.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/t_bitst.o: ../../include/openssl/pkcs7.h ../../include/openssl/safestack.h
$(KBE_CONFIG)/t_bitst.o: ../../include/openssl/sha.h ../../include/openssl/stack.h
$(KBE_CONFIG)/t_bitst.o: ../../include/openssl/symhacks.h ../../include/openssl/x509.h
$(KBE_CONFIG)/t_bitst.o: ../../include/openssl/x509_vfy.h ../../include/openssl/x509v3.h
$(KBE_CONFIG)/t_bitst.o: ../cryptlib.h t_bitst.c
$(KBE_CONFIG)/t_crl.o: ../../e_os.h ../../include/openssl/asn1.h ../../include/openssl/bio.h
$(KBE_CONFIG)/t_crl.o: ../../include/openssl/bn.h ../../include/openssl/buffer.h
$(KBE_CONFIG)/t_crl.o: ../../include/openssl/conf.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/t_crl.o: ../../include/openssl/e_os2.h ../../include/openssl/ec.h
$(KBE_CONFIG)/t_crl.o: ../../include/openssl/ecdh.h ../../include/openssl/ecdsa.h
$(KBE_CONFIG)/t_crl.o: ../../include/openssl/err.h ../../include/openssl/evp.h
$(KBE_CONFIG)/t_crl.o: ../../include/openssl/lhash.h ../../include/openssl/obj_mac.h
$(KBE_CONFIG)/t_crl.o: ../../include/openssl/objects.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/t_crl.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/t_crl.o: ../../include/openssl/pkcs7.h ../../include/openssl/safestack.h
$(KBE_CONFIG)/t_crl.o: ../../include/openssl/sha.h ../../include/openssl/stack.h
$(KBE_CONFIG)/t_crl.o: ../../include/openssl/symhacks.h ../../include/openssl/x509.h
$(KBE_CONFIG)/t_crl.o: ../../include/openssl/x509_vfy.h ../../include/openssl/x509v3.h
$(KBE_CONFIG)/t_crl.o: ../cryptlib.h t_crl.c
$(KBE_CONFIG)/t_pkey.o: ../../e_os.h ../../include/openssl/asn1.h ../../include/openssl/bio.h
$(KBE_CONFIG)/t_pkey.o: ../../include/openssl/bn.h ../../include/openssl/buffer.h
$(KBE_CONFIG)/t_pkey.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(KBE_CONFIG)/t_pkey.o: ../../include/openssl/ec.h ../../include/openssl/err.h
$(KBE_CONFIG)/t_pkey.o: ../../include/openssl/lhash.h ../../include/openssl/obj_mac.h
$(KBE_CONFIG)/t_pkey.o: ../../include/openssl/objects.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/t_pkey.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/t_pkey.o: ../../include/openssl/rsa.h ../../include/openssl/safestack.h
$(KBE_CONFIG)/t_pkey.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
$(KBE_CONFIG)/t_pkey.o: ../cryptlib.h t_pkey.c
$(KBE_CONFIG)/t_req.o: ../../e_os.h ../../include/openssl/asn1.h ../../include/openssl/bio.h
$(KBE_CONFIG)/t_req.o: ../../include/openssl/bn.h ../../include/openssl/buffer.h
$(KBE_CONFIG)/t_req.o: ../../include/openssl/conf.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/t_req.o: ../../include/openssl/e_os2.h ../../include/openssl/ec.h
$(KBE_CONFIG)/t_req.o: ../../include/openssl/ecdh.h ../../include/openssl/ecdsa.h
$(KBE_CONFIG)/t_req.o: ../../include/openssl/err.h ../../include/openssl/evp.h
$(KBE_CONFIG)/t_req.o: ../../include/openssl/lhash.h ../../include/openssl/obj_mac.h
$(KBE_CONFIG)/t_req.o: ../../include/openssl/objects.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/t_req.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/t_req.o: ../../include/openssl/pkcs7.h ../../include/openssl/rsa.h
$(KBE_CONFIG)/t_req.o: ../../include/openssl/safestack.h ../../include/openssl/sha.h
$(KBE_CONFIG)/t_req.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
$(KBE_CONFIG)/t_req.o: ../../include/openssl/x509.h ../../include/openssl/x509_vfy.h
$(KBE_CONFIG)/t_req.o: ../../include/openssl/x509v3.h ../cryptlib.h t_req.c
$(KBE_CONFIG)/t_spki.o: ../../e_os.h ../../include/openssl/asn1.h ../../include/openssl/bio.h
$(KBE_CONFIG)/t_spki.o: ../../include/openssl/bn.h ../../include/openssl/buffer.h
$(KBE_CONFIG)/t_spki.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(KBE_CONFIG)/t_spki.o: ../../include/openssl/ec.h ../../include/openssl/ecdh.h
$(KBE_CONFIG)/t_spki.o: ../../include/openssl/ecdsa.h ../../include/openssl/err.h
$(KBE_CONFIG)/t_spki.o: ../../include/openssl/evp.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/t_spki.o: ../../include/openssl/obj_mac.h ../../include/openssl/objects.h
$(KBE_CONFIG)/t_spki.o: ../../include/openssl/opensslconf.h ../../include/openssl/opensslv.h
$(KBE_CONFIG)/t_spki.o: ../../include/openssl/ossl_typ.h ../../include/openssl/pkcs7.h
$(KBE_CONFIG)/t_spki.o: ../../include/openssl/rsa.h ../../include/openssl/safestack.h
$(KBE_CONFIG)/t_spki.o: ../../include/openssl/sha.h ../../include/openssl/stack.h
$(KBE_CONFIG)/t_spki.o: ../../include/openssl/symhacks.h ../../include/openssl/x509.h
$(KBE_CONFIG)/t_spki.o: ../../include/openssl/x509_vfy.h ../cryptlib.h t_spki.c
$(KBE_CONFIG)/t_x509.o: ../../e_os.h ../../include/openssl/asn1.h ../../include/openssl/bio.h
$(KBE_CONFIG)/t_x509.o: ../../include/openssl/bn.h ../../include/openssl/buffer.h
$(KBE_CONFIG)/t_x509.o: ../../include/openssl/conf.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/t_x509.o: ../../include/openssl/e_os2.h ../../include/openssl/ec.h
$(KBE_CONFIG)/t_x509.o: ../../include/openssl/ecdh.h ../../include/openssl/ecdsa.h
$(KBE_CONFIG)/t_x509.o: ../../include/openssl/err.h ../../include/openssl/evp.h
$(KBE_CONFIG)/t_x509.o: ../../include/openssl/lhash.h ../../include/openssl/obj_mac.h
$(KBE_CONFIG)/t_x509.o: ../../include/openssl/objects.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/t_x509.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/t_x509.o: ../../include/openssl/pkcs7.h ../../include/openssl/rsa.h
$(KBE_CONFIG)/t_x509.o: ../../include/openssl/safestack.h ../../include/openssl/sha.h
$(KBE_CONFIG)/t_x509.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
$(KBE_CONFIG)/t_x509.o: ../../include/openssl/x509.h ../../include/openssl/x509_vfy.h
$(KBE_CONFIG)/t_x509.o: ../../include/openssl/x509v3.h ../cryptlib.h t_x509.c
$(KBE_CONFIG)/t_x509a.o: ../../e_os.h ../../include/openssl/asn1.h
$(KBE_CONFIG)/t_x509a.o: ../../include/openssl/bio.h ../../include/openssl/buffer.h
$(KBE_CONFIG)/t_x509a.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(KBE_CONFIG)/t_x509a.o: ../../include/openssl/ec.h ../../include/openssl/ecdh.h
$(KBE_CONFIG)/t_x509a.o: ../../include/openssl/ecdsa.h ../../include/openssl/err.h
$(KBE_CONFIG)/t_x509a.o: ../../include/openssl/evp.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/t_x509a.o: ../../include/openssl/obj_mac.h ../../include/openssl/objects.h
$(KBE_CONFIG)/t_x509a.o: ../../include/openssl/opensslconf.h ../../include/openssl/opensslv.h
$(KBE_CONFIG)/t_x509a.o: ../../include/openssl/ossl_typ.h ../../include/openssl/pkcs7.h
$(KBE_CONFIG)/t_x509a.o: ../../include/openssl/safestack.h ../../include/openssl/sha.h
$(KBE_CONFIG)/t_x509a.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
$(KBE_CONFIG)/t_x509a.o: ../../include/openssl/x509.h ../../include/openssl/x509_vfy.h
$(KBE_CONFIG)/t_x509a.o: ../cryptlib.h t_x509a.c
$(KBE_CONFIG)/tasn_dec.o: ../../include/openssl/asn1.h ../../include/openssl/asn1t.h
$(KBE_CONFIG)/tasn_dec.o: ../../include/openssl/bio.h ../../include/openssl/buffer.h
$(KBE_CONFIG)/tasn_dec.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(KBE_CONFIG)/tasn_dec.o: ../../include/openssl/err.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/tasn_dec.o: ../../include/openssl/obj_mac.h ../../include/openssl/objects.h
$(KBE_CONFIG)/tasn_dec.o: ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/tasn_dec.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/tasn_dec.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/tasn_dec.o: ../../include/openssl/symhacks.h tasn_dec.c
$(KBE_CONFIG)/tasn_enc.o: ../../e_os.h ../../include/openssl/asn1.h
$(KBE_CONFIG)/tasn_enc.o: ../../include/openssl/asn1t.h ../../include/openssl/bio.h
$(KBE_CONFIG)/tasn_enc.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/tasn_enc.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(KBE_CONFIG)/tasn_enc.o: ../../include/openssl/lhash.h ../../include/openssl/obj_mac.h
$(KBE_CONFIG)/tasn_enc.o: ../../include/openssl/objects.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/tasn_enc.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/tasn_enc.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/tasn_enc.o: ../../include/openssl/symhacks.h ../cryptlib.h tasn_enc.c
$(KBE_CONFIG)/tasn_fre.o: ../../include/openssl/asn1.h ../../include/openssl/asn1t.h
$(KBE_CONFIG)/tasn_fre.o: ../../include/openssl/bio.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/tasn_fre.o: ../../include/openssl/e_os2.h ../../include/openssl/obj_mac.h
$(KBE_CONFIG)/tasn_fre.o: ../../include/openssl/objects.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/tasn_fre.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/tasn_fre.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/tasn_fre.o: ../../include/openssl/symhacks.h tasn_fre.c
$(KBE_CONFIG)/tasn_new.o: ../../include/openssl/asn1.h ../../include/openssl/asn1t.h
$(KBE_CONFIG)/tasn_new.o: ../../include/openssl/bio.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/tasn_new.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(KBE_CONFIG)/tasn_new.o: ../../include/openssl/lhash.h ../../include/openssl/obj_mac.h
$(KBE_CONFIG)/tasn_new.o: ../../include/openssl/objects.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/tasn_new.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/tasn_new.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/tasn_new.o: ../../include/openssl/symhacks.h tasn_new.c
$(KBE_CONFIG)/tasn_typ.o: ../../include/openssl/asn1.h ../../include/openssl/asn1t.h
$(KBE_CONFIG)/tasn_typ.o: ../../include/openssl/bio.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/tasn_typ.o: ../../include/openssl/e_os2.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/tasn_typ.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/tasn_typ.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/tasn_typ.o: ../../include/openssl/symhacks.h tasn_typ.c
$(KBE_CONFIG)/tasn_utl.o: ../../include/openssl/asn1.h ../../include/openssl/asn1t.h
$(KBE_CONFIG)/tasn_utl.o: ../../include/openssl/bio.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/tasn_utl.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(KBE_CONFIG)/tasn_utl.o: ../../include/openssl/lhash.h ../../include/openssl/obj_mac.h
$(KBE_CONFIG)/tasn_utl.o: ../../include/openssl/objects.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/tasn_utl.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/tasn_utl.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/tasn_utl.o: ../../include/openssl/symhacks.h tasn_utl.c
$(KBE_CONFIG)/x_algor.o: ../../include/openssl/asn1.h ../../include/openssl/asn1t.h
$(KBE_CONFIG)/x_algor.o: ../../include/openssl/bio.h ../../include/openssl/buffer.h
$(KBE_CONFIG)/x_algor.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(KBE_CONFIG)/x_algor.o: ../../include/openssl/ec.h ../../include/openssl/ecdh.h
$(KBE_CONFIG)/x_algor.o: ../../include/openssl/ecdsa.h ../../include/openssl/evp.h
$(KBE_CONFIG)/x_algor.o: ../../include/openssl/lhash.h ../../include/openssl/obj_mac.h
$(KBE_CONFIG)/x_algor.o: ../../include/openssl/objects.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/x_algor.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/x_algor.o: ../../include/openssl/pkcs7.h ../../include/openssl/safestack.h
$(KBE_CONFIG)/x_algor.o: ../../include/openssl/sha.h ../../include/openssl/stack.h
$(KBE_CONFIG)/x_algor.o: ../../include/openssl/symhacks.h ../../include/openssl/x509.h
$(KBE_CONFIG)/x_algor.o: ../../include/openssl/x509_vfy.h x_algor.c
$(KBE_CONFIG)/x_attrib.o: ../../e_os.h ../../include/openssl/asn1.h
$(KBE_CONFIG)/x_attrib.o: ../../include/openssl/asn1t.h ../../include/openssl/bio.h
$(KBE_CONFIG)/x_attrib.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/x_attrib.o: ../../include/openssl/e_os2.h ../../include/openssl/ec.h
$(KBE_CONFIG)/x_attrib.o: ../../include/openssl/ecdh.h ../../include/openssl/ecdsa.h
$(KBE_CONFIG)/x_attrib.o: ../../include/openssl/err.h ../../include/openssl/evp.h
$(KBE_CONFIG)/x_attrib.o: ../../include/openssl/lhash.h ../../include/openssl/obj_mac.h
$(KBE_CONFIG)/x_attrib.o: ../../include/openssl/objects.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/x_attrib.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/x_attrib.o: ../../include/openssl/pkcs7.h ../../include/openssl/safestack.h
$(KBE_CONFIG)/x_attrib.o: ../../include/openssl/sha.h ../../include/openssl/stack.h
$(KBE_CONFIG)/x_attrib.o: ../../include/openssl/symhacks.h ../../include/openssl/x509.h
$(KBE_CONFIG)/x_attrib.o: ../../include/openssl/x509_vfy.h ../cryptlib.h x_attrib.c
$(KBE_CONFIG)/x_bignum.o: ../../e_os.h ../../include/openssl/asn1.h
$(KBE_CONFIG)/x_bignum.o: ../../include/openssl/asn1t.h ../../include/openssl/bio.h
$(KBE_CONFIG)/x_bignum.o: ../../include/openssl/bn.h ../../include/openssl/buffer.h
$(KBE_CONFIG)/x_bignum.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(KBE_CONFIG)/x_bignum.o: ../../include/openssl/err.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/x_bignum.o: ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/x_bignum.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/x_bignum.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/x_bignum.o: ../../include/openssl/symhacks.h ../cryptlib.h x_bignum.c
$(KBE_CONFIG)/x_crl.o: ../../e_os.h ../../include/openssl/asn1.h
$(KBE_CONFIG)/x_crl.o: ../../include/openssl/asn1t.h ../../include/openssl/bio.h
$(KBE_CONFIG)/x_crl.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/x_crl.o: ../../include/openssl/e_os2.h ../../include/openssl/ec.h
$(KBE_CONFIG)/x_crl.o: ../../include/openssl/ecdh.h ../../include/openssl/ecdsa.h
$(KBE_CONFIG)/x_crl.o: ../../include/openssl/err.h ../../include/openssl/evp.h
$(KBE_CONFIG)/x_crl.o: ../../include/openssl/lhash.h ../../include/openssl/obj_mac.h
$(KBE_CONFIG)/x_crl.o: ../../include/openssl/objects.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/x_crl.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/x_crl.o: ../../include/openssl/pkcs7.h ../../include/openssl/safestack.h
$(KBE_CONFIG)/x_crl.o: ../../include/openssl/sha.h ../../include/openssl/stack.h
$(KBE_CONFIG)/x_crl.o: ../../include/openssl/symhacks.h ../../include/openssl/x509.h
$(KBE_CONFIG)/x_crl.o: ../../include/openssl/x509_vfy.h ../cryptlib.h x_crl.c
$(KBE_CONFIG)/x_exten.o: ../../include/openssl/asn1.h ../../include/openssl/asn1t.h
$(KBE_CONFIG)/x_exten.o: ../../include/openssl/bio.h ../../include/openssl/buffer.h
$(KBE_CONFIG)/x_exten.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(KBE_CONFIG)/x_exten.o: ../../include/openssl/ec.h ../../include/openssl/ecdh.h
$(KBE_CONFIG)/x_exten.o: ../../include/openssl/ecdsa.h ../../include/openssl/evp.h
$(KBE_CONFIG)/x_exten.o: ../../include/openssl/lhash.h ../../include/openssl/obj_mac.h
$(KBE_CONFIG)/x_exten.o: ../../include/openssl/objects.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/x_exten.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/x_exten.o: ../../include/openssl/pkcs7.h ../../include/openssl/safestack.h
$(KBE_CONFIG)/x_exten.o: ../../include/openssl/sha.h ../../include/openssl/stack.h
$(KBE_CONFIG)/x_exten.o: ../../include/openssl/symhacks.h ../../include/openssl/x509.h
$(KBE_CONFIG)/x_exten.o: ../../include/openssl/x509_vfy.h x_exten.c
$(KBE_CONFIG)/x_info.o: ../../e_os.h ../../include/openssl/asn1.h ../../include/openssl/bio.h
$(KBE_CONFIG)/x_info.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/x_info.o: ../../include/openssl/e_os2.h ../../include/openssl/ec.h
$(KBE_CONFIG)/x_info.o: ../../include/openssl/ecdh.h ../../include/openssl/ecdsa.h
$(KBE_CONFIG)/x_info.o: ../../include/openssl/err.h ../../include/openssl/evp.h
$(KBE_CONFIG)/x_info.o: ../../include/openssl/lhash.h ../../include/openssl/obj_mac.h
$(KBE_CONFIG)/x_info.o: ../../include/openssl/objects.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/x_info.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/x_info.o: ../../include/openssl/pkcs7.h ../../include/openssl/safestack.h
$(KBE_CONFIG)/x_info.o: ../../include/openssl/sha.h ../../include/openssl/stack.h
$(KBE_CONFIG)/x_info.o: ../../include/openssl/symhacks.h ../../include/openssl/x509.h
$(KBE_CONFIG)/x_info.o: ../../include/openssl/x509_vfy.h ../cryptlib.h x_info.c
$(KBE_CONFIG)/x_long.o: ../../e_os.h ../../include/openssl/asn1.h
$(KBE_CONFIG)/x_long.o: ../../include/openssl/asn1t.h ../../include/openssl/bio.h
$(KBE_CONFIG)/x_long.o: ../../include/openssl/bn.h ../../include/openssl/buffer.h
$(KBE_CONFIG)/x_long.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(KBE_CONFIG)/x_long.o: ../../include/openssl/err.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/x_long.o: ../../include/openssl/opensslconf.h ../../include/openssl/opensslv.h
$(KBE_CONFIG)/x_long.o: ../../include/openssl/ossl_typ.h ../../include/openssl/safestack.h
$(KBE_CONFIG)/x_long.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
$(KBE_CONFIG)/x_long.o: ../cryptlib.h x_long.c
$(KBE_CONFIG)/x_name.o: ../../e_os.h ../../include/openssl/asn1.h
$(KBE_CONFIG)/x_name.o: ../../include/openssl/asn1t.h ../../include/openssl/bio.h
$(KBE_CONFIG)/x_name.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/x_name.o: ../../include/openssl/e_os2.h ../../include/openssl/ec.h
$(KBE_CONFIG)/x_name.o: ../../include/openssl/ecdh.h ../../include/openssl/ecdsa.h
$(KBE_CONFIG)/x_name.o: ../../include/openssl/err.h ../../include/openssl/evp.h
$(KBE_CONFIG)/x_name.o: ../../include/openssl/lhash.h ../../include/openssl/obj_mac.h
$(KBE_CONFIG)/x_name.o: ../../include/openssl/objects.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/x_name.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/x_name.o: ../../include/openssl/pkcs7.h ../../include/openssl/safestack.h
$(KBE_CONFIG)/x_name.o: ../../include/openssl/sha.h ../../include/openssl/stack.h
$(KBE_CONFIG)/x_name.o: ../../include/openssl/symhacks.h ../../include/openssl/x509.h
$(KBE_CONFIG)/x_name.o: ../../include/openssl/x509_vfy.h ../cryptlib.h x_name.c
$(KBE_CONFIG)/x_pkey.o: ../../e_os.h ../../include/openssl/asn1.h
$(KBE_CONFIG)/x_pkey.o: ../../include/openssl/asn1_mac.h ../../include/openssl/bio.h
$(KBE_CONFIG)/x_pkey.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/x_pkey.o: ../../include/openssl/e_os2.h ../../include/openssl/ec.h
$(KBE_CONFIG)/x_pkey.o: ../../include/openssl/ecdh.h ../../include/openssl/ecdsa.h
$(KBE_CONFIG)/x_pkey.o: ../../include/openssl/err.h ../../include/openssl/evp.h
$(KBE_CONFIG)/x_pkey.o: ../../include/openssl/lhash.h ../../include/openssl/obj_mac.h
$(KBE_CONFIG)/x_pkey.o: ../../include/openssl/objects.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/x_pkey.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/x_pkey.o: ../../include/openssl/pkcs7.h ../../include/openssl/safestack.h
$(KBE_CONFIG)/x_pkey.o: ../../include/openssl/sha.h ../../include/openssl/stack.h
$(KBE_CONFIG)/x_pkey.o: ../../include/openssl/symhacks.h ../../include/openssl/x509.h
$(KBE_CONFIG)/x_pkey.o: ../../include/openssl/x509_vfy.h ../cryptlib.h x_pkey.c
$(KBE_CONFIG)/x_pubkey.o: ../../e_os.h ../../include/openssl/asn1.h
$(KBE_CONFIG)/x_pubkey.o: ../../include/openssl/asn1t.h ../../include/openssl/bio.h
$(KBE_CONFIG)/x_pubkey.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/x_pubkey.o: ../../include/openssl/e_os2.h ../../include/openssl/ec.h
$(KBE_CONFIG)/x_pubkey.o: ../../include/openssl/ecdh.h ../../include/openssl/ecdsa.h
$(KBE_CONFIG)/x_pubkey.o: ../../include/openssl/err.h ../../include/openssl/evp.h
$(KBE_CONFIG)/x_pubkey.o: ../../include/openssl/lhash.h ../../include/openssl/obj_mac.h
$(KBE_CONFIG)/x_pubkey.o: ../../include/openssl/objects.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/x_pubkey.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/x_pubkey.o: ../../include/openssl/pkcs7.h ../../include/openssl/rsa.h
$(KBE_CONFIG)/x_pubkey.o: ../../include/openssl/safestack.h ../../include/openssl/sha.h
$(KBE_CONFIG)/x_pubkey.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
$(KBE_CONFIG)/x_pubkey.o: ../../include/openssl/x509.h ../../include/openssl/x509_vfy.h
$(KBE_CONFIG)/x_pubkey.o: ../cryptlib.h x_pubkey.c
$(KBE_CONFIG)/x_req.o: ../../e_os.h ../../include/openssl/asn1.h
$(KBE_CONFIG)/x_req.o: ../../include/openssl/asn1t.h ../../include/openssl/bio.h
$(KBE_CONFIG)/x_req.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/x_req.o: ../../include/openssl/e_os2.h ../../include/openssl/ec.h
$(KBE_CONFIG)/x_req.o: ../../include/openssl/ecdh.h ../../include/openssl/ecdsa.h
$(KBE_CONFIG)/x_req.o: ../../include/openssl/err.h ../../include/openssl/evp.h
$(KBE_CONFIG)/x_req.o: ../../include/openssl/lhash.h ../../include/openssl/obj_mac.h
$(KBE_CONFIG)/x_req.o: ../../include/openssl/objects.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/x_req.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/x_req.o: ../../include/openssl/pkcs7.h ../../include/openssl/safestack.h
$(KBE_CONFIG)/x_req.o: ../../include/openssl/sha.h ../../include/openssl/stack.h
$(KBE_CONFIG)/x_req.o: ../../include/openssl/symhacks.h ../../include/openssl/x509.h
$(KBE_CONFIG)/x_req.o: ../../include/openssl/x509_vfy.h ../cryptlib.h x_req.c
$(KBE_CONFIG)/x_sig.o: ../../e_os.h ../../include/openssl/asn1.h
$(KBE_CONFIG)/x_sig.o: ../../include/openssl/asn1t.h ../../include/openssl/bio.h
$(KBE_CONFIG)/x_sig.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/x_sig.o: ../../include/openssl/e_os2.h ../../include/openssl/ec.h
$(KBE_CONFIG)/x_sig.o: ../../include/openssl/ecdh.h ../../include/openssl/ecdsa.h
$(KBE_CONFIG)/x_sig.o: ../../include/openssl/err.h ../../include/openssl/evp.h
$(KBE_CONFIG)/x_sig.o: ../../include/openssl/lhash.h ../../include/openssl/obj_mac.h
$(KBE_CONFIG)/x_sig.o: ../../include/openssl/objects.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/x_sig.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/x_sig.o: ../../include/openssl/pkcs7.h ../../include/openssl/safestack.h
$(KBE_CONFIG)/x_sig.o: ../../include/openssl/sha.h ../../include/openssl/stack.h
$(KBE_CONFIG)/x_sig.o: ../../include/openssl/symhacks.h ../../include/openssl/x509.h
$(KBE_CONFIG)/x_sig.o: ../../include/openssl/x509_vfy.h ../cryptlib.h x_sig.c
$(KBE_CONFIG)/x_spki.o: ../../e_os.h ../../include/openssl/asn1.h
$(KBE_CONFIG)/x_spki.o: ../../include/openssl/asn1t.h ../../include/openssl/bio.h
$(KBE_CONFIG)/x_spki.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/x_spki.o: ../../include/openssl/e_os2.h ../../include/openssl/ec.h
$(KBE_CONFIG)/x_spki.o: ../../include/openssl/ecdh.h ../../include/openssl/ecdsa.h
$(KBE_CONFIG)/x_spki.o: ../../include/openssl/err.h ../../include/openssl/evp.h
$(KBE_CONFIG)/x_spki.o: ../../include/openssl/lhash.h ../../include/openssl/obj_mac.h
$(KBE_CONFIG)/x_spki.o: ../../include/openssl/objects.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/x_spki.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/x_spki.o: ../../include/openssl/pkcs7.h ../../include/openssl/safestack.h
$(KBE_CONFIG)/x_spki.o: ../../include/openssl/sha.h ../../include/openssl/stack.h
$(KBE_CONFIG)/x_spki.o: ../../include/openssl/symhacks.h ../../include/openssl/x509.h
$(KBE_CONFIG)/x_spki.o: ../../include/openssl/x509_vfy.h ../cryptlib.h x_spki.c
$(KBE_CONFIG)/x_val.o: ../../e_os.h ../../include/openssl/asn1.h
$(KBE_CONFIG)/x_val.o: ../../include/openssl/asn1t.h ../../include/openssl/bio.h
$(KBE_CONFIG)/x_val.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/x_val.o: ../../include/openssl/e_os2.h ../../include/openssl/ec.h
$(KBE_CONFIG)/x_val.o: ../../include/openssl/ecdh.h ../../include/openssl/ecdsa.h
$(KBE_CONFIG)/x_val.o: ../../include/openssl/err.h ../../include/openssl/evp.h
$(KBE_CONFIG)/x_val.o: ../../include/openssl/lhash.h ../../include/openssl/obj_mac.h
$(KBE_CONFIG)/x_val.o: ../../include/openssl/objects.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/x_val.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/x_val.o: ../../include/openssl/pkcs7.h ../../include/openssl/safestack.h
$(KBE_CONFIG)/x_val.o: ../../include/openssl/sha.h ../../include/openssl/stack.h
$(KBE_CONFIG)/x_val.o: ../../include/openssl/symhacks.h ../../include/openssl/x509.h
$(KBE_CONFIG)/x_val.o: ../../include/openssl/x509_vfy.h ../cryptlib.h x_val.c
$(KBE_CONFIG)/x_x509.o: ../../e_os.h ../../include/openssl/asn1.h
$(KBE_CONFIG)/x_x509.o: ../../include/openssl/asn1t.h ../../include/openssl/bio.h
$(KBE_CONFIG)/x_x509.o: ../../include/openssl/buffer.h ../../include/openssl/conf.h
$(KBE_CONFIG)/x_x509.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(KBE_CONFIG)/x_x509.o: ../../include/openssl/ec.h ../../include/openssl/ecdh.h
$(KBE_CONFIG)/x_x509.o: ../../include/openssl/ecdsa.h ../../include/openssl/err.h
$(KBE_CONFIG)/x_x509.o: ../../include/openssl/evp.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/x_x509.o: ../../include/openssl/obj_mac.h ../../include/openssl/objects.h
$(KBE_CONFIG)/x_x509.o: ../../include/openssl/opensslconf.h ../../include/openssl/opensslv.h
$(KBE_CONFIG)/x_x509.o: ../../include/openssl/ossl_typ.h ../../include/openssl/pkcs7.h
$(KBE_CONFIG)/x_x509.o: ../../include/openssl/safestack.h ../../include/openssl/sha.h
$(KBE_CONFIG)/x_x509.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
$(KBE_CONFIG)/x_x509.o: ../../include/openssl/x509.h ../../include/openssl/x509_vfy.h
$(KBE_CONFIG)/x_x509.o: ../../include/openssl/x509v3.h ../cryptlib.h x_x509.c
$(KBE_CONFIG)/x_x509a.o: ../../e_os.h ../../include/openssl/asn1.h
$(KBE_CONFIG)/x_x509a.o: ../../include/openssl/asn1t.h ../../include/openssl/bio.h
$(KBE_CONFIG)/x_x509a.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/x_x509a.o: ../../include/openssl/e_os2.h ../../include/openssl/ec.h
$(KBE_CONFIG)/x_x509a.o: ../../include/openssl/ecdh.h ../../include/openssl/ecdsa.h
$(KBE_CONFIG)/x_x509a.o: ../../include/openssl/err.h ../../include/openssl/evp.h
$(KBE_CONFIG)/x_x509a.o: ../../include/openssl/lhash.h ../../include/openssl/obj_mac.h
$(KBE_CONFIG)/x_x509a.o: ../../include/openssl/objects.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/x_x509a.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/x_x509a.o: ../../include/openssl/pkcs7.h ../../include/openssl/safestack.h
$(KBE_CONFIG)/x_x509a.o: ../../include/openssl/sha.h ../../include/openssl/stack.h
$(KBE_CONFIG)/x_x509a.o: ../../include/openssl/symhacks.h ../../include/openssl/x509.h
$(KBE_CONFIG)/x_x509a.o: ../../include/openssl/x509_vfy.h ../cryptlib.h x_x509a.c
