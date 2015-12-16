#
# Obtain the global build environment
#

include $(APR_WORK)/build/NWGNUenvironment.inc

#
# Define base targets and rules
# 

TARGETS = libs nlms install clobber_libs clobber_nlms clean installdev

.PHONY : $(TARGETS) default all help $(NO_LICENSE_FILE)

# Here is where we will use the NO_LICENSE_FILE variable to see if we need to
# restart the make with it defined

ifdef NO_LICENSE_FILE

default: NO_LICENSE_FILE

all: NO_LICENSE_FILE

install :: NO_LICENSE_FILE

installdev :: NO_LICENSE_FILE

NO_LICENSE_FILE :
	$(MAKE) $(MAKECMDGOALS) -f NWGNUmakefile RELEASE=$(RELEASE) DEST="$(INSTALL)" LM_LICENSE_FILE="$(METROWERKS)/license.dat"

else # LM_LICENSE_FILE must be defined so use the real targets

default: $(SUBDIRS) libs nlms

all: $(SUBDIRS) libs nlms install

$(TARGETS) :: $(SUBDIRS)

endif #NO_LICENSE_FILE check

help :
	@echo $(DL)targets for RELEASE=$(RELEASE):$(DL)
	@echo $(DL)(default) . . . . libs nlms$(DL)
	@echo $(DL)all . . . . . . . does everything (libs nlms install)$(DL)
	@echo $(DL)libs. . . . . . . builds all libs$(DL)
	@echo $(DL)nlms. . . . . . . builds all nlms$(DL)
	@echo $(DL)install . . . . . builds libs and nlms and copies install files to$(DL)
	@echo $(DL)                  "$(INSTALL)"$(DL)
	@echo $(DL)installdev. . . . copies headers and files needed for development to$(DL)
	@echo $(DL)                  "$(INSTALL)"$(DL)
	@echo $(DL)clean . . . . . . deletes $(OBJDIR) dirs, *.err, and *.map$(DL)
	@echo $(DL)clobber_all . . . deletes all possible output from the make$(DL)
	@echo $(DL)clobber_install . deletes all files in $(INSTALL)$(DL)
	@$(ECHONL)
	@echo $(DL)Multiple targets can be used on a single make command line -$(DL)
	@echo $(DL)(i.e. $(MAKE) clean all)$(DL)
	@$(ECHONL)
	@echo $(DL)You can also specify RELEASE=debug, RELEASE=noopt, or RELEASE=release$(DL)
	@echo $(DL)The default is RELEASE=release$(DL)

clobber_all :: clean clobber_install

clobber_install ::
	$(call RMDIR,$(INSTALL))

#
# build recursive targets
#

$(SUBDIRS) : FORCE
ifneq "$(MAKECMDGOALS)" "clean"
ifneq "$(findstring clobber_,$(MAKECMDGOALS))" "clobber_"
	@$(ECHONL)
	@echo Building $(CURDIR)/$@
endif
endif
	$(MAKE) -C $@ $(MAKECMDGOALS) -f NWGNUmakefile RELEASE=$(RELEASE) DEST="$(INSTALL)" LM_LICENSE_FILE="$(LM_LICENSE_FILE)"
	@$(ECHONL)

FORCE : ;

#
# Standard targets
#

clean :: $(SUBDIRS) $(APRTEST)
	@echo Cleaning up $(CURDIR)
	$(call RMDIR,$(OBJDIR))
	$(call DEL,*.err)
	$(call DEL,*.map)
	$(call DEL,*.tmp)
#	$(call DEL,*.d)

$(OBJDIR) ::
	$(call MKDIR,$@)

