# **********************************************************************
#
# Short description:
# Makefile template for CCH service binary
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

CNZ_NAME ?= cch_cnz
CAA_NAME ?= cchadm_caa
CXC_NAME ?= cch_cxc

# Basic paths
VOB_PATH = $(FIXS_ROOT)
CNZ_PATH = $(VOB_PATH)/$(CNZ_NAME)
CAA_PATH = $(CNZ_PATH)/$(CAA_NAME)
CXC_PATH = $(CNZ_PATH)/$(CXC_NAME)
                                  
include $(CXC_PATH)/include.mk
                                 
DOXYGENFILECONFIG := $(COMMON_ROOT)/doxygen_common.conf

CAA_NAME = cchadm_caa
CAA_PATH = $(CNZ_PATH)/$(CAA_NAME)

# here you can add own lib paths
LIBSDIR  +=

CFLAGS +=  -fPIC -fmessage-length=0 -Wstrict-overflow=0 -Wno-ignored-qualifiers -Wno-empty-body
CCOVFLAGS += -fprofile-arcs -ftest-coverage

ifeq ($(COVERAGE),1)
CFLAGS += -fprofile-arcs -ftest-coverage --coverage
#CINCLUDES += -Icsprot/inc
GCOV_FLAGS =
LIBS += -lgcov -lacs_aeh
endif

LDFLAGS += -g
CCHSRV_APNAME = fixs_cchd
CCHSRV_DEBUG = fixs_cchd_dbg

CCHSRV_APBIN = $(OUTDIR)/$(CCHSRV_APNAME)

# here you can add own Include paths and/or other includes
LIBSDIR_BEFORE += #-L /vobs/cm4ap/ntacs/cs_cnz/cs_cxc/bin/lib_ext #-L "/vobs/cm4ap/ntacs/apgcc_cnz/apgcc_cxc/bin/lib_ext" 
CINCLUDES += -I inc #-I ../../../ntacs/apgcc_cnz/apgccapi_caa/OMHandling/inc_ext -I ../../../ntacs/apgcc_cnz/apgccapi_caa/OMHandling/inc_int

#trap adaptation TO REMOVE
#CINCLUDES += -I/vobs/cm4ap/ntacs/trapds_cnz/trapdsapi_caa/inc_ext

#cs adaptation TO REMOVE
CINCLUDES += #-I /vobs/cm4ap/ntacs/cs_cnz/csapi_caa/inc_ext

CINCLUDES += -I$(AP_SDK_INC)
CINCLUDES += -I$(ACE_SDK_INC)
CINCLUDES += -I$(BOOST_SDK_INC)
CINCLUDES += -I$(COREMW_SDK_INC)
CINCLUDES += -I$(LIB_XERCES_SDK_INC)
CINCLUDES += -I$(LOTC_SDK_INC)
CINCLUDES += -I$(LIB_SSH2_PATH)
#CINCLUDES += -I"$(AFS_SDK)/LDE/4_0/usr/include/linux"

#CINCLUDES += -I$(IODEV_VOB)/ACE_SDK/ACE_wrappers/
#CINCLUDES += -I$(IODEV_VOB)/ACE_SDK/ACE_wrappers/ace
#CINCLUDES += -I$(IODEV_VOB)/AP_SDK/Include
#CINCLUDES += -I$(IODEV_VOB)/3PP_SDK/libXerces/include
#CINCLUDES += -I$(IODEV_VOB)/LOTC_SDK/usr/include
#CINCLUDES += -I$(IODEV_VOB)/BOOST_SDK/boost_1_44_0/include
#CINCLUDES += -I/tool_tmp/libs/usr/include

#/Home/qlucdor/libs/usr/lib64

#CINCLUDES += -I/Home/eanform/Desktop/xerces-c-3.1.1-x86-linux-gcc-3.4/xerces-c-3.1.1-x86-linux-gcc-3.4/include
#LIBSDIR += -L"/vobs/cm4ap/ntacs/cs_cnz/cs_cxc/bin/lib_ext" #TO REMOVE
#LIBSDIR += -L$(LIBAPI_EXT_DIR) -L$(LIBAPI_INT_DIR) -L$(CXC_PATH)/ext_lib -L$(IODEV_VOB)/AP_SDK/Lib -L$(IODEV_VOB)/3PP_SDK/libXerces/lib64 \
-L$(IODEV_VOB)/AP_SDK/Lib -L"$(IODEV_VOB)/3PP_SDK/log4cplus/lib" -L$(IODEV_VOB)/LOTC_SDK/usr/lib64 -L$(IODEV_VOB)/BOOST_SDK/boost_1_44_0/lib \
-L$(IODEV_VOB)/3PP_SDK/libssh2/lib64

