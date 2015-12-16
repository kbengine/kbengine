#
# OpenSSL/crypto/pkcs7/Makefile
#

ifndef KBE_ROOT
export KBE_ROOT := $(subst /kbe/src/lib/dependencies/openssl/crypto/pkcs7,,$(CURDIR))
endif

LIBDIR= $(KBE_ROOT)/kbe/src/libs


DIR=	pkcs7
TOP=	../..
CC=	cc
INCLUDES= -I.. -I$(TOP) -I../../include
CFLAG=-g
MAKEFILE=	Makefile
AR=		ar r

PEX_LIBS=
EX_LIBS=
 
CFLAGS= $(INCLUDES) $(CFLAG)

GENERAL=Makefile README
TEST=
APPS=

LIB=$(LIBDIR)/libcrypto.a
LIBSRC=	pk7_asn1.c pk7_lib.c pkcs7err.c pk7_doit.c pk7_smime.c pk7_attr.c \
	pk7_mime.c
LIBOBJ= $(KBE_CONFIG)/pk7_asn1.o $(KBE_CONFIG)/pk7_lib.o $(KBE_CONFIG)/pkcs7err.o \
	$(KBE_CONFIG)/pk7_doit.o $(KBE_CONFIG)/pk7_smime.o $(KBE_CONFIG)/pk7_attr.o \
	$(KBE_CONFIG)/pk7_mime.o

SRC= $(LIBSRC)

EXHEADER=  pkcs7.h
HEADER=	$(EXHEADER)

ALL=    $(GENERAL) $(SRC) $(HEADER)

top:
	(cd ../..; $(MAKE) DIRS=crypto SDIRS=$(DIR) sub_all)

test:

all:	lib

testapps: enc dec sign verify

enc: enc.o lib
	$(CC) $(CFLAGS) -o enc enc.o $(PEX_LIBS) $(LIB) $(EX_LIBS)

dec: dec.o lib
	$(CC) $(CFLAGS) -o dec dec.o $(PEX_LIBS) $(LIB) $(EX_LIBS)

sign: sign.o lib
	$(CC) $(CFLAGS) -o sign sign.o $(PEX_LIBS) $(LIB) $(EX_LIBS)

