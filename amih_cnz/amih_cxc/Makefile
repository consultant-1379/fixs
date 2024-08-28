# **********************************************************************
#
# Short description:
# Makefile for amih
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
#            2018-04-23   xcsrajn   ESM adoptations
#            2010-09-23   xmikhal   Created
#          
# **********************************************************************

CURDIR = $(shell pwd)
REPO_NAME = fixs
FIXS_ROOT = $(shell echo $(CURDIR) | sed 's@'/$(REPO_NAME)'.*@'/$(REPO_NAME)'@g')
COMMON_ROOT = $(FIXS_ROOT)/common

include ./include.mk

CNZ_PATH = $(FIXS_ROOT)/amih_cnz
CAA_DIRS = $(CNZ_PATH)/amihadm_caa #$(CNZ_PATH)/amihcmd_caa #$(CNZ_PATH)/amihadm_caa/amihprot $(CNZ_PATH)/amihapi_caa  
RPM_SPEC = amih.spec
SDP_FILE = $(BLOCK_NAME)-$(CXC_NR)-$(CXC_VER).x86_64.sdp

DOCDIR = doc
DOXYGEN_OUTPUT = $(DOCDIR)/docgen
#BLOCK_NAME= CCH_CNZ
BLOCK_VERSION = AMIH_CNZ_1_0
BLOCK_SEARCH = $(CNZ_PATH)/amihapi_caa/src $(CNZ_PATH)/amihapi_caa/inc_int $(CNZ_PATH)/amihapi_caa/inc_ext $(CNZ_PATH)/amihcmd_caa/src $(CNZ_PATH)/amihcmd_caa/inc $(CNZ_PATH)/amihadm_caa/src $(CNZ_PATH)/amihadm_caa/inc #$(CNZ_PATH)/amihadm_caa/amihprot/src $(CNZ_PATH)/amihadm_caa/amihprot/inc 
export DOXYGEN_OUTPUT
export BLOCK_NAME
export BLOCK_VERSION
export BLOCK_SEARCH

