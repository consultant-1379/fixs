#!/bin/bash
##
# ------------------------------------------------------------------------
#     Copyright (C) 2016 Ericsson AB. All rights reserved.
# ------------------------------------------------------------------------
##
# Name:
#       trefc_helper.sh
##
# Description:
#       A script implemented to delete model for specific target platform
##
# Changelog:

# -  12 Nov 2018 -Naveen Kumar G (zgxxnav)
#       First version

AP_TYPE=`cat /storage/system/config/apos/aptype.conf`
HW_TYPE=`cat /storage/system/config/apos/installation_hw`

campInit() {
    if [[ "$AP_TYPE" == "AP2" || "$HW_TYPE" == "VM" ]];then
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

