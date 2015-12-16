BIN  = logger
SRCS =						\
	logger					\
	logger_interface			\
	logwatcher				\
	main

ASMS =

MY_LIBS =		\
	server		\
	network		\
	thread
	

BUILD_TIME_FILE = main

USE_OPENSSL = 1

ifndef NO_USE_LOG4CXX
	NO_USE_LOG4CXX = 0
	CPPFLAGS += -DLOG4CXX_STATIC
endif

#HAS_PCH = 1
CPPFLAGS += -DKBE_LOGGER

ifndef KBE_ROOT
export KBE_ROOT := $(subst /kbe/src/server/tools/$(BIN),,$(CURDIR))
endif

include $(KBE_ROOT)/kbe/src/build/common.mak
