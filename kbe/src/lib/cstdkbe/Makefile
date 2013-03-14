LIB =	cstdkbe

SRCS =				\
	blowfish		\
	cstdkbe			\
	tasks			\
	timer			\
	timestamp		\
	strutil			\
	kbeversion		\
	kbekey			\
	md5			\
	rsa			\
	memorystream

ifndef KBE_ROOT
export KBE_ROOT := $(subst /kbe/src/lib/$(LIB),,$(CURDIR))
endif

include $(KBE_ROOT)/kbe/src/build/common.mak