# Thread Local Storage is broken in redhat8 (with a non-tls ld-linux.so) and
# Debian (up to and including Sarge).
# LDLINUX_TLS_IS_BROKEN = 0

ifndef KBE_CONFIG
	KBE_CONFIG=Hybrid
	ifeq ($(shell uname -m),x86_64)
		 KBE_CONFIG=Hybrid64
	endif
endif

ALLOW_32BIT_BUILD=1

ifeq (,$(findstring 64,$(KBE_CONFIG)))
 ifeq (0,$(ALLOW_32BIT_BUILD))

all::
	@echo "ERROR: 32 bit builds are not supported as of KBEngine ($(MF_CONFIG))"
	@false

 endif
endif

# This variable is used by src/lib/python/configure to determine whether to
# print out nasty error messages.
export BUILDING_KBENGINE=1


ifeq (,$(findstring $(KBE_CONFIG), Release Hybrid Debug Evaluation \
	Debug_SingleThreaded \
	Hybrid_SingleThreaded \
	Hybrid64 Hybrid64_SingleThreaded \
	Hybrid_SystemPython Hybrid64_SystemPython \
	Debug_SystemPython Debug64_SystemPython \
	Release_SingleThreaded  \
	Debug64 Debug64_SingleThreaded \
	Debug64_GCOV Debug64_GCOV_SingleThreaded Debug64_GCOV_SystemPython \
	Debug_GCOV Debug_GCOV_SingleThreaded Debug_GCOV_SystemPython ))
all:: 
	@echo Error - Unknown configuration type $(KBE_CONFIG)
	@false
endif

LIBDIR = $(KBE_ROOT)/kbe/src/lib/bin/$(KBE_CONFIG)

ifneq (,$(findstring s, $(MAKEFLAGS)))
QUIET_BUILD=1
endif


# In order to build src/lib/python, which includes this file, we need to define
# this even when not explicitly requiring Python. This assists in setting up
# the target for libpython<version>.a when common.mak is re-included.
PYTHONLIB = python3.2


# If SEPARATE_DEBUG_INFO is defined, the debug information for an executable
# will be placed in a separate file. For example, cellapp and cellapp.dbg. The
# majority of the executable's size is debug information.
# SEPARATE_DEBUG_INFO=1

# This file is used for somewhat of a hack. We want to display a line of info
# the first time a .o is made for a component (and not display if no .o files
# are made.
MSG_FILE := make$(MAKELEVEL)_$(shell echo $$RANDOM).tmp

ifdef BIN
MAKE_LIBS=1
ifndef INSTALL_DIR
ifeq ($(IS_COMMAND),1)
	OUTPUTDIR = $(KBE_ROOT)/kbe/bin/$(KBE_CONFIG)/commands
else
	OUTPUTDIR = $(KBE_ROOT)/kbe/bin/$(KBE_CONFIG)
endif # IS_COMMAND == 1
else # INSTALL_DIR

# INSTALL_ALL_CONFIGS has been put in to be used by unit_tests so the Debug
# and Hybrid binaries are both placed in KBE_ROOT/tests/KBE_CONFIG not just
# the Hybrid builds.
ifdef INSTALL_ALL_CONFIGS
	OUTPUTDIR = $(INSTALL_DIR)/$(KBE_CONFIG)
else
# For the tools, the Hybrid configuration is automatically made into the install
# directory. Other configurations are made locally.
ifeq ($(KBE_CONFIG), Hybrid) 
	OUTPUTDIR = $(INSTALL_DIR)
else # KBE_CONFIG == Hybrid

ifeq ($(KBE_CONFIG), Hybrid64) 
	OUTPUTDIR = $(INSTALL_DIR)
else # KBE_CONFIG == Hybrid64
	OUTPUTDIR = $(KBE_CONFIG)
endif # KBE_CONFIG == Hybrid64

