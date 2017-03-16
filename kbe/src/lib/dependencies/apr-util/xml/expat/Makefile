
all:: config.status

CONFIG_ARGS =

config.status:  configure
	@chmod -R 755 .
	$(SHELL) configure $(CONFIG_ARGS)
	@make
	@if ! test -e ../../../../../libs/libexpat.a; then \cp -f ".libs/libexpat.a" "../../../../../libs"; fi

.PHONY: clean

clean::
	echo "clean"
