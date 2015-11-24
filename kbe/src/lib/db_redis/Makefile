LIB =	db_redis

SRCS =					\
	common				\
	db_exception			\
	db_transaction			\
	db_interface_redis		\
	entity_table_redis		\
	kbe_table_redis			\
	redis_watcher

USE_PYTHON = 1
USE_G3DMATH = 1

ifndef KBE_ROOT
export KBE_ROOT := $(subst /kbe/src/lib/$(LIB),,$(CURDIR))
endif

include $(KBE_ROOT)/kbe/src/build/common.mak
KBE_INCLUDES += -I $(KBE_ROOT)/kbe/src/lib/dependencies/hiredis