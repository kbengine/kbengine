BIN  = bots
SRCS =						\
	bots					\
	bots_interface				\
	main

ASMS =

MY_LIBS =		\
	server		\
	entitydef	\
	pyscript	\
	network		\
	thread
	

BUILD_TIME_FILE = main
USE_PYTHON = 1
USE_G3DMATH=1
	
ifndef USE_OPENSSL
	USE_OPENSSL = 0
endif

ifndef NO_USE_LOG4CXX
	NO_USE_LOG4CXX = 0
endif

#HAS_PCH = 1
CPPFLAGS += -DKBE_BOTS

ifndef KBE_ROOT
export KBE_ROOT := $(subst /kbe/src/server/tools/$(BIN),,$(CURDIR))
endif

include $(KBE_ROOT)/kbe/src/build/common.mak

