all:: mongodb

mongodb:
	@chmod -R 755 .
	@rm -rf install
	@mkdir install
	./configure --prefix=$(CURDIR)/install
	@make
	@make install

.PHONY: clean

clean::
	echo "clean"
