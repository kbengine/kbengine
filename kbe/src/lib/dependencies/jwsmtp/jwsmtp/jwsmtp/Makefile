LIB =	jwsmtp

SRCS =				\
	base64			\
	compat			\
	mailer

ifndef KBE_ROOT
export KBE_ROOT := $(subst /kbe/src/lib/dependencies/jwsmtp/jwsmtp/$(LIB),,$(CURDIR))
endif

include $(KBE_ROOT)/kbe/src/build/common.mak