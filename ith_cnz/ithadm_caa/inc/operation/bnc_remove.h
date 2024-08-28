/*
 * bnc_remove.h
 *
 *  Created on: Apr 29, 2016
 *      Author: eanform
 */

#ifndef ITHADM_CAA_INC_OPERATION_BNC_REMOVE_H_
#define ITHADM_CAA_INC_OPERATION_BNC_REMOVE_H_


#include "operation/operationbase.h"

namespace operation
{
	/**
	 * @class remove_switchboard_configuration
	 *
	 * @brief Remove switchboard configuration
	 *
	 * @sa OperationBase
	 */
	class bnc_remove: public OperationBase
	{
	public:

		/// Constructor.
		bnc_remove();

		/// Destructor.
		virtual ~bnc_remove() { }

		/**
		 * @brief Set an operation info details.
		 *
		 * @param  op_details Operation details.
		 */
		virtual void setOperationDetails(const void* op_details);

		/**
		 * @brief  Implementation of the operation.
		 */
		virtual int call();

	private:

		std::string m_switchboard_key;
	};

} /* namespace operation */



#endif /* ITHADM_CAA_INC_OPERATION_BNC_REMOVE_H_ */
