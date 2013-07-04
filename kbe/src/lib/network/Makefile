LIB =	network

SRCS =				\
	address			\
	bundle_broadcast	\
	bundle			\
	channel			\
	common			\
	delayed_channels	\
	error_reporter		\
	event_dispatcher	\
	event_poller		\
	encryption_filter	\
	fixed_messages		\
	html5_packet_reader	\
	html5_packet_filter	\
	interface_defs		\
	message_handler		\
	mercurystats		\
	listener_receiver	\
	network_interface	\
	packet_filter		\
	packet_reader		\
	packet_sender		\
	packet_receiver		\
	endpoint		\
	tcp_packet		\
	tcp_packet_receiver	\
	udp_packet		\
	udp_packet_receiver	\
	websocket_protocol	

USE_OPENSSL = 1

ifndef KBE_ROOT
export KBE_ROOT := $(subst /kbe/src/lib/$(LIB),,$(CURDIR))
endif

include $(KBE_ROOT)/kbe/src/build/common.mak