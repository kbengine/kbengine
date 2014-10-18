#
# OpenSSL/crypto/ui/Makefile
#

ifndef KBE_ROOT
export KBE_ROOT := $(subst /kbe/src/lib/dependencies/openssl/crypto/ui,,$(CURDIR))
endif

LIBDIR= $(KBE_ROOT)/kbe/src/libs

DIR=	ui
TOP=	../..
CC=	cc
INCLUDES= -I.. -I$(TOP) -I../../include
CFLAG=-g
MAKEFILE=	Makefile
AR=		ar r

CFLAGS= $(INCLUDES) $(CFLAG)

GENERAL=Makefile
#TEST= uitest.c
TEST=
APPS=

COMPATSRC= ui_compat.c
COMPATOBJ= $(KBE_CONFIG)/ui_compat.o

LIB=$(LIBDIR)/libcrypto.a
LIBSRC= ui_err.c ui_lib.c ui_openssl.c ui_util.c $(COMPATSRC)
LIBOBJ= $(KBE_CONFIG)/ui_err.o $(KBE_CONFIG)/ui_lib.o $(KBE_CONFIG)/ui_openssl.o $(KBE_CONFIG)/ui_util.o $(COMPATOBJ)

SRC= $(LIBSRC)

EXHEADER= ui.h ui_compat.h
HEADER=	$(EXHEADER) ui_locl.h

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

$(KBE_CONFIG)/ui_compat.o: ../../include/openssl/e_os2.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/ui_compat.o: ../../include/openssl/ossl_typ.h ../../include/openssl/safestack.h
$(KBE_CONFIG)/ui_compat.o: ../../include/openssl/stack.h ../../include/openssl/ui.h
$(KBE_CONFIG)/ui_compat.o: ../../include/openssl/ui_compat.h ui_compat.c
$(KBE_CONFIG)/ui_err.o: ../../include/openssl/bio.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/ui_err.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(KBE_CONFIG)/ui_err.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/ui_err.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/ui_err.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/ui_err.o: ../../include/openssl/symhacks.h ../../include/openssl/ui.h ui_err.c
$(KBE_CONFIG)/ui_lib.o: ../../e_os.h ../../include/openssl/bio.h
$(KBE_CONFIG)/ui_lib.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/ui_lib.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(KBE_CONFIG)/ui_lib.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/ui_lib.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/ui_lib.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/ui_lib.o: ../../include/openssl/symhacks.h ../../include/openssl/ui.h
$(KBE_CONFIG)/ui_lib.o: ../cryptlib.h ui_lib.c ui_locl.h
$(KBE_CONFIG)/ui_openssl.o: ../../e_os.h ../../include/openssl/bio.h
$(KBE_CONFIG)/ui_openssl.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/ui_openssl.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(KBE_CONFIG)/ui_openssl.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/ui_openssl.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/ui_openssl.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/ui_openssl.o: ../../include/openssl/symhacks.h ../../include/openssl/ui.h
$(KBE_CONFIG)/ui_openssl.o: ../cryptlib.h ui_locl.h ui_openssl.c
$(KBE_CONFIG)/ui_util.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(KBE_CONFIG)/ui_util.o: ../../include/openssl/opensslconf.h ../../include/openssl/opensslv.h
$(KBE_CONFIG)/ui_util.o: ../../include/openssl/ossl_typ.h ../../include/openssl/safestack.h
$(KBE_CONFIG)/ui_util.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
$(KBE_CONFIG)/ui_util.o: ../../include/openssl/ui.h ui_locl.h ui_util.c
