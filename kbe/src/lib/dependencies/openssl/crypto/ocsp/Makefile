#
# OpenSSL/ocsp/Makefile
#


ifndef KBE_ROOT
export KBE_ROOT := $(subst /kbe/src/lib/dependencies/openssl/crypto/ocsp,,$(CURDIR))
endif

LIBDIR= $(KBE_ROOT)/kbe/src/libs



DIR=	ocsp
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
LIBSRC= ocsp_asn.c ocsp_ext.c ocsp_ht.c ocsp_lib.c ocsp_cl.c \
	ocsp_srv.c ocsp_prn.c ocsp_vfy.c ocsp_err.c

LIBOBJ= $(KBE_CONFIG)/ocsp_asn.o $(KBE_CONFIG)/ocsp_ext.o $(KBE_CONFIG)/ocsp_ht.o \
	$(KBE_CONFIG)/ocsp_lib.o $(KBE_CONFIG)/ocsp_cl.o $(KBE_CONFIG)/ocsp_srv.o \
	$(KBE_CONFIG)/ocsp_prn.o $(KBE_CONFIG)/ocsp_vfy.o $(KBE_CONFIG)/ocsp_err.o

SRC= $(LIBSRC)

EXHEADER= ocsp.h
HEADER=	$(EXHEADER)

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

