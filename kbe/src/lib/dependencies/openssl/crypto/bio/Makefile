#
# OpenSSL/crypto/bio/Makefile
#

ifndef KBE_ROOT
export KBE_ROOT := $(subst /kbe/src/lib/dependencies/openssl/crypto/bio,,$(CURDIR))
endif

LIBDIR= $(KBE_ROOT)/kbe/src/libs

DIR=	bio
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
LIBSRC= bio_lib.c bio_cb.c bio_err.c \
	bss_mem.c bss_null.c bss_fd.c \
	bss_file.c bss_sock.c bss_conn.c \
	bf_null.c bf_buff.c b_print.c b_dump.c \
	b_sock.c bss_acpt.c bf_nbio.c bss_log.c bss_bio.c \
	bss_dgram.c
#	bf_lbuf.c
LIBOBJ= $(KBE_CONFIG)/bio_lib.o $(KBE_CONFIG)/bio_cb.o \
	$(KBE_CONFIG)/bio_err.o $(KBE_CONFIG)/bss_mem.o $(KBE_CONFIG)/bss_null.o \
	$(KBE_CONFIG)/bss_fd.o $(KBE_CONFIG)/bss_file.o $(KBE_CONFIG)/bss_sock.o \
	$(KBE_CONFIG)/bss_conn.o $(KBE_CONFIG)/bf_null.o $(KBE_CONFIG)/bf_buff.o \
	$(KBE_CONFIG)/b_print.o $(KBE_CONFIG)/b_dump.o $(KBE_CONFIG)/b_sock.o \
	$(KBE_CONFIG)/bss_acpt.o $(KBE_CONFIG)/bf_nbio.o $(KBE_CONFIG)/bss_log.o \
	$(KBE_CONFIG)/bss_bio.o $(KBE_CONFIG)/bss_dgram.o
#	bf_lbuf.o

SRC= $(LIBSRC)

EXHEADER= bio.h
HEADER=	bio_lcl.h $(EXHEADER)

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
	@headerlist="$(EXHEADER)"; for i in $$headerlist; \
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

