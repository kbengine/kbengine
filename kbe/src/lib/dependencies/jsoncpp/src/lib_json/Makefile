LIB =	jsoncpp

SRCS =				\
	json_reader		\
	json_value		\
	json_writer

ifndef KBE_ROOT
export KBE_ROOT := $(subst /kbe/src/lib/dependencies/$(LIB),,$(CURDIR))
endif

include $(KBE_ROOT)/kbe/src/build/common.mak