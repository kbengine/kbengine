BIN  = cellapp
SRCS =						\
	all_clients				\
	view_trigger			\
	cell					\
	cells					\
	cellapp					\
	cellapp_interface		\
	clients_remote_entity_method		\
	controller				\
	controllers				\
	client_entity			\
	client_entity_method	\
	forward_message_over_handler		\
	entity					\
	entityref				\
	entity_remotemethod		\
	entity_coordinate_node	\
	entity_component		\
	ghost_manager			\
	history_event			\
	initprogress_handler	\
	loadnavmesh_threadtasks	\
	main					\
	space					\
	spacememory				\
	spacememorys			\
	space_viewer			\
	move_controller			\
	moveto_entity_handler	\
	moveto_point_handler	\
	navigate_handler		\
	profile					\
	proximity_controller	\
	coordinate_node			\
	coordinate_system		\
	rotator_handler			\
	range_trigger			\
	range_trigger_node		\
	real_entity_method		\
	trap_trigger			\
	turn_controller			\
	updatable				\
	updatables				\
	watch_obj_pools			\
	witness					\
	witnessed_timeout_handler

ASMS =

MY_LIBS =		\
	entitydef	\
	server		\
	pyscript	\
	network		\
	navigation	\
	thread
	

BUILD_TIME_FILE = main
USE_PYTHON = 1
USE_G3DMATH = 1
USE_OPENSSL = 1
USE_TMXPARSER = 1

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

