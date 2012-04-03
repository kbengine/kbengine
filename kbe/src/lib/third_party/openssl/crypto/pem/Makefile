#
# OpenSSL/crypto/pem/Makefile
#

DIR=	pem
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

LIB=$(BWLIBDIR)/libcrypto.a
LIBSRC= pem_sign.c pem_seal.c pem_info.c pem_lib.c pem_all.c pem_err.c \
	pem_x509.c pem_xaux.c pem_oth.c pem_pk8.c pem_pkey.c

LIBOBJ=	$(MF_CONFIG)/pem_sign.o $(MF_CONFIG)/pem_seal.o \
	$(MF_CONFIG)/pem_info.o $(MF_CONFIG)/pem_lib.o $(MF_CONFIG)/pem_all.o \
	$(MF_CONFIG)/pem_err.o $(MF_CONFIG)/pem_x509.o $(MF_CONFIG)/pem_xaux.o \
	$(MF_CONFIG)/pem_oth.o $(MF_CONFIG)/pem_pk8.o $(MF_CONFIG)/pem_pkey.o

SRC= $(LIBSRC)

EXHEADER= pem.h pem2.h
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

links: $(EXHEADER)
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
	$(MAKEDEPEND) -- $(CFLAG) $(INCLUDES) $(DEPFLAG) -- $(LIBSRC)

dclean:
	$(PERL) -pe 'if (/^# DO NOT DELETE THIS LINE/) {print; exit(0);}' $(MAKEFILE) >Makefile.new
	mv -f Makefile.new $(MAKEFILE)

