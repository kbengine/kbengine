LIB =	client_lib

SRCS =				\
	client_interface	\
	clientapp		\
	clientobjectbase	\
	entity			\
	entity_aspect		\
	event			\
	profile

ifndef KBE_ROOT
export KBE_ROOT := $(subst /kbe/src/lib/$(LIB),,$(CURDIR))
endif

USE_PYTHON=1

include $(KBE_ROOT)/kbe/src/build/common.mak