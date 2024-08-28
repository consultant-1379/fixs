/*
 * fixs_ith_server.cpp
 *
 * Revision History
 *
 * mm/dd/yyyy	    Author	Description
 *===========================================================
 * 01/26/2016 	   xludesi 	Base version
 * 07/28/2021	   XSIGANO	HZ29732
 * 07/27/2022      ZNITPAR	For the feature "Improvement of SMX configuration lifecycle in AMI",
 *                              Aligning the service startup behaviour in APG restore case with
 *                              that of normal service restart behaviour. Removed fixs_ith_brf_policy_handler
 *                              related function calls during service startup for APG restore. Removed Commented
 *                              line of codes for the code cleanup.
 *
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <syslog.h>

#include <ace/Signal.h>

#include "fixs_ith_programconstants.h"
#include "fixs_ith_programmacros.h"
#include "fixs_ith_logger.h"
#include "fixs_ith_haappmanager.h"
#include "fixs_ith_workingset.h"
#include "common/utility.h"


#include "fixs_ith_server.h"

#if defined (FIXS_ITH_CXC_VER)
#define FIXS_ITH_CXC_VER_STR FIXS_ITH_STRINGIZE(FIXS_ITH_CXC_VER)
#else
#define FIXS_ITH_CXC_VER_STR "UNKNOW"
#endif

#if defined (FIXS_ITH_BUILD)
#define FIXS_ITH_BUILD_STR FIXS_ITH_STRINGIZE(FIXS_ITH_BUILD)
#else
#define FIXS_ITH_BUILD_STR "UNKNOW"
#endif

#define EXIT_PROGRAM_COND(var) ((var) == fixs_ith::PROGRAM_STATE_EXIT_PROGRAM)
#define RESTART_FUNCTION_COND(var) ((var) == fixs_ith::PROGRAM_STATE_RESTART_FUNCTION)
#define BREAK_LOOP_ON(cond) if ((cond)) break
#define BREAK_LOOP_ON_LOG(cond, level) \
	if ((cond)) { \
		FIXS_ITH_LOG(level, "Breaking loop on [" FIXS_ITH_STRINGIZE(cond) "] condition"); \
		break; \
	}

int fixs_ith_server::_proc_signals_to_catch [] = {SIGHUP, SIGINT, SIGPIPE, SIGTERM};


int fixs_ith_server::work ()
{
	int ret_code = fixs_ith::ERR_NO_ERRORS;
	int noha = _cmdoption_parser.noha();

	// Check for multiple program instance running, but only when running in no HA mode
	if (noha && (ret_code = multiple_process_instance_running_check()))
		return ret_code;

	if (noha)
	{ 	// --noha option provided: ITH server is running in no-HA mode [DEBUG mode]
		fixs_ith_logger::open(FIXS_ITH_LOGGER_APPENDER_NAME);

		FIXS_ITH_LOG(LOG_LEVEL_INFO, "ITH Server started [ DEBUG MODE ]");

		// Create the ITH Server working set and initialize the program state
		fixs_ith::workingSet_t::instance()->set_program_state(fixs_ith::PROGRAM_STATE_RUNNING);

		// Set the signals handler
		if(init_signals_handler()) {
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'init_signals_handler' failed: process signal handler initialization failure: ITH server will retry asap to reinitialize signals handler");
		}

		// starts ITH Server activities
		start_jobs();

		// also this thread will enter into the ITH Server reactor event loop
		enter_reactor_event_loop();

		// we should be here only if a SIGTERM or SIGINT signal has been received from ITH Server running in DEBUG mode

		// wait ITH server scheduler termination
		fixs_ith::workingSet_t::instance()->get_main_scheduler()->wait();

		// wait ITH server jobs termination
		wait_and_release_jobs_resources();
	}
	else
	{ 	// ITH server started by the HA framework [HA Mode]
		// Demonize the ITH Server and prepare for communication with the HA framework
		pid_t parent_pid = ::getpid();
		fixs_ith_haappmanager ha_application_manager(FIXS_ITH_HA_DAEMON_NAME, this);

		_ha_app_manager_ptr = &ha_application_manager;

		// Initialize the ITH Server logger. This operation MUST be executed after demonization
		// because TRA library creates a thread to monitor "log4cplus.properties' file
		fixs_ith_logger::open(FIXS_ITH_LOGGER_APPENDER_NAME);

		// Check demonization result
		pid_t child_pid = ::getpid();
		if (child_pid != parent_pid) { // OK: ITH server successfully demonized
			FIXS_ITH_LOG(LOG_LEVEL_INFO, "ITH Server started (PID = %d)", child_pid);
		} else { // ERROR: ITH server was not demonized
			FIXS_ITH_LOG(LOG_LEVEL_WARN,
					"ITH server was not demonized correctly: child process was aborted on creation: the parent process (PID == %d) continues taking the control",
					parent_pid);
		}

		// Create the ITH Server working set and initialize the program state
		fixs_ith::workingSet_t::instance()->set_program_state(fixs_ith::PROGRAM_STATE_RUNNING);

		// Set the signals handler
		if(init_signals_handler()) {
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'init_signals_handler' failed: process signal handler initialization failure: ITH server will retry asap to reinitialize signals handler");
		}

		// Start the HA task (responsible for processing the HA framework requests)
		ha_application_manager.start_ha_task();

		// Activate communication with the HA framework (blocking call)
		ACS_APGCC_HA_ReturnType ha_call_result = ha_application_manager.activate();

		switch (ha_call_result) {
		case ACS_APGCC_HA_SUCCESS:
			FIXS_ITH_SYSLOG(LOG_DEBUG, LOG_LEVEL_DEBUG, "HA Application Gracefully closing...");
			break;
		case ACS_APGCC_HA_FAILURE:
			FIXS_ITH_SYSLOG(LOG_ERR, LOG_LEVEL_ERROR, "HA Activation Failed: ha_call_result == %d", ha_call_result);
			ret_code = fixs_ith::ERR_HA_FAILURE;
			break;
		case ACS_APGCC_HA_FAILURE_CLOSE:
			FIXS_ITH_SYSLOG(LOG_ERR, LOG_LEVEL_ERROR, "HA Application Failed to Gracefully closing: ha_call_result == %d", ha_call_result);
			ret_code = fixs_ith::ERR_HA_FAILURE;
			break;
		default:
			FIXS_ITH_SYSLOG(LOG_WARNING, LOG_LEVEL_WARN, "HA Application error code unknown: ha_call_result == %d", ha_call_result);
			ret_code = fixs_ith::ERR_HA_FAILURE;
			break;
		}

		_ha_app_manager_ptr = 0;

		// stop the HA Task
		ha_application_manager.stop_ha_task();
	}

	// Reset the signals handler
	if (reset_signals_handler()) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'reset_signals_handler' failed: resetting signals handler");
	}

	// Delete lock file
	if(noha && (::unlink(FIXS_ITH_SERVER_LOCK_FILE_PATH) < 0))
		FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "Call '::unlink' failed: main thread failed to delete the ITH server lock file '%s'", FIXS_ITH_SERVER_LOCK_FILE_PATH);

	if(!noha && (ret_code == fixs_ith::ERR_NO_ERRORS))
		FIXS_ITH_SYSLOG(LOG_DEBUG, LOG_LEVEL_DEBUG, "HA Application Gracefully closed.");

	FIXS_ITH_LOG(LOG_LEVEL_INFO, "ITH Server exiting");

	// Finalize the ITH Server logger
	fixs_ith_logger::close();

	return ret_code;
}


int fixs_ith_server::start_jobs()
{
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Starting ITH Server jobs (CXC Version == <%s>, BUILD == <%s>) ...",
			FIXS_ITH_CXC_VER_STR, FIXS_ITH_BUILD_STR);

	fixs_ith::workingSet_t::instance()->set_program_state(fixs_ith::PROGRAM_STATE_RUNNING_NODE_ACTIVE);

	if(thr_count() > 0)
	{
		FIXS_ITH_LOG(LOG_LEVEL_WARN, "... ITH Server jobs already started !");
		return fixs_ith::ERR_NO_ERRORS;
	}

#ifdef FIXS_ITH_EXECUTE_TESTS
	execute_tests();
#endif

        //Create the directory for configuration patches
	fixs_ith::workingSet_t::instance()->createPatchDirectory();

	/* PHASE 1: PREPARE for task activation  ... */

	// Initialize Operation Dispatcher
	if(fixs_ith::workingSet_t::instance()->start_main_scheduler() != fixs_ith::ERR_NO_ERRORS)
	{
		FIXS_ITH_LOG(LOG_LEVEL_FATAL, "Cannot initialize the Operation Dispatcher thread");
		return fixs_ith::ERR_START_SCHEDULER;
	}

	// Subscribe to CS to receive HWC table changes notifications
	fixs_ith::workingSet_t::instance()->schedule_until_completion(operation::CS_SUBSCRIBE,0,3);

	// Subscribe to TRAPD  to receive SNMP TRAPS
	fixs_ith::workingSet_t::instance()->schedule_until_completion(operation::TRAPDS_SUBSCRIBE,0,3);

	// Register IMM implementer (note that when such operation is execute, it also orders a RELOAD CONTEXT FROM IMM operation)
	fixs_ith::workingSet_t::instance()->schedule_until_completion(operation::REGISTER_IMM_IMPLEMENTER,0,3);

	// Register BRF Backup participant implementer
	fixs_ith::workingSet_t::instance()->schedule_until_completion(operation::REGISTER_TO_BRF,0,3);

	// Schedule SMX-APG Network Configuration Consistency check for all switch boards
        fixs_ith::workingSet_t::instance()->schedule_until_completion(operation::CHECK_CONFIG_CONSISTENCY, 300, 300, "", 1);

	// Reset the ITH main reactor event loop, so that "run_reactor_event_loop()" can be executed again by ITH worker threads
	fixs_ith::workingSet_t::instance()->get_main_reactor().reset_reactor_event_loop();

	/* PHASE 2: ACTIVATE the task */

	int activate_res = activate(THR_NEW_LWP | THR_JOINABLE | THR_INHERIT_SCHED, FIXS_ITH_SERVER_N_THREADS);
	if (activate_res) { // ERROR
		FIXS_ITH_LOG(LOG_LEVEL_FATAL, "Call 'activate' failed: cannot start worker threads !");
		return fixs_ith::ERR_START_JOBS;
	}

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, " ... OK ! %d worker threads started successfully", FIXS_ITH_SERVER_N_THREADS); // OK

	return fixs_ith::ERR_NO_ERRORS;
}

