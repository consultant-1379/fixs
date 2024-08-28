/*
 * pim_adaptation.cpp
 *
 *  Created on: Feb 7, 2017
 *      Author: xclaele
 */


#include "operation/pim_adaptation.h"

#include "fixs_ith_programconstants.h"
#include "switch_interface/full_adapter.h"
#include "fixs_ith_logger.h"
#include "fixs_ith_workingset.h"

namespace operation {

pim_adaptation::pim_adaptation():
OperationBase(operation::PIM_ADAPTATION),
m_switchboard_key(),
m_ifIndex(0),
m_sb_plane(fixs_ith::TRANSPORT_PLANE)
{

}

void pim_adaptation::setOperationDetails(const void* op_details)
{
	const op_parameters * parameter = reinterpret_cast<const op_parameters*>(op_details);
	m_switchboard_key.assign(parameter->switchboard_key);
	m_ifIndex = parameter->ifIndex;
	m_sb_plane = parameter->sb_plane;
}

int pim_adaptation::call()
{
	int sid = syscall(SYS_gettid);
	FIXS_ITH_LOG(LOG_LEVEL_INFO, "Thread (%d) PIM ADAPTATION' operation requested for context", sid);
	
	FIXS_ITH_LOG(LOG_LEVEL_INFO, " ->>> 'PIM ADAPTATION' operation requested for context:<%s>", m_switchboard_key.empty() ? "ALL" : m_switchboard_key.c_str());

	// if the operation has been requested on a specific SFP port of a specific switch board, perform the procedure only for the specified SFP port,
	// otherwise, perform it for all A2-A8 SFP ports in the configuration

	fixs_ith::ErrorConstants call_result = fixs_ith::ERR_NO_ERRORS;

	if (m_switchboard_key.empty()) // For all switch boards
	{
		std::set<std::string> switchBoardKeys;
		// Load Switchboard Keys from CS
		if (fixs_ith::workingSet_t::instance()->get_sbdatamanager().get_switchboard_keys(switchBoardKeys))
		{ // ERROR
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "UNABLE to load switchboard keys from CS");
			call_result = fixs_ith::ERR_CS_ERROR;
		}
		else
		{
			for (std::set<std::string>::iterator sb_it = switchBoardKeys.begin(); switchBoardKeys.end() != sb_it; ++sb_it)
			{
				engine::contextAccess_t contextAccess(*sb_it, engine::GET_EXISTING, engine::SHARED_ACCESS);
				engine::Context* context = contextAccess.getContext();

				if (context)
				{
					fixs_ith::SwitchBoardPlaneConstants planes[] = {fixs_ith::CONTROL_PLANE, fixs_ith::TRANSPORT_PLANE};
					std::list<fixs_ith::SwitchBoardPlaneConstants> planes_list(planes, planes + sizeof(planes)/sizeof(fixs_ith::SwitchBoardPlaneConstants));

					for(std::list<fixs_ith::SwitchBoardPlaneConstants>::iterator p_it = planes_list.begin(); p_it != planes_list.end(); ++p_it)
					{
						std::set<uint16_t> ifIndexes = context->getEthernetPortsIndexes(*p_it);

						for(std::set<uint16_t>::iterator if_it = ifIndexes.begin(); if_it != ifIndexes.end(); ++if_it)
						{
							pim_adaptation::op_parameters op_params;
							op_params.switchboard_key = *sb_it;
							op_params.ifIndex = *if_it;
							op_params.sb_plane = *p_it;
							fixs_ith::workingSet_t::instance()->schedule_until_completion(*sb_it, operation::PIM_ADAPTATION, 0, 3, &op_params, sizeof(op_params));
						}
					}
				}
                                else
                                {
                                   FIXS_ITH_LOG(LOG_LEVEL_INFO,"pim_adaptation::call() context is null, setting call_result as error");
                                   call_result = fixs_ith::ERR_NULL_POINTER;
                                }
			}
		}
	}
	else if (0 == m_ifIndex) // For all Ethernet ports
	{
		engine::contextAccess_t contextAccess(m_switchboard_key, engine::GET_EXISTING, engine::SHARED_ACCESS);
		engine::Context* context = contextAccess.getContext();

		if (context)
		{
			fixs_ith::SwitchBoardPlaneConstants planes[] = {fixs_ith::CONTROL_PLANE, fixs_ith::TRANSPORT_PLANE};
			std::list<fixs_ith::SwitchBoardPlaneConstants> planes_list(planes, planes + sizeof(planes)/sizeof(fixs_ith::SwitchBoardPlaneConstants));

			for(std::list<fixs_ith::SwitchBoardPlaneConstants>::iterator p_it = planes_list.begin(); p_it != planes_list.end(); ++p_it)
			{
				std::set<uint16_t> ifIndexes = context->getEthernetPortsIndexes(*p_it);

				for(std::set<uint16_t>::iterator if_it = ifIndexes.begin(); if_it != ifIndexes.end(); ++if_it)
				{
					pim_adaptation::op_parameters op_params;
					op_params.switchboard_key = m_switchboard_key;
					op_params.ifIndex = *if_it;
					op_params.sb_plane = *p_it;
					fixs_ith::workingSet_t::instance()->schedule_until_completion(m_switchboard_key, operation::PIM_ADAPTATION, 0, 3, &op_params, sizeof(op_params));
				}
			}
		}
                else
                {
                   FIXS_ITH_LOG(LOG_LEVEL_INFO,"pim_adaptation::call() context is null, setting call_result as error");
                   call_result = fixs_ith::ERR_NULL_POINTER;
                }
	}
	else if(isSFPport())
	{
		FIXS_ITH_LOG(LOG_LEVEL_INFO, "[%s][%s] Calling 'm_pim_adaptation()' for ifIndex:<%d>",
				m_switchboard_key.c_str(), common::utility::planeToString(m_sb_plane).c_str(), m_ifIndex);

		// perform the pim procedure
		call_result = m_pim_adaptation(m_switchboard_key, m_sb_plane, m_ifIndex);

		if(fixs_ith::ERR_NO_ERRORS != call_result)
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] Call 'm_pim_adaptation()' failed for ifIndex:<%d>",
					m_switchboard_key.c_str(), common::utility::planeToString(m_sb_plane).c_str(), m_ifIndex);
		}
	}
	else
	{
		FIXS_ITH_LOG(LOG_LEVEL_INFO, "[%s][%s] Calling 'm_set_auto_neg_admin_status()' for ifIndex:<%d>",
				m_switchboard_key.c_str(), common::utility::planeToString(m_sb_plane).c_str(), m_ifIndex);

		// set auto negotiation admin status
		m_set_auto_neg_admin_status(m_switchboard_key, m_sb_plane, m_ifIndex);
	}


	// set result to caller
	m_operationResult.setErrorCode(call_result);
	setResultToCaller();
	return fixs_ith::ERR_NO_ERRORS;
}

