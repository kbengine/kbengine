
all:: config.status

CONFIG_ARGS = --with-apr=../apr

config.status:  configure
	@chmod -R 755 .
	$(SHELL) configure $(CONFIG_ARGS)
	@make

.PHONY: clean

clean::
	echo "clean"
