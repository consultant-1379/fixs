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

#ifndef ITHADM_CAA_INC_OPERATION_CHECK_CONFIG_CONSISTENCY_H_
#define ITHADM_CAA_INC_OPERATION_CHECK_CONFIG_CONSISTENCY_H_

#include "operation/operationbase.h"

namespace operation
{
	/**
	 * @class check_config_consistency
	 *
	 * @brief check that the network configuration on Switch Boards is aligned with
	 * the network configuration in the APG Infrastructure Transport Model
	 *
	 * @sa OperationBase
	 */
	class check_config_consistency: public OperationBase
	{
	public:

		/// Constructor.
		check_config_consistency();

		/// Destructor.
		virtual ~check_config_consistency() {}

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
		 int m_check_context_cfg_consistency(const std::string & switchboard_key);

	private:

		std::string m_switchboard_key;
	};




} /* namespace operation */





#endif /* ITHADM_CAA_INC_OPERATION_CHECK_CONFIG_CONSISTENCY_H_ */
