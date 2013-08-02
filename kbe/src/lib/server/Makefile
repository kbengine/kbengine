LIB =	server

SRCS =				\
	common			\
	componentbridge		\
	components		\
	component_active_report_handler \
	entity_app		\
	eventhistory_stats	\
	forward_messagebuffer	\
	globaldata_client	\
	globaldata_server	\
	idallocate		\
	pendingLoginmgr		\
	profile_handler		\
	script_timers		\
	serverapp		\
	serverconfig		\
	serverinfos		\
	sendmail_threadtasks	\
	shutdowner		\
	signal_handler		\
	telnet_handler		\
	telnet_server

ifndef KBE_ROOT
export KBE_ROOT := $(subst /kbe/src/lib/$(LIB),,$(CURDIR))
endif

USE_PYTHON=1
USE_JWSMTP = 1

ifndef NO_USE_LOG4CXX
	NO_USE_LOG4CXX = 0
	CPPFLAGS += -DLOG4CXX_STATIC
endif

include $(KBE_ROOT)/kbe/src/build/common.mak