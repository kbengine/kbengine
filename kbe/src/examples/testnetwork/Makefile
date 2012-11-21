BIN  = testnetwork
SRCS =				\
	main				

ASMS =

MY_LIBS =		\
	cstdkbe		\
	network		\
	html5		\
	server		\
	thread
	

BUILD_TIME_FILE = main
USE_PYTHON = 1
USE_G3DMATH=1
	
ifndef USE_OPENSSL
	USE_OPENSSL = 0
endif

ifndef NO_USE_LOG4CXX
	NO_USE_LOG4CXX = 0
	CPPFLAGS += -DLOG4CXX_STATIC
endif

#HAS_PCH = 1

ifndef KBE_ROOT
export KBE_ROOT := $(subst /kbe/src/examples/$(BIN),,$(CURDIR))
endif

include $(KBE_ROOT)/kbe/src/build/common.mak

