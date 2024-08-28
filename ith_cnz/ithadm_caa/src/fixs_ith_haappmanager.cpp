/*
 * fixs_ith_haappmanager.cpp
 *
 *  Created on: Jan 22, 2016
 *      Author: xludesi
 */

#include <syslog.h>

#include "fixs_ith_programconstants.h"
#include "fixs_ith_logger.h"
#include "fixs_ith_programmacros.h"
#include "fixs_ith_ha_task.h"
#include "fixs_ith_workingset.h"
#include "fixs_ith_haappmanager.h"


fixs_ith_haappmanager::fixs_ith_haappmanager (fixs_ith_server * fixs_ith_server)
: ACS_APGCC_ApplicationManager(),_ha_task(0), _fixs_ith_server(fixs_ith_server)
{
	_ha_pipe[0] = _ha_pipe[1] = ACE_INVALID_HANDLE;
	_initialized = (init() == 0);
}

fixs_ith_haappmanager::fixs_ith_haappmanager(const char * daemon_name, fixs_ith_server * fixs_ith_server)
: ACS_APGCC_ApplicationManager(daemon_name),_ha_task(0), _fixs_ith_server(fixs_ith_server)
{
	_ha_pipe[0] = _ha_pipe[1] = ACE_INVALID_HANDLE;
	_initialized = (init() == 0);
}

fixs_ith_haappmanager::fixs_ith_haappmanager(const char * daemon_name, const char * username, fixs_ith_server * fixs_ith_server)
: ACS_APGCC_ApplicationManager(daemon_name, username),_ha_task(0), _fixs_ith_server(fixs_ith_server)
{
	_ha_pipe[0] = _ha_pipe[1] = ACE_INVALID_HANDLE;
	_initialized = (init() == 0);
}


int fixs_ith_haappmanager::start_ha_task()
{
	if(!_initialized) _initialized = (init() == 0);

	return _initialized ? _ha_task->start(): -1;
}

int fixs_ith_haappmanager::stop_ha_task()
{
	return _initialized ? _ha_task->stop(): -1;
}


fixs_ith_haappmanager::~fixs_ith_haappmanager()
{
	if(_ha_task)
	{
		_ha_task->stop();
		delete _ha_task;
	}
}


int fixs_ith_haappmanager::init()
{
	int return_code = 0;

	// create HA pipe if not already done
	if ((_ha_pipe[0] == ACE_INVALID_HANDLE) && !::pipe(_ha_pipe))
	{
		::syslog(LOG_DEBUG, "HA pipe initialized successfully");
	}
	else
	{
		_ha_pipe[0] = _ha_pipe[1] = ACE_INVALID_HANDLE;
		::syslog(LOG_ERR,"HA pipe: initialization failed: call '::pipe' failed. ERRNO = %d", errno);
		//return_code = fixs_ith::ERR_PIPE_CALL;
		return_code = -1;
	}

	if(!return_code)
	{
		// Set NON blocking mode on both ends of the pipe (reader/writer)
		if (::fcntl(_ha_pipe[0], F_SETFL, O_NONBLOCK) == -1) { // ERROR: Setting non block flag on the reader end of the pipe
			::syslog(LOG_ERR,"HA operation pipe: fcntl of non-block flag failed on reader part: call '::fcntl' failed. ERRNO = %d", errno);
			//return_code = fixs_ith::ERR_FCNTL_CALL;
			return_code = -2;
		} else
			::syslog(LOG_DEBUG,"HA pipe: non blocking flag successfully set on the reader part");

		if (::fcntl(_ha_pipe[1], F_SETFL, O_NONBLOCK) == -1) { // ERROR: Setting non block flag on the writer end of the pipe
			::syslog(LOG_ERR,"HA pipe: fcntl of non-block flag failed on writer part: call '::fcntl' failed. ERRNO = %d", errno);
			//return_code = acs_apbm::ERR_FCNTL_CALL;
			return_code = -3;
		} else
			::syslog(LOG_DEBUG,"HA pipe: non blocking flag successfully set on the writer part");
	}

	// create HA task, if not already done
	if(!return_code && !_ha_task)
	{
		_ha_task = new (std::nothrow) fixs_ith_ha_task(_fixs_ith_server, _ha_pipe[0]);
		if(!_ha_task)
		{
			::syslog(LOG_ERR,"Unable to allocate HA task on the HEAP");
			return_code = -4;
		}
		else
		{
			::syslog(LOG_DEBUG,"HA task successfully allocated on the HEAP");
		}
	}

/*
	// activate HA task, if not already done
	if(!return_code && !_ha_task->is_active())
	{
		int call_result = _ha_task->start();
		if(call_result < 0)
		{
			::syslog(LOG_ERR,"Unable to activate the HA task");
			return_code = -5;
		}
		else
		{
			::syslog(LOG_DEBUG,"HA task successfully activated");
		}
	}
*/
	return return_code;
}


