
all:: config.status

CONFIG_ARGS = --with-openssl=$(KBE_ROOT)/kbe/src/lib/dependencies/openssl

ifeq ($(pythonbuild), release)
	CONFIG_ARGS += --enable-optimizations 
else
endif

config.status:
	@chmod -R 755 .
	export KBE_ROOT=$(KBE_ROOT)
	$(SHELL) configure $(CONFIG_ARGS)
	@make

.PHONY: clean

clean::
	echo "clean"
