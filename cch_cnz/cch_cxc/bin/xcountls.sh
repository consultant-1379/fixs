#!/bin/bash
# ------------------------------------------------------------------------
# Copyright (C) 2012 Ericsson AB. All rights reserved.
# ------------------------------------------------------------------------
##
# Name:
#      xcountls.sh
# Description:
#       A script to wrap the invocation of xcountls the COM CLI.
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

umask 002
/usr/bin/sudo /opt/ap/fixs/bin/xcountls "$@"

exit $?