fixs_ith::ErrorConstants pim_adaptation::m_pim_adaptation(const std::string & switchboard_key, const fixs_ith::SwitchBoardPlaneConstants& plane, const uint16_t& ifIndex, bool isInContext, bool isAutoNegToBeEnabled)
{
	/* PIM ADAPTATION PROCEDURE:
	 *
	 * 1) Read the MAU-MIB::ifMauTypeListBits.ifIndex.1 object where "ifIndex" is the port number associated with the PiM
	 *
	 * 2) Read the value of the "AutoNegotiate" attribute of the EthernetPort MOC instance related to the PiM
	 *
	 * 3) Set the MAU-MIB::ifMauDefaultType.ifIndex.1 in line with the value read at step 1
	 * 
	 * 4) If the value read in step 1 indicates "b1000baseSXFD" or "b1000baseLXFD" and the value read at step 2 is "ENABLED" then enable auto-negotiation
	 *	  on the SMX using the object MAU-MIB::ifMauAutoNegAdminStatus.ifIndex.1; Otherwise, that is "AutoNegotiate" attribute is set to "DISABLED" or
	 *	  MAU-MIB::ifMauTypeListBits.ifIndex.1 value is not equal to the indicated ones, disable auto-negotiation on the SMX using the object
	 *	  MAU-MIB::ifMauAutoNegAdminStatus.ifIndex.1
	 *
	 * 
	 *
	 */

	// STEP 0
	fixs_ith_switchboardinfo switch_board;
	if(int call_result = fixs_ith::workingSet_t::instance()->get_sbdatamanager().get_switchboard_info(switch_board, switchboard_key))
	{
		// ERROR: On retrieving info for the current Switch/SMX board
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "ERROR retrieving switch board info --> Switch/SMX board:<%s>, call_result:<%d>. "
				"PiM adaptation procedure NOT started!", switchboard_key.c_str(), call_result);
		return fixs_ith::ERR_CS_NO_BOARD_FOUND;
	}


	// STEP 1
	fixs_ith_snmpmanager & snmpmanager = fixs_ith::workingSet_t::instance()->get_snmpmanager();
	std::vector<bool> if_mau_type;
	uint32_t timeout_ms = DEFAULT_SNMP_TIMEOUT_MS;
	if(int call_result = snmpmanager.get_if_mau_type(if_mau_type, ifIndex, switch_board, plane, &timeout_ms))
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "ERROR retrieving MAU type info --> Switch/SMX board:<%s>, ifIndex:<%d>, call_result:<%d>. "
				"PiM adaptation procedure stopped at STEP 1!", switchboard_key.c_str(), ifIndex, call_result);
		return fixs_ith::ERR_GENERIC;
	}


	// STEP 2
	bool is_auto_neg_enabled =  false;
	bool mau_auto_neg_status_enabled = false;

	if(!isInContext)
	{
		if(int call_result = fixs_ith::workingSet_t::instance()->getConfigurationManager().getEthPortAutoNegotiateValue(switchboard_key, plane, ifIndex, is_auto_neg_enabled))
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "ERROR retrieving 'AutoNegotiate' value --> Switch/SMX board:<%s>, ifIndex:<%d>, call_result:<%d>. "
					"PiM adaptation procedure stopped at STEP 2!", switchboard_key.c_str(), ifIndex, call_result);
			return fixs_ith::ERR_GENERIC;
		}
	}
	else
	{
		is_auto_neg_enabled = isAutoNegToBeEnabled;
	}


	std::vector<bool>::reverse_iterator rit;
	is_auto_neg_enabled = (is_auto_neg_enabled != if_mau_type[fixs_ith_snmp::IF_MAU_TYPE_1000_BASE_TFD]);
	size_t upper_flag_pos = 0;
	rit = std::find(if_mau_type.rbegin(), if_mau_type.rend(), true);
	if (rit != if_mau_type.rend())
		upper_flag_pos = rit.base() - if_mau_type.begin() - 1;
	fixs_ith_snmp::IfMauTypeListBitsConstants if_mau_default_type = (fixs_ith_snmp::IfMauTypeListBitsConstants)upper_flag_pos;

	// TR HX22681
	
	if(if_mau_default_type)
	{
		
		// STEP 3
		if(int call_result = snmpmanager.set_if_mau_default_type(if_mau_default_type, ifIndex, switch_board, plane))
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "ERROR setting default MAU type --> Switch/SMX board:<%s>, ifIndex:<%d>, ifMauDefaultType:<%d>, call_result:<%d>. "
					"PiM adaptation procedure stopped at STEP 3!", switchboard_key.c_str(), ifIndex, if_mau_default_type, call_result);
			return fixs_ith::ERR_GENERIC;
		}
		
		// STEP 4
		int result = -1;
		if(is_auto_neg_enabled && (if_mau_type[fixs_ith_snmp::IF_MAU_TYPE_1000_BASE_LXFD] || if_mau_type[fixs_ith_snmp::IF_MAU_TYPE_1000_BASE_SXFD] || if_mau_type[fixs_ith_snmp::IF_MAU_TYPE_1000_BASE_TFD]))
		{
			result = snmpmanager.set_if_mau_auto_neg_admin_status(fixs_ith_snmp::FIXS_ITH_MAU_AUTO_NEG_STATUS_ENABLED, ifIndex, switch_board, plane);
			mau_auto_neg_status_enabled = true;
		}
		else
		{
			result = snmpmanager.set_if_mau_auto_neg_admin_status(fixs_ith_snmp::FIXS_ITH_MAU_AUTO_NEG_STATUS_DISABLED, ifIndex, switch_board, plane);
		}

		if(result)
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "ERROR setting auto negotiation admin status --> Switch/SMX board:<%s>, ifIndex:<%d>, call_result:<%d>. "
					"PiM adaptation procedure stopped at STEP 4!", switchboard_key.c_str(), ifIndex, result);
			return fixs_ith::ERR_GENERIC;
		}

	}

	FIXS_ITH_LOG(LOG_LEVEL_INFO, "PiM adaptation procedure successfully completed --> Switch/SMX board:<%s>, ifIndex:<%d>, ifMauAutoNegAdminStatus:<%s>, ifMauDefaultType:<%d>",
			switchboard_key.c_str(), ifIndex, mau_auto_neg_status_enabled ? "ENABLED" : "DISABLED", if_mau_default_type);

	return fixs_ith::ERR_NO_ERRORS;
}

