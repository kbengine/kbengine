
all:: config.status

CONFIG_ARGS =

config.status:
	@chmod -R 755 .
	./configure
	@make
	@cp -f "$(KBE_ROOT)/kbe/src/lib/python/libpython3.7m.a" "$(KBE_ROOT)/kbe/src/libs/libpython.a" 

.PHONY: clean

clean::
	echo "clean"