int fixs_ith_server::stop_jobs(bool wait_termination)
{
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Stopping  ITH Server jobs ...");

	int return_code = fixs_ith::ERR_NO_ERRORS;

	if(thr_count() == 0)
	{
		FIXS_ITH_LOG(LOG_LEVEL_WARN, "... ITH Server jobs already stopped !");
		return fixs_ith::ERR_NO_ERRORS;
	}

	// Save the current program state to decide what to do after
	fixs_ith::ProgramStateConstants program_state_save = fixs_ith::workingSet_t::instance()->get_program_state();

	// Signal we're going to stop working
	fixs_ith::workingSet_t::instance()->set_program_state(fixs_ith::PROGRAM_STATE_STOP_WORKING);

	switch (program_state_save) {
	case fixs_ith::PROGRAM_STATE_RUNNING_NODE_ACTIVE:
	{
		// Unsubscribe from CS for HWC table changes notifications
		fixs_ith::workingSet_t::instance()->schedule(operation::CS_UNSUBSCRIBE);

		// Unsubscribe from TRAPDS
		fixs_ith::workingSet_t::instance()->schedule(operation::TRAPDS_UNSUBSCRIBE);

		// Unregister from BRF
		fixs_ith::workingSet_t::instance()->schedule(operation::UNREGISTER_FROM_BRF);

		// Stop Context schedulers (and always wait for termination when running in HA mode)
		if(fixs_ith::workingSet_t::instance()->stop_context_schedulers(is_running_in_noha_mode() ? wait_termination: true) != fixs_ith::ERR_NO_ERRORS)
		{
			FIXS_ITH_LOG(LOG_LEVEL_FATAL, "Cannot stop the Context Dispatcher threads");
			return_code = fixs_ith::ERR_STOP_SCHEDULER;
		}

		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Scheduling STOP operation");

		// STOP Operation scheduled
		fixs_ith::workingSet_t::instance()->schedule(operation::STOP);

		// Stop Operation Dispatcher (and always wait for termination when running in HA mode)
		if(fixs_ith::workingSet_t::instance()->stop_main_scheduler(is_running_in_noha_mode() ? wait_termination: true) != fixs_ith::ERR_NO_ERRORS)
		{
			FIXS_ITH_LOG(LOG_LEVEL_FATAL, "Cannot stop the Operation Dispatcher thread");
			return_code = fixs_ith::ERR_STOP_SCHEDULER;
		}

	}
	break;
	case fixs_ith::PROGRAM_STATE_RUNNING_NODE_PASSIVE:
		FIXS_ITH_LOG(LOG_LEVEL_WARN, " ... ITH Server is running on the PASSIVE node. No jobs to be stopped !");
		break;
	case fixs_ith::PROGRAM_STATE_RUNNING_NODE_UNDEFINED:
		FIXS_ITH_LOG(LOG_LEVEL_WARN, " ... ITH Server is running on a node whose state is UNDEFINED. No action !");
		break;
	default: break;
	}

	// Stop the task, by ending the main reactor event loop
	ACE_Reactor & ith_main_reactor = fixs_ith::workingSet_t::instance()->get_main_reactor();
	if (!ith_main_reactor.reactor_event_loop_done())
	{
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Ending the main reactor event loop...");
		ith_main_reactor.end_reactor_event_loop();
	}

	// If requested, wait for task termination
	if(wait_termination)
	{
		return_code = wait_and_release_jobs_resources();
	}
        // TODO: ??? In case of errors reset the program state to the previous state
	if(return_code)
		fixs_ith::workingSet_t::instance()->set_program_state(program_state_save);

	return return_code;
}

