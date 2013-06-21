ifndef KBE_ROOT
export KBE_ROOT := $(subst /kbe/src/examples,,$(CURDIR))
endif

all clean realclean all_config install unit_tests unit_tests_build unit_tests_run unit_tests_clean::

# Check with and without the trailing slash
ifneq ($(CURDIR),$(KBE_ROOT)/kbe/src/examples)
ifneq ($(CURDIR),$(KBE_ROOT)kbe/src/examples)
	@echo 'Error: KBE_ROOT=$(KBE_ROOT)'
	@echo '       is not the root of $(CURDIR)'
	@echo 'Remove the KBE_ROOT environment variable.'
	@false
endif
endif

	#$(MAKE) -C ../lib $@
	#$(MAKE) -C testnetwork $@

ifdef KBE_CONFIG
	@echo completed $@ \(KBE_CONFIG = $(KBE_CONFIG)\)
else
	@echo completed $@
endif
	$(MAKE) done

done:
ifdef DO_NOT_BELL
else
	echo -n 
endif

server:
	$(MAKE) all
