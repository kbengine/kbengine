check_PROGRAMS = test_uuid
test_uuid_SOURCES = test_uuid.c
test_uuid_LDADD = libuuid.la $(SOCKET_LIBS)
test_uuid_CFLAGS = -I$(top_srcdir)

EXTRA_DIST = uuid.pc.in
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = uuid.pc

# includes
uuidincdir = $(includedir)/uuid
uuidinc_HEADERS = uuid.h

lib_LTLIBRARIES = libuuid.la

libuuid_la_SOURCES = \
	clear.c \
	compare.c \
	copy.c \
	gen_uuid.c \
	isnull.c \
	pack.c \
	parse.c \
	unpack.c \
	unparse.c \
	uuidd.h \
	uuidd.h \
	uuidP.h \
	uuid_time.c \
	randutils.c \
	$(uuidinc_HEADERS)

noinst_HEADERS = \
	all-io.h \
	c.h \
	randutils.h

libuuid_la_LIBADD  = $(SOCKET_LIBS)
libuuid_la_CFLAGS  = -I$(top_srcdir)
libuuid_la_LDFLAGS = -version-info $(LIBUUID_VERSION_INFO)
