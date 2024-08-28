#!/bin/bash
# ------------------------------------------------------------------------
# Copyright (C) 2012 Ericsson AB. All rights reserved.
# ------------------------------------------------------------------------
##
# Name:
#      ipmifwprint.sh
# Description:
#       A script to wrap the invocation of ipmifwprint the COM CLI.
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

/usr/bin/sudo /opt/ap/fixs/bin/ipmifwprint "$@"

exit $?

