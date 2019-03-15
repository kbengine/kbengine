
all:: config.status

CONFIG_ARGS =

config.status:
	@chmod -R 755 .
	./configure
	@make

.PHONY: clean

clean::
	echo "clean"
