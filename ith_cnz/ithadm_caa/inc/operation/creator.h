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
#ifndef ITHADM_CAA_INC_FIXS_ITH_CREATOR_H_
#define ITHADM_CAA_INC_FIXS_ITH_CREATOR_H_

#include "operation/creatorinterface.h"
#include "operation/scheduler.h"

#include "boost/shared_ptr.hpp"



namespace operation {

	/**
	 * @class  Creator
	 *
	 * @brief
	 * Factory class of operation requests.
	 *
	 * The requested operation is queued into the operation scheduler
	 * in order to be executed into a sequential way.
	 *
	 * @sa  OperationScheduler
	 */
	class Creator : public  CreatorInterface
	{
		friend class Scheduler;
	 public:

		///  Constructor.
		Creator(boost::shared_ptr<Scheduler>& scheduler):_scheduler(scheduler) { };

		/// Destructor.
		virtual ~Creator() { };

		/**
		 * @brief Schedule execution of an operation.
		 *
		 * The specified operation is created and queued into the operation scheduler queue.
		 *
		 * @param  id Operation identifier
		 *
		 * @retval >0 The number of operation requests on the queue after adding
		 *            the specified request.
		 * @retval -1 if an error occurs; errno contains further information.
		 */
		virtual int schedule(const operation::identifier_t id);

		/**
		 * @brief Schedule execution of an operation.
		 *
		 * The specified operation is created and queued into the operation scheduler queue.
		 *
		 * @param  id Operation identifier
		 * @param  op_details Operation details
		 *
		 * @retval >0 The number of operation requests on the queue after adding
		 *            the specified request.
		 * @retval -1 if an error occurs; errno contains further information.
		 */
		virtual int schedule(const operation::identifier_t id, const void* op_details);

		/**
		 * @brief Schedule execution of an operation.
		 *
		 * The specified operation is created and queued into the operation scheduler queue.
		 *
		 * @param  id Operation identifier
		 * @param  op_result Operation excution result
		 * @param  op_details Operation details
		 *
		 * @retval >0 The number of operation requests on the queue after adding
		 *            the specified request.
		 * @retval -1 if an error occurs; errno contains further information.
		 */
		virtual int schedule(const operation::identifier_t id, ACE_Future<operation::result>* op_result, const void* op_details);

		virtual int execute(const operation::identifier_t id, const void* op_details);

	 protected:

		/**
		 * @brief Create an operation object
		 *
		 * @param  id Identifier of the operation
		 *
		 * @retval Pointer to the created operation object.
		 *
		 */
		virtual  OperationBase* make(const operation::identifier_t id);

	 private:
		/// operation scheduler used to enqueue the operation
		boost::shared_ptr<Scheduler> _scheduler;
	};

} /* namespace operation */

#endif /* ITHADM_CAA_INC_FIXS_ITH_CREATOR_H_ */
