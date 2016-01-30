
all:: config.status

config.status:  config
	@chmod -R 755 .
	$(SHELL) config -fPIC
	@make
	@if ! test -e ../../../libs/libssl.a; then \cp -f "libssl.a" "../../../libs"; fi
	@if ! test -e ../../../libs/libcrypto.a; then \cp -f "libcrypto.a" "../../../libs"; fi

.PHONY: clean

clean::
	echo "clean"

