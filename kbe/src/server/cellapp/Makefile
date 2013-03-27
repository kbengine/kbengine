BIN  = cellapp
SRCS =						\
	all_clients				\
	cell					\
	cells					\
	cellapp					\
	cellapp_interface			\
	clients_remote_entity_method		\
	forward_message_over_handler		\
	entity					\
	entity_remotemethod			\
	entity_range_node			\
	history_event				\
	main					\
	space					\
	spaces					\
	updatable				\
	updatables				\
	witness					\
	profile					\
	range_node				\
	range_list

ASMS =

MY_LIBS =		\
	entitydef	\
	server		\
	pyscript	\
	network		\
	html5		\
	thread
	

BUILD_TIME_FILE = main
USE_PYTHON = 1
USE_G3DMATH=1
USE_OPENSSL = 1

ifndef NO_USE_LOG4CXX
	NO_USE_LOG4CXX = 0
	CPPFLAGS += -DLOG4CXX_STATIC
endif

#HAS_PCH = 1
CPPFLAGS += -DKBE_CELLAPP

ifndef KBE_ROOT
export KBE_ROOT := $(subst /kbe/src/server/$(BIN),,$(CURDIR))
endif

include $(KBE_ROOT)/kbe/src/build/common.mak

