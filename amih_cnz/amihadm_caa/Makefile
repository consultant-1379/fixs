# **********************************************************************
#
# Short description:
# Makefile template for AMIH service binary
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

CNZ_NAME ?= amih_cnz
CAA_NAME ?= amihadm_caa
CXC_NAME ?= amih_cxc

# Basic paths
VOB_PATH = $(FIXS_ROOT)
CNZ_PATH = $(VOB_PATH)/$(CNZ_NAME)
CAA_PATH = $(CNZ_PATH)/$(CAA_NAME)
CXC_PATH = $(CNZ_PATH)/$(CXC_NAME)
                                  
include $(CXC_PATH)/include.mk
                                 
DOXYGENFILECONFIG := $(COMMON_ROOT)/doxygen_common.conf

# here you can add own lib paths
LIBSDIR  +=

CFLAGS += -fPIC -fmessage-length=0 -Wstrict-overflow=0
CCOVFLAGS += -fprofile-arcs -ftest-coverage

ifeq ($(COVERAGE),1)
CFLAGS += -fprofile-arcs -ftest-coverage --coverage
#CINCLUDES += -Icsprot/inc
GCOV_FLAGS =
LIBS += -lgcov
endif

LDFLAGS += -g
AMIHSRV_APNAME = fixs_amihd

AMIHSRV_DEBUG = fixs_amihd_dbg

AMIHSRV_APBIN = $(OUTDIR)/$(AMIHSRV_APNAME)

# here you can add own Include paths and/or other includes
#LIBSDIR_BEFORE += -L $(FIXS_ROOT)/amih_cnz/amih_cxc/bin/lib_ext #-L "/vobs/cm4ap/ntacs/apgcc_cnz/apgcc_cxc/bin/lib_ext" 
CINCLUDES += -I inc #-I ../../../ntacs/apgcc_cnz/apgccapi_caa/OMHandling/inc_ext -I ../../../ntacs/apgcc_cnz/apgccapi_caa/OMHandling/inc_int
#CINCLUDES += -I/vobs/cm4ap/ntacs/cs_cnz/csapi_caa/inc_ext
#CINCLUDES += -I/vobs/cm4ap/ntacs/cs_cnz/csapi_caa/inc_int
CINCLUDES += -I ../amihapi_caa/inc_ext
CINCLUDES += -I ../amihapi_caa/inc_int
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

#CINCLUDES += -I/Home/eanform/Desktop/xerces-c-3.1.1-x86-linux-gcc-3.4/xerces-c-3.1.1-x86-linux-gcc-3.4/include
#LIBSDIR += -L$(LIBAPI_EXT_DIR) -L$(LIBAPI_INT_DIR) -L$(CXC_PATH)/ext_lib -L$(IODEV_VOB)/AP_SDK/Lib -L$(IODEV_VOB)/3PP_SDK/libXerces/lib64 \
-L$(IODEV_VOB)/AP_SDK/Lib -L"$(IODEV_VOB)/3PP_SDK/log4cplus/lib" -L$(IODEV_VOB)/LOTC_SDK/usr/lib64 -L$(IODEV_VOB)/BOOST_SDK/boost_1_44_0/lib
#LIBS +=  -lrt -lacs_apgcc -lpthread -lacs_apgcc -ldhcpctl -lomapi -ldst -lacs_tra -lxerces-c -licuuc -licudata -lnetsnmp -lboost_serialization -lacs_csapi -lacs_jtp -lacs_dsd -lacs_aeh
#LIBSDIR += -L /vobs/cm4ap/ntacs/cs_cnz/cs_cxc/bin/lib_ext
LIBSDIR += -L$(LIBAPI_EXT_DIR) -L$(LIBAPI_INT_DIR) -L$(CXC_PATH)/ext_lib
LIBSDIR += -L$(AP_SDK_LIB)
LIBSDIR += -L$(ACE_SDK_LIB)
LIBSDIR += -L$(LIB_XERCES_SDK_LIB)
LIBSDIR += -L$(LIB_LOGCPLUS_SDK_LIB)
LIBSDIR += -L$(LOTC_SDK_LIB) -L$(LOTC_SDK_PATH)/lib64
LIBSDIR += -L$(BOOST_SDK_LIB)
LIBSDIR += -L$(COREMW_SDK_LIB)
LIBSDIR += -L$(SEC_CREDU_SDK_LIB) -L$(SEC_SDK_LIB)

