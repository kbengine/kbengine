#
# OpenSSL/crypto/store/Makefile
#

ifndef KBE_ROOT
export KBE_ROOT := $(subst /kbe/src/lib/dependencies/openssl/crypto/store,,$(CURDIR))
endif

LIBDIR= $(KBE_ROOT)/kbe/src/libs

DIR=	store
TOP=	../..
CC=	cc
INCLUDES= -I.. -I$(TOP) -I../../include
CFLAG=-g
MAKEFILE=	Makefile
AR=		ar r

CFLAGS= $(INCLUDES) $(CFLAG)

GENERAL=Makefile
#TEST= storetest.c
TEST=
APPS=

LIB=$(LIBDIR)/libcrypto.a
LIBSRC= str_err.c str_lib.c str_meth.c str_mem.c
LIBOBJ= $(KBE_CONFIG)/str_err.o $(KBE_CONFIG)/str_lib.o \
	$(KBE_CONFIG)/str_meth.o $(KBE_CONFIG)/str_mem.o

SRC= $(LIBSRC)

#EXHEADER= store.h str_compat.h
EXHEADER= store.h
HEADER=	$(EXHEADER) str_locl.h

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

$(KBE_CONFIG)/str_err.o: ../../include/openssl/bio.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/str_err.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(KBE_CONFIG)/str_err.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/str_err.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/str_err.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/str_err.o: ../../include/openssl/store.h ../../include/openssl/symhacks.h
$(KBE_CONFIG)/str_err.o: str_err.c
$(KBE_CONFIG)/str_lib.o: ../../include/openssl/asn1.h ../../include/openssl/bio.h
$(KBE_CONFIG)/str_lib.o: ../../include/openssl/bn.h ../../include/openssl/buffer.h
$(KBE_CONFIG)/str_lib.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(KBE_CONFIG)/str_lib.o: ../../include/openssl/ec.h ../../include/openssl/ecdh.h
$(KBE_CONFIG)/str_lib.o: ../../include/openssl/ecdsa.h ../../include/openssl/engine.h
$(KBE_CONFIG)/str_lib.o: ../../include/openssl/err.h ../../include/openssl/evp.h
$(KBE_CONFIG)/str_lib.o: ../../include/openssl/lhash.h ../../include/openssl/obj_mac.h
$(KBE_CONFIG)/str_lib.o: ../../include/openssl/objects.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/str_lib.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/str_lib.o: ../../include/openssl/pkcs7.h ../../include/openssl/safestack.h
$(KBE_CONFIG)/str_lib.o: ../../include/openssl/sha.h ../../include/openssl/stack.h
$(KBE_CONFIG)/str_lib.o: ../../include/openssl/store.h ../../include/openssl/symhacks.h
$(KBE_CONFIG)/str_lib.o: ../../include/openssl/x509.h ../../include/openssl/x509_vfy.h
$(KBE_CONFIG)/str_lib.o: str_lib.c str_locl.h
$(KBE_CONFIG)/str_mem.o: ../../include/openssl/bio.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/str_mem.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(KBE_CONFIG)/str_mem.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/str_mem.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/str_mem.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/str_mem.o: ../../include/openssl/store.h ../../include/openssl/symhacks.h
$(KBE_CONFIG)/str_mem.o: str_locl.h str_mem.c
$(KBE_CONFIG)/str_meth.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/str_meth.o: ../../include/openssl/e_os2.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/str_meth.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/str_meth.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/str_meth.o: ../../include/openssl/store.h ../../include/openssl/symhacks.h
$(KBE_CONFIG)/str_meth.o: str_locl.h str_meth.c