ACS_APGCC_ReturnType fixs_ith_haappmanager::performStateTransitionToActiveJobs (ACS_APGCC_AMF_HA_StateT previous_state)
{
	FIXS_ITH_SYSLOG(LOG_DEBUG, LOG_LEVEL_DEBUG, "--- Requested transition to ACTIVE state ---");

	/* Check if we have received the ACTIVE State again.
	 * This means that, our application is already Active and
	 * again we have got a callback from AMF to go active.
	 * Ignore this case anyway. This case should rarely happen
	 */
	if (previous_state == ACS_APGCC_AMF_HA_ACTIVE) return ACS_APGCC_SUCCESS;

	/* Send HA request to the HA Task */
	ACE_TCHAR state[1] = {'A'};
	::write(_ha_pipe[1], &state, sizeof(state));

	FIXS_ITH_SYSLOG(LOG_DEBUG, LOG_LEVEL_DEBUG, "--- transition completed. Returning '%d' response to HA framework  ---", ACS_APGCC_SUCCESS);

	return ACS_APGCC_SUCCESS;
}

ACS_APGCC_ReturnType fixs_ith_haappmanager::performStateTransitionToPassiveJobs (ACS_APGCC_AMF_HA_StateT previous_state)
{
	FIXS_ITH_SYSLOG(LOG_DEBUG, LOG_LEVEL_DEBUG, "--- Requested transition to STANDBY state ---");

	/* Check if we have received the PASSIVE State Again.
	 * This means that, our application was already Passive and
	 * again we have got a callback from AMF to go passive.
	 * Ignore this case anyway. This case should rarely happen.
	 */
	if (previous_state == ACS_APGCC_AMF_HA_STANDBY) return ACS_APGCC_SUCCESS;

	/* Send HA request to the HA Task */
	ACE_TCHAR state[1] = {'P'};
	::write(_ha_pipe[1], &state, sizeof(state));

	FIXS_ITH_SYSLOG(LOG_DEBUG, LOG_LEVEL_DEBUG, "--- transition completed. Returning '%d' response to HA framework  ---", ACS_APGCC_SUCCESS);

	return ACS_APGCC_SUCCESS;
}

ACS_APGCC_ReturnType fixs_ith_haappmanager::performStateTransitionToQueisingJobs (ACS_APGCC_AMF_HA_StateT previous_state)
{
	FIXS_ITH_SYSLOG(LOG_DEBUG, LOG_LEVEL_DEBUG, "--- Requested transition to QUIESCING state ---");

	/* When an Active application loses its assignment (due to SI/SU SHUTDOWN operation),
	 *  AMF despatches QUEISCING state to application by invoking
	 *  performStateTransitionToQueiscingJobs() API
	 */
	if (previous_state == ACS_APGCC_AMF_HA_QUIESCING) return ACS_APGCC_SUCCESS;

	/* Send HA request to the HA Task */
	ACE_TCHAR state[1] = {'S'};
	::write(_ha_pipe[1], &state, sizeof(state));

	// wait for ITH Server jobs termination and release jobs resources
	_fixs_ith_server->wait_and_release_jobs_resources();

	FIXS_ITH_SYSLOG(LOG_DEBUG, LOG_LEVEL_DEBUG, "--- transition completed. Returning '%d' response to HA framework  ---", ACS_APGCC_SUCCESS);

	return ACS_APGCC_SUCCESS;
}

