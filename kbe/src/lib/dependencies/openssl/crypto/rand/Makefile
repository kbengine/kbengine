#
# OpenSSL/crypto/rand/Makefile
#

ifndef KBE_ROOT
export KBE_ROOT := $(subst /kbe/src/lib/dependencies/openssl/crypto/rand,,$(CURDIR))
endif

LIBDIR= $(KBE_ROOT)/kbe/src/libs

DIR=	rand
TOP=	../..
CC=	cc
INCLUDES=
CFLAG=-g
MAKEFILE=	Makefile
AR=		ar r

CFLAGS= $(INCLUDES) $(CFLAG)

GENERAL=Makefile
TEST= randtest.c
APPS=

LIB=$(LIBDIR)/libcrypto.a
LIBSRC=md_rand.c randfile.c rand_lib.c rand_err.c rand_egd.c \
	rand_win.c rand_unix.c rand_os2.c rand_nw.c
LIBOBJ=$(KBE_CONFIG)/md_rand.o $(KBE_CONFIG)/randfile.o $(KBE_CONFIG)/rand_lib.o \
	$(KBE_CONFIG)/rand_err.o $(KBE_CONFIG)/rand_egd.o $(KBE_CONFIG)/rand_win.o \
	$(KBE_CONFIG)/rand_unix.o $(KBE_CONFIG)/rand_os2.o $(KBE_CONFIG)/rand_nw.o

SRC= $(LIBSRC)