int fixs_ith_server::svc () {
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Task thread started running svc function: threads in the main task == %zu", this->thr_count());

	// Check the exit condition. If it's false this task thread will enter into the reactor loop
	fixs_ith::ProgramStateConstants program_state = fixs_ith::workingSet_t::instance()->get_program_state();
	ACE_Reactor & ith_main_reactor = fixs_ith::workingSet_t::instance()->get_main_reactor();
	for (;;) {
		BREAK_LOOP_ON_LOG(EXIT_PROGRAM_COND(program_state), LOG_LEVEL_DEBUG);

		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Task thread: entering into the main reactor event loop");
		// Letting this task thread to run the main reactor event loop. This for loop is to prevent errors
		if (ith_main_reactor.run_reactor_event_loop()) { // ERROR
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "Call 'run_reactor_event_loop' failed: task thread failed to enter into the main reactor event loop");
			::sleep(FIXS_ITH_TASK_ENTER_REACTOR_LOOP_DELAY);
		} else {
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Task thread: out from the main reactor event loop");
			break; // OK: task thread has ran into the main reactor event loop and now is exiting to stop working
		}
	}

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Task thread exiting from svc function");

	return fixs_ith::ERR_NO_ERRORS;
}

int fixs_ith_server::multiple_process_instance_running_check () {
	// Multiple server instance check: if there is another ITH server instance
	// already running then exit immediately

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Checking ITH server multiple instance running...");
	int lock_fd = ::open(FIXS_ITH_SERVER_LOCK_FILE_PATH, O_CREAT | O_WRONLY | O_APPEND, 0664);
	if (lock_fd < 0) {
		FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "Call '::open' failed");
		return fixs_ith::PROGRAM_EXIT_LOCK_FILE_OPEN_ERROR;
	}
	errno = 0;
	if (::flock(lock_fd, LOCK_EX | LOCK_NB) < 0) {
		int errno_save = errno;
		if (errno_save == EWOULDBLOCK) {
			::fprintf(::stderr, "Another ITH Server instance running\n");
			FIXS_ITH_LOG_ERRNO(errno_save, LOG_LEVEL_ERROR, "Another ITH Server instance running found");
			return fixs_ith::PROGRAM_EXIT_ANOTHER_SERVER_RUNNING;
		}

		FIXS_ITH_LOG_ERRNO(errno_save, LOG_LEVEL_ERROR, "Call '::flock' failed");
		return fixs_ith::PROGRAM_EXIT_LOCK_FILE_LOCKING_ERROR;
	}

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "No ITH server multiple instance running");

	return fixs_ith::ERR_NO_ERRORS;
}


