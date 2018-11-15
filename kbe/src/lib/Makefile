ifndef KBE_ROOT
export KBE_ROOT := $(subst /kbe/src/lib,,$(CURDIR))
endif

ifndef KBE_CONFIG
	KBE_CONFIG=Hybrid
	ifeq ($(shell uname -m),x86_64)
		 KBE_CONFIG=Hybrid64
	endif
endif

ifneq (,$(findstring 64, $(KBE_CONFIG)))
	OPENSSL_CONFIG+="x86_64=1"
else
	OPENSSL_CONFIG+=
endif

# This directory must match the directory in src/build/common.mak
LIBDIR=$(KBE_ROOT)/kbe/src/lib/bin

all install clean full:
	test -d $(LIBDIR) || mkdir $(LIBDIR)
	cd dependencies/fmt/src && $(MAKE) $@
	cd dependencies/jemalloc && $(MAKE) $@ && if ! test -e ../../../libs/libjemalloc.a; then \cp -f "./lib/libjemalloc.a" "../../../libs"; fi
	cd dependencies/openssl && $(MAKE) $@ && if ! test -e ../../../libs/libssl.a; then \cp -f "libssl.a" "../../../libs"; fi && if ! test -e ../../../libs/libcrypto.a; then \cp -f "libcrypto.a" "../../../libs"; fi
	cd dependencies/curl && $(MAKE) $@
	cd dependencies/hiredis && $(MAKE) $@
	cd dependencies/zip && $(MAKE) $@
	cd dependencies/g3dlite && $(MAKE) $@
	cd dependencies/tinyxml && $(MAKE) $@
	cd dependencies/sigar/linux && $(MAKE) $@
	cd dependencies/tmxparser && $(MAKE) $@
	cd dependencies/jwsmtp/jwsmtp/jwsmtp && $(MAKE) $@
	cd dependencies/apr && $(MAKE) $@ && if ! test -e ../../../libs/libapr-1.a; then \cp -f ".libs/libapr-1.a" "../../../libs"; fi
	cd dependencies/expat && $(MAKE) $@ && if ! test -e ../../../libs/libexpat.a; then \cp -f "./.libs/libexpat.a" "../../../libs"; fi
	cd dependencies/apr-util && $(MAKE) $@ && if ! test -e ../../../libs/libaprutil-1.a; then \cp -f ".libs/libaprutil-1.a" "../../../libs"; fi
	cd dependencies/log4cxx && $(MAKE) $@ && if ! test -e ../../../libs/liblog4cxx.a; then \cp -f "src/main/cpp/.libs/liblog4cxx.a" "../../../libs"; fi
	cd python && $(MAKE) $@
	cd client_lib && $(MAKE) $@
	cd common && $(MAKE) $@
	cd db_redis && $(MAKE) $@
	cd db_mysql && $(MAKE) $@
	cd db_interface && $(MAKE) $@
	cd entitydef && $(MAKE) $@
	cd math && $(MAKE) $@
	cd resmgr && $(MAKE) $@
	cd pyscript && $(MAKE) $@
	cd server && $(MAKE) $@
	cd navigation && $(MAKE) $@
	cd network && $(MAKE) $@
	cd helper && $(MAKE) $@
	cd thread && $(MAKE) $@
	cd xml && $(MAKE) $@

