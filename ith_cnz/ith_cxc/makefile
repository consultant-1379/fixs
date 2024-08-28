# **********************************************************************
#
# Short description:
# Makefile template for XXX library
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
# -----      ----------   --------  --------------------------
#            2018-04-23   xcsrajn   ESM adoptations  
#            2010-11-19   xgencol   Adjusted
#            2010-10-26   teiggam   Created
#                                                            
# **********************************************************************
 
# BEGIN: GIT INTRO.
CURDIR = $(shell pwd)
REPO_NAME = fixs
FIXS_ROOT = $(shell echo $(CURDIR) | sed 's@'/$(REPO_NAME)'.*@'/$(REPO_NAME)'@g')
COMMON_ROOT = $(FIXS_ROOT)/common

include $(COMMON_ROOT)/common.mk 

CXC_NAME ?= FIXS_ITHBIN
CXC_NR ?= CXC1371528_16
CXC_VER ?= R1A01
export CXC_VER

BASE_SW_VER ?= 3.7.0-R1A
CXC_BLK_NAME?=fixs_ith

RPM_SPEC = ith.spec

CURDIR = $(shell pwd)
CXCPATH = $(CURDIR)
CXCDIR = $(CXCPATH)
export CXCDIR
export CXCPATH
FX_CLEAN_TMP := rm -rf /tmp/$(USER)/*

ITH_BUILD := $(shell date --rfc-3339=seconds)
export ITH_BUILD

#$(MAKE) $(MFLAGS) CXCDIR=$(CXCPATH) release

CAA_API_DIR = ../ithapi_caa
export CAA_API_DIR

CAA_ADM_DIR = ../ithadm_caa
export CAA_ADM_DIR

CAA_CMD_DIR = ../ithcmd_caa
export CAA_CMD_DIR

CAA_DIRS = $(CAA_ADM_DIR) $(CAA_CMD_DIR)

DOXYGEN_OUTPUT=./docgen
BLOCK_NAME=$(CXC_NAME)
BLOCK_VERSION=$(CXC_NAME)_$(CXC_VER)
BLOCK_SEARCH=$(CAA_API_DIR)/$(SRCDIR) $(CAA_API_DIR)/$(INCEXTDIR) $(CAA_API_DIR)/$(INCINTDIR) $(CAA_ADM_DIR)/$(SRCDIR) $(CAA_ADM_DIR)/inc $(CAA_CMD_DIR)/$(SRCDIR) $(CAA_CMD_DIR)/inc

FX_CLEAN_CONTRIB := find ../ -name \*.contrib* | xargs -I {} rm {}

export DOXYGEN_OUTPUT
export BLOCK_NAME
export BLOCK_VERSION
export BLOCK_SEARCH

.PHONY: all
all:
	@for i in $(CAA_DIRS) ; do \
		(echo "*****************" ; \
	     echo "***    ALL    ***" ; \
		 echo "*****************" ; \
		echo "cd $$i" ; \
		cd $$i ; \
		$(MAKE) $(MFLAGS) COMMON_ROOT=$(COMMON_ROOT) FIXS_ROOT=$(FIXS_ROOT) checkdirs ;\
		$(MAKE) $(MFLAGS) COMMON_ROOT=$(COMMON_ROOT) FIXS_ROOT=$(FIXS_ROOT) all) ;\
		if [ $$? != 0 ] ; \
		then $(ECHO) "Error: please see the printout !!!";\
		exit $$?;\
		fi;\
		echo "===================" ; \
		echo "====  END ALL  ====" ; \
		echo "===================" ; \
		done
	  $(call FX_CLEAN_TMP)

.PHONY: clean_and_build
clean_and_build: clean all

.PHONY: release
release: all rpm_package sdp_package esm_package ap_sdk documentation metrics

.PHONY: nightbuild
nightbuild: del_update distclean all rpm_package sdp_package esm_package documentation metrics doc_package copy_package 

.PHONY: del_update
del_update:
	$(call rpm_del_update,$(CXCPATH),$(BLOCK_NAME),$(CXC_VER),$(OLD_CXC_VER),$(CXC_NR))
	$(call sdp_del_update,$(CXCPATH),$(BLOCK_NAME),$(CXC_VER),$(OLD_CXC_VER),$(CXC_NR))
	$(call esm_del_update,$(CXCPATH),$(BLOCK_NAME),$(CXC_VER),$(OLD_ESM_VER),$(CXC_NR))

.PHONY: documentation
documentation:
	$(call doc_creation)
	
CPPCKECK_SRCS = $(CAA_ADM_DIR)/$(SRCDIR)/*.cpp
CPPCKECK_INCS = -I$(CAA_ADM_DIR)/inc

.PHONY: cppcheck_m
cppcheck_m:
	$(call cppcheck_doc_creation,$(CPPCKECK_SRCS),$(CPPCKECK_INCS))

CCCC_SRCS = $(CAA_ADM_DIR)/$(SRCDIR)/*

.PHONY: cccc_m
cccc_m:
	$(call cccc_doc_creation,$(CCCC_SRCS))

SLOCCOUNT_SRCS = $(CAA_ADM_DIR)/

.PHONY: sloccount_m
sloccount_m:
	$(call sloccount_doc_creation,$(SLOCCOUNT_SRCS))

.PHONY: metrics
metrics: cppcheck_m cccc_m sloccount_m
		
.PHONY: clean
clean:
		$(SILENT)$(ECHO) '===================================================================='
		$(SILENT)$(ECHO) 'CLEANING OBJECT FILES FOR BLOCK $(BLOCK_NAME)'
		$(SILENT)$(ECHO) '===================================================================='
		$(NEW_LINE)
		$(call FX_CLEAN_CONTRIB)
		$(call FX_CLEAN_TMP)
		@for i in $(CAA_DIRS) ; do \
			(echo "*****************" ; \
			 echo "***   CLEAN   ***" ; \
			 echo "*****************" ; \
			 echo "cd $$i" ; \
			 cd $$i ; \
			 $(MAKE) $(MFLAGS) COMMON_ROOT=$(COMMON_ROOT) FIXS_ROOT=$(FIXS_ROOT) clean ) ;\
			 if [ $$? != 0 ] ; \
			then $(ECHO) "Error: please see the printout !!!";\
			exit $$?;\
			fi;\
			 echo "=====================" ; \
			 echo "====  END CLEAN  ====" ; \
			 echo "=====================" ; \
		done

.PHONY: distclean
distclean:
	@for i in $(CAA_DIRS) ; do \
		(echo "+++++++++++++++++" ; \
		echo "cd $$i" ; \
		cd $$i ; \
		$(MAKE) $(MFLAGS) COMMON_ROOT=$(COMMON_ROOT) FIXS_ROOT=$(FIXS_ROOT) distclean ) ;\
		if [ $$? != 0 ] ; \
		then $(ECHO) "Error: please see the printout !!!";\
		exit $$?;\
		fi;\
		echo "+++++++++++++++++" ; \
	done
	$(call FX_CLEAN_TMP)

.PHONY: rpm_package
rpm_package:
	$(call rpm_file_creation,$(RPM_SPEC),$(CXC_NAME),$(CXC_NR),$(CXC_VER))

.PHONY: sdp_package
sdp_package:
	$(SILENT)$(ECHO) 'SDP packaging ...'
	$(call sdp_creation_US3,$(CXCPATH),$(BLOCK_NAME),$(CXC_VER),$(CXC_OLD_VER),$(CXC_NR))
	$(call up_creation,$(CXC_NAME),$(CXC_NR),$(CXC_VER),$(CXC_OLD_VER),$(CXCPATH),$(BASE_SW_VER))
.PHONY: esm_package
esm_package:
	$(call esm_pkg_creation,$(CXCPATH),$(CXC_BLK_NAME),$(CXC_VER),$(BASE_SW_VER),$(CXC_NR),$(CXC_NAME))
                      
.PHONY: upload_package 
upload_package: 
	$(call upload_package,$(CXC_BLK_NAME),$(CXC_NR),$(CXC_NAME),$(CXC_VER))

.PHONY: copy_package
copy_package:
	$(call copy_file_to_integration,$(RPMDIR)/$(CXC_NAME)-$(CXC_NR)-$(CXC_VER).x86_64.rpm)
	$(call copy_file_to_integration,$(CXC_NAME)-$(CXC_NR)-$(CXC_VER)-documentation.tar)

.PHONY: doc_package
doc_package: documentation metrics
	$(SILENT)$(ECHO) 'DOC and Metrics packaging ...'
	$(call doc_tarfile_creation,$(CXC_NAME)-$(CXC_NR)-$(CXC_VER)-documentation.tar)

.PHONY: tar_package
tar_package:
	$(SILENT)$(ECHO) 'TAR packaging ...'
	$(call tar_creation,$(BLOCK_NAME),$(CXC_NR),$(CXC_VER))

.PHONY: ap_sdk
ap_sdk:
	$(SILENT)$(ECHO) 'AP SDK updating ...'
	@for i in $(CAA_DIRS) ; do \
		(echo "+++++++++++++++++" ; \
		echo "cd $$i" ; \
		cd $$i ; \
		$(MAKE) $(MFLAGS) COMMON_ROOT=$(COMMON_ROOT) FIXS_ROOT=$(FIXS_ROOT) ap_sdk ) ;\
		if [ $$? != 0 ] ; \
		then $(ECHO) "Error: please see the printout !!!";\
		exit $$?;\
		fi;\
		echo "+++++++++++++++++" ; \
	done

.PHONY: undo_ap_sdk
undo_ap_sdk:
	$(SILENT)$(ECHO) 'UNDO AP SDK packaging ...'
	$(SILENT)$(ECHO) '**********************************'
	$(SILENT)$(ECHO) '****** NOT YET IMPLEMENTED *******'
	$(SILENT)$(ECHO) '**********************************'

