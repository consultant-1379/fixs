/*
 * check_config_consistency.cpp
 *
 *  Created on: Dec 09, 2016
 *      Author: xpamsur
 */


#include "operation/check_sfpport_consistency.h"

#include "fixs_ith_programconstants.h"
#include "switch_interface/full_adapter.h"
#include "fixs_ith_logger.h"
#include "fixs_ith_workingset.h"
#include "fixs_ith_snmpmanager.h"
#include "fixs_ith_sbdatamanager.h"
#include "netconf/netconf.h"
#include <cstring>


namespace operation {

check_sfpport_consistency::check_sfpport_consistency():
		OperationBase(CHECK_SFPPORT_CONSISTENCY),
		m_switchboard_key()
{

}

void check_sfpport_consistency::setOperationDetails(const void* op_details)
{
	const char* key = reinterpret_cast<const char*>(op_details);

	m_switchboard_key.assign(key);
}

int check_sfpport_consistency::call()
{
	FIXS_ITH_LOG(LOG_LEVEL_INFO, " ->>> 'CHECK SFPPORT CONSISTENCY' operation requested. Context <%s>", m_switchboard_key.empty() ? "ALL": m_switchboard_key.c_str());

	fixs_ith_snmpmanager & snmpmanager = fixs_ith::workingSet_t::instance()->get_snmpmanager();
	fixs_ith_switchboardinfo switch_board;
	int call_result = -1;
	char switch_board_ipn_str_addresses [FIXS_ITH_CONFIG_IPN_COUNT][16] = {};
	uint32_t timeout_ms = DEFAULT_SNMP_TIMEOUT_MS;

	std::set<std::string> switchBoardKeys;
	call_result =  fixs_ith::workingSet_t::instance()->get_sbdatamanager().get_switchboard_keys(switchBoardKeys);
	if(call_result != fixs_ith_sbdatamanager::SBM_OK)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,"Unable to load Switch Board keys ");
		m_operationResult.setErrorCode(fixs_ith::ERR_CS_ERROR);
		setResultToCaller();
		return fixs_ith::ERR_CS_ERROR;
	}

	for (std::set<std::string>::const_iterator cit = switchBoardKeys.begin(); cit != switchBoardKeys.end(); ++cit)
	{
		if(int call_result = fixs_ith::workingSet_t::instance()->get_sbdatamanager().get_switchboard_info(switch_board, *cit))
		{
			// ERROR: On retrieving info for the current Switch/SMX board
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "ERROR: Retrieving data info about the Switch/SMX board <%s> : call_result == <%d>. "
					"Interface Port fault alarm not raised!", cit->c_str(), call_result);
			m_operationResult.setErrorCode(call_result);
			setResultToCaller();
			return call_result;
		}

		if(fixs_ith_sbdatamanager::get_switch_board_target_ipns (switch_board_ipn_str_addresses[0], switch_board_ipn_str_addresses[1],
				switch_board, fixs_ith::TRANSPORT_PLANE)) {
			m_operationResult.setErrorCode(netconf::ERR_IP_ADDRESS_NOT_FOUND);
			setResultToCaller();
			return netconf::ERR_IP_ADDRESS_NOT_FOUND;
		}

		FIXS_ITH_LOG(LOG_LEVEL_INFO, "IPNA == %s and IPNB == %s",switch_board_ipn_str_addresses[0], switch_board_ipn_str_addresses[1]);

		::srandom(static_cast<unsigned>(::time(0)));
		int ipn_index = ::random() & 0x01;
		int16_t port_presencestatus=-1,interface_num=-1;

		for (interface_num=ITH_SNMP_COMMUNITY_PIM_PORT_START;interface_num<=ITH_SNMP_COMMUNITY_PIM_PORT_END;interface_num++)
		{
			if (interface_num == ITH_SNMP_COMMUNITY_PIM_PORT_NOT_EXIST)
				continue;
			call_result=fixs_ith_snmp::ERR_SNMP_REQUEST_NOT_SENT;
			for (int i = 0; (snmpmanager.isCommunicationFailure(call_result) && i < 2) ; ++i, ipn_index = (ipn_index + 1) & 0x01) {
				call_result = snmpmanager.get_pimPresence_status(port_presencestatus, switch_board.magazine,switch_board.slot_position,switch_board_ipn_str_addresses[ipn_index],&timeout_ms,interface_num);
				if (call_result)
					FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
						"Port PIM get info: Call 'get_pimPresence_status' failed: switch board slot == %d, switch board IPN = '%s': call_result == %d",switch_board.slot_position, switch_board_ipn_str_addresses[ipn_index], call_result);
				else {
					if ((call_result = raiseCeasePimAlarm(interface_num,port_presencestatus,switch_board.magazine,switch_board.slot_position,switch_board_ipn_str_addresses[ipn_index],&timeout_ms,*cit))) {
						FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'raiseCeasePimAlarm' failed: call_result == %dand switchboard key==%s", call_result,cit->c_str());
					}
				}
			}//end of FOR with ipn_index as var
		} //end of FOR with interface_num var

	}

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "############################################################################################ \n"
			"PIM STATUS FETCHING COMPLETED..RESCHEDULED AGAIN..WAIT FOR 2 mins\n"
			"############################################################################################  \n");
	m_operationResult.setErrorCode(fixs_ith::ERR_FOREVER_OP); //intentionally thrown an error to reschedule polling mechanism
	setResultToCaller();
	return fixs_ith::ERR_NO_ERRORS;
}

