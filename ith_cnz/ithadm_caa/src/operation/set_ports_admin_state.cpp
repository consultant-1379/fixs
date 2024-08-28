/*
 * set_ports_admin_state.cpp
 *
 *  Created on: Nov 3, 2016
 *      Author: xassore
 */


#include "operation/cease_alarm.h"

#include "operation/set_ports_admin_state.h"
#include "operation/operation.h"
#include "fixs_ith_programconstants.h"
#include "fixs_ith_programmacros.h"

#include "fixs_ith_logger.h"
#include "fixs_ith_workingset.h"


namespace operation
{

	/* **************************************
	 * class set_ports_admin_state
	 * ************************************** */

	set_ports_admin_state::set_ports_admin_state()
	: OperationBase(SET_PORT_ADMIN_STATE), m_switchboard_key(), m_adminStateValue(imm::EMPTY_ADMIN_STATE) {}


	void set_ports_admin_state::setOperationDetails(const void* op_details)
	{
		const op_parameters * parameter = reinterpret_cast<const op_parameters*>(op_details);

		m_switchboard_key.assign(parameter->switchboard_key);
		m_adminStateValue = parameter->adminStateValue;
	}

	int set_ports_admin_state::call()
	{
		FIXS_ITH_LOG(LOG_LEVEL_INFO, " ->>> 'Set Ports ADMIN STATE' operation requested");

		FIXS_ITH_LOG(LOG_LEVEL_INFO, "set_ports_admin_state:: start setting admin state of ports on switch board '%s' ...",
				m_switchboard_key.c_str());

		//Create context if it doesn't exist
		engine::contextAccess_t contextAccess(m_switchboard_key, engine::CREATE_IF_NOT_FOUND, engine::SHARED_ACCESS);

    	fixs_ith::ErrorConstants call_result = fixs_ith::workingSet_t::instance()->getConfigurationManager().setAdminStateOfDelayedPorts(m_switchboard_key, m_adminStateValue );

    	if (fixs_ith::ERR_NO_ERRORS != call_result && fixs_ith::ERR_CS_NO_BOARD_FOUND != call_result)
    	{ // ERROR
    		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'start' failed: cannot set admin state of ports on switch board having key %s", m_switchboard_key.c_str());

      	}
		else //Admin State successful set or board removed: in both cases do not reschedule the operation
		{
			// TODO: CEASE ALARM
			if (fixs_ith::workingSet_t::instance()->getAlarmHandler().ceaseSMXConfigurationFaultAlarm(m_switchboard_key)) {
				FIXS_ITH_LOG(LOG_LEVEL_INFO, " ->>> 'Set Ports ADMIN STATE': alarm SMXConfigurationFaultAlarm() successfully ceased");
			} else { // ERROR: Ceasing the alarm
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, " ->>> 'Set Ports ADMIN STATE': m_switchboard_key == <%s>: "
						"ceaseSMXConfigurationFaultAlarm() failed", m_switchboard_key.c_str());

				// we need to schedule the CEASE_ALARM operation
				cease_alarm::op_parameters op_params;

				op_params.alarm_type = alarms::Alarm::SMX_CONFIGURATION_FAULT;
				op_params.alarm_obj_reference = m_switchboard_key;

				FIXS_ITH_LOG_IMPL(LOG_LEVEL_INFO, " ->>> 'Set Ports ADMIN STATE': Scheduling the CEASE_ALARM operation");


				fixs_ith::workingSet_t::instance()->schedule_until_completion(CEASE_ALARM, 7, 7, &op_params, sizeof(op_params));
			}

			FIXS_ITH_LOG(LOG_LEVEL_INFO, "set_ports_admin_state : setting admin state of ports executed successfully"); // OK
		}

		m_operationResult.setErrorCode(call_result);
		setResultToCaller();

		return fixs_ith::ERR_NO_ERRORS;
	}

}


