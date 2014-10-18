#
# OpenSSL/crypto/md5/Makefile
#

ifndef KBE_ROOT
export KBE_ROOT := $(subst /kbe/src/lib/dependencies/openssl/crypto/md5,,$(CURDIR))
endif

LIBDIR= $(KBE_ROOT)/kbe/src/libs


DIR=    md5
TOP=    ../..
CC=     cc
CPP=    $(CC) -E
INCLUDES=-I.. -I$(TOP) -I../../include
CFLAG=-g
MAKEFILE=       Makefile
AR=             ar r

MD5_ASM_OBJ=

CFLAGS= $(INCLUDES) $(CFLAG)
ASFLAGS= $(INCLUDES) $(ASFLAG)
AFLAGS= $(ASFLAGS)

GENERAL=Makefile
TEST=md5test.c
APPS=

LIB=$(LIBDIR)/libcrypto.a
LIBSRC=md5_dgst.c md5_one.c
LIBOBJ=$(KBE_CONFIG)/md5_dgst.o $(KBE_CONFIG)/md5_one.o $(MD5_ASM_OBJ)

SRC= $(LIBSRC)

EXHEADER= md5.h
HEADER= md5_locl.h $(EXHEADER)

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
$(KBE_CONFIG)/mx86-elf.s: asm/md5-586.pl ../perlasm/x86asm.pl
	(cd asm; $(PERL) md5-586.pl elf $(CFLAGS) > ../$@)
# COFF
$(KBE_CONFIG)/mx86-cof.s: asm/md5-586.pl ../perlasm/x86asm.pl
	(cd asm; $(PERL) md5-586.pl coff $(CFLAGS) > ../$@)
# a.out
$(KBE_CONFIG)/mx86-out.s: asm/md5-586.pl ../perlasm/x86asm.pl
	(cd asm; $(PERL) md5-586.pl a.out $(CFLAGS) > ../$@)

md5-sparcv8plus.o: asm/md5-sparcv9.S
	$(CC) $(ASFLAGS) -DMD5_BLOCK_DATA_ORDER -c \
		-o md5-sparcv8plus.o asm/md5-sparcv9.S

# Old GNU assembler doesn't understand V9 instructions, so we
# hire /usr/ccs/bin/as to do the job. Note that option is called
# *-gcc27, but even gcc 2>=8 users may experience similar problem
# if they didn't bother to upgrade GNU assembler. Such users should
# not choose this option, but be adviced to *remove* GNU assembler
# or upgrade it.
md5-sparcv8plus-gcc27.o: asm/md5-sparcv9.S
	$(CC) $(ASFLAGS) -DMD5_BLOCK_DATA_ORDER -E asm/md5-sparcv9.S | \
		/usr/ccs/bin/as -xarch=v8plus - -o md5-sparcv8plus-gcc27.o

md5-sparcv9.o: asm/md5-sparcv9.S
	$(CC) $(ASFLAGS) -DMD5_BLOCK_DATA_ORDER -c \
		-o md5-sparcv9.o asm/md5-sparcv9.S

$(KBE_CONFIG)/md5-x86_64.s:	asm/md5-x86_64.pl;	$(PERL) asm/md5-x86_64.pl $@

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

$(KBE_CONFIG)/md5_dgst.o: ../../include/openssl/e_os2.h ../../include/openssl/md5.h
$(KBE_CONFIG)/md5_dgst.o: ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/md5_dgst.o: ../../include/openssl/opensslv.h ../md32_common.h md5_dgst.c
$(KBE_CONFIG)/md5_dgst.o: md5_locl.h
$(KBE_CONFIG)/md5_one.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(KBE_CONFIG)/md5_one.o: ../../include/openssl/md5.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/md5_one.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/md5_one.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/md5_one.o: ../../include/openssl/symhacks.h md5_one.c
