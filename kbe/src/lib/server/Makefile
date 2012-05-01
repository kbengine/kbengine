LIB =	server

SRCS =				\
	engine_componentmgr	\
	entity_app		\
	script_timers		\
	serverapp		\
	serverconfig		\
	serverinfos		\
	signal_handler

ifndef KBE_ROOT
export KBE_ROOT := $(subst /kbe/src/lib/$(LIB),,$(CURDIR))
endif

USE_PYTHON=1
include $(KBE_ROOT)/kbe/src/build/common.mak