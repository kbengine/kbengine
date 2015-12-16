#
# crypto/ecdsa/Makefile
#

ifndef KBE_ROOT
export KBE_ROOT := $(subst /kbe/src/lib/dependencies/openssl/crypto/ecdsa,,$(CURDIR))
endif

LIBDIR= $(KBE_ROOT)/kbe/src/libs

DIR=	ecdsa
TOP=	../..
CC=	cc
INCLUDES= -I.. -I$(TOP) -I../../include
CFLAG=-g -Wall
MAKEFILE=	Makefile
AR=		ar r

CFLAGS= $(INCLUDES) $(CFLAG)

GENERAL=Makefile
TEST=ecdsatest.c
APPS=

LIB=$(LIBDIR)/libcrypto.a
LIBSRC=	ecs_lib.c ecs_asn1.c ecs_ossl.c ecs_sign.c ecs_vrf.c ecs_err.c

LIBOBJ=	$(KBE_CONFIG)/ecs_lib.o $(KBE_CONFIG)/ecs_asn1.o \
	$(KBE_CONFIG)/ecs_ossl.o $(KBE_CONFIG)/ecs_sign.o $(KBE_CONFIG)/ecs_vrf.o \
	$(KBE_CONFIG)/ecs_err.o

SRC= $(LIBSRC)

EXHEADER= ecdsa.h
HEADER=	ecs_locl.h $(EXHEADER)

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
	rm -f $(KBE_CONFIG)/*.o *.o */*.o *.obj lib tags core .pure .nfs* *.old *.bak fluff

# DO NOT DELETE THIS LINE -- make depend depends on it.

$(KBE_CONFIG)/ecs_asn1.o: ../../include/openssl/asn1.h ../../include/openssl/asn1t.h
$(KBE_CONFIG)/ecs_asn1.o: ../../include/openssl/bio.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/ecs_asn1.o: ../../include/openssl/e_os2.h ../../include/openssl/ec.h
$(KBE_CONFIG)/ecs_asn1.o: ../../include/openssl/ecdsa.h ../../include/openssl/err.h
$(KBE_CONFIG)/ecs_asn1.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/ecs_asn1.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/ecs_asn1.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/ecs_asn1.o: ../../include/openssl/symhacks.h ecs_asn1.c ecs_locl.h
$(KBE_CONFIG)/ecs_err.o: ../../include/openssl/asn1.h ../../include/openssl/bio.h
$(KBE_CONFIG)/ecs_err.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(KBE_CONFIG)/ecs_err.o: ../../include/openssl/ec.h ../../include/openssl/ecdsa.h
$(KBE_CONFIG)/ecs_err.o: ../../include/openssl/err.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/ecs_err.o: ../../include/openssl/opensslconf.h ../../include/openssl/opensslv.h
$(KBE_CONFIG)/ecs_err.o: ../../include/openssl/ossl_typ.h ../../include/openssl/safestack.h
$(KBE_CONFIG)/ecs_err.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
$(KBE_CONFIG)/ecs_err.o: ecs_err.c
$(KBE_CONFIG)/ecs_lib.o: ../../include/openssl/asn1.h ../../include/openssl/bio.h
$(KBE_CONFIG)/ecs_lib.o: ../../include/openssl/bn.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/ecs_lib.o: ../../include/openssl/e_os2.h ../../include/openssl/ec.h
$(KBE_CONFIG)/ecs_lib.o: ../../include/openssl/ecdsa.h ../../include/openssl/engine.h
$(KBE_CONFIG)/ecs_lib.o: ../../include/openssl/err.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/ecs_lib.o: ../../include/openssl/opensslconf.h ../../include/openssl/opensslv.h
$(KBE_CONFIG)/ecs_lib.o: ../../include/openssl/ossl_typ.h ../../include/openssl/safestack.h
$(KBE_CONFIG)/ecs_lib.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
$(KBE_CONFIG)/ecs_lib.o: ecs_lib.c ecs_locl.h
$(KBE_CONFIG)/ecs_ossl.o: ../../include/openssl/asn1.h ../../include/openssl/bio.h
$(KBE_CONFIG)/ecs_ossl.o: ../../include/openssl/bn.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/ecs_ossl.o: ../../include/openssl/e_os2.h ../../include/openssl/ec.h
$(KBE_CONFIG)/ecs_ossl.o: ../../include/openssl/ecdsa.h ../../include/openssl/err.h
$(KBE_CONFIG)/ecs_ossl.o: ../../include/openssl/lhash.h ../../include/openssl/obj_mac.h
$(KBE_CONFIG)/ecs_ossl.o: ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/ecs_ossl.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/ecs_ossl.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/ecs_ossl.o: ../../include/openssl/symhacks.h ecs_locl.h ecs_ossl.c
$(KBE_CONFIG)/ecs_sign.o: ../../include/openssl/asn1.h ../../include/openssl/bio.h
$(KBE_CONFIG)/ecs_sign.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(KBE_CONFIG)/ecs_sign.o: ../../include/openssl/ec.h ../../include/openssl/ecdsa.h
$(KBE_CONFIG)/ecs_sign.o: ../../include/openssl/engine.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/ecs_sign.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/ecs_sign.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/ecs_sign.o: ../../include/openssl/symhacks.h ecs_locl.h ecs_sign.c
$(KBE_CONFIG)/ecs_vrf.o: ../../include/openssl/asn1.h ../../include/openssl/bio.h
$(KBE_CONFIG)/ecs_vrf.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(KBE_CONFIG)/ecs_vrf.o: ../../include/openssl/ec.h ../../include/openssl/ecdsa.h
$(KBE_CONFIG)/ecs_vrf.o: ../../include/openssl/engine.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/ecs_vrf.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/ecs_vrf.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/ecs_vrf.o: ../../include/openssl/symhacks.h ecs_locl.h ecs_vrf.c