#LIBSDIR += -L /vobs/cm4ap/ntacs/trapds_cnz/trapds_cxc/bin/lib_ext
LIBSDIR += -L$(LIBAPI_EXT_DIR) -L$(LIBAPI_INT_DIR) -L$(CXC_PATH)/ext_lib 
LIBSDIR += -L$(AP_SDK_LIB)
LIBSDIR += -L$(ACE_SDK_LIB) 
LIBSDIR += -L$(LIB_XERCES_SDK_LIB) 
LIBSDIR += -L$(LIB_LOGCPLUS_SDK_LIB)
LIBSDIR += -L"$(3PP_SDK_PATH)/libssh2/1_0/lib64"
LIBSDIR += -L$(LOTC_SDK_LIB) -L$(LOTC_SDK_PATH)/lib64
LIBSDIR += -L$(BOOST_SDK_LIB)
LIBSDIR += -L$(COREMW_SDK_LIB)
LIBSDIR += -L$(SEC_CREDU_SDK_LIB) -L$(SEC_SDK_LIB) -L$(LIB_SSH2_SDK_LIB)
#LIBSDIR += -L$(IODEV_VOB)/3PP_SDK/libssh2/lib64

#-L /vobs/cm4ap/ntacs/trapds_cnz/trapds_cxc/bin/lib_ext 
#-L"/vobs/cm4ap/ntacs/cs_cnz/cs_cxc/bin/lib_ext" #TO REMOVE

#LIBS += -lrt -lpthread -ldhcpctl -lomapi -ldst -licuuc -licudata -lnetsnmp -lacs_dsd -lcap
#LIBS += -lgcc_s -lstdc++ -lrt -lpthread -lacs_tra -lACE -lacs_aeh -lacs_apgcc -lacs_dsd -lacs_jtp -lnetsnmp 
#CS IMPACTS
#LIBS += -lacs_csapi -lboost_serialization -lboost_filesystem -lboost_system -llog4cplus -lsec_crypto_api -lsec_credu_api
#TRAPDS IMPACTS
#LIBS += -lacs_trapdsapi -lacs_aeh -lacs_prc 
#DMX IMPACTS
#LIBS += -lacs_nclib -lxerces-c 
#LIBS += -lopensaf_core
   
# here you can add own libs
LIBS += -lrt -lpthread -lnetsnmp
LIBS += -lacs_tra -llog4cplus
LIBS += -lacs_apbmpip -lacs_apbm
LIBS += -lacs_dsd -lacs_csapi -lacs_apgcc -lacs_trapdsapi -lacs_nclib -lssh2 -lxerces-c -lACE -lacs_aeh -lacs_prc \
-lSaImmOm -lSaImmOi -lSaAmf -lSaCkpt -lSaClm -lSaLog -lSaNtf \
-lboost_serialization -lboost_filesystem  -lboost_system -lsec_crypto_api -lsec_credu_api -lacs_jtp -lcap

SOURCES +=FIXS_CCH_SRV.cpp FIXS_CCH_HA_AppManager.cpp FIXS_CCH_Event.cpp FIXS_CCH_Server.cpp FIXS_CCH_Util.cpp \
			 FIXS_CCH_ImmInterface.cpp MyImplementer_2.cpp FIXS_CCH_IMM_Util.cpp FIXS_CCH_DiskHandler.cpp \
			FIXS_CCH_ImmHandler.cpp FIXS_CCH_BladeSwManagement.cpp FIXS_CCH_Installer.cpp FIXS_CCH_ThreadActions.cpp \
			FIXS_CCH_SoftwareMap.cpp FIXS_CCH_UpgradeManager.cpp FIXS_CCH_StateMachine.cpp FIXS_CCH_SNMPManager.cpp \
			FIXS_CCH_ApManagedSw.cpp FIXS_CCH_CpManagedSw.cpp FIXS_CCH_PowerFanUpgrade.cpp FIXS_CCH_TrapTimer.cpp \
			FIXS_CCH_AlarmHandler.cpp FIXS_CCH_CmdHandler.cpp FIXS_CCH_JTPServer.cpp FIXS_CCH_JTPComm.cpp FIXS_CCH_JTPClient.cpp FIXS_CCH_JTPHandler.cpp \
			FIXS_CCH_TrapHandler.cpp FIXS_CCH_SNMPTrapReceiver.cpp FIXS_CCH_FwUpgrade.cpp FIXS_CCH_FSMDataStorage.cpp \
			FIXS_CCH_ScxUpgrade.cpp FIXS_CCH_IpmiUpgrade.cpp FIXS_CCH_BladePercistanceValue.cpp FIXS_CCH_BrmPersistentDataOwner.cpp \
			FIXS_CCH_CmxUpgrade.cpp FIXS_CCH_IptbUpgrade.cpp FIXS_CCH_SmxUpgrade.cpp\
			FIXS_CCH_IronsideRemoteSender.cpp FIXS_CCH_IronsideUdpCommand.cpp FIXS_CCH_IronsideCommandBuilder.cpp \
   		         FIXS_CCH_IronsideManager.cpp FIXS_CCH_IplbUpgrade.cpp 
	
