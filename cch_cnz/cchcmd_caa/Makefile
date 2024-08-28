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
CAA_NAME ?= cchcmd_caa
CXC_NAME ?= cch_cxc

# Basic paths
VOB_PATH = $(FIXS_ROOT)
CNZ_PATH = $(VOB_PATH)/$(CNZ_NAME)
CAA_PATH = $(CNZ_PATH)/$(CAA_NAME)
CXC_PATH = $(CNZ_PATH)/$(CXC_NAME)
                                  
include $(CXC_PATH)/include.mk

                                 
DOXYGENFILECONFIG := $(COMMON_ROOT)/doxygen_common.conf

CAA_NAME = xcountls
#CAA_PATH = $(CNZ_PATH)/$(CAA_NAME)

# here you can add own lib paths
LIBSDIR  +=

CFLAGS += -fPIC -fmessage-length=0 -Wstrict-overflow=0 -Wno-ignored-qualifiers -Wno-empty-body
CCOVFLAGS += -fprofile-arcs -ftest-coverage

ifeq ($(COVERAGE),1)
CFLAGS += -fprofile-arcs -ftest-coverage --coverage
#CINCLUDES += -Icsprot/inc
GCOV_FLAGS =
LIBS += -lgcov
endif

#OUTDIR = /nishome/xanttro/workspace/xcountls/bin
LDFLAGS += 
XCOUNT_APNAME = xcountls
XCOUNT_DEBUG = xcountls_dbg
XCOUNT_APBIN = $(OUTDIR)/$(XCOUNT_APNAME)
XCOUNT_SRC = cmd_Utils.cpp fixs_cch_cmdshelfmanager.cpp Get_opt.cpp FIXS_CCH_PCDataStorage.cpp xcountls_Util.cpp xcountls.cpp \
	    #fixs_cch_netconfparser.cpp  #fixs_cch_netconfbuilder.cpp/
XCOUNT_OBJ = $(XCOUNT_SRC:.cpp=.obj)

FWVER_APNAME = fwprint
FWVER_DEBUG = fwprint_dbg
FWVER_APBIN = $(OUTDIR)/$(FWVER_APNAME)
FWVER_SRC = fixs_cch_cmdshelfmanager.cpp cmd_Utils.cpp Get_opt.cpp cch_fwprintcmd.cpp #fixs_cch_netconfparser.cpp #fixs_cch_netconfbuilder.cpp 
FWVER_OBJ = $(FWVER_SRC:.cpp=.obj)

FWUPG_APNAME = fwupgexec
FWUPG_DEBUG = fwupgexec_dbg
FWUPG_APBIN = $(OUTDIR)/$(FWUPG_APNAME)
FWUPG_SRC = fixs_cch_cmdshelfmanager.cpp cmd_Utils.cpp Get_opt.cpp cch_fwupgexeccmd.cpp #fixs_cch_statemachine.cpp fixs_cch_cmdsnmpmanager.cpp 
FWUPG_OBJ = $(FWUPG_SRC:.cpp=.obj)

PFMUPG_APNAME = pfmupgexec
PFMUPG_DEBUG = pfmupgexec_dbg
PFMUPG_APBIN = $(OUTDIR)/$(PFMUPG_APNAME)
PFMUPG_SRC = cmd_Utils.cpp Get_opt.cpp cch_fwupgradecmd.cpp 
PFMUPG_OBJ = $(PFMUPG_SRC:.cpp=.obj)

PFMVER_APNAME = pfmfwprint
PFMVER_DEBUG = pfmfwprint_dbg
PFMVER_APBIN = $(OUTDIR)/$(PFMVER_APNAME)
PFMVER_SRC = cmd_Utils.cpp Get_opt.cpp cch_pfmfwprintcmd.cpp 
PFMVER_OBJ = $(PFMVER_SRC:.cpp=.obj)

IPMIUPG_APNAME = ipmiupgexec
IPMIUPG_DEBUG = ipmiupgexec_dbg
IPMIUPG_APBIN = $(OUTDIR)/$(IPMIUPG_APNAME)
IPMIUPG_SRC = cmd_Utils.cpp Get_opt.cpp cch_ipmiupgradecmd.cpp  
IPMIUPG_OBJ = $(IPMIUPG_SRC:.cpp=.obj)

IPMIVER_APNAME = ipmifwprint
IPMIVER_DEBUG = ipmifwprint_dbg
IPMIVER_APBIN = $(OUTDIR)/$(IPMIVER_APNAME)
IPMIVER_SRC = cmd_Utils.cpp Get_opt.cpp cch_ipmifwprintcmd.cpp 
IPMIVER_OBJ = $(IPMIVER_SRC:.cpp=.obj)

