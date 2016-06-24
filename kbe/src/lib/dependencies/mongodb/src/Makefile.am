pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = \
	$(top_builddir)/src/libmongoc-1.0.pc \
	$(top_builddir)/src/libmongoc-priv.pc
if ENABLE_SSL
pkgconfig_DATA += $(top_builddir)/src/libmongoc-ssl-1.0.pc
endif


lib_LTLIBRARIES += libmongoc-priv.la
lib_LTLIBRARIES += libmongoc-1.0.la


MONGOC_CPPFLAGS_SHARED = \
	-DMONGOC_COMPILATION \
	$(LIBC_FEATURES) \
	$(OPTIMIZE_CFLAGS) \
	$(COVERAGE_CFLAGS) \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/mongoc \
	-I$(top_builddir)/src/mongoc \
	$(BSON_CFLAGS) \
	$(PTHREAD_CFLAGS) \
	$(SSL_CFLAGS) \
	$(SASL_CFLAGS)
if OS_SOLARIS
MONGOC_CPPFLAGS_SHARED += -D_REENTRANT
endif
if !ENABLE_DEBUG
MONGOC_CPPFLAGS_SHARED += \
	-DBSON_DISABLE_ASSERT \
	-DBSON_DISABLE_CHECKS
endif
libmongoc_priv_la_CPPFLAGS = $(MONGOC_CPPFLAGS_SHARED)
libmongoc_1_0_la_CPPFLAGS = $(MONGOC_CPPFLAGS_SHARED)


MONGOC_LDFLAGS_SHARED = $(OPTIMIZE_LDFLAGS) $(COVERAGE_LDFLAGS)
libmongoc_priv_la_LDFLAGS = \
	$(MONGOC_LDFLAGS_SHARED) \
	-no-undefined

libmongoc_1_0_la_LDFLAGS = \
	$(MONGOC_LDFLAGS_SHARED) \
	-no-undefined \
	-rpath $(libdir)

if HAVE_LD_VERSION_SCRIPT
libmongoc_1_0_la_LDFLAGS += -Wl,--version-script=$(srcdir)/build/autotools/versions.ldscript
else
libmongoc_1_0_la_LDFLAGS += -export-symbols-regex "^mongoc_"
endif

MONGOC_LIBADD_SHARED = \
	$(BSON_LIBS) \
	$(PTHREAD_LIBS) \
	$(SHM_LIB) \
	$(SSL_LIBS) \
	$(SASL_LIBS)
if OS_WIN32
MONGOC_LIBADD_SHARED += -lws2_32
endif
libmongoc_priv_la_LIBADD = $(MONGOC_LIBADD_SHARED)
libmongoc_1_0_la_LIBADD = $(MONGOC_LIBADD_SHARED)


# Sources defined in src/mongoc/Makefile.am
MONGOC_SOURCES_SHARED = 
libmongoc_1_0_la_SOURCES = $(MONGOC_SOURCES_SHARED)
libmongoc_priv_la_SOURCES = $(MONGOC_SOURCES_SHARED)


include src/mongoc/Makefile.am
include src/tools/Makefile.am

EXTRA_DIST += \
	src/libmongoc.symbols \
	build/autotools/versions.ldscript \
	src/libmongoc-1.0.pc.in \
	src/libmongoc-ssl-1.0.pc.in
