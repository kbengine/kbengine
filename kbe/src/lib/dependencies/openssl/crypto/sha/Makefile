#
# OpenSSL/crypto/sha/Makefile
#

ifndef KBE_ROOT
export KBE_ROOT := $(subst /kbe/src/lib/dependencies/openssl/crypto/sha,,$(CURDIR))
endif

LIBDIR= $(KBE_ROOT)/kbe/src/libs

DIR=    sha
TOP=    ../..
CC=     cc
CPP=    $(CC) -E
INCLUDES=
CFLAG=-g
MAKEFILE=       Makefile
AR=             ar r

SHA1_ASM_OBJ=

CFLAGS= $(INCLUDES) $(CFLAG)
ASFLAGS= $(INCLUDES) $(ASFLAG)
AFLAGS= $(ASFLAGS)

GENERAL=Makefile
TEST=shatest.c sha1test.c sha256t.c sha512t.c
APPS=

LIB=$(LIBDIR)/libcrypto.a
LIBSRC=sha_dgst.c sha1dgst.c sha_one.c sha1_one.c sha256.c sha512.c
LIBOBJ=$(KBE_CONFIG)/sha_dgst.o $(KBE_CONFIG)/sha1dgst.o $(KBE_CONFIG)/sha_one.o $(KBE_CONFIG)/sha1_one.o $(KBE_CONFIG)/sha256.o $(KBE_CONFIG)/sha512.o $(SHA1_ASM_OBJ)

SRC= $(LIBSRC)

EXHEADER= sha.h
HEADER= sha_locl.h $(EXHEADER)

ALL=    $(GENERAL) $(SRC) $(HEADER)

top:
	(cd ../..; $(MAKE) DIRS=crypto SDIRS=$(DIR) sub_all)

all:    lib

$(KBE_CONFIG):
	@-mkdir $(KBE_CONFIG)

lib:    $(KBE_CONFIG) $(LIBOBJ)
	$(AR) $(LIB) $(LIBOBJ)
	$(RANLIB) $(LIB) || echo Never mind.
	@touch lib

$(KBE_CONFIG)/%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

# ELF
$(KBE_CONFIG)/sx86-elf.s: asm/sha1-586.pl ../perlasm/x86asm.pl
	(cd asm; $(PERL) sha1-586.pl elf $(CFLAGS) $(PROCESSOR) > ../$@)
$(KBE_CONFIG)/s512sse2-elf.s:	asm/sha512-sse2.pl ../perlasm/x86asm.pl
	(cd asm; $(PERL) sha512-sse2.pl elf $(CFLAGS) $(PROCESSOR) > ../$@)
# COFF
$(KBE_CONFIG)/sx86-cof.s: asm/sha1-586.pl ../perlasm/x86asm.pl
	(cd asm; $(PERL) sha1-586.pl coff $(CFLAGS) $(PROCESSOR) > ../$@)
$(KBE_CONFIG)/s512sse2-cof.s:     asm/sha512-sse2.pl ../perlasm/x86asm.pl
	(cd asm; $(PERL) sha512-sse2.pl coff $(CFLAGS) $(PROCESSOR) > ../$@)
# a.out
$(KBE_CONFIG)/sx86-out.s: asm/sha1-586.pl ../perlasm/x86asm.pl
	(cd asm; $(PERL) sha1-586.pl a.out $(CFLAGS) $(PROCESSOR) > ../$@)
$(KBE_CONFIG)/s512sse2-out.s:     asm/sha512-sse2.pl ../perlasm/x86asm.pl
	(cd asm; $(PERL) sha512-sse2.pl a.out $(CFLAGS) $(PROCESSOR) > ../$@)

$(KBE_CONFIG)/sha1-ia64.s:   asm/sha1-ia64.pl
	(cd asm; $(PERL) sha1-ia64.pl $(CFLAGS) ) > $@
$(KBE_CONFIG)/sha256-ia64.s: asm/sha512-ia64.pl
	(cd asm; $(PERL) sha512-ia64.pl ../$@ $(CFLAGS))
$(KBE_CONFIG)/sha512-ia64.s: asm/sha512-ia64.pl
	(cd asm; $(PERL) sha512-ia64.pl ../$@ $(CFLAGS))

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
	rm -f $(KBE_CONFIG)/*.s *.s $(KBE_CONFIG)/*.o *.o *.obj lib tags core .pure .nfs* *.old *.bak fluff

# DO NOT DELETE THIS LINE -- make depend depends on it.

$(KBE_CONFIG)/sha1_one.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(KBE_CONFIG)/sha1_one.o: ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/sha1_one.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/sha1_one.o: ../../include/openssl/safestack.h ../../include/openssl/sha.h
$(KBE_CONFIG)/sha1_one.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
$(KBE_CONFIG)/sha1_one.o: sha1_one.c
$(KBE_CONFIG)/sha1dgst.o: ../../include/openssl/e_os2.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/sha1dgst.o: ../../include/openssl/opensslv.h ../../include/openssl/sha.h
$(KBE_CONFIG)/sha1dgst.o: ../md32_common.h sha1dgst.c sha_locl.h
$(KBE_CONFIG)/sha256.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(KBE_CONFIG)/sha256.o: ../../include/openssl/opensslconf.h ../../include/openssl/opensslv.h
$(KBE_CONFIG)/sha256.o: ../../include/openssl/ossl_typ.h ../../include/openssl/safestack.h
$(KBE_CONFIG)/sha256.o: ../../include/openssl/sha.h ../../include/openssl/stack.h
$(KBE_CONFIG)/sha256.o: ../../include/openssl/symhacks.h ../md32_common.h sha256.c
$(KBE_CONFIG)/sha512.o: ../../e_os.h ../../include/openssl/bio.h
$(KBE_CONFIG)/sha512.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/sha512.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(KBE_CONFIG)/sha512.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/sha512.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/sha512.o: ../../include/openssl/safestack.h ../../include/openssl/sha.h
$(KBE_CONFIG)/sha512.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
$(KBE_CONFIG)/sha512.o: ../cryptlib.h sha512.c
$(KBE_CONFIG)/sha_dgst.o: ../../include/openssl/e_os2.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/sha_dgst.o: ../../include/openssl/opensslv.h ../../include/openssl/sha.h
$(KBE_CONFIG)/sha_dgst.o: ../md32_common.h sha_dgst.c sha_locl.h
$(KBE_CONFIG)/sha_one.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(KBE_CONFIG)/sha_one.o: ../../include/openssl/opensslconf.h ../../include/openssl/opensslv.h
$(KBE_CONFIG)/sha_one.o: ../../include/openssl/ossl_typ.h ../../include/openssl/safestack.h
$(KBE_CONFIG)/sha_one.o: ../../include/openssl/sha.h ../../include/openssl/stack.h
$(KBE_CONFIG)/sha_one.o: ../../include/openssl/symhacks.h sha_one.c