$(KBE_CONFIG)/ocsp_asn.o: ../../include/openssl/asn1.h ../../include/openssl/asn1t.h
$(KBE_CONFIG)/ocsp_asn.o: ../../include/openssl/bio.h ../../include/openssl/buffer.h
$(KBE_CONFIG)/ocsp_asn.o: ../../include/openssl/conf.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/ocsp_asn.o: ../../include/openssl/e_os2.h ../../include/openssl/ec.h
$(KBE_CONFIG)/ocsp_asn.o: ../../include/openssl/ecdh.h ../../include/openssl/ecdsa.h
$(KBE_CONFIG)/ocsp_asn.o: ../../include/openssl/evp.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/ocsp_asn.o: ../../include/openssl/obj_mac.h ../../include/openssl/objects.h
$(KBE_CONFIG)/ocsp_asn.o: ../../include/openssl/ocsp.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/ocsp_asn.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/ocsp_asn.o: ../../include/openssl/pkcs7.h ../../include/openssl/safestack.h
$(KBE_CONFIG)/ocsp_asn.o: ../../include/openssl/sha.h ../../include/openssl/stack.h
$(KBE_CONFIG)/ocsp_asn.o: ../../include/openssl/symhacks.h ../../include/openssl/x509.h
$(KBE_CONFIG)/ocsp_asn.o: ../../include/openssl/x509_vfy.h ../../include/openssl/x509v3.h
$(KBE_CONFIG)/ocsp_asn.o: ocsp_asn.c
$(KBE_CONFIG)/ocsp_cl.o: ../../e_os.h ../../include/openssl/asn1.h
$(KBE_CONFIG)/ocsp_cl.o: ../../include/openssl/bio.h ../../include/openssl/buffer.h
$(KBE_CONFIG)/ocsp_cl.o: ../../include/openssl/conf.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/ocsp_cl.o: ../../include/openssl/e_os2.h ../../include/openssl/ec.h
$(KBE_CONFIG)/ocsp_cl.o: ../../include/openssl/ecdh.h ../../include/openssl/ecdsa.h
$(KBE_CONFIG)/ocsp_cl.o: ../../include/openssl/err.h ../../include/openssl/evp.h
$(KBE_CONFIG)/ocsp_cl.o: ../../include/openssl/lhash.h ../../include/openssl/obj_mac.h
$(KBE_CONFIG)/ocsp_cl.o: ../../include/openssl/objects.h ../../include/openssl/ocsp.h
$(KBE_CONFIG)/ocsp_cl.o: ../../include/openssl/opensslconf.h ../../include/openssl/opensslv.h
$(KBE_CONFIG)/ocsp_cl.o: ../../include/openssl/ossl_typ.h ../../include/openssl/pem.h
$(KBE_CONFIG)/ocsp_cl.o: ../../include/openssl/pem2.h ../../include/openssl/pkcs7.h
$(KBE_CONFIG)/ocsp_cl.o: ../../include/openssl/rand.h ../../include/openssl/safestack.h
$(KBE_CONFIG)/ocsp_cl.o: ../../include/openssl/sha.h ../../include/openssl/stack.h
$(KBE_CONFIG)/ocsp_cl.o: ../../include/openssl/symhacks.h ../../include/openssl/x509.h
$(KBE_CONFIG)/ocsp_cl.o: ../../include/openssl/x509_vfy.h ../../include/openssl/x509v3.h
$(KBE_CONFIG)/ocsp_cl.o: ../cryptlib.h ocsp_cl.c
$(KBE_CONFIG)/ocsp_err.o: ../../include/openssl/asn1.h ../../include/openssl/bio.h
$(KBE_CONFIG)/ocsp_err.o: ../../include/openssl/buffer.h ../../include/openssl/conf.h
$(KBE_CONFIG)/ocsp_err.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(KBE_CONFIG)/ocsp_err.o: ../../include/openssl/ec.h ../../include/openssl/ecdh.h
$(KBE_CONFIG)/ocsp_err.o: ../../include/openssl/ecdsa.h ../../include/openssl/err.h
$(KBE_CONFIG)/ocsp_err.o: ../../include/openssl/evp.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/ocsp_err.o: ../../include/openssl/obj_mac.h ../../include/openssl/objects.h
$(KBE_CONFIG)/ocsp_err.o: ../../include/openssl/ocsp.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/ocsp_err.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/ocsp_err.o: ../../include/openssl/pkcs7.h ../../include/openssl/safestack.h
$(KBE_CONFIG)/ocsp_err.o: ../../include/openssl/sha.h ../../include/openssl/stack.h
$(KBE_CONFIG)/ocsp_err.o: ../../include/openssl/symhacks.h ../../include/openssl/x509.h
$(KBE_CONFIG)/ocsp_err.o: ../../include/openssl/x509_vfy.h ../../include/openssl/x509v3.h
$(KBE_CONFIG)/ocsp_err.o: ocsp_err.c
$(KBE_CONFIG)/ocsp_ext.o: ../../e_os.h ../../include/openssl/asn1.h
$(KBE_CONFIG)/ocsp_ext.o: ../../include/openssl/bio.h ../../include/openssl/buffer.h
$(KBE_CONFIG)/ocsp_ext.o: ../../include/openssl/conf.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/ocsp_ext.o: ../../include/openssl/e_os2.h ../../include/openssl/ec.h
$(KBE_CONFIG)/ocsp_ext.o: ../../include/openssl/ecdh.h ../../include/openssl/ecdsa.h
$(KBE_CONFIG)/ocsp_ext.o: ../../include/openssl/err.h ../../include/openssl/evp.h
$(KBE_CONFIG)/ocsp_ext.o: ../../include/openssl/lhash.h ../../include/openssl/obj_mac.h
$(KBE_CONFIG)/ocsp_ext.o: ../../include/openssl/objects.h ../../include/openssl/ocsp.h
$(KBE_CONFIG)/ocsp_ext.o: ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/ocsp_ext.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/ocsp_ext.o: ../../include/openssl/pkcs7.h ../../include/openssl/rand.h
$(KBE_CONFIG)/ocsp_ext.o: ../../include/openssl/safestack.h ../../include/openssl/sha.h
$(KBE_CONFIG)/ocsp_ext.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
$(KBE_CONFIG)/ocsp_ext.o: ../../include/openssl/x509.h ../../include/openssl/x509_vfy.h
$(KBE_CONFIG)/ocsp_ext.o: ../../include/openssl/x509v3.h ../cryptlib.h ocsp_ext.c
$(KBE_CONFIG)/ocsp_ht.o: ../../include/openssl/asn1.h ../../include/openssl/bio.h
$(KBE_CONFIG)/ocsp_ht.o: ../../include/openssl/buffer.h ../../include/openssl/conf.h
$(KBE_CONFIG)/ocsp_ht.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(KBE_CONFIG)/ocsp_ht.o: ../../include/openssl/ec.h ../../include/openssl/ecdh.h
$(KBE_CONFIG)/ocsp_ht.o: ../../include/openssl/ecdsa.h ../../include/openssl/err.h
$(KBE_CONFIG)/ocsp_ht.o: ../../include/openssl/evp.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/ocsp_ht.o: ../../include/openssl/obj_mac.h ../../include/openssl/objects.h
$(KBE_CONFIG)/ocsp_ht.o: ../../include/openssl/ocsp.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/ocsp_ht.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/ocsp_ht.o: ../../include/openssl/pkcs7.h ../../include/openssl/safestack.h
$(KBE_CONFIG)/ocsp_ht.o: ../../include/openssl/sha.h ../../include/openssl/stack.h
$(KBE_CONFIG)/ocsp_ht.o: ../../include/openssl/symhacks.h ../../include/openssl/x509.h
$(KBE_CONFIG)/ocsp_ht.o: ../../include/openssl/x509_vfy.h ../../include/openssl/x509v3.h
$(KBE_CONFIG)/ocsp_ht.o: ocsp_ht.c
$(KBE_CONFIG)/ocsp_lib.o: ../../e_os.h ../../include/openssl/asn1.h
$(KBE_CONFIG)/ocsp_lib.o: ../../include/openssl/bio.h ../../include/openssl/buffer.h
$(KBE_CONFIG)/ocsp_lib.o: ../../include/openssl/conf.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/ocsp_lib.o: ../../include/openssl/e_os2.h ../../include/openssl/ec.h
$(KBE_CONFIG)/ocsp_lib.o: ../../include/openssl/ecdh.h ../../include/openssl/ecdsa.h
$(KBE_CONFIG)/ocsp_lib.o: ../../include/openssl/err.h ../../include/openssl/evp.h
$(KBE_CONFIG)/ocsp_lib.o: ../../include/openssl/lhash.h ../../include/openssl/obj_mac.h
$(KBE_CONFIG)/ocsp_lib.o: ../../include/openssl/objects.h ../../include/openssl/ocsp.h
$(KBE_CONFIG)/ocsp_lib.o: ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/ocsp_lib.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/ocsp_lib.o: ../../include/openssl/pem.h ../../include/openssl/pem2.h
$(KBE_CONFIG)/ocsp_lib.o: ../../include/openssl/pkcs7.h ../../include/openssl/rand.h
$(KBE_CONFIG)/ocsp_lib.o: ../../include/openssl/safestack.h ../../include/openssl/sha.h
$(KBE_CONFIG)/ocsp_lib.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
$(KBE_CONFIG)/ocsp_lib.o: ../../include/openssl/x509.h ../../include/openssl/x509_vfy.h
$(KBE_CONFIG)/ocsp_lib.o: ../../include/openssl/x509v3.h ../cryptlib.h ocsp_lib.c
$(KBE_CONFIG)/ocsp_prn.o: ../../include/openssl/asn1.h ../../include/openssl/bio.h
$(KBE_CONFIG)/ocsp_prn.o: ../../include/openssl/buffer.h ../../include/openssl/conf.h
$(KBE_CONFIG)/ocsp_prn.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(KBE_CONFIG)/ocsp_prn.o: ../../include/openssl/ec.h ../../include/openssl/ecdh.h
$(KBE_CONFIG)/ocsp_prn.o: ../../include/openssl/ecdsa.h ../../include/openssl/err.h
$(KBE_CONFIG)/ocsp_prn.o: ../../include/openssl/evp.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/ocsp_prn.o: ../../include/openssl/obj_mac.h ../../include/openssl/objects.h
$(KBE_CONFIG)/ocsp_prn.o: ../../include/openssl/ocsp.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/ocsp_prn.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/ocsp_prn.o: ../../include/openssl/pem.h ../../include/openssl/pem2.h
$(KBE_CONFIG)/ocsp_prn.o: ../../include/openssl/pkcs7.h ../../include/openssl/safestack.h
$(KBE_CONFIG)/ocsp_prn.o: ../../include/openssl/sha.h ../../include/openssl/stack.h
$(KBE_CONFIG)/ocsp_prn.o: ../../include/openssl/symhacks.h ../../include/openssl/x509.h
$(KBE_CONFIG)/ocsp_prn.o: ../../include/openssl/x509_vfy.h ../../include/openssl/x509v3.h
$(KBE_CONFIG)/ocsp_prn.o: ocsp_prn.c
$(KBE_CONFIG)/ocsp_srv.o: ../../e_os.h ../../include/openssl/asn1.h
$(KBE_CONFIG)/ocsp_srv.o: ../../include/openssl/bio.h ../../include/openssl/buffer.h
$(KBE_CONFIG)/ocsp_srv.o: ../../include/openssl/conf.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/ocsp_srv.o: ../../include/openssl/e_os2.h ../../include/openssl/ec.h
$(KBE_CONFIG)/ocsp_srv.o: ../../include/openssl/ecdh.h ../../include/openssl/ecdsa.h
$(KBE_CONFIG)/ocsp_srv.o: ../../include/openssl/err.h ../../include/openssl/evp.h
$(KBE_CONFIG)/ocsp_srv.o: ../../include/openssl/lhash.h ../../include/openssl/obj_mac.h
$(KBE_CONFIG)/ocsp_srv.o: ../../include/openssl/objects.h ../../include/openssl/ocsp.h
$(KBE_CONFIG)/ocsp_srv.o: ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/ocsp_srv.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/ocsp_srv.o: ../../include/openssl/pem.h ../../include/openssl/pem2.h
$(KBE_CONFIG)/ocsp_srv.o: ../../include/openssl/pkcs7.h ../../include/openssl/rand.h
$(KBE_CONFIG)/ocsp_srv.o: ../../include/openssl/safestack.h ../../include/openssl/sha.h
$(KBE_CONFIG)/ocsp_srv.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
$(KBE_CONFIG)/ocsp_srv.o: ../../include/openssl/x509.h ../../include/openssl/x509_vfy.h
$(KBE_CONFIG)/ocsp_srv.o: ../../include/openssl/x509v3.h ../cryptlib.h ocsp_srv.c
$(KBE_CONFIG)/ocsp_vfy.o: ../../include/openssl/asn1.h ../../include/openssl/bio.h
$(KBE_CONFIG)/ocsp_vfy.o: ../../include/openssl/buffer.h ../../include/openssl/conf.h
$(KBE_CONFIG)/ocsp_vfy.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(KBE_CONFIG)/ocsp_vfy.o: ../../include/openssl/ec.h ../../include/openssl/ecdh.h
$(KBE_CONFIG)/ocsp_vfy.o: ../../include/openssl/ecdsa.h ../../include/openssl/err.h
$(KBE_CONFIG)/ocsp_vfy.o: ../../include/openssl/evp.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/ocsp_vfy.o: ../../include/openssl/obj_mac.h ../../include/openssl/objects.h
$(KBE_CONFIG)/ocsp_vfy.o: ../../include/openssl/ocsp.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/ocsp_vfy.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/ocsp_vfy.o: ../../include/openssl/pkcs7.h ../../include/openssl/safestack.h
$(KBE_CONFIG)/ocsp_vfy.o: ../../include/openssl/sha.h ../../include/openssl/stack.h
$(KBE_CONFIG)/ocsp_vfy.o: ../../include/openssl/symhacks.h ../../include/openssl/x509.h
$(KBE_CONFIG)/ocsp_vfy.o: ../../include/openssl/x509_vfy.h ../../include/openssl/x509v3.h
$(KBE_CONFIG)/ocsp_vfy.o: ocsp_vfy.c
