# **********************************************************************
#
# Short description:
# Makefile template for CMXH service binary
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

CNZ_NAME ?= cmxh_cnz
CAA_NAME ?= cmxhadm_caa
CXC_NAME ?= cmxh_cxc

# Basic paths
VOB_PATH = $(FIXS_ROOT)
CNZ_PATH = $(VOB_PATH)/$(CNZ_NAME)
CAA_PATH = $(CNZ_PATH)/$(CAA_NAME)
CXC_PATH = $(CNZ_PATH)/$(CXC_NAME)
                                  
include $(CXC_PATH)/include.mk
                                 
DOXYGENFILECONFIG := $(COMMON_ROOT)/doxygen_common.conf

CAA_NAME = cmxhadm_caa
CAA_PATH = $(CNZ_PATH)/$(CAA_NAME)

# here you can add own lib paths
LIBSDIR  +=

CFLAGS += -fPIC -fmessage-length=0 -Wstrict-overflow=0 -Wno-ignored-qualifiers -Wno-empty-body
CCOVFLAGS += -fprofile-arcs -ftest-coverage

ifeq ($(COVERAGE),1)
CFLAGS += -fprofile-arcs -ftest-coverage --coverage
#CINCLUDES += -Icsprot/inc
GCOV_FLAGS =
LIBS += -lgcov -lacs_aeh
endif

LDFLAGS += -g
CMXHSRV_APNAME = fixs_cmxhd
CMXHSRV_DEBUG = fixs_cmxhd_dbg

CMXHSRV_APBIN = $(OUTDIR)/$(CMXHSRV_APNAME)

# here you can add own Include paths and/or other includes
#LIBSDIR_BEFORE += -L /vobs/cm4ap/ntacs/cs_cnz/cs_cxc/bin/lib_ext #-L "/vobs/cm4ap/ntacs/apgcc_cnz/apgcc_cxc/bin/lib_ext" 
CINCLUDES += -I inc #-I ../../../ntacs/apgcc_cnz/apgccapi_caa/OMHandling/inc_ext -I ../../../ntacs/apgcc_cnz/apgccapi_caa/OMHandling/inc_int

#trap adaptation TO REMOVE
#CINCLUDES += -I/vobs/cm4ap/ntacs/trapds_cnz/trapdsapi_caa/inc_ext

#cs adaptation TO REMOVE
#CINCLUDES += -I/vobs/cm4ap/ntacs/cs_cnz/csapi_caa/inc_ext

#CINCLUDES +=  -I/vobs/cm4ap/ntacs/cs_cnz/csapi_caa/inc_ext #TO REMOVE

#CINCLUDES += -I$(IODEV_VOB)/ACE_SDK/ACE_wrappers/
#CINCLUDES += -I$(IODEV_VOB)/ACE_SDK/ACE_wrappers/ace
#CINCLUDES += -I$(IODEV_VOB)/AP_SDK/Include
#CINCLUDES += -I$(IODEV_VOB)/3PP_SDK/libXerces/include
#CINCLUDES += -I$(IODEV_VOB)/LOTC_SDK/usr/include
#CINCLUDES += -I$(IODEV_VOB)/BOOST_SDK/boost_1_44_0/include
#CINCLUDES += -I/tool_tmp/libs/usr/include

CINCLUDES += -I$(AP_SDK_INC)
CINCLUDES += -I$(ACE_SDK_INC)
CINCLUDES += -I$(BOOST_SDK_INC)
CINCLUDES += -I$(COREMW_SDK_INC)
CINCLUDES += -I$(LIB_XERCES_SDK_INC)
CINCLUDES += -I$(LOTC_SDK_INC)

#/Home/qlucdor/libs/usr/lib64
#LIBSDIR += -L /vobs/cm4ap/ntacs/cs_cnz/cs_cxc/bin/lib_ext
#CINCLUDES += -I/Home/eanform/Desktop/xerces-c-3.1.1-x86-linux-gcc-3.4/xerces-c-3.1.1-x86-linux-gcc-3.4/include
#LIBSDIR += -L/home/xdebdch/Lib_temp
#LIBSDIR += -L /vobs/cm4ap/ntacs/trapds_cnz/trapds_cxc/bin/lib_ext
LIBSDIR += -L$(LIBAPI_EXT_DIR) -L$(LIBAPI_INT_DIR) -L$(CXC_PATH)/ext_lib  
LIBSDIR += -L$(AP_SDK_LIB)
LIBSDIR += -L$(ACE_SDK_LIB)
LIBSDIR += -L$(LIB_XERCES_SDK_LIB)
LIBSDIR += -L$(LIB_LOGCPLUS_SDK_LIB)
LIBSDIR += -L$(LIB_SSH2_SDK_LIB)
LIBSDIR += -L$(LOTC_SDK_LIB) -L$(LOTC_SDK_PATH)/lib64
LIBSDIR += -L$(BOOST_SDK_LIB)
LIBSDIR += -L$(COREMW_SDK_LIB)
LIBSDIR += -L$(SEC_CREDU_SDK_LIB) -L$(SEC_SDK_LIB)

