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

#ifndef ITHADM_CAA_INC_OPERATION_PIM_ADAPTATION_H_
#define ITHADM_CAA_INC_OPERATION_PIM_ADAPTATION_H_

#include "operation/operationbase.h"
#include "fixs_ith_snmpconstants.h"

namespace operation
{
	/**
	 * @class pim_adaptation
	 *
	 * @brief This operation handles the A2-A8 plug-in module changes and configures SMX ports properly
	 *
	 * @sa OperationBase
	 */
	class pim_adaptation : public OperationBase
	{
	public:

		struct op_parameters
		{
			std::string switchboard_key;
			uint16_t ifIndex;
			fixs_ith::switchboard_plane_t sb_plane;
		};

		// Constructor.
		pim_adaptation();

		// Destructor.
		virtual ~pim_adaptation() {}

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

		/**
		 * @brief  Implementation of the operation for SFP ports.
		 */
		static fixs_ith::ErrorConstants m_pim_adaptation(const std::string& switchboard_key, const fixs_ith::SwitchBoardPlaneConstants& plane, const uint16_t& ifIndex, bool isInContext = false, bool isAutoNegToBeEnabled = false);
		static fixs_ith::ErrorConstants m_set_auto_neg_admin_status(const std::string& switchboard_key, const fixs_ith::SwitchBoardPlaneConstants& plane, const uint16_t& ifIndex); //TR_HZ29732
	
	private:

		/**
		 * @brief  Set auto negotiation for not-SFP ports.
		 */
		//fixs_ith::ErrorConstants m_set_auto_neg_admin_status(const std::string& switchboard_key, const fixs_ith::SwitchBoardPlaneConstants& plane, const uint16_t& ifIndex);


		bool isSFPport() const {return ((m_sb_plane == fixs_ith::TRANSPORT_PLANE) && ((m_ifIndex > ITH_SNMP_COMMUNITY_PIM_PORT_START && m_ifIndex < ITH_SNMP_COMMUNITY_PIM_PORT_NOT_EXIST)
				|| (m_ifIndex > ITH_SNMP_COMMUNITY_PIM_PORT_NOT_EXIST && m_ifIndex <= ITH_SNMP_COMMUNITY_PIM_PORT_END)));};

	private:

		std::string m_switchboard_key;
		uint16_t m_ifIndex;
		fixs_ith::switchboard_plane_t m_sb_plane;
	};



} /* namespace operation */

#endif /* ITHADM_CAA_INC_OPERATION_PIM_ADAPTATION_H_ */