# here you can add own Include paths and/or other includes
#LIBSDIR_BEFORE += -L /vobs/cm4ap/ntacs/cs_cnz/cs_cxc/bin/lib_ext #-L #"/vobs/cm4ap/ntacs/apgcc_cnz/apgcc_cxc/bin/lib_ext"

#cs adaptation TO REMOVE
#CINCLUDES += -I/vobs/cm4ap/ntacs/cs_cnz/csapi_caa/inc_ext
CINCLUDES += -I /vobs/cm4ap/ntacs/apbm_cnz/apbmapi_caa/inc_ext/
CINCLUDES += -I /vobs/cm4ap/ntacs/apbm_cnz/apbmapi_caa/inc_int/

CINCLUDES += -I inc -I cchprot/inc -I ../cchapi_caa/inc_ext -I ../cchapi_caa/inc_int -I../cchcmd_caa/common/inc -I../cchadm_caa/inc -I common/inc 
#CINCLUDES += -I$(IODEV_VOB)/ACE_SDK/ACE_wrappers/
#CINCLUDES += -I$(IODEV_VOB)/ACE_SDK/ACE_wrappers/ace
#CINCLUDES += -I$(IODEV_VOB)/ACE_SDK/ACE_wrappers/ASNMP
#CINCLUDES += -I$(IODEV_VOB)/AP_SDK/Include
#CINCLUDES += -I$(IODEV_VOB)/3PP_SDK/libXerces/include
#CINCLUDES += -I$(IODEV_VOB)/LOTC_SDK/usr/include
#CINCLUDES += -I$(IODEV_VOB)/BOOST_SDK/boost_1_44_0/include

CINCLUDES += -I$(AP_SDK_INC)
CINCLUDES += -I$(ACE_SDK_INC)
CINCLUDES += -I$(BOOST_SDK_INC)
CINCLUDES += -I$(COREMW_SDK_INC)
CINCLUDES += -I$(LIB_XERCES_SDK_INC)
CINCLUDES += -I$(LOTC_SDK_INC)
CINCLUDES += -I$(LIB_CURL_SDK_INC)
#CINCLUDES += -I$(IODEV_VOB)/3PP_SDK/libssh2

#LIBSDIR += -L"/vobs/cm4ap/ntacs/cs_cnz/cs_cxc/bin/lib_ext" #TO REMOVE
#LIBSDIR += -L$(LIBAPI_EXT_DIR) -L$(LIBAPI_INT_DIR) -L$(CXC_PATH)/ext_lib -L$(IODEV_VOB)/AP_SDK/Lib -L$(IODEV_VOB)/3PP_SDK/libXerces/lib64 \
-L$(IODEV_VOB)/AP_SDK/Lib -L"$(IODEV_VOB)/3PP_SDK/log4cplus/lib" -L/vobs/cm4ap/ntacs/apbm_cnz/net-snmp/lib64 \
-L$(IODEV_VOB)/BOOST_SDK/boost_1_44_0/lib -L$(IODEV_VOB)/3PP_SDK/libssh2/lib64
#-L$(IODEV_VOB)/AP_SDK/Lib -L"$(IODEV_VOB)/3PP_SDK/log4cplus/lib" -L/tool_tmp/libs/usr/lib64 -L/usr/lib

LIBSDIR += -L"/vobs/cm4ap/ntacs/apbm_cnz/apbm_cxc/bin/lib_ext" 
LIBSDIR += -L$(LIBAPI_EXT_DIR) -L$(LIBAPI_INT_DIR) -L$(CXC_PATH)/ext_lib
LIBSDIR += -L$(AP_SDK_LIB)
LIBSDIR += -L$(ACE_SDK_LIB)
LIBSDIR += -L$(LIB_XERCES_SDK_LIB)
LIBSDIR += -L$(LIB_LOGCPLUS_SDK_LIB)
LIBSDIR += -L$(LOTC_SDK_LIB) -L$(LOTC_SDK_PATH)/lib64
LIBSDIR += -L$(BOOST_SDK_LIB)
LIBSDIR += -L$(COREMW_SDK_LIB)
LIBSDIR += -L$(SEC_CREDU_SDK_LIB) -L$(SEC_SDK_LIB) -L$(LIB_SSH2_SDK_LIB)
#LIBSDIR += -L$(SEC_CREDU_SDK_LIB) -L$(SEC_SDK_LIB) -L$(IODEV_VOB)/3PP_SDK/libssh2/lib64

#LIBS += -lrt -lpthread -ldhcpctl -lomapi -ldst -lxerces-c -licuuc \
-licudata -lnetsnmp -lboost_serialization -lacs_tra -lacs_csapi -lacs_dsd -lacs_apbm -lacs_nclib -lssh2
# -lboost_unit_test_framework #-lboost_filesystem -lboost_iostreams -lboost_system \
-lboost_date_time -lboost_graph -lboost_program_options -lboost_regex -lboost_signals

