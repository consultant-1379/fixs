/*
 * operationreload_context_after_cold_start_trap.cpp
 *
 *  Created on: Apr 8, 2016
 *      Author: estevol
 */

#include "operation/reload_context_after_cold_start_trap.h"
#include "operation/set_ports_admin_state.h"
#include "fixs_ith_programconstants.h"

#include "fixs_ith_logger.h"
#include "fixs_ith_workingset.h"
#include "common/utility.h"

namespace operation {

reload_context_after_cold_start_trap::reload_context_after_cold_start_trap() :
		OperationBase(RELOAD_CONTEXT_AFTER_COLD_START_TRAP), m_switchboard_key() {

}

reload_context_after_cold_start_trap::~reload_context_after_cold_start_trap() {
	// TODO Auto-generated destructor stub
}

void reload_context_after_cold_start_trap::setOperationDetails(
		const void* op_details) {
	const char* key = reinterpret_cast<const char*>(op_details);

	m_switchboard_key.assign(key);
}

int reload_context_after_cold_start_trap::call() {
	FIXS_ITH_LOG(LOG_LEVEL_INFO,
			" ->>> 'RELOAD CONTEXT AFTER [COLD START TRAP] OR [CONFIGURATION INCONSISTENCY DETECTED]' operation requested. Context <%s>",
			m_switchboard_key.c_str());

	std::string magazine_str;
	int32_t slot;
	int alarm_info_call_result =
			common::utility::get_magazine_and_slot_from_sb_key(magazine_str,
					slot, m_switchboard_key);

	if (alarm_info_call_result < 0) {
		// ERROR: Retrieving magazine and slot information
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				" ->>> Call to get_magazine_and_slot_from_sb_key() failed: " "Cannot raise or cease any configuration alarm: m_switchboard_key == <%s>.",
				m_switchboard_key.c_str());
	}

	// Reload MOs
	int call_result =
			fixs_ith::workingSet_t::instance()->getConfigurationManager().loadConfiguration(
					m_switchboard_key);

	if (fixs_ith::ERR_NO_ERRORS != call_result) {
		if (alarm_info_call_result >= 0)
			fixs_ith::workingSet_t::instance()->getAlarmHandler().raiseSMXConfigurationFaultAlarm(
					m_switchboard_key, magazine_str, slot);

		FIXS_ITH_LOG(LOG_LEVEL_INFO,
				" ->>> 'RELOAD CONTEXT FROM IMM' operation failed. Context <%s>.",
				m_switchboard_key.c_str());
	} else {
		fixs_ith::workingSet_t::instance()->getConfigurationManager().removeFromReloadInProgressSet(
				m_switchboard_key);

		/* HX22681 BEGIN */
		// Schedule PIM adaptation procedure
		struct {
			std::string switchboard_key;
			uint16_t ifIndex;
			fixs_ith::switchboard_plane_t sb_plane;
		} op_params;
		op_params.switchboard_key = m_switchboard_key;
		op_params.ifIndex = 0; // For all A2-A8 SFP ports
		op_params.sb_plane = fixs_ith::TRANSPORT_PLANE;

		// Schedule PIM adaptation only in case of SMX
		if (common::utility::isSwitchBoardSMX()) {
			fixs_ith::workingSet_t::instance()->schedule_until_completion(
					m_switchboard_key, operation::PIM_ADAPTATION, 5, 3,
					&op_params, sizeof(op_params));
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
					"Scheduled operation 'PIM ADAPTATION' for SMX:<%s>",
					m_switchboard_key.c_str());
		}
		/* HX22681 END */

		// Schedule SET_PORT_ADMIN_STATE
		operation::set_ports_admin_state::op_parameters params;
		::memset(params.switchboard_key, 0,
				FIXS_ITH_ARRAY_SIZE(params.switchboard_key));
		strncpy(params.switchboard_key, m_switchboard_key.c_str(),
				FIXS_ITH_ARRAY_SIZE(params.switchboard_key) - 1);
		params.adminStateValue = imm::EMPTY_ADMIN_STATE; // SET_PORT_ADMIN_STATE operation will read the admin state value from Context map.
		/* HX22681 BEGIN */
		fixs_ith::workingSet_t::instance()->schedule_until_completion(
				m_switchboard_key, operation::SET_PORT_ADMIN_STATE, 20, 10,
				&params, sizeof(params));
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
				"Scheduled operation 'SET_PORT_ADMIN_STATE' for SMX:<%s>",
				m_switchboard_key.c_str());
		/* HX22681 END */

//		if (alarm_info_call_result >= 0)
//			fixs_ith::workingSet_t::instance()->getAlarmHandler().ceaseSMXConfigurationFaultAlarm(m_switchboard_key);
		FIXS_ITH_LOG(LOG_LEVEL_INFO,
				"RELOAD CONTEXT AFTER [COLD START TRAP] OR [CONFIGURATION INCONSISTENCY DETECTED] executed successfully"); // OK
	}

	// set result to caller
	m_operationResult.setErrorCode(call_result);
	setResultToCaller();

	return fixs_ith::ERR_NO_ERRORS;
}

} /* namespace operation */
