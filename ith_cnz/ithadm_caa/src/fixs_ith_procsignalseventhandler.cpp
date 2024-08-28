/*
 * fixs_ith_procsignalseventhandler.cpp
 *
 *  Created on: Feb 3, 2016
 *      Author: xludesi
 */

#include <string.h>

#include "fixs_ith_programconstants.h"
#include "fixs_ith_programmacros.h"
#include "fixs_ith_logger.h"
#include "fixs_ith_cmdoptionparser.h"
#include "fixs_ith_server.h"

#include "fixs_ith_procsignalseventhandler.h"

int fixs_ith_procsignalseventhandler::handle_signal (int signum, siginfo_t * /*siginfo*/, ucontext_t * /*context*/) {
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Signal handler: received signal number %d ('%s')", signum, ::strsignal(signum));

	// Catch termination signals only
	if ((signum == SIGINT) || (signum == SIGTERM)) {
		if (_fixs_ith_server->is_running_in_noha_mode()) { // ITH server started from shell with --noha option enabled
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Signal handler: stopping ITH Server... ");

			// --noha option specified: stop work
			if(_fixs_ith_server->stop_jobs(false))
			{
				//TODO: decide how to handle the error
			}
		} else { // ITH server started by CoreMW in HA mode
			// The ITH server was started by CoreMW cluster. Cluster commands should be used to stop the server.
			FIXS_ITH_LOG(LOG_LEVEL_WARN, "Signal handler: ITH server started by CoreMW in HA mode so SIGINT and SIGTERM signals are ignored! You must use CoreMW (AMF) application management commands to terminate an ITH server running without --noha option");
		}
	} else if (signum == SIGPIPE) {
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Signal handler: signal %d (%s) ignored by ITH server", signum, ::strsignal(signum));
	} else if (signum == SIGHUP) {
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Signal handler: handling of signal %d (%s) NOT YET IMPLEMENTED", signum, ::strsignal(signum));
	}

	return fixs_ith::ERR_NO_ERRORS;
}

