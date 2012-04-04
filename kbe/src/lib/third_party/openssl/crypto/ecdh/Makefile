#
# crypto/ecdh/Makefile
#

DIR=	ecdh
TOP=	../..
CC=	cc
INCLUDES= -I.. -I$(TOP) -I../../include
CFLAG=-g -Wall
MAKEFILE=	Makefile
AR=		ar r

CFLAGS= $(INCLUDES) $(CFLAG)

GENERAL=Makefile
TEST=ecdhtest.c
APPS=

LIB=$(BWLIBDIR)/libcrypto.a
LIBSRC=	ech_lib.c ech_ossl.c ech_key.c ech_err.c

LIBOBJ=	$(MF_CONFIG)/ech_lib.o $(MF_CONFIG)/ech_ossl.o \
	$(MF_CONFIG)/ech_key.o $(MF_CONFIG)/ech_err.o

SRC= $(LIBSRC)

EXHEADER= ecdh.h
HEADER=	ech_locl.h $(EXHEADER)

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
	@headerlist="$(EXHEADER)"; for i in $$headerlist; \
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
	rm -f $(MF_CONFIG)/*.o *.o */*.o *.obj lib tags core .pure .nfs* *.old *.bak fluff

# DO NOT DELETE THIS LINE -- make depend depends on it.

$(MF_CONFIG)/ech_err.o: ../../include/openssl/asn1.h ../../include/openssl/bio.h
$(MF_CONFIG)/ech_err.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(MF_CONFIG)/ech_err.o: ../../include/openssl/ec.h ../../include/openssl/ecdh.h
$(MF_CONFIG)/ech_err.o: ../../include/openssl/err.h ../../include/openssl/lhash.h
$(MF_CONFIG)/ech_err.o: ../../include/openssl/opensslconf.h ../../include/openssl/opensslv.h
$(MF_CONFIG)/ech_err.o: ../../include/openssl/ossl_typ.h ../../include/openssl/safestack.h
$(MF_CONFIG)/ech_err.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
$(MF_CONFIG)/ech_err.o: ech_err.c
$(MF_CONFIG)/ech_key.o: ../../include/openssl/asn1.h ../../include/openssl/bio.h
$(MF_CONFIG)/ech_key.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(MF_CONFIG)/ech_key.o: ../../include/openssl/ec.h ../../include/openssl/ecdh.h
$(MF_CONFIG)/ech_key.o: ../../include/openssl/engine.h ../../include/openssl/opensslconf.h
$(MF_CONFIG)/ech_key.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(MF_CONFIG)/ech_key.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(MF_CONFIG)/ech_key.o: ../../include/openssl/symhacks.h ech_key.c ech_locl.h
$(MF_CONFIG)/ech_lib.o: ../../include/openssl/asn1.h ../../include/openssl/bio.h
$(MF_CONFIG)/ech_lib.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(MF_CONFIG)/ech_lib.o: ../../include/openssl/ec.h ../../include/openssl/ecdh.h
$(MF_CONFIG)/ech_lib.o: ../../include/openssl/engine.h ../../include/openssl/err.h
$(MF_CONFIG)/ech_lib.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(MF_CONFIG)/ech_lib.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(MF_CONFIG)/ech_lib.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(MF_CONFIG)/ech_lib.o: ../../include/openssl/symhacks.h ech_lib.c ech_locl.h
$(MF_CONFIG)/ech_ossl.o: ../../e_os.h ../../include/openssl/asn1.h
$(MF_CONFIG)/ech_ossl.o: ../../include/openssl/bio.h ../../include/openssl/bn.h
$(MF_CONFIG)/ech_ossl.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(MF_CONFIG)/ech_ossl.o: ../../include/openssl/e_os2.h ../../include/openssl/ec.h
$(MF_CONFIG)/ech_ossl.o: ../../include/openssl/ecdh.h ../../include/openssl/err.h
$(MF_CONFIG)/ech_ossl.o: ../../include/openssl/lhash.h ../../include/openssl/obj_mac.h
$(MF_CONFIG)/ech_ossl.o: ../../include/openssl/opensslconf.h
$(MF_CONFIG)/ech_ossl.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(MF_CONFIG)/ech_ossl.o: ../../include/openssl/safestack.h ../../include/openssl/sha.h
$(MF_CONFIG)/ech_ossl.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
$(MF_CONFIG)/ech_ossl.o: ../cryptlib.h ech_locl.h ech_ossl.c
