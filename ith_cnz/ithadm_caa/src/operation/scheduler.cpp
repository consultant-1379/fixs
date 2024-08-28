//	********************************************************
//
//	 COPYRIGHT Ericsson 2015
//	All rights reserved.
//
//	The Copyright to the computer program(s) herein
//	is the property of Ericsson 2015.
//	The program(s) may be used and/or copied only with
//	the written permission from Ericsson 2015 or in
//	accordance with the terms and conditions stipulated in
//	the agreement/contract under which the program(s) have
//	been supplied.
//
//	********************************************************
#include "operation/scheduler.h"

#include "operation/operationbase.h"
#include "operation/creator.h"
#include "fixs_ith_logger.h"
#include <ace/Method_Request.h>
#include <ace/Recursive_Thread_Mutex.h>
#include <ace/Guard_T.h>

#include "fixs_ith_programmacros.h"
#include "fixs_ith_programconstants.h"
#include <memory>


namespace operation
{
	Scheduler::~Scheduler()
	{
		// Delete all queued request
		while(!((bool)m_ActivationQueue.is_empty()) )
		{
			// Dequeue the next method object
			std::auto_ptr<ACE_Method_Request> cmdRequest(m_ActivationQueue.dequeue());
		}
	}

	int Scheduler::svc(void)
	{
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Starting execution...");
		bool svcRun = true;

		int result = fixs_ith::ERR_NO_ERRORS;

		while(svcRun)
		{
			// FIXS_ITH_TRACE_MESSAGE("Waiting for operation requests");

			// Dequeue the next method object
			std::auto_ptr<ACE_Method_Request> cmdRequest(m_ActivationQueue.dequeue());

			//interrogate the auto_ptr to check if it is null
			if (cmdRequest.get())
			{
				// FIXS_ITH_TRACE_MESSAGE("Executing Operation");
				if(cmdRequest->call() == fixs_ith::ERR_SVC_DEACTIVATE)
				{
					// NOTE: only "SHUTDOWN" operation can return 'fixs_ith::ERR_SVC_DEACTIVATE'
					svcRun = false;
				}
			}
			else
			{
				// FIXS_ITH_TRACE_MESSAGE("WARNING: READ NULL POINTER");
				FIXS_ITH_LOG(LOG_LEVEL_WARN, "READ NULL POINTER");
			}
		}

		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "...Terminating execution");
		return result;
	}

	int Scheduler::open(void *args)
	{
		// FIXS_ITH_TRACE_FUNCTION;
		UNUSED(args);

		int result = fixs_ith::ERR_NO_ERRORS;

		if(activate(THR_NEW_LWP | THR_JOINABLE | THR_INHERIT_SCHED))
		{
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "cannot start svc thread");
			//FIXS_ITH_TRACE_MESSAGE("ERROR: cannot start svc thread. errno: %d", errno);

			result = fixs_ith::ERR_SVC_ACTIVATE;
		}
		else
		{
			m_enqueue_enabled = true;
		}

		return result;
	}

	int Scheduler::start()
	{
		//FIXS_ITH_TRACE_FUNCTION;

		int result = fixs_ith::ERR_NO_ERRORS;

		if( thr_count() > 0U )
		{
			//ERROR CASE
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "thread already running. thr_count(): %zu", thr_count());
			// FIXS_ITH_TRACE_MESSAGE("ERROR: thread already running. thr_count(): %zu", thr_count());
			result = fixs_ith::ERR_SCHEDULER_ALREADY_STARTED;
		}
		else
		{
			result = open();
		}

		return result;
	}

	int Scheduler::stop(bool wait_termination)
	{
		// FIXS_ITH_TRACE_FUNCTION;

		if (thr_count() > 0)
		{
			boost::shared_ptr<operation::Scheduler> nullScheduler;
			operation::Creator operationFactoryCreator(nullScheduler);
			ACE_Method_Request* terminate = operationFactoryCreator.make(SHUTDOWN);

			ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_synch_mutex);
			if(!guard.locked())
				FIXS_ITH_LOG(LOG_LEVEL_WARN, "Unable to acquire internal synch mutex !");

			int result = m_ActivationQueue.enqueue(terminate);

			FIXS_ITH_LOG(LOG_LEVEL_INFO, "Thread running, Shutdown request enqueue result:<%d>", result);

			// to deny enqueueing after scheduler SHUTDOWN
			m_enqueue_enabled = false;
		}

		return wait_termination ? wait(): fixs_ith::ERR_NO_ERRORS;
	}

	int Scheduler::enqueue(ACE_Method_Request* cmdRequest)
	{
		ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_synch_mutex);

		if(!guard.locked())
			return fixs_ith::ERR_SCHEDULER_QUEUE_LOCK;

		if(!m_enqueue_enabled)
			return fixs_ith::ERR_SCHEDULER_ENQUEUE_NOT_ENABLED;

		if( -1 == m_ActivationQueue.enqueue(cmdRequest))
			return fixs_ith::ERR_SCHEDULER_ENQUEUE_FAILURE;

		return fixs_ith::ERR_NO_ERRORS;
	}

	bool Scheduler::is_running() const
	{
		return (thr_count() > 0);
	}

}
