ifndef KBE_ROOT
export KBE_ROOT := $(subst /kbe/src,,$(CURDIR))
endif


all clean:
	$(MAKE) -C lib $@
	$(MAKE) -C server $@

ARCH=
ifeq ($(shell uname -m),x86_64)
	 ARCH=64
endif

all_configs:
	$(MAKE) KBE_CONFIG=Hybrid$(ARCH) all
	$(MAKE) KBE_CONFIG=Debug$(ARCH) all


all_configs_clean:
	$(MAKE) KBE_CONFIG=Hybrid$(ARCH) cleanall
	$(MAKE) KBE_CONFIG=Debug$(ARCH) cleanall


coverage:
	$(MAKE) KBE_CONFIG=Debug$(ARCH)_GCOV all


coverage_clean:
	$(MAKE) KBE_CONFIG=Debug$(ARCH)_GCOV cleanall


cleanall:
	$(MAKE) clean