SLOCCOUNT_SRCS = $(CNZ_PATH)/amihapi_caa/ $(CNZ_PATH)/amihcmd_caa/ $(CNZ_PATH)/amihadm_caa/  #$(CNZ_PATH)/amihadm_caa/amihprot/
CCCC_SRCS = $(CNZ_PATH)/amihapi_caa/src/* $(CNZ_PATH)/amihcmd_caa/src/* $(CNZ_PATH)/csadm_caa/src/* #$(CNZ_PATH)/amihadm_caa/amihprot/src/*
CPPCKECK_SRCS =  $(CNZ_PATH)/amihcmd_caa/src/*.cpp $(CNZ_PATH)/amihadm_caa/src/*.cpp #$(CNZ_PATH)/amihadm_caa/amihprot/src/*.cpp $(CNZ_PATH)/amihapi_caa/src/*.cpp
CPPCKECK_INCS = -I$(CNZ_PATH)/amihapi_caa/inc -I$(CNZ_PATH)/amihcmd_caa/inc -I$(CNZ_PATH)/amihadm_caa/inc  #-I$(CNZ_PATH)/amihadm_caa/amihprot/inc
	
#Create all CAA folders
.NOTPARALLEL:
.PHONY: all
all:
	@for i in $(CAA_DIRS) ; do \
	$(MAKE) -C $$i $(MFLAGS) COMMON_ROOT=$(COMMON_ROOT) FIXS_ROOT=$(FIXS_ROOT) all; \
	if [ $$? != 0 ]; \
	then $(ECHO) "Error: please see the printout !!!"; \
	exit 3; \
	fi; \
	done;

.PHONY: del_update
del_update:
	$(call rpm_del_update,$(CXC_PATH),$(BLOCK_NAME),$(CXC_VER),$(OLD_CXC_VER),$(CXC_NR))
	$(call sdp_del_update,$(CXC_PATH),$(BLOCK_NAME),$(CXC_VER),$(OLD_CXC_VER),$(CXC_NR))
	$(call esm_del_update,$(CXC_PATH),$(BLOCK_NAME),$(CXC_VER),$(OLD_ESM_VER),$(CXC_NR))

#Create all delivery file
.PHONY: release
release: all rpm_package #ap_sdk sdp_package tar_package esm_package documentation metrics

#Create sdp packages for all CAAs
.PHONY: sdp_package
sdp_package:
	$(SILENT)$(ECHO) '===================================================================='
	$(SILENT)$(ECHO) 'BUILDING SDP FOR BLOCK $(BLOCK_NAME)'
	$(SILENT)$(ECHO) '===================================================================='
	$(NEW_LINE)
	$(call sdp_creation_US3,$(CXC_PATH),$(BLOCK_NAME),$(CXC_VER),$(CXC_OLD_VER),$(CXC_NR))
	$(call up_creation,$(CXC_NAME),$(CXC_NR),$(CXC_VER),$(CXC_OLD_VER),$(CXC_PATH),$(BASE_SW_VER))

.PHONY: esm_package
esm_package:
	$(call esm_pkg_creation,$(CXC_PATH),$(CXC_BLK_NAME),$(CXC_VER),$(BASE_SW_VER),$(CXC_NR),$(CXC_NAME))
                      
.PHONY: upload_package 
upload_package: 
	$(call upload_package,$(CXC_BLK_NAME),$(CXC_NR),$(CXC_NAME),$(CXC_VER))

#Create rpm packages for all CAAs
# NOTE: Renaming packages for now since buildrpm doesn't name them with revision
.PHONY: rpm_package
rpm_package: 
	$(SILENT)$(ECHO) '===================================================================='
	$(SILENT)$(ECHO) 'BUILDING RPM FOR BLOCK $(BLOCK_NAME)'
	$(SILENT)$(ECHO) '===================================================================='
	$(NEW_LINE)	
	$(call rpm_file_creation,$(RPM_SPEC),$(BLOCK_NAME),$(CXC_NR),$(CXC_VER))

#Create TAR package
.PHONY: tar_package
tar_package:
	$(SILENT)$(ECHO) '-CALL TO TAR_PACKAGE COMMENTED. UNCOMMENT WHEN AVAILABLE IN IO_DEV COMMON.MK RELEASE'
#$(call tar_file_creation,$(BLOCK_NAME)-$(CXC_NR)-$(VERSION).x86_64.tar)

#Clean CAA folders build files
.PHONY: clean
clean:		
	@for i in $(CAA_DIRS); do \
	cd $$i ; \
	$(MAKE) $(MFLAGS) COMMON_ROOT=$(COMMON_ROOT) FIXS_ROOT=$(FIXS_ROOT) clean  ; \
	cd .. ; \
	done;
	$(call FX_CLEAN_TMP)

#Clean CAA folders binary files
.PHONY: distclean
distclean:
	@for i in $(CAA_DIRS); do \
	cd $$i ; \
	$(MAKE) $(MFLAGS) COMMON_ROOT=$(COMMON_ROOT) FIXS_ROOT=$(FIXS_ROOT) distclean ; \
	cd .. ; \
	done;
	$(SILENT) $(RM) $(RPMDIR)/$(BLOCK_NAME)-$(CXC_NR)-$(CXC_VER).x86_64.rpm
	$(call FX_CLEAN_TMP)
#	$(SILENT) $(RM) $(SDPDIR)/rpms/$(BLOCK_NAME)-$(CXC_NR)-$(CXC_VER).x86_64.rpm
#	$(SILENT) $(RM) $(SDPDIR)/$(SDP_FILE)

# Perform chain of code analysis tools
######################################################################################
# METRICS
# Perform chain of code analysis tools
######################################################################################
.PHONY: metrics
metrics: cppcheck_m cccc_m sloccount_m	

######################################################################################
# CPPCHECK
# - analysis tool for C/C++ code
######################################################################################

.PHONY: cppcheck_m
cppcheck_m:
	$(SILENT)$(ECHO) '===================================================================='
	$(SILENT)$(ECHO) 'CPPCHECK FOR BLOCK $(BLOCK_NAME)'
	$(SILENT)$(ECHO) '===================================================================='
	$(SEPARATOR_STR)
	$(call cppcheck_doc_creation,$(CPPCKECK_SRCS),$(CPPCKECK_INCS))

######################################################################################
# CCCC
# - C and C++ Code Counter
######################################################################################

.PHONY: cccc_m
cccc_m:
	$(SILENT)$(ECHO) '===================================================================='
	$(SILENT)$(ECHO) 'CCCC FOR BLOCK $(BLOCK_NAME)'
	$(SILENT)$(ECHO) '===================================================================='	
	$(call cccc_doc_creation,$(CCCC_SRCS))
   
######################################################################################
# SLOC
# - tools for counting physical Source Lines of Code
######################################################################################

.PHONY: sloccount_m
sloccount_m:
	$(SILENT)$(ECHO) '===================================================================='
	$(SILENT)$(ECHO) 'SLOC FOR BLOCK $(BLOCK_NAME)'
	$(SILENT)$(ECHO) '===================================================================='
	$(call sloccount_doc_creation,$(SLOCCOUNT_SRCS))

######################################################################################
# DOCUMENTATION
# Generate HTML documentation with Doxygen for each CAA
######################################################################################	
.PHONY: documentation
documentation:
	$(SILENT)$(ECHO) '===================================================================='
	$(SILENT)$(ECHO) 'DOCUMENTATION CHAIN FOR BLOCK $(BLOCK_NAME)'
	$(SILENT)$(ECHO) '===================================================================='
	$(NEW_LINE)
	$(call doc_creation)
	$(NEW_LINE)
	$(SILENT)$(ECHO) '===================================================================='
	$(NEW_LINE)

# ClearCase checkouts	
.PHONY: checkout
checkout:	
	$(NEW_LINE)
	$(SEPARATOR_STR)
	$(SILENT)$(ECHO) 'Checking out RPM and SDP...'
	$(SILENT)if [ -f $(RPMDIR)/$(BLOCK_NAME)-$(CXC_NR)-$(CXC_VER).x86_64.rpm ]; then if [ '$(strip $(findstring CHECKEDOUT , $(shell $(CLEARTOOL) ls $(RPMDIR)/$(BLOCK_NAME)-$(CXC_NR)-$(CXC_VER).x86_64.rpm)))' != 'CHECKEDOUT' ]; then cleartool co -nc $(RPMDIR)/$(BLOCK_NAME)-$(CXC_NR)-$(CXC_VER).x86_64.rpm; else echo 'Binary is already checked out!'; fi; fi
#	$(SILENT)if [ -f $(SDPDIR)/$(BLOCK_NAME)-$(CXC_NR)-$(CXC_VER).x86_64.sdp ]; then if [ '$(strip $(findstring CHECKEDOUT , $(shell $(CLEARTOOL) ls $(SDPDIR)/$(BLOCK_NAME)-$(CXC_NR)-$(CXC_VER).x86_64.sdp)))' != 'CHECKEDOUT' ]; then cleartool co -nc $(SDPDIR)/$(BLOCK_NAME)-$(CXC_NR)-$(CXC_VER).x86_64.sdp; else echo 'Binary is already checked out!'; fi; fi
	$(NEW_LINE)

# ClearCase checkins
.PHONY: checkin
checkin:
	$(NEW_LINE)
	$(SEPARATOR_STR)
	$(SILENT)$(ECHO) 'Checking in RPM and SDP...'
	$(SILENT)if [ -f $(RPMDIR)/$(BLOCK_NAME)-$(CXC_NR)-$(CXC_VER).x86_64.rpm ]; then if [ '$(strip $(findstring CHECKEDOUT , $(shell $(CLEARTOOL) ls $(RPMDIR)/$(BLOCK_NAME)-$(CXC_NR)-$(CXC_VER).x86_64.rpm)))' = 'CHECKEDOUT' ]; then cleartool ci -nc $(RPMDIR)/$(BLOCK_NAME)-$(CXC_NR)-$(CXC_VER).x86_64.rpm; else echo 'Binary is not checked out!'; fi; fi
#	$(SILENT)if [ -f $(SDPDIR)/$(BLOCK_NAME)-$(CXC_NR)-$(CXC_VER).x86_64.sdp ]; then if [ '$(strip $(findstring CHECKEDOUT , $(shell $(CLEARTOOL) ls $(SDPDIR)/$(BLOCK_NAME)-$(CXC_NR)-$(CXC_VER).x86_64.sdp)))' = 'CHECKEDOUT' ]; then cleartool ci -nc $(SDPDIR)/$(BLOCK_NAME)-$(CXC_NR)-$(CXC_VER).x86_64.sdp; else echo 'Binary is not checked out!'; fi; fi
	$(NEW_LINE)
	
# Undo checkouts
.PHONY: uncheckout
uncheckout:
	$(NEW_LINE)
	$(SEPARATOR_STR)
	$(SILENT)$(ECHO) 'Undoing checkout of RPM and SDP...'
	$(SILENT)touch $(RPMDIR)/$(BLOCK_NAME)-$(CXC_NR)-$(CXC_VER).x86_64.rpm
#	$(SILENT)touch $(SDPDIR)/$(BLOCK_NAME)-$(CXC_NR)-$(CXC_VER).x86_64.sdp
	$(SILENT)if [ -f $(RPMDIR)/$(BLOCK_NAME)-$(CXC_NR)-$(CXC_VER).x86_64.rpm ]; then if [ '$(strip $(findstring CHECKEDOUT , $(shell $(CLEARTOOL) ls $(RPMDIR)/$(BLOCK_NAME)-$(CXC_NR)-$(CXC_VER).x86_64.rpm)))' = 'CHECKEDOUT' ]; then cleartool uncheckout -rm $(RPMDIR)/$(BLOCK_NAME)-$(CXC_NR)-$(CXC_VER).x86_64.rpm; else echo 'Binary is not checked out!'; fi; fi
#	$(SILENT)if [ -f $(SDPDIR)/$(BLOCK_NAME)-$(CXC_NR)-$(CXC_VER).x86_64.sdp ]; then if [ '$(strip $(findstring CHECKEDOUT , $(shell $(CLEARTOOL) ls $(SDPDIR)/$(BLOCK_NAME)-$(CXC_NR)-$(CXC_VER).x86_64.sdp)))' = 'CHECKEDOUT' ]; then cleartool uncheckout -rm $(SDPDIR)/$(BLOCK_NAME)-$(CXC_NR)-$(CXC_VER).x86_64.sdp; else echo 'Binary is not checked out!'; fi; fi
	$(NEW_LINE)

######################################################################################
# AP_SDK synchronization
######################################################################################
.PHONY: ap_sdk
ap_sdk:
	$(NEW_LINE)
	$(SEPARATOR_STR)
	$(SILENT)$(ECHO) 'Synchronizing AP_SDK...'
	$(SILENT)$(ECHO) 'NOT IMPLEMENTED'
	$(NEW_LINE)

# Depends
.PHONY: depend
depend: $(SRCFILES)
	makedepend $(CINCLUDES) $^
	
# DO NOT DELETE THIS LINE -- make depend needs it

