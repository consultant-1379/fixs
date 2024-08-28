#!/bin/bash
##
# ------------------------------------------------------------------------
#       Copyright (C) 2013 Ericsson AB. All rights reserved.
# ------------------------------------------------------------------------
##
# Name:
#       fixerls.sh
# Description:
#       A script to wrap the invocation of fixerls from the COM CLI.
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
#   Tue Aug 23 2013 - Thanh Nguyen (xdtthng)
#   First version.
##

/usr/bin/sudo /opt/ap/fixs/bin/fixerls "$@"

exit $?
