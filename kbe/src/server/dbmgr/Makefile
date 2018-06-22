BIN  = dbmgr
SRCS =						\
	buffered_dbtasks		\
	dbmgr					\
	dbmgr_interface			\
	dbtasks					\
	entity_component		\
	interfaces_handler		\
	main					\
	profile					\
	sync_app_datas_handler	\
	update_dblog_handler

ASMS =

MY_LIBS =		\
	entitydef	\
	server		\
	pyscript	\
	network		\
	thread		\
	db_interface\
	db_mysql	\
	db_redis

BUILD_TIME_FILE = main
USE_PYTHON = 1
USE_G3DMATH = 1
USE_MYSQL = 1
USE_REDIS = 1
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

