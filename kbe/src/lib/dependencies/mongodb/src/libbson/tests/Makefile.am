noinst_PROGRAMS += test-libbson
TEST_PROGS = test-libbson


test_libbson_SOURCES = \
	tests/TestSuite.c \
	tests/TestSuite.h \
	tests/test-libbson.c \
	tests/test-atomic.c \
	tests/test-bson.c \
	tests/test-endian.c \
	tests/test-clock.c \
	tests/test-error.c \
	tests/test-iso8601.c \
	tests/test-iter.c \
	tests/test-json.c \
	tests/test-oid.c \
	tests/test-reader.c \
	tests/test-string.c \
	tests/test-utf8.c \
	tests/test-value.c \
	tests/test-version.c \
	tests/test-writer.c \
	tests/test-bcon-basic.c \
	tests/test-bcon-extract.c

test_libbson_CPPFLAGS = \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/bson \
	-I$(top_builddir)/src/bson \
	$(PTHREAD_CFLAGS) \
	$(COVERAGE_CFLAGS) \
	-DBSON_COMPILATION \
	-DBINARY_DIR="\"$(top_srcdir)/tests/binary\"" \
	-DJSON_DIR="\"$(top_srcdir)/tests/json\""

test_libbson_LDFLAGS = $(COVERAGE_LDFLAGS)

test_libbson_LDADD = \
	$(PTHREAD_LIBS) \
	libbson-1.0.la \
	libbson.la


check: test


TEST_ARGS = 
if OS_DARWIN
TEST_ARGS += -f -p
endif


test: $(TEST_PROGS)
	@ for TEST_PROG in $(TEST_PROGS) ; do \
		$(LIBTOOL) --mode=execute $$VALGRIND ./$$TEST_PROG $(TEST_ARGS) -F test.log; \
	done
if CAN_ABI_CHECK
	@ $(srcdir)/tests/abicheck.sh "$(srcdir)/src/libbson.symbols"
endif

valgrind: $(TEST_PROGS)
	$(LIBTOOL) --mode=execute valgrind --leak-check=full ./test-libbson -f -p

DISTCLEANFILES += test.log


EXTRA_DIST += \
	tests/abicheck.sh \
	tests/bson-tests.h \
	tests/binary/binary_deprecated.bson \
	tests/binary/codewscope.bson \
	tests/binary/dollarquery.bson \
	tests/binary/dotkey.bson \
	tests/binary/dotquery.bson \
	tests/binary/eurokey.bson \
	tests/binary/overflow1.bson \
	tests/binary/overflow2.bson \
	tests/binary/overflow3.bson \
	tests/binary/overflow4.bson \
	tests/binary/readergrow.bson \
	tests/binary/stackoverflow.bson \
	tests/binary/stream.bson \
	tests/binary/stream_corrupt.bson \
	tests/binary/test10.bson \
	tests/binary/test11.bson \
	tests/binary/test12.bson \
	tests/binary/test13.bson \
	tests/binary/test14.bson \
	tests/binary/test15.bson \
	tests/binary/test16.bson \
	tests/binary/test17.bson \
	tests/binary/test18.bson \
	tests/binary/test19.bson \
	tests/binary/test1.bson \
	tests/binary/test20.bson \
	tests/binary/test21.bson \
	tests/binary/test22.bson \
	tests/binary/test23.bson \
	tests/binary/test24.bson \
	tests/binary/test25.bson \
	tests/binary/test26.bson \
	tests/binary/test27.bson \
	tests/binary/test28.bson \
	tests/binary/test29.bson \
	tests/binary/test2.bson \
	tests/binary/test30.bson \
	tests/binary/test31.bson \
	tests/binary/test32.bson \
	tests/binary/test33.bson \
	tests/binary/test34.bson \
	tests/binary/test35.bson \
	tests/binary/test36.bson \
	tests/binary/test37.bson \
	tests/binary/test38.bson \
	tests/binary/test39.bson \
	tests/binary/test3.bson \
	tests/binary/test4.bson \
	tests/binary/test40.bson \
	tests/binary/test41.bson \
	tests/binary/test42.bson \
	tests/binary/test43.bson \
	tests/binary/test44.bson \
	tests/binary/test45.bson \
	tests/binary/test46.bson \
	tests/binary/test47.bson \
	tests/binary/test48.bson \
	tests/binary/test49.bson \
	tests/binary/test5.bson \
	tests/binary/test50.bson \
	tests/binary/test51.bson \
	tests/binary/test52.bson \
	tests/binary/test53.bson \
	tests/binary/test54.bson \
	tests/binary/test55.bson \
	tests/binary/test56.bson \
	tests/binary/test57.bson \
	tests/binary/test6.bson \
	tests/binary/test7.bson \
	tests/binary/test8.bson \
	tests/binary/test9.bson \
	tests/binary/trailingnull.bson \
	tests/json/test.json
