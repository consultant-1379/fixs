# **********************************************************************
#
# Short description:
# Makefile template for TREFC service binary
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
#            2010-08-27   xgalrob   Created
#           
# **********************************************************************

CNZ_NAME ?= trefc_cnz
CAA_NAME ?= trefcadm_caa
CXC_NAME ?= trefc_cxc

# Basic paths
VOB_PATH = $(FIXS_ROOT)
CNZ_PATH = $(VOB_PATH)/$(CNZ_NAME)
CAA_PATH = $(CNZ_PATH)/$(CAA_NAME)
CXC_PATH = $(CNZ_PATH)/$(CXC_NAME)
                                  
include $(CXC_PATH)/include.mk
                                 
DOXYGENFILECONFIG := $(COMMON_ROOT)/doxygen_common.conf

CAA_NAME = trefcadm_caa
CAA_PATH = $(CNZ_PATH)/$(CAA_NAME)

# here you can add own lib paths
LIBSDIR  +=

CFLAGS += -fPIC -fmessage-length=0 -Wstrict-overflow=0 -Wno-ignored-qualifiers -Wno-empty-body
CCOVFLAGS += -fprofile-arcs -ftest-coverage

ifeq ($(COVERAGE),1)
CFLAGS += -fprofile-arcs -ftest-coverage --coverage
GCOV_FLAGS =
LIBS += -lgcov
endif

LDFLAGS += -g
TREFCSRV_APNAME = fixs_trefcd

TREFCSRV_DEBUG = fixs_trefcd_dbg

TREFCSRV_APBIN = $(OUTDIR)/$(TREFCSRV_APNAME)

# here you can add own Include paths and/or other includes
LIBSDIR_BEFORE +=-L "$(ACS_ROOT)/apgcc_cnz/apgcc_cxc/bin/lib_ext" #-L /vobs/cm4ap/ntacs/cs_cnz/cs_cxc/bin/lib_ext -L "/vobs/cm4ap/ntacs/apgcc_cnz/apgcc_cxc/bin/lib_ext" 
CINCLUDES += -I inc -I $(ACS_ROOT)/apgcc_cnz/apgccapi_caa/OMHandling/inc_ext -I $(ACS_ROOT)/apgcc_cnz/apgccapi_caa/OMHandling/inc_int
#CINCLUDES += -I /vobs/cm4ap/ntacs/trapds_cnz/trapdsapi_caa/inc_ext

#CINCLUDES += -I$(IODEV_VOB)/ACE_SDK/ACE_wrappers/
#CINCLUDES += -I$(IODEV_VOB)/ACE_SDK/ACE_wrappers/ace
#CINCLUDES += -I$(IODEV_VOB)/AP_SDK/Include
#CINCLUDES += -I$(IODEV_VOB)/3PP_SDK/libXerces/include
#CINCLUDES += -I$(IODEV_VOB)/LOTC_SDK/usr/include
#CINCLUDES += -I$(IODEV_VOB)/BOOST_SDK/boost_1_44_0/include

CINCLUDES += -I$(INCDIR) -I$(HCAPIINCDIR) 
CINCLUDES += -I$(LIB_XERCES_SDK_INC)
CINCLUDES += -I$(ACE_SDK_INC) -I$(COREMW_SDK_INC) -I$(AP_SDK_INC) -I$(BOOST_SDK_INC) -I$(LOTC_SDK_INC) -I$(SEC_SDK_INC) -I$(SEC_CREDU_SDK_INC)
CINCLUDES += -I$(LIB_LOGCPLUS_SDK_INC) -I$(LOTC_SDK_DHCP_INC)

#CINCLUDES += -I/tool_tmp/libs/usr/include

#/Home/qlucdor/libs/usr/lib64

