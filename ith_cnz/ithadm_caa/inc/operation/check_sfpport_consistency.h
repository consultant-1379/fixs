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

#ifndef ITHADM_CAA_INC_OPERATION_CHECK_SFPPORT_CONSISTENCY_H_
#define ITHADM_CAA_INC_OPERATION_CHECK_SFPPORT_CONSISTENCY_H_

#include "operation/operationbase.h"

namespace operation
{
	/**
	 * @class check_sfpport_consistency
	 *
	 * @brief check that the network configuration on Switch Boards is aligned with
	 * the network configuration in the APG Infrastructure Transport Model
	 *
	 * @sa OperationBase
	 */
	class check_sfpport_consistency: public OperationBase
	{
	public:

		/// Constructor.
		check_sfpport_consistency();

		/// Destructor.
		virtual ~check_sfpport_consistency() {}

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
		int raiseCeasePimAlarm(int16_t if_index, int16_t & port_presencestatus,uint32_t board_magazine,int32_t board_slot,const char * switch_board_ip,
				const unsigned * timeout_ms,const std::string & switchboard_key);

	private:

		std::string m_switchboard_key;
	};

} /* namespace operation */


#endif /* ITHADM_CAA_INC_OPERATION_CHECK_SFPPORT_CONSISTENCY_H_ */
