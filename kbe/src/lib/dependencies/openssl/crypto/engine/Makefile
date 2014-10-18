#
# OpenSSL/crypto/engine/Makefile
#

ifndef KBE_ROOT
export KBE_ROOT := $(subst /kbe/src/lib/dependencies/openssl/crypto/engine,,$(CURDIR))
endif

LIBDIR= $(KBE_ROOT)/kbe/src/libs

DIR=	engine
TOP=	../..
CC=	cc
INCLUDES= -I.. -I$(TOP) -I../../include
CFLAG=-g
MAKEFILE=	Makefile
AR=		ar r

CFLAGS= $(INCLUDES) $(CFLAG)

GENERAL=Makefile
TEST= enginetest.c
APPS=

LIB=$(LIBDIR)/libcrypto.a
LIBSRC= eng_err.c eng_lib.c eng_list.c eng_init.c eng_ctrl.c \
	eng_table.c eng_pkey.c eng_fat.c eng_all.c \
	tb_rsa.c tb_dsa.c tb_ecdsa.c tb_dh.c tb_ecdh.c tb_rand.c tb_store.c \
	tb_cipher.c tb_digest.c \
	eng_openssl.c eng_cnf.c eng_dyn.c eng_cryptodev.c eng_padlock.c
LIBOBJ= $(KBE_CONFIG)/eng_err.o $(KBE_CONFIG)/eng_lib.o $(KBE_CONFIG)/eng_list.o \
	$(KBE_CONFIG)/eng_init.o $(KBE_CONFIG)/eng_ctrl.o $(KBE_CONFIG)/eng_table.o \
	$(KBE_CONFIG)/eng_pkey.o $(KBE_CONFIG)/eng_fat.o $(KBE_CONFIG)/eng_all.o \
	$(KBE_CONFIG)/tb_rsa.o $(KBE_CONFIG)/tb_dsa.o $(KBE_CONFIG)/tb_ecdsa.o \
	$(KBE_CONFIG)/tb_dh.o $(KBE_CONFIG)/tb_ecdh.o $(KBE_CONFIG)/tb_rand.o \
	$(KBE_CONFIG)/tb_store.o $(KBE_CONFIG)/tb_cipher.o $(KBE_CONFIG)/tb_digest.o \
	$(KBE_CONFIG)/eng_openssl.o $(KBE_CONFIG)/eng_cnf.o $(KBE_CONFIG)/eng_dyn.o \
	$(KBE_CONFIG)/eng_cryptodev.o $(KBE_CONFIG)/eng_padlock.o

SRC= $(LIBSRC)

EXHEADER= engine.h
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
	$(MAKEDEPEND) -- $(CFLAG) $(INCLUDES) $(DEPFLAG) -- $(PROGS) $(LIBSRC)

dclean:
	$(PERL) -pe 'if (/^# DO NOT DELETE THIS LINE/) {print; exit(0);}' $(MAKEFILE) >Makefile.new
	mv -f Makefile.new $(MAKEFILE)

clean:
	rm -f $(KBE_CONFIG)/*.o *.o */*.o *.obj lib tags core .pure .nfs* *.old *.bak fluff

# DO NOT DELETE THIS LINE -- make depend depends on it.

