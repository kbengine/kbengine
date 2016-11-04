
all:: config.status

CONFIG_ARGS =

config.status:  configure
	@chmod -R 755 .
	$(SHELL) configure $(CONFIG_ARGS)
	@make

.PHONY: clean

clean::
	echo "clean"
