lib_LTLIBRARIES += libbson-1.0.la

libbson_1_0_la_SOURCES =
libbson_1_0_la_LIBADD = \
	libbson.la \
	libyajl.la \
	-lm
libbson_1_0_la_LDFLAGS = \
	$(OPTIMIZE_LDFLAGS) \
	$(COVERAGE_LDFLAGS) \
	-no-undefined \
	-rpath $(libdir)

if HAVE_LD_VERSION_SCRIPT
libbson_1_0_la_LDFLAGS += -Wl,--version-script=$(srcdir)/build/autotools/versions.ldscript
else
libbson_1_0_la_LDFLAGS += -export-symbols-regex "^(bson_|bcon_)"
endif

include src/bson/Makefile.am
include src/yajl/Makefile.am

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = $(top_builddir)/src/libbson-1.0.pc

EXTRA_DIST += \
	src/libbson.symbols \
	build/autotools/versions.ldscript \
	src/libbson-1.0.pc.in
