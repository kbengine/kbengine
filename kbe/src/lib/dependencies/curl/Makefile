ifndef KBE_ROOT
export KBE_ROOT := $(subst /kbe/src/lib/dependencies/curl,,$(CURDIR))
endif

all:: config.status

CONFIG_ARGS =

config.status:
	@chmod -R 755 .
	./buildconf
	./configure --with-ssl=$(KBE_ROOT)/kbe/src/lib/dependencies/openssl --disable-shared --enable-static --disable-ldap --disable-sspi --without-librtmp --disable-ftp --disable-file --disable-dict --disable-telnet --disable-tftp --disable-rtsp --disable-pop3 --disable-imap --disable-smtp --disable-gopher --disable-smb --without-libidn
	@cp -f "kbe/Makefile" "src" 
	@make

.PHONY: clean

clean::
	echo "clean"
