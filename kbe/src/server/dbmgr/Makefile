BIN  = dbmgr
SRCS =						\
	dbmgr					\
	dbmgr_interface				\
	main

ASMS =

MY_LIBS =		\
	entitydef	\
	server		\
	pyscript	\
	network		\
	thread		\
	dbmgr_lib	\
	db_mysql
	

BUILD_TIME_FILE = main
USE_PYTHON = 1
USE_G3DMATH=1
USE_MYSQL = 1

ifndef USE_OPENSSL
	USE_OPENSSL = 0
endif

ifndef NO_USE_LOG4CXX
	NO_USE_LOG4CXX = 0
endif

#HAS_PCH = 1
CPPFLAGS += -DKBE_DBMGR

ifndef KBE_ROOT
export KBE_ROOT := $(subst /kbe/src/server/$(BIN),,$(CURDIR))
endif

#KBE_INCLUDES += -I $(KBE_ROOT)/kbe/src/lib/third_party/mysql/include

include $(KBE_ROOT)/kbe/src/build/common.mak

