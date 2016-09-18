all:: mongodb

mongodb:
	@find ./ -name "aclocal.m4" |xargs touch 
	@find ./ -name "configure" |xargs touch
	@find ./ -name "Makefile.in" |xargs touch
	@chmod -R 755 .
	@rm -rf install
	@mkdir install
	./configure --prefix=$(CURDIR)/install
	@make
	@make install
	@cp ./install/lib/libbson-1.0.so  ../../../libs/libbson-1.0.so
	@cp ./install/lib/libmongoc-1.0.so  ../../../libs/libmongoc-1.0.so
	
.PHONY: clean

clean::
	echo "clean"
