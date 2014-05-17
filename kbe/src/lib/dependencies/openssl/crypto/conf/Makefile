#
# OpenSSL/crypto/conf/Makefile
#

ifndef KBE_ROOT
export KBE_ROOT := $(subst /kbe/src/lib/dependencies/openssl/crypto/conf,,$(CURDIR))
endif

LIBDIR= $(KBE_ROOT)/kbe/src/libs

DIR=	conf
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
LIBSRC= conf_err.c conf_lib.c conf_api.c conf_def.c conf_mod.c \
	 conf_mall.c conf_sap.c

LIBOBJ=	$(KBE_CONFIG)/conf_err.o $(KBE_CONFIG)/conf_lib.o \
	$(KBE_CONFIG)/conf_api.o $(KBE_CONFIG)/conf_def.o $(KBE_CONFIG)/conf_mod.o \
	$(KBE_CONFIG)/conf_mall.o $(KBE_CONFIG)/conf_sap.o

SRC= $(LIBSRC)

EXHEADER= conf.h conf_api.h
HEADER=	conf_def.h $(EXHEADER)

ALL=    $(GENERAL) $(SRC) $(HEADER)

top:
	(cd ../..; $(MAKE) DIRS=crypto SDIRS=$(DIR) sub_all)

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
	$(MAKEDEPEND) -- $(CFLAG) $(INCLUDES) $(DEPFLAG) -- $(LIBSRC)

dclean:
	$(PERL) -pe 'if (/^# DO NOT DELETE THIS LINE/) {print; exit(0);}' $(MAKEFILE) >Makefile.new
	mv -f Makefile.new $(MAKEFILE)

