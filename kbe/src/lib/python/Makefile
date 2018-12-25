
all:: config.status

CONFIG_ARGS =

config.status:
	@chmod -R 755 .
	./configure --enable-optimizations
	@make

.PHONY: clean

clean::
	echo "clean"