int fixs_ith_server::init_signals_handler()
{
	int return_code = fixs_ith::ERR_NO_ERRORS;

	// Adding the process signal handler for the signals ITH server has to catch
	for (size_t sig_index = 0; sig_index < FIXS_ITH_ARRAY_SIZE(_proc_signals_to_catch); ++sig_index) {
		int signal = _proc_signals_to_catch[sig_index];

		// Try to add the process signal handler for signal
		if (!_proc_catched_signals.is_member(signal)) {
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "registering the handler for the signal %d (%s)", signal, ::strsignal(signal));

			// signal not present, adding it
			errno = 0;
			if (fixs_ith::workingSet_t::instance()->get_main_reactor().register_handler(signal, &_proc_signals_event_handler) < 0) { //ERROR: adding the signal handler
				return_code |= fixs_ith::ERR_REACTOR_HANDLER_REGISTER_FAILURE;
				FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "Call 'register_handler' failed: cannot register the handler for the signal %d (%s)", signal, ::strsignal(signal));
			} else _proc_catched_signals.sig_add(signal); //OK: remember this signal was added.
		}
	}

	return return_code;
}


int fixs_ith_server::reset_signals_handler()
{
	int return_code = fixs_ith::ERR_NO_ERRORS;

	// Removing the process signal handling
	for (size_t sig_index = 0; sig_index < FIXS_ITH_ARRAY_SIZE(_proc_signals_to_catch); ++sig_index) {
		int signal = _proc_signals_to_catch[sig_index];

		// Try to remove the process signal handler for signal
		if (_proc_catched_signals.is_member(signal)) {
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "removing  the handler for signal %d (%s)", signal, ::strsignal(signal));

			// signal present, removing it
			errno = 0;
			if (fixs_ith::workingSet_t::instance()->get_main_reactor().remove_handler(signal, reinterpret_cast<ACE_Sig_Action *>(0)) < 0) { //ERROR: removing the signal handler
				return_code |= fixs_ith::ERR_REACTOR_HANDLER_REMOVE_FAILURE;
				FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "Call 'remove_handler' failed: cannot remove the handler for the signal %d (%s)", signal, ::strsignal(signal));
			} else _proc_catched_signals.sig_del(signal); //OK: remember this signal was removed.
		}
	}

	return return_code;
}


