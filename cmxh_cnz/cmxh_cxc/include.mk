# **********************************************************************
#
# Short description:
# CMXH common.mk configuration file
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
#            2015-05-11   xshipap   Updated for AXE16A
#            2010-10-18   xminaon   Updated with Eclipse fix
#            2010-10-01   xmikhal   Created
# **********************************************************************

CURDIR = $(shell pwd)
REPO_NAME = fixs
FIXS_ROOT = $(shell echo $(CURDIR) | sed 's@'/$(REPO_NAME)'.*@'/$(REPO_NAME)'@g')
COMMON_ROOT = $(FIXS_ROOT)/common

# Handling Eclipse clearcase paths
ifeq ($(ECLIPSE),1)
IODEV_VOB := /vobs/IO_Developments
VOB_PATH := $(FIXS_ROOT)
else
IODEV_VOB = /vobs/IO_Developments
VOB_PATH = $(FIXS_ROOT)
endif
CNZ_NAME = cmxh_cnz
CNZ_PATH = $(VOB_PATH)/$(CNZ_NAME)

CXC_NAME ?= FIXS_CMXHBIN

CXC_NAME_PATH ?= cmxh_cxc
CXC_PATH = $(CNZ_PATH)/$(CXC_NAME_PATH)
CXCDIR = $(CNZ_PATH)/$(CXC_NAME_PATH)
CXC_NR ?= CXC1371534_9
VERSION = 4.0.0-P1A00
#CXC_OLD_VER ?= -
CXC_VER ?= R1K
BASE_SW_VER?=3.6.0-R1A
CXC_BLK_NAME?=fixs_cmxh

#CXC_VER ?= P1B07
FX_CLEAN_TMP := rm -rf /tmp/$(USER)/*

BLOCK_NAME = FIXS_CMXHBIN
DOXYGENCONFIGFILE = $(CXCDIR)/doc/cmxh_doxygen.cfg
include $(COMMON_ROOT)/common.mk
LIBAPI_DIR = $(CNZ_PATH)/cmxh_cxc/bin/lib_ext

LIBAPI_EXT_DIR = $(CNZ_PATH)/cmxh_cxc/bin/lib_ext
LIBAPI_INT_DIR = $(CNZ_PATH)/cmxh_cxc/bin/lib_int

CCCC_FLAGS += --xml_outfile=$(CCCC_OUTPUT)/cccc.xml

# Handling local changes in API during development by adding local cmxhapi inc
# before IO_Developments vob
CFLAGS += -I../cmxhapi_caa/inc -I $(CNZ_PATH)/cmxhapi_caa/inc

# Handling optimization for release builds
ifneq ($(DEBUG),1)
CFLAGS += -O3
endif

