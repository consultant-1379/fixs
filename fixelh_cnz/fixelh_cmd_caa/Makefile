# **********************************************************************
#
# Short description:
# Makefile busrv
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
# Rev		 Date		  Name      What
# -----      ---------    --------  --------------------------
#		     2013-05-28   xdtthng   Created
#		     2016-01-26   egiaava   Updated for SLES12 and new BOOST 1_54
#
# ********************************************************************

# The following variables are defined in common.mk file
#SRCDIR = src
#OBJDIR = obj
#OUTDIR = $(CXC_PATH)/bin
#INCDIR = inc

CNZ_NAME ?= fixelh_cnz
CAA_NAME ?= fixelh_cmd_caa
CXC_NAME ?= fixelh_cxc

# Basic paths
VOB_PATH = $(FIXS_ROOT)
CNZ_PATH = $(VOB_PATH)/$(CNZ_NAME)
CAA_PATH = $(CNZ_PATH)/$(CAA_NAME)
CXC_PATH = $(CNZ_PATH)/$(CXC_NAME)
                                  
include $(COMMON_ROOT)/common.mk
                                 
DOXYGENFILECONFIG := $(COMMON_ROOT)/doxygen_common.conf

FIXERLH_APNAME = fixerls
FIXERLH_APEXE  = $(OUTDIR)/$(FIXERLH_APNAME)

# This is for testing new api version
# Comment this out, once the api is released into IO developemtn
# FIXS_CMD_LIB = /vobs/mas/mas_anz/misclh_cnz/misclh_cxc/bin/lib_ext
# FIXS_CMD_INC = /vobs/mas/mas_anz/misclh_cnz/misclhapi_caa/inc_ext

# Define objs want to build
FIXERLH_OBJ = $(OBJDIR)/fixerls.o
      
# Define own Include paths
#CINCLUDES += -I$(FIXS_CMD_INC)
CINCLUDES += -I$(INCDIR) -I$(BOOST_SDK_INC) -I$(AP_SDK_INC) -I$(3PP_SDK_PATH) -I$(COREMW_SDK_INC)

# Define own lib paths
#LIBSDIR += -L$(FIXS_CMD_LIB)
LIBSDIR += -L$(BOOST_SDK_LIB) -L$(AP_SDK_LIB) -L$(LIB_LOGCPLUS_SDK_LIB)\

# Define own Libs
LIBS += -lboost_regex -lboost_system -lboost_thread -lacs_apgcc -llog4cplus -lacs_tra -lacs_csapi\
-lacs_aeh -lacs_prc
LIBS += -lmas_fixscmd

# Define own CFLAGS
ifeq ($(COVERAGE),1)
	CFLAGS += -O0
else
	CFLAGS += -Os
endif
#CFLAGS += -g -Wall -ansi -DNDEBUG -DLOCAL_BUILD
CFLAGS += -g -Wall -ansi -DNDEBUG

LDFLAGS += -Wl,-rpath-link,$(AP_SDK_LIB)
LDFLAGS += -Wl,-rpath-link,$(COREMW_SDK_LIB)
LDFLAGS += -Wl,-rpath-link,$(SEC_SDK_LIB)
LDFLAGS += -Wl,-rpath-link,$(SEC_CREDU_SDK_LIB)
LDFLAGS += -Wl,-rpath-link,$(LIB_LOGCPLUS_SDK_LIB)
LDFLAGS += -Wl,-rpath-link,$(LIB_XERCES_SDK_LIB)
LDFLAGS += -Wl,-rpath-link,$(BOOST_SDK_LIB)
LDFLAGS += -Wl,-rpath-link,$(ACE_SDK_LIB)

.PHONY: all
all: $(OUTDIR)/$(FIXERLH_APNAME)
   
$(OUTDIR)/$(FIXERLH_APNAME): $(FIXERLH_OBJ)
	$(SILENT)$(ECHO) 'Linking $@... '
	$(SILENT)$(CC) -o $(FIXERLH_APEXE) $(FIXERLH_OBJ) $(LDFLAGS) $(LIBSDIR) $(LIBS)
	$(SILENT)$(ECHO) '$(FIXERLH_APEXE) is created'
	$(call stripp,$(FIXERLH_APNAME))
	$(SEPARATOR_STR)
	$(NEW_LINE)	

# Include dependecies in order to build when header file changes
DEPS = $(FIXERLH_OBJ:.o=.d)
ifeq ($(MAKECMDGOALS),all)	
	-include $(DEPS)
else
	ifeq ($(MAKECMDGOALS),)
		-include $(DEPS)
	endif
endif

# this rule is defined in common.mk
#$(OBJDIR)/%.o: $(SRCDIR)/%.cpp

.PHONY: clean
clean:
	$(SILENT)$(RM) -r $(OBJDIR)/*

.PHONY: distclean
distclean: clean
	$(SILENT)$(RM) -r $(OUTDIR)/$(FIXERLH_APNAME)
	$(SILENT)$(RM) -r $(DEBUGDIR)/$(FIXERLH_APNAME)_dbg


