//	********************************************************
//
//	 COPYRIGHT Ericsson 2016
//	All rights reserved.
//
//	The Copyright to the computer program(s) herein
//	is the property of Ericsson 2016.
//	The program(s) may be used and/or copied only with
//	the written permission from Ericsson 2016 or in
//	accordance with the terms and conditions stipulated in
//	the agreement/contract under which the program(s) have
//	been supplied.
//
//	********************************************************


#ifndef ITHADM_CAA_INC_OPERATION_BNC_INSTANTIATE_H_
#define ITHADM_CAA_INC_OPERATION_BNC_INSTANTIATE_H_

#include "operation/operationbase.h"

namespace operation
{
	/**
	 * @class bnc_start
	 *
	 * @brief Instantiate  imm objects for BNC
	 *
	 * @sa OperationBase
	 */
	class bnc_instantiate: public OperationBase
	{
	public:

		/// Constructor.
		bnc_instantiate();

		/// Destructor.
		virtual ~bnc_instantiate() { }

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



#endif /* ITHADM_CAA_INC_OPERATION_BNC_INSTANTIATE_H_ */
