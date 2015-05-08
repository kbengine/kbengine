LIB =	client_lib

SRCS =				\
	client_interface	\
	clientapp		\
	clientobjectbase	\
	config			\
	script_callbacks	\
	entity			\
	entity_aspect		\
	event			\
	moveto_point_handler	\
	profile

ifndef KBE_ROOT
export KBE_ROOT := $(subst /kbe/src/lib/$(LIB),,$(CURDIR))
endif

USE_PYTHON=1
USE_OPENSSL=1

ifndef NO_USE_LOG4CXX
	NO_USE_LOG4CXX = 0
	CPPFLAGS += -DLOG4CXX_STATIC
endif

include $(KBE_ROOT)/kbe/src/build/common.mak