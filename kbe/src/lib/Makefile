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
	cd dependencies/hiredis && $(MAKE) $@
	cd dependencies/zip && $(MAKE) $@
	cd dependencies/g3dlite && $(MAKE) $@
	cd dependencies/tinyxml && $(MAKE) $@
	cd dependencies/sigar/linux && $(MAKE) $@
	cd dependencies/tmxparser && $(MAKE) $@
	cd dependencies/jwsmtp/jwsmtp/jwsmtp && $(MAKE) $@
	cd dependencies/jemalloc && $(MAKE) $@
	cd dependencies/apr && $(MAKE) $@
	cd dependencies/expat && $(MAKE) $@
	cd dependencies/apr-util && $(MAKE) $@
	cd dependencies/log4cxx && $(MAKE) $@
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

