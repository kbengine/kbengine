#
# OpenSSL/crypto/store/Makefile
#

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

LIB=$(BWLIBDIR)/libcrypto.a
LIBSRC= str_err.c str_lib.c str_meth.c str_mem.c
LIBOBJ= $(MF_CONFIG)/str_err.o $(MF_CONFIG)/str_lib.o \
	$(MF_CONFIG)/str_meth.o $(MF_CONFIG)/str_mem.o

SRC= $(LIBSRC)

#EXHEADER= store.h str_compat.h
EXHEADER= store.h
HEADER=	$(EXHEADER) str_locl.h

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

$(MF_CONFIG)/str_err.o: ../../include/openssl/bio.h ../../include/openssl/crypto.h
$(MF_CONFIG)/str_err.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(MF_CONFIG)/str_err.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(MF_CONFIG)/str_err.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(MF_CONFIG)/str_err.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(MF_CONFIG)/str_err.o: ../../include/openssl/store.h ../../include/openssl/symhacks.h
$(MF_CONFIG)/str_err.o: str_err.c
$(MF_CONFIG)/str_lib.o: ../../include/openssl/asn1.h ../../include/openssl/bio.h
$(MF_CONFIG)/str_lib.o: ../../include/openssl/bn.h ../../include/openssl/buffer.h
$(MF_CONFIG)/str_lib.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(MF_CONFIG)/str_lib.o: ../../include/openssl/ec.h ../../include/openssl/ecdh.h
$(MF_CONFIG)/str_lib.o: ../../include/openssl/ecdsa.h ../../include/openssl/engine.h
$(MF_CONFIG)/str_lib.o: ../../include/openssl/err.h ../../include/openssl/evp.h
$(MF_CONFIG)/str_lib.o: ../../include/openssl/lhash.h ../../include/openssl/obj_mac.h
$(MF_CONFIG)/str_lib.o: ../../include/openssl/objects.h ../../include/openssl/opensslconf.h
$(MF_CONFIG)/str_lib.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(MF_CONFIG)/str_lib.o: ../../include/openssl/pkcs7.h ../../include/openssl/safestack.h
$(MF_CONFIG)/str_lib.o: ../../include/openssl/sha.h ../../include/openssl/stack.h
$(MF_CONFIG)/str_lib.o: ../../include/openssl/store.h ../../include/openssl/symhacks.h
$(MF_CONFIG)/str_lib.o: ../../include/openssl/x509.h ../../include/openssl/x509_vfy.h
$(MF_CONFIG)/str_lib.o: str_lib.c str_locl.h
$(MF_CONFIG)/str_mem.o: ../../include/openssl/bio.h ../../include/openssl/crypto.h
$(MF_CONFIG)/str_mem.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(MF_CONFIG)/str_mem.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(MF_CONFIG)/str_mem.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(MF_CONFIG)/str_mem.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(MF_CONFIG)/str_mem.o: ../../include/openssl/store.h ../../include/openssl/symhacks.h
$(MF_CONFIG)/str_mem.o: str_locl.h str_mem.c
$(MF_CONFIG)/str_meth.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(MF_CONFIG)/str_meth.o: ../../include/openssl/e_os2.h ../../include/openssl/opensslconf.h
$(MF_CONFIG)/str_meth.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(MF_CONFIG)/str_meth.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(MF_CONFIG)/str_meth.o: ../../include/openssl/store.h ../../include/openssl/symhacks.h
$(MF_CONFIG)/str_meth.o: str_locl.h str_meth.c
