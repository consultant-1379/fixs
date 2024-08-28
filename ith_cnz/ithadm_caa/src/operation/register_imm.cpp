/*
 * register_imm.cpp
 *
 * Revision History
 *
 * mm/dd/yyyy	    Author	Description
 *==================================================
 * 04/04/2016 	   estevol 	Base version
 * 07/28/2021      XSIGANO      HZ29732
 * 07/27/2022      ZNITPAR	For the feature "Improvement of SMX configuration lifecycle in AMI",
 *                              Aligning the service startup behaviour in APG restore case with
 *                              that of normal service restart behaviour. Removed fixs_ith_brf_policy_handler
 *                              related function calls during service startup for APG restore scenario.
 *                              Removed commented line of codes for code cleanup.
 *
 */

#include "operation/magazine_switchboards_config_restore.h"

#include "operation/register_imm.h"
#include "common/utility.h"
#include "fixs_ith_programconstants.h"

#include "fixs_ith_logger.h"
#include "fixs_ith_workingset.h"

namespace operation {

	register_imm::register_imm()
	 : OperationBase(REGISTER_IMM_IMPLEMENTER)
	{

	}

	int register_imm::call()
	{
		FIXS_ITH_LOG(LOG_LEVEL_INFO, " ->>> 'IMM Registration' operation requested");

		// Register Object Implementers
		int call_result =  fixs_ith::workingSet_t::instance()->registerObjectImplementers();
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Object Implementers registration result:<%d>", call_result );

		if (call_result)
		{ // ERROR
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'IMM Registration' failed: cannot register IMM implementers");
		}
		else
		{
			FIXS_ITH_LOG(LOG_LEVEL_INFO, "IMM Implementer registration completed successfully."); // OK

			// Here a normal restart is supposed
			operation::identifier_t op_id = RELOAD_ALL_FROM_IMM;
			const void * op_details = 0;
			size_t op_details_size = 0;

			call_result = fixs_ith::workingSet_t::instance()->schedule_until_completion(op_id, 0, 3, op_details, op_details_size);
			if(common::utility::isSwitchBoardSMX())  //HZ29732
			{
				FIXS_ITH_LOG(LOG_LEVEL_WARN, "It is normal service so scheduling PIM");
				// Schedule PIM adaptation procedure
				struct {
					std::string switchboard_key;
					uint16_t ifIndex;
					fixs_ith::switchboard_plane_t sb_plane;
					} op_params;
				op_params.switchboard_key = ""; // // For all switch boards
				op_params.ifIndex = 0;
				op_params.sb_plane = fixs_ith::TRANSPORT_PLANE;
				fixs_ith::workingSet_t::instance()->schedule_until_completion(operation::PIM_ADAPTATION, 60, 3, &op_params, sizeof(op_params));
					
				// Schedule SMX-Transport switch board PIM alarm check
				fixs_ith::workingSet_t::instance()->schedule_until_completion(operation::CHECK_SFPPORT_CONSISTENCY, 120, 120, "", 1);
	       		}
                      }

		// set result to caller
		m_operationResult.setErrorCode(call_result);
		setResultToCaller();

		return fixs_ith::ERR_NO_ERRORS;
	}
} /* namespace operation */