$(KBE_CONFIG)/eng_all.o: ../../e_os.h ../../include/openssl/bio.h
$(KBE_CONFIG)/eng_all.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/eng_all.o: ../../include/openssl/e_os2.h ../../include/openssl/engine.h
$(KBE_CONFIG)/eng_all.o: ../../include/openssl/err.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/eng_all.o: ../../include/openssl/opensslconf.h ../../include/openssl/opensslv.h
$(KBE_CONFIG)/eng_all.o: ../../include/openssl/ossl_typ.h ../../include/openssl/safestack.h
$(KBE_CONFIG)/eng_all.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
$(KBE_CONFIG)/eng_all.o: ../cryptlib.h eng_all.c eng_int.h
$(KBE_CONFIG)/eng_cnf.o: ../../e_os.h ../../include/openssl/bio.h
$(KBE_CONFIG)/eng_cnf.o: ../../include/openssl/buffer.h ../../include/openssl/conf.h
$(KBE_CONFIG)/eng_cnf.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(KBE_CONFIG)/eng_cnf.o: ../../include/openssl/engine.h ../../include/openssl/err.h
$(KBE_CONFIG)/eng_cnf.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/eng_cnf.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/eng_cnf.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/eng_cnf.o: ../../include/openssl/symhacks.h ../cryptlib.h eng_cnf.c eng_int.h
$(KBE_CONFIG)/eng_cryptodev.o: ../../include/openssl/asn1.h ../../include/openssl/bio.h
$(KBE_CONFIG)/eng_cryptodev.o: ../../include/openssl/bn.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/eng_cryptodev.o: ../../include/openssl/e_os2.h ../../include/openssl/engine.h
$(KBE_CONFIG)/eng_cryptodev.o: ../../include/openssl/evp.h ../../include/openssl/obj_mac.h
$(KBE_CONFIG)/eng_cryptodev.o: ../../include/openssl/objects.h
$(KBE_CONFIG)/eng_cryptodev.o: ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/eng_cryptodev.o: ../../include/openssl/opensslv.h
$(KBE_CONFIG)/eng_cryptodev.o: ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/eng_cryptodev.o: ../../include/openssl/safestack.h
$(KBE_CONFIG)/eng_cryptodev.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
$(KBE_CONFIG)/eng_cryptodev.o: eng_cryptodev.c
$(KBE_CONFIG)/eng_ctrl.o: ../../e_os.h ../../include/openssl/bio.h
$(KBE_CONFIG)/eng_ctrl.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/eng_ctrl.o: ../../include/openssl/e_os2.h ../../include/openssl/engine.h
$(KBE_CONFIG)/eng_ctrl.o: ../../include/openssl/err.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/eng_ctrl.o: ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/eng_ctrl.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/eng_ctrl.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/eng_ctrl.o: ../../include/openssl/symhacks.h ../cryptlib.h eng_ctrl.c eng_int.h
$(KBE_CONFIG)/eng_dyn.o: ../../e_os.h ../../include/openssl/bio.h
$(KBE_CONFIG)/eng_dyn.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/eng_dyn.o: ../../include/openssl/dso.h ../../include/openssl/e_os2.h
$(KBE_CONFIG)/eng_dyn.o: ../../include/openssl/engine.h ../../include/openssl/err.h
$(KBE_CONFIG)/eng_dyn.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/eng_dyn.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/eng_dyn.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/eng_dyn.o: ../../include/openssl/symhacks.h ../cryptlib.h eng_dyn.c eng_int.h
$(KBE_CONFIG)/eng_err.o: ../../include/openssl/bio.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/eng_err.o: ../../include/openssl/e_os2.h ../../include/openssl/engine.h
$(KBE_CONFIG)/eng_err.o: ../../include/openssl/err.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/eng_err.o: ../../include/openssl/opensslconf.h ../../include/openssl/opensslv.h
$(KBE_CONFIG)/eng_err.o: ../../include/openssl/ossl_typ.h ../../include/openssl/safestack.h
$(KBE_CONFIG)/eng_err.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
$(KBE_CONFIG)/eng_err.o: eng_err.c
$(KBE_CONFIG)/eng_fat.o: ../../e_os.h ../../include/openssl/bio.h
$(KBE_CONFIG)/eng_fat.o: ../../include/openssl/buffer.h ../../include/openssl/conf.h
$(KBE_CONFIG)/eng_fat.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(KBE_CONFIG)/eng_fat.o: ../../include/openssl/engine.h ../../include/openssl/err.h
$(KBE_CONFIG)/eng_fat.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/eng_fat.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/eng_fat.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/eng_fat.o: ../../include/openssl/symhacks.h ../cryptlib.h eng_fat.c eng_int.h
$(KBE_CONFIG)/eng_init.o: ../../e_os.h ../../include/openssl/bio.h
$(KBE_CONFIG)/eng_init.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/eng_init.o: ../../include/openssl/e_os2.h ../../include/openssl/engine.h
$(KBE_CONFIG)/eng_init.o: ../../include/openssl/err.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/eng_init.o: ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/eng_init.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/eng_init.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/eng_init.o: ../../include/openssl/symhacks.h ../cryptlib.h eng_init.c eng_int.h
$(KBE_CONFIG)/eng_lib.o: ../../e_os.h ../../include/openssl/bio.h
$(KBE_CONFIG)/eng_lib.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/eng_lib.o: ../../include/openssl/e_os2.h ../../include/openssl/engine.h
$(KBE_CONFIG)/eng_lib.o: ../../include/openssl/err.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/eng_lib.o: ../../include/openssl/opensslconf.h ../../include/openssl/opensslv.h
$(KBE_CONFIG)/eng_lib.o: ../../include/openssl/ossl_typ.h ../../include/openssl/rand.h
$(KBE_CONFIG)/eng_lib.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/eng_lib.o: ../../include/openssl/symhacks.h ../cryptlib.h eng_int.h eng_lib.c
$(KBE_CONFIG)/eng_list.o: ../../e_os.h ../../include/openssl/bio.h
$(KBE_CONFIG)/eng_list.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/eng_list.o: ../../include/openssl/e_os2.h ../../include/openssl/engine.h
$(KBE_CONFIG)/eng_list.o: ../../include/openssl/err.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/eng_list.o: ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/eng_list.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/eng_list.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/eng_list.o: ../../include/openssl/symhacks.h ../cryptlib.h eng_int.h eng_list.c
$(KBE_CONFIG)/eng_openssl.o: ../../e_os.h ../../include/openssl/asn1.h
$(KBE_CONFIG)/eng_openssl.o: ../../include/openssl/bio.h ../../include/openssl/buffer.h
$(KBE_CONFIG)/eng_openssl.o: ../../include/openssl/crypto.h ../../include/openssl/dso.h
$(KBE_CONFIG)/eng_openssl.o: ../../include/openssl/e_os2.h ../../include/openssl/ec.h
$(KBE_CONFIG)/eng_openssl.o: ../../include/openssl/ecdh.h ../../include/openssl/ecdsa.h
$(KBE_CONFIG)/eng_openssl.o: ../../include/openssl/engine.h ../../include/openssl/err.h
$(KBE_CONFIG)/eng_openssl.o: ../../include/openssl/evp.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/eng_openssl.o: ../../include/openssl/obj_mac.h ../../include/openssl/objects.h
$(KBE_CONFIG)/eng_openssl.o: ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/eng_openssl.o: ../../include/openssl/opensslv.h
$(KBE_CONFIG)/eng_openssl.o: ../../include/openssl/ossl_typ.h ../../include/openssl/pem.h
$(KBE_CONFIG)/eng_openssl.o: ../../include/openssl/pem2.h ../../include/openssl/pkcs7.h
$(KBE_CONFIG)/eng_openssl.o: ../../include/openssl/rand.h ../../include/openssl/rsa.h
$(KBE_CONFIG)/eng_openssl.o: ../../include/openssl/safestack.h ../../include/openssl/sha.h
$(KBE_CONFIG)/eng_openssl.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
$(KBE_CONFIG)/eng_openssl.o: ../../include/openssl/x509.h ../../include/openssl/x509_vfy.h
$(KBE_CONFIG)/eng_openssl.o: ../cryptlib.h eng_openssl.c
$(KBE_CONFIG)/eng_padlock.o: ../../include/openssl/aes.h ../../include/openssl/asn1.h
$(KBE_CONFIG)/eng_padlock.o: ../../include/openssl/bio.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/eng_padlock.o: ../../include/openssl/dso.h ../../include/openssl/e_os2.h
$(KBE_CONFIG)/eng_padlock.o: ../../include/openssl/engine.h ../../include/openssl/err.h
$(KBE_CONFIG)/eng_padlock.o: ../../include/openssl/evp.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/eng_padlock.o: ../../include/openssl/obj_mac.h ../../include/openssl/objects.h
$(KBE_CONFIG)/eng_padlock.o: ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/eng_padlock.o: ../../include/openssl/opensslv.h
$(KBE_CONFIG)/eng_padlock.o: ../../include/openssl/ossl_typ.h ../../include/openssl/rand.h
$(KBE_CONFIG)/eng_padlock.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/eng_padlock.o: ../../include/openssl/symhacks.h eng_padlock.c
$(KBE_CONFIG)/eng_pkey.o: ../../e_os.h ../../include/openssl/bio.h
$(KBE_CONFIG)/eng_pkey.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/eng_pkey.o: ../../include/openssl/e_os2.h ../../include/openssl/engine.h
$(KBE_CONFIG)/eng_pkey.o: ../../include/openssl/err.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/eng_pkey.o: ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/eng_pkey.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/eng_pkey.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/eng_pkey.o: ../../include/openssl/symhacks.h ../cryptlib.h eng_int.h eng_pkey.c
$(KBE_CONFIG)/eng_table.o: ../../e_os.h ../../include/openssl/asn1.h
$(KBE_CONFIG)/eng_table.o: ../../include/openssl/bio.h ../../include/openssl/buffer.h
$(KBE_CONFIG)/eng_table.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(KBE_CONFIG)/eng_table.o: ../../include/openssl/engine.h ../../include/openssl/err.h
$(KBE_CONFIG)/eng_table.o: ../../include/openssl/evp.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/eng_table.o: ../../include/openssl/obj_mac.h ../../include/openssl/objects.h
$(KBE_CONFIG)/eng_table.o: ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/eng_table.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/eng_table.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/eng_table.o: ../../include/openssl/symhacks.h ../cryptlib.h eng_int.h
$(KBE_CONFIG)/eng_table.o: eng_table.c
$(KBE_CONFIG)/tb_cipher.o: ../../e_os.h ../../include/openssl/bio.h
$(KBE_CONFIG)/tb_cipher.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/tb_cipher.o: ../../include/openssl/e_os2.h ../../include/openssl/engine.h
$(KBE_CONFIG)/tb_cipher.o: ../../include/openssl/err.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/tb_cipher.o: ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/tb_cipher.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/tb_cipher.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/tb_cipher.o: ../../include/openssl/symhacks.h ../cryptlib.h eng_int.h
$(KBE_CONFIG)/tb_cipher.o: tb_cipher.c
$(KBE_CONFIG)/tb_dh.o: ../../e_os.h ../../include/openssl/bio.h
$(KBE_CONFIG)/tb_dh.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/tb_dh.o: ../../include/openssl/e_os2.h ../../include/openssl/engine.h
$(KBE_CONFIG)/tb_dh.o: ../../include/openssl/err.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/tb_dh.o: ../../include/openssl/opensslconf.h ../../include/openssl/opensslv.h
$(KBE_CONFIG)/tb_dh.o: ../../include/openssl/ossl_typ.h ../../include/openssl/safestack.h
$(KBE_CONFIG)/tb_dh.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
$(KBE_CONFIG)/tb_dh.o: ../cryptlib.h eng_int.h tb_dh.c
$(KBE_CONFIG)/tb_digest.o: ../../e_os.h ../../include/openssl/bio.h
$(KBE_CONFIG)/tb_digest.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/tb_digest.o: ../../include/openssl/e_os2.h ../../include/openssl/engine.h
$(KBE_CONFIG)/tb_digest.o: ../../include/openssl/err.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/tb_digest.o: ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/tb_digest.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/tb_digest.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/tb_digest.o: ../../include/openssl/symhacks.h ../cryptlib.h eng_int.h
$(KBE_CONFIG)/tb_digest.o: tb_digest.c
$(KBE_CONFIG)/tb_dsa.o: ../../e_os.h ../../include/openssl/bio.h
$(KBE_CONFIG)/tb_dsa.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/tb_dsa.o: ../../include/openssl/e_os2.h ../../include/openssl/engine.h
$(KBE_CONFIG)/tb_dsa.o: ../../include/openssl/err.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/tb_dsa.o: ../../include/openssl/opensslconf.h ../../include/openssl/opensslv.h
$(KBE_CONFIG)/tb_dsa.o: ../../include/openssl/ossl_typ.h ../../include/openssl/safestack.h
$(KBE_CONFIG)/tb_dsa.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
$(KBE_CONFIG)/tb_dsa.o: ../cryptlib.h eng_int.h tb_dsa.c
$(KBE_CONFIG)/tb_ecdh.o: ../../e_os.h ../../include/openssl/bio.h
$(KBE_CONFIG)/tb_ecdh.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/tb_ecdh.o: ../../include/openssl/e_os2.h ../../include/openssl/engine.h
$(KBE_CONFIG)/tb_ecdh.o: ../../include/openssl/err.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/tb_ecdh.o: ../../include/openssl/opensslconf.h ../../include/openssl/opensslv.h
$(KBE_CONFIG)/tb_ecdh.o: ../../include/openssl/ossl_typ.h ../../include/openssl/safestack.h
$(KBE_CONFIG)/tb_ecdh.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
$(KBE_CONFIG)/tb_ecdh.o: ../cryptlib.h eng_int.h tb_ecdh.c
$(KBE_CONFIG)/tb_ecdsa.o: ../../e_os.h ../../include/openssl/bio.h
$(KBE_CONFIG)/tb_ecdsa.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/tb_ecdsa.o: ../../include/openssl/e_os2.h ../../include/openssl/engine.h
$(KBE_CONFIG)/tb_ecdsa.o: ../../include/openssl/err.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/tb_ecdsa.o: ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/tb_ecdsa.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/tb_ecdsa.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/tb_ecdsa.o: ../../include/openssl/symhacks.h ../cryptlib.h eng_int.h tb_ecdsa.c
$(KBE_CONFIG)/tb_rand.o: ../../e_os.h ../../include/openssl/bio.h
$(KBE_CONFIG)/tb_rand.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/tb_rand.o: ../../include/openssl/e_os2.h ../../include/openssl/engine.h
$(KBE_CONFIG)/tb_rand.o: ../../include/openssl/err.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/tb_rand.o: ../../include/openssl/opensslconf.h ../../include/openssl/opensslv.h
$(KBE_CONFIG)/tb_rand.o: ../../include/openssl/ossl_typ.h ../../include/openssl/safestack.h
$(KBE_CONFIG)/tb_rand.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
$(KBE_CONFIG)/tb_rand.o: ../cryptlib.h eng_int.h tb_rand.c
$(KBE_CONFIG)/tb_rsa.o: ../../e_os.h ../../include/openssl/bio.h
$(KBE_CONFIG)/tb_rsa.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/tb_rsa.o: ../../include/openssl/e_os2.h ../../include/openssl/engine.h
$(KBE_CONFIG)/tb_rsa.o: ../../include/openssl/err.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/tb_rsa.o: ../../include/openssl/opensslconf.h ../../include/openssl/opensslv.h
$(KBE_CONFIG)/tb_rsa.o: ../../include/openssl/ossl_typ.h ../../include/openssl/safestack.h
$(KBE_CONFIG)/tb_rsa.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
$(KBE_CONFIG)/tb_rsa.o: ../cryptlib.h eng_int.h tb_rsa.c
$(KBE_CONFIG)/tb_store.o: ../../e_os.h ../../include/openssl/bio.h
$(KBE_CONFIG)/tb_store.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/tb_store.o: ../../include/openssl/e_os2.h ../../include/openssl/engine.h
$(KBE_CONFIG)/tb_store.o: ../../include/openssl/err.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/tb_store.o: ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/tb_store.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/tb_store.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/tb_store.o: ../../include/openssl/symhacks.h ../cryptlib.h eng_int.h tb_store.c
