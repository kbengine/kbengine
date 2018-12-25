
all:: config.status

CONFIG_ARGS =

config.status:
	@chmod -R 755 .
	./configure --with-openssl=$(KBE_ROOT)/kbe/src/lib/dependencies/openssl
	@make

.PHONY: clean

clean::
	echo "clean"
