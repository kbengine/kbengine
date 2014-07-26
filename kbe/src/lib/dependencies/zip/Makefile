LIB = zip

CSRCS =					\
	adler32				\
	compress			\
	crc32				\
	deflate				\
	gzio				\
	infback				\
	inffast				\
	inflate				\
	inftrees			\
	trees				\
	uncompr				\
	zutil

ifndef KBE_ROOT
export KBE_ROOT := $(subst /kbe/src/lib/dependencies/$(LIB),,$(CURDIR))
endif

include $(KBE_ROOT)/kbe/src/build/common.mak