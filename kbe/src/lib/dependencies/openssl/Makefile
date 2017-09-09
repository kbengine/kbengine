
all:: config.status

config.status:  config
	@chmod -R 755 .
	$(SHELL) config -fPIC
	@make

.PHONY: clean

clean::
	echo "clean"

