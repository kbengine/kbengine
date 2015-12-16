#
# crypto/ec/Makefile
#

ifndef KBE_ROOT
export KBE_ROOT := $(subst /kbe/src/lib/dependencies/openssl/crypto/ec,,$(CURDIR))
endif

LIBDIR= $(KBE_ROOT)/kbe/src/libs

DIR=	ec
TOP=	../..
CC=	cc
INCLUDES= -I.. -I$(TOP) -I../../include
CFLAG=-g
MAKEFILE=	Makefile
AR=		ar r

CFLAGS= $(INCLUDES) $(CFLAG)

GENERAL=Makefile
TEST=ectest.c
APPS=

LIB=$(LIBDIR)/libcrypto.a
LIBSRC=	ec_lib.c ecp_smpl.c ecp_mont.c ecp_nist.c ec_cvt.c ec_mult.c\
	ec_err.c ec_curve.c ec_check.c ec_print.c ec_asn1.c ec_key.c\
	ec2_smpl.c ec2_smpt.c ec2_mult.c

LIBOBJ=	$(KBE_CONFIG)/ec_lib.o $(KBE_CONFIG)/ecp_smpl.o $(KBE_CONFIG)/ecp_mont.o \
	$(KBE_CONFIG)/ecp_nist.o $(KBE_CONFIG)/ec_cvt.o $(KBE_CONFIG)/ec_mult.o \
	$(KBE_CONFIG)/ec_err.o $(KBE_CONFIG)/ec_curve.o $(KBE_CONFIG)/ec_check.o \
	$(KBE_CONFIG)/ec_print.o $(KBE_CONFIG)/ec_asn1.o $(KBE_CONFIG)/ec_key.o \
	$(KBE_CONFIG)/ec2_smpl.o $(KBE_CONFIG)/ec2_mult.o

SRC= $(LIBSRC)

EXHEADER= ec.h
HEADER=	ec_lcl.h $(EXHEADER)

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