#CINCLUDES += -I/Home/eanform/Desktop/xerces-c-3.1.1-x86-linux-gcc-3.4/xerces-c-3.1.1-x86-linux-gcc-3.4/include
#-L /vobs/cm4ap/ntacs/trapds_cnz/trapds_cxc/bin/lib_ext
#LIBSDIR += -L$(LIBAPI_EXT_DIR) -L$(LIBAPI_INT_DIR) -L$(CXC_PATH)/ext_lib  -L$(IODEV_VOB)/AP_SDK/Lib -L$(IODEV_VOB)/3PP_SDK/libXerces/lib64 \
-L$(IODEV_VOB)/AP_SDK/Lib -L"$(IODEV_VOB)/3PP_SDK/log4cplus/lib" -L$(IODEV_VOB)/LOTC_SDK/usr/lib64 -L$(IODEV_VOB)/BOOST_SDK/boost_1_44_0/lib
LIBSDIR += -L$(LIBAPI_EXT_DIR) -L$(LIBAPI_INT_DIR) -L$(CXC_PATH)/ext_lib  -L$(AP_SDK_LIB) -L$(LOTC_SDK_DHCP_LIB) -L$(LIB_XERCES_SDK_LIB) \
 -L$(LIB_LOGCPLUS_SDK_LIB) -L$(LOTC_SDK_LIB) -L$(BOOST_SDK_LIB) -L$(COREMW_SDK_LIB) -L$(SEC_SDK_LIB) -L$(SEC_CREDU_SDK_LIB) -L$(ACE_SDK_LIB) -L$(SEC_ACS_SDK_LIB)

LIBS += -lrt -lpthread -lacs_apgcc -ldhcpctl -lomapi -lacs_tra -lxerces-c -licuuc -licudata -lnetsnmp -lacs_jtp -lacs_dsd 
#CS IMPACTS
LIBS += -lboost_serialization -lboost_system 
#TRAPDS IMPACTS
LIBS += -lacs_trapdsapi -lacs_aeh -lsec_crypto_api -lboost_filesystem -lacs_prc -lacs_csapi -llog4cplus -lsec_credu_api

LIBS += $(ACE_LIBS)
LIBS += $(COREMW_LIBS)
LIBS += $(SEC_LIBS)
#LIBS += $(SEC_CREDU_LIBS)
 

SOURCES +=FIXS_TREFC_SRV.cpp FIXS_TREFC_HA_AppManager.cpp FIXS_TREFC_Server.cpp FIXS_TREFC_ImmHandler.cpp FIXS_TREFC_Util.cpp \
		 MyImplementer.cpp MyImplementer_2.cpp FIXS_TREFC_Event.cpp FIXS_TREFC_IMM_Util.cpp FIXS_TREFC_Manager.cpp \
		 FIXS_TREFC_OI_TimeServer.cpp FIXS_TREFC_OI_TimeService.cpp FIXS_TREFC_OI_NetworkConfiguration.cpp \
		 FIXS_TREFC_AlarmHandler.cpp FIXS_TREFC_TimeChecker.cpp FIXS_TREFC_SNMPManager.cpp FIXS_TREFC_SNMPTrapReceiver.cpp \
		 FIXS_TREFC_ApzScxHandler.cpp FIXS_TREFC_AptScxHandler.cpp \
		 FIXS_TREFC_JTPClient.cpp FIXS_TREFC_JTPComm.cpp FIXS_TREFC_JTPHandler.cpp FIXS_TREFC_JTPServer.cpp \
		 FIXS_TREFC_TrapSubscriber.cpp FIXS_TREFC_ImmInterface.cpp FIXS_TREFC_Logger.cpp \
		 FIXS_TREFC_OI_SwitchTimeService.cpp \
		 FIXS_TREFC_OI_SwitchTimeReference.cpp FIXS_TREFC_ThreadActions.cpp
		#FIXS_TREFC_CmdHandler.cpp FIXS_TREFC_DSDHandler.cpp  FIXS_TREFC_OI_GpsDevice.cpp
				 
	
OBJ = $(SOURCES:.cpp=.obj)
OBJFILES = $(addprefix $(OBJDIR)/,$(OBJ))

DEPS := $(SOURCES:.cpp=.d)
ifneq ($(MAKECMDGOALS),clean)
	-include $(DEPS)
endif

