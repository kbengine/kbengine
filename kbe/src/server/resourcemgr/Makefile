BIN  = resourcemgr
SRCS =						\
	resourcemgr				\
	resourcemgr_interface			\
	main

ASMS =

MY_LIBS =		\
	server		\
	network		\
	thread
	

BUILD_TIME_FILE = main

ifndef USE_OPENSSL
	USE_OPENSSL = 0
endif

ifndef NO_USE_LOG4CXX
	NO_USE_LOG4CXX = 0
endif

#HAS_PCH = 1
CPPFLAGS += -DKBE_RESOURCEMGR

ifndef KBE_ROOT
export KBE_ROOT := $(subst /kbe/src/server/$(BIN),,$(CURDIR))
endif

include $(KBE_ROOT)/kbe/src/build/common.mak
