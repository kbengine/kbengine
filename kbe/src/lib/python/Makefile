
all:: config.status

CONFIG_ARGS = --with-openssl=$(KBE_ROOT)/kbe/src/lib/dependencies/openssl

ifeq ($(python-optimizations), 1)
	echo "=>python-optimizations=1"
	CONFIG_ARGS += --enable-optimizations 
else
	echo "=>python-optimizations=0"
endif

config.status:
	@chmod -R 755 .
	export KBE_ROOT=$(KBE_ROOT)
	$(SHELL) configure $(CONFIG_ARGS)
	@make

.PHONY: clean

clean::
	echo "clean"
