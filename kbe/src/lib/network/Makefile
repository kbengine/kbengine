LIB =	network

SRCS =				\
	event_dispatcher	\
	event_poller		\
	socket			\
	address

ifndef KBE_ROOT
export KBE_ROOT := $(subst /kbe/src/lib/$(LIB),,$(CURDIR))
endif

include $(KBE_ROOT)/kbe/src/build/common.mak