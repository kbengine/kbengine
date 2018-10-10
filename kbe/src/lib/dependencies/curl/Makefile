
all:: config.status

CONFIG_ARGS =

config.status:
	@chmod -R 755 .
	./buildconf
	./configure --with-ssl=../openssl --disable-shared
	@make

.PHONY: clean

clean::
	echo "clean"
