BIN  = bots
SRCS =						\
	bots					\
	bots_interface			\
	clientobject			\
	create_and_login_handler\
	profile					\
	main					\
	pybots					\
	kcp_packet_receiver_ex	\
	kcp_packet_sender_ex	\
	tcp_packet_receiver_ex	\
	tcp_packet_sender_ex

ASMS =

MY_LIBS =		\
	client_lib	\
	entitydef	\
	server		\
	pyscript	\
	network		\
	thread
	
	

BUILD_TIME_FILE = main
USE_PYTHON = 1
USE_G3DMATH = 1
USE_OPENSSL = 1

ifndef NO_USE_LOG4CXX
	NO_USE_LOG4CXX = 0
	CPPFLAGS += -DLOG4CXX_STATIC
endif

#HAS_PCH = 1
CPPFLAGS += -DKBE_BOTS

ifndef KBE_ROOT
export KBE_ROOT := $(subst /kbe/src/server/tools/$(BIN),,$(CURDIR))
endif

include $(KBE_ROOT)/kbe/src/build/common.mak

