
all:: config.status

CONFIG_ARGS = --with-apr=../apr

config.status:  configure
	@chmod -R 755 .
	$(SHELL) configure $(CONFIG_ARGS)
	@make
	@if ! test -e ../../../libs/libaprutil-1.a; then \cp -f ".libs/libaprutil-1.a" "../../../libs"; fi

.PHONY: clean

clean::
	echo "clean"
