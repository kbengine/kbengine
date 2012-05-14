LIB =	network

SRCS =				\
	address			\
	broadcast_handler	\
	bundle			\
	channel			\
	delayed_channels	\
	error_reporter		\
	event_dispatcher	\
	event_poller		\
	interface_defs		\
	message_handler		\
	listener_receiver	\
	network_interface	\
	packet_filter		\
	packet_sender		\
	packet_receiver		\
	endpoint		\
	tcp_packet		\
	tcp_packet_receiver	\
	udp_packet		\
	udp_packet_receiver

ifndef KBE_ROOT
export KBE_ROOT := $(subst /kbe/src/lib/$(LIB),,$(CURDIR))
endif

include $(KBE_ROOT)/kbe/src/build/common.mak