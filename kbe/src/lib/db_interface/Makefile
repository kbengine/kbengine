LIB =	db_interface

SRCS =				\
	db_interface		\
	db_tasks		\
	db_threadpool		\
	entity_table		\
	kbe_tables

USE_PYTHON = 1
USE_G3DMATH=1
USE_OPENSSL = 1

ifndef KBE_ROOT
export KBE_ROOT := $(subst /kbe/src/lib/$(LIB),,$(CURDIR))
endif

include $(KBE_ROOT)/kbe/src/build/common.mak
KBE_INCLUDES += -I $(KBE_ROOT)/kbe/src/lib/dependencies/hiredis