endif # KBE_CONFIG == Hybrid
endif # INSTALL_DIR
endif # INSTALL_ALL_CONFIGS

	OUTPUTFILE = $(OUTPUTDIR)/$(BIN)
endif # BIN

ifdef SO
MAKE_LIBS=1
ifndef OUTPUTDIR
	OUTPUTDIR = $(KBE_ROOT)/kbe/bin/$(KBE_CONFIG)/$(COMPONENT)-extensions
endif # OUTPUTDIR
	OUTPUTFILE = $(OUTPUTDIR)/$(SO).so
endif # SO

ifdef LIB
	OUTPUTDIR = $(LIBDIR)
	OUTPUTFILE = $(OUTPUTDIR)/lib$(LIB).a
endif

#----------------------------------------------------------------------------
# Macros
#----------------------------------------------------------------------------

# Our source files
OUR_C = $(addsuffix .c, $(CSRCS))
OUR_CPP = $(addsuffix .cpp, $(SRCS))
OUR_ASMS = $(addsuffix .s, $(ASMS))
ALL_SRC = $(SRCS) $(CSRCS) $(ASMS)

# All .o files that need to be linked
OBJS = $(addsuffix .o, $(ALL_SRC))

# Standard libs that everyone gets
# don't want these for a shared object - we'll use the exe's instead
ifndef SO
ifndef NO_EXTRA_LIBS
MY_LIBS += tinyxml math cstdkbe log
endif
endif

# Include and lib paths
LDFLAGS += -L$(LIBDIR)
KBE_INCLUDES += -I $(KBE_ROOT)/kbe/src/lib
KBE_INCLUDES += -I $(KBE_ROOT)/kbe/src
KBE_INCLUDES += -I $(KBE_ROOT)/kbe/src/common
KBE_INCLUDES += -I $(KBE_ROOT)/kbe/src/server
KBE_INCLUDES += -I $(KBE_ROOT)/kbe/src/lib/third_party/tinyxml

# Preprocessor output only (useful when debugging macros)
# CPPFLAGS += -E
# CPPFLAGS += -save-temps

LDLIBS += $(addprefix -l, $(MY_LIBS))
LDLIBS += -lm

ifdef USE_PYTHON

 USE_KBE_PYTHON = 1

 # If empty string != SystemPython
 ifneq (,$(findstring SystemPython, $(KBE_CONFIG)))
	USE_KBE_PYTHON = 0
 endif

 # If empty string != SingleThreaded
 ifneq (,$(findstring SingleThreaded, $(KBE_CONFIG)))
	USE_KBE_PYTHON = 1
	KBE_STANDARD_PYTHON = 1
	# These flags are defined so that any kbengine library that includes
	# the Python headers will work correctly. The src/lib/python Makefile
	# has its own definition of these if it needs to build the same way.
	CPPFLAGS+=-DKBE_DONT_WRAP_MALLOC -DKBE_PY_NO_RES_FS
 endif
 

 USE_KBE_PYTHON = 1

 # This is the version of python kbengine is redistributing
 KBE_INCLUDES += -I $(KBE_ROOT)/kbe/src/lib/python/Include
 LDLIBS += -l$(PYTHONLIB) -lpthread -lutil -ldl


endif # USE_PYTHON

# everyone needs pthread if LDLINUX_TLS_IS_BROKEN
ifdef LDLINUX_TLS_IS_BROKEN
CPPFLAGS += -DLDLINUX_TLS_IS_BROKEN
LDLIBS += -lpthread
endif

LDFLAGS += -export-dynamic

# The OpenSSL redist is used for all builds as cstdkbe/md5.[ch]pp depends
# on the OpenSSL MD5 implementation.
OPENSSL_DIR = $(KBE_ROOT)/kbe/src/lib/third_party/openssl
KBE_INCLUDES += -I$(OPENSSL_DIR)/include
ifeq ($(USE_OPENSSL),1)
LDLIBS += -lssl -lcrypto -ldl
CPPFLAGS += -DUSE_OPENSSL
endif

