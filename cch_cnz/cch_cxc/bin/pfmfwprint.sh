#!/bin/bash
# ------------------------------------------------------------------------
# Copyright (C) 2012 Ericsson AB. All rights reserved.
# ------------------------------------------------------------------------
##
# Name:
#      pfmfwprint.sh
# Description:
#       A script to wrap the invocation of pfmfwprint the COM CLI.
# Note:
#       None.
##
# Usage:
#       None.
##
# Output:
#       None.
##
# Changelog:
# - 17/10/12 - Andrea Formica (eanform)
#       First version.
##

/usr/bin/sudo /opt/ap/fixs/bin/pfmfwprint "$@"

exit $?

