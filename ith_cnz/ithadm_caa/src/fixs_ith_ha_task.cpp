/*
 * fixs_ith_ha_task.cpp
 *
 *  Created on: Jan 25, 2016
 *      Author: xludesi
 */

#include "fixs_ith_ha_task.h"
#include "fixs_ith_logger.h"
#include "fixs_ith_programmacros.h"
#include <ace/Reactor.h>

fixs_ith_ha_task::fixs_ith_ha_task( fixs_ith_server * fixs_ith_server, ACE_HANDLE ha_pipe_handle)
:_ha_reactor_impl(), _ha_reactor(&_ha_reactor_impl), _ha_evt_handler(fixs_ith_server, ha_pipe_handle), _fixs_ith_server(fixs_ith_server)
{
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Trying to register the HA Task event handler with the HA Task reactor...");

	errno=0;
	if(_ha_reactor.register_handler(&_ha_evt_handler, ACE_Event_Handler::READ_MASK) < 0)
	{ // ERROR: registering the event handler
		int errno_save=errno;
		FIXS_ITH_LOG_ERRNO(errno_save, LOG_LEVEL_ERROR, "Call 'register_handler' failed: cannot register the HA Task event handler with the HA Task reactor");
	}
	else
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "HA Task event handler successfully registered with the HA Task reactor");
}


fixs_ith_ha_task::~fixs_ith_ha_task()
{
	_ha_reactor.remove_handler(&_ha_evt_handler, ACE_Event_Handler::READ_MASK);
	stop();
}


int fixs_ith_ha_task::start()
{
	int retval = 0;

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Starting HA Task ...");

	// Check that the HA pipe event handler is registered onto the HA reactor for READ events
	ACE_Event_Handler * evt_hdlr = 0;
	if(! _ha_reactor.handler(_ha_evt_handler.get_handle(), ACE_Event_Handler::READ_MASK, & evt_hdlr))
	{
		//FIXS_ITH_LOG(LOG_LEVEL_WARN, "HA pipe event handler NOT registered with the HA reactor");

		//FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Trying to add the HA pipe event handler with the HA reactor...");

		errno=0;
		if(_ha_reactor.register_handler(&_ha_evt_handler, ACE_Event_Handler::READ_MASK) < 0)
		{   // ERROR: registering the event handler
			int errno_save=errno;
			FIXS_ITH_LOG_ERRNO(errno_save, LOG_LEVEL_ERROR, "Unable to start HA task ! Call 'register_handler' failed: cannot register the HA Task event handler with the HA Task reactor");
			return -1;
		}
		else
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "HA Task event handler successfully registered with the HA Task reactor");
	}


	// Activate the task, if not already done
	if(!is_active())
	{
		// if the HA reactor was previously ended, reset it before activating the task again !
		if(_ha_reactor.reactor_event_loop_done())
			_ha_reactor.reset_reactor_event_loop();

		int call_result = activate(THR_NEW_LWP | THR_JOINABLE | THR_INHERIT_SCHED);
		if(call_result < 0)
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to start HA task ! 'ACE_Task_Base::activate()' returned < %d >", call_result);
			retval = -2;
		}
		else if(call_result == 0)
			FIXS_ITH_LOG(LOG_LEVEL_INFO, "HA task successfully STARTED.");
		else /* call_result == 1*/
			FIXS_ITH_LOG(LOG_LEVEL_WARN, "HA task already started ! 'ACE_TASK_Base::activate()' returned  < %d >", call_result);
	}

	return retval;
}


int fixs_ith_ha_task::stop()
{
	int retval = 0;

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Stopping HA Task ...");

	if(!is_active())
	{
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "HA Task was already STOPPED !");
	}
	else
	{
		// terminate reactor event loop
		int call_result = _ha_reactor.end_reactor_event_loop();
		if(call_result < 0)
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to stop HA Task ! 'ACE_Reactor::end_reactor_event_loop()' returned < %d >",call_result);
			retval = -1;
		}
		else
		{
			//wait for task termination
			call_result = wait();
			if(call_result == -1)
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "An error occurred while waiting HA Task termination: Call 'ACE_Task_Base::wait()' returned < %d > ! errno == %d", call_result, errno);
				retval = -1;
			}
			else
				FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "HA Task successfully STOPPED.");
		}
	}

	return retval;
}


int fixs_ith_ha_task::svc()
{
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "HA Task thread started");

	FIXS_ITH_LOG(LOG_LEVEL_TRACE, "HA Task thread: entering into the main reactor event loop");
	// Letting this task thread to run the main reactor event loop. This for loop is to prevent errors
	if (_ha_reactor.run_reactor_event_loop()) { // ERROR: entering into the main reactor event loop
		FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "Call 'run_reactor_event_loop' failed: task thread failed to enter into the main reactor event loop");
		::sleep(1);  // TODO: check
	} else {
		FIXS_ITH_LOG(LOG_LEVEL_TRACE, "HA Task thread: out from the main reactor event loop");
		// break;  OK: task thread has ran into the main reactor event loop and now is exiting to stop working
	}

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "HA Task thread exiting from svc function");

	return 0;
}

