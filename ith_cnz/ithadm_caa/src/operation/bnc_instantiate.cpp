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

#include "operation/cease_alarm.h"

#include "operation/bnc_instantiate.h"
#include "operation/operation.h"
#include "fixs_ith_programconstants.h"
#include "fixs_ith_programmacros.h"
#include "operation/set_ports_admin_state.h"
#include "fixs_ith_logger.h"
#include "fixs_ith_workingset.h"


namespace operation
{

/* **************************************
 * class bnc_instantiate
 * ************************************** */

	bnc_instantiate::bnc_instantiate()
    : OperationBase(BNC_OBJECTS_INSTANTIATE)
    {
    }

	void bnc_instantiate::setOperationDetails(const void* op_details)
	{
		const char* key = reinterpret_cast<const char*>(op_details);

		m_switchboard_key.assign(key);
	}

    int bnc_instantiate::call()
    {
        FIXS_ITH_LOG(LOG_LEVEL_INFO, " ->>> 'Instantiate BNC IMM OBJECTS' operation requested");

    	FIXS_ITH_LOG(LOG_LEVEL_INFO, "bnc_instantiate:: start creation of BNC IMM objects for switch board '%s' ...",
    			m_switchboard_key.c_str());

		//Create context if it doesn't exist
		engine::contextAccess_t contextAccess(m_switchboard_key, engine::CREATE_IF_NOT_FOUND, engine::SHARED_ACCESS);

    	fixs_ith::ErrorConstants call_result = fixs_ith::workingSet_t::instance()->get_transportBNChandler().create_bnc_objects(m_switchboard_key);

    	if (fixs_ith::ERR_NO_ERRORS != call_result && fixs_ith::ERR_CS_NO_BOARD_FOUND != call_result)
    	{ // ERROR
    		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'start' failed: cannot create BNC IMM objects");

    		// Raise the SMXConfigurationFault alarm
				std::string mag_str;
				int32_t slot_pos = -1;

				if (common::utility::get_magazine_and_slot_from_sb_key(mag_str, slot_pos, m_switchboard_key)) {
					// ERROR: Getting magazine and slot from the key
					FIXS_ITH_LOG(LOG_LEVEL_ERROR, " ->>> 'Instantiate BNC IMM OBJECTS': call to "
						"'get_magazine_and_slot_from_sb_key()' failed: Cannot get magazine and slot from "
						"the m_switchboard_key string: raiseSMXConfigurationFault alarm not raised: m_switchboard_key == <%s>",
						m_switchboard_key.c_str());
				} else {
					if (fixs_ith::workingSet_t::instance()->getAlarmHandler().raiseSMXConfigurationFaultAlarm(m_switchboard_key, mag_str, slot_pos)) {
						FIXS_ITH_LOG(LOG_LEVEL_INFO, " ->>> 'Instantiate BNC IMM OBJECTS': alarm SMXConfigurationFaultAlarm() successfully raised");
					} else { // ERROR: Raising the alarm
						FIXS_ITH_LOG(LOG_LEVEL_ERROR, " ->>> 'Instantiate BNC IMM OBJECTS': m_switchboard_key == <%s>, "
							"mag_str == <%s>, slot_pos == <%d>: raiseSMXConfigurationFaultAlarm() failed", m_switchboard_key.c_str(),
							mag_str.c_str(), slot_pos);
					}
				}
    	}
    	else
    	{
    		//Objects created or board removed: in both cases do not reschedule the operation

    		//    		// TODO: CEASE ALARM
    		//				if (fixs_ith::workingSet_t::instance()->getAlarmHandler().ceaseSMXConfigurationFaultAlarm(m_switchboard_key)) {
    		//					FIXS_ITH_LOG(LOG_LEVEL_INFO, " ->>> 'Instantiate BNC IMM OBJECTS': alarm SMXConfigurationFaultAlarm() successfully ceased");
    		//				} else { // ERROR: Ceasing the alarm
    		//					FIXS_ITH_LOG(LOG_LEVEL_ERROR, " ->>> 'Instantiate BNC IMM OBJECTS': m_switchboard_key == <%s>: "
    		//						"ceaseSMXConfigurationFaultAlarm() failed", m_switchboard_key.c_str());
    		//
    		//					// we need to schedule the CEASE_ALARM operation
    		//					cease_alarm::op_parameters op_params;
    		//
    		//					op_params.alarm_type = alarms::Alarm::SMX_CONFIGURATION_FAULT;
    		//					op_params.alarm_obj_reference = m_switchboard_key;
    		//
    		//					FIXS_ITH_LOG_IMPL(LOG_LEVEL_INFO, " ->>> 'Instantiate BNC IMM OBJECTS': Scheduling the CEASE_ALARM operation");
    		//
    		//					fixs_ith::workingSet_t::instance()->schedule_until_completion(CEASE_ALARM, 7, 7, &op_params, sizeof(op_params));

    		operation::set_ports_admin_state::op_parameters params;
    		::memset(params.switchboard_key, 0, FIXS_ITH_ARRAY_SIZE(params.switchboard_key));
    		strncpy(params.switchboard_key, m_switchboard_key.c_str(), FIXS_ITH_ARRAY_SIZE(params.switchboard_key)-1);
    		params.adminStateValue = imm::EMPTY_ADMIN_STATE;  // SET_PORT_ADMIN_STATE operation will read the admin state value from Context map.

    		fixs_ith::workingSet_t::instance()->schedule_until_completion(m_switchboard_key, operation::SET_PORT_ADMIN_STATE,0,10,&params, sizeof(params));

    		FIXS_ITH_LOG(LOG_LEVEL_INFO, "bnc_instantiate : Instantiate BNC imm objects executed successfully"); // OK
    	}

    	m_operationResult.setErrorCode(call_result);
  		setResultToCaller();

  		return fixs_ith::ERR_NO_ERRORS;
    }
}
