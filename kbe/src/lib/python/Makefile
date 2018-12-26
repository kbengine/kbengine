
all:: config.status

CONFIG_ARGS =

config.status:
	@chmod -R 755 .
	export KBE_ROOT=$(KBE_ROOT)
	./configure --with-openssl=$(KBE_ROOT)/kbe/src/lib/dependencies/openssl
	@make

.PHONY: clean

clean::
	echo "clean"
