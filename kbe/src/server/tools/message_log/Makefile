BIN  = messagelog
SRCS =						\
	messagelog				\
	messagelog_interface			\
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
	CPPFLAGS += -DLOG4CXX_STATIC
endif

#HAS_PCH = 1
CPPFLAGS += -DKBE_MESSAGELOG

ifndef KBE_ROOT
export KBE_ROOT := $(subst /kbe/src/server/tools/$(BIN),,$(CURDIR))
endif

include $(KBE_ROOT)/kbe/src/build/common.mak
