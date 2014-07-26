#
# OpenSSL/crypto/pkcs12/Makefile
#

ifndef KBE_ROOT
export KBE_ROOT := $(subst /kbe/src/lib/dependencies/openssl/crypto/pkcs12,,$(CURDIR))
endif

LIBDIR= $(KBE_ROOT)/kbe/src/libs


DIR=	pkcs12
TOP=	../..
CC=	cc
INCLUDES= -I.. -I$(TOP) -I../../include
CFLAG=-g
MAKEFILE=	Makefile
AR=		ar r

CFLAGS= $(INCLUDES) $(CFLAG)

GENERAL=Makefile
TEST=
APPS=

LIB=$(LIBDIR)/libcrypto.a
LIBSRC= p12_add.c p12_asn.c p12_attr.c p12_crpt.c p12_crt.c p12_decr.c \
	p12_init.c p12_key.c p12_kiss.c p12_mutl.c\
	p12_utl.c p12_npas.c pk12err.c p12_p8d.c p12_p8e.c
LIBOBJ= $(KBE_CONFIG)/p12_add.o $(KBE_CONFIG)/p12_asn.o $(KBE_CONFIG)/p12_attr.o \
	$(KBE_CONFIG)/p12_crpt.o $(KBE_CONFIG)/p12_crt.o $(KBE_CONFIG)/p12_decr.o \
	$(KBE_CONFIG)/p12_init.o $(KBE_CONFIG)/p12_key.o $(KBE_CONFIG)/p12_kiss.o \
	$(KBE_CONFIG)/p12_mutl.o	$(KBE_CONFIG)/p12_utl.o $(KBE_CONFIG)/p12_npas.o \
	$(KBE_CONFIG)/pk12err.o $(KBE_CONFIG)/p12_p8d.o $(KBE_CONFIG)/p12_p8e.o

SRC= $(LIBSRC)

EXHEADER=  pkcs12.h
HEADER=	$(EXHEADER)

ALL=    $(GENERAL) $(SRC) $(HEADER)

top:
	(cd ../..; $(MAKE) DIRS=crypto SDIRS=$(DIR) sub_all)

test:

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
	@[ -n "$(MAKEDEPEND)" ] # should be set by upper Makefile...
	$(MAKEDEPEND) -- $(CFLAG) $(INCLUDES) $(DEPFLAG) -- $(PROGS) $(LIBSRC)

dclean:
	$(PERL) -pe 'if (/^# DO NOT DELETE THIS LINE/) {print; exit(0);}' $(MAKEFILE) >Makefile.new
	mv -f Makefile.new $(MAKEFILE)

