LIB =	sigar

CSRCS =				\
	sigar			\
	sigar_cache		\
	sigar_fileinfo		\
	sigar_format		\
	sigar_getline		\
	sigar_ptql		\
	sigar_signal		\
	sigar_util		\
	linux_sigar

ifndef KBE_ROOT
export KBE_ROOT := $(subst /kbe/src/lib/dependencies/$(LIB)/linux,,$(CURDIR))
endif

include $(KBE_ROOT)/kbe/src/build/common.mak