clean:
	rm -f $(MF_CONFIG)/*.o *.o *.obj lib tags core .pure .nfs* *.old *.bak fluff

# DO NOT DELETE THIS LINE -- make depend depends on it.

$(MF_CONFIG)/pem_all.o: ../../e_os.h ../../include/openssl/asn1.h
$(MF_CONFIG)/pem_all.o: ../../include/openssl/bio.h ../../include/openssl/buffer.h
$(MF_CONFIG)/pem_all.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(MF_CONFIG)/pem_all.o: ../../include/openssl/ec.h ../../include/openssl/ecdh.h
$(MF_CONFIG)/pem_all.o: ../../include/openssl/ecdsa.h ../../include/openssl/err.h
$(MF_CONFIG)/pem_all.o: ../../include/openssl/evp.h ../../include/openssl/lhash.h
$(MF_CONFIG)/pem_all.o: ../../include/openssl/obj_mac.h ../../include/openssl/objects.h
$(MF_CONFIG)/pem_all.o: ../../include/openssl/opensslconf.h ../../include/openssl/opensslv.h
$(MF_CONFIG)/pem_all.o: ../../include/openssl/ossl_typ.h ../../include/openssl/pem.h
$(MF_CONFIG)/pem_all.o: ../../include/openssl/pem2.h ../../include/openssl/pkcs7.h
$(MF_CONFIG)/pem_all.o: ../../include/openssl/rsa.h ../../include/openssl/safestack.h
$(MF_CONFIG)/pem_all.o: ../../include/openssl/sha.h ../../include/openssl/stack.h
$(MF_CONFIG)/pem_all.o: ../../include/openssl/symhacks.h ../../include/openssl/x509.h
$(MF_CONFIG)/pem_all.o: ../../include/openssl/x509_vfy.h ../cryptlib.h pem_all.c
$(MF_CONFIG)/pem_err.o: ../../include/openssl/asn1.h ../../include/openssl/bio.h
$(MF_CONFIG)/pem_err.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(MF_CONFIG)/pem_err.o: ../../include/openssl/e_os2.h ../../include/openssl/ec.h
$(MF_CONFIG)/pem_err.o: ../../include/openssl/ecdh.h ../../include/openssl/ecdsa.h
$(MF_CONFIG)/pem_err.o: ../../include/openssl/err.h ../../include/openssl/evp.h
$(MF_CONFIG)/pem_err.o: ../../include/openssl/lhash.h ../../include/openssl/obj_mac.h
$(MF_CONFIG)/pem_err.o: ../../include/openssl/objects.h ../../include/openssl/opensslconf.h
$(MF_CONFIG)/pem_err.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(MF_CONFIG)/pem_err.o: ../../include/openssl/pem.h ../../include/openssl/pem2.h
$(MF_CONFIG)/pem_err.o: ../../include/openssl/pkcs7.h ../../include/openssl/safestack.h
$(MF_CONFIG)/pem_err.o: ../../include/openssl/sha.h ../../include/openssl/stack.h
$(MF_CONFIG)/pem_err.o: ../../include/openssl/symhacks.h ../../include/openssl/x509.h
$(MF_CONFIG)/pem_err.o: ../../include/openssl/x509_vfy.h pem_err.c
$(MF_CONFIG)/pem_info.o: ../../e_os.h ../../include/openssl/asn1.h
$(MF_CONFIG)/pem_info.o: ../../include/openssl/bio.h ../../include/openssl/buffer.h
$(MF_CONFIG)/pem_info.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(MF_CONFIG)/pem_info.o: ../../include/openssl/ec.h ../../include/openssl/ecdh.h
$(MF_CONFIG)/pem_info.o: ../../include/openssl/ecdsa.h ../../include/openssl/err.h
$(MF_CONFIG)/pem_info.o: ../../include/openssl/evp.h ../../include/openssl/lhash.h
$(MF_CONFIG)/pem_info.o: ../../include/openssl/obj_mac.h ../../include/openssl/objects.h
$(MF_CONFIG)/pem_info.o: ../../include/openssl/opensslconf.h
$(MF_CONFIG)/pem_info.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(MF_CONFIG)/pem_info.o: ../../include/openssl/pem.h ../../include/openssl/pem2.h
$(MF_CONFIG)/pem_info.o: ../../include/openssl/pkcs7.h ../../include/openssl/rsa.h
$(MF_CONFIG)/pem_info.o: ../../include/openssl/safestack.h ../../include/openssl/sha.h
$(MF_CONFIG)/pem_info.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
$(MF_CONFIG)/pem_info.o: ../../include/openssl/x509.h ../../include/openssl/x509_vfy.h
$(MF_CONFIG)/pem_info.o: ../cryptlib.h pem_info.c
$(MF_CONFIG)/pem_lib.o: ../../e_os.h ../../include/openssl/asn1.h
$(MF_CONFIG)/pem_lib.o: ../../include/openssl/bio.h ../../include/openssl/buffer.h
$(MF_CONFIG)/pem_lib.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(MF_CONFIG)/pem_lib.o: ../../include/openssl/ec.h ../../include/openssl/ecdh.h
$(MF_CONFIG)/pem_lib.o: ../../include/openssl/ecdsa.h ../../include/openssl/err.h
$(MF_CONFIG)/pem_lib.o: ../../include/openssl/evp.h ../../include/openssl/lhash.h
$(MF_CONFIG)/pem_lib.o: ../../include/openssl/obj_mac.h ../../include/openssl/objects.h
$(MF_CONFIG)/pem_lib.o: ../../include/openssl/opensslconf.h ../../include/openssl/opensslv.h
$(MF_CONFIG)/pem_lib.o: ../../include/openssl/ossl_typ.h ../../include/openssl/pem.h
$(MF_CONFIG)/pem_lib.o: ../../include/openssl/pem2.h ../../include/openssl/pkcs12.h
$(MF_CONFIG)/pem_lib.o: ../../include/openssl/pkcs7.h ../../include/openssl/rand.h
$(MF_CONFIG)/pem_lib.o: ../../include/openssl/safestack.h ../../include/openssl/sha.h
$(MF_CONFIG)/pem_lib.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
$(MF_CONFIG)/pem_lib.o: ../../include/openssl/x509.h ../../include/openssl/x509_vfy.h
$(MF_CONFIG)/pem_lib.o: ../cryptlib.h pem_lib.c
$(MF_CONFIG)/pem_oth.o: ../../e_os.h ../../include/openssl/asn1.h
$(MF_CONFIG)/pem_oth.o: ../../include/openssl/bio.h ../../include/openssl/buffer.h
$(MF_CONFIG)/pem_oth.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(MF_CONFIG)/pem_oth.o: ../../include/openssl/ec.h ../../include/openssl/ecdh.h
$(MF_CONFIG)/pem_oth.o: ../../include/openssl/ecdsa.h ../../include/openssl/err.h
$(MF_CONFIG)/pem_oth.o: ../../include/openssl/evp.h ../../include/openssl/lhash.h
$(MF_CONFIG)/pem_oth.o: ../../include/openssl/obj_mac.h ../../include/openssl/objects.h
$(MF_CONFIG)/pem_oth.o: ../../include/openssl/opensslconf.h ../../include/openssl/opensslv.h
$(MF_CONFIG)/pem_oth.o: ../../include/openssl/ossl_typ.h ../../include/openssl/pem.h
$(MF_CONFIG)/pem_oth.o: ../../include/openssl/pem2.h ../../include/openssl/pkcs7.h
$(MF_CONFIG)/pem_oth.o: ../../include/openssl/rand.h ../../include/openssl/safestack.h
$(MF_CONFIG)/pem_oth.o: ../../include/openssl/sha.h ../../include/openssl/stack.h
$(MF_CONFIG)/pem_oth.o: ../../include/openssl/symhacks.h ../../include/openssl/x509.h
$(MF_CONFIG)/pem_oth.o: ../../include/openssl/x509_vfy.h ../cryptlib.h pem_oth.c
$(MF_CONFIG)/pem_pk8.o: ../../e_os.h ../../include/openssl/asn1.h
$(MF_CONFIG)/pem_pk8.o: ../../include/openssl/bio.h ../../include/openssl/buffer.h
$(MF_CONFIG)/pem_pk8.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(MF_CONFIG)/pem_pk8.o: ../../include/openssl/ec.h ../../include/openssl/ecdh.h
$(MF_CONFIG)/pem_pk8.o: ../../include/openssl/ecdsa.h ../../include/openssl/err.h
$(MF_CONFIG)/pem_pk8.o: ../../include/openssl/evp.h ../../include/openssl/lhash.h
$(MF_CONFIG)/pem_pk8.o: ../../include/openssl/obj_mac.h ../../include/openssl/objects.h
$(MF_CONFIG)/pem_pk8.o: ../../include/openssl/opensslconf.h ../../include/openssl/opensslv.h
$(MF_CONFIG)/pem_pk8.o: ../../include/openssl/ossl_typ.h ../../include/openssl/pem.h
$(MF_CONFIG)/pem_pk8.o: ../../include/openssl/pem2.h ../../include/openssl/pkcs12.h
$(MF_CONFIG)/pem_pk8.o: ../../include/openssl/pkcs7.h ../../include/openssl/rand.h
$(MF_CONFIG)/pem_pk8.o: ../../include/openssl/safestack.h ../../include/openssl/sha.h
$(MF_CONFIG)/pem_pk8.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
$(MF_CONFIG)/pem_pk8.o: ../../include/openssl/x509.h ../../include/openssl/x509_vfy.h
$(MF_CONFIG)/pem_pk8.o: ../cryptlib.h pem_pk8.c
$(MF_CONFIG)/pem_pkey.o: ../../e_os.h ../../include/openssl/asn1.h
$(MF_CONFIG)/pem_pkey.o: ../../include/openssl/bio.h ../../include/openssl/buffer.h
$(MF_CONFIG)/pem_pkey.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(MF_CONFIG)/pem_pkey.o: ../../include/openssl/ec.h ../../include/openssl/ecdh.h
$(MF_CONFIG)/pem_pkey.o: ../../include/openssl/ecdsa.h ../../include/openssl/err.h
$(MF_CONFIG)/pem_pkey.o: ../../include/openssl/evp.h ../../include/openssl/lhash.h
$(MF_CONFIG)/pem_pkey.o: ../../include/openssl/obj_mac.h ../../include/openssl/objects.h
$(MF_CONFIG)/pem_pkey.o: ../../include/openssl/opensslconf.h
$(MF_CONFIG)/pem_pkey.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(MF_CONFIG)/pem_pkey.o: ../../include/openssl/pem.h ../../include/openssl/pem2.h
$(MF_CONFIG)/pem_pkey.o: ../../include/openssl/pkcs12.h ../../include/openssl/pkcs7.h
$(MF_CONFIG)/pem_pkey.o: ../../include/openssl/rand.h ../../include/openssl/safestack.h
$(MF_CONFIG)/pem_pkey.o: ../../include/openssl/sha.h ../../include/openssl/stack.h
$(MF_CONFIG)/pem_pkey.o: ../../include/openssl/symhacks.h ../../include/openssl/x509.h
$(MF_CONFIG)/pem_pkey.o: ../../include/openssl/x509_vfy.h ../cryptlib.h pem_pkey.c
$(MF_CONFIG)/pem_seal.o: ../../e_os.h ../../include/openssl/asn1.h
$(MF_CONFIG)/pem_seal.o: ../../include/openssl/bio.h ../../include/openssl/buffer.h
$(MF_CONFIG)/pem_seal.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(MF_CONFIG)/pem_seal.o: ../../include/openssl/ec.h ../../include/openssl/ecdh.h
$(MF_CONFIG)/pem_seal.o: ../../include/openssl/ecdsa.h ../../include/openssl/err.h
$(MF_CONFIG)/pem_seal.o: ../../include/openssl/evp.h ../../include/openssl/lhash.h
$(MF_CONFIG)/pem_seal.o: ../../include/openssl/obj_mac.h ../../include/openssl/objects.h
$(MF_CONFIG)/pem_seal.o: ../../include/openssl/opensslconf.h
$(MF_CONFIG)/pem_seal.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(MF_CONFIG)/pem_seal.o: ../../include/openssl/pem.h ../../include/openssl/pem2.h
$(MF_CONFIG)/pem_seal.o: ../../include/openssl/pkcs7.h ../../include/openssl/rand.h
$(MF_CONFIG)/pem_seal.o: ../../include/openssl/rsa.h ../../include/openssl/safestack.h
$(MF_CONFIG)/pem_seal.o: ../../include/openssl/sha.h ../../include/openssl/stack.h
$(MF_CONFIG)/pem_seal.o: ../../include/openssl/symhacks.h ../../include/openssl/x509.h
$(MF_CONFIG)/pem_seal.o: ../../include/openssl/x509_vfy.h ../cryptlib.h pem_seal.c
$(MF_CONFIG)/pem_sign.o: ../../e_os.h ../../include/openssl/asn1.h
$(MF_CONFIG)/pem_sign.o: ../../include/openssl/bio.h ../../include/openssl/buffer.h
$(MF_CONFIG)/pem_sign.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(MF_CONFIG)/pem_sign.o: ../../include/openssl/ec.h ../../include/openssl/ecdh.h
$(MF_CONFIG)/pem_sign.o: ../../include/openssl/ecdsa.h ../../include/openssl/err.h
$(MF_CONFIG)/pem_sign.o: ../../include/openssl/evp.h ../../include/openssl/lhash.h
$(MF_CONFIG)/pem_sign.o: ../../include/openssl/obj_mac.h ../../include/openssl/objects.h
$(MF_CONFIG)/pem_sign.o: ../../include/openssl/opensslconf.h
$(MF_CONFIG)/pem_sign.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(MF_CONFIG)/pem_sign.o: ../../include/openssl/pem.h ../../include/openssl/pem2.h
$(MF_CONFIG)/pem_sign.o: ../../include/openssl/pkcs7.h ../../include/openssl/rand.h
$(MF_CONFIG)/pem_sign.o: ../../include/openssl/safestack.h ../../include/openssl/sha.h
$(MF_CONFIG)/pem_sign.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
$(MF_CONFIG)/pem_sign.o: ../../include/openssl/x509.h ../../include/openssl/x509_vfy.h
$(MF_CONFIG)/pem_sign.o: ../cryptlib.h pem_sign.c
$(MF_CONFIG)/pem_x509.o: ../../e_os.h ../../include/openssl/asn1.h
$(MF_CONFIG)/pem_x509.o: ../../include/openssl/bio.h ../../include/openssl/buffer.h
$(MF_CONFIG)/pem_x509.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(MF_CONFIG)/pem_x509.o: ../../include/openssl/ec.h ../../include/openssl/ecdh.h
$(MF_CONFIG)/pem_x509.o: ../../include/openssl/ecdsa.h ../../include/openssl/err.h
$(MF_CONFIG)/pem_x509.o: ../../include/openssl/evp.h ../../include/openssl/lhash.h
$(MF_CONFIG)/pem_x509.o: ../../include/openssl/obj_mac.h ../../include/openssl/objects.h
$(MF_CONFIG)/pem_x509.o: ../../include/openssl/opensslconf.h
$(MF_CONFIG)/pem_x509.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(MF_CONFIG)/pem_x509.o: ../../include/openssl/pem.h ../../include/openssl/pem2.h
$(MF_CONFIG)/pem_x509.o: ../../include/openssl/pkcs7.h ../../include/openssl/safestack.h
$(MF_CONFIG)/pem_x509.o: ../../include/openssl/sha.h ../../include/openssl/stack.h
$(MF_CONFIG)/pem_x509.o: ../../include/openssl/symhacks.h ../../include/openssl/x509.h
$(MF_CONFIG)/pem_x509.o: ../../include/openssl/x509_vfy.h ../cryptlib.h pem_x509.c
$(MF_CONFIG)/pem_xaux.o: ../../e_os.h ../../include/openssl/asn1.h
$(MF_CONFIG)/pem_xaux.o: ../../include/openssl/bio.h ../../include/openssl/buffer.h
$(MF_CONFIG)/pem_xaux.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(MF_CONFIG)/pem_xaux.o: ../../include/openssl/ec.h ../../include/openssl/ecdh.h
$(MF_CONFIG)/pem_xaux.o: ../../include/openssl/ecdsa.h ../../include/openssl/err.h
$(MF_CONFIG)/pem_xaux.o: ../../include/openssl/evp.h ../../include/openssl/lhash.h
$(MF_CONFIG)/pem_xaux.o: ../../include/openssl/obj_mac.h ../../include/openssl/objects.h
$(MF_CONFIG)/pem_xaux.o: ../../include/openssl/opensslconf.h
$(MF_CONFIG)/pem_xaux.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(MF_CONFIG)/pem_xaux.o: ../../include/openssl/pem.h ../../include/openssl/pem2.h
$(MF_CONFIG)/pem_xaux.o: ../../include/openssl/pkcs7.h ../../include/openssl/safestack.h
$(MF_CONFIG)/pem_xaux.o: ../../include/openssl/sha.h ../../include/openssl/stack.h
$(MF_CONFIG)/pem_xaux.o: ../../include/openssl/symhacks.h ../../include/openssl/x509.h
$(MF_CONFIG)/pem_xaux.o: ../../include/openssl/x509_vfy.h ../cryptlib.h pem_xaux.c