clean:
	rm -f $(KBE_CONFIG)/*.o *.o *.obj lib tags core .pure .nfs* *.old *.bak fluff

# DO NOT DELETE THIS LINE -- make depend depends on it.

$(KBE_CONFIG)/conf_api.o: ../../e_os.h ../../include/openssl/bio.h
$(KBE_CONFIG)/conf_api.o: ../../include/openssl/conf.h ../../include/openssl/conf_api.h
$(KBE_CONFIG)/conf_api.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(KBE_CONFIG)/conf_api.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/conf_api.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/conf_api.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/conf_api.o: ../../include/openssl/symhacks.h conf_api.c
$(KBE_CONFIG)/conf_def.o: ../../e_os.h ../../include/openssl/bio.h
$(KBE_CONFIG)/conf_def.o: ../../include/openssl/buffer.h ../../include/openssl/conf.h
$(KBE_CONFIG)/conf_def.o: ../../include/openssl/conf_api.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/conf_def.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(KBE_CONFIG)/conf_def.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/conf_def.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/conf_def.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/conf_def.o: ../../include/openssl/symhacks.h ../cryptlib.h conf_def.c
$(KBE_CONFIG)/conf_def.o: conf_def.h
$(KBE_CONFIG)/conf_err.o: ../../include/openssl/bio.h ../../include/openssl/conf.h
$(KBE_CONFIG)/conf_err.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(KBE_CONFIG)/conf_err.o: ../../include/openssl/err.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/conf_err.o: ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/conf_err.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/conf_err.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/conf_err.o: ../../include/openssl/symhacks.h conf_err.c
$(KBE_CONFIG)/conf_lib.o: ../../include/openssl/bio.h ../../include/openssl/conf.h
$(KBE_CONFIG)/conf_lib.o: ../../include/openssl/conf_api.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/conf_lib.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(KBE_CONFIG)/conf_lib.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/conf_lib.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/conf_lib.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/conf_lib.o: ../../include/openssl/symhacks.h conf_lib.c
$(KBE_CONFIG)/conf_mall.o: ../../e_os.h ../../include/openssl/asn1.h
$(KBE_CONFIG)/conf_mall.o: ../../include/openssl/bio.h ../../include/openssl/buffer.h
$(KBE_CONFIG)/conf_mall.o: ../../include/openssl/conf.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/conf_mall.o: ../../include/openssl/dso.h ../../include/openssl/e_os2.h
$(KBE_CONFIG)/conf_mall.o: ../../include/openssl/ec.h ../../include/openssl/ecdh.h
$(KBE_CONFIG)/conf_mall.o: ../../include/openssl/ecdsa.h ../../include/openssl/engine.h
$(KBE_CONFIG)/conf_mall.o: ../../include/openssl/err.h ../../include/openssl/evp.h
$(KBE_CONFIG)/conf_mall.o: ../../include/openssl/lhash.h ../../include/openssl/obj_mac.h
$(KBE_CONFIG)/conf_mall.o: ../../include/openssl/objects.h
$(KBE_CONFIG)/conf_mall.o: ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/conf_mall.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/conf_mall.o: ../../include/openssl/pkcs7.h ../../include/openssl/safestack.h
$(KBE_CONFIG)/conf_mall.o: ../../include/openssl/sha.h ../../include/openssl/stack.h
$(KBE_CONFIG)/conf_mall.o: ../../include/openssl/symhacks.h ../../include/openssl/x509.h
$(KBE_CONFIG)/conf_mall.o: ../../include/openssl/x509_vfy.h ../cryptlib.h conf_mall.c
$(KBE_CONFIG)/conf_mod.o: ../../e_os.h ../../include/openssl/asn1.h
$(KBE_CONFIG)/conf_mod.o: ../../include/openssl/bio.h ../../include/openssl/buffer.h
$(KBE_CONFIG)/conf_mod.o: ../../include/openssl/conf.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/conf_mod.o: ../../include/openssl/dso.h ../../include/openssl/e_os2.h
$(KBE_CONFIG)/conf_mod.o: ../../include/openssl/ec.h ../../include/openssl/ecdh.h
$(KBE_CONFIG)/conf_mod.o: ../../include/openssl/ecdsa.h ../../include/openssl/err.h
$(KBE_CONFIG)/conf_mod.o: ../../include/openssl/evp.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/conf_mod.o: ../../include/openssl/obj_mac.h ../../include/openssl/objects.h
$(KBE_CONFIG)/conf_mod.o: ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/conf_mod.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/conf_mod.o: ../../include/openssl/pkcs7.h ../../include/openssl/safestack.h
$(KBE_CONFIG)/conf_mod.o: ../../include/openssl/sha.h ../../include/openssl/stack.h
$(KBE_CONFIG)/conf_mod.o: ../../include/openssl/symhacks.h ../../include/openssl/x509.h
$(KBE_CONFIG)/conf_mod.o: ../../include/openssl/x509_vfy.h ../cryptlib.h conf_mod.c
$(KBE_CONFIG)/conf_sap.o: ../../e_os.h ../../include/openssl/asn1.h
$(KBE_CONFIG)/conf_sap.o: ../../include/openssl/bio.h ../../include/openssl/buffer.h
$(KBE_CONFIG)/conf_sap.o: ../../include/openssl/conf.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/conf_sap.o: ../../include/openssl/dso.h ../../include/openssl/e_os2.h
$(KBE_CONFIG)/conf_sap.o: ../../include/openssl/ec.h ../../include/openssl/ecdh.h
$(KBE_CONFIG)/conf_sap.o: ../../include/openssl/ecdsa.h ../../include/openssl/engine.h
$(KBE_CONFIG)/conf_sap.o: ../../include/openssl/err.h ../../include/openssl/evp.h
$(KBE_CONFIG)/conf_sap.o: ../../include/openssl/lhash.h ../../include/openssl/obj_mac.h
$(KBE_CONFIG)/conf_sap.o: ../../include/openssl/objects.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/conf_sap.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/conf_sap.o: ../../include/openssl/pkcs7.h ../../include/openssl/safestack.h
$(KBE_CONFIG)/conf_sap.o: ../../include/openssl/sha.h ../../include/openssl/stack.h
$(KBE_CONFIG)/conf_sap.o: ../../include/openssl/symhacks.h ../../include/openssl/x509.h
$(KBE_CONFIG)/conf_sap.o: ../../include/openssl/x509_vfy.h ../cryptlib.h conf_sap.c
