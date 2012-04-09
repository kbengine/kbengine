
ifndef KBE_ROOT
export KBE_ROOT := $(subst /kbe/src/lib,,$(CURDIR))
endif

ifndef KBE_CONFIG
	KBE_CONFIG=Hybrid
	ifeq ($(shell uname -m),x86_64)
		 KBE_CONFIG=Hybrid64
	endif
endif

ifneq (,$(findstring 64, $(KBE_CONFIG)))
	OPENSSL_CONFIG+="x86_64=1"
else
	OPENSSL_CONFIG+=
endif

# This directory must match the directory in src/build/common.mak
LIBDIR=$(KBE_ROOT)/kbe/src/lib/bin

all install clean full:
	test -d $(LIBDIR) || mkdir $(LIBDIR)
	cd cstdkbe && $(MAKE) $@
	cd entitydef && $(MAKE) $@
	cd math && $(MAKE) $@
	cd pyscript && $(MAKE) $@
	cd server && $(MAKE) $@
	cd helper && $(MAKE) $@
	cd thread && $(MAKE) $@
	cd xmlplus && $(MAKE) $@
	cd third_party/g3dlite && $(MAKE) $@
	cd third_party/tinyxml && $(MAKE) $@

UNITTEST_LIBS = cstdkbe \
				entitydef \
				math \
				pyscript \
				helper \
				thread \
				network \
				thread	\

unit_tests: unit_tests_build unit_tests_run

unit_tests_build:
	# Build the unit tests
	@for i in $(UNITTEST_LIBS); do		\
        $(MAKE) -C $$i/unit_test;		\
		if [ $$? != 0 ]; then			\
			exit 1;						\
		fi								\
     done

unit_tests_run:
	# Run the unit tests
	@for i in $(UNITTEST_LIBS); do			\
        echo "Running unit test:" $$i ;		\
        $(MAKE) -s -C $$i/unit_test run;	\
		if [ $$? != 0 ]; then				\
			exit 1;							\
		fi									\
     done


unit_tests_clean:
	$(MAKE) -C unit_test_lib clean
	# Clean the unit tests
	@for i in $(UNITTEST_LIBS); do			\
        $(MAKE) -C $$i/unit_test clean;		\
		if [ $$? != 0 ]; then				\
			exit 1;							\
		fi									\
     done

version kbeversion kbe_version:
	@$(MAKE) -s -C server $@
