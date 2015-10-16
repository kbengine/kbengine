LIB =	db_redis

SRCS =					\
	common				\
	db_interface_redis

ifndef KBE_ROOT
export KBE_ROOT := $(subst /kbe/src/lib/$(LIB),,$(CURDIR))
endif

include $(KBE_ROOT)/kbe/src/build/common.mak
KBE_INCLUDES += -I $(KBE_ROOT)/kbe/src/lib/dependencies/hiredis