/*
 * check_config_consistency.cpp
 *
 *  Created on: Jul 6, 2016
 *      Author: xassore
 */

#include "operation/check_config_consistency.h"

#include "fixs_ith_programconstants.h"
#include "switch_interface/full_adapter.h"
#include "fixs_ith_logger.h"
#include "fixs_ith_workingset.h"
#include "common/utility.h"

namespace operation {

check_config_consistency::check_config_consistency() :
		OperationBase(CHECK_CONFIG_CONSISTENCY), m_switchboard_key() {

}

void check_config_consistency::setOperationDetails(const void* op_details) {
	const char* key = reinterpret_cast<const char*>(op_details);

	m_switchboard_key.assign(key);
}

int check_config_consistency::call() {
	FIXS_ITH_LOG(LOG_LEVEL_INFO,
			" ->>> 'CHECK CONFIGURATION CONSISTENCY' operationrequested. Context <%s>",
			m_switchboard_key.empty() ? "ALL": m_switchboard_key.c_str());

	// if the operation has been requested on a specific switch board, do the check only for the specified switch board,
	// otherwise, do the check for all switch boards which are in configuration

	int call_result = fixs_ith::ERR_NO_ERRORS;
	int failure_counter = 0;

	if (m_switchboard_key.empty()) {
		std::set<std::string> switchBoardKeys;
		// Load Switchboard Keys from CS
		call_result =
				fixs_ith::workingSet_t::instance()->get_sbdatamanager().get_switchboard_keys(
						switchBoardKeys);
		if (call_result) { // ERROR
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
					"UNABLE to load switchboard keys from CS");
			m_operationResult.setErrorCode(fixs_ith::ERR_CS_ERROR);
			setResultToCaller();
			return fixs_ith::ERR_NO_ERRORS;
		}
		int failure_counter = 0;

		for (std::set<std::string>::iterator it = switchBoardKeys.begin();
				it != switchBoardKeys.end(); ++it) {
			//Use context schedulers to execute consistency check for each context
			failure_counter +=
					(fixs_ith::ERR_NO_ERRORS
							== fixs_ith::workingSet_t::instance()->schedule(*it,
									CHECK_CONFIG_CONSISTENCY, it->c_str())) ?
							0 : 1;
		}
	} else {
		if (fixs_ith::workingSet_t::instance()->getConfigurationManager().isReloadInProgress(
				m_switchboard_key)) {
			FIXS_ITH_LOG(LOG_LEVEL_INFO,
					" Skip Consistency Check for Context <%s> because a 'reload Context from IMM' is in progress",
					m_switchboard_key.c_str());
		} else {
			// perform the consistency check
			int call_result = m_check_context_cfg_consistency(
					m_switchboard_key);
			if (call_result == fixs_ith::ERR_CONFIG_INCONSISTENCY_FOUND) // --> INCONSISTENCY FOUND !
					{
				FIXS_ITH_LOG(LOG_LEVEL_INFO,
						" Scheduling RELOAD_CONTEXT_AFTER_COLD_START_TRAP due to inconsistency found. Context <%s>",
						m_switchboard_key.c_str());

				// Reload MOs
				if (fixs_ith::workingSet_t::instance()->getConfigurationManager().addToReloadInProgressSet(
						m_switchboard_key)) {
					int sched_result =
							fixs_ith::workingSet_t::instance()->schedule_until_completion(
									m_switchboard_key,
									operation::RELOAD_CONTEXT_AFTER_COLD_START_TRAP,
									0, 5, (m_switchboard_key).c_str(),
									((m_switchboard_key).length() + 1)
											/ sizeof(uint8_t));

					if (sched_result < 0) {
						FIXS_ITH_LOG(LOG_LEVEL_ERROR,
								"Unable to schedule operation 'RELOAD_CONTEXT_AFTER_COLD_START_TRAP'. Call 'schedule_until_completion()' failed. call_result == %d",
								sched_result);

						failure_counter++;
						fixs_ith::workingSet_t::instance()->getConfigurationManager().removeFromReloadInProgressSet(
								m_switchboard_key);
					} else {
						FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
								"scheduled operation 'RELOAD_CONTEXT_AFTER_COLD_START_TRAP'. Call 'schedule_until_completion()' success. call_result == %d",
								sched_result);
					}
				} else {
					FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
							"RELOAD_CONTEXT_AFTER_COLD_START_TRAP operation for SMX <%s>is already in queue. It does not need to be rescheduled.",
							m_switchboard_key.c_str());
				}
			} else if (call_result < 0)	// --> Unable to perform the check
					{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR,
						"Call 'm_check_context_cfg_consistency()' failed for Context '%s': call_result == %d",
						m_switchboard_key.c_str(), call_result);

