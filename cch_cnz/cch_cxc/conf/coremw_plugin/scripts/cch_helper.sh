#!/bin/bash
##
# ------------------------------------------------------------------------
#     Copyright (C) 2016 Ericsson AB. All rights reserved.
# ------------------------------------------------------------------------
##
# Name:
#      cch_helper.sh
##
# Description:
#       A script implemented to delete model for specific target platform
##
# Changelog:

# -  12 Nov 2018 -Naveen Kumar G (zgxxnav)
#       First version

HW_TYPE=`cat /storage/system/config/apos/installation_hw`
is_swm_2_0="/cluster/storage/system/config/apos/swm_version"

campInit() {
    if [[ "$HW_TYPE" == "VM" ]];then
        is_node_active
        node_id=$?

        if [[ $node_id == "1" ]];then        
           `ssh SC-2-1 apg-adm -l IPNAADMBIN`
        else
           `ssh SC-2-2 apg-adm -l IPNAADMBIN`
        fi
        `cmw-model-delete $SDP_VER`
        `cmw-model-done`
        if [[ $node_id == "1" ]];then
           `ssh SC-2-1 apg-adm -u IPNAADMBIN`
        else
           `ssh SC-2-2 apg-adm -u IPNAADMBIN`
        fi
    fi
}

campComplete() {
    /bin/true
}

is_node_active() {

        local nodeA_HAState=""
        local nodeB_HAState=""

        if which immlist &> /dev/null ; then
                if [ -f $is_swm_2_0 ]; then
                        nodeA_HAState=$(immlist "safCSIComp=safComp=apos.haagent\\,safSu=SC-1\\,safSg=2N\\,safApp=ERIC-apg.nbi.aggregation.service,safCsi=apos.haagent,safSi=apg.nbi.aggregation.service-2N-1,safApp=ERIC-apg.nbi.aggregation.service" 2> /dev/null | grep -w "saAmfCSICompHAState" | awk '{ print $3 }')
                        nodeB_HAState=$(immlist "safCSIComp=safComp=apos.haagent\\,safSu=SC-2\\,safSg=2N\\,safApp=ERIC-apg.nbi.aggregation.service,safCsi=apos.haagent,safSi=apg.nbi.aggregation.service-2N-1,safApp=ERIC-apg.nbi.aggregation.service" 2> /dev/null | grep -w "saAmfCSICompHAState" | awk '{ print $3 }')
                else
                        nodeA_HAState=$(immlist "safCSIComp=safComp=Agent\\,safSu=1\\,safSg=2N\\,safApp=ERIC-APG,safCsi=AGENT,safSi=AGENT,safApp=ERIC-APG" 2> /dev/null | grep -w "saAmfCSICompHAState" | awk '{ print $3 }')
                        nodeB_HAState=$(immlist "safCSIComp=safComp=Agent\\,safSu=2\\,safSg=2N\\,safApp=ERIC-APG,safCsi=AGENT,safSi=AGENT,safApp=ERIC-APG" 2> /dev/null | grep -w "saAmfCSICompHAState" | awk '{ print $3 }')
                fi
        fi

        if [ "$nodeA_HAState" == "1" ] ; then
           return 1
        elif [ "$nodeB_HAState" == "1" ] ; then
           return 2
        fi

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

