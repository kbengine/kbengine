BIN  = logger
SRCS =						\
	logger					\
	logger_interface		\
	logwatcher				\
	profile					\
	main

ASMS =

MY_LIBS =		\
	server		\
	network		\
	pyscript	\
	thread
	

BUILD_TIME_FILE = main

USE_G3DMATH = 1
USE_OPENSSL = 1
USE_PYTHON = 1

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
