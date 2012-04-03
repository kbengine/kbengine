#
# OpenSSL/crypto/ui/Makefile
#

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
COMPATOBJ= $(MF_CONFIG)/ui_compat.o

LIB=$(BWLIBDIR)/libcrypto.a
LIBSRC= ui_err.c ui_lib.c ui_openssl.c ui_util.c $(COMPATSRC)
LIBOBJ= $(MF_CONFIG)/ui_err.o $(MF_CONFIG)/ui_lib.o $(MF_CONFIG)/ui_openssl.o $(MF_CONFIG)/ui_util.o $(COMPATOBJ)

SRC= $(LIBSRC)

EXHEADER= ui.h ui_compat.h
HEADER=	$(EXHEADER) ui_locl.h

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
	$(MAKEDEPEND) -- $(CFLAG) $(INCLUDES) $(DEPFLAG) -- $(PROGS) $(LIBSRC)

dclean:
	$(PERL) -pe 'if (/^# DO NOT DELETE THIS LINE/) {print; exit(0);}' $(MAKEFILE) >Makefile.new
	mv -f Makefile.new $(MAKEFILE)

clean:
	rm -f $(MF_CONFIG)/*.o *.o */*.o *.obj lib tags core .pure .nfs* *.old *.bak fluff

# DO NOT DELETE THIS LINE -- make depend depends on it.

$(MF_CONFIG)/ui_compat.o: ../../include/openssl/e_os2.h ../../include/openssl/opensslconf.h
$(MF_CONFIG)/ui_compat.o: ../../include/openssl/ossl_typ.h ../../include/openssl/safestack.h
$(MF_CONFIG)/ui_compat.o: ../../include/openssl/stack.h ../../include/openssl/ui.h
$(MF_CONFIG)/ui_compat.o: ../../include/openssl/ui_compat.h ui_compat.c
$(MF_CONFIG)/ui_err.o: ../../include/openssl/bio.h ../../include/openssl/crypto.h
$(MF_CONFIG)/ui_err.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(MF_CONFIG)/ui_err.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(MF_CONFIG)/ui_err.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(MF_CONFIG)/ui_err.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(MF_CONFIG)/ui_err.o: ../../include/openssl/symhacks.h ../../include/openssl/ui.h ui_err.c
$(MF_CONFIG)/ui_lib.o: ../../e_os.h ../../include/openssl/bio.h
$(MF_CONFIG)/ui_lib.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(MF_CONFIG)/ui_lib.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(MF_CONFIG)/ui_lib.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(MF_CONFIG)/ui_lib.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(MF_CONFIG)/ui_lib.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(MF_CONFIG)/ui_lib.o: ../../include/openssl/symhacks.h ../../include/openssl/ui.h
$(MF_CONFIG)/ui_lib.o: ../cryptlib.h ui_lib.c ui_locl.h
$(MF_CONFIG)/ui_openssl.o: ../../e_os.h ../../include/openssl/bio.h
$(MF_CONFIG)/ui_openssl.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(MF_CONFIG)/ui_openssl.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(MF_CONFIG)/ui_openssl.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(MF_CONFIG)/ui_openssl.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(MF_CONFIG)/ui_openssl.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(MF_CONFIG)/ui_openssl.o: ../../include/openssl/symhacks.h ../../include/openssl/ui.h
$(MF_CONFIG)/ui_openssl.o: ../cryptlib.h ui_locl.h ui_openssl.c
$(MF_CONFIG)/ui_util.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(MF_CONFIG)/ui_util.o: ../../include/openssl/opensslconf.h ../../include/openssl/opensslv.h
$(MF_CONFIG)/ui_util.o: ../../include/openssl/ossl_typ.h ../../include/openssl/safestack.h
$(MF_CONFIG)/ui_util.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
$(MF_CONFIG)/ui_util.o: ../../include/openssl/ui.h ui_locl.h ui_util.c
