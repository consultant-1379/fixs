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

CNZ_NAME ?= trefc_cnz
CAA_NAME ?= trefccmd_caa
CXC_NAME ?= trefc_cxc

# Basic paths
VOB_PATH = $(FIXS_ROOT)
CNZ_PATH = $(VOB_PATH)/$(CNZ_NAME)
CAA_PATH = $(CNZ_PATH)/$(CAA_NAME)
CXC_PATH = $(CNZ_PATH)/$(CXC_NAME)
                                  
include $(CXC_PATH)/include.mk

DOXYGENFILECONFIG := $(COMMON_ROOT)/doxygen_common.conf

CAA_NAME = cchcmd_caa
CAA_PATH = $(CNZ_PATH)/$(CAA_NAME)

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

LDFLAGS += 
CCHSRV_APNAME = showpkg
CCHSRV_APBIN = $(OUTDIR)/$(CCHSRV_APNAME)

# here you can add own Include paths and/or other includes
#CINCLUDES += -I inc -I cchprot/inc -I ../cchapi_caa/inc_ext -I ../cchapi_caa/inc_int -I../cchcmd_caa/inc \
#-I common/inc -I ../cchadm_caa/inc

CINCLUDES += -I inc -I ../cchcmd_caa/inc -I ../cchapi_caa/inc_ext -I ../cchapi_caa/inc_int \
-I ../cchadm_caa/inc

CINCLUDES += -I$(ACE_SDK_INC)
CINCLUDES += -I$(ACE_SDK_INC)/ace
CINCLUDES += -I$(ACE_SDK_INC)/ASNMP
CINCLUDES += -I$(AP_SDK_INC)
#CINCLUDES += -I/Home/eanform/Desktop/xerces-c-3.1.1-x86-linux-gcc-3.4/xerces-c-3.1.1-x86-linux-gcc-3.4/include

LIBSDIR += -L$(LIBAPI_EXT_DIR) -L$(LIBAPI_INT_DIR) -L$(CXC_PATH)/ext_lib -L$(AP_SDK_LIB)
LIBS += -lrt -lpthread -ldhcpctl -lomapi -ldst -lacs_tra -lacs_apgcc -lacs_dsd

SOURCES += cch_upgradecmd.cpp Get_opt.cpp
	
OBJ = $(SOURCES:.cpp=.obj)
OBJFILES = $(addprefix $(OBJDIR)/,$(OBJ))

DEPS := $(SOURCES:.cpp=.d)
ifneq ($(MAKECMDGOALS),clean)
	-include $(DEPS)
endif

vpath %.cpp $(SRCDIR) src ../cchapi_caa/src ../cchcmd_caa/src ../cchadm_caa/src
vpath %.h $(INCDIR) inc cchprot/inc ../cchapi_caa/inc_int ../cchapi_caa/inc_ext ../cchadm_caa/inc
vpath %.obj $(OBJDIR)
#temporary added below
#vpath %.cpp cchprot/src

#Build
.PHONY: all
all: $(CCHSRV_APNAME)

#Build + check-in check-out
.PHONY: buildall
buildall: $(CCHSRV_APNAME)

#Create binary
.PHONY: $(CCHSRV_APNAME)
$(CCHSRV_APNAME): fs $(OBJ)
	$(NEW_LINE)
	$(SEPARATOR_STR)
	$(SILENT)$(ECHO) 'Creating executable : $(CCHSRV_APNAME)'
#   $(SILENT)$(RM) -f $(CCHSRV_APBIN)
#	$(CC) $(CFLAGS) $(APGINCLUDE) $(CINCLUDES) -o $(CCHSRV_APBIN) $(OBJFILES) $(LDFLAGS) $(LIBSDIR) $(LIBS)
	$(CC) $(OBJFILES) $(LDFLAGS) $(LIBSDIR) $(LIBS) -o $(CCHSRV_APBIN)
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
	$(SILENT)$(MV) $(OUTDIR)/$(CCHSRV_APNAME).dbg $(CXCDIR)/debug/SymbolFiles/$(CCHSRV_APNAME).debug

#Create dependencies
.PHONY: depend
depend: $(SRCFILES)
	makedepend $(CINCLUDES) $^

#Create obj folder if not existing
.PHONY: fs
fs:
	if [ ! -d $(OBJDIR) ]; then mkdir $(OBJDIR); fi

## DO NOT DELETE THIS LINE -- make depend needs it

