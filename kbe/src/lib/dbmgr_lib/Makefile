LIB =	dbmgr_lib

SRCS =				\
	db_interface		\
	entity_table

ifndef KBE_ROOT
export KBE_ROOT := $(subst /kbe/src/lib/$(LIB),,$(CURDIR))
endif

include $(KBE_ROOT)/kbe/src/build/common.mak