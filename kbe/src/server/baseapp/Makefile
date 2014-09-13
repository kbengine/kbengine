BIN  = baseapp
SRCS =						\
	archiver				\
	base					\
	baseapp					\
	baseapp_interface			\
	backuper				\
	base_messages_forward_handler		\
	data_download				\
	data_downloads				\
	base_remotemethod			\
	forward_message_over_handler		\
	initprogress_handler			\
	restore_entity_handler			\
	sync_entitystreamtemplate_handler	\
	main					\
	proxy					\
	proxy_forwarder				\
	py_file_descriptor			\
	profile

ASMS =

MY_LIBS =		\
	server		\
	entitydef	\
	pyscript	\
	network		\
	thread
	

BUILD_TIME_FILE = main
USE_PYTHON = 1
USE_G3DMATH=1
USE_OPENSSL = 1

ifndef NO_USE_LOG4CXX
	NO_USE_LOG4CXX = 0
	CPPFLAGS += -DLOG4CXX_STATIC
endif

CPPFLAGS += -DKBE_BASEAPP

#HAS_PCH = 1
USE_JWSMTP = 1

ifndef KBE_ROOT
export KBE_ROOT := $(subst /kbe/src/server/$(BIN),,$(CURDIR))
endif

include $(KBE_ROOT)/kbe/src/build/common.mak

