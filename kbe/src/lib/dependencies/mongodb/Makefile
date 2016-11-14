all:: mongodb

mongodb:
	@find ./ -name "aclocal.m4" |xargs touch 
	@find ./ -name "configure" |xargs touch
	@find ./ -name "Makefile.in" |xargs touch
	@chmod -R 755 .
	@rm -rf install
	@mkdir install
	./configure --prefix=$(CURDIR)/install --enable-shared=no --enable-static=yes
	@make
	@make install
	@cp ./install/lib/libbson-1.0.a  ../../../libs/libbson-1.0.a
	@cp ./install/lib/libmongoc-1.0.a  ../../../libs/libmongoc-1.0.a
	
.PHONY: clean

clean::
	echo "clean"
