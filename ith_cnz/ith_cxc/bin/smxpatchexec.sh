#!/bin/bash
##
# ------------------------------------------------------------------------
#     Copyright (C) 2012 Ericsson AB. All rights reserved.
# ------------------------------------------------------------------------
##
# Name:
#       smxpatchexec.sh
# Description:
#       A script to wrap the invocation of smxpatchexec.
# Note:
#	None.
##
# Usage:
#	None.
##
# Output:
#       None.
##
# Changelog:
# - Tue Oct 25 2016 - Stefano Volpe
#	First version.
##

/usr/bin/sudo /opt/ap/fixs/bin/smxpatchexec "$@"

exit $?
