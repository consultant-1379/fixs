#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

#include <ace/Future.h>

#include "operation/operation.h"
#include "operation/operationbase.h"
#include "operation/creator.h"
#include "operation/scheduler.h"
#include "fixs_ith_logger.h"
#include "fixs_ith_programmacros.h"
#include "fixs_ith_csreader.h"
#include "fixs_ith_cshwctablechangeobserver.h"
#include "fixs_ith_trapdsobserver.h"
#include "fixs_ith_programconstants.h"
#include "fixs_ith_workingset.h"
#include "operation/timer_handler.h"


//FIXS_ITH_TRACE_DEFINE(FIXS_ITH_workingset)


namespace fixs_ith
{
	workingset::workingset()
	:_main_reactor_impl(),
	 _main_reactor(&_main_reactor_impl),
	 _cs_reader(),
	 _cs_hwc_table_change_observer(),
	 _trapds_observer(),
	 _snmpmanager(),
	 _netconfmanager(),
	 _transportBNChandler(),
	 _sbdataManager(),
	 m_immHandler(),
	 _main_scheduler(boost::make_shared<operation::Scheduler>()),
	 m_configurationManager(),
	 m_alarmHandler(),
	 _program_state(fixs_ith::PROGRAM_STATE_UNKNOWN)
	{
		pid_t parent_pid = ::getpid();
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "workingset created by ITH daemon pid:<%d>", parent_pid);
	}

	workingset::~workingset()
	{
		pid_t parent_pid = ::getpid();
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "workingset deleted by ITH daemon pid:<%d>", parent_pid);
	}

	int workingset::stop_context_schedulers(bool wait_termination)
	{
		ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_synch_mutex);

		int overall_result = fixs_ith::ERR_NO_ERRORS;

		for (scheduler_map_t::iterator it = _context_scheduler_map.begin(); it != _context_scheduler_map.end(); ++it)
		{
			//First signal to all scheduler to stop
			int result = it->second->stop(false);
			overall_result = (fixs_ith::ERR_NO_ERRORS != result)? result: overall_result;
		}

		//Then wait for each one to end (if required)
		if (wait_termination)
		{
			for (scheduler_map_t::iterator it = _context_scheduler_map.begin(); it != _context_scheduler_map.end(); ++it)
			{
				if (it->second->is_running())
				{
					int result = it->second->wait_termination();
					overall_result = (fixs_ith::ERR_NO_ERRORS != result)? result: overall_result;
				}
			}

			_context_scheduler_map.clear();
		}

		return overall_result;

	}

	int workingset::schedule(operation::identifier_t id, const void * op_details)
	{
		return schedule(_main_scheduler, id, op_details);
	}

	int workingset::schedule(operation::identifier_t id, ACE_Future<operation::result>* op_result, const void* op_details)
	{
		return schedule(_main_scheduler, id, op_result, op_details);
	}

	int workingset::schedule(const std::string& contextKey, operation::identifier_t id, const void * op_details)
	{
		return schedule(get_context_scheduler(contextKey), id, op_details);
	}

	int workingset::schedule(const std::string& contextKey, operation::identifier_t id, ACE_Future<operation::result>* op_result, const void* op_details)
	{
		return schedule(get_context_scheduler(contextKey), id, op_result, op_details);
	}

	int workingset::schedule_until_completion(operation::identifier_t id, unsigned int delay, unsigned int interval, const void * op_details,  size_t op_details_size)
	{
		return schedule_until_completion(_main_scheduler, id, delay, interval, op_details, op_details_size);
	}

	int workingset::schedule_until_completion(const std::string& contextKey, operation::identifier_t id, unsigned int delay, unsigned int interval, const void * op_details,  size_t op_details_size)
	{
		return schedule_until_completion(get_context_scheduler(contextKey), id, delay, interval, op_details, op_details_size);
	}

	boost::shared_ptr<operation::Scheduler> workingset::get_context_scheduler(const std::string& contextKey)
	{
		ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_synch_mutex);

		scheduler_map_t::iterator it = _context_scheduler_map.find(contextKey);

		if (it == _context_scheduler_map.end())
		{
			boost::shared_ptr<operation::Scheduler> context_scheduler = boost::make_shared<operation::Scheduler>();
			context_scheduler->start();

			_context_scheduler_map.insert(std::pair<std::string, boost::shared_ptr<operation::Scheduler> > (contextKey, context_scheduler));
			return context_scheduler;
		}
		else
		{
			if (!it->second->is_running())
			{
				it->second->start();
			}
			return (it->second);
		}
	}

	int workingset::schedule(boost::shared_ptr<operation::Scheduler> scheduler, operation::identifier_t id, const void * op_details)
	{
		operation::Creator operationFactoryCreator(scheduler);
		return operationFactoryCreator.schedule(id, op_details);
	}

	int workingset::schedule(boost::shared_ptr<operation::Scheduler> scheduler, operation::identifier_t id, ACE_Future<operation::result>* op_result, const void* op_details)
	{
		operation::Creator operationFactoryCreator(scheduler);
		return operationFactoryCreator.schedule(id, op_result, op_details);
	}

	int workingset::schedule_until_completion(boost::shared_ptr<operation::Scheduler> scheduler, operation::identifier_t id, unsigned int delay, unsigned int interval, const void * op_details,  size_t op_details_size)
	{
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Scheduling operation '%d' with delay %u and interval %u...", id, delay, interval);

		// check 'interval' parameter: MUST be greater than 0
		if(interval == 0)
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Invalid value ['0'] provided for 'interval' parameter. It must be greater than 0  !");
			return fixs_ith::ERR_SCHEDULER_INVALID_INTERVAL;
		}

		// create a 'timer event handler' object, that will be used to enqueue the operation and wait for result
		operation::timer_handler * op_tmr_hdlr = operation::timer_handler::create_instance(scheduler, id, op_details, op_details_size, delay, interval);
		if(!op_tmr_hdlr)
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "NO memory available to schedule the operation '%d' (TIMER creation failure) !", id);
			return fixs_ith::ERR_MEMORY_BAD_ALLOC;
		}

		// in case the provided 'delay' is 0, directly enqueue the operation without waiting the first "timer tick"
		if(delay == 0)
		{
			int schedule_res = schedule(scheduler, id, & (op_tmr_hdlr->_op_result), op_tmr_hdlr-> _op_data);
			if(schedule_res != fixs_ith::ERR_NO_ERRORS)
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, " *** TMR_HDLR ## %03d : unable to schedule OP '%d'. error_code == %d", op_tmr_hdlr->_obj_id, op_tmr_hdlr->_op_id, schedule_res);
				delete op_tmr_hdlr;
				return schedule_res;
			}
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, " *** TMR_HDLR ## %03d : OP '%d' successfully scheduled ! ", op_tmr_hdlr->_obj_id, op_tmr_hdlr->_op_id);
		}

		// now schedule the timer on the ITH main reactor with the provided 'delay' and 'interval'
		ACE_Time_Value tv(delay,0);
		int timer_id = _main_reactor.schedule_timer(op_tmr_hdlr,0,tv);
		if(timer_id == -1)
		{
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "Error while scheduling the OP '%d' with delay '%u' ('schedule_timer' failure) %s !",
					id, delay, ((delay == 0) ? "[but the OP was successfully enqueued]": ""));

			// TODO: if delay is 0, we cannot destroy the timer event handler object, because the OP
			// was successfully enqueued and the ITH Scheduler uses  'op_tmr_hdlr->_op_result' to return the OP result.
			// In this case we could defer the timer event handler object destruction, but at the moment we wait for operation completion
			// before destroying the timer object.
			if( (delay == 0))
			{
				operation::result op_res;
				FIXS_ITH_LOG(LOG_LEVEL_DEBUG, " *** TMR_HDLR ## %03d : waiting for OP '%d' result ", op_tmr_hdlr->_obj_id, op_tmr_hdlr->_op_id);
				op_tmr_hdlr->_op_result.get(op_res);
				FIXS_ITH_LOG(LOG_LEVEL_DEBUG, " *** TMR_HDLR ## %03d : OP '%d' result obtained. ErrorCode = %d", op_tmr_hdlr->_obj_id, op_tmr_hdlr->_op_id, op_res.errorCode);
			}

			delete op_tmr_hdlr;

			return fixs_ith::ERR_REACTOR_SCHEDULE_TIMER_FAILURE;
		}

		op_tmr_hdlr->set_timer_id(timer_id);

		return fixs_ith::ERR_NO_ERRORS;
	}

}

