#
# OpenSSL/crypto/bn/Makefile
#

ifndef KBE_ROOT
export KBE_ROOT := $(subst /kbe/src/lib/dependencies/openssl/crypto/bn,,$(CURDIR))
endif

LIBDIR= $(KBE_ROOT)/kbe/src/libs

DIR=	bn
TOP=	../..
CC=	cc
CPP=    $(CC) -E
INCLUDES= -I.. -I$(TOP) -I../../include
CFLAG=-g
MAKEFILE=	Makefile
AR=		ar r

BN_ASM=		$(KBE_CONFIG)/bn_asm.o
# or use
#BN_ASM=	bn86-elf.o

CFLAGS= $(INCLUDES) $(CFLAG)
ASFLAGS= $(INCLUDES) $(ASFLAG)
AFLAGS= $(ASFLAGS)

GENERAL=Makefile
TEST=bntest.c exptest.c
APPS=

LIB=$(LIBDIR)/libcrypto.a
LIBSRC=	bn_add.c bn_div.c bn_exp.c bn_lib.c bn_ctx.c bn_mul.c bn_mod.c \
	bn_print.c bn_rand.c bn_shift.c bn_word.c bn_blind.c \
	bn_kron.c bn_sqrt.c bn_gcd.c bn_prime.c bn_err.c bn_sqr.c bn_asm.c \
	bn_recp.c bn_mont.c bn_mpi.c bn_exp2.c bn_gf2m.c bn_nist.c \
	bn_depr.c bn_const.c

LIBOBJ=	$(KBE_CONFIG)/bn_add.o $(KBE_CONFIG)/bn_div.o $(KBE_CONFIG)/bn_exp.o \
	$(KBE_CONFIG)/bn_lib.o $(KBE_CONFIG)/bn_ctx.o $(KBE_CONFIG)/bn_mul.o \
	$(KBE_CONFIG)/bn_mod.o $(KBE_CONFIG)/bn_print.o $(KBE_CONFIG)/bn_rand.o \
	$(KBE_CONFIG)/bn_shift.o $(KBE_CONFIG)/bn_word.o $(KBE_CONFIG)/bn_blind.o \
	$(KBE_CONFIG)/bn_kron.o $(KBE_CONFIG)/bn_sqrt.o $(KBE_CONFIG)/bn_gcd.o \
	$(KBE_CONFIG)/bn_prime.o $(KBE_CONFIG)/bn_err.o $(KBE_CONFIG)/bn_sqr.o \
	$(BN_ASM) $(KBE_CONFIG)/bn_recp.o $(KBE_CONFIG)/bn_mont.o \
	$(KBE_CONFIG)/bn_mpi.o $(KBE_CONFIG)/bn_exp2.o $(KBE_CONFIG)/bn_gf2m.o \
	$(KBE_CONFIG)/bn_nist.o $(KBE_CONFIG)/bn_depr.o $(KBE_CONFIG)/bn_const.o

SRC= $(LIBSRC)

EXHEADER= bn.h
HEADER=	bn_lcl.h bn_prime.h $(EXHEADER)

ALL=    $(GENERAL) $(SRC) $(HEADER)

top:
	(cd ../..; $(MAKE) DIRS=crypto SDIRS=$(DIR) sub_all)

all:	lib

bn_prime.h: bn_prime.pl
	$(PERL) bn_prime.pl >bn_prime.h

divtest: divtest.c ../../libcrypto.a
	cc -I../../include divtest.c -o divtest ../../libcrypto.a

bnbug: bnbug.c ../../libcrypto.a top
	cc -g -I../../include bnbug.c -o bnbug ../../libcrypto.a

$(KBE_CONFIG):
	@-mkdir $(KBE_CONFIG)

lib:	$(KBE_CONFIG) $(LIBOBJ)
	$(AR) $(LIB) $(LIBOBJ)
	$(RANLIB) $(LIB) || echo Never mind.
	@touch lib

