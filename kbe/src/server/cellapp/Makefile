BIN  = cellapp
SRCS =						\
	entities				\
	entity					\
	main

ASMS =

MY_LIBS =		\
	entitydef	\
	pyscript	\
	server		\
	network
	

BUILD_TIME_FILE = main
USE_PYTHON = 1
USE_G3DMATH=1
	
ifndef USE_OPENSSL
	USE_OPENSSL = 0
endif

#HAS_PCH = 1

ifndef KBE_ROOT
export KBE_ROOT := $(subst /kbe/src/server/$(BIN),,$(CURDIR))
endif

include $(KBE_ROOT)/kbe/src/build/common.mak

