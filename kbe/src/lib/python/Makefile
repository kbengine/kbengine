
all:: config.status

CONFIG_ARGS = --with-openssl=$(KBE_ROOT)/kbe/src/lib/dependencies/openssl

ifeq ($(pythonbuild), release)
	CONFIG_ARGS += --enable-optimizations 
else
endif

config.status:
	@chmod -R 755 .
	export KBE_ROOT=$(KBE_ROOT)
	$(SHELL) configure $(CONFIG_ARGS)
	@make

.PHONY: clean

clean::
	echo "clean"

# KBE modifys:
# 1: PCbuild vcxprojs to KBE-vcxproj

# 2: Modules/zlibmodule.c added 
#	#if defined( __WIN32__ ) || defined( WIN32 ) || defined( _WIN32 )
#	#define ZLIB_WINAPI
#	#endif

# 3: configure
# OPENSSL_LDFLAGS="-L${KBE_ROOT}/kbe/src/libs"

# 4:Modules/Setup.dist

# 5:DEFS="BASEMODLIBS=$BASELIBS\\\\$NL$DEFS"
