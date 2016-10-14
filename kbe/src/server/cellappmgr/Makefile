BIN  = cellappmgr
SRCS =						\
	cell					\
	cells					\
	cellapp					\
	cellappmgr				\
	cellappmgr_interface			\
	main					\
	space					\
	spaces					\
	space_viewer

ASMS =

MY_LIBS =		\
	server		\
	network	\
	thread
	

BUILD_TIME_FILE = main
USE_PYTHON = 1
USE_G3DMATH = 1
USE_OPENSSL = 1

ifndef NO_USE_LOG4CXX
	NO_USE_LOG4CXX = 0
	CPPFLAGS += -DLOG4CXX_STATIC
endif

#HAS_PCH = 1
CPPFLAGS += -DKBE_CELLAPPMGR

ifndef KBE_ROOT
export KBE_ROOT := $(subst /kbe/src/server/$(BIN),,$(CURDIR))
endif

include $(KBE_ROOT)/kbe/src/build/common.mak

