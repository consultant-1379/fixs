
CNZ_NAME ?= ith_cnz
CAA_NAME ?= ithadm_caa
CXC_NAME ?= ith_cxc

# Basic paths
VOB_PATH = $(FIXS_ROOT)
CNZ_PATH = $(VOB_PATH)/$(CNZ_NAME)
CAA_PATH = $(CNZ_PATH)/$(CAA_NAME)
CXC_PATH = $(CNZ_PATH)/$(CXC_NAME)
                                  
include $(COMMON_ROOT)/common.mk
                                 
DOXYGENFILECONFIG := $(COMMON_ROOT)/doxygen_common.conf

#***********************************************************************************************
# ADDING HERE ALL SOFTWARE MODULES UNDER "src/" TO USE 
MODULES := operation imm engine common netconf netconf/parser switch_interface alarm command
#************************************************************************************************

DOXYGEN_OUTPUT=$(DOCDIR)/docgen

# here you can add own Include paths and/or other includes before those included by common.mk file
CINCLUDES += -I$(INCDIR)
CINCLUDES += -I"$(AP_SDK_INC)"
CINCLUDES += -I$(3PP_SDK_PATH)
CINCLUDES += -I$(BOOST_SDK_INC)
CINCLUDES += -I$(ACE_SDK_INC)
CINCLUDES += -I$(COREMW_SDK_INC)
CINCLUDES += -I$(LOTC_SDK_INC)

## here you can add own lib paths
LIBSDIR += -L$(LIB_EXT)
LIBSDIR += -L$(AP_SDK_LIB)
LIBSDIR += -L$(LIB_LOGCPLUS_SDK_LIB)
LIBSDIR += -L$(BOOST_SDK_LIB)
LIBSDIR += -L$(ACE_SDK_LIB)
LIBSDIR += -L$(COREMW_SDK_LIB)
LIBSDIR += -L$(LOTC_SDK_LIB)

# here you can add own libs
# here you can add own libs
LIBS += -lrt -lpthread -lnetsnmp
LIBS += -lacs_tra
LIBS += -lacs_csapi -lacs_apgcc -lacs_trapdsapi -lacs_nclib -lacs_dsd -lacs_aeh -lACE -lboost_system -lboost_regex -lxml2

VPATH += $(SRCDIR) $(INCDIR) $(OUTDIR) $(OBJDIR)

SRC_DIR := $(SRCDIR)
SRC_DIR += $(addprefix $(SRCDIR)/,$(MODULES))

BUILD_DIR := $(addprefix $(OBJDIR)/,$(MODULES))
OBJ_CLEAN := $(addsuffix /*.obj, $(BUILD_DIR))
DEP_CLEAN := $(addsuffix /*.d, $(BUILD_DIR))

## source files
SRC := $(foreach sdir,$(SRC_DIR),$(wildcard $(sdir)/*.cpp)) 

## Object files
MODULE_OBJS := $(patsubst src/%.cpp, $(OBJDIR)/%.obj, $(SRC))

### TEMPORARY
#MODULE_OBJS += $(CAA_CMD_DIR)/$(OBJDIR)/common/patch_cmd.obj

DEPS := $(MODULE_OBJS:.obj=.d)

vpath %.cpp $(SRC_DIR)

ifeq ($(MAKECMDGOALS),all)
-include $(DEPS)
endif

# here you can add own linker flags
LDFLAGS +=

## here you can add own compiler flags
#CFLAGS += -O3 -fPIC -fmessage-length=0
CFLAGS = -g -fPIC -fmessage-length=0 -Wall

# here you can add more defines
# trace enabled
#EXTRA_DEFINES += -DFIXS_ITH_API_HAS_TRACE
#EXTRA_DEFINES += -DFIXS_ITH_TRACE_USE_SOURCE_INFO
#EXTRA_DEFINES += -DFIXS_ITH_TRACE_HAS_FUNCTION_TRACE
#EXTRA_DEFINES += -DFIXS_ITH_TRACE_USE_PRETTY_FUNCTION
# log enabled
EXTRA_DEFINES += -DFIXS_ITH_LOGGING_ENABLED
EXTRA_DEFINES += -DFIXS_ITH_LOGGING_HAS_SOURCE_INFO -DFIXS_ITH_LOGGING_USE_PRETTY_FUNCTION

EXTRA_CFLAGS += -Wextra $(EXTRA_DEFINES) -DFIXS_ITH_CXC_VER=$(CXC_VER) -DFIXS_ITH_BUILD="$(ITH_BUILD)" -Wno-ignored-qualifiers -Wno-unused-local-typedefs

# Setting application name
APPNAME = fixs_ithd
APPNAME_DBG = fixs_ithd_dbg

.PHONY: all CFLAGS += $(GCOV_FLAGS)
all: checkdirs $(OUTDIR)/$(APPNAME)

$(OUTDIR)/$(APPNAME): $(MODULE_OBJS)
	$(NEW_LINE)
	$(SEPARATOR_STR)
	$(SILENT)$(ECHO) 'Building application: $(APPNAME)'
	$(SILENT)$(ECHO) "Linking $@..."
	$(SILENT)$(CC) $(CFLAGS) $(EXTRA_CFLAGS) -o $(OUTDIR)/$(APPNAME) $(MODULE_OBJS) $(LDFLAGS) $(LIBSDIR) $(LIBS)
	#$(SILENT)$(call stripp,$(APPNAME))
	$(SEPARATOR_STR)
	$(NEW_LINE)

checkdirs: $(BUILD_DIR)

$(BUILD_DIR):
	@mkdir -pm 777 $@

.PHONY: clean
clean: checkdirs
	$(RM) -r $(OBJDIR)/*.obj
	$(RM) -r $(OBJDIR)/*.d
	$(RM) -r $(OBJ_CLEAN)
	$(RM) -r $(DEP_CLEAN)	

.PHONY: distclean
distclean: clean
	$(RM) -r $(OUTDIR)/$(APPNAME)
	$(RM) $(OUTDIR)/.debug/$(APPNAME_DBG)

######################################################################################
# DOCUMENTATION
# Generate HTML documentation with Doxygen for each CAA
######################################################################################	
.PHONY: documentation
documentation:
	$(call doc_creation)
    