void fixs_ith_server::enter_reactor_event_loop()
{
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Main thread: entering into the main reactor event loop");

	//TODO: handle errors
	fixs_ith::workingSet_t::instance()->get_main_reactor().run_reactor_event_loop();

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Main thread: out from the main reactor event loop");

}

int fixs_ith_server::wait_and_release_jobs_resources()
{
	int ret_code = fixs_ith::ERR_NO_ERRORS;

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, " ... waiting for ITH server jobs termination ");

	// wait for ITH Server jobs termination
	if(!is_active())
	{
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, " ... ITH Server jobs already terminated. No wait");
	}
	else if(wait() != 0)
	{
		FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "Error while waiting for ITH Server jobs termination !");
		ret_code = fixs_ith::ERR_WAIT_JOBS;
	}
	else
	{
		// OK, ITH Server jobs terminated !
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, " ... OK ! %d worker threads terminated", FIXS_ITH_SERVER_N_THREADS);
	}

	// if ITH Server jobs are not active, cancel all pending timers on main reactor
	if(ret_code == fixs_ith::ERR_NO_ERRORS)
	{
		// Cancel pending timers on the main reactor
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, " ... canceling pending timers on the main reactor ");
		int close_res = fixs_ith::workingSet_t::instance()->get_main_reactor().timer_queue()->close();
		if(close_res != 0 )
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, " ... unable to cancel pending timers on the main reactor. close() result : %d ", close_res);
			ret_code = fixs_ith::ERR_REACTOR_TIMER_QUEUE_CLOSE_FAILURE;
		}
		else {
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, " ... pending timers successfully canceled");
		}
	}

	return ret_code;
}