ACS_APGCC_ReturnType fixs_ith_haappmanager::performStateTransitionToQuiescedJobs (ACS_APGCC_AMF_HA_StateT previous_state)
{
	FIXS_ITH_SYSLOG(LOG_DEBUG, LOG_LEVEL_DEBUG, "--- Requested transition to QUIESCED state ---");

	/* An Active application when loses its assignment (due to SI/SU LOCK
	 * operation), AMF despatches QUEISCED state to application by invoking
	 * performStateTransitionToQueiscedJobs() API
	*/

	// If we are in the same previous state then nothing will be done
	if (previous_state == ACS_APGCC_AMF_HA_QUIESCED) return ACS_APGCC_SUCCESS;

	/* Send HA request to the HA Task */
	ACE_TCHAR state[1] = {'S'};
	::write(_ha_pipe[1], &state, sizeof(state));

	// wait for ITH Server jobs termination and release jobs resources
	_fixs_ith_server->wait_and_release_jobs_resources();

	fixs_ith::workingSet_t::instance()->set_program_state(fixs_ith::PROGRAM_STATE_RUNNING_QUIESCED);

	FIXS_ITH_SYSLOG(LOG_DEBUG, LOG_LEVEL_DEBUG, "--- transition completed. Returning '%d' response to HA framework  ---", ACS_APGCC_SUCCESS);

	return ACS_APGCC_SUCCESS;
}

ACS_APGCC_ReturnType fixs_ith_haappmanager::performComponentHealthCheck ()
{
	//FIXS_ITH_SYSLOG(LOG_DEBUG, LOG_LEVEL_DEBUG, "Callback performStateTransitionToQuiescedJobs invoked");

	return ACS_APGCC_SUCCESS;
}

ACS_APGCC_ReturnType fixs_ith_haappmanager::performComponentTerminateJobs ()
{
	FIXS_ITH_SYSLOG(LOG_DEBUG, LOG_LEVEL_DEBUG, "--- Requested Application TERMINATE ---");

	/* performComponentTerminateJobs() is invoked by AMF on issuing
	 * Lock-Instantiation operation on the SU of an application.
	 */

	/* Send HA request to the HA Task */
	ACE_TCHAR state[1] = {'S'};
	::write(_ha_pipe[1], &state, sizeof(state));

	// wait for ITH Server jobs termination
	_fixs_ith_server->wait_and_release_jobs_resources();

	fixs_ith::workingSet_t::instance()->set_program_state(fixs_ith::PROGRAM_STATE_EXIT_PROGRAM);

	FIXS_ITH_SYSLOG(LOG_DEBUG, LOG_LEVEL_DEBUG, "--- transition completed. Returning '%d' response to HA framework  ---", ACS_APGCC_SUCCESS);

	return ACS_APGCC_SUCCESS;
}

ACS_APGCC_ReturnType fixs_ith_haappmanager::performComponentRemoveJobs ()
{
	FIXS_ITH_SYSLOG(LOG_DEBUG, LOG_LEVEL_DEBUG, "--- Removed Application assignment ---");

	/* performComponentRemoveJobs() is invoked by AMF to remove the current assignment on the application. */

	/* Send HA request to the HA Task */
	ACE_TCHAR state[1] = {'S'};
	::write(_ha_pipe[1], &state, sizeof(state));

	// wait for ITH Server jobs termination
	_fixs_ith_server->wait_and_release_jobs_resources();

	fixs_ith::workingSet_t::instance()->set_program_state(fixs_ith::PROGRAM_STATE_RUNNING_NODE_UNDEFINED);

	FIXS_ITH_SYSLOG(LOG_DEBUG, LOG_LEVEL_DEBUG, "--- transition completed. Returning '%d' response to HA framework  ---", ACS_APGCC_SUCCESS);

	return ACS_APGCC_SUCCESS;
}

ACS_APGCC_ReturnType fixs_ith_haappmanager::performApplicationShutdownJobs ()
{
	/*  performApplicationShutdownJobs() method is invoked when the cleanup
	 *	clc_script operation is performed on the application during the
	 *	componentErrorReport API invocation by the application with the recovery
	 *	option as COMPONENT RESTART and during shutdown of the node.
	*/

	FIXS_ITH_SYSLOG(LOG_DEBUG, LOG_LEVEL_DEBUG, "--- Requested Application SHUTDOWN ---");

	/* Send HA request to the HA Task */
	ACE_TCHAR state[1] = {'S'};
	::write(_ha_pipe[1], &state, sizeof(state));

	// wait for ITH Server jobs termination
	_fixs_ith_server->wait_and_release_jobs_resources();

	fixs_ith::workingSet_t::instance()->set_program_state(fixs_ith::PROGRAM_STATE_EXIT_PROGRAM);

	FIXS_ITH_SYSLOG(LOG_DEBUG, LOG_LEVEL_DEBUG, "--- transition completed. Returning '%d' response to HA framework  ---", ACS_APGCC_SUCCESS);

	return ACS_APGCC_SUCCESS;
}



