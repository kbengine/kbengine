#
# OpenSSL/crypto/comp/Makefile
#

DIR=	comp
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

LIB=$(BWLIBDIR)/libcrypto.a
LIBSRC= comp_lib.c comp_err.c \
	c_rle.c c_zlib.c

LIBOBJ=	$(MF_CONFIG)/comp_lib.o $(MF_CONFIG)/comp_err.o \
	$(MF_CONFIG)/c_rle.o $(MF_CONFIG)/c_zlib.o

SRC= $(LIBSRC)

EXHEADER= comp.h
HEADER=	$(EXHEADER)

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
	$(MAKEDEPEND) -- $(CFLAG) $(INCLUDES) $(DEPFLAG) -- $(LIBSRC)

dclean:
	$(PERL) -pe 'if (/^# DO NOT DELETE THIS LINE/) {print; exit(0);}' $(MAKEFILE) >Makefile.new
	mv -f Makefile.new $(MAKEFILE)

clean:
	rm -f $(MF_CONFIG)/*.o *.o *.obj lib tags core .pure .nfs* *.old *.bak fluff

# DO NOT DELETE THIS LINE -- make depend depends on it.

$(MF_CONFIG)/c_rle.o: ../../include/openssl/asn1.h ../../include/openssl/bio.h
$(MF_CONFIG)/c_rle.o: ../../include/openssl/comp.h ../../include/openssl/crypto.h
$(MF_CONFIG)/c_rle.o: ../../include/openssl/e_os2.h ../../include/openssl/obj_mac.h
$(MF_CONFIG)/c_rle.o: ../../include/openssl/objects.h ../../include/openssl/opensslconf.h
$(MF_CONFIG)/c_rle.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(MF_CONFIG)/c_rle.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(MF_CONFIG)/c_rle.o: ../../include/openssl/symhacks.h c_rle.c
$(MF_CONFIG)/c_zlib.o: ../../include/openssl/asn1.h ../../include/openssl/bio.h
$(MF_CONFIG)/c_zlib.o: ../../include/openssl/comp.h ../../include/openssl/crypto.h
$(MF_CONFIG)/c_zlib.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(MF_CONFIG)/c_zlib.o: ../../include/openssl/lhash.h ../../include/openssl/obj_mac.h
$(MF_CONFIG)/c_zlib.o: ../../include/openssl/objects.h ../../include/openssl/opensslconf.h
$(MF_CONFIG)/c_zlib.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(MF_CONFIG)/c_zlib.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(MF_CONFIG)/c_zlib.o: ../../include/openssl/symhacks.h c_zlib.c
$(MF_CONFIG)/comp_err.o: ../../include/openssl/bio.h ../../include/openssl/comp.h
$(MF_CONFIG)/comp_err.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(MF_CONFIG)/comp_err.o: ../../include/openssl/err.h ../../include/openssl/lhash.h
$(MF_CONFIG)/comp_err.o: ../../include/openssl/opensslconf.h
$(MF_CONFIG)/comp_err.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(MF_CONFIG)/comp_err.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(MF_CONFIG)/comp_err.o: ../../include/openssl/symhacks.h comp_err.c
$(MF_CONFIG)/comp_lib.o: ../../include/openssl/asn1.h ../../include/openssl/bio.h
$(MF_CONFIG)/comp_lib.o: ../../include/openssl/comp.h ../../include/openssl/crypto.h
$(MF_CONFIG)/comp_lib.o: ../../include/openssl/e_os2.h ../../include/openssl/obj_mac.h
$(MF_CONFIG)/comp_lib.o: ../../include/openssl/objects.h ../../include/openssl/opensslconf.h
$(MF_CONFIG)/comp_lib.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(MF_CONFIG)/comp_lib.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(MF_CONFIG)/comp_lib.o: ../../include/openssl/symhacks.h comp_lib.c
