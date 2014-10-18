#
# OpenSSL/crypto/dso/Makefile
#

ifndef KBE_ROOT
export KBE_ROOT := $(subst /kbe/src/lib/dependencies/openssl/crypto/dso,,$(CURDIR))
endif

LIBDIR= $(KBE_ROOT)/kbe/src/libs

DIR=	dso
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
LIBSRC= dso_dl.c dso_dlfcn.c dso_err.c dso_lib.c dso_null.c \
	dso_openssl.c dso_win32.c dso_vms.c
LIBOBJ= $(KBE_CONFIG)/dso_dl.o $(KBE_CONFIG)/dso_dlfcn.o $(KBE_CONFIG)/dso_err.o \
	$(KBE_CONFIG)/dso_lib.o $(KBE_CONFIG)/dso_null.o $(KBE_CONFIG)/dso_openssl.o \
	$(KBE_CONFIG)/dso_win32.o $(KBE_CONFIG)/dso_vms.o

SRC= $(LIBSRC)

EXHEADER= dso.h
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

$(KBE_CONFIG)/dso_dl.o: ../../e_os.h ../../include/openssl/bio.h
$(KBE_CONFIG)/dso_dl.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/dso_dl.o: ../../include/openssl/dso.h ../../include/openssl/e_os2.h
$(KBE_CONFIG)/dso_dl.o: ../../include/openssl/err.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/dso_dl.o: ../../include/openssl/opensslconf.h ../../include/openssl/opensslv.h
$(KBE_CONFIG)/dso_dl.o: ../../include/openssl/ossl_typ.h ../../include/openssl/safestack.h
$(KBE_CONFIG)/dso_dl.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
$(KBE_CONFIG)/dso_dl.o: ../cryptlib.h dso_dl.c
$(KBE_CONFIG)/dso_dlfcn.o: ../../e_os.h ../../include/openssl/bio.h
$(KBE_CONFIG)/dso_dlfcn.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/dso_dlfcn.o: ../../include/openssl/dso.h ../../include/openssl/e_os2.h
$(KBE_CONFIG)/dso_dlfcn.o: ../../include/openssl/err.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/dso_dlfcn.o: ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/dso_dlfcn.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/dso_dlfcn.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/dso_dlfcn.o: ../../include/openssl/symhacks.h ../cryptlib.h dso_dlfcn.c
$(KBE_CONFIG)/dso_err.o: ../../include/openssl/bio.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/dso_err.o: ../../include/openssl/dso.h ../../include/openssl/e_os2.h
$(KBE_CONFIG)/dso_err.o: ../../include/openssl/err.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/dso_err.o: ../../include/openssl/opensslconf.h ../../include/openssl/opensslv.h
$(KBE_CONFIG)/dso_err.o: ../../include/openssl/ossl_typ.h ../../include/openssl/safestack.h
$(KBE_CONFIG)/dso_err.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
$(KBE_CONFIG)/dso_err.o: dso_err.c
$(KBE_CONFIG)/dso_lib.o: ../../e_os.h ../../include/openssl/bio.h
$(KBE_CONFIG)/dso_lib.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/dso_lib.o: ../../include/openssl/dso.h ../../include/openssl/e_os2.h
$(KBE_CONFIG)/dso_lib.o: ../../include/openssl/err.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/dso_lib.o: ../../include/openssl/opensslconf.h ../../include/openssl/opensslv.h
$(KBE_CONFIG)/dso_lib.o: ../../include/openssl/ossl_typ.h ../../include/openssl/safestack.h
$(KBE_CONFIG)/dso_lib.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
$(KBE_CONFIG)/dso_lib.o: ../cryptlib.h dso_lib.c
$(KBE_CONFIG)/dso_null.o: ../../e_os.h ../../include/openssl/bio.h
$(KBE_CONFIG)/dso_null.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/dso_null.o: ../../include/openssl/dso.h ../../include/openssl/e_os2.h
$(KBE_CONFIG)/dso_null.o: ../../include/openssl/err.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/dso_null.o: ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/dso_null.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/dso_null.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/dso_null.o: ../../include/openssl/symhacks.h ../cryptlib.h dso_null.c
$(KBE_CONFIG)/dso_openssl.o: ../../e_os.h ../../include/openssl/bio.h
$(KBE_CONFIG)/dso_openssl.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/dso_openssl.o: ../../include/openssl/dso.h ../../include/openssl/e_os2.h
$(KBE_CONFIG)/dso_openssl.o: ../../include/openssl/err.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/dso_openssl.o: ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/dso_openssl.o: ../../include/openssl/opensslv.h
$(KBE_CONFIG)/dso_openssl.o: ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/dso_openssl.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/dso_openssl.o: ../../include/openssl/symhacks.h ../cryptlib.h dso_openssl.c
$(KBE_CONFIG)/dso_vms.o: ../../e_os.h ../../include/openssl/bio.h
$(KBE_CONFIG)/dso_vms.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/dso_vms.o: ../../include/openssl/dso.h ../../include/openssl/e_os2.h
$(KBE_CONFIG)/dso_vms.o: ../../include/openssl/err.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/dso_vms.o: ../../include/openssl/opensslconf.h ../../include/openssl/opensslv.h
$(KBE_CONFIG)/dso_vms.o: ../../include/openssl/ossl_typ.h ../../include/openssl/safestack.h
$(KBE_CONFIG)/dso_vms.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
$(KBE_CONFIG)/dso_vms.o: ../cryptlib.h dso_vms.c
$(KBE_CONFIG)/dso_win32.o: ../../e_os.h ../../include/openssl/bio.h
$(KBE_CONFIG)/dso_win32.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/dso_win32.o: ../../include/openssl/dso.h ../../include/openssl/e_os2.h
$(KBE_CONFIG)/dso_win32.o: ../../include/openssl/err.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/dso_win32.o: ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/dso_win32.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/dso_win32.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/dso_win32.o: ../../include/openssl/symhacks.h ../cryptlib.h dso_win32.c
