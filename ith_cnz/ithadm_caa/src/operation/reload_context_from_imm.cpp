/*
 * reload_context_from_imm.cpp
 *
 *  Created on: Apr 4, 2016
 *      Author: estevol
 */

#include "operation/reload_context_from_imm.h"

#include "fixs_ith_programconstants.h"
#include "common/utility.h"

#include "fixs_ith_logger.h"
#include "fixs_ith_workingset.h"

namespace operation {

reload_context_from_imm::reload_context_from_imm():
OperationBase(RELOAD_CONTEXT_FROM_IMM),
m_switchboard_key()
{

}

reload_context_from_imm::~reload_context_from_imm() {
	// TODO Auto-generated destructor stub
}


void reload_context_from_imm::setOperationDetails(const void* op_details)
{
	const char* key = reinterpret_cast<const char*>(op_details);

	m_switchboard_key.assign(key);
}

int reload_context_from_imm::call()
{
	FIXS_ITH_LOG(LOG_LEVEL_INFO, " ->>> 'RELOAD CONTEXT FROM IMM' operation requested. Context <%s>", m_switchboard_key.c_str());

	// Reload MOs
	int call_result = fixs_ith::workingSet_t::instance()->getConfigurationManager().loadConfiguration(m_switchboard_key);

	if (fixs_ith::ERR_NO_ERRORS != call_result)
	{
		if (fixs_ith::ERR_CS_NO_BOARD_FOUND == call_result)
		{
			//board removed from HWC table
			call_result = fixs_ith::ERR_NO_ERRORS;
		}
		else
		{
			FIXS_ITH_LOG(LOG_LEVEL_INFO, " ->>> 'RELOAD CONTEXT FROM IMM' operation failed. Context <%s>.", m_switchboard_key.c_str());

			if (m_switchboard_key.empty()) {
				FIXS_ITH_LOG(LOG_LEVEL_WARN, " ->>> 'RELOAD CONTEXT FROM IMM': m_switchboard_key operation parameter is empty");
			} else {
				std::string mag_str;
				int32_t slot_pos = -1;

				if (common::utility::get_magazine_and_slot_from_sb_key(mag_str, slot_pos, m_switchboard_key)) {
					// ERROR: Getting magazine and slot from the key
					FIXS_ITH_LOG(LOG_LEVEL_ERROR, " ->>> 'RELOAD CONTEXT FROM IMM': call to "
						"'get_magazine_and_slot_from_sb_key()' failed: Cannot get magazine and slot from "
						"the m_switchboard_key string: raiseSMXConfigurationFault alarm not raised: m_switchboard_key == <%s>",
						m_switchboard_key.c_str());
				} else {
					if (fixs_ith::workingSet_t::instance()->getAlarmHandler().raiseSMXConfigurationFaultAlarm(m_switchboard_key, mag_str, slot_pos)) {
						FIXS_ITH_LOG(LOG_LEVEL_INFO, " ->>> 'RELOAD CONTEXT FROM IMM': alarm raiseSMXConfigurationFaultAlarm() successfully raised");
					} else { // ERROR: Raising the alarm
						FIXS_ITH_LOG(LOG_LEVEL_ERROR, " ->>> 'RELOAD CONTEXT FROM IMM': m_switchboard_key == <%s>, "
							"mag_str == <%s>, slot_pos == <%d>: raiseSMXConfigurationFaultAlarm() failed", m_switchboard_key.c_str(),
							mag_str.c_str(), slot_pos);
					}
				}
			}
		}
	}
	else
	{
		fixs_ith::workingSet_t::instance()->getConfigurationManager().removeFromReloadInProgressSet(m_switchboard_key);

		if (m_switchboard_key.empty()) {
			FIXS_ITH_LOG(LOG_LEVEL_WARN, " ->>> 'RELOAD CONTEXT FROM IMM': m_switchboard_key operation parameter is empty");
		} else {
			fixs_ith::workingSet_t::instance()->schedule_until_completion(m_switchboard_key, operation::BNC_OBJECTS_INSTANTIATE,0,10, m_switchboard_key.c_str(), (m_switchboard_key.length() + 1)/sizeof(uint8_t));
		}
	}

	// set result to caller
	m_operationResult.setErrorCode(call_result);
	setResultToCaller();

	return call_result;
}


} /* namespace operation */
