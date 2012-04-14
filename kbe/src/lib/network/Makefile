LIB =	network

SRCS =				\
	address			\
	bundle			\
	channel			\
	error_reporter		\
	event_dispatcher	\
	event_poller		\
	network_interface	\
	packet			\
	packet_filter		\
	packet_receiver		\
	socket			\
	

ifndef KBE_ROOT
export KBE_ROOT := $(subst /kbe/src/lib/$(LIB),,$(CURDIR))
endif

include $(KBE_ROOT)/kbe/src/build/common.mak