verify: verify.o example.o lib
	$(CC) $(CFLAGS) -o verify verify.o $(PEX_LIBS) example.o $(LIB) $(EX_LIBS)

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
	rm -f $(KBE_CONFIG)/*.o *.o *.obj lib tags core .pure .nfs* *.old *.bak fluff enc dec sign verify

# DO NOT DELETE THIS LINE -- make depend depends on it.

$(KBE_CONFIG)/pk7_asn1.o: ../../e_os.h ../../include/openssl/asn1.h
$(KBE_CONFIG)/pk7_asn1.o: ../../include/openssl/asn1t.h ../../include/openssl/bio.h
$(KBE_CONFIG)/pk7_asn1.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/pk7_asn1.o: ../../include/openssl/e_os2.h ../../include/openssl/ec.h
$(KBE_CONFIG)/pk7_asn1.o: ../../include/openssl/ecdh.h ../../include/openssl/ecdsa.h
$(KBE_CONFIG)/pk7_asn1.o: ../../include/openssl/err.h ../../include/openssl/evp.h
$(KBE_CONFIG)/pk7_asn1.o: ../../include/openssl/lhash.h ../../include/openssl/obj_mac.h
$(KBE_CONFIG)/pk7_asn1.o: ../../include/openssl/objects.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/pk7_asn1.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/pk7_asn1.o: ../../include/openssl/pkcs7.h ../../include/openssl/safestack.h
$(KBE_CONFIG)/pk7_asn1.o: ../../include/openssl/sha.h ../../include/openssl/stack.h
$(KBE_CONFIG)/pk7_asn1.o: ../../include/openssl/symhacks.h ../../include/openssl/x509.h
$(KBE_CONFIG)/pk7_asn1.o: ../../include/openssl/x509_vfy.h ../cryptlib.h pk7_asn1.c
$(KBE_CONFIG)/pk7_attr.o: ../../include/openssl/asn1.h ../../include/openssl/bio.h
$(KBE_CONFIG)/pk7_attr.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/pk7_attr.o: ../../include/openssl/e_os2.h ../../include/openssl/ec.h
$(KBE_CONFIG)/pk7_attr.o: ../../include/openssl/ecdh.h ../../include/openssl/ecdsa.h
$(KBE_CONFIG)/pk7_attr.o: ../../include/openssl/err.h ../../include/openssl/evp.h
$(KBE_CONFIG)/pk7_attr.o: ../../include/openssl/lhash.h ../../include/openssl/obj_mac.h
$(KBE_CONFIG)/pk7_attr.o: ../../include/openssl/objects.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/pk7_attr.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/pk7_attr.o: ../../include/openssl/pem.h ../../include/openssl/pem2.h
$(KBE_CONFIG)/pk7_attr.o: ../../include/openssl/pkcs7.h ../../include/openssl/safestack.h
$(KBE_CONFIG)/pk7_attr.o: ../../include/openssl/sha.h ../../include/openssl/stack.h
$(KBE_CONFIG)/pk7_attr.o: ../../include/openssl/symhacks.h ../../include/openssl/x509.h
$(KBE_CONFIG)/pk7_attr.o: ../../include/openssl/x509_vfy.h pk7_attr.c
$(KBE_CONFIG)/pk7_doit.o: ../../e_os.h ../../include/openssl/asn1.h
$(KBE_CONFIG)/pk7_doit.o: ../../include/openssl/bio.h ../../include/openssl/buffer.h
$(KBE_CONFIG)/pk7_doit.o: ../../include/openssl/conf.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/pk7_doit.o: ../../include/openssl/e_os2.h ../../include/openssl/ec.h
$(KBE_CONFIG)/pk7_doit.o: ../../include/openssl/ecdh.h ../../include/openssl/ecdsa.h
$(KBE_CONFIG)/pk7_doit.o: ../../include/openssl/err.h ../../include/openssl/evp.h
$(KBE_CONFIG)/pk7_doit.o: ../../include/openssl/lhash.h ../../include/openssl/obj_mac.h
$(KBE_CONFIG)/pk7_doit.o: ../../include/openssl/objects.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/pk7_doit.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/pk7_doit.o: ../../include/openssl/pkcs7.h ../../include/openssl/rand.h
$(KBE_CONFIG)/pk7_doit.o: ../../include/openssl/safestack.h ../../include/openssl/sha.h
$(KBE_CONFIG)/pk7_doit.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
$(KBE_CONFIG)/pk7_doit.o: ../../include/openssl/x509.h ../../include/openssl/x509_vfy.h
$(KBE_CONFIG)/pk7_doit.o: ../../include/openssl/x509v3.h ../cryptlib.h pk7_doit.c
$(KBE_CONFIG)/pk7_lib.o: ../../e_os.h ../../include/openssl/asn1.h
$(KBE_CONFIG)/pk7_lib.o: ../../include/openssl/bio.h ../../include/openssl/buffer.h
$(KBE_CONFIG)/pk7_lib.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(KBE_CONFIG)/pk7_lib.o: ../../include/openssl/ec.h ../../include/openssl/ecdh.h
$(KBE_CONFIG)/pk7_lib.o: ../../include/openssl/ecdsa.h ../../include/openssl/err.h
$(KBE_CONFIG)/pk7_lib.o: ../../include/openssl/evp.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/pk7_lib.o: ../../include/openssl/obj_mac.h ../../include/openssl/objects.h
$(KBE_CONFIG)/pk7_lib.o: ../../include/openssl/opensslconf.h ../../include/openssl/opensslv.h
$(KBE_CONFIG)/pk7_lib.o: ../../include/openssl/ossl_typ.h ../../include/openssl/pkcs7.h
$(KBE_CONFIG)/pk7_lib.o: ../../include/openssl/safestack.h ../../include/openssl/sha.h
$(KBE_CONFIG)/pk7_lib.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
$(KBE_CONFIG)/pk7_lib.o: ../../include/openssl/x509.h ../../include/openssl/x509_vfy.h
$(KBE_CONFIG)/pk7_lib.o: ../cryptlib.h pk7_lib.c
$(KBE_CONFIG)/pk7_mime.o: ../../e_os.h ../../include/openssl/asn1.h
$(KBE_CONFIG)/pk7_mime.o: ../../include/openssl/bio.h ../../include/openssl/buffer.h
$(KBE_CONFIG)/pk7_mime.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(KBE_CONFIG)/pk7_mime.o: ../../include/openssl/ec.h ../../include/openssl/ecdh.h
$(KBE_CONFIG)/pk7_mime.o: ../../include/openssl/ecdsa.h ../../include/openssl/err.h
$(KBE_CONFIG)/pk7_mime.o: ../../include/openssl/evp.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/pk7_mime.o: ../../include/openssl/obj_mac.h ../../include/openssl/objects.h
$(KBE_CONFIG)/pk7_mime.o: ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/pk7_mime.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/pk7_mime.o: ../../include/openssl/pkcs7.h ../../include/openssl/rand.h
$(KBE_CONFIG)/pk7_mime.o: ../../include/openssl/safestack.h ../../include/openssl/sha.h
$(KBE_CONFIG)/pk7_mime.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
$(KBE_CONFIG)/pk7_mime.o: ../../include/openssl/x509.h ../../include/openssl/x509_vfy.h
$(KBE_CONFIG)/pk7_mime.o: ../cryptlib.h pk7_mime.c
$(KBE_CONFIG)/pk7_smime.o: ../../e_os.h ../../include/openssl/asn1.h
$(KBE_CONFIG)/pk7_smime.o: ../../include/openssl/bio.h ../../include/openssl/buffer.h
$(KBE_CONFIG)/pk7_smime.o: ../../include/openssl/conf.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/pk7_smime.o: ../../include/openssl/e_os2.h ../../include/openssl/ec.h
$(KBE_CONFIG)/pk7_smime.o: ../../include/openssl/ecdh.h ../../include/openssl/ecdsa.h
$(KBE_CONFIG)/pk7_smime.o: ../../include/openssl/err.h ../../include/openssl/evp.h
$(KBE_CONFIG)/pk7_smime.o: ../../include/openssl/lhash.h ../../include/openssl/obj_mac.h
$(KBE_CONFIG)/pk7_smime.o: ../../include/openssl/objects.h
$(KBE_CONFIG)/pk7_smime.o: ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/pk7_smime.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/pk7_smime.o: ../../include/openssl/pkcs7.h ../../include/openssl/safestack.h
$(KBE_CONFIG)/pk7_smime.o: ../../include/openssl/sha.h ../../include/openssl/stack.h
$(KBE_CONFIG)/pk7_smime.o: ../../include/openssl/symhacks.h ../../include/openssl/x509.h
$(KBE_CONFIG)/pk7_smime.o: ../../include/openssl/x509_vfy.h ../../include/openssl/x509v3.h
$(KBE_CONFIG)/pk7_smime.o: ../cryptlib.h pk7_smime.c
$(KBE_CONFIG)/pkcs7err.o: ../../include/openssl/asn1.h ../../include/openssl/bio.h
$(KBE_CONFIG)/pkcs7err.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(KBE_CONFIG)/pkcs7err.o: ../../include/openssl/err.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/pkcs7err.o: ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/pkcs7err.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/pkcs7err.o: ../../include/openssl/pkcs7.h ../../include/openssl/safestack.h
$(KBE_CONFIG)/pkcs7err.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
$(KBE_CONFIG)/pkcs7err.o: pkcs7err.c
