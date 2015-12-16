
all:: config.status

CONFIG_ARGS =

config.status:  configure
	@chmod -R 755 .
	$(SHELL) configure $(CONFIG_ARGS)
	@make
	@if ! test -e ../../../libs/libapr-1.a; then \cp -f ".libs/libapr-1.a" "../../../libs"; fi

.PHONY: clean

clean::
	echo "clean"