				failure_counter++;
			} else // (call_result == 0) --> NO INCONSISTENCY FOUND !
			{
				// NO ACTION NEEDED
			}
		}
	}

	// set result to caller
	m_operationResult.setErrorCode(
			failure_counter > 0 ?
					fixs_ith::ERR_GENERIC : fixs_ith::ERR_FOREVER_OP);
	setResultToCaller();

	return fixs_ith::ERR_NO_ERRORS;
}

// Private method
int check_config_consistency::m_check_context_cfg_consistency(
		const std::string & switchboard_key) {
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
			"Checking Configuration Consistency for Switch Board having key %s",
			switchboard_key.c_str());

	bool isSMX = common::utility::isSwitchBoardSMX();
	bool isCMX = common::utility::isSwitchBoardCMX();

	fixs_ith_switchboardinfo switchboard_info;
	fixs_ith_sbdatamanager::sbm_constant call_result =
			fixs_ith::workingSet_t::instance()->get_sbdatamanager().get_switchboard_info(
					switchboard_info, switchboard_key);
	if (call_result != fixs_ith_sbdatamanager::SBM_OK) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"Unable to check Configuration Consistency for Switch Board having key %s: [Switch Board NOT FOUND]",
				switchboard_key.c_str());
		return fixs_ith::ERR_CONFIG_CONTEXT_NOT_FOUND;
	}

	std::string vlan_to_check;
	if (isSMX) {
		// Get the VLAN to be checked
		if (switchboard_info.magazine_id == fixs_ith::APZ_MAGAZINE)
			vlan_to_check = engine::tipcVlan::control::NAME;
		else
			vlan_to_check =
					(switchboard_info.slot_position == fixs_ith::BOARD_SMX_LEFT) ?
							engine::defaultVlan::control::left::NAME :
							engine::defaultVlan::control::right::NAME;
	}

	if (isCMX) {

		// Get the VLAN to be checked
		if (switchboard_info.magazine_id == fixs_ith::APZ_MAGAZINE)
			vlan_to_check = engine::drbdVlan::transport::NAME;
		else
			vlan_to_check =
					(switchboard_info.slot_position == fixs_ith::BOARD_CMX_LEFT) ?
							engine::defaultVlan::transport::left::NAME :
							engine::defaultVlan::transport::right::NAME;
	}

	// retrieve from Infrastructure MOM the info about the selected VLAN
	engine::contextAccess_t contextAccess(switchboard_key, engine::GET_EXISTING,
			engine::SHARED_ACCESS);
	engine::Context* context = contextAccess.getContext();

	if (!context) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"Unable to check Configuration Consistency for Switch Board having key %s: [CONTEXT NOT ACCESSIBLE]",
				switchboard_key.c_str());
		return fixs_ith::ERR_CONFIG_CONTEXT_NOT_ACCESSIBLE;
	}

	boost::shared_ptr<engine::Vlan> vlaninfo;
	if (isSMX) {
		vlaninfo = context->getVlanByName(fixs_ith::CONTROL_PLANE,
				vlan_to_check, false);
		if (vlaninfo == 0) {
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
					"Unable to check Configuration Consistency for Switch Board having key %s: [VLAN TO CHECK NOT FOUND]",
					switchboard_key.c_str());
			return fixs_ith::ERR_VLAN_NOT_FOUND_IN_CONTEXT;

		}
	}

	if (isCMX) {
		vlaninfo = context->getVlanByName(fixs_ith::TRANSPORT_PLANE,
				vlan_to_check, false);
		if (vlaninfo == 0) {
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
					"Unable to check Configuration Consistency for Switch Board having key %s: [VLAN TO CHECK NOT FOUND]",
					switchboard_key.c_str());
			return fixs_ith::ERR_VLAN_NOT_FOUND_IN_CONTEXT;
		}

	}

	/*if(common::utility::isSwitchBoardSMX())
	 {
	 if(vlaninfo == 0)
	 {
	 FIXS_ITH_LOG(LOG_LEVEL_ERROR,"Unable to check Configuration Consistency for Switch Board having key %s: [VLAN TO CHECK NOT FOUND]",switchboard_key.c_str());
	 return fixs_ith::ERR_VLAN_NOT_FOUND_IN_CONTEXT;
	 }
	 }


	 if(common::utility::isSwitchBoardCMX())
	 {
	 if(vlaninfo == 0)
	 {
	 FIXS_ITH_LOG(LOG_LEVEL_ERROR,"Unable to check Configuration Consistency for Switch Board having key %s: [VLAN TO CHECK NOT FOUND]",switchboard_key.c_str());
	 return fixs_ith::ERR_VLAN_NOT_FOUND_IN_CONTEXT;
	 }
	 }*/

	// now retrieve from SNMP manager the info about the selected VLAN
	fixs_ith_switchInterfaceAdapter swi_adapter;
	std::set<uint16_t> sb_tagged_ports;
	std::set<uint16_t> sb_untagged_ports;
	unsigned int op_timeout = 5000;

	// get info about the TAGGED and UNTAGGED ports of the selected VLAN
	int get_ports_res;
	if (isSMX) {

		// get info about the TAGGED and UNTAGGED ports of the selected VLAN
		get_ports_res = swi_adapter.get_vlan_ports(sb_tagged_ports,
				sb_untagged_ports, vlaninfo->getVlanId(), switchboard_key,
				fixs_ith::CONTROL_PLANE, &op_timeout);

	}

	if (isCMX) {

		// get info about the TAGGED and UNTAGGED ports of the selected VLAN
		get_ports_res = swi_adapter.get_vlan_ports(sb_tagged_ports,
				sb_untagged_ports, vlaninfo->getVlanId(), switchboard_key,
				fixs_ith::TRANSPORT_PLANE, &op_timeout);

	}

	// if all info are available, perform the consistency check
	bool inconsistency_detected = false;
	if (get_ports_res == switch_interface::SBIA_OK) {
		// compare vlan configurations between Infrastructure MOM and SMX
		std::set<uint16_t> imm_tagged_ports(
				vlaninfo->getTaggedBridgePortsIndex());
		std::set<uint16_t> imm_untagged_ports(
				vlaninfo->getUntaggedBridgePortsIndex());

		if ((imm_tagged_ports.size() != sb_tagged_ports.size())
				|| (imm_untagged_ports.size() != sb_untagged_ports.size())
				|| (!std::equal(imm_tagged_ports.begin(),
						imm_tagged_ports.end(), sb_tagged_ports.begin()))
				|| (!std::equal(imm_untagged_ports.begin(),
						imm_untagged_ports.end(), sb_untagged_ports.begin()))) {
			inconsistency_detected = true;
		}
	} else if ((get_ports_res == switch_interface::SBIA_OBJ_NOT_FOUND)) {
		inconsistency_detected = true;
	} else {// check not executable because we were unable to get VLAN data from SMX  !!
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"Unable to check Configuration Consistency for Switch Board having key %s: [UNABLE TO GET VLAN PORTS FROM SWITCH BOARD]",
				switchboard_key.c_str());
		return fixs_ith::ERR_GENERIC;
	}

	// Set the priority of all the ports to default priority in case the switchboard is CMX
	if (isCMX) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"Setting the priority of ports on switch board %d",
				switchboard_info.slot_position);
		uint16_t end_if_index = 174;
		uint16_t increment = 5;
		for (uint16_t if_index = 1; if_index <= end_if_index;
				if_index = if_index + increment) {
			if (if_index != 121 && if_index != 126) {
				const int result =
						fixs_ith::workingSet_t::instance()->get_snmpmanager().set_mib_port_default_user_priority(
								switchboard_info, if_index);
				if (result != fixs_ith::ERR_NO_ERRORS) {
					FIXS_ITH_LOG(LOG_LEVEL_ERROR,
							"Call 'set_port_default_user_priority' failed: cannot set the priority of index == %d on CMXB present at slot postion == %d",
							if_index, switchboard_info.slot_position);
					inconsistency_detected = true;

				} else
					FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
							"Call 'set_port_default_user_priority' successful: Default priority is set for the port at index == %d on CMXB present at slot postion == %d",
							if_index, switchboard_info.slot_position);
			}
			if (if_index == 171)
				increment = 1;
			if (inconsistency_detected)
				break;

		}
	}

	if (inconsistency_detected) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"Configuration Consistency completed for Switch Board having key %s: [MISCONFIGURATION DETECTED]",
				switchboard_key.c_str());
		return fixs_ith::ERR_CONFIG_INCONSISTENCY_FOUND;
	}

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
			"Configuration Consistency completed for Switch Board having key %s: [NO CONFIG ISSUES FOUND]",
			switchboard_key.c_str());
	return fixs_ith::ERR_NO_ERRORS;
}

} /* namespace operation */

