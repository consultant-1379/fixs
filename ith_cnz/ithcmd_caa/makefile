# **********************************************************************
#
# Short description:
# Makefile template for DSD Private Implementation Library
# **********************************************************************
#
# Ericsson AB 2010 All rights reserved.
# The information in this document is the property of Ericsson.
# Except as specifically authorized in writing by Ericsson, the receiver of this
# document shall keep the information contained herein confidential and shall protect
# the same in whole or in part from disclosure and dissemination to third parties.
# Disclosure and disseminations to the receivers employees shall only be made
# on a strict need to know basis.
#
# **********************************************************************
#
# Rev        Date         Name      What
# -----      -------      --------  --------------------------
#            2016-10-14   estevol   Created
# **********************************************************************

CNZ_NAME ?= ith_cnz
CAA_NAME ?= ithcmd_caa
CXC_NAME ?= ith_cxc

# Basic paths
VOB_PATH = $(FIXS_ROOT)
CNZ_PATH = $(VOB_PATH)/$(CNZ_NAME)
CAA_PATH = $(CNZ_PATH)/$(CAA_NAME)
CXC_PATH = $(CNZ_PATH)/$(CXC_NAME)
                                  
include $(COMMON_ROOT)/common.mk
                                 
DOXYGENFILECONFIG := $(COMMON_ROOT)/doxygen_common.conf

MODULE_NAME = ITH_CMD
MODULE_VERSION = 1_0
MODULE_SEARCH = ./src ./inc
export DOXYGEN_OUTPUT
export MODULE_NAME
export MODULE_VERSION
export MODULE_SEARCH

SRC_DIR := $(SRCDIR)

## source files
SRC := $(foreach sdir,$(SRC_DIR),$(wildcard $(sdir)/*.cpp))
#COMMON_SRC := $(foreach sdir,../ithadm_caa/$(SRCDIR)/common,$(wildcard $(sdir)/*.cpp))

## Object files
ITH_PATCHEXEC_OBJS := $(patsubst src/%.cpp, $(OBJDIR)/%.obj, $(SRC))
ITH_PATCHEXEC_OBJS += $(wildcard $(CAA_ADM_DIR)/$(OBJDIR)/command/*.obj)
 

## Dependencies

ITH_PATCHEXEC_DEPS := $(ITH_PATCHEXEC_OBJS:.obj=.d)

ifneq ($(MAKECMDGOALS),clean)
-include $(ITH_PATCHEXEC_DEPS)
endif

## Application Command Names
ITH_PATCHEXEC_APPNAME := smxpatchexec

#APBM_HWICONF_CMDNAME := $(APBM_HWICONF_APPNAME)
ITH_PATCHEXEC_CMDNAME := $(ITH_PATCHEXEC_APPNAME)

ITH_PATCHEXEC_CMDNAME_DBG := $(ITH_PATCHEXEC_APPNAME)_dbg

## here you can add own lib pathsEDITCFG
LIBSDIR += -L"$(CXCPATH)/bin/lib_ext"
LIBSDIR += -L"$(AP_SDK_LIB)"
LIBSDIR += -L"$(ACE_SDK_LIB)"
LIBSDIR += -L"$(COREMW_SDK_LIB)"
LIBSDIR += -L"$(SEC_CREDU_SDK_LIB)"

## here you can add own compiler flags
CFLAGS += -Wno-empty-body -Wno-unused-parameter -Wno-unused-local-typedefs

# here you can add own linker flags
LDFLAGS += -Wl,-rpath-link,$(AP_SDK_LIB) -Wl,-rpath-link,$(ACE_SDK_LIB) -Wl,-rpath-link,$(COREMW_SDK_LIB) -Wl,-rpath-link,$(SEC_SDK_LIB) 
LDFLAGS += -Wl,-rpath-link,$(SEC_CREDU_SDK_LIB) -Wl,-rpath-link,$(BOOST_SDK_LIB) -Wl,-rpath-link,$(LIB_LOGCPLUS_SDK_LIB) -Wl,-rpath-link,$(LOTC_SDK_LIB)

EXTRA_DEFINES += -DBS_DEBUG 
EXTRA_DEFINES += -g

# here you can add more defines
EXTRA_CFLAGS += -Wextra $(EXTRA_DEFINES)

# here you can add own Include paths and/or other includes before those included by common.mk file
CINCLUDES += -I"inc"
CINCLUDES += -I"$(CAA_ADM_DIR)/inc"
CINCLUDES += -I"$(AP_SDK_INC)"
CINCLUDES += -I$(3PP_SDK_PATH)
CINCLUDES += -I$(BOOST_SDK_INC)
CINCLUDES += -I$(ACE_SDK_INC)
CINCLUDES += -I$(COREMW_SDK_INC)
CINCLUDES += -I$(LOTC_SDK_INC)

PATCHEXEC_APEXE = $(OUTDIR)/$(ITH_PATCHEXEC_CMDNAME)

# here you can add own libs
LIBS += -lrt -lpthread -lacs_nclib -lacs_csapi -lnetsnmp 

VPATH += $(SRCDIR) $(OUTDIR) inc $(OBJDIR)

.PHONY: all CFLAGS += $(GCOV_FLAGS)
all:	$(OUTDIR)/$(ITH_PATCHEXEC_APPNAME)

$(OUTDIR)/$(ITH_PATCHEXEC_APPNAME): $(ITH_PATCHEXEC_OBJS)
	$(NEW_LINE)
	$(SEPARATOR_STR)
	$(SILENT)$(ECHO) 'Creating Application: $(ITH_PATCHEXEC_CMDNAME)'
	$(SILENT)$(CC) $(CFLAGS) -o $(PATCHEXEC_APEXE) $(ITH_PATCHEXEC_OBJS) $(LDFLAGS) $(LIBSDIR) $(LIBS)
	$(call stripp,$(ITH_PATCHEXEC_APPNAME))
	$(SEPARATOR_STR)
	$(NEW_LINE)

checkdirs: $(BUILD_DIR)

$(BUILD_DIR):
	@mkdir -pm 777 $@

	

	

.PHONY: clean
clean:
	$(RM) -r $(OBJDIR)/*.obj
	$(RM) -r $(OBJDIR)/*.d

.PHONY: distclean

distclean: clean
	$(RM) -r $(OUTDIR)/$(ITH_PATCHEXEC_CMDNAME)

####################
# $(call build_application,AppName,OutpuFile,ObjectFiles)
define build_application
$(NEW_LINE)
$(SEPARATOR_STR)
$(SILENT)$(ECHO) 'Building Application: $1'
$(SILENT)$(CC) $(CFLAGS) $(EXTRA_CFLAGS) -o $2 $3 $(LDFLAGS) $(LIBSDIR) $(LIBS)
$(SEPARATOR_STR)
$(NEW_LINE)
endef

