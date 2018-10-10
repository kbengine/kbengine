
all:: config.status

CONFIG_ARGS =

config.status:  autogen
	@chmod -R 755 .
	./autogen.sh
	@make

.PHONY: clean

clean::
	echo "clean"
