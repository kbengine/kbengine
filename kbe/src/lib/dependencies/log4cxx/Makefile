
all:: config.status

CONFIG_ARGS = --with-apr=../apr --with-apr-util=../apr-util

config.status:  configure
	@chmod -R 755 .
	$(SHELL) configure $(CONFIG_ARGS)
	@make
	@if ! test -e ../../../libs/liblog4cxx.a; then \cp -f "src/main/cpp/.libs/liblog4cxx.a" "../../../libs"; fi

.PHONY: clean

clean::
	echo "clean"