G3DMATH_DIR = $(KBE_ROOT)/kbe/src/lib/third_party/g3dlite
KBE_INCLUDES += -I$(G3DMATH_DIR)
ifeq ($(USE_G3DMATH),1)
LDLIBS += -lg3dlite
CPPFLAGS += -DUSE_G3DMATH
endif

ifneq (,$(findstring 64,$(KBE_CONFIG)))
	x86_64=1
	OPENSSL_CONFIG="x86_64=1"
	PYTHON_EXTRA_CFLAGS="EXTRA_CFLAGS=-m64 -fPIC"
	ARCHFLAGS=-m64 -fPIC
	MYSQL_CONFIG_PATH=/usr/lib64/mysql/mysql_config
else
	OPENSSL_CONFIG=
	PYTHON_EXTRA_CFLAGS="EXTRA_CFLAGS=-m32"
	ARCHFLAGS=-m32
	MYSQL_CONFIG_PATH=/usr/lib/mysql/mysql_config
endif

ifdef USE_CPPUNITLITE2
CPPUNITLITE2_DIR = $(KBE_ROOT)/kbe/src/lib/third_party/CppUnitLite2/src/
KBE_INCLUDES += -I$(CPPUNITLITE2_DIR)
LDLIBS += -lCppUnitLite2
endif

# Use backwards compatible hash table style. This is because Fedora Core 6
# defaults to using "gnu" style hash tables which produces incompatible
# binaries with FC5 and before.
#
# By setting it to "both", we can have advantages of the faster hash style
# on FC6 systems and backwards compatibilities with older systems, at a
# small size penalty which is < 0.1% of file size.
ifneq ($(shell gcc -dumpspecs|grep "hash-style"),)
LDFLAGS += -Wl,--hash-style=both
endif

#----------------------------------------------------------------------------
# Flags
#----------------------------------------------------------------------------

ifndef CC
CC = gcc
endif

ifndef CXX
CXX = g++
endif

ifdef QUIET_BUILD
ARFLAGS = rsu
else
ARFLAGS = rsuv
endif
# CXXFLAGS = -W -Wall -pipe -Wno-uninitialized -Wno-deprecated
CXXFLAGS = $(ARCHFLAGS) -pipe
CXXFLAGS += -Wall -Wno-deprecated
CXXFLAGS += -Wno-uninitialized -Wno-char-subscripts
CXXFLAGS += -fno-strict-aliasing -Wno-non-virtual-dtor
#CXXFLAGS += -Werror

CPPFLAGS += -DKBE_SERVER -MMD -DKBE_CONFIG=\"${KBE_CONFIG}\"

ifeq (,$(findstring SingleThreaded,$(KBE_CONFIG)))
LDLIBS += -lpthread
endif

# CPPFLAGS += -D_POSIX_THREADS -D_POSIX_THREAD_SAFE_FUNCTIONS -D_REENTRANT
# CPPFLAGS += -DINSTRUMENTATION
# CPPFLAGS += -DUDP_PROXIES

ifeq ($(KBE_CONFIG), Release)
	CXXFLAGS += -O3
	CPPFLAGS += -DCODE_INLINE -D_RELEASE
endif

ifneq (,$(findstring Hybrid,$(KBE_CONFIG)))
	CXXFLAGS += -O3 -g
	CPPFLAGS += -DCODE_INLINE -DKBE_USE_ASSERTS -D_HYBRID
endif

ifeq ($(KBE_CONFIG), Evaluation)
	CXXFLAGS += -O3 -g
	CPPFLAGS += -DCODE_INLINE -DKBE_USE_ASSERTS -D_HYBRID -DKBE_EVALUATION
endif

ifneq (,$(findstring Debug,$(KBE_CONFIG)))
	CXXFLAGS += -g
	CPPFLAGS += -DKBE_USE_ASSERTS -D_DEBUG
endif

