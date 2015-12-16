#
# OpenSSL/crypto/idea/Makefile
#

ifndef KBE_ROOT
export KBE_ROOT := $(subst /kbe/src/lib/dependencies/openssl/crypto/idea,,$(CURDIR))
endif

LIBDIR= $(KBE_ROOT)/kbe/src/libs

DIR=	idea
TOP=	../..
CC=	cc
INCLUDES=
CFLAG=-g
MAKEFILE=	Makefile
AR=		ar r

CFLAGS= $(INCLUDES) $(CFLAG)

GENERAL=Makefile
TEST=ideatest.c
APPS=

LIB=$(LIBDIR)/libcrypto.a
LIBSRC=i_cbc.c i_cfb64.c i_ofb64.c i_ecb.c i_skey.c
LIBOBJ=$(KBE_CONFIG)/i_cbc.o $(KBE_CONFIG)/i_cfb64.o $(KBE_CONFIG)/i_ofb64.o $(KBE_CONFIG)/i_ecb.o $(KBE_CONFIG)/i_skey.o

SRC= $(LIBSRC)

EXHEADER= idea.h
HEADER=	idea_lcl.h $(EXHEADER)

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
	rm -f $(KBE_CONFIG)/*.o *.obj lib tags core .pure .nfs* *.old *.bak fluff

# DO NOT DELETE THIS LINE -- make depend depends on it.

$(KBE_CONFIG)/i_cbc.o: ../../include/openssl/idea.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/i_cbc.o: i_cbc.c idea_lcl.h
$(KBE_CONFIG)/i_cfb64.o: ../../include/openssl/idea.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/i_cfb64.o: i_cfb64.c idea_lcl.h
$(KBE_CONFIG)/i_ecb.o: ../../include/openssl/idea.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/i_ecb.o: ../../include/openssl/opensslv.h i_ecb.c idea_lcl.h
$(KBE_CONFIG)/i_ofb64.o: ../../include/openssl/idea.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/i_ofb64.o: i_ofb64.c idea_lcl.h
$(KBE_CONFIG)/i_skey.o: ../../include/openssl/idea.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/i_skey.o: i_skey.c idea_lcl.h
