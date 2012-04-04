#
# OpenSSL/crypto/bio/Makefile
#

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

LIB=$(BWLIBDIR)/libcrypto.a
LIBSRC= bio_lib.c bio_cb.c bio_err.c \
	bss_mem.c bss_null.c bss_fd.c \
	bss_file.c bss_sock.c bss_conn.c \
	bf_null.c bf_buff.c b_print.c b_dump.c \
	b_sock.c bss_acpt.c bf_nbio.c bss_log.c bss_bio.c \
	bss_dgram.c
#	bf_lbuf.c
LIBOBJ= $(MF_CONFIG)/bio_lib.o $(MF_CONFIG)/bio_cb.o \
	$(MF_CONFIG)/bio_err.o $(MF_CONFIG)/bss_mem.o $(MF_CONFIG)/bss_null.o \
	$(MF_CONFIG)/bss_fd.o $(MF_CONFIG)/bss_file.o $(MF_CONFIG)/bss_sock.o \
	$(MF_CONFIG)/bss_conn.o $(MF_CONFIG)/bf_null.o $(MF_CONFIG)/bf_buff.o \
	$(MF_CONFIG)/b_print.o $(MF_CONFIG)/b_dump.o $(MF_CONFIG)/b_sock.o \
	$(MF_CONFIG)/bss_acpt.o $(MF_CONFIG)/bf_nbio.o $(MF_CONFIG)/bss_log.o \
	$(MF_CONFIG)/bss_bio.o $(MF_CONFIG)/bss_dgram.o
#	bf_lbuf.o

SRC= $(LIBSRC)