fixs_ith::ErrorConstants pim_adaptation::m_set_auto_neg_admin_status(const std::string& switchboard_key, const fixs_ith::SwitchBoardPlaneConstants& plane, const uint16_t& ifIndex)
{
	// STEP 0
	fixs_ith_switchboardinfo switch_board;
	if(int call_result = fixs_ith::workingSet_t::instance()->get_sbdatamanager().get_switchboard_info(switch_board, switchboard_key))
	{
		// ERROR: On retrieving info for the current Switch/SMX board
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "ERROR retrieving switch board info --> Switch/SMX board:<%s>, call_result:<%d>",
				switchboard_key.c_str(), call_result);
		return fixs_ith::ERR_CS_NO_BOARD_FOUND;
	}

	// STEP 1
	bool is_auto_neg_enabled =  false;
	if(int call_result = fixs_ith::workingSet_t::instance()->getConfigurationManager().getEthPortAutoNegotiateValue(switchboard_key, plane, ifIndex, is_auto_neg_enabled))
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "ERROR retrieving 'AutoNegotiate' value --> Switch/SMX board:<%s>, ifIndex:<%d>, call_result:<%d>. "
				"Call 'm_set_auto_neg_admin_status' failed.", switchboard_key.c_str(), ifIndex, call_result);
		return fixs_ith::ERR_GENERIC;
	}

	// STEP 2
	fixs_ith_snmpmanager & snmpmanager = fixs_ith::workingSet_t::instance()->get_snmpmanager();
	if(int call_result = snmpmanager.set_if_mau_auto_neg_admin_status(
			is_auto_neg_enabled ? fixs_ith_snmp::FIXS_ITH_MAU_AUTO_NEG_STATUS_ENABLED : fixs_ith_snmp::FIXS_ITH_MAU_AUTO_NEG_STATUS_DISABLED,
					ifIndex, switch_board, plane))
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "[%s][%s] ERROR setting auto negotiation admin status for ifIndex:<%d>, call_result:<%d>",
				switchboard_key.c_str(), common::utility::planeToString(plane).c_str(), ifIndex, call_result);
		return fixs_ith::ERR_GENERIC;
	}

	FIXS_ITH_LOG(LOG_LEVEL_INFO, "[%s][%s] Set of auto negotiation successfully completed for ifIndex:<%d>, ifMauAutoNegAdminStatus:<%s>",
			switchboard_key.c_str(), common::utility::planeToString(plane).c_str(), ifIndex, is_auto_neg_enabled ? "ENABLED" : "DISABLED");

	return fixs_ith::ERR_NO_ERRORS;
}

} /* namespace operation */