OBJ = $(SOURCES:.cpp=.obj)
OBJFILES = $(addprefix $(OBJDIR)/,$(OBJ))

DEPS := $(SOURCES:.cpp=.d)
ifneq ($(MAKECMDGOALS),clean)
	-include $(DEPS)
endif

vpath %.cpp $(SRCDIR) ../cchapi_caa/src ../cchcmd_caa/common/src common/src
vpath %.h $(INCDIR) cchprot/inc ../cchapi_caa/inc_int ../cchapi_caa/inc_ext
vpath %.obj $(OBJDIR)
#temporary added below
#vpath %.cpp cchprot/src

#Create all
.PHONY: all
all: $(CCHSRV_APNAME)

#Create binary
.PHONY: $(CCHSRV_APNAME)
$(CCHSRV_APNAME): fs $(OBJ)
	$(NEW_LINE)
	$(SEPARATOR_STR)
	$(SILENT)$(ECHO) 'Creating executable : $(CCHSRV_APNAME)'
#   $(SILENT)$(RM) -f $(CCHSRV_APBIN)
#	$(CC) $(CFLAGS) $(APGINCLUDE) $(CINCLUDES) -o $(CCHSRV_APBIN) $(OBJFILES) $(LDFLAGS) $(LIBSDIR) $(LIBS)
	$(CC) $(OBJFILES) $(LDFLAGS) $(LIBSDIR_BEFORE) $(LIBSDIR) $(LIBS) -o $(CCHSRV_APBIN)
	$(call stripp,$(CCHSRV_APNAME))
	$(SEPARATOR_STR)
	$(NEW_LINE)
	
# Perform prebuild actions like ClearCase checkouts
	$(NEW_LINE)

# ClearCase checkins
.PHONY: checkin
checkin:
	$(NEW_LINE)
	$(SEPARATOR_STR)
	$(SILENT)$(ECHO) 'Checking in CCH srv binary $(CCHSRV_APBIN)...'
	$(SILENT) if [ '$(strip $(findstring CHECKEDOUT , $(shell $(CLEARTOOL) ls $(CCHSRV_APBIN))))' = 'CHECKEDOUT' ]; then cleartool ci -nc $(CCHSRV_APBIN); else echo 'Binary is not checked out!'; fi
	$(NEW_LINE)

# Undo checkouts
.PHONY: uncheckout
uncheckout:
	$(NEW_LINE)
	$(SEPARATOR_STR)
	$(SILENT)$(ECHO) 'Undoing checkout of CCH srv binary $(CCHSRV_APBIN)...'
	$(SILENT) if [ '$(strip $(findstring CHECKEDOUT , $(shell $(CLEARTOOL) ls $(CCHSRV_APBIN))))' = 'CHECKEDOUT' ]; then cleartool uncheckout -rm $(CCHSRV_APBIN); else echo 'Binary is not checked out!'; fi
	$(NEW_LINE)

#Clean build files
.PHONY: clean
clean:
	$(SILENT)$(ECHO) 'Cleaning SRV build files...'
	$(SILENT)$(RM) -f $(OBJDIR)/*

#Clean binary files
.PHONY: distclean
distclean: clean
	$(SILENT)$(ECHO) 'Cleaning SRV binary files: $(CCHSRV_APNAME)'
	$(SILENT)$(RM) -f $(OUTDIR)/$(CCHSRV_APNAME)

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
	$(SILENT)$(ECHO) 'Extracting symbols from $(CCHSRV_APNAME)'
	$(SILENT)$(RM) $(OUTDIR)/$(CCHSRV_APNAME).dbg
	$(SILENT)$(OBJCOPY) --only-keep-debug $(OUTDIR)/$(CCHSRV_APNAME) $(OUTDIR)/$(CCHSRV_APNAME).dbg
	$(SILENT)$(OBJCOPY) --strip-debug $(OUTDIR)/$(CCHSRV_APNAME)
	$(SILENT)$(OBJCOPY) --add-gnu-debuglink=$(OUTDIR)/$(CCHSRV_APNAME).dbg $(OUTDIR)/$(CCHSRV_APNAME)
	readelf -n $(OUTDIR)/$(CCHSRV_APNAME)
#TODO: RENAME AND MOVE ACCORDING TO DR SS5
	$(SILENT)$(MV) $(OUTDIR)/$(CCHSRV_APNAME).dbg $(CXC_PATH)/debug/SymbolFiles/$(CCHSRV_APNAME).debug
	
		
#Create dependencies
.PHONY: depend
depend: $(SRCFILES)
	makedepend $(CINCLUDES) $^

#Create obj folder if not existing
.PHONY: fs
fs:
	if [ ! -d $(OBJDIR) ]; then mkdir $(OBJDIR); fi

## DO NOT DELETE THIS LINE -- make depend needs it

