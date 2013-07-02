LIB =	html5

SRCS =				\
	sha1			\
	base64			\
	html5_packet_reader	\
	websocket_protocol	

ifndef KBE_ROOT
export KBE_ROOT := $(subst /kbe/src/lib/$(LIB),,$(CURDIR))
endif

include $(KBE_ROOT)/kbe/src/build/common.mak