BIN  = dbmgr
SRCS =						\
	billinghandler				\
	dbmgr					\
	dbmgr_interface				\
	dbtasks					\
	db_threadpool				\
	buffered_dbtasks			\
	main					\
	sync_app_datas_handler

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
USE_G3DMATH = 1
USE_MYSQL = 1
USE_OPENSSL = 1


ifndef NO_USE_LOG4CXX
	NO_USE_LOG4CXX = 0
	CPPFLAGS += -DLOG4CXX_STATIC
endif

#HAS_PCH = 1
CPPFLAGS += -DKBE_DBMGR

ifndef KBE_ROOT
export KBE_ROOT := $(subst /kbe/src/server/$(BIN),,$(CURDIR))
endif

#KBE_INCLUDES += -I $(KBE_ROOT)/kbe/src/lib/dependencies/mysql/include

include $(KBE_ROOT)/kbe/src/build/common.mak

