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
#ifndef ITHADM_CAA_INC_OPERATION_SCHEDULER_H_
#define ITHADM_CAA_INC_OPERATION_SCHEDULER_H_

#include <boost/noncopyable.hpp>
#include <ace/Task.h>
#include <ace/Recursive_Thread_Mutex.h>
#include <ace/Activation_Queue.h>



class ACE_Method_Request;

namespace operation
{
	/**
	 * @class Scheduler
	 *
	 * @brief A scheduler class derived from @c ACE_Task_Base.
	 *
	 * Maintains a priority-ordered queue of operation objects.
	 * Subsequently removes each operation request and invokes its @c call() method.
	 *
	 */
	class Scheduler : public ACE_Task_Base, private boost::noncopyable
	{
	 public:

		/// Constructor.
		inline Scheduler() : ACE_Task_Base(), m_ActivationQueue(), m_enqueue_enabled(false), m_synch_mutex() {}

		/// Destructor.
		virtual ~Scheduler();

		/**
		 *	@brief	Scheduler function thread.
		 */
		virtual int svc(void);

		/**
		 * @brief	Activates the scheduler thread.
		*/
		virtual int open(void *args = 0);

		/**
		 *	@brief	Initializes the scheduler task and prepare it to run as thread.
		 */
		virtual int start();

		/**
		 * @brief	Enqueues a @a Shutdown operation into the scheduler and eventually
		 * waits the thread termination.
		 */
		virtual int stop(bool wait_termination = true);

		/**
		 * @brief	This method enqueue a command in the queue
		 */
		int enqueue(ACE_Method_Request* cmdRequest);

		/**
		 * @brief	This method returns the scheduler running status
		 */
		bool is_running() const;

		/**
		 * @brief	This method waits for scheduler termination
		 */
		int wait_termination() {return wait();};

	 private:

		/**
		 * @brief	Queue of operation to execute.
		 *
		 * @sa ACE_Activation_Queue.
		 */
		ACE_Activation_Queue m_ActivationQueue;

		// used to indicate if is possible to enqueue a method request
		bool m_enqueue_enabled;

		ACE_Recursive_Thread_Mutex m_synch_mutex;

	};
}

#endif /* ITHADM_CAA_INC_OPERATION_SCHEDULER_H_ */