# here you can add own libs
LIBS += -lrt -lpthread -lnetsnmp
LIBS += -lacs_tra -llog4cplus
LIBS += -lacs_apbmpip -lacs_apbm
LIBS += -lacs_dsd -lacs_csapi -lacs_apgcc -lacs_trapdsapi -lacs_nclib -lssh2 -lxerces-c -lACE -lacs_aeh -lacs_prc \
-lSaImmOm -lSaImmOi -lSaAmf -lSaCkpt -lSaClm -lSaLog -lSaNtf \
-lboost_serialization -lboost_filesystem  -lboost_system -lsec_crypto_api -lsec_credu_api

SOURCES +=FIXS_CMXH_Logger.cpp FIXS_CMXH_SRV.cpp FIXS_CMXH_HA_AppManager.cpp FIXS_CMXH_Event.cpp FIXS_CMXH_Server.cpp FIXS_CMXH_ImmInterface.cpp FIXS_CMXH_Util.cpp \
			FIXS_CMXH_IMM_Util.cpp FIXS_CMXH_Manager.cpp FIXS_CMXH_SNMPManager.cpp FIXS_CMXH_TrapHandler.cpp FIXS_CMXH_SNMPTrapReceiver.cpp FIXS_CMXH_SNMPHandler.cpp 

OBJ = $(SOURCES:.cpp=.obj)
OBJFILES = $(addprefix $(OBJDIR)/,$(OBJ))

DEPS := $(SOURCES:.cpp=.d)
ifneq ($(MAKECMDGOALS),clean)
	-include $(DEPS)
endif

vpath %.cpp $(SRCDIR) ../cmxhapi_caa/src ../cmxhcmd_caa/common/src common/src
vpath %.h $(INCDIR) cmxhprot/inc ../cmxhapi_caa/inc_int ../cmxhapi_caa/inc_ext
vpath %.obj $(OBJDIR)
#temporary added below
#vpath %.cpp cmxhhprot/src

#Create all
.PHONY: all
all: $(CMXHSRV_APNAME)

#Create binary
.PHONY: $(CMXHSRV_APNAME)
$(CMXHSRV_APNAME): fs $(OBJ)
	$(NEW_LINE)
	$(SEPARATOR_STR)
	$(SILENT)$(ECHO) 'Creating executable : $(CMXHSRV_APNAME)'
#   $(SILENT)$(RM) -f $(CMXHSRV_APBIN)
#	$(CC) $(CFLAGS) $(APGINCLUDE) $(CINCLUDES) -o $(CMXHSRV_APBIN) $(OBJFILES) $(LDFLAGS) $(LIBSDIR) $(LIBS)
	$(CC) $(OBJFILES) $(LDFLAGS) $(LIBSDIR_BEFORE) $(LIBSDIR) $(LIBS) -o $(CMXHSRV_APBIN)
	$(call stripp,$(CMXHSRV_APNAME))
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
	$(SILENT)$(ECHO) 'Cleaning SRV binary files: $(CMXHSRV_APNAME)'
	$(SILENT)$(RM) -f $(OUTDIR)/$(CMXHSRV_APNAME)

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
	$(SILENT)$(ECHO) 'Extracting symbols from $(CMXHSRV_APNAME)'
	$(SILENT)$(RM) $(OUTDIR)/$(CMXHSRV_APNAME).dbg
	$(SILENT)$(OBJCOPY) --only-keep-debug $(OUTDIR)/$(CMXHSRV_APNAME) $(OUTDIR)/$(CMXHSRV_APNAME).dbg
	$(SILENT)$(OBJCOPY) --strip-debug $(OUTDIR)/$(CMXHSRV_APNAME)
	$(SILENT)$(OBJCOPY) --add-gnu-debuglink=$(OUTDIR)/$(CMXHSRV_APNAME).dbg $(OUTDIR)/$(CMXHSRV_APNAME)
	readelf -n $(OUTDIR)/$(CMXHSRV_APNAME)
#TODO: RENAME AND MOVE ACCORDING TO DR SS5
	$(SILENT)$(MV) $(OUTDIR)/$(CMXHSRV_APNAME).dbg $(CXC_PATH)/debug/SymbolFiles/$(CMXHSRV_APNAME).debug
	
		
#Create dependencies
.PHONY: depend
depend: $(SRCFILES)
	makedepend $(CINCLUDES) $^

#Create obj folder if not existing
.PHONY: fs
fs:
	if [ ! -d $(OBJDIR) ]; then mkdir $(OBJDIR); fi

## DO NOT DELETE THIS LINE -- make depend needs it