EXHEADER= rand.h
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
	rm -f $(KBE_CONFIG)/*.o *.o *.obj lib tags core .pure .nfs* *.old *.bak fluff

# DO NOT DELETE THIS LINE -- make depend depends on it.

$(KBE_CONFIG)/md_rand.o: ../../e_os.h ../../include/openssl/asn1.h
$(KBE_CONFIG)/md_rand.o: ../../include/openssl/bio.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/md_rand.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(KBE_CONFIG)/md_rand.o: ../../include/openssl/evp.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/md_rand.o: ../../include/openssl/obj_mac.h ../../include/openssl/objects.h
$(KBE_CONFIG)/md_rand.o: ../../include/openssl/opensslconf.h ../../include/openssl/opensslv.h
$(KBE_CONFIG)/md_rand.o: ../../include/openssl/ossl_typ.h ../../include/openssl/rand.h
$(KBE_CONFIG)/md_rand.o: ../../include/openssl/safestack.h ../../include/openssl/sha.h
$(KBE_CONFIG)/md_rand.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
$(KBE_CONFIG)/md_rand.o: md_rand.c rand_lcl.h
$(KBE_CONFIG)/rand_egd.o: ../../include/openssl/buffer.h ../../include/openssl/e_os2.h
$(KBE_CONFIG)/rand_egd.o: ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/rand_egd.o: ../../include/openssl/ossl_typ.h ../../include/openssl/rand.h
$(KBE_CONFIG)/rand_egd.o: rand_egd.c
$(KBE_CONFIG)/rand_err.o: ../../include/openssl/bio.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/rand_err.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(KBE_CONFIG)/rand_err.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/rand_err.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/rand_err.o: ../../include/openssl/rand.h ../../include/openssl/safestack.h
$(KBE_CONFIG)/rand_err.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
$(KBE_CONFIG)/rand_err.o: rand_err.c
$(KBE_CONFIG)/rand_lib.o: ../../e_os.h ../../include/openssl/bio.h
$(KBE_CONFIG)/rand_lib.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/rand_lib.o: ../../include/openssl/e_os2.h ../../include/openssl/engine.h
$(KBE_CONFIG)/rand_lib.o: ../../include/openssl/err.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/rand_lib.o: ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/rand_lib.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/rand_lib.o: ../../include/openssl/rand.h ../../include/openssl/safestack.h
$(KBE_CONFIG)/rand_lib.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
$(KBE_CONFIG)/rand_lib.o: ../cryptlib.h rand_lib.c
$(KBE_CONFIG)/rand_nw.o: ../../e_os.h ../../include/openssl/asn1.h
$(KBE_CONFIG)/rand_nw.o: ../../include/openssl/bio.h ../../include/openssl/buffer.h
$(KBE_CONFIG)/rand_nw.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(KBE_CONFIG)/rand_nw.o: ../../include/openssl/err.h ../../include/openssl/evp.h
$(KBE_CONFIG)/rand_nw.o: ../../include/openssl/lhash.h ../../include/openssl/obj_mac.h
$(KBE_CONFIG)/rand_nw.o: ../../include/openssl/objects.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/rand_nw.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/rand_nw.o: ../../include/openssl/rand.h ../../include/openssl/safestack.h
$(KBE_CONFIG)/rand_nw.o: ../../include/openssl/sha.h ../../include/openssl/stack.h
$(KBE_CONFIG)/rand_nw.o: ../../include/openssl/symhacks.h ../cryptlib.h rand_lcl.h rand_nw.c
$(KBE_CONFIG)/rand_os2.o: ../../e_os.h ../../include/openssl/asn1.h
$(KBE_CONFIG)/rand_os2.o: ../../include/openssl/bio.h ../../include/openssl/buffer.h
$(KBE_CONFIG)/rand_os2.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(KBE_CONFIG)/rand_os2.o: ../../include/openssl/err.h ../../include/openssl/evp.h
$(KBE_CONFIG)/rand_os2.o: ../../include/openssl/lhash.h ../../include/openssl/obj_mac.h
$(KBE_CONFIG)/rand_os2.o: ../../include/openssl/objects.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/rand_os2.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/rand_os2.o: ../../include/openssl/rand.h ../../include/openssl/safestack.h
$(KBE_CONFIG)/rand_os2.o: ../../include/openssl/sha.h ../../include/openssl/stack.h
$(KBE_CONFIG)/rand_os2.o: ../../include/openssl/symhacks.h ../cryptlib.h rand_lcl.h
$(KBE_CONFIG)/rand_os2.o: rand_os2.c
$(KBE_CONFIG)/rand_unix.o: ../../e_os.h ../../include/openssl/asn1.h
$(KBE_CONFIG)/rand_unix.o: ../../include/openssl/bio.h ../../include/openssl/buffer.h
$(KBE_CONFIG)/rand_unix.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(KBE_CONFIG)/rand_unix.o: ../../include/openssl/err.h ../../include/openssl/evp.h
$(KBE_CONFIG)/rand_unix.o: ../../include/openssl/lhash.h ../../include/openssl/obj_mac.h
$(KBE_CONFIG)/rand_unix.o: ../../include/openssl/objects.h
$(KBE_CONFIG)/rand_unix.o: ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/rand_unix.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/rand_unix.o: ../../include/openssl/rand.h ../../include/openssl/safestack.h
$(KBE_CONFIG)/rand_unix.o: ../../include/openssl/sha.h ../../include/openssl/stack.h
$(KBE_CONFIG)/rand_unix.o: ../../include/openssl/symhacks.h ../cryptlib.h rand_lcl.h
$(KBE_CONFIG)/rand_unix.o: rand_unix.c
$(KBE_CONFIG)/rand_win.o: ../../e_os.h ../../include/openssl/asn1.h
$(KBE_CONFIG)/rand_win.o: ../../include/openssl/bio.h ../../include/openssl/buffer.h
$(KBE_CONFIG)/rand_win.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(KBE_CONFIG)/rand_win.o: ../../include/openssl/err.h ../../include/openssl/evp.h
$(KBE_CONFIG)/rand_win.o: ../../include/openssl/lhash.h ../../include/openssl/obj_mac.h
$(KBE_CONFIG)/rand_win.o: ../../include/openssl/objects.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/rand_win.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/rand_win.o: ../../include/openssl/rand.h ../../include/openssl/safestack.h
$(KBE_CONFIG)/rand_win.o: ../../include/openssl/sha.h ../../include/openssl/stack.h
$(KBE_CONFIG)/rand_win.o: ../../include/openssl/symhacks.h ../cryptlib.h rand_lcl.h
$(KBE_CONFIG)/rand_win.o: rand_win.c
$(KBE_CONFIG)/randfile.o: ../../e_os.h ../../include/openssl/buffer.h
$(KBE_CONFIG)/randfile.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(KBE_CONFIG)/randfile.o: ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/randfile.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/randfile.o: ../../include/openssl/rand.h ../../include/openssl/safestack.h
$(KBE_CONFIG)/randfile.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
$(KBE_CONFIG)/randfile.o: randfile.c
