LIB =	entitydef

SRCS =				\
	array			\
	common			\
	datatype		\
	datatypes		\
	entity_mailbox		\
	entitydef		\
	entitymailboxabstract	\
	fixeddict		\
	method			\
	property		\
	scriptdef_module

ifndef KBE_ROOT
export KBE_ROOT := $(subst /kbe/src/lib/$(LIB),,$(CURDIR))
endif

USE_PYTHON=1
USE_G3DMATH=1
include $(KBE_ROOT)/kbe/src/build/common.mak