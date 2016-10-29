LIB =	db_mongodb

SRCS =					\
	common				\
	db_exception			\
	db_transaction			\
	db_interface_mongodb	\
	entity_table_mongodb	\
	kbe_table_mongodb
	
USE_PYTHON = 1
USE_G3DMATH = 1

ifndef KBE_ROOT
export KBE_ROOT := $(subst /kbe/src/lib/$(LIB),,$(CURDIR))
endif

include $(KBE_ROOT)/kbe/src/build/common.mak
KBE_INCLUDES += -I $(KBE_ROOT)/kbe/src/lib/dependencies/mongodb/src/mongoc
KBE_INCLUDES += -I $(KBE_ROOT)/kbe/src/lib/dependencies/mongodb/src/libbson/src/bson
KBE_INCLUDES += -I $(KBE_ROOT)/kbe/src/lib/dependencies/mongodb/src/libbson/src/yajl
