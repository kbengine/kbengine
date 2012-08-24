LIB =	dbmgr_lib

SRCS =				\
	db_interface		\
	entity_table

USE_PYTHON = 1
USE_G3DMATH=1

ifndef KBE_ROOT
export KBE_ROOT := $(subst /kbe/src/lib/$(LIB),,$(CURDIR))
endif

include $(KBE_ROOT)/kbe/src/build/common.mak