$(KBE_CONFIG)/%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

# ELF
$(KBE_CONFIG)/bn86-elf.s:	asm/bn-586.pl ../perlasm/x86asm.pl
	(cd asm; $(PERL) bn-586.pl elf $(CFLAGS) > ../$@)
$(KBE_CONFIG)/co86-elf.s:	asm/co-586.pl ../perlasm/x86asm.pl
	(cd asm; $(PERL) co-586.pl elf $(CFLAGS) > ../$@)
# COFF
$(KBE_CONFIG)/bn86-cof.s: asm/bn-586.pl ../perlasm/x86asm.pl
	(cd asm; $(PERL) bn-586.pl coff $(CFLAGS) > ../$@)
$(KBE_CONFIG)/co86-cof.s: asm/co-586.pl ../perlasm/x86asm.pl
	(cd asm; $(PERL) co-586.pl coff $(CFLAGS) > ../$@)
# a.out
$(KBE_CONFIG)/bn86-out.s: asm/bn-586.pl ../perlasm/x86asm.pl
	(cd asm; $(PERL) bn-586.pl a.out $(CFLAGS) > ../$@)
$(KBE_CONFIG)/co86-out.s: asm/co-586.pl ../perlasm/x86asm.pl
	(cd asm; $(PERL) co-586.pl a.out $(CFLAGS) > ../$@)

$(KBE_CONFIG)/sparcv8.o:	asm/sparcv8.S
	$(CC) $(CFLAGS) -c asm/sparcv8.S
$(KBE_CONFIG)/sparcv8plus.o:	asm/sparcv8plus.S
	$(CC) $(CFLAGS) -c asm/sparcv8plus.S

$(KBE_CONFIG)/bn-mips3.o:	asm/mips3.s
	@if [ "$(CC)" = "gcc" ]; then \
		ABI=`expr "$(CFLAGS)" : ".*-mabi=\([n3264]*\)"` && \
		as -$$ABI -O -o $@ asm/mips3.s; \
	else	$(CC) -c $(CFLAGS) -o $@ asm/mips3.s; fi

$(KBE_CONFIG)/x86_64-gcc.o:	asm/x86_64-gcc.c
	$(CC) $(CFLAGS) -c -o $@ asm/x86_64-gcc.c

$(KBE_CONFIG)/bn-ia64.s:	asm/ia64.S
	$(CC) $(CFLAGS) -E asm/ia64.S > $@

# GNU assembler fails to compile PA-RISC2 modules, insist on calling
# vendor assembler...
$(KBE_CONFIG)/pa-risc2W.o: asm/pa-risc2W.s
	/usr/ccs/bin/as -o pa-risc2W.o asm/pa-risc2W.s
$(KBE_CONFIG)/pa-risc2.o: asm/pa-risc2.s
	/usr/ccs/bin/as -o pa-risc2.o asm/pa-risc2.s

# ppc - AIX, Linux, MacOS X...
$(KBE_CONFIG)/linux_ppc32.s: asm/ppc.pl;	$(PERL) $< $@
$(KBE_CONFIG)/linux_ppc64.s: asm/ppc.pl;	$(PERL) $< $@
$(KBE_CONFIG)/aix_ppc32.s: asm/ppc.pl;	$(PERL) asm/ppc.pl $@
$(KBE_CONFIG)/aix_ppc64.s: asm/ppc.pl;	$(PERL) asm/ppc.pl $@
$(KBE_CONFIG)/osx_ppc32.s: asm/ppc.pl;	$(PERL) $< $@

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

exptest:
	rm -f exptest
	gcc -I../../include -g2 -ggdb -o exptest exptest.c ../../libcrypto.a

div:
	rm -f a.out
	gcc -I.. -g div.c ../../libcrypto.a

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