int check_sfpport_consistency::raiseCeasePimAlarm(int16_t if_index, int16_t & port_presencestatus,uint32_t board_magazine,int32_t board_slot,const char * switch_board_ip,const unsigned * timeout_ms,const std::string & switchboard_key)
{
	engine::contextAccess_t access(switchboard_key, engine::GET_EXISTING, engine::SHARED_ACCESS);
	engine::Context * context = access.getContext();

	if (!context) {
		// The context is not ready.
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "ERROR: context == <NULL>: The context retrieved for SHARED_ACCESS is not ready. "
				"PIM fault alarm not raised/ceased!switchkey=%s",switchboard_key.c_str());
		return fixs_ith::ERR_GENERIC;
	}

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "context == <0x%p>", reinterpret_cast<void *>(context));

	if (boost::shared_ptr<engine::EthernetPort> if_port = context->getEthernetPortByIndex(fixs_ith::TRANSPORT_PLANE, if_index)) {
		std::string if_port_dn = if_port->getDN();
		std::string if_port_rdn_value;

		if (!common::utility::getRDNValue(if_port_dn, imm::ethernetPort_attribute::RDN.c_str(), if_port_rdn_value)) {
			// ERROR: Fetching the RDN value from the ethernet port object DN
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "ERROR: Fetching the RDN value from the ethernet port object DN. if_port_dn == <%s>. "
					"PIM fault alarm not raised!", if_port_dn.c_str());
			return fixs_ith::ERR_GENERIC;
		}

		std::string if_port_rdn = common::utility::getRDNAttribute(if_port_dn) + "=" + if_port_rdn_value;
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "if_port_dn == <%s>, if_port_rdn == <%s>, if_port_rdn_value == <%s>",
				if_port_dn.c_str(), if_port_rdn.c_str(), if_port_rdn_value.c_str());

		std::string magazine;
		std::string slot;
		std::string port_name;

		if (common::utility::getEthernetPortPositionFromRdnValue(magazine, slot, port_name, if_port_rdn_value)) {
			// ERROR: Ethernet port RDN malformed
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "ERROR: Getting the interface port coordinates {magazine, slot, port} from RDN value. "
					"if_port_dn == <%s>, if_port_rdn == <%s>, if_port_rdn_value == <%s>. "
					"PIM fault alarm not raised/ceased!", if_port_dn.c_str(), if_port_rdn.c_str(), if_port_rdn_value.c_str());
			return fixs_ith::ERR_GENERIC;
		}

		fixs_ith_snmpmanager & snmpmanager = fixs_ith::workingSet_t::instance()->get_snmpmanager();
		int16_t port_matchingstatus=-1;
		if (port_presencestatus == fixs_ith_snmp::PRESENT){
			snmpmanager.get_pimMatching_status(port_matchingstatus,board_magazine,board_slot,if_index,switch_board_ip,timeout_ms);
		}

		if (port_presencestatus==fixs_ith_snmp::ABSENT)
			fixs_ith::workingSet_t::instance()->getAlarmHandler().ceasePimFaultAlarm(if_port_rdn);
		else if((port_matchingstatus==fixs_ith_snmp::MISMATCH))
			fixs_ith::workingSet_t::instance()->getAlarmHandler().raisePimFaultAlarm(if_port_rdn, magazine, slot, port_name);
		else if((port_presencestatus==fixs_ith_snmp::PRESENT) && (port_matchingstatus==fixs_ith_snmp::MATCH)){
			fixs_ith::workingSet_t::instance()->getAlarmHandler().ceasePimFaultAlarm(if_port_rdn);
		}
		else {
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "DEBUG: PIM fault alarm not raised/ceased! with portPresenceStatus== <%d>, portMatchingStatus== <%d> ",port_presencestatus,port_matchingstatus );
		}
	} else {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "ERROR: Interface Port at index <%d> not found in the empty context. "
				"PIM fault alarm not raised/ceased!", if_index);
		return fixs_ith::ERR_GENERIC;
	}

	return fixs_ith::ERR_NO_ERRORS;
}

} /* namespace operation */