clean:
	rm -f $(KBE_CONFIG)/*.o *.o *.obj lib tags core .pure .nfs* *.old *.bak fluff

# DO NOT DELETE THIS LINE -- make depend depends on it.

$(KBE_CONFIG)/p12_add.o: ../../e_os.h ../../include/openssl/asn1.h
$(KBE_CONFIG)/p12_add.o: ../../include/openssl/bio.h ../../include/openssl/buffer.h
$(KBE_CONFIG)/p12_add.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(KBE_CONFIG)/p12_add.o: ../../include/openssl/ec.h ../../include/openssl/ecdh.h
$(KBE_CONFIG)/p12_add.o: ../../include/openssl/ecdsa.h ../../include/openssl/err.h
$(KBE_CONFIG)/p12_add.o: ../../include/openssl/evp.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/p12_add.o: ../../include/openssl/obj_mac.h ../../include/openssl/objects.h
$(KBE_CONFIG)/p12_add.o: ../../include/openssl/opensslconf.h ../../include/openssl/opensslv.h
$(KBE_CONFIG)/p12_add.o: ../../include/openssl/ossl_typ.h ../../include/openssl/pkcs12.h
$(KBE_CONFIG)/p12_add.o: ../../include/openssl/pkcs7.h ../../include/openssl/safestack.h
$(KBE_CONFIG)/p12_add.o: ../../include/openssl/sha.h ../../include/openssl/stack.h
$(KBE_CONFIG)/p12_add.o: ../../include/openssl/symhacks.h ../../include/openssl/x509.h
$(KBE_CONFIG)/p12_add.o: ../../include/openssl/x509_vfy.h ../cryptlib.h p12_add.c
$(KBE_CONFIG)/p12_asn.o: ../../e_os.h ../../include/openssl/asn1.h
$(KBE_CONFIG)/p12_asn.o: ../../include/openssl/asn1t.h ../../include/openssl/bio.h
$(KBE_CONFIG)/p12_asn.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/p12_asn.o: ../../include/openssl/e_os2.h ../../include/openssl/ec.h
$(KBE_CONFIG)/p12_asn.o: ../../include/openssl/ecdh.h ../../include/openssl/ecdsa.h
$(KBE_CONFIG)/p12_asn.o: ../../include/openssl/err.h ../../include/openssl/evp.h
$(KBE_CONFIG)/p12_asn.o: ../../include/openssl/lhash.h ../../include/openssl/obj_mac.h
$(KBE_CONFIG)/p12_asn.o: ../../include/openssl/objects.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/p12_asn.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/p12_asn.o: ../../include/openssl/pkcs12.h ../../include/openssl/pkcs7.h
$(KBE_CONFIG)/p12_asn.o: ../../include/openssl/safestack.h ../../include/openssl/sha.h
$(KBE_CONFIG)/p12_asn.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
$(KBE_CONFIG)/p12_asn.o: ../../include/openssl/x509.h ../../include/openssl/x509_vfy.h
$(KBE_CONFIG)/p12_asn.o: ../cryptlib.h p12_asn.c
$(KBE_CONFIG)/p12_attr.o: ../../e_os.h ../../include/openssl/asn1.h
$(KBE_CONFIG)/p12_attr.o: ../../include/openssl/bio.h ../../include/openssl/buffer.h
$(KBE_CONFIG)/p12_attr.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(KBE_CONFIG)/p12_attr.o: ../../include/openssl/ec.h ../../include/openssl/ecdh.h
$(KBE_CONFIG)/p12_attr.o: ../../include/openssl/ecdsa.h ../../include/openssl/err.h
$(KBE_CONFIG)/p12_attr.o: ../../include/openssl/evp.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/p12_attr.o: ../../include/openssl/obj_mac.h ../../include/openssl/objects.h
$(KBE_CONFIG)/p12_attr.o: ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/p12_attr.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/p12_attr.o: ../../include/openssl/pkcs12.h ../../include/openssl/pkcs7.h
$(KBE_CONFIG)/p12_attr.o: ../../include/openssl/safestack.h ../../include/openssl/sha.h
$(KBE_CONFIG)/p12_attr.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
$(KBE_CONFIG)/p12_attr.o: ../../include/openssl/x509.h ../../include/openssl/x509_vfy.h
$(KBE_CONFIG)/p12_attr.o: ../cryptlib.h p12_attr.c
$(KBE_CONFIG)/p12_crpt.o: ../../e_os.h ../../include/openssl/asn1.h
$(KBE_CONFIG)/p12_crpt.o: ../../include/openssl/bio.h ../../include/openssl/buffer.h
$(KBE_CONFIG)/p12_crpt.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(KBE_CONFIG)/p12_crpt.o: ../../include/openssl/ec.h ../../include/openssl/ecdh.h
$(KBE_CONFIG)/p12_crpt.o: ../../include/openssl/ecdsa.h ../../include/openssl/err.h
$(KBE_CONFIG)/p12_crpt.o: ../../include/openssl/evp.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/p12_crpt.o: ../../include/openssl/obj_mac.h ../../include/openssl/objects.h
$(KBE_CONFIG)/p12_crpt.o: ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/p12_crpt.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/p12_crpt.o: ../../include/openssl/pkcs12.h ../../include/openssl/pkcs7.h
$(KBE_CONFIG)/p12_crpt.o: ../../include/openssl/safestack.h ../../include/openssl/sha.h
$(KBE_CONFIG)/p12_crpt.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
$(KBE_CONFIG)/p12_crpt.o: ../../include/openssl/x509.h ../../include/openssl/x509_vfy.h
$(KBE_CONFIG)/p12_crpt.o: ../cryptlib.h p12_crpt.c
$(KBE_CONFIG)/p12_crt.o: ../../e_os.h ../../include/openssl/asn1.h
$(KBE_CONFIG)/p12_crt.o: ../../include/openssl/bio.h ../../include/openssl/buffer.h
$(KBE_CONFIG)/p12_crt.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(KBE_CONFIG)/p12_crt.o: ../../include/openssl/ec.h ../../include/openssl/ecdh.h
$(KBE_CONFIG)/p12_crt.o: ../../include/openssl/ecdsa.h ../../include/openssl/err.h
$(KBE_CONFIG)/p12_crt.o: ../../include/openssl/evp.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/p12_crt.o: ../../include/openssl/obj_mac.h ../../include/openssl/objects.h
$(KBE_CONFIG)/p12_crt.o: ../../include/openssl/opensslconf.h ../../include/openssl/opensslv.h
$(KBE_CONFIG)/p12_crt.o: ../../include/openssl/ossl_typ.h ../../include/openssl/pkcs12.h
$(KBE_CONFIG)/p12_crt.o: ../../include/openssl/pkcs7.h ../../include/openssl/safestack.h
$(KBE_CONFIG)/p12_crt.o: ../../include/openssl/sha.h ../../include/openssl/stack.h
$(KBE_CONFIG)/p12_crt.o: ../../include/openssl/symhacks.h ../../include/openssl/x509.h
$(KBE_CONFIG)/p12_crt.o: ../../include/openssl/x509_vfy.h ../cryptlib.h p12_crt.c
$(KBE_CONFIG)/p12_decr.o: ../../e_os.h ../../include/openssl/asn1.h
$(KBE_CONFIG)/p12_decr.o: ../../include/openssl/bio.h ../../include/openssl/buffer.h
$(KBE_CONFIG)/p12_decr.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(KBE_CONFIG)/p12_decr.o: ../../include/openssl/ec.h ../../include/openssl/ecdh.h
$(KBE_CONFIG)/p12_decr.o: ../../include/openssl/ecdsa.h ../../include/openssl/err.h
$(KBE_CONFIG)/p12_decr.o: ../../include/openssl/evp.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/p12_decr.o: ../../include/openssl/obj_mac.h ../../include/openssl/objects.h
$(KBE_CONFIG)/p12_decr.o: ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/p12_decr.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/p12_decr.o: ../../include/openssl/pkcs12.h ../../include/openssl/pkcs7.h
$(KBE_CONFIG)/p12_decr.o: ../../include/openssl/safestack.h ../../include/openssl/sha.h
$(KBE_CONFIG)/p12_decr.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
$(KBE_CONFIG)/p12_decr.o: ../../include/openssl/x509.h ../../include/openssl/x509_vfy.h
$(KBE_CONFIG)/p12_decr.o: ../cryptlib.h p12_decr.c
$(KBE_CONFIG)/p12_init.o: ../../e_os.h ../../include/openssl/asn1.h
$(KBE_CONFIG)/p12_init.o: ../../include/openssl/bio.h ../../include/openssl/buffer.h
$(KBE_CONFIG)/p12_init.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(KBE_CONFIG)/p12_init.o: ../../include/openssl/ec.h ../../include/openssl/ecdh.h
$(KBE_CONFIG)/p12_init.o: ../../include/openssl/ecdsa.h ../../include/openssl/err.h
$(KBE_CONFIG)/p12_init.o: ../../include/openssl/evp.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/p12_init.o: ../../include/openssl/obj_mac.h ../../include/openssl/objects.h
$(KBE_CONFIG)/p12_init.o: ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/p12_init.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/p12_init.o: ../../include/openssl/pkcs12.h ../../include/openssl/pkcs7.h
$(KBE_CONFIG)/p12_init.o: ../../include/openssl/safestack.h ../../include/openssl/sha.h
$(KBE_CONFIG)/p12_init.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
$(KBE_CONFIG)/p12_init.o: ../../include/openssl/x509.h ../../include/openssl/x509_vfy.h
$(KBE_CONFIG)/p12_init.o: ../cryptlib.h p12_init.c
$(KBE_CONFIG)/p12_key.o: ../../e_os.h ../../include/openssl/asn1.h
$(KBE_CONFIG)/p12_key.o: ../../include/openssl/bio.h ../../include/openssl/bn.h
$(KBE_CONFIG)/p12_key.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/p12_key.o: ../../include/openssl/e_os2.h ../../include/openssl/ec.h
$(KBE_CONFIG)/p12_key.o: ../../include/openssl/ecdh.h ../../include/openssl/ecdsa.h
$(KBE_CONFIG)/p12_key.o: ../../include/openssl/err.h ../../include/openssl/evp.h
$(KBE_CONFIG)/p12_key.o: ../../include/openssl/lhash.h ../../include/openssl/obj_mac.h
$(KBE_CONFIG)/p12_key.o: ../../include/openssl/objects.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/p12_key.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/p12_key.o: ../../include/openssl/pkcs12.h ../../include/openssl/pkcs7.h
$(KBE_CONFIG)/p12_key.o: ../../include/openssl/safestack.h ../../include/openssl/sha.h
$(KBE_CONFIG)/p12_key.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
$(KBE_CONFIG)/p12_key.o: ../../include/openssl/x509.h ../../include/openssl/x509_vfy.h
$(KBE_CONFIG)/p12_key.o: ../cryptlib.h p12_key.c
$(KBE_CONFIG)/p12_kiss.o: ../../e_os.h ../../include/openssl/asn1.h
$(KBE_CONFIG)/p12_kiss.o: ../../include/openssl/bio.h ../../include/openssl/buffer.h
$(KBE_CONFIG)/p12_kiss.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(KBE_CONFIG)/p12_kiss.o: ../../include/openssl/ec.h ../../include/openssl/ecdh.h
$(KBE_CONFIG)/p12_kiss.o: ../../include/openssl/ecdsa.h ../../include/openssl/err.h
$(KBE_CONFIG)/p12_kiss.o: ../../include/openssl/evp.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/p12_kiss.o: ../../include/openssl/obj_mac.h ../../include/openssl/objects.h
$(KBE_CONFIG)/p12_kiss.o: ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/p12_kiss.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/p12_kiss.o: ../../include/openssl/pkcs12.h ../../include/openssl/pkcs7.h
$(KBE_CONFIG)/p12_kiss.o: ../../include/openssl/safestack.h ../../include/openssl/sha.h
$(KBE_CONFIG)/p12_kiss.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
$(KBE_CONFIG)/p12_kiss.o: ../../include/openssl/x509.h ../../include/openssl/x509_vfy.h
$(KBE_CONFIG)/p12_kiss.o: ../cryptlib.h p12_kiss.c
$(KBE_CONFIG)/p12_mutl.o: ../../e_os.h ../../include/openssl/asn1.h
$(KBE_CONFIG)/p12_mutl.o: ../../include/openssl/bio.h ../../include/openssl/buffer.h
$(KBE_CONFIG)/p12_mutl.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(KBE_CONFIG)/p12_mutl.o: ../../include/openssl/ec.h ../../include/openssl/ecdh.h
$(KBE_CONFIG)/p12_mutl.o: ../../include/openssl/ecdsa.h ../../include/openssl/err.h
$(KBE_CONFIG)/p12_mutl.o: ../../include/openssl/evp.h ../../include/openssl/hmac.h
$(KBE_CONFIG)/p12_mutl.o: ../../include/openssl/lhash.h ../../include/openssl/obj_mac.h
$(KBE_CONFIG)/p12_mutl.o: ../../include/openssl/objects.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/p12_mutl.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/p12_mutl.o: ../../include/openssl/pkcs12.h ../../include/openssl/pkcs7.h
$(KBE_CONFIG)/p12_mutl.o: ../../include/openssl/rand.h ../../include/openssl/safestack.h
$(KBE_CONFIG)/p12_mutl.o: ../../include/openssl/sha.h ../../include/openssl/stack.h
$(KBE_CONFIG)/p12_mutl.o: ../../include/openssl/symhacks.h ../../include/openssl/x509.h
$(KBE_CONFIG)/p12_mutl.o: ../../include/openssl/x509_vfy.h ../cryptlib.h p12_mutl.c
$(KBE_CONFIG)/p12_npas.o: ../../include/openssl/asn1.h ../../include/openssl/bio.h
$(KBE_CONFIG)/p12_npas.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/p12_npas.o: ../../include/openssl/e_os2.h ../../include/openssl/ec.h
$(KBE_CONFIG)/p12_npas.o: ../../include/openssl/ecdh.h ../../include/openssl/ecdsa.h
$(KBE_CONFIG)/p12_npas.o: ../../include/openssl/err.h ../../include/openssl/evp.h
$(KBE_CONFIG)/p12_npas.o: ../../include/openssl/lhash.h ../../include/openssl/obj_mac.h
$(KBE_CONFIG)/p12_npas.o: ../../include/openssl/objects.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/p12_npas.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/p12_npas.o: ../../include/openssl/pem.h ../../include/openssl/pem2.h
$(KBE_CONFIG)/p12_npas.o: ../../include/openssl/pkcs12.h ../../include/openssl/pkcs7.h
$(KBE_CONFIG)/p12_npas.o: ../../include/openssl/safestack.h ../../include/openssl/sha.h
$(KBE_CONFIG)/p12_npas.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
$(KBE_CONFIG)/p12_npas.o: ../../include/openssl/x509.h ../../include/openssl/x509_vfy.h
$(KBE_CONFIG)/p12_npas.o: p12_npas.c
$(KBE_CONFIG)/p12_p8d.o: ../../e_os.h ../../include/openssl/asn1.h
$(KBE_CONFIG)/p12_p8d.o: ../../include/openssl/bio.h ../../include/openssl/buffer.h
$(KBE_CONFIG)/p12_p8d.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(KBE_CONFIG)/p12_p8d.o: ../../include/openssl/ec.h ../../include/openssl/ecdh.h
$(KBE_CONFIG)/p12_p8d.o: ../../include/openssl/ecdsa.h ../../include/openssl/err.h
$(KBE_CONFIG)/p12_p8d.o: ../../include/openssl/evp.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/p12_p8d.o: ../../include/openssl/obj_mac.h ../../include/openssl/objects.h
$(KBE_CONFIG)/p12_p8d.o: ../../include/openssl/opensslconf.h ../../include/openssl/opensslv.h
$(KBE_CONFIG)/p12_p8d.o: ../../include/openssl/ossl_typ.h ../../include/openssl/pkcs12.h
$(KBE_CONFIG)/p12_p8d.o: ../../include/openssl/pkcs7.h ../../include/openssl/safestack.h
$(KBE_CONFIG)/p12_p8d.o: ../../include/openssl/sha.h ../../include/openssl/stack.h
$(KBE_CONFIG)/p12_p8d.o: ../../include/openssl/symhacks.h ../../include/openssl/x509.h
$(KBE_CONFIG)/p12_p8d.o: ../../include/openssl/x509_vfy.h ../cryptlib.h p12_p8d.c
$(KBE_CONFIG)/p12_p8e.o: ../../e_os.h ../../include/openssl/asn1.h
$(KBE_CONFIG)/p12_p8e.o: ../../include/openssl/bio.h ../../include/openssl/buffer.h
$(KBE_CONFIG)/p12_p8e.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(KBE_CONFIG)/p12_p8e.o: ../../include/openssl/ec.h ../../include/openssl/ecdh.h
$(KBE_CONFIG)/p12_p8e.o: ../../include/openssl/ecdsa.h ../../include/openssl/err.h
$(KBE_CONFIG)/p12_p8e.o: ../../include/openssl/evp.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/p12_p8e.o: ../../include/openssl/obj_mac.h ../../include/openssl/objects.h
$(KBE_CONFIG)/p12_p8e.o: ../../include/openssl/opensslconf.h ../../include/openssl/opensslv.h
$(KBE_CONFIG)/p12_p8e.o: ../../include/openssl/ossl_typ.h ../../include/openssl/pkcs12.h
$(KBE_CONFIG)/p12_p8e.o: ../../include/openssl/pkcs7.h ../../include/openssl/safestack.h
$(KBE_CONFIG)/p12_p8e.o: ../../include/openssl/sha.h ../../include/openssl/stack.h
$(KBE_CONFIG)/p12_p8e.o: ../../include/openssl/symhacks.h ../../include/openssl/x509.h
$(KBE_CONFIG)/p12_p8e.o: ../../include/openssl/x509_vfy.h ../cryptlib.h p12_p8e.c
$(KBE_CONFIG)/p12_utl.o: ../../e_os.h ../../include/openssl/asn1.h
$(KBE_CONFIG)/p12_utl.o: ../../include/openssl/bio.h ../../include/openssl/buffer.h
$(KBE_CONFIG)/p12_utl.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(KBE_CONFIG)/p12_utl.o: ../../include/openssl/ec.h ../../include/openssl/ecdh.h
$(KBE_CONFIG)/p12_utl.o: ../../include/openssl/ecdsa.h ../../include/openssl/err.h
$(KBE_CONFIG)/p12_utl.o: ../../include/openssl/evp.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/p12_utl.o: ../../include/openssl/obj_mac.h ../../include/openssl/objects.h
$(KBE_CONFIG)/p12_utl.o: ../../include/openssl/opensslconf.h ../../include/openssl/opensslv.h
$(KBE_CONFIG)/p12_utl.o: ../../include/openssl/ossl_typ.h ../../include/openssl/pkcs12.h
$(KBE_CONFIG)/p12_utl.o: ../../include/openssl/pkcs7.h ../../include/openssl/safestack.h
$(KBE_CONFIG)/p12_utl.o: ../../include/openssl/sha.h ../../include/openssl/stack.h
$(KBE_CONFIG)/p12_utl.o: ../../include/openssl/symhacks.h ../../include/openssl/x509.h
$(KBE_CONFIG)/p12_utl.o: ../../include/openssl/x509_vfy.h ../cryptlib.h p12_utl.c
$(KBE_CONFIG)/pk12err.o: ../../include/openssl/asn1.h ../../include/openssl/bio.h
$(KBE_CONFIG)/pk12err.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/pk12err.o: ../../include/openssl/e_os2.h ../../include/openssl/ec.h
$(KBE_CONFIG)/pk12err.o: ../../include/openssl/ecdh.h ../../include/openssl/ecdsa.h
$(KBE_CONFIG)/pk12err.o: ../../include/openssl/err.h ../../include/openssl/evp.h
$(KBE_CONFIG)/pk12err.o: ../../include/openssl/lhash.h ../../include/openssl/obj_mac.h
$(KBE_CONFIG)/pk12err.o: ../../include/openssl/objects.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/pk12err.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/pk12err.o: ../../include/openssl/pkcs12.h ../../include/openssl/pkcs7.h
$(KBE_CONFIG)/pk12err.o: ../../include/openssl/safestack.h ../../include/openssl/sha.h
$(KBE_CONFIG)/pk12err.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
$(KBE_CONFIG)/pk12err.o: ../../include/openssl/x509.h ../../include/openssl/x509_vfy.h
$(KBE_CONFIG)/pk12err.o: pk12err.c