int
fixs_ith_server::execute_tests () {
	if (!_cmdoption_parser.noha()) return fixs_ith::ERR_NO_ERRORS;

	int call_result = fixs_ith::ERR_NO_ERRORS;
	const char * const sb_key = "15.0.0.0_0";

	fixs_ith_switchboardinfo switchboard_info;
	call_result = fixs_ith::workingSet_t::instance()->get_sbdatamanager().get_switchboard_info(switchboard_info, sb_key);

	if (call_result != fixs_ith_sbdatamanager::SBM_OK) {
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "<<TEST>>: Call 'get_switchboard_info()' failed: sb_key == <%s>", sb_key);
		return -1;
	}

	std::vector<bool> if_mau_type;
	uint16_t if_index = 131;

	call_result = fixs_ith::workingSet_t::instance()->get_snmpmanager().get_if_mau_type(
			if_mau_type, if_index, switchboard_info, fixs_ith::TRANSPORT_PLANE);

	if (call_result) { // ERROR
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "<<TEST>>: Call 'get_if_mau_type()' failed: sb_key == <%s>: if_index == <%u>", sb_key, if_index);
		return -1;
	}

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "<<TEST>>: if_mau_type.size() == <%lu>", if_mau_type.size());

	for (size_t i = 0; i < if_mau_type.size(); ++i) {
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "<<TEST>>: if_mau_type[%lu] == <%u>", i, static_cast<unsigned>(if_mau_type[i]));
	}



	std::vector<oid> if_mau_default_type;

	call_result = fixs_ith::workingSet_t::instance()->get_snmpmanager().get_if_mau_default_type(
			if_mau_default_type, if_index, switchboard_info, fixs_ith::TRANSPORT_PLANE);

	if (call_result) { // ERROR
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "<<TEST>>: Call 'get_if_mau_default_type()' failed: sb_key == <%s>: if_index == <%u>", sb_key, if_index);
		return -1;
	}

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "<<TEST>>: if_mau_default_type.size() == <%lu>", if_mau_default_type.size());

	for (size_t i = 0; i < if_mau_default_type.size(); ++i) {
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "<<TEST>>: if_mau_default_type[%lu] == <%lu>", i, if_mau_default_type[i]);
	}



	int if_mau_neg_admin_status = 0;

	call_result = fixs_ith::workingSet_t::instance()->get_snmpmanager().get_if_mau_auto_neg_admin_status(if_mau_neg_admin_status,
			if_index, switchboard_info, fixs_ith::TRANSPORT_PLANE);

	if (call_result) { // ERROR
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "<<TEST>>: Call 'get_if_mau_auto_neg_admin_status()' failed: sb_key == <%s>: if_index == <%u>", sb_key, if_index);
		return -1;
	}

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "<<TEST>>: if_mau_neg_admin_status == <%d>", if_mau_neg_admin_status);



	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "<<TEST>>: SET IF MAU DEFAULT TYPE in progress: default_type == <%u>: if_index == <%u>",
			fixs_ith_snmp::IF_MAU_TYPE_40G_BASE_SR4, if_index);

	call_result = fixs_ith::workingSet_t::instance()->get_snmpmanager().set_if_mau_default_type(fixs_ith_snmp::IF_MAU_TYPE_40G_BASE_SR4,
			if_index, switchboard_info, fixs_ith::TRANSPORT_PLANE);

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "<<TEST>>: SET IF MAU DEFAULT TYPE: call_result == <%d>", call_result);



	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "<<TEST>>: SET IF MAU AUTO NEG STATUS in progress: status == <%u>: if_index == <%u>",
			fixs_ith_snmp::FIXS_ITH_MAU_AUTO_NEG_STATUS_DISABLED, if_index);

	call_result = fixs_ith::workingSet_t::instance()->get_snmpmanager().set_if_mau_auto_neg_admin_status(
			fixs_ith_snmp::FIXS_ITH_MAU_AUTO_NEG_STATUS_DISABLED, if_index, switchboard_info, fixs_ith::TRANSPORT_PLANE);

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "<<TEST>>: SET IF MAU AUTO NEG STATUS: call_result == <%d>", call_result);

	return fixs_ith::ERR_NO_ERRORS;
}