vpath %.cpp $(SRCDIR) ../trefcapi_caa/src ../trefccmd_caa/common/src common/src
vpath %.h $(INCDIR) ../trefcapi_caa/inc_int ../trefcapi_caa/inc_ext
vpath %.obj $(OBJDIR)
#temporary added below
#vpath %.cpp cchprot/src

#Create all
.PHONY: all
all: $(TREFCSRV_APNAME)

#Create binary
.PHONY: $(TREFCSRV_APNAME)
$(TREFCSRV_APNAME): fs $(OBJ)
	$(NEW_LINE)
	$(SEPARATOR_STR)
	$(SILENT)$(ECHO) 'Creating executable : $(TREFCSRV_APNAME)'
#   $(SILENT)$(RM) -f $(TREFCSRV_APBIN)
#	$(CC) $(CFLAGS) $(APGINCLUDE) $(CINCLUDES) -o $(TREFCSRV_APBIN) $(OBJFILES) $(LDFLAGS) $(LIBSDIR) $(LIBS)
	$(CC) $(OBJFILES) $(LDFLAGS) $(LIBSDIR_BEFORE) $(LIBSDIR) $(LIBS) -o $(TREFCSRV_APBIN)
	$(call stripp,$(TREFCSRV_APNAME))
	$(SEPARATOR_STR)
	$(NEW_LINE)
	
#Clean build files
.PHONY: clean
clean:
	$(SILENT)$(ECHO) 'Cleaning SRV build files...'
	$(SILENT)$(RM) -f $(OBJDIR)/*

#Clean binary files
.PHONY: distclean
distclean: clean
	$(SILENT)$(ECHO) 'Cleaning SRV binary files: $(TREFCSRV_APNAME)'
	$(SILENT)$(RM) -f $(OUTDIR)/$(TREFCSRV_APNAME)

# Perform chain of code analysis tools - add more?
.PHONY: metrics
metrics : cccc lint

# CCCC
# C and C++ Code Counter
.PHONY: cccc
cccc:
	$(SILENT)$(ECHO) 'C/C++ Code Counter file: $@'
	$(CCCC) $(wildcard $(INCDIR)/*) 
	$(CCCC) $(wildcard $(SRCDIR)/*) 

# LINT
# Splint is a tool for static code analysis
.PHONY: lint
lint:
	$(SILENT)$(ECHO) 'LINT file: $@'
	$(LINT) $(wildcard $(INCDIR)/*) 
	$(LINT) $(wildcard $(SRCDIR)/*)     

# Extract debug symbols
.PHONY: symbols
symbols:
	$(SILENT)$(ECHO) 'Extracting symbols from $(TREFCSRV_APNAME)'
	$(SILENT)$(RM) $(OUTDIR)/$(TREFCSRV_APNAME).dbg
	$(SILENT)$(OBJCOPY) --only-keep-debug $(OUTDIR)/$(TREFCSRV_APNAME) $(OUTDIR)/$(TREFCSRV_APNAME).dbg
	$(SILENT)$(OBJCOPY) --strip-debug $(OUTDIR)/$(TREFCSRV_APNAME)
	$(SILENT)$(OBJCOPY) --add-gnu-debuglink=$(OUTDIR)/$(TREFCSRV_APNAME).dbg $(OUTDIR)/$(TREFCSRV_APNAME)
	readelf -n $(OUTDIR)/$(TREFCSRV_APNAME)
#TODO: RENAME AND MOVE ACCORDING TO DR SS5
	$(SILENT)$(MV) $(OUTDIR)/$(TREFCSRV_APNAME).dbg $(CXC_PATH)/debug/SymbolFiles/$(TREFCSRV_APNAME).debug
	
		
#Create dependencies
.PHONY: depend
depend: $(SRCFILES)
	makedepend $(CINCLUDES) $^

#Create obj folder if not existing
.PHONY: fs
fs:
	if [ ! -d $(OBJDIR) ]; then mkdir $(OBJDIR); fi

## DO NOT DELETE THIS LINE -- make depend needs it

