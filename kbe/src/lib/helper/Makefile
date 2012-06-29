LIB =	helper

SRCS =				\
	debug_helper		

ifndef KBE_ROOT
export KBE_ROOT := $(subst /kbe/src/lib/$(LIB),,$(CURDIR))
endif

ifndef NO_USE_LOG4CXX
	NO_USE_LOG4CXX = 0
endif

include $(KBE_ROOT)/kbe/src/build/common.mak