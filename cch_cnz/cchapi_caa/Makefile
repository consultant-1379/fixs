# **********************************************************************
#
# Short description:
# Makefile template for CCH API library
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
CAA_NAME ?= cchapi_caa
CXC_NAME ?= cch_cxc

# Basic paths
VOB_PATH = $(FIXS_ROOT)
CNZ_PATH = $(VOB_PATH)/$(CNZ_NAME)
CAA_PATH = $(CNZ_PATH)/$(CAA_NAME)
CXC_PATH = $(CNZ_PATH)/$(CXC_NAME)
                                  
include $(CXC_PATH)/include.mk
                                 
DOXYGENFILECONFIG := $(COMMON_ROOT)/doxygen_common.conf

CAA_NAME = cchapi_caa
CAA_PATH = $(CNZ_PATH)/$(CAA_NAME)

#LIB_NAME = ACS_CSAPI
#LIB_REL_MAJOR ?= 2
#LIB_REL_MINOR ?= 2

LIB_NAME = fixs_cchapi
LIB_REL_MAJOR ?= 3
LIB_REL_MINOR ?= 1
LIB_REL_BUILD ?= 0
LIB_VER ?= $(LIB_REL_MAJOR).$(LIB_REL_MINOR).$(LIB_REL_BUILD)
LIB_FILENAME = lib$(LIB_NAME).so.$(LIB_VER)
LIB_SONAME = lib$(LIB_NAME).so.$(LIB_REL_MAJOR)
LIB_LINKER_NAME = lib$(LIB_NAME).so

FIXS_CCHAPI_APLIB = $(LIBAPI_EXT_DIR)/$(LIB_FILENAME)
FIXS_CCHAPI_APLIB_SONAME = $(LIBAPI_EXT_DIR)/$(LIB_SONAME)
FIXS_CCHAPI_APLIB_SO = $(LIBAPI_EXT_DIR)/$(LIB_LINKER_NAME)

CURRDIR = $(PWD)

# here you can add own lib paths
LIBSDIR  += -L$(LIBAPI_INT_DIR) -L$(IODEV_VOB)/AP_SDK/Lib

CFLAGS += -fPIC -fmessage-length=0
CCOVFLAGS += -fprofile-arcs -ftest-coverage

ifeq ($(COVERAGE),1)
CFLAGS += -fprofile-arcs -ftest-coverage --coverage
CFLAGS += -I../cchadm_caa/cchprot/inc
GCOV_FLAGS =
LIBS += -lgcov

endif

LDFLAGS += -shared -Wl,-soname=$(LIB_SONAME) 

# here you can add own Include paths and/or other includes
CINCLUDES += -I inc_ext -I inc_int -I ../cchadm_caa/cchprot/inc -I ../cchadm_caa/inc -I ../cchcmd_caa/common/inc \
-I../cchadm_caa/common/inc
LIBS += -lrt -lpthread # -lprot -lacs_tra

SOURCES =

OBJ = $(SOURCES:.cpp=.obj)
OBJDIR=$(CURDIR)/obj
OBJFILES = $(addprefix $(OBJDIR)/,$(OBJ))

DEPS := $(SOURCES:.cpp=.d)
ifneq ($(MAKECMDGOALS),clean)
	#-include $(DEPS)
endif

vpath %.cpp $(SRCDIR)
vpath %.h $(INCDIR)
vpath %.obj $(OBJDIR)
vpath %.cpp ../cchcmd_caa/common/src
vpath %.cpp ../cchadm_caa/cchprot/src
vpath %.cpp ../cchadm_caa/src
vpath %.cpp ../cchadm_caa/common/src

#Create all
.PHONY: all
all: $(LIB_FILENAME)

#Build 
.PHONY: buildall
all: $(LIB_FILENAME)

#Create library
.PHONY: $(LIB_FILENAME)
$(LIB_FILENAME): fs $(OBJ)
	$(NEW_LINE)
	$(SEPARATOR_STR)
	$(SILENT)$(ECHO) 'Creating library: $(LIB_FILENAME)'
#$(SILENT)$(RM) $(OBJDIR)/$(LIB_FILENAME)
	$(CC) $(CFLAGS) $(APGINCLUDE) $(CINCLUDES) -o $(FIXS_CCHAPI_APLIB) $(OBJFILES) $(LDFLAGS) $(LIBSDIR) $(LIBS)
	cd $(LIBAPI_EXT_DIR)
	ln -sf $(LIB_FILENAME) $(LIBAPI_EXT_DIR)/$(LIB_SONAME)
	ln -sf $(LIB_SONAME) $(LIBAPI_EXT_DIR)/$(LIB_LINKER_NAME)
	cd $(CURRDIR)
	$(SEPARATOR_STR)
	$(NEW_LINE)
	
# Perform prebuild actions like ClearCase checkouts
#Clean build files
.PHONY: clean
clean:
	$(SILENT)$(ECHO) 'Cleaning API build files...'
	$(SILENT)$(RM) -f $(OBJDIR)/*

#Clean binary files
.PHONY: distclean
distclean: clean
  $(SILENT)$(ECHO) 'Cleaning API binary files: $(LIB_FILENAME) and linkers...'
  $(SILENT)$(RM) -f $(LIBAPI_EXT_DIR)/$(LIB_FILENAME)
  $(SILENT)$(RM) -f $(LIBAPI_EXT_DIR)/$(LIB_SONAME)
  $(SILENT)$(RM) -f $(LIBAPI_EXT_DIR)/$(LIB_FILENAME)

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
	$(SILENT)$(ECHO) 'Extracting symbols from $(LIB_FILENAME)'
	$(SILENT)$(RM) $(LIBAPI_EXT_DIR)/$(LIB_FILENAME).dbg
	$(SILENT)$(OBJCOPY) --only-keep-debug $(LIBAPI_EXT_DIR)/$(LIB_FILENAME) $(LIBAPI_EXT_DIR)/$(LIB_FILENAME).dbg
	$(SILENT)$(OBJCOPY) --strip-debug $(LIBAPI_EXT_DIR)/$(LIB_FILENAME)
	$(SILENT)$(OBJCOPY) --add-gnu-debuglink=$(LIBAPI_EXT_DIR)/$(LIB_FILENAME) $(LIBAPI_EXT_DIR)/$(LIB_FILENAME)
	readelf -n $(LIBAPI_EXT_DIR)/$(LIB_FILENAME)
#TODO: RENAME AND MOVE ACCORDING TO DR SS5
	$(SILENT)$(MV) $(LIBAPI_EXT_DIR)/$(LIB_FILENAME).dbg $(CXCDIR)/debug/SymbolFiles/$(LIB_FILENAME).debug
  
#Create dependencies
.PHONY: depend
depend: $(SRCFILES)
	makedepend $(CINCLUDES) $^

#Create obj folder if not existing
.PHONY: fs
fs:
	if [ ! -d $(OBJDIR) ]; then mkdir $(OBJDIR); fi

## DO NOT DELETE THIS LINE -- make depend needs it

