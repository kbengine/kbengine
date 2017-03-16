ifndef KBE_ROOT
export KBE_ROOT := $(subst /kbe/src/server,,$(CURDIR))
endif

all clean realclean all_config install::

# Check with and without the trailing slash
ifneq ($(CURDIR),$(KBE_ROOT)/kbe/src/server)
ifneq ($(CURDIR),$(KBE_ROOT)kbe/src/server)
	@echo 'Error: KBE_ROOT=$(KBE_ROOT)'
	@echo '       is not the root of $(CURDIR)'
	@echo 'Remove the KBE_ROOT environment variable.'
	@false
endif
endif

	$(MAKE) -C ../lib $@
	$(MAKE) -C baseapp $@
	$(MAKE) -C baseappmgr $@
	$(MAKE) -C cellapp $@
	$(MAKE) -C cellappmgr $@
	$(MAKE) -C dbmgr $@
	$(MAKE) -C loginapp $@
	$(MAKE) -C machine $@
	$(MAKE) -C tools $@

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