LIBS += -lgcc_s -lstdc++ -lrt -lpthread -lacs_tra -lACE -lacs_aeh -lacs_apgcc -lacs_dsd -lacs_jtp -lnetsnmp
#CS IMPACTS
LIBS += -lacs_csapi -lboost_serialization -lboost_filesystem -lboost_system -llog4cplus -lsec_crypto_api -lsec_credu_api
#TRAPDS IMPACTS
LIBS += -lacs_trapdsapi -lacs_aeh -lacs_prc
#DMX IMPACTS
LIBS += -lacs_nclib -lxerces-c
LIBS += -lSaImmOm -lSaImmOi -lSaAmf -lSaCkpt -lSaClm -lSaLog -lSaNtf \

SOURCES +=FIXS_AMIH_Event.cpp FIXS_AMIH_HA_AppManager.cpp FIXS_AMIH_HwcSubscriber.cpp FIXS_AMIH_Server.cpp FIXS_AMIH_SRV.cpp  FIXS_AMIH_Util.cpp FIXS_AMIH_JTPClient.cpp FIXS_AMIH_JTPHandler.cpp
		#FIXS_AMIH_DSDHandler.cpp 		
	
OBJ = $(SOURCES:.cpp=.obj)
OBJFILES = $(addprefix $(OBJDIR)/,$(OBJ))

DEPS := $(SOURCES:.cpp=.d)
ifneq ($(MAKECMDGOALS),clean)
	-include $(DEPS)
endif

vpath %.cpp $(SRCDIR) ../amihapi_caa/src ../amihcmd_caa/common/src common/src
vpath %.h $(INCDIR) amihprot/inc ../amihapi_caa/inc_int ../amihapi_caa/inc_ext
vpath %.obj $(OBJDIR)
#temporary added below
#vpath %.cpp amihprot/src

#Create all
.PHONY: all
all: $(AMIHSRV_APNAME)

#Create binary
.PHONY: $(AMIHSRV_APNAME)
$(AMIHSRV_APNAME): fs $(OBJ)
	$(NEW_LINE)
	$(SEPARATOR_STR)
	$(SILENT)$(ECHO) 'Creating executable : $(AMIHSRV_APNAME)'
#   $(SILENT)$(RM) -f $(AMIHSRV_APBIN)
#	$(CC) $(CFLAGS) $(APGINCLUDE) $(CINCLUDES) -o $(AMIHSRV_APBIN) $(OBJFILES) $(LDFLAGS) $(LIBSDIR) $(LIBS)
	$(CC) $(OBJFILES) $(LDFLAGS) $(LIBSDIR_BEFORE) $(LIBSDIR) $(LIBS) -o $(AMIHSRV_APBIN)
	$(call stripp,$(AMIHSRV_APNAME))
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
	$(SILENT)$(ECHO) 'Cleaning SRV binary files: $(AMIHSRV_APNAME)'
	$(SILENT)$(RM) -f $(OUTDIR)/$(AMIHSRV_APNAME)

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
	$(SILENT)$(ECHO) 'Extracting symbols from $(AMIHSRV_APNAME)'
	$(SILENT)$(RM) $(OUTDIR)/$(AMIHSRV_APNAME).dbg
	$(SILENT)$(OBJCOPY) --only-keep-debug $(OUTDIR)/$(AMIHSRV_APNAME) $(OUTDIR)/$(AMIHSRV_APNAME).dbg
	$(SILENT)$(OBJCOPY) --strip-debug $(OUTDIR)/$(AMIHSRV_APNAME)
	$(SILENT)$(OBJCOPY) --add-gnu-debuglink=$(OUTDIR)/$(AMIHSRV_APNAME).dbg $(OUTDIR)/$(AMIHSRV_APNAME)
	readelf -n $(OUTDIR)/$(AMIHSRV_APNAME)
#TODO: RENAME AND MOVE ACCORDING TO DR SS5
	$(SILENT)$(MV) $(OUTDIR)/$(AMIHSRV_APNAME).dbg $(CXC_PATH)/debug/SymbolFiles/$(AMIHSRV_APNAME).debug
	
		
#Create dependencies
.PHONY: depend
depend: $(SRCFILES)
	makedepend $(CINCLUDES) $^

#Create obj folder if not existing
.PHONY: fs
fs:
	if [ ! -d $(OBJDIR) ]; then mkdir $(OBJDIR); fi

## DO NOT DELETE THIS LINE -- make depend needs it

