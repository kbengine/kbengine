LIB =	pyscript

SRCS =					\
	copy				\
	map					\
	math				\
	pickler				\
	py_compression		\
	py_gc				\
	pyurl				\
	py_platform			\
	pystruct			\
	pyprofile			\
	pyprofile_handler	\
	py_memorystream		\
	pywatcher			\
	script				\
	scriptobject		\
	scriptstdout		\
	scriptstderr		\
	scriptstdouterr		\
	scriptstdouterrhook	\
	sequence			\
	vector2				\
	vector3				\
	vector4

ifndef KBE_ROOT
export KBE_ROOT := $(subst /kbe/src/lib/$(LIB),,$(CURDIR))
endif

USE_PYTHON=1
USE_G3DMATH=1
include $(KBE_ROOT)/kbe/src/build/common.mak