$(KBE_CONFIG)/bn_add.o: ../../e_os.h ../../include/openssl/bio.h ../../include/openssl/bn.h
$(KBE_CONFIG)/bn_add.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/bn_add.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(KBE_CONFIG)/bn_add.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/bn_add.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/bn_add.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/bn_add.o: ../../include/openssl/symhacks.h ../cryptlib.h bn_add.c bn_lcl.h
$(KBE_CONFIG)/bn_asm.o: ../../e_os.h ../../include/openssl/bio.h ../../include/openssl/bn.h
$(KBE_CONFIG)/bn_asm.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/bn_asm.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(KBE_CONFIG)/bn_asm.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/bn_asm.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/bn_asm.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/bn_asm.o: ../../include/openssl/symhacks.h ../cryptlib.h bn_asm.c bn_lcl.h
$(KBE_CONFIG)/bn_blind.o: ../../e_os.h ../../include/openssl/bio.h ../../include/openssl/bn.h
$(KBE_CONFIG)/bn_blind.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/bn_blind.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(KBE_CONFIG)/bn_blind.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/bn_blind.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/bn_blind.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/bn_blind.o: ../../include/openssl/symhacks.h ../cryptlib.h bn_blind.c bn_lcl.h
$(KBE_CONFIG)/bn_const.o: ../../include/openssl/e_os2.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/bn_const.o: ../../include/openssl/ossl_typ.h bn.h bn_const.c
$(KBE_CONFIG)/bn_ctx.o: ../../e_os.h ../../include/openssl/bio.h ../../include/openssl/bn.h
$(KBE_CONFIG)/bn_ctx.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/bn_ctx.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(KBE_CONFIG)/bn_ctx.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/bn_ctx.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/bn_ctx.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/bn_ctx.o: ../../include/openssl/symhacks.h ../cryptlib.h bn_ctx.c bn_lcl.h
$(KBE_CONFIG)/bn_depr.o: ../../e_os.h ../../include/openssl/bio.h ../../include/openssl/bn.h
$(KBE_CONFIG)/bn_depr.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/bn_depr.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(KBE_CONFIG)/bn_depr.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/bn_depr.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/bn_depr.o: ../../include/openssl/rand.h ../../include/openssl/safestack.h
$(KBE_CONFIG)/bn_depr.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
$(KBE_CONFIG)/bn_depr.o: ../cryptlib.h bn_depr.c bn_lcl.h
$(KBE_CONFIG)/bn_div.o: ../../e_os.h ../../include/openssl/bio.h ../../include/openssl/bn.h
$(KBE_CONFIG)/bn_div.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/bn_div.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(KBE_CONFIG)/bn_div.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/bn_div.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/bn_div.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/bn_div.o: ../../include/openssl/symhacks.h ../cryptlib.h bn_div.c bn_lcl.h
$(KBE_CONFIG)/bn_err.o: ../../include/openssl/bio.h ../../include/openssl/bn.h
$(KBE_CONFIG)/bn_err.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(KBE_CONFIG)/bn_err.o: ../../include/openssl/err.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/bn_err.o: ../../include/openssl/opensslconf.h ../../include/openssl/opensslv.h
$(KBE_CONFIG)/bn_err.o: ../../include/openssl/ossl_typ.h ../../include/openssl/safestack.h
$(KBE_CONFIG)/bn_err.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
$(KBE_CONFIG)/bn_err.o: bn_err.c
$(KBE_CONFIG)/bn_exp.o: ../../e_os.h ../../include/openssl/bio.h ../../include/openssl/bn.h
$(KBE_CONFIG)/bn_exp.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/bn_exp.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(KBE_CONFIG)/bn_exp.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/bn_exp.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/bn_exp.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/bn_exp.o: ../../include/openssl/symhacks.h ../cryptlib.h bn_exp.c bn_lcl.h
$(KBE_CONFIG)/bn_exp2.o: ../../e_os.h ../../include/openssl/bio.h ../../include/openssl/bn.h
$(KBE_CONFIG)/bn_exp2.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/bn_exp2.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(KBE_CONFIG)/bn_exp2.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/bn_exp2.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/bn_exp2.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/bn_exp2.o: ../../include/openssl/symhacks.h ../cryptlib.h bn_exp2.c bn_lcl.h
$(KBE_CONFIG)/bn_gcd.o: ../../e_os.h ../../include/openssl/bio.h ../../include/openssl/bn.h
$(KBE_CONFIG)/bn_gcd.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/bn_gcd.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(KBE_CONFIG)/bn_gcd.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/bn_gcd.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/bn_gcd.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/bn_gcd.o: ../../include/openssl/symhacks.h ../cryptlib.h bn_gcd.c bn_lcl.h
$(KBE_CONFIG)/bn_gf2m.o: ../../e_os.h ../../include/openssl/bio.h ../../include/openssl/bn.h
$(KBE_CONFIG)/bn_gf2m.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/bn_gf2m.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(KBE_CONFIG)/bn_gf2m.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/bn_gf2m.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/bn_gf2m.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/bn_gf2m.o: ../../include/openssl/symhacks.h ../cryptlib.h bn_gf2m.c bn_lcl.h
$(KBE_CONFIG)/bn_kron.o: ../../e_os.h ../../include/openssl/bio.h ../../include/openssl/bn.h
$(KBE_CONFIG)/bn_kron.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/bn_kron.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(KBE_CONFIG)/bn_kron.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/bn_kron.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/bn_kron.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/bn_kron.o: ../../include/openssl/symhacks.h ../cryptlib.h bn_kron.c bn_lcl.h
$(KBE_CONFIG)/bn_lib.o: ../../e_os.h ../../include/openssl/bio.h ../../include/openssl/bn.h
$(KBE_CONFIG)/bn_lib.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/bn_lib.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(KBE_CONFIG)/bn_lib.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/bn_lib.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/bn_lib.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/bn_lib.o: ../../include/openssl/symhacks.h ../cryptlib.h bn_lcl.h bn_lib.c
$(KBE_CONFIG)/bn_mod.o: ../../e_os.h ../../include/openssl/bio.h ../../include/openssl/bn.h
$(KBE_CONFIG)/bn_mod.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/bn_mod.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(KBE_CONFIG)/bn_mod.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/bn_mod.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/bn_mod.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/bn_mod.o: ../../include/openssl/symhacks.h ../cryptlib.h bn_lcl.h bn_mod.c
$(KBE_CONFIG)/bn_mont.o: ../../e_os.h ../../include/openssl/bio.h ../../include/openssl/bn.h
$(KBE_CONFIG)/bn_mont.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/bn_mont.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(KBE_CONFIG)/bn_mont.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/bn_mont.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/bn_mont.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/bn_mont.o: ../../include/openssl/symhacks.h ../cryptlib.h bn_lcl.h bn_mont.c
$(KBE_CONFIG)/bn_mpi.o: ../../e_os.h ../../include/openssl/bio.h ../../include/openssl/bn.h
$(KBE_CONFIG)/bn_mpi.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/bn_mpi.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(KBE_CONFIG)/bn_mpi.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/bn_mpi.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/bn_mpi.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/bn_mpi.o: ../../include/openssl/symhacks.h ../cryptlib.h bn_lcl.h bn_mpi.c
$(KBE_CONFIG)/bn_mul.o: ../../e_os.h ../../include/openssl/bio.h ../../include/openssl/bn.h
$(KBE_CONFIG)/bn_mul.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/bn_mul.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(KBE_CONFIG)/bn_mul.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/bn_mul.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/bn_mul.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/bn_mul.o: ../../include/openssl/symhacks.h ../cryptlib.h bn_lcl.h bn_mul.c
$(KBE_CONFIG)/bn_nist.o: ../../e_os.h ../../include/openssl/bio.h ../../include/openssl/bn.h
$(KBE_CONFIG)/bn_nist.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/bn_nist.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(KBE_CONFIG)/bn_nist.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/bn_nist.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/bn_nist.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/bn_nist.o: ../../include/openssl/symhacks.h ../cryptlib.h bn_lcl.h bn_nist.c
$(KBE_CONFIG)/bn_prime.o: ../../e_os.h ../../include/openssl/bio.h ../../include/openssl/bn.h
$(KBE_CONFIG)/bn_prime.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/bn_prime.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(KBE_CONFIG)/bn_prime.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/bn_prime.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/bn_prime.o: ../../include/openssl/rand.h ../../include/openssl/safestack.h
$(KBE_CONFIG)/bn_prime.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
$(KBE_CONFIG)/bn_prime.o: ../cryptlib.h bn_lcl.h bn_prime.c bn_prime.h
$(KBE_CONFIG)/bn_print.o: ../../e_os.h ../../include/openssl/bio.h ../../include/openssl/bn.h
$(KBE_CONFIG)/bn_print.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/bn_print.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(KBE_CONFIG)/bn_print.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/bn_print.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/bn_print.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/bn_print.o: ../../include/openssl/symhacks.h ../cryptlib.h bn_lcl.h bn_print.c
$(KBE_CONFIG)/bn_rand.o: ../../e_os.h ../../include/openssl/bio.h ../../include/openssl/bn.h
$(KBE_CONFIG)/bn_rand.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/bn_rand.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(KBE_CONFIG)/bn_rand.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/bn_rand.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/bn_rand.o: ../../include/openssl/rand.h ../../include/openssl/safestack.h
$(KBE_CONFIG)/bn_rand.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
$(KBE_CONFIG)/bn_rand.o: ../cryptlib.h bn_lcl.h bn_rand.c
$(KBE_CONFIG)/bn_recp.o: ../../e_os.h ../../include/openssl/bio.h ../../include/openssl/bn.h
$(KBE_CONFIG)/bn_recp.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/bn_recp.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(KBE_CONFIG)/bn_recp.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/bn_recp.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/bn_recp.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/bn_recp.o: ../../include/openssl/symhacks.h ../cryptlib.h bn_lcl.h bn_recp.c
$(KBE_CONFIG)/bn_shift.o: ../../e_os.h ../../include/openssl/bio.h ../../include/openssl/bn.h
$(KBE_CONFIG)/bn_shift.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/bn_shift.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(KBE_CONFIG)/bn_shift.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/bn_shift.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/bn_shift.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/bn_shift.o: ../../include/openssl/symhacks.h ../cryptlib.h bn_lcl.h bn_shift.c
$(KBE_CONFIG)/bn_sqr.o: ../../e_os.h ../../include/openssl/bio.h ../../include/openssl/bn.h
$(KBE_CONFIG)/bn_sqr.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/bn_sqr.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(KBE_CONFIG)/bn_sqr.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/bn_sqr.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/bn_sqr.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/bn_sqr.o: ../../include/openssl/symhacks.h ../cryptlib.h bn_lcl.h bn_sqr.c
$(KBE_CONFIG)/bn_sqrt.o: ../../e_os.h ../../include/openssl/bio.h ../../include/openssl/bn.h
$(KBE_CONFIG)/bn_sqrt.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/bn_sqrt.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(KBE_CONFIG)/bn_sqrt.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/bn_sqrt.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/bn_sqrt.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/bn_sqrt.o: ../../include/openssl/symhacks.h ../cryptlib.h bn_lcl.h bn_sqrt.c
$(KBE_CONFIG)/bn_word.o: ../../e_os.h ../../include/openssl/bio.h ../../include/openssl/bn.h
$(KBE_CONFIG)/bn_word.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/bn_word.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(KBE_CONFIG)/bn_word.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/bn_word.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/bn_word.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/bn_word.o: ../../include/openssl/symhacks.h ../cryptlib.h bn_lcl.h bn_word.c