EXHEADER= bio.h
HEADER=	bio_lcl.h $(EXHEADER)

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
	rm -f $(MF_CONFIG)/*.o *.o *.obj lib tags core .pure .nfs* *.old *.bak fluff

# DO NOT DELETE THIS LINE -- make depend depends on it.

$(MF_CONFIG)/b_dump.o: ../../e_os.h ../../include/openssl/bio.h
$(MF_CONFIG)/b_dump.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(MF_CONFIG)/b_dump.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(MF_CONFIG)/b_dump.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(MF_CONFIG)/b_dump.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(MF_CONFIG)/b_dump.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(MF_CONFIG)/b_dump.o: ../../include/openssl/symhacks.h ../cryptlib.h b_dump.c bio_lcl.h
$(MF_CONFIG)/b_print.o: ../../e_os.h ../../include/openssl/bio.h ../../include/openssl/bn.h
$(MF_CONFIG)/b_print.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(MF_CONFIG)/b_print.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(MF_CONFIG)/b_print.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(MF_CONFIG)/b_print.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(MF_CONFIG)/b_print.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(MF_CONFIG)/b_print.o: ../../include/openssl/symhacks.h ../cryptlib.h b_print.c
$(MF_CONFIG)/b_sock.o: ../../e_os.h ../../include/openssl/bio.h
$(MF_CONFIG)/b_sock.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(MF_CONFIG)/b_sock.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(MF_CONFIG)/b_sock.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(MF_CONFIG)/b_sock.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(MF_CONFIG)/b_sock.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(MF_CONFIG)/b_sock.o: ../../include/openssl/symhacks.h ../cryptlib.h b_sock.c
$(MF_CONFIG)/bf_buff.o: ../../e_os.h ../../include/openssl/bio.h
$(MF_CONFIG)/bf_buff.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(MF_CONFIG)/bf_buff.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(MF_CONFIG)/bf_buff.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(MF_CONFIG)/bf_buff.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(MF_CONFIG)/bf_buff.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(MF_CONFIG)/bf_buff.o: ../../include/openssl/symhacks.h ../cryptlib.h bf_buff.c
$(MF_CONFIG)/bf_nbio.o: ../../e_os.h ../../include/openssl/bio.h
$(MF_CONFIG)/bf_nbio.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(MF_CONFIG)/bf_nbio.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(MF_CONFIG)/bf_nbio.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(MF_CONFIG)/bf_nbio.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(MF_CONFIG)/bf_nbio.o: ../../include/openssl/rand.h ../../include/openssl/safestack.h
$(MF_CONFIG)/bf_nbio.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
$(MF_CONFIG)/bf_nbio.o: ../cryptlib.h bf_nbio.c
$(MF_CONFIG)/bf_null.o: ../../e_os.h ../../include/openssl/bio.h
$(MF_CONFIG)/bf_null.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(MF_CONFIG)/bf_null.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(MF_CONFIG)/bf_null.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(MF_CONFIG)/bf_null.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(MF_CONFIG)/bf_null.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(MF_CONFIG)/bf_null.o: ../../include/openssl/symhacks.h ../cryptlib.h bf_null.c
$(MF_CONFIG)/bio_cb.o: ../../e_os.h ../../include/openssl/bio.h
$(MF_CONFIG)/bio_cb.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(MF_CONFIG)/bio_cb.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(MF_CONFIG)/bio_cb.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(MF_CONFIG)/bio_cb.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(MF_CONFIG)/bio_cb.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(MF_CONFIG)/bio_cb.o: ../../include/openssl/symhacks.h ../cryptlib.h bio_cb.c
$(MF_CONFIG)/bio_err.o: ../../include/openssl/bio.h ../../include/openssl/crypto.h
$(MF_CONFIG)/bio_err.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(MF_CONFIG)/bio_err.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(MF_CONFIG)/bio_err.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(MF_CONFIG)/bio_err.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(MF_CONFIG)/bio_err.o: ../../include/openssl/symhacks.h bio_err.c
$(MF_CONFIG)/bio_lib.o: ../../e_os.h ../../include/openssl/bio.h
$(MF_CONFIG)/bio_lib.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(MF_CONFIG)/bio_lib.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(MF_CONFIG)/bio_lib.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(MF_CONFIG)/bio_lib.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(MF_CONFIG)/bio_lib.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(MF_CONFIG)/bio_lib.o: ../../include/openssl/symhacks.h ../cryptlib.h bio_lib.c
$(MF_CONFIG)/bss_acpt.o: ../../e_os.h ../../include/openssl/bio.h
$(MF_CONFIG)/bss_acpt.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(MF_CONFIG)/bss_acpt.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(MF_CONFIG)/bss_acpt.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(MF_CONFIG)/bss_acpt.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(MF_CONFIG)/bss_acpt.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(MF_CONFIG)/bss_acpt.o: ../../include/openssl/symhacks.h ../cryptlib.h bss_acpt.c
$(MF_CONFIG)/bss_bio.o: ../../e_os.h ../../include/openssl/bio.h
$(MF_CONFIG)/bss_bio.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
$(MF_CONFIG)/bss_bio.o: ../../include/openssl/err.h ../../include/openssl/lhash.h
$(MF_CONFIG)/bss_bio.o: ../../include/openssl/opensslconf.h ../../include/openssl/opensslv.h
$(MF_CONFIG)/bss_bio.o: ../../include/openssl/ossl_typ.h ../../include/openssl/safestack.h
$(MF_CONFIG)/bss_bio.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
$(MF_CONFIG)/bss_bio.o: bss_bio.c
$(MF_CONFIG)/bss_conn.o: ../../e_os.h ../../include/openssl/bio.h
$(MF_CONFIG)/bss_conn.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(MF_CONFIG)/bss_conn.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(MF_CONFIG)/bss_conn.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(MF_CONFIG)/bss_conn.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(MF_CONFIG)/bss_conn.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(MF_CONFIG)/bss_conn.o: ../../include/openssl/symhacks.h ../cryptlib.h bss_conn.c
$(MF_CONFIG)/bss_dgram.o: ../../e_os.h ../../include/openssl/bio.h
$(MF_CONFIG)/bss_dgram.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(MF_CONFIG)/bss_dgram.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(MF_CONFIG)/bss_dgram.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(MF_CONFIG)/bss_dgram.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(MF_CONFIG)/bss_dgram.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(MF_CONFIG)/bss_dgram.o: ../../include/openssl/symhacks.h ../cryptlib.h bss_dgram.c
$(MF_CONFIG)/bss_fd.o: ../../e_os.h ../../include/openssl/bio.h
$(MF_CONFIG)/bss_fd.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(MF_CONFIG)/bss_fd.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(MF_CONFIG)/bss_fd.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(MF_CONFIG)/bss_fd.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(MF_CONFIG)/bss_fd.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(MF_CONFIG)/bss_fd.o: ../../include/openssl/symhacks.h ../cryptlib.h bio_lcl.h bss_fd.c
$(MF_CONFIG)/bss_file.o: ../../e_os.h ../../include/openssl/bio.h
$(MF_CONFIG)/bss_file.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(MF_CONFIG)/bss_file.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(MF_CONFIG)/bss_file.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(MF_CONFIG)/bss_file.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(MF_CONFIG)/bss_file.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(MF_CONFIG)/bss_file.o: ../../include/openssl/symhacks.h ../cryptlib.h bio_lcl.h bss_file.c
$(MF_CONFIG)/bss_log.o: ../../e_os.h ../../include/openssl/bio.h
$(MF_CONFIG)/bss_log.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(MF_CONFIG)/bss_log.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(MF_CONFIG)/bss_log.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(MF_CONFIG)/bss_log.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(MF_CONFIG)/bss_log.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(MF_CONFIG)/bss_log.o: ../../include/openssl/symhacks.h ../cryptlib.h bss_log.c
$(MF_CONFIG)/bss_mem.o: ../../e_os.h ../../include/openssl/bio.h
$(MF_CONFIG)/bss_mem.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(MF_CONFIG)/bss_mem.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(MF_CONFIG)/bss_mem.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(MF_CONFIG)/bss_mem.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(MF_CONFIG)/bss_mem.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(MF_CONFIG)/bss_mem.o: ../../include/openssl/symhacks.h ../cryptlib.h bss_mem.c
$(MF_CONFIG)/bss_null.o: ../../e_os.h ../../include/openssl/bio.h
$(MF_CONFIG)/bss_null.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(MF_CONFIG)/bss_null.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(MF_CONFIG)/bss_null.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(MF_CONFIG)/bss_null.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(MF_CONFIG)/bss_null.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(MF_CONFIG)/bss_null.o: ../../include/openssl/symhacks.h ../cryptlib.h bss_null.c
$(MF_CONFIG)/bss_sock.o: ../../e_os.h ../../include/openssl/bio.h
$(MF_CONFIG)/bss_sock.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
$(MF_CONFIG)/bss_sock.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
$(MF_CONFIG)/bss_sock.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
$(MF_CONFIG)/bss_sock.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
$(MF_CONFIG)/bss_sock.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
$(MF_CONFIG)/bss_sock.o: ../../include/openssl/symhacks.h ../cryptlib.h bss_sock.c
