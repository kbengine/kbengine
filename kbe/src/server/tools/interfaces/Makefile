BIN  = interfaces
SRCS =						\
	interfaces				\
	interfaces_tasks			\
	interfaces_interface			\
	orders					\
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
CPPFLAGS += -DKBE_INTERFACES

ifndef KBE_ROOT
export KBE_ROOT := $(subst /kbe/src/server/tools/$(BIN),,$(CURDIR))
endif

#KBE_INCLUDES += -I $(KBE_ROOT)/kbe/src/lib/dependencies/mysql/include

include $(KBE_ROOT)/kbe/src/build/common.mak

