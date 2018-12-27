
all:: config.status

ifeq ($(KBE_CONFIG), Release)
	CONFIG_ARGS = --enable-optimizations --with-openssl=$(KBE_ROOT)/kbe/src/lib/dependencies/openssl
else
	CONFIG_ARGS = --with-openssl=$(KBE_ROOT)/kbe/src/lib/dependencies/openssl
endif

config.status:
	@chmod -R 755 .
	export KBE_ROOT=$(KBE_ROOT)
	$(SHELL) configure $(CONFIG_ARGS)
	@make

.PHONY: clean

clean::
	echo "clean"
