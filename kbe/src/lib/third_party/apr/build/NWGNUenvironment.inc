#
# Setup needed Tools and Libraries
#

ifeq "$(wildcard $(APR_WORK)/build/NWGNUcustom.inc)" "$(APR_WORK)/build/NWGNUcustom.inc"
include $(APR_WORK)/build/NWGNUcustom.inc
CUSTOM_INI = $(AP_WORK)/NWGNUcustom.ini
endif

ifndef VERBOSE
.SILENT:
endif

#
# Treat like an include
#
ifndef EnvironmentDefined

#
# simple macros for parsing makefiles
#
EOLIST:=
EMPTY :=
COMMA := ,
SPACE := $(EMPTY) $(EMPTY)

#
# Base environment
#

# Try and handle case issues
ifndef NOVELLLIBC
ifdef NovellLibC
NOVELLLIBC = $(NovellLibC)
endif
endif

ifndef NOVELLLIBC
NOVELLLIBC = C:/novell/ndk/libc
endif
ifneq "$(wildcard $(NOVELLLIBC)/include/ndkvers.h)" "$(NOVELLLIBC)/include/ndkvers.h"
$(error NOVELLLIBC does not point to a valid Novell LIBC SDK) 
endif

ifndef LDAPSDK
LDAPSDK = C:/novell/ndk/cldapsdk/NetWare/libc
endif
ifneq "$(wildcard $(LDAPSDK)/inc/ldap.h)" "$(LDAPSDK)/inc/ldap.h"
$(error LDAPSDK does not point to a valid Novell CLDAP SDK) 
endif

ifdef EXPATSDK
ifeq "$(wildcard $(EXPATSDK)/include/expat.h)" "$(EXPATSDK)/include/expat.h"
EXPAT_IMP = $(EXPATSDK)/imports/expatlbc.imp
EXPAT_INC = $(EXPATSDK)/include
EXPAT_LIB = $(EXPATSDK)/lib/expat.lib
EXPAT_NLM = EXPATLBC
else
$(error EXPATSDK does not point to a valid EXPAT SDK) 
endif
endif
ifdef EXPATSRC
ifeq "$(wildcard $(EXPATSRC)/lib/xmlparse.c)" "$(EXPATSRC)/lib/xmlparse.c"
EXPAT_INC = $(EXPATSRC)/lib
EXPAT_LIB = $(EXPATSRC)/lib/$(OBJDIR)/expat.lib
else
$(error EXPATSRC does not point to a valid EXPAT source tree) 
endif
endif
ifndef EXPAT_INC
EXPATSRC  = $(APUXML)/expat
EXPAT_INC = $(EXPATSRC)/lib
EXPAT_LIB = $(EXPATSRC)/lib/$(OBJDIR)/expat.lib
endif

ifndef METROWERKS
METROWERKS = $(ProgramFiles)\Metrowerks\CodeWarrior
endif

# If LM_LICENSE_FILE isn't defined, define a variable that can be used to
# restart make with it defined
ifndef LM_LICENSE_FILE
NO_LICENSE_FILE = NO_LICENSE_FILE
endif

#
# Set the Release type that you want to build, possible values are:
#
#  debug	- full debug switches are set
#  noopt	- normal switches are set
#  release	- optimization switches are set (default)

ifdef reltype
RELEASE = $(reltype)
endif

ifdef RELTYPE
RELEASE = $(RELTYPE)
endif

ifdef debug
RELEASE = debug
endif

ifdef DEBUG
RELEASE = debug
endif

ifdef noopt
RELEASE = noopt
endif

ifdef NOOPT
RELEASE = noopt
endif

ifdef optimized
RELEASE = release
endif

ifdef OPTIMIZED
RELEASE = release
endif

ifndef RELEASE
RELEASE = release
endif

ifeq "$(strip $(RELEASE))" "optimized"
RELEASE = release
endif

OBJDIR = obj_$(RELEASE)

#
# Setup compiler information
#

# MetroWerks NLM tools
CC	= mwccnlm
CPP	= mwccnlm
LINK	= mwldnlm
LIB	= mwldnlm -type library -w nocmdline

# Setup build tools
AWK	= awk

#
# Declare Command and tool macros here
#

ifeq ($(findstring /sh,$(SHELL)),/sh)
DEL	= rm -f $1
RMDIR	= rm -rf $1
MKDIR	= mkdir -p $1
COPY	= cp -av $1 $2
COPYR	= cp -ar $1 $2
ECHONL	= echo ""
DL	= '
CAT	= cat
else
ifeq "$(OS)" "Windows_NT"
DEL	= $(shell if exist $(subst /,\,$1) del /q /f 2>NUL $(subst /,\,$1))
RMDIR	= $(shell if exist $(subst /,\,$1)\NUL rd /q /s 2>NUL $(subst /,\,$1))
else
DEL	= $(shell if exist $(subst /,\,$1) del 2>NUL $(subst /,\,$1))
RMDIR	= $(shell if exist $(subst /,\,$1)\NUL deltree /y 2>NUL $(subst /,\,$1))
endif
ECHONL	= $(ComSpec) /c echo.
MKDIR	= $(shell if not exist $(subst /,\,$1)\NUL md 2>NUL $(subst /,\,$1))
COPY	= copy /y 2>NUL $(subst /,\,$1) $(subst /,\,$2)
COPYR	= xcopy /y /e 2>NUL $(subst /,\,$1) $(subst /,\,$2)
CAT	= type
endif