$(KBE_CONFIG)/ec2_mult.o: ../../include/openssl/asn1.h ../../include/openssl/bio.h
$(KBE_CONFIG)/ec2_mult.o: ../../include/openssl/bn.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/ec2_mult.o: ../../include/openssl/e_os2.h ../../include/openssl/ec.h
$(KBE_CONFIG)/ec2_mult.o: ../../include/openssl/err.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/ec2_mult.o: ../../include/openssl/obj_mac.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/ec2_mult.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/ec2_mult.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/ec2_mult.o: ../../include/openssl/symhacks.h ec2_mult.c ec_lcl.h
$(KBE_CONFIG)/ec2_smpl.o: ../../include/openssl/asn1.h ../../include/openssl/bio.h
$(KBE_CONFIG)/ec2_smpl.o: ../../include/openssl/bn.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/ec2_smpl.o: ../../include/openssl/e_os2.h ../../include/openssl/ec.h
$(KBE_CONFIG)/ec2_smpl.o: ../../include/openssl/err.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/ec2_smpl.o: ../../include/openssl/obj_mac.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/ec2_smpl.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/ec2_smpl.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/ec2_smpl.o: ../../include/openssl/symhacks.h ec2_smpl.c ec2_smpt.c ec_lcl.h
$(KBE_CONFIG)/ec2_smpt.o: ec2_smpt.c
$(KBE_CONFIG)/ec_asn1.o: ../../include/openssl/asn1.h ../../include/openssl/asn1t.h
$(KBE_CONFIG)/ec_asn1.o: ../../include/openssl/bio.h ../../include/openssl/bn.h
$(KBE_CONFIG)/ec_asn1.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(KBE_CONFIG)/ec_asn1.o: ../../include/openssl/ec.h ../../include/openssl/err.h
$(KBE_CONFIG)/ec_asn1.o: ../../include/openssl/lhash.h ../../include/openssl/obj_mac.h
$(KBE_CONFIG)/ec_asn1.o: ../../include/openssl/objects.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/ec_asn1.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/ec_asn1.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/ec_asn1.o: ../../include/openssl/symhacks.h ec_asn1.c ec_lcl.h
$(KBE_CONFIG)/ec_check.o: ../../include/openssl/asn1.h ../../include/openssl/bio.h
$(KBE_CONFIG)/ec_check.o: ../../include/openssl/bn.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/ec_check.o: ../../include/openssl/e_os2.h ../../include/openssl/ec.h
$(KBE_CONFIG)/ec_check.o: ../../include/openssl/err.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/ec_check.o: ../../include/openssl/obj_mac.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/ec_check.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/ec_check.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/ec_check.o: ../../include/openssl/symhacks.h ec_check.c ec_lcl.h
$(KBE_CONFIG)/ec_curve.o: ../../include/openssl/asn1.h ../../include/openssl/bio.h
$(KBE_CONFIG)/ec_curve.o: ../../include/openssl/bn.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/ec_curve.o: ../../include/openssl/e_os2.h ../../include/openssl/ec.h
$(KBE_CONFIG)/ec_curve.o: ../../include/openssl/err.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/ec_curve.o: ../../include/openssl/obj_mac.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/ec_curve.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/ec_curve.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/ec_curve.o: ../../include/openssl/symhacks.h ec_curve.c ec_lcl.h
$(KBE_CONFIG)/ec_cvt.o: ../../include/openssl/asn1.h ../../include/openssl/bio.h
$(KBE_CONFIG)/ec_cvt.o: ../../include/openssl/bn.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/ec_cvt.o: ../../include/openssl/e_os2.h ../../include/openssl/ec.h
$(KBE_CONFIG)/ec_cvt.o: ../../include/openssl/err.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/ec_cvt.o: ../../include/openssl/obj_mac.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/ec_cvt.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/ec_cvt.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/ec_cvt.o: ../../include/openssl/symhacks.h ec_cvt.c ec_lcl.h
$(KBE_CONFIG)/ec_err.o: ../../include/openssl/asn1.h ../../include/openssl/bio.h
$(KBE_CONFIG)/ec_err.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(KBE_CONFIG)/ec_err.o: ../../include/openssl/ec.h ../../include/openssl/err.h
$(KBE_CONFIG)/ec_err.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/ec_err.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/ec_err.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/ec_err.o: ../../include/openssl/symhacks.h ec_err.c
$(KBE_CONFIG)/ec_key.o: ../../include/openssl/asn1.h ../../include/openssl/bio.h
$(KBE_CONFIG)/ec_key.o: ../../include/openssl/bn.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/ec_key.o: ../../include/openssl/e_os2.h ../../include/openssl/ec.h
$(KBE_CONFIG)/ec_key.o: ../../include/openssl/err.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/ec_key.o: ../../include/openssl/obj_mac.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/ec_key.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/ec_key.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/ec_key.o: ../../include/openssl/symhacks.h ec_key.c ec_lcl.h
$(KBE_CONFIG)/ec_lib.o: ../../include/openssl/asn1.h ../../include/openssl/bio.h
$(KBE_CONFIG)/ec_lib.o: ../../include/openssl/bn.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/ec_lib.o: ../../include/openssl/e_os2.h ../../include/openssl/ec.h
$(KBE_CONFIG)/ec_lib.o: ../../include/openssl/err.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/ec_lib.o: ../../include/openssl/obj_mac.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/ec_lib.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/ec_lib.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/ec_lib.o: ../../include/openssl/symhacks.h ec_lcl.h ec_lib.c
$(KBE_CONFIG)/ec_mult.o: ../../include/openssl/asn1.h ../../include/openssl/bio.h
$(KBE_CONFIG)/ec_mult.o: ../../include/openssl/bn.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/ec_mult.o: ../../include/openssl/e_os2.h ../../include/openssl/ec.h
$(KBE_CONFIG)/ec_mult.o: ../../include/openssl/err.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/ec_mult.o: ../../include/openssl/obj_mac.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/ec_mult.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/ec_mult.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/ec_mult.o: ../../include/openssl/symhacks.h ec_lcl.h ec_mult.c
$(KBE_CONFIG)/ec_print.o: ../../include/openssl/asn1.h ../../include/openssl/bio.h
$(KBE_CONFIG)/ec_print.o: ../../include/openssl/bn.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/ec_print.o: ../../include/openssl/e_os2.h ../../include/openssl/ec.h
$(KBE_CONFIG)/ec_print.o: ../../include/openssl/obj_mac.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/ec_print.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/ec_print.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/ec_print.o: ../../include/openssl/symhacks.h ec_lcl.h ec_print.c
$(KBE_CONFIG)/ecp_mont.o: ../../include/openssl/asn1.h ../../include/openssl/bio.h
$(KBE_CONFIG)/ecp_mont.o: ../../include/openssl/bn.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/ecp_mont.o: ../../include/openssl/e_os2.h ../../include/openssl/ec.h
$(KBE_CONFIG)/ecp_mont.o: ../../include/openssl/err.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/ecp_mont.o: ../../include/openssl/obj_mac.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/ecp_mont.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/ecp_mont.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/ecp_mont.o: ../../include/openssl/symhacks.h ec_lcl.h ecp_mont.c
$(KBE_CONFIG)/ecp_nist.o: ../../include/openssl/asn1.h ../../include/openssl/bio.h
$(KBE_CONFIG)/ecp_nist.o: ../../include/openssl/bn.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/ecp_nist.o: ../../include/openssl/e_os2.h ../../include/openssl/ec.h
$(KBE_CONFIG)/ecp_nist.o: ../../include/openssl/err.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/ecp_nist.o: ../../include/openssl/obj_mac.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/ecp_nist.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/ecp_nist.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/ecp_nist.o: ../../include/openssl/symhacks.h ec_lcl.h ecp_nist.c
$(KBE_CONFIG)/ecp_smpl.o: ../../include/openssl/asn1.h ../../include/openssl/bio.h
$(KBE_CONFIG)/ecp_smpl.o: ../../include/openssl/bn.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/ecp_smpl.o: ../../include/openssl/e_os2.h ../../include/openssl/ec.h
$(KBE_CONFIG)/ecp_smpl.o: ../../include/openssl/err.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/ecp_smpl.o: ../../include/openssl/obj_mac.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/ecp_smpl.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/ecp_smpl.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/ecp_smpl.o: ../../include/openssl/symhacks.h ec_lcl.h ecp_smpl.c
