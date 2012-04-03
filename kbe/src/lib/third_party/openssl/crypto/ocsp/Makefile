#
# OpenSSL/ocsp/Makefile
#

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

LIB=$(BWLIBDIR)/libcrypto.a
LIBSRC= ocsp_asn.c ocsp_ext.c ocsp_ht.c ocsp_lib.c ocsp_cl.c \
	ocsp_srv.c ocsp_prn.c ocsp_vfy.c ocsp_err.c

LIBOBJ= $(MF_CONFIG)/ocsp_asn.o $(MF_CONFIG)/ocsp_ext.o $(MF_CONFIG)/ocsp_ht.o \
	$(MF_CONFIG)/ocsp_lib.o $(MF_CONFIG)/ocsp_cl.o $(MF_CONFIG)/ocsp_srv.o \
	$(MF_CONFIG)/ocsp_prn.o $(MF_CONFIG)/ocsp_vfy.o $(MF_CONFIG)/ocsp_err.o

SRC= $(LIBSRC)

EXHEADER= ocsp.h
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

$(MF_CONFIG)/ocsp_asn.o: ../../include/openssl/asn1.h ../../include/openssl/asn1t.h
$(MF_CONFIG)/ocsp_asn.o: ../../include/openssl/bio.h ../../include/openssl/buffer.h
$(MF_CONFIG)/ocsp_asn.o: ../../include/openssl/conf.h ../../include/openssl/crypto.h
$(MF_CONFIG)/ocsp_asn.o: ../../include/openssl/e_os2.h ../../include/openssl/ec.h
$(MF_CONFIG)/ocsp_asn.o: ../../include/openssl/ecdh.h ../../include/openssl/ecdsa.h
$(MF_CONFIG)/ocsp_asn.o: ../../include/openssl/evp.h ../../include/openssl/lhash.h
$(MF_CONFIG)/ocsp_asn.o: ../../include/openssl/obj_mac.h ../../include/openssl/objects.h
$(MF_CONFIG)/ocsp_asn.o: ../../include/openssl/ocsp.h ../../include/openssl/opensslconf.h
$(MF_CONFIG)/ocsp_asn.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(MF_CONFIG)/ocsp_asn.o: ../../include/openssl/pkcs7.h ../../include/openssl/safestack.h
$(MF_CONFIG)/ocsp_asn.o: ../../include/openssl/sha.h ../../include/openssl/stack.h
$(MF_CONFIG)/ocsp_asn.o: ../../include/openssl/symhacks.h ../../include/openssl/x509.h
$(MF_CONFIG)/ocsp_asn.o: ../../include/openssl/x509_vfy.h ../../include/openssl/x509v3.h
$(MF_CONFIG)/ocsp_asn.o: ocsp_asn.c
$(MF_CONFIG)/ocsp_cl.o: ../../e_os.h ../../include/openssl/asn1.h
$(MF_CONFIG)/ocsp_cl.o: ../../include/openssl/bio.h ../../include/openssl/buffer.h
$(MF_CONFIG)/ocsp_cl.o: ../../include/openssl/conf.h ../../include/openssl/crypto.h
$(MF_CONFIG)/ocsp_cl.o: ../../include/openssl/e_os2.h ../../include/openssl/ec.h
$(MF_CONFIG)/ocsp_cl.o: ../../include/openssl/ecdh.h ../../include/openssl/ecdsa.h
$(MF_CONFIG)/ocsp_cl.o: ../../include/openssl/err.h ../../include/openssl/evp.h
$(MF_CONFIG)/ocsp_cl.o: ../../include/openssl/lhash.h ../../include/openssl/obj_mac.h
$(MF_CONFIG)/ocsp_cl.o: ../../include/openssl/objects.h ../../include/openssl/ocsp.h
$(MF_CONFIG)/ocsp_cl.o: ../../include/openssl/opensslconf.h ../../include/openssl/opensslv.h
$(MF_CONFIG)/ocsp_cl.o: ../../include/openssl/ossl_typ.h ../../include/openssl/pem.h
$(MF_CONFIG)/ocsp_cl.o: ../../include/openssl/pem2.h ../../include/openssl/pkcs7.h
$(MF_CONFIG)/ocsp_cl.o: ../../include/openssl/rand.h ../../include/openssl/safestack.h
$(MF_CONFIG)/ocsp_cl.o: ../../include/openssl/sha.h ../../include/openssl/stack.h
$(MF_CONFIG)/ocsp_cl.o: ../../include/openssl/symhacks.h ../../include/openssl/x509.h
$(MF_CONFIG)/ocsp_cl.o: ../../include/openssl/x509_vfy.h ../../include/openssl/x509v3.h
$(MF_CONFIG)/ocsp_cl.o: ../cryptlib.h ocsp_cl.c
$(MF_CONFIG)/ocsp_err.o: ../../include/openssl/asn1.h ../../include/openssl/bio.h
$(MF_CONFIG)/ocsp_err.o: ../../include/openssl/buffer.h ../../include/openssl/conf.h
$(MF_CONFIG)/ocsp_err.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(MF_CONFIG)/ocsp_err.o: ../../include/openssl/ec.h ../../include/openssl/ecdh.h
$(MF_CONFIG)/ocsp_err.o: ../../include/openssl/ecdsa.h ../../include/openssl/err.h
$(MF_CONFIG)/ocsp_err.o: ../../include/openssl/evp.h ../../include/openssl/lhash.h
$(MF_CONFIG)/ocsp_err.o: ../../include/openssl/obj_mac.h ../../include/openssl/objects.h
$(MF_CONFIG)/ocsp_err.o: ../../include/openssl/ocsp.h ../../include/openssl/opensslconf.h
$(MF_CONFIG)/ocsp_err.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(MF_CONFIG)/ocsp_err.o: ../../include/openssl/pkcs7.h ../../include/openssl/safestack.h
$(MF_CONFIG)/ocsp_err.o: ../../include/openssl/sha.h ../../include/openssl/stack.h
$(MF_CONFIG)/ocsp_err.o: ../../include/openssl/symhacks.h ../../include/openssl/x509.h
$(MF_CONFIG)/ocsp_err.o: ../../include/openssl/x509_vfy.h ../../include/openssl/x509v3.h
$(MF_CONFIG)/ocsp_err.o: ocsp_err.c
$(MF_CONFIG)/ocsp_ext.o: ../../e_os.h ../../include/openssl/asn1.h
$(MF_CONFIG)/ocsp_ext.o: ../../include/openssl/bio.h ../../include/openssl/buffer.h
$(MF_CONFIG)/ocsp_ext.o: ../../include/openssl/conf.h ../../include/openssl/crypto.h
$(MF_CONFIG)/ocsp_ext.o: ../../include/openssl/e_os2.h ../../include/openssl/ec.h
$(MF_CONFIG)/ocsp_ext.o: ../../include/openssl/ecdh.h ../../include/openssl/ecdsa.h
$(MF_CONFIG)/ocsp_ext.o: ../../include/openssl/err.h ../../include/openssl/evp.h
$(MF_CONFIG)/ocsp_ext.o: ../../include/openssl/lhash.h ../../include/openssl/obj_mac.h
$(MF_CONFIG)/ocsp_ext.o: ../../include/openssl/objects.h ../../include/openssl/ocsp.h
$(MF_CONFIG)/ocsp_ext.o: ../../include/openssl/opensslconf.h
$(MF_CONFIG)/ocsp_ext.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(MF_CONFIG)/ocsp_ext.o: ../../include/openssl/pkcs7.h ../../include/openssl/rand.h
$(MF_CONFIG)/ocsp_ext.o: ../../include/openssl/safestack.h ../../include/openssl/sha.h
$(MF_CONFIG)/ocsp_ext.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
$(MF_CONFIG)/ocsp_ext.o: ../../include/openssl/x509.h ../../include/openssl/x509_vfy.h
$(MF_CONFIG)/ocsp_ext.o: ../../include/openssl/x509v3.h ../cryptlib.h ocsp_ext.c
$(MF_CONFIG)/ocsp_ht.o: ../../include/openssl/asn1.h ../../include/openssl/bio.h
$(MF_CONFIG)/ocsp_ht.o: ../../include/openssl/buffer.h ../../include/openssl/conf.h
$(MF_CONFIG)/ocsp_ht.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(MF_CONFIG)/ocsp_ht.o: ../../include/openssl/ec.h ../../include/openssl/ecdh.h
$(MF_CONFIG)/ocsp_ht.o: ../../include/openssl/ecdsa.h ../../include/openssl/err.h
$(MF_CONFIG)/ocsp_ht.o: ../../include/openssl/evp.h ../../include/openssl/lhash.h
$(MF_CONFIG)/ocsp_ht.o: ../../include/openssl/obj_mac.h ../../include/openssl/objects.h
$(MF_CONFIG)/ocsp_ht.o: ../../include/openssl/ocsp.h ../../include/openssl/opensslconf.h
$(MF_CONFIG)/ocsp_ht.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(MF_CONFIG)/ocsp_ht.o: ../../include/openssl/pkcs7.h ../../include/openssl/safestack.h
$(MF_CONFIG)/ocsp_ht.o: ../../include/openssl/sha.h ../../include/openssl/stack.h
$(MF_CONFIG)/ocsp_ht.o: ../../include/openssl/symhacks.h ../../include/openssl/x509.h
$(MF_CONFIG)/ocsp_ht.o: ../../include/openssl/x509_vfy.h ../../include/openssl/x509v3.h
$(MF_CONFIG)/ocsp_ht.o: ocsp_ht.c
$(MF_CONFIG)/ocsp_lib.o: ../../e_os.h ../../include/openssl/asn1.h
$(MF_CONFIG)/ocsp_lib.o: ../../include/openssl/bio.h ../../include/openssl/buffer.h
$(MF_CONFIG)/ocsp_lib.o: ../../include/openssl/conf.h ../../include/openssl/crypto.h
$(MF_CONFIG)/ocsp_lib.o: ../../include/openssl/e_os2.h ../../include/openssl/ec.h
$(MF_CONFIG)/ocsp_lib.o: ../../include/openssl/ecdh.h ../../include/openssl/ecdsa.h
$(MF_CONFIG)/ocsp_lib.o: ../../include/openssl/err.h ../../include/openssl/evp.h
$(MF_CONFIG)/ocsp_lib.o: ../../include/openssl/lhash.h ../../include/openssl/obj_mac.h
$(MF_CONFIG)/ocsp_lib.o: ../../include/openssl/objects.h ../../include/openssl/ocsp.h
$(MF_CONFIG)/ocsp_lib.o: ../../include/openssl/opensslconf.h
$(MF_CONFIG)/ocsp_lib.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(MF_CONFIG)/ocsp_lib.o: ../../include/openssl/pem.h ../../include/openssl/pem2.h
$(MF_CONFIG)/ocsp_lib.o: ../../include/openssl/pkcs7.h ../../include/openssl/rand.h
$(MF_CONFIG)/ocsp_lib.o: ../../include/openssl/safestack.h ../../include/openssl/sha.h
$(MF_CONFIG)/ocsp_lib.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
$(MF_CONFIG)/ocsp_lib.o: ../../include/openssl/x509.h ../../include/openssl/x509_vfy.h
$(MF_CONFIG)/ocsp_lib.o: ../../include/openssl/x509v3.h ../cryptlib.h ocsp_lib.c
$(MF_CONFIG)/ocsp_prn.o: ../../include/openssl/asn1.h ../../include/openssl/bio.h
$(MF_CONFIG)/ocsp_prn.o: ../../include/openssl/buffer.h ../../include/openssl/conf.h
$(MF_CONFIG)/ocsp_prn.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(MF_CONFIG)/ocsp_prn.o: ../../include/openssl/ec.h ../../include/openssl/ecdh.h
$(MF_CONFIG)/ocsp_prn.o: ../../include/openssl/ecdsa.h ../../include/openssl/err.h
$(MF_CONFIG)/ocsp_prn.o: ../../include/openssl/evp.h ../../include/openssl/lhash.h
$(MF_CONFIG)/ocsp_prn.o: ../../include/openssl/obj_mac.h ../../include/openssl/objects.h
$(MF_CONFIG)/ocsp_prn.o: ../../include/openssl/ocsp.h ../../include/openssl/opensslconf.h
$(MF_CONFIG)/ocsp_prn.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(MF_CONFIG)/ocsp_prn.o: ../../include/openssl/pem.h ../../include/openssl/pem2.h
$(MF_CONFIG)/ocsp_prn.o: ../../include/openssl/pkcs7.h ../../include/openssl/safestack.h
$(MF_CONFIG)/ocsp_prn.o: ../../include/openssl/sha.h ../../include/openssl/stack.h
$(MF_CONFIG)/ocsp_prn.o: ../../include/openssl/symhacks.h ../../include/openssl/x509.h
$(MF_CONFIG)/ocsp_prn.o: ../../include/openssl/x509_vfy.h ../../include/openssl/x509v3.h
$(MF_CONFIG)/ocsp_prn.o: ocsp_prn.c
$(MF_CONFIG)/ocsp_srv.o: ../../e_os.h ../../include/openssl/asn1.h
$(MF_CONFIG)/ocsp_srv.o: ../../include/openssl/bio.h ../../include/openssl/buffer.h
$(MF_CONFIG)/ocsp_srv.o: ../../include/openssl/conf.h ../../include/openssl/crypto.h
$(MF_CONFIG)/ocsp_srv.o: ../../include/openssl/e_os2.h ../../include/openssl/ec.h
$(MF_CONFIG)/ocsp_srv.o: ../../include/openssl/ecdh.h ../../include/openssl/ecdsa.h
$(MF_CONFIG)/ocsp_srv.o: ../../include/openssl/err.h ../../include/openssl/evp.h
$(MF_CONFIG)/ocsp_srv.o: ../../include/openssl/lhash.h ../../include/openssl/obj_mac.h
$(MF_CONFIG)/ocsp_srv.o: ../../include/openssl/objects.h ../../include/openssl/ocsp.h
$(MF_CONFIG)/ocsp_srv.o: ../../include/openssl/opensslconf.h
$(MF_CONFIG)/ocsp_srv.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(MF_CONFIG)/ocsp_srv.o: ../../include/openssl/pem.h ../../include/openssl/pem2.h
$(MF_CONFIG)/ocsp_srv.o: ../../include/openssl/pkcs7.h ../../include/openssl/rand.h
$(MF_CONFIG)/ocsp_srv.o: ../../include/openssl/safestack.h ../../include/openssl/sha.h
$(MF_CONFIG)/ocsp_srv.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
$(MF_CONFIG)/ocsp_srv.o: ../../include/openssl/x509.h ../../include/openssl/x509_vfy.h
$(MF_CONFIG)/ocsp_srv.o: ../../include/openssl/x509v3.h ../cryptlib.h ocsp_srv.c
$(MF_CONFIG)/ocsp_vfy.o: ../../include/openssl/asn1.h ../../include/openssl/bio.h
$(MF_CONFIG)/ocsp_vfy.o: ../../include/openssl/buffer.h ../../include/openssl/conf.h
$(MF_CONFIG)/ocsp_vfy.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(MF_CONFIG)/ocsp_vfy.o: ../../include/openssl/ec.h ../../include/openssl/ecdh.h
$(MF_CONFIG)/ocsp_vfy.o: ../../include/openssl/ecdsa.h ../../include/openssl/err.h
$(MF_CONFIG)/ocsp_vfy.o: ../../include/openssl/evp.h ../../include/openssl/lhash.h
$(MF_CONFIG)/ocsp_vfy.o: ../../include/openssl/obj_mac.h ../../include/openssl/objects.h
$(MF_CONFIG)/ocsp_vfy.o: ../../include/openssl/ocsp.h ../../include/openssl/opensslconf.h
$(MF_CONFIG)/ocsp_vfy.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(MF_CONFIG)/ocsp_vfy.o: ../../include/openssl/pkcs7.h ../../include/openssl/safestack.h
$(MF_CONFIG)/ocsp_vfy.o: ../../include/openssl/sha.h ../../include/openssl/stack.h
$(MF_CONFIG)/ocsp_vfy.o: ../../include/openssl/symhacks.h ../../include/openssl/x509.h
$(MF_CONFIG)/ocsp_vfy.o: ../../include/openssl/x509_vfy.h ../../include/openssl/x509v3.h
$(MF_CONFIG)/ocsp_vfy.o: ocsp_vfy.c