$(KBE_CONFIG)/b_dump.o: ../../e_os.h ../../include/openssl/bio.h
$(KBE_CONFIG)/b_dump.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/b_dump.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(KBE_CONFIG)/b_dump.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/b_dump.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/b_dump.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/b_dump.o: ../../include/openssl/symhacks.h ../cryptlib.h b_dump.c bio_lcl.h
$(KBE_CONFIG)/b_print.o: ../../e_os.h ../../include/openssl/bio.h ../../include/openssl/bn.h
$(KBE_CONFIG)/b_print.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/b_print.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(KBE_CONFIG)/b_print.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/b_print.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/b_print.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/b_print.o: ../../include/openssl/symhacks.h ../cryptlib.h b_print.c
$(KBE_CONFIG)/b_sock.o: ../../e_os.h ../../include/openssl/bio.h
$(KBE_CONFIG)/b_sock.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/b_sock.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(KBE_CONFIG)/b_sock.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/b_sock.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/b_sock.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/b_sock.o: ../../include/openssl/symhacks.h ../cryptlib.h b_sock.c
$(KBE_CONFIG)/bf_buff.o: ../../e_os.h ../../include/openssl/bio.h
$(KBE_CONFIG)/bf_buff.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/bf_buff.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(KBE_CONFIG)/bf_buff.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/bf_buff.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/bf_buff.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/bf_buff.o: ../../include/openssl/symhacks.h ../cryptlib.h bf_buff.c
$(KBE_CONFIG)/bf_nbio.o: ../../e_os.h ../../include/openssl/bio.h
$(KBE_CONFIG)/bf_nbio.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/bf_nbio.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(KBE_CONFIG)/bf_nbio.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/bf_nbio.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/bf_nbio.o: ../../include/openssl/rand.h ../../include/openssl/safestack.h
$(KBE_CONFIG)/bf_nbio.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
$(KBE_CONFIG)/bf_nbio.o: ../cryptlib.h bf_nbio.c
$(KBE_CONFIG)/bf_null.o: ../../e_os.h ../../include/openssl/bio.h
$(KBE_CONFIG)/bf_null.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/bf_null.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(KBE_CONFIG)/bf_null.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/bf_null.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/bf_null.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/bf_null.o: ../../include/openssl/symhacks.h ../cryptlib.h bf_null.c
$(KBE_CONFIG)/bio_cb.o: ../../e_os.h ../../include/openssl/bio.h
$(KBE_CONFIG)/bio_cb.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/bio_cb.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(KBE_CONFIG)/bio_cb.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/bio_cb.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/bio_cb.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/bio_cb.o: ../../include/openssl/symhacks.h ../cryptlib.h bio_cb.c
$(KBE_CONFIG)/bio_err.o: ../../include/openssl/bio.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/bio_err.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(KBE_CONFIG)/bio_err.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/bio_err.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/bio_err.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/bio_err.o: ../../include/openssl/symhacks.h bio_err.c
$(KBE_CONFIG)/bio_lib.o: ../../e_os.h ../../include/openssl/bio.h
$(KBE_CONFIG)/bio_lib.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/bio_lib.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(KBE_CONFIG)/bio_lib.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/bio_lib.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/bio_lib.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/bio_lib.o: ../../include/openssl/symhacks.h ../cryptlib.h bio_lib.c
$(KBE_CONFIG)/bss_acpt.o: ../../e_os.h ../../include/openssl/bio.h
$(KBE_CONFIG)/bss_acpt.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/bss_acpt.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(KBE_CONFIG)/bss_acpt.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/bss_acpt.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/bss_acpt.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/bss_acpt.o: ../../include/openssl/symhacks.h ../cryptlib.h bss_acpt.c
$(KBE_CONFIG)/bss_bio.o: ../../e_os.h ../../include/openssl/bio.h
$(KBE_CONFIG)/bss_bio.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(KBE_CONFIG)/bss_bio.o: ../../include/openssl/err.h ../../include/openssl/lhash.h
$(KBE_CONFIG)/bss_bio.o: ../../include/openssl/opensslconf.h ../../include/openssl/opensslv.h
$(KBE_CONFIG)/bss_bio.o: ../../include/openssl/ossl_typ.h ../../include/openssl/safestack.h
$(KBE_CONFIG)/bss_bio.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
$(KBE_CONFIG)/bss_bio.o: bss_bio.c
$(KBE_CONFIG)/bss_conn.o: ../../e_os.h ../../include/openssl/bio.h
$(KBE_CONFIG)/bss_conn.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/bss_conn.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(KBE_CONFIG)/bss_conn.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/bss_conn.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/bss_conn.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/bss_conn.o: ../../include/openssl/symhacks.h ../cryptlib.h bss_conn.c
$(KBE_CONFIG)/bss_dgram.o: ../../e_os.h ../../include/openssl/bio.h
$(KBE_CONFIG)/bss_dgram.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/bss_dgram.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(KBE_CONFIG)/bss_dgram.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/bss_dgram.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/bss_dgram.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/bss_dgram.o: ../../include/openssl/symhacks.h ../cryptlib.h bss_dgram.c
$(KBE_CONFIG)/bss_fd.o: ../../e_os.h ../../include/openssl/bio.h
$(KBE_CONFIG)/bss_fd.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/bss_fd.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(KBE_CONFIG)/bss_fd.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/bss_fd.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/bss_fd.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/bss_fd.o: ../../include/openssl/symhacks.h ../cryptlib.h bio_lcl.h bss_fd.c
$(KBE_CONFIG)/bss_file.o: ../../e_os.h ../../include/openssl/bio.h
$(KBE_CONFIG)/bss_file.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/bss_file.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(KBE_CONFIG)/bss_file.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/bss_file.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/bss_file.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/bss_file.o: ../../include/openssl/symhacks.h ../cryptlib.h bio_lcl.h bss_file.c
$(KBE_CONFIG)/bss_log.o: ../../e_os.h ../../include/openssl/bio.h
$(KBE_CONFIG)/bss_log.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/bss_log.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(KBE_CONFIG)/bss_log.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/bss_log.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/bss_log.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/bss_log.o: ../../include/openssl/symhacks.h ../cryptlib.h bss_log.c
$(KBE_CONFIG)/bss_mem.o: ../../e_os.h ../../include/openssl/bio.h
$(KBE_CONFIG)/bss_mem.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/bss_mem.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(KBE_CONFIG)/bss_mem.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/bss_mem.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/bss_mem.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/bss_mem.o: ../../include/openssl/symhacks.h ../cryptlib.h bss_mem.c
$(KBE_CONFIG)/bss_null.o: ../../e_os.h ../../include/openssl/bio.h
$(KBE_CONFIG)/bss_null.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/bss_null.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(KBE_CONFIG)/bss_null.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/bss_null.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/bss_null.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/bss_null.o: ../../include/openssl/symhacks.h ../cryptlib.h bss_null.c
$(KBE_CONFIG)/bss_sock.o: ../../e_os.h ../../include/openssl/bio.h
$(KBE_CONFIG)/bss_sock.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(KBE_CONFIG)/bss_sock.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(KBE_CONFIG)/bss_sock.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(KBE_CONFIG)/bss_sock.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(KBE_CONFIG)/bss_sock.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(KBE_CONFIG)/bss_sock.o: ../../include/openssl/symhacks.h ../cryptlib.h bss_sock.c