LIBS += -lgcc_s -lstdc++ -lrt -lSaImmOm -lSaImmOi -lSaAmf -lSaClm -lpthread -lacs_tra -lACE -lacs_aeh -lacs_apgcc -lacs_dsd -lacs_jtp -lnetsnmp -lacs_apbm
#CS IMPACTS
LIBS += -lacs_csapi -lboost_serialization -lboost_filesystem -lboost_system -llog4cplus -lsec_crypto_api -lsec_credu_api
#TRAPDS IMPACTS
LIBS += -lacs_trapdsapi -lacs_aeh -lacs_prc
#DMX IMPACTS
LIBS += -lacs_nclib -lssh2 -lxerces-c
#LIBS += -lopensaf_core
 
#SOURCES += fixs_cch_cmdsnmpmanager.cpp Get_opt.cpp FIXS_CCH_PCDataStorage.cpp xcountls_Util.cpp xcountls.cpp cch_fwupgradecmd.cpp

#OBJ = $(SOURCES:.cpp=.obj)
#OBJFILES = $(addprefix $(OBJDIR)/,$(OBJ))

#DEPS := $(SOURCES:.cpp=.d)
#ifneq ($(MAKECMDGOALS),clean)
#	-include $(DEPS)
#endif

vpath %.cpp $(SRCDIR) #../cchapi_caa/src ../cchcmd_caa/common/src common/src
vpath %.h $(INCDIR) #cchprot/inc ../cchapi_caa/inc_int ../cchapi_caa/inc_ext
vpath %.obj $(OBJDIR)
#temporary added below
#vpath %.cpp cchprot/src

#Create all
.PHONY: all
all: $(XCOUNT_APNAME) $(FWVER_APNAME) $(FWUPG_APNAME) $(PFMUPG_APNAME) $(PFMVER_APNAME) $(IPMIUPG_APNAME) $(IPMIVER_APNAME) #$(PFM_APNAME)

#Create binary
.PHONY: $(XCOUNT_APNAME)
$(XCOUNT_APNAME): fs $(XCOUNT_OBJ)
	$(NEW_LINE)
	$(SILENT)$(ECHO) 'Creating executable : $(XCOUNT_APNAME)'
	$(CC) $(LDFLAGS) $(LIBSDIR) $(LIBS) $(addprefix $(OBJDIR)/,$(XCOUNT_OBJ)) -o $(XCOUNT_APBIN)
	$(call stripp,$(XCOUNT_APNAME))
	$(NEW_LINE)
	
.PHONY: $(FWVER_APNAME)
$(FWVER_APNAME): fs $(FWVER_OBJ)
	$(NEW_LINE)
	$(SILENT)$(ECHO) 'Creating executable : $(FWVER_APNAME)'
	$(CC) $(LDFLAGS) $(LIBSDIR) $(LIBS) $(addprefix $(OBJDIR)/,$(FWVER_OBJ)) -o $(FWVER_APBIN)
	$(call stripp,$(FWVER_APNAME))
	$(NEW_LINE)		

.PHONY: $(FWUPG_APNAME)
$(FWUPG_APNAME): fs $(FWUPG_OBJ)
	$(NEW_LINE)
	$(SILENT)$(ECHO) 'Creating executable : $(FWUPG_APNAME)'
	$(CC) $(LDFLAGS) $(LIBSDIR) $(LIBS) $(addprefix $(OBJDIR)/,$(FWUPG_OBJ)) -o $(FWUPG_APBIN)
	$(call stripp,$(FWUPG_APNAME))
	$(NEW_LINE)	
	
.PHONY: $(PFMUPG_APNAME)
$(PFMUPG_APNAME): fs $(PFMUPG_OBJ)
	$(NEW_LINE)
	$(SILENT)$(ECHO) 'Creating executable : $(PFMUPG_APNAME)'
	$(CC) $(LDFLAGS) $(LIBSDIR) $(LIBS) $(addprefix $(OBJDIR)/,$(PFMUPG_OBJ)) -o $(PFMUPG_APBIN)
	$(call stripp,$(PFMUPG_APNAME))
	$(NEW_LINE)	
	
.PHONY: $(PFMVER_APNAME)
$(PFMVER_APNAME): fs $(PFMVER_OBJ)
	$(NEW_LINE)
	$(SILENT)$(ECHO) 'Creating executable : $(PFMVER_APNAME)'
	$(CC) $(LDFLAGS) $(LIBSDIR) $(LIBS) $(addprefix $(OBJDIR)/,$(PFMVER_OBJ)) -o $(PFMVER_APBIN)
	$(call stripp,$(PFMVER_APNAME))
	$(NEW_LINE)	
	
