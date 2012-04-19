LIB =	server

SRCS =				\
	entity_app		\
	serverapp		\
	serverconfig		\
	serverinfos

ifndef KBE_ROOT
export KBE_ROOT := $(subst /kbe/src/lib/$(LIB),,$(CURDIR))
endif

USE_PYTHON=1
include $(KBE_ROOT)/kbe/src/build/common.mak