ifeq ($(KBE_CONFIG), Release_SingleThreaded)
	CXXFLAGS += -O3
	CPPFLAGS += -DCODE_INLINE -D_RELEASE -DKBE_SINGLE_THREADED
endif

ifneq (,$(findstring SingleThreaded,$(KBE_CONFIG)))
	CPPFLAGS += -DKBE_SINGLE_THREADED

endif

ifneq (,$(findstring Evaluation,$(KBE_CONFIG)))
	CPPFLAGS += -DKBE_EVALUATION

endif

ifneq (,$(findstring GCOV, $(KBE_CONFIG)))
	CXXFLAGS += -fprofile-arcs -ftest-coverage 
endif


CCFLAGS += $(MY_DEFINES) $(MY_CPPFLAGS)
LDFLAGS += $(MY_LDFLAGS)

CFLAGS += $(ARCHFLAGS)

#----------------------------------------------------------------------------
# Build variables
#----------------------------------------------------------------------------

# These variables are defined by make (see 'make -p').

# Add KBE_INCLUDES to the compilation variables. By not including them in
# the CFLAGS / CXXFLAGS it helps tidy up the link step.
COMPILE.c = $(CC) $(CFLAGS) $(CPPFLAGS) $(KBE_INCLUDES) $(TARGET_ARCH) -c
COMPILE.cc = $(CXX) $(CXXFLAGS) $(CPPFLAGS) $(KBE_INCLUDES) $(TARGET_ARCH) -c

# Removed CPPFLAGS from the linker to remove all our -DDEFINES during linking
LINK.cc = $(CXX) $(CXXFLAGS) $(LDFLAGS) $(TARGET_ARCH)



#----------------------------------------------------------------------------
# Targets
#----------------------------------------------------------------------------

all:: $(OUTPUTDIR) $(KBE_CONFIG) $(OUTPUTFILE) done

all_config:
	$(MAKE) KBE_CONFIG=Debug
	$(MAKE) KBE_CONFIG=Hybrid
	$(MAKE) KBE_CONFIG=Release

done:
ifdef DO_NOT_BELL
else
ifeq (0, $(MAKELEVEL))
	@echo -n 
endif
endif


ifdef QUIET_BUILD
RM_FLAGS = "-f"
else
RM_FLAGS = "-fv"
endif

ifeq ($(wildcard *.cpp *.c $(KBE_CONFIG)/*.o), )	# only if it has some cpps/c or object files!
SHOULD_NOT_LINK=1
endif

clean::
	@filemissing=0;  					\
	 for i in $(SRCS); do 				\
		if [ -e $$i.cpp ]; then		 	\
			rm $(RM_FLAGS) $(KBE_CONFIG)*/`basename $$i`.[do]; \
		else 							\
			filemissing=1; 				\
		fi; 							\
	 done; 								\
	 if [ $$filemissing -ne 1 ]; then	\
		rm $(RM_FLAGS) $(KBE_CONFIG)*/* ;\
	 fi
ifdef SHOULD_NOT_LINK
	@echo Not removing $(OUTPUTFILE) since no source to remake
else
ifdef LIB
	@rm $(RM_FLAGS) $(OUTPUTDIR)*/lib$(LIB).a
else
	@rm $(RM_FLAGS) $(OUTPUTFILE)
endif
endif

ifneq ($(OUTPUTDIR), $(KBE_CONFIG))
$(OUTPUTDIR):
	@mkdir -p $(OUTPUTDIR)
endif

$(KBE_CONFIG):
	@mkdir -p $(KBE_CONFIG)

ifdef INSTALL_DIR
install::
	@mkdir -p $(INSTALL_DIR)
	@cp $(OUTPUTFILE) $(INSTALL_DIR)
else
install::
endif

ifneq ($(wildcard unit_test), )	# only if it has some cpps/c or object files!
HAS_UNIT_TEST=1
endif

unit_tests:: unit_tests_build unit_tests_run

unit_tests_build::
ifdef HAS_UNIT_TEST
	$(MAKE) -C unit_test
endif

unit_tests_run::
ifdef HAS_UNIT_TEST
	$(MAKE) -C unit_test run
endif

unit_tests_clean::
ifdef HAS_UNIT_TEST
	$(MAKE) -C unit_test clean
endif

#----------------------------------------------------------------------------
# Library dependencies
#----------------------------------------------------------------------------

# Get the full path for all non-system libraries, so we can use them
# as dependencies on the main target. We need to do a recursive make
# to work out the dependencies of each lib, and a phony target is
# necessary so the libs still get checked after they are built the
# first time.

ifdef MAKE_LIBS
MY_LIBNAMES = $(foreach L, $(MY_LIBS), $(LIBDIR)/lib$(L).a)

.PHONY: always

KBE_PYTHONLIB=$(LIBDIR)/lib$(PYTHONLIB).a

ifdef USE_KBE_PYTHON
$(KBE_PYTHONLIB): always
	@$(MAKE) -C $(KBE_ROOT)/kbe/src/lib/python $(LIBDIR)/lib$(PYTHONLIB).a \
		"KBE_STANDARD_PYTHON=$(KBE_STANDARD_PYTHON)" \
		"KBE_CONFIG=$(KBE_CONFIG)" \
		$(PYTHON_EXTRA_CFLAGS) 
endif

ifeq ($(USE_OPENSSL),1)
$(LIBDIR)/libcrypto.a: always
	@$(MAKE) -C $(OPENSSL_DIR) $(OPENSSL_CONFIG) build_crypto

$(LIBDIR)/libssl.a: always
	@$(MAKE) -C $(OPENSSL_DIR) $(OPENSSL_CONFIG) build_ssl
endif

ifdef USE_CPPUNITLITE2
$(LIBDIR)/libCppUnitLite2.a: always
	@$(MAKE) -C $(CPPUNITLITE2_DIR)
endif

# Strip the prefixed "lib" string. Be careful not to strip any _lib
$(MY_LIBNAMES): always
	$(MAKE) -C $(KBE_ROOT)/kbe/src/lib/$(subst XXXXX,_lib,$(subst lib,,$(subst _lib,XXXXX,$(*F)))) \
		"KBE_CONFIG=$(KBE_CONFIG)"

endif # MAKE_LIBS

#----------------------------------------------------------------------------
# File dependencies
#----------------------------------------------------------------------------

# If the dependency file doesn't exist, neither does the .o
# The .d will be created the first time the .o is built, so this is fine.

ifneq ($(OUR_CPP),)
-include $(addprefix $(KBE_CONFIG)/, $(notdir $(OUR_CPP:.cpp=.d)))
endif

# About the notdir: For some annoying reason, and despite the information
# in the gcc man page, %.d's are always written into the current directory.
# There's no way I'm redefining the default %.cpp rule to later move this
# (or to cd first, even 'tho that could be quite useful), so each binary
# has its own version of the %.d's. I think Murph would like this
# This does however raise one minor requirement: the binary cannot use two
# sources with the same name even if they are in different directories.
DIRLESS_OBJS = $(notdir $(OBJS))
CONFIG_OBJS = $(addprefix $(KBE_CONFIG)/, $(DIRLESS_OBJS))

# Macro that will return any string in the second arg that matches the string in
# the first argument
grep = $(foreach a,$(2),$(if $(findstring $(1),$(a)),$(a)))

# Rules to set up vpaths for sources outside the current directory.
$(foreach dd,$(call grep,/,$(CSRCS)),$(eval vpath $(notdir $(dd)).c $(dir $(dd))))
$(foreach dd,$(call grep,/,$(SRCS)),$(eval vpath $(notdir $(dd)).cpp $(dir $(dd))))
$(foreach dd,$(call grep,/,$(ASMS)),$(eval vpath $(notdir $(dd)).s $(dir $(dd))))

#----------------------------------------------------------------------------
# Precompiled headers
#----------------------------------------------------------------------------

ifdef HAS_PCH
$(KBE_CONFIG)/pch.hpp:
	echo '#include "../pch.hpp"' > $(KBE_CONFIG)/pch.hpp

$(KBE_CONFIG)/pch.hpp.gch: $(KBE_CONFIG)/pch.hpp pch.hpp
ifdef QUIET_BUILD
	test -e $(MSG_FILE) && cat $(MSG_FILE); rm -f $(MSG_FILE)
	@echo pch.hpp
endif
	rm -f $(KBE_CONFIG)/pch.hpp.gch
	$(COMPILE.cc) -x c++-header $(KBE_CONFIG)/pch.hpp $(OUTPUT_OPTION)

-include $(KBE_CONFIG)/pch.hpp.d

PCH_DEP = $(KBE_CONFIG)/pch.hpp.gch
CPPFLAGS += -include $(KBE_CONFIG)/pch.hpp
else
PCH_DEP =
endif


#----------------------------------------------------------------------------
# Implicit rules
#----------------------------------------------------------------------------

# This implicit rule is needed for three reasons.
# 1. To place .o files in the $(KBE_CONFIG) directory.
# 2. To move the .d file into the $(KBE_CONFIG) directory
# 3. To change the target in the .d file to include the $(KBE_CONFIG) directory.

# Note there is a bug in gcc 2.91, where the dependency file is always
# placed in the current directory regardless of the path. If we find it
# in the current directory, we move it into the KBE_CONFIG directory.
# So this should work for both old and new versions of gcc.

$(KBE_CONFIG)/%.o: %.cpp $(PCH_DEP)
ifdef QUIET_BUILD
	test -e $(MSG_FILE) && cat $(MSG_FILE); rm -f $(MSG_FILE)
	@echo $<
endif
	$(COMPILE.cc) $< $(OUTPUT_OPTION)
	@if test -e $*.d; then echo -n $(KBE_CONFIG)/ > $(KBE_CONFIG)/$*.d; \
		cat $*.d >> $(KBE_CONFIG)/$*.d; rm $*.d; fi

$(KBE_CONFIG)/%.o: %.c $(PCH_DEP)
ifdef QUIET_BUILD
	test -e $(MSG_FILE) && cat $(MSG_FILE); rm -f $(MSG_FILE)
	@echo $<
endif
	$(COMPILE.c) $< $(OUTPUT_OPTION)
	@if test -e $*.d; then echo -n $(KBE_CONFIG)/ > $(KBE_CONFIG)/$*.d; \
		cat $*.d >> $(KBE_CONFIG)/$*.d; rm $*.d; fi

#----------------------------------------------------------------------------
# Local targets
#----------------------------------------------------------------------------

ifeq ($(USE_OPENSSL),1)
OPENSSL_DEP = $(LIBDIR)/libssl.a $(LIBDIR)/libcrypto.a
else
OPENSSL_DEP =
endif


# For executables

ifdef BIN

# This target is an additional one for executables to create the file containing
# the "first line" info. This is the info that is displayed if any .o are made.
ifdef QUIET_BUILD
$(OUTPUTDIR)/$(BIN)::
	@echo -e \\n------ Configuration $(@F) - $(KBE_CONFIG) ------ > $(MSG_FILE)
endif

ifdef USE_KBE_PYTHON
PYTHON_DEP = $(KBE_PYTHONLIB)
else
PYTHON_DEP =
endif

ifdef USE_CPPUNITLITE2
CPPUNITLITE2_DEP = $(LIBDIR)/libCppUnitLite2.a
else
CPPUNITLITE2_DEP =
endif


$(OUTPUTDIR)/$(BIN):: $(CONFIG_OBJS) $(MY_LIBNAMES) $(PYTHON_DEP) $(OPENSSL_DEP) $(CPPUNITLITE2_DEP)

ifdef QUIET_BUILD
	test -e $(MSG_FILE) && cat $(MSG_FILE); rm -f $(MSG_FILE)
endif
ifdef BUILD_TIME_FILE
	@echo Updating Compile Time String
	@if test -e $(BUILD_TIME_FILE).cpp; then touch $(BUILD_TIME_FILE).cpp; $(MAKE) $(KBE_CONFIG)/$(BUILD_TIME_FILE).o; fi
endif
ifdef QUIET_BUILD
	@echo Linking...
endif
	$(LINK.cc) -o $@ $(CONFIG_OBJS) $(LDLIBS) $(POSTLINK)
ifdef SEPARATE_DEBUG_INFO
	@objcopy --only-keep-debug $@ $@.dbg
	@objcopy --strip-debug $@
	@objcopy --add-gnu-debuglink=$@.dbg $@
endif
ifdef QUIET_BUILD
	@echo $@
endif

# This target is an additional one to clean up "first line" info.
ifdef QUIET_BUILD
$(OUTPUTDIR)/$(BIN)::
	@rm -f $(MSG_FILE)
endif

endif # BIN



# for shared objects
ifdef SO

ifdef USE_KBE_PYTHON
PYTHON_DEP = $(KBE_PYTHONLIB)
else
PYTHON_DEP =
endif

ifdef QUIET_BUILD
$(OUTPUTDIR)/$(SO).so::
	@echo -e \\n------ Configuration $(@F) - $(KBE_CONFIG) ------ > $(MSG_FILE)
endif

ifdef BUILD_TIME_FILE
BUILD_TIME_FILE_OBJ= $(KBE_CONFIG)/$(BUILD_TIME_FILE).o
endif

$(OUTPUTDIR)/$(SO).so:: $(CONFIG_OBJS) $(MY_LIBNAMES) $(PYTHON_DEP) $(BUILD_TIME_FILE_OBJ) $(OPENSSL_DEP)
ifdef BUILD_TIME_FILE
	@echo Updating Compile Time String
	@if test -e $(BUILD_TIME_FILE).cpp; then \
		touch -m $(BUILD_TIME_FILE).cpp; \
		$(MAKE) $(BUILD_TIME_FILE_OBJ); \
	fi
endif # BUILD_TIME_FILE

ifdef QUIET_BUILD
	test -e $(MSG_FILE) && cat $(MSG_FILE); rm -f $(MSG_FILE)
	@echo Linking...
endif
	$(LINK.cc) -shared -o $@ $(CONFIG_OBJS) $(BUILD_TIME_FILE_OBJ) $(LDLIBS) $(POSTLINK) 
ifdef QUIET_BUILD
	@echo $@
endif

# This target is an additional one to clean up "first line" info.
ifdef QUIET_BUILD
$(OUTPUTDIR)/$(SO).so::
	@rm -f $(MSG_FILE)
endif

endif # SO



# For libraries

ifdef LIB

ifndef SHOULD_NOT_LINK
ifdef QUIET_BUILD
$(OUTPUTDIR)/lib$(LIB).a::
	@echo -e \\n------ Configuration $(@F) - $(KBE_CONFIG) ------ > $(MSG_FILE)
endif

$(OUTPUTDIR)/lib$(LIB).a:: $(CONFIG_OBJS)
ifdef QUIET_BUILD
	test -e $(MSG_FILE) && cat $(MSG_FILE); rm -f $(MSG_FILE)
	@echo Archiving to $(@F)
endif
	@$(AR) $(ARFLAGS) $@ $(CONFIG_OBJS)
ifdef QUIET_BUILD
	@echo $@
endif

ifdef QUIET_BUILD
$(OUTPUTDIR)/lib$(LIB).a::
	@rm -f $(MSG_FILE)
endif

else	# wildcard
#do nothing if no cpps
$(OUTPUTDIR)/lib$(LIB).a::
	@echo Not building library \'$(LIB)\' since source not present.
endif

endif	# LIB