ifdef IPV6
ifndef USE_STDSOCKETS
USE_STDSOCKETS=1
endif
endif

NOVI	= $(NOVELLLIBC)/imports
PRELUDE	= $(NOVI)/libcpre.o

INCDIRS	= $(NOVELLLIBC)/include;

DEFINES	= -DNETWARE
ifdef USE_STDSOCKETS
DEFINES	+= -DUSE_BSD_SOCKETS
else
DEFINES	+= -DUSE_WINSOCK
INCDIRS	+= $(NOVELLLIBC)/include/winsock;
endif
ifndef DEBUG
DEFINES	+= -DNDEBUG
endif

ifdef USE_STDSOCKETS
VERSION_SKT = (BSDSOCK)
else
VERSION_SKT = (WINSOCK)
endif

# MetroWerks static Libraries
CLIB3S	= $(METROWERKS)/Novell Support/Metrowerks Support/Libraries/Runtime/mwcrtl.lib
MATH3S	=
PLIB3S	= $(METROWERKS)/Novell Support/Metrowerks Support/Libraries/MSL C++/MWCPP.lib

# Base compile flags
# and prefix or precompiled header added here.

# The default flags are as follows:
#
# -c                    compile only, no link
# -gccinc               search directory of referencing file first for #includes
# -Cpp_exceptions off   disable C++ exceptions
# -RTTI off             disable C++ run-time typing information
# -align 4              align on 4 byte bounderies
# -w nocmdline          disable command-line driver/parser warnings
# -proc PII             generate code base on Pentium II instruction set
# -inst mmx             use MMX extensions (not used)

CFLAGS = -c -w nocmdline -gccinc -Cpp_exceptions off -RTTI off -align 4 -proc PII

ifeq "$(REQUIRE_PROTOTYPES)" "1"
CFLAGS += -r
endif

# -g                    generate debugging information
# -O0                   level 0 optimizations
ifeq "$(RELEASE)" "debug"
CFLAGS += -g -O0
endif

# -O4,p                 level 4 optimizations, optimize for speed
ifeq "$(RELEASE)" "release"
CFLAGS += -O4,p
endif

# -prefix apr_arch_pre_nw.h      #include pre_nw.h for all files
CFLAGS += -prefix apr_arch_pre_nw.h


ifneq ($(findstring /sh,$(SHELL)),/sh)
PATH:=$(PATH);$(METROWERKS)\bin;$(METROWERKS)\Other Metrowerks Tools\Command Line Tools
endif

#
# Declare major project deliverables output directories here
#

ifdef DEST
INSTALL = $(DEST)
ifeq (\, $(findstring \,$(INSTALL)))
INSTDIRS = $(DEST)
endif
endif

ifdef dest
INSTALL = $(dest)
ifeq (\, $(findstring \,$(INSTALL)))
INSTDIRS = $(dest)
endif
endif

ifndef INSTALL
INSTALL = $(APR_WORK)/Dist
INSTDIRS = $(APR_WORK)/Dist
BASEDIR = Apr
endif

# Add support for building IPV6 alongside
ifneq "$(IPV6)" ""
DEFINES += -DNW_BUILD_IPV6
# INCDIRS := $(NOVELLLIBC)/include/winsock/IPV6;$(INCDIRS)

ifneq "$(findstring IPV6,$(OBJDIR))" "IPV6"
OBJDIR := $(OBJDIR)_IPV6
endif
        
ifneq "$(findstring IPV6,$(INSTALL))" "IPV6"
INSTALL := $(INSTALL)_IPV6
endif        

ifneq "$(findstring IPV6,$(INSTDIRS))" "IPV6"
INSTDIRS := $(INSTDIRS)_IPV6
endif

endif

ifdef DEST

ifndef BASEDIR
BASEDIR = Apache2
endif

endif

INSTALLBASE := $(INSTALL)/$(BASEDIR)

INSTDEVDIRS := \
	$(INSTDIRS) \
	$(INSTALLBASE) \
	$(INSTALLBASE)/include \
	$(INSTALLBASE)/lib \
	$(INSTALLBASE)/bin 

INSTDIRS += \
	$(INSTALLBASE)

#
# Common directories
#

APR		= $(subst \,/,$(APR_WORK))
APU		= $(subst \,/,$(APU_WORK))
APRBUILD	= $(APR)/build
APULDAP		= $(APU)/ldap
APUXML		= $(APU)/xml
APRTEST		= $(APR)/test
APUTEST		= $(APU)/test

#
# Internal Libraries
#

APRLIB		= $(APR)/$(OBJDIR)/aprlib.lib
APULIB		= $(APU)/$(OBJDIR)/apulib.lib
APULDAPLIB	= $(APULDAP)/$(OBJDIR)/apuldap.lib
APUXMLLIB	= $(APUXML)/$(OBJDIR)/apuxml.lib

#
# Additional general defines
#

EnvironmentDefined = 1
endif # ifndef EnvironmentDefined

# This is always set so that it will show up in lower directories

ifdef Path
Path = $(PATH)
endif

