LIB =	pyscript

SRCS =				\
	map			\
	math			\
	pickler			\
	script			\
	scriptobject		\
	scriptstdouterr		\
	scriptStdouterrhook	\
	sequence		\
	vector2			\
	vector3			\
	vector4

ifndef KBE_ROOT
export KBE_ROOT := $(subst /kbe/src/lib/$(LIB),,$(CURDIR))
endif

USE_PYTHON=1
USE_G3DMATH=1
include $(KBE_ROOT)/kbe/src/build/common.mak