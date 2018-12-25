
all:: config.status

CONFIG_ARGS =

config.status:
	@chmod -R 755 .
	./configure --enable-optimizations -l=$(KBE_ROOT)/kbe/src/libs/ -I$(KBE_ROOT)/kbe/src/lib/dependencies/openssl/include
	@make

.PHONY: clean

clean::
	echo "clean"