.PHONY: $(IPMIUPG_APNAME)
$(IPMIUPG_APNAME): fs $(IPMIUPG_OBJ)
	$(NEW_LINE)
	$(SILENT)$(ECHO) 'Creating executable : $(IPMIUPG_APNAME)'
	$(CC) $(LDFLAGS) $(LIBSDIR) $(LIBS) $(addprefix $(OBJDIR)/,$(IPMIUPG_OBJ)) -o $(IPMIUPG_APBIN)
	$(call stripp,$(IPMIUPG_APNAME))
	$(NEW_LINE)	
	
.PHONY: $(IPMIVER_APNAME)
$(IPMIVER_APNAME): fs $(IPMIVER_OBJ)
	$(NEW_LINE)
	$(SILENT)$(ECHO) 'Creating executable : $(IPMIVER_APNAME)'
	$(CC) $(LDFLAGS) $(LIBSDIR) $(LIBS) $(addprefix $(OBJDIR)/,$(IPMIVER_OBJ)) -o $(IPMIVER_APBIN)
	$(call stripp,$(IPMIVER_APNAME))
	$(NEW_LINE)	
		
#Clean build files
.PHONY: clean
clean:
	$(SILENT)$(ECHO) 'Cleaning SRV build files...'
	$(SILENT)$(RM) -f $(OBJDIR)/*

#Clean binary files
.PHONY: distclean
distclean: clean
	$(SILENT)$(ECHO) 'Cleaning SRV binary files: $(XCOUNT_APNAME)'
	$(SILENT)$(RM) -f $(OUTDIR)/$(XCOUNT_APNAME)
	#$(SILENT)$(ECHO) 'Cleaning SRV binary files: $(PFM_APNAME)'
	#$(SILENT)$(RM) -f $(OUTDIR)/$(PFM_APNAME)
	$(SILENT)$(ECHO) 'Cleaning SRV binary files: $(FWVER_APNAME)'
	$(SILENT)$(RM) -f $(OUTDIR)/$(FWVER_APNAME)
	$(SILENT)$(ECHO) 'Cleaning SRV binary files: $(FWUPG_APNAME)'
	$(SILENT)$(RM) -f $(OUTDIR)/$(FWUPG_APNAME)
	$(SILENT)$(ECHO) 'Cleaning SRV binary files: $(PFMUPG_APNAME)'
	$(SILENT)$(RM) -f $(OUTDIR)/$(PFMUPG_APNAME)
	$(SILENT)$(ECHO) 'Cleaning SRV binary files: $(PFMVER_APNAME)'
	$(SILENT)$(RM) -f $(OUTDIR)/$(PFMVER_APNAME)
	$(SILENT)$(ECHO) 'Cleaning SRV binary files: $(IPMIUPG_APNAME)'
	$(SILENT)$(RM) -f $(OUTDIR)/$(IPMIUPG_APNAME)
	$(SILENT)$(ECHO) 'Cleaning SRV binary files: $(IPMIVER_APNAME)'
	$(SILENT)$(RM) -f $(OUTDIR)/$(IPMIVER_APNAME)

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
	$(SILENT)$(ECHO) 'Extracting symbols from $(XCOUNT_APNAME)'
	$(SILENT)$(RM) $(OUTDIR)/$(XCOUNT_APNAME).dbg
	$(SILENT)$(OBJCOPY) --only-keep-debug $(OUTDIR)/$(XCOUNT_APNAME) $(OUTDIR)/$(XCOUNT_APNAME).dbg
	$(SILENT)$(OBJCOPY) --strip-debug $(OUTDIR)/$(XCOUNT_APNAME)
	$(SILENT)$(OBJCOPY) --add-gnu-debuglink=$(OUTDIR)/$(XCOUNT_APNAME).dbg $(OUTDIR)/$(XCOUNT_APNAME)
	readelf -n $(OUTDIR)/$(XCOUNT_APNAME)
#TODO: RENAME AND MOVE ACCORDING TO DR SS5
	$(SILENT)$(MV) $(OUTDIR)/$(XCOUNT_APNAME).dbg $(CXC_PATH)/debug/SymbolFiles/$(XCOUNT_APNAME).debug
	
		
#Create dependencies
.PHONY: depend
depend: $(SRCFILES)
	makedepend $(CINCLUDES) $^

#Create obj folder if not existing
.PHONY: fs
fs:
	if [ ! -d $(OBJDIR) ]; then mkdir $(OBJDIR); fi

## DO NOT DELETE THIS LINE -- make depend needs it

