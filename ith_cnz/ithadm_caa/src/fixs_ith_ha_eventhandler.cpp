/*
 * fixs_ith_ha_eventhandler.cpp
 *
 *  Created on: Jan 25, 2016
 *      Author: xludesi
 */

#include "fixs_ith_logger.h"
#include "fixs_ith_programmacros.h"
#include "fixs_ith_ha_eventhandler.h"
#include "fixs_ith_workingset.h"


int fixs_ith_ha_eventhandler::handle_input (ACE_HANDLE fd )
{
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "HA Task signaled: data ready to be read");

	int call_result = 0;
	uint8_t op = 0;
	ssize_t read_call_result = 0;

	// An HA request arrived from the HA framework. Read and process it !
	if ((read_call_result = ::read(fd, &op, 1)) < 0) { // ERROR: reading a byte from the HA pipe
		int errno_save = errno;

		// I need to check the errno code to view the root cause of the failed read operation
		if ((errno_save == EAGAIN) || (errno_save == EWOULDBLOCK)) { // Data are not yet ready on the pipe
			FIXS_ITH_LOG_ERRNO(errno_save, LOG_LEVEL_WARN, "HA Task: data are not yet ready on the pipe: ignoring this error waiting data to be handled by a thread");
		} else if (errno_save == EINTR) { // WARNING: System call read was interrupted by a signal
			FIXS_ITH_LOG_ERRNO(errno_save, LOG_LEVEL_WARN, "HA Task: '::read' system call interrupted by a signal before any data was read: ignoring this error, maybe the program is ending");
		} else { // ERROR: all other error will return -1
			FIXS_ITH_LOG_ERRNO(errno_save, LOG_LEVEL_ERROR, "Call '::read' failed: reading a code byte from HA Task pipe");
		}
	} else if (read_call_result == 0) {
		// WARNING: reading beyond the end-of-file on the pipe. The pipe seems to be empty; for a NON-BLOCKING pipe this could be normal
		FIXS_ITH_LOG(LOG_LEVEL_WARN, "HA Task: zero bytes read from the pipe, maybe another thread has already handled data: ignoring this warning");
	} else { // OK: Interpret the operation code and act accordingly
		call_result = handle_ha_framework_request(fd, op);
	}

	return call_result;
}


int fixs_ith_ha_eventhandler::handle_ha_framework_request (ACE_HANDLE /*fd*/, int op)
{
	int return_code = 0;

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Processing HA framework request '%c' ... ", op);

	switch (op) {
	case 'A':
		/* START ACTIVE WORK */
		_fixs_ith_server->start_jobs();
		break;
	case 'P':
		/* START PASSIVE WORK */
		if(_fixs_ith_server->is_active())
		{	// transition from ACTIVE to PASSIVE WORK. Stop ITH Server jobs and wait for their termination
			_fixs_ith_server->stop_jobs(true);
		}
		fixs_ith::workingSet_t::instance()->set_program_state(fixs_ith::PROGRAM_STATE_RUNNING_NODE_PASSIVE);
		break;
	case 'S':
		 /* STOP WORKER THREADS */
		// order 'stop' of ITH Server jobs without waiting for their termination.
		// fixs_ith_happmanager::performStateTransition*() methods will wait for jobs termination
		_fixs_ith_server->stop_jobs(false);
		break;
	default:
		/*TODO UNEXPECTED OPERATION*/ break;
	}

	//FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "HA framework request '%c' successfully processed ! ", op);


	return return_code;
}

