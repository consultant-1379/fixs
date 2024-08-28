#!/bin/bash
##
# ------------------------------------------------------------------------
#     Copyright (C) 2018 Ericsson AB. All rights reserved.
# ------------------------------------------------------------------------
##
# Name:
#       ith_helper.sh
##
# Description:
#       A script implemented to delete model for specific target platform
##
# Changelog:

# -  12 Nov 2018 -Naveen Kumar G (zgxxnav)
#	First version

AP_TYPE=`cat /storage/system/config/apos/aptype.conf`
HW_TYPE=`cat /storage/system/config/apos/installation_hw`
SHELF_ARCH=`cat /storage/system/config/apos/shelf_architecture`
TARGET_TYPE=""

campInit() {

    if [[ "$HW_TYPE" == "GEP7"* ]];then
        TARGET_TYPE="GEP7${SHELF_ARCH}"
    elif [[ "$HW_TYPE" == "GEP5"* ]];then
        TARGET_TYPE="GEP5${SHELF_ARCH}"
    elif [[ "$HW_TYPE" == "GEP2" ]];then
        TARGET_TYPE="GEP2${SHELF_ARCH}"
    fi

    if [[ "$AP_TYPE" == "AP2"  ||  "$HW_TYPE" == "VM" ]];then
        `cmw-model-delete $SDP_VER`
        `cmw-model-done`
    elif [[ "$SHELF_ARCH" != "SMX" && "$TARGET_TYPE" != "GEP7SCX" ]];then
        `cmw-model-delete $SDP_VER`
        `cmw-model-done`
    fi
}

campComplete() {
    /bin/true
}

case $1 in
    init)
        SDP_VER=$2
        campInit
        ;;
    complete)
        campComplete
        ;;
esac

