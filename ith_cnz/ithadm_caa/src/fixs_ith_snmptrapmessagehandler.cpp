/*
 * fixs_ith_snmptrapmessagehandler.cpp
 *
 *  Created on: Feb 4, 2016
 *      Author: xassore
 */

#include "fixs_ith_logger.h"
#include "fixs_ith_snmpconstants.h"
#include "fixs_ith_programmacros.h"
#include "fixs_ith_csreader.h"
#include "fixs_ith_snmpmanagercallbacks.h"
#include "fixs_ith_snmpmanager.h"
#include "fixs_ith_workingset.h"

#include "engine/context.h"
#include "engine/interfaceipv4.h"
#include "engine/bfdsessionipv4.h"

#include "fixs_ith_snmptrapmessagehandler.h"
#include "fixs_ith_trapdsobserver.h"
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/library/snmp_transport.h>

// Valorize attribute (TRAP)
const oid m_oidSnmpTrap[] = { 1, 3, 6, 1, 6, 3, 1, 1, 4, 1, 0 };
oid m_oidColdStart[] = { 1, 3, 6, 1, 6, 3, 1, 1, 5, 1 };
oid m_oidWarmRestart[] = { 1, 3, 6, 1, 4, 1, 193, 177, 2, 2, 5, 0, 4 };
// (VALUE)

oid m_oidContFileValue[] = { 1, 3, 6, 1, 4, 1, 193, 177, 2, 2, 4, 2, 1, 0 };
oid m_oidContFileResult[] = { 1, 3, 6, 1, 4, 1, 193, 177, 2, 2, 4, 2, 2, 0 };
oid m_oidContFileTransferComplete[] = { 1, 3, 6, 1, 4, 1, 193, 177, 2, 2, 4, 1,
		2 };

namespace {
oid oid_trap_scx_ColdStart[] = { FIXS_ITH_SMX_OID_TRAP_COLD_START };
oid oid_trap_scx_WarmStart[] = { FIXS_ITH_SMX_OID_TRAP_WARM_START };
oid oid_trap_scx_VRRPMasterChange[] =
		{ FIXS_ITH_SMX_OID_TRAP_VRRP_MASTER_CHANGE };
oid oid_trap_scx_InterfaceLinkDown[] = { FIXS_ITH_SMX_OID_TRAP_LINK_DOWN };
oid oid_trap_scx_InterfaceLinkUp[] = { FIXS_ITH_SMX_OID_TRAP_LINK_UP };

oid oid_genVrrpVrId[] = { SNMP_GEN_VRRP_VRID };
oid oid_trap_smx_pim[] = { FIXS_ITH_SMX_OID_TRAP_PIM };

inline int is_front_port_to_monitor(const std::string & port_name) {
	//      The interface names:
	//      BP_n refers to the backplane port connected to slot n.
	//      A1-4, A5-8 are front SFP+ ports setup in combined mode
	//      A[1-8] are front SFP+ ports setup in individual mode
	//      B[1-2]_x-y are front MTP ports setup in combined mode
	//      AUXn and CONTROLn are internal SMX ports.

	if (common::utility::isSwitchBoardSMX()) {
		return (((port_name.length() > 1) && (std::toupper(port_name[0]) == 'A')
				&& std::isdigit(port_name[1]) && ((port_name[1] - '0') >= 2)) ?
				1 : 0);
	}

	else if (common::utility::isSwitchBoardCMX()) {
		std::string cmx_front_ports[12] = { "E_1", "E_2", "E_3", "E_4", "E_5",
				"E_6", "E_7", "E_8", "GE_1", "GE_2", "GE_3", "GE_4" };
		int32_t itr = 0;
		while (itr <= 11) {
			if ((port_name.compare(cmx_front_ports[itr])) == 0) {
				return 1;
			}
			itr++;
		}
		return 0;
	} else
		return 1;
}
}
namespace BFD_MIB {
oid oid_trap_smx_SessionUp[] = { FIXS_ITH_SMX_OID_TRAP_BFD_SESS_UP };
oid oid_trap_smx_SessionDown[] = { FIXS_ITH_SMX_OID_TRAP_BFD_SESS_DOWN };

oid oid_trap_smx_RouterId[] = { SNMP_GEN_BFD_ROUTERID };
oid oid_trap_smx_IFIndex[] = { SNMP_GEN_BFD_IF_INDEX };
oid oid_trap_smx_SessionDiagnostic[] = { SNMP_GEN_BFD_SESS_DIAG };
oid oid_trap_smx_PeerIPType[] = { SNMP_GEN_BFD_PEER_IP_ADDR_TYPE };
oid oid_trap_smx_PeerIP[] = { SNMP_GEN_BFD_PEER_IP_ADDR };
}
namespace BFD_SESSION_STATE {
enum {
	noDiagnostic = 0U,
	controlDetectionTimeExpired,
	echoFunctionFailed,
	neighborSignaledSessionDown,
	forwardingPlaneReset,
	pathDown,
	concatenatedPathDown,
	administrativelyDown,
	reverseConcatenatedPathDown,
	misConnectivityDefect
};
}

int fixs_ith_snmptrapmessagehandler::handle_trap(
		ACS_TRAPDS_StructVariable & pdu) {
	FIXS_ITH_LOG(LOG_LEVEL_INFO,
			"A new trap received from trap handle_trap service");
	FIXS_ITH_LOG(LOG_LEVEL_INFO,
			"Trap PDU message handler: callback called: a new trap received from trap dispatcher service");

	trapds_var_list_t & var_list = pdu.nvls;
	trapds_var_list_t::iterator var_it = var_list.begin();
	trapds_var_list_t::iterator var_list_end = var_list.end();
	const char * source_trap_ip = var_it->ipTrap.c_str();
	ACS_TRAPDS_StructVariable v2 = pdu; //GEP2-GEP5 HW SWAP feature
	bool isSMX = false, isCMX = false; //GEP2-GEP5 HW SWAP feature

	// Check: PDU is empty?
	if (var_it++ == var_list_end) { // ERROR: PDU is empty
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"The received trap PDU message doesn't keep any trap data: an enpty PDU trap message received");
		return fixs_ith_snmp::ERR_SNMP_PDU_NO_VARIABLE_BOUND;
	}

	// Note: At previous if the iterator was forwarded by one position because the trap
	// oid received is at position two.
	// Check the trap oid received if any.
	if (var_it == var_list_end) { // ERROR: no trap oid in PDU
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"No trap oid into the second position of the PDU");
		return fixs_ith_snmp::ERR_SNMP_NOT_AXE_TRAP;
	}

	if (common::utility::isSwitchBoardSMX()) {
		isSMX = true;
	} else if (common::utility::isSwitchBoardCMX()) {
		isCMX = true;
	} else {
		return fixs_ith::ERR_GENERIC;
	}

	std::string board_key;
	if (!getSmxIdFromIp(source_trap_ip, board_key)) {
		// WARNING: The Switch/SMX board in input not found on configuration manager
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"ERROR: No Switch/SMX board found into the configuration manager with address <%s>. ",
				source_trap_ip);
		return fixs_ith::ERR_GENERIC;
	}

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
			"The Switch/SMX board with address <%s> has key == <%s>",
			source_trap_ip, board_key.c_str());

	if (isSMX) {
		oid snmp_trap_oid[] = { FIXS_ITH_SNMP_TRAP_OID };

		if (fixs_ith_snmpmanager::oid_compare(var_it->name.oids,
				var_it->name.length - 1, snmp_trap_oid,
				FIXS_ITH_ARRAY_SIZE(snmp_trap_oid))) {
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
					"Error in the received pdu: the pdu doesn't contain a snmpTrap message");
			char buffer[2 * 1024] = { 0 };
			oid_to_string(buffer, FIXS_ITH_ARRAY_SIZE(buffer),
					var_it->val.objid.oids, var_it->val.objid.length);

			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Trap oid: %s", buffer);
			return fixs_ith_snmp::ERR_SNMP_NOT_PDU_TRAP;
		}

	}
	if (isCMX) {
		int contFileResult = 0;
		std::string contFileNameValue("");
		bool CMXLogFlag = false;
		bool isCold = false;
		bool isWarm = false;
		bool isVrrpMasterChange = false;
		bool isBFDSessionUp = false;
		bool isBFDSessionDown = false;
		bool isIfPortDown = false;
		bool isIfPortUP = false;

		// Init
		ACS_TRAPDS_varlist td;

		for (std::list<ACS_TRAPDS_varlist>::iterator it = v2.nvls.begin();
				it != v2.nvls.end(); it++) {

			td = *it;
			std::string td_name_oid("");
			OidToString(td.name.oids, td.name.length, &td_name_oid);
			FIXS_ITH_LOG(LOG_LEVEL_INFO, "oid_1:<%s>", td_name_oid.c_str());

			if (fixs_ith_snmpmanager::oid_compare(td.name.oids, td.name.length,
					m_oidSnmpTrap, FIXS_ITH_ARRAY_SIZE(m_oidSnmpTrap)) == 0) {
				FIXS_ITH_LOG(LOG_LEVEL_INFO, "oid_3");

				std::string td_oid("");
				OidToString(td.val.objid.oids, td.val.objid.length, &td_oid);
				FIXS_ITH_LOG(LOG_LEVEL_INFO, "oid_4:<%s>", td_oid.c_str());

				if (fixs_ith_snmpmanager::oid_compare(td.val.objid.oids,
						td.val.objid.length, m_oidColdStart,
						FIXS_ITH_ARRAY_SIZE(m_oidColdStart)) == 0) {
					std::string td_oid("");
					OidToString(td.val.objid.oids, td.val.objid.length,
							&td_oid);
					FIXS_ITH_LOG(LOG_LEVEL_ERROR, "oid_ColdStarti : %s",
							td_oid.c_str());
					std::string cold_start_oid("");
					OidToString(m_oidColdStart,
							FIXS_ITH_ARRAY_SIZE(m_oidColdStart),
							&cold_start_oid);
					FIXS_ITH_LOG(LOG_LEVEL_INFO, "cold_start_oid: %s",
							cold_start_oid.c_str());
					isCold = true;
				}

				if (fixs_ith_snmpmanager::oid_compare(td.val.objid.oids,
						td.val.objid.length, m_oidWarmRestart,
						FIXS_ITH_ARRAY_SIZE(m_oidWarmRestart)) == 0) {
					std::string td_oid("");
					OidToString(td.val.objid.oids, td.val.objid.length,
							&td_oid);
					FIXS_ITH_LOG(LOG_LEVEL_INFO, "oid_WarmStart: %s",
							td_oid.c_str());
					std::string warm_start_oid("");
					OidToString(m_oidWarmRestart,
							FIXS_ITH_ARRAY_SIZE(m_oidWarmRestart),
							&warm_start_oid);
					FIXS_ITH_LOG(LOG_LEVEL_INFO, "oid_WarmStart : %s",
							warm_start_oid.c_str());
					isWarm = true;
				}

				// VRRP Master Change Trap
				if (fixs_ith_snmpmanager::oid_compare(td.val.objid.oids,
						td.val.objid.length, oid_trap_scx_VRRPMasterChange,
						FIXS_ITH_ARRAY_SIZE(oid_trap_scx_VRRPMasterChange))
						== 0) {
					std::string td_oid("");
					OidToString(td.val.objid.oids, td.val.objid.length,
							&td_oid);
					FIXS_ITH_LOG(LOG_LEVEL_INFO,
							"A call to handle_VrrpMasterChange is called with ip and trapid : '%s' %s",
							source_trap_ip, td_oid.c_str());
					isVrrpMasterChange = true;
				}

				// BFD Session Up Trap
				if (fixs_ith_snmpmanager::oid_compare(td.val.objid.oids,
						td.val.objid.length, BFD_MIB::oid_trap_smx_SessionUp,
						FIXS_ITH_ARRAY_SIZE(BFD_MIB::oid_trap_smx_SessionUp))
						== 0) {
					std::string td_oid("");
					OidToString(td.val.objid.oids, td.val.objid.length,
							&td_oid);
					FIXS_ITH_LOG(LOG_LEVEL_INFO,
							"A call to handle_BFDSessionUp is called with IP:<%s> %s",
							source_trap_ip, td_oid.c_str());
					isBFDSessionUp = true;
				}

				// BFD Session Down Trap
				if (fixs_ith_snmpmanager::oid_compare(td.val.objid.oids,
						td.val.objid.length, BFD_MIB::oid_trap_smx_SessionDown,
						FIXS_ITH_ARRAY_SIZE(BFD_MIB::oid_trap_smx_SessionDown))
						== 0) {
					std::string td_oid("");
					OidToString(td.val.objid.oids, td.val.objid.length,
							&td_oid);
					FIXS_ITH_LOG(LOG_LEVEL_INFO,
							"A call to handle_BFDSessionDown is called with IP:<%s> %s",
							source_trap_ip, td_oid.c_str());
					isBFDSessionDown = true;
				}

				// Interface Port Down Trap
				if (fixs_ith_snmpmanager::oid_compare(td.val.objid.oids,
						td.val.objid.length, oid_trap_scx_InterfaceLinkDown,
						FIXS_ITH_ARRAY_SIZE(oid_trap_scx_InterfaceLinkDown))
						== 0) {
					std::string td_oid("");
					OidToString(td.val.objid.oids, td.val.objid.length,
							&td_oid);
					FIXS_ITH_LOG(LOG_LEVEL_INFO,
							"Handling the Interface Port Down trap: " "calling 'handle_InterfaceLinkDown': an Interaface Port fault alarm should be raised: " "source_trap_ip == '%s' %s",
							source_trap_ip, td_oid.c_str());
					std::string if_down_oid("");
					OidToString(oid_trap_scx_InterfaceLinkDown,
							FIXS_ITH_ARRAY_SIZE(oid_trap_scx_InterfaceLinkDown),
							&if_down_oid);
					FIXS_ITH_LOG(LOG_LEVEL_INFO, "if_down_oid: %s",
							if_down_oid.c_str());
					isIfPortDown = true;
				}

				// Interface Port Up Trap
				if (fixs_ith_snmpmanager::oid_compare(td.val.objid.oids,
						td.val.objid.length, oid_trap_scx_InterfaceLinkUp,
						FIXS_ITH_ARRAY_SIZE(oid_trap_scx_InterfaceLinkUp))
						== 0) {
					std::string td_oid("");
					OidToString(td.val.objid.oids, td.val.objid.length,
							&td_oid);
					FIXS_ITH_LOG(LOG_LEVEL_INFO,
							"Handling the Interface Port Up trap: " "calling 'handle_InterfaceLinkUp': an Interaface Port fault alarm should be ceased: " "source_trap_ip == '%s' %s",
							source_trap_ip, td_oid.c_str());
					isIfPortUP = true;
				}

				if (fixs_ith_snmpmanager::oid_compare(td.val.objid.oids,
						td.val.objid.length, m_oidContFileResult,
						FIXS_ITH_ARRAY_SIZE(m_oidContFileResult)) == 0) {
					std::string td_oid("");
					OidToString(td.val.objid.oids, td.val.objid.length,
							&td_oid);

					contFileResult = td.val.integer;
					FIXS_ITH_LOG(LOG_LEVEL_INFO,
							"DBG: in Handle Trap contFileresult= %d",
							contFileResult);
					FIXS_ITH_LOG(LOG_LEVEL_INFO,
							"Handling the contFileresult trap, source_trap_ip == '%s', trap_oid =%s",
							source_trap_ip, td_oid.c_str());
				}

				if (fixs_ith_snmpmanager::oid_compare(td.val.objid.oids,
						td.val.objid.length, m_oidContFileValue,
						FIXS_ITH_ARRAY_SIZE(m_oidContFileValue)) == 0) {
					contFileNameValue = td.val.string;
					std::string td_oid("");
					OidToString(td.val.objid.oids, td.val.objid.length,
							&td_oid);
					FIXS_ITH_LOG(LOG_LEVEL_INFO,
							"DBG: in Handle Trap container file name= %s",
							contFileNameValue.c_str());
				}

				if (fixs_ith_snmpmanager::oid_compare(td.val.objid.oids,
						td.val.objid.length, m_oidContFileTransferComplete,
						FIXS_ITH_ARRAY_SIZE(m_oidContFileTransferComplete))
						== 0) {
					FIXS_ITH_LOG(LOG_LEVEL_INFO,
							"Setting the cmxLogFlag to true");
					CMXLogFlag = true;
				}

			}

		} // end for

		if (CMXLogFlag && contFileResult) {
			FIXS_ITH_LOG(LOG_LEVEL_INFO,
					"CMX Log collection completed, renaming the file");
			common::utility::renameCopiedContFile(contFileNameValue,
					(td.ipTrap));
		}

		if (isCold || isWarm) {
			// Print common part
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
					"*********** TRAP: ColdStart or WarmStart received ");
			return handle_ColdStartTrap(board_key, source_trap_ip);
		}

		if (isVrrpMasterChange) {
			return handle_VrrpMasterChange(++var_it, var_list_end, board_key);
		}

		if (isBFDSessionUp) {
			FIXS_ITH_LOG(LOG_LEVEL_INFO, "A call to handle_BFDSessionUp");
			return handle_BfdSessionUp(++var_it, var_list_end, board_key);
		}

		if (isBFDSessionDown) {
			FIXS_ITH_LOG(LOG_LEVEL_INFO, "A call to handle_BFDSessionDown");
			return handle_BfdSessionDown(++var_it, var_list_end, board_key);
		}

		if (isIfPortDown) {
			FIXS_ITH_LOG(LOG_LEVEL_INFO,
					"A call to handle handle_InterfaceLinkDown");
			return handle_InterfaceLinkDown(++var_it, var_list_end, board_key);
		}

		if (isIfPortUP) {
			FIXS_ITH_LOG(LOG_LEVEL_INFO,
					"A call to handle handle_InterfaceLinkUp");
			return handle_InterfaceLinkUp(++var_it, var_list_end, board_key);

		}

	}

	// Check the type: it need to be an object id
	if (var_it->type != ASN_OBJECT_ID) { // ERROR: not an object id
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"No trap OID is specified in the received pdu");
		return fixs_ith_snmp::ERR_SNMP_NOT_PDU_TRAP;
	}

	char buffer[2 * 1024] = { 0 };

	oid_to_string(buffer, FIXS_ITH_ARRAY_SIZE(buffer), var_it->val.objid.oids,
			var_it->val.objid.length);

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Received the following trap oid: %s",
			buffer);

	if (isSMX) {
		//Only SMX switch boards can send Wrong PIM module notification
		if (fixs_ith_snmpmanager::oid_compare(var_it->val.objid.oids,
				var_it->val.objid.length, oid_trap_smx_pim,
				FIXS_ITH_ARRAY_SIZE(oid_trap_smx_pim)) == 0) {
			FIXS_ITH_LOG(LOG_LEVEL_INFO,
					"A call to handle_PimTrap is called with ip '%s'",
					source_trap_ip);
			return handle_PimTrap(++var_it, var_list_end, board_key);
		}
		//Only SCX switch boards can send COLD START trap
		if (fixs_ith_snmpmanager::oid_compare(var_it->val.objid.oids,
				var_it->val.objid.length, oid_trap_scx_ColdStart,
				FIXS_ITH_ARRAY_SIZE(oid_trap_scx_ColdStart)) == 0) {
			FIXS_ITH_LOG(LOG_LEVEL_INFO,
					"A call to handle_ColdStartTrap is called with ip '%s'",
					source_trap_ip);
			return handle_ColdStartTrap(board_key, source_trap_ip);
		}

		//Only SCX switch boards can send WARM START trap
		if (fixs_ith_snmpmanager::oid_compare(var_it->val.objid.oids,
				var_it->val.objid.length, oid_trap_scx_WarmStart,
				FIXS_ITH_ARRAY_SIZE(oid_trap_scx_WarmStart)) == 0) {
			FIXS_ITH_LOG(LOG_LEVEL_INFO,
					"A call to handle_WarmStartTrap is called with ip '%s'",
					source_trap_ip);
			return handle_WarmStartTrap(board_key, source_trap_ip);
		}

		// VRRP Master Change Trap
		if (fixs_ith_snmpmanager::oid_compare(var_it->val.objid.oids,
				var_it->val.objid.length, oid_trap_scx_VRRPMasterChange,
				FIXS_ITH_ARRAY_SIZE(oid_trap_scx_VRRPMasterChange)) == 0) {
			FIXS_ITH_LOG(LOG_LEVEL_INFO,
					"A call to handle_VrrpMasterChange is called with ip '%s'",
					source_trap_ip);
			return handle_VrrpMasterChange(++var_it, var_list_end, board_key);
		}

		// BFD Session Up Trap
		if (fixs_ith_snmpmanager::oid_compare(var_it->val.objid.oids,
				var_it->val.objid.length, BFD_MIB::oid_trap_smx_SessionUp,
				FIXS_ITH_ARRAY_SIZE(BFD_MIB::oid_trap_smx_SessionUp)) == 0) {
			FIXS_ITH_LOG(LOG_LEVEL_INFO,
					"A call to handle_BFDSessionUp is called with IP:<%s>",
					source_trap_ip);
			return handle_BfdSessionUp(++var_it, var_list_end, board_key);
		}

		// BFD Session Down Trap
		if (fixs_ith_snmpmanager::oid_compare(var_it->val.objid.oids,
				var_it->val.objid.length, BFD_MIB::oid_trap_smx_SessionDown,
				FIXS_ITH_ARRAY_SIZE(BFD_MIB::oid_trap_smx_SessionDown)) == 0) {
			FIXS_ITH_LOG(LOG_LEVEL_INFO,
					"A call to handle_BFDSessionDown is called with IP:<%s>",
					source_trap_ip);
			return handle_BfdSessionDown(++var_it, var_list_end, board_key);
		}

		// Interface Port Down Trap
		if (fixs_ith_snmpmanager::oid_compare(var_it->val.objid.oids,
				var_it->val.objid.length, oid_trap_scx_InterfaceLinkDown,
				FIXS_ITH_ARRAY_SIZE(oid_trap_scx_InterfaceLinkDown)) == 0) {
			FIXS_ITH_LOG(LOG_LEVEL_INFO,
					"Handling the Interface Port Down trap: " "calling 'handle_InterfaceLinkDown': an Interaface Port fault alarm should be raised: " "source_trap_ip == '%s'",
					source_trap_ip);
			return handle_InterfaceLinkDown(++var_it, var_list_end, board_key);
		}

		// Interface Port Up Trap
		if (fixs_ith_snmpmanager::oid_compare(var_it->val.objid.oids,
				var_it->val.objid.length, oid_trap_scx_InterfaceLinkUp,
				FIXS_ITH_ARRAY_SIZE(oid_trap_scx_InterfaceLinkUp)) == 0) {
			FIXS_ITH_LOG(LOG_LEVEL_INFO,
					"Handling the Interface Port Up trap: " "calling 'handle_InterfaceLinkUp': an Interaface Port fault alarm should be ceased: " "source_trap_ip == '%s'",
					source_trap_ip);
			return handle_InterfaceLinkUp(++var_it, var_list_end, board_key);
		}
	} //END of if(isSMX)

	FIXS_ITH_LOG(LOG_LEVEL_INFO, "Unknown trap within the received pdu");

	return fixs_ith_snmp::ERR_SNMP_UNKNOWN_TRAP;
}

int fixs_ith_snmptrapmessagehandler::handle_ColdStartTrap(
		const std::string& board_key, const char * source_trap_ip) const {
	if (!fixs_ith::workingSet_t::instance()->get_trapdsobserver().is_trap_enabled(
			board_key, trap_mask::COLD_RESTART)) {
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
				"Cold start TRAP for SMX <%s> is DISABLED", board_key.c_str());
		return fixs_ith::ERR_TRAP_DISABLED;
	}

	if (fixs_ith::workingSet_t::instance()->getConfigurationManager().addToReloadInProgressSet(
			board_key)) {
		fixs_ith::workingSet_t::instance()->schedule_until_completion(board_key,
				operation::RELOAD_CONTEXT_AFTER_COLD_START_TRAP, 0, 5,
				board_key.c_str(), (board_key.length() + 1) / sizeof(uint8_t));
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
				"Scheduled operation 'RELOAD_CONTEXT_AFTER_COLD_START_TRAP' for SMX:<%s>",
				board_key.c_str());

		//Handle Logging in case of CMX
		if (common::utility::isSwitchBoardCMX()) {
			//CMX Logging functionality, delete old logs in CMX
			int logging_call_result =
					fixs_ith::workingSet_t::instance()->getConfigurationManager().deleteOldSNMPLogs(
							board_key);
			if (logging_call_result > 0) {
				//ERROR: Unable to delete old logs
				FIXS_ITH_LOG(LOG_LEVEL_ERROR,
						" ->>> Unable to delete old snmp logs for switchboard, m_switchboard_key == <%s>.",
						board_key.c_str());
			} else {
				FIXS_ITH_LOG(LOG_LEVEL_ERROR,
						" ->>> Successfully able to delete old snmp logs for switchboard, m_switchboard_key == <%s>.",
						board_key.c_str());
			}
			// Get logs from CMX
			int get_logs_call_result =
					fixs_ith::workingSet_t::instance()->getConfigurationManager().getLogsFromCMX(
							board_key, source_trap_ip); // Add ip address here
			if (get_logs_call_result > 0) {
				//ERROR: Unable to delete old logs
				FIXS_ITH_LOG(LOG_LEVEL_ERROR,
						" ->>> Failed to get the logs from switchboard, m_switchboard_key == <%s>.",
						board_key.c_str());
			} else {
				FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
						" ->>> CMXB log files collection successfully started for switchboard m_switchboard_key == <%s>.",
						board_key.c_str());
			}

		}
		return fixs_ith::ERR_NO_ERRORS;
	}

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
			"Cold start TRAP for Switch Board:<%s> is already in queue. Operation 'COLD_TRAP_RECEIVED' does not need to be rescheduled.",
			board_key.c_str());

	return fixs_ith::ERR_NO_ERRORS;
}

int fixs_ith_snmptrapmessagehandler::handle_VrrpMasterChange(
		trapds_var_list_t::iterator& var_it,
		trapds_var_list_t::iterator& var_list_end,
		const std::string& board_key) const {
	int result = fixs_ith::ERR_NO_ERRORS;

	if (!fixs_ith::workingSet_t::instance()->get_trapdsobserver().is_trap_enabled(
			board_key, trap_mask::VRRP_CHANGE)) {
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
				"VRRP MAster Change TRAP for SMX <%s> is DISABLED",
				board_key.c_str());
		return fixs_ith::ERR_TRAP_DISABLED;
	}

	int32_t vrIdentity = 0;

	for (; var_it != var_list_end; var_it++) {
		char buffer[2 * 1024] = { 0 };
		oid_to_string(buffer, FIXS_ITH_ARRAY_SIZE(buffer), var_it->name.oids,
				var_it->name.length);

		if (var_it->type == ASN_INTEGER) {
			if (fixs_ith_snmpmanager::oid_compare(var_it->name.oids,
					var_it->name.length, oid_genVrrpVrId,
					FIXS_ITH_ARRAY_SIZE(oid_genVrrpVrId)) == 0) {
				vrIdentity = var_it->val.integer;
				FIXS_ITH_LOG(LOG_LEVEL_INFO, "VRID fetched from MIB: <%d>",
						vrIdentity);
			}
		}
	}

	//Access to empty context
	std::string smxId = "";
	engine::contextAccess_t access(smxId, engine::GET_EXISTING,
			engine::SHARED_ACCESS);
	engine::Context* context = access.getContext();
	boost::shared_ptr<engine::VrrpInterface> vrrpInterfaceObj =
			context->getVrrpInterfaceById(vrIdentity);
	std::string vrrpMasterChangeObjOfRef;

	if (NULL != vrrpInterfaceObj) {
		//Object of reference chosen to be the RDN of Vrrpv3Interface
		vrrpMasterChangeObjOfRef = "Vrrpv3Interface=";
		vrrpMasterChangeObjOfRef += common::utility::getVrrpInstanceNameFromDN(
				vrrpInterfaceObj->getDN());

		fixs_ith::workingSet_t::instance()->getAlarmHandler().raiseVrrpMasterChangeAlarm(
				vrrpMasterChangeObjOfRef);
	} else {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"ERROR: vrIdentity <%d> not found in the empty context. VRRP master change alarm " "not raised!",
				vrIdentity);
	}

	return result;
}

int fixs_ith_snmptrapmessagehandler::handle_BfdSessionUp(
		trapds_var_list_t::iterator& var_it,
		trapds_var_list_t::iterator& var_list_end,
		const std::string& board_key) const {

	if (!fixs_ith::workingSet_t::instance()->get_trapdsobserver().is_trap_enabled(
			board_key, trap_mask::BFD_UP)) {
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
				"BFD SESSION UP TRAP for SMX <%s> is DISABLED",
				board_key.c_str());
		return fixs_ith::ERR_TRAP_DISABLED;
	}

	int result = fixs_ith::ERR_NO_ERRORS;

	std::string routerNameOnSmx;
	std::string ipAddress;
	int sessionState = 0U;

	if (!getBFDSessionInfoFromBfdMib(var_it, var_list_end, routerNameOnSmx,
			ipAddress, sessionState)) {
		// WARNING: The Switch/SMX board in input not found on configuration manager
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"ERROR: Failed to get Router or IP address from BFD-MIB, BFD Session down alarm not ceased!");
		return fixs_ith::ERR_GENERIC;
	}

	// Retrieve the operative context
	engine::contextAccess_t access(board_key, engine::GET_EXISTING,
			engine::SHARED_ACCESS);
	engine::Context* context = access.getContext();

	if (!context) {
		// The context is not ready.
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"ERROR: context == <NULL>: The context retrieved for SHARED_ACCESS is not ready." "BFD Session down alarm not ceased!");
		return fixs_ith::ERR_GENERIC;
	}

	std::string routerName(board_key);
	routerName.push_back(':');
	routerName.append(routerNameOnSmx);
	// get bfd session dn
	boost::shared_ptr<engine::BfdSessionIPv4> bfdSession =
			context->getBfdSessionIPv4ByName(routerName, ipAddress);

	if (bfdSession
			&& fixs_ith::workingSet_t::instance()->getAlarmHandler().ceaseBfdSessionFaultAlarm(
					bfdSession->getDN())) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "ERROR: BFD Session down alarm ceased!");
	} else {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"ERROR: BFD Session down alarm NOT ceased!");
		return fixs_ith::ERR_GENERIC;
	}

	return result;
}

int fixs_ith_snmptrapmessagehandler::handle_BfdSessionDown(
		trapds_var_list_t::iterator& var_it,
		trapds_var_list_t::iterator& var_list_end,
		const std::string& board_key) const {
	int result = fixs_ith::ERR_NO_ERRORS;

	if (!fixs_ith::workingSet_t::instance()->get_trapdsobserver().is_trap_enabled(
			board_key, trap_mask::BFD_DOWN)) {
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
				"BFD SESSION DOWN TRAP for SMX <%s> is DISABLED",
				board_key.c_str());
		return fixs_ith::ERR_TRAP_DISABLED;
	}

	std::string routerNameOnSmx;
	std::string ipAddress;
	int sessionState = 0U;

	if (!getBFDSessionInfoFromBfdMib(var_it, var_list_end, routerNameOnSmx,
			ipAddress, sessionState)) {
		// WARNING: The Switch/SMX board in input not found on configuration manager
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"ERROR: Failed to get Router or IP address from BFD-MIB, BFD Session down alarm not raised!");
		return fixs_ith::ERR_GENERIC;
	}

	if (BFD_SESSION_STATE::administrativelyDown == sessionState) {
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
				"Skip the BFD alarm, since state is adminDown");
		return fixs_ith::ERR_NO_ERRORS;
	}

	// Retrieve the operative context
	engine::contextAccess_t access(board_key, engine::GET_EXISTING,
			engine::SHARED_ACCESS);
	engine::Context* context = access.getContext();

	if (!context) {
		// The context is not ready.
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"ERROR: context == <NULL>: The context retrieved for SHARED_ACCESS is not ready. " "BFD Session down alarm not raised!");
		return fixs_ith::ERR_GENERIC;
	}

	std::string routerName(board_key);
	routerName.push_back(':');
	routerName.append(routerNameOnSmx);

	std::set<boost::shared_ptr<engine::InterfaceIPv4> > interfaces =
			context->getInterfaces(routerName);

	std::set<boost::shared_ptr<engine::InterfaceIPv4> >::const_iterator iterfaceIt =
			interfaces.begin();

	std::string interfaceName;
	const uint32_t bfdSessionAddress = inet_network(ipAddress.c_str());

	for (; interfaces.end() != iterfaceIt; ++iterfaceIt) {
		if ((*iterfaceIt)->isBfdEnabled()) {
			boost::shared_ptr<engine::Address> address =
					context->getRouterInterfaceAddress(routerName,
							(*iterfaceIt)->getName());

			std::string ipAsCIDR(address->getAddress());

			std::string ipAddress(
					ipAsCIDR.substr(0,
							ipAsCIDR.find(common::utility::parserTag::slash)));
			std::stringstream ipNetworkPrefix(
					ipAsCIDR.substr(
							ipAsCIDR.find(common::utility::parserTag::slash)
									+ 1U));

			uint32_t numberOfBits = 0U;
			ipNetworkPrefix >> numberOfBits;
			const uint32_t subnetMask = (~uint32_t(0)
					<< (common::MAX_NETWORK_PREFIX - numberOfBits));

			FIXS_ITH_LOG(LOG_LEVEL_INFO,
					"Address:<%s> Network Prefix:<%u> on Router:<%s>",
					ipAddress.c_str(), numberOfBits, routerNameOnSmx.c_str());

			const uint32_t interfaceAddress = inet_network(ipAddress.c_str());

			if ((bfdSessionAddress & subnetMask)
					== (interfaceAddress & subnetMask)) {
				interfaceName.assign((*iterfaceIt)->getName());

				FIXS_ITH_LOG(LOG_LEVEL_INFO,
						"Address:<%s> monitored by Interface:<%s> on Router:<%s>",
						ipAddress.c_str(), interfaceName.c_str(),
						routerNameOnSmx.c_str());
				break;
			}
		}
	}

	// get bfd session dn
	boost::shared_ptr<engine::BfdSessionIPv4> bfdSession =
			context->getBfdSessionIPv4ByName(routerName, ipAddress);

	if (!interfaceName.empty() && bfdSession) {
		std::string bfdSessionDN(bfdSession->getDN());
		fixs_ith::workingSet_t::instance()->getAlarmHandler().raiseBfdSessionFaultAlarm(
				bfdSessionDN, routerName, ipAddress);
	}

	return result;
}

bool fixs_ith_snmptrapmessagehandler::getSmxIdFromIp(const char* boardIp,
		std::string& smxKey) const {
	bool result = false;

	// Retrieve the Switch/SMX board key to load the correct context
	std::set<std::string> switchBoardKeys;
	if (int call_result =
			fixs_ith::workingSet_t::instance()->get_sbdatamanager().get_switchboard_keys(
					switchBoardKeys)) {
		// ERROR: On retrieving all keys from CS
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"ERROR: Retrieving Switch/SMX board keys from CS: call_result == <%d>. " "Interface Port fault alarm not raised!",
				call_result);
		return result;
	}

	for (std::set<std::string>::const_iterator cit = switchBoardKeys.begin();
			cit != switchBoardKeys.end(); ++cit) {
		fixs_ith_switchboardinfo info;
		if (int call_result =
				fixs_ith::workingSet_t::instance()->get_sbdatamanager().get_switchboard_info(
						info, *cit)) {
			// ERROR: On retrieving info for the current Switch/SMX board
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
					"ERROR: Retrieving data info about the Switch/SMX board <%s> from CS: call_result == <%d>. " "Interface Port fault alarm not raised!",
					cit->c_str(), call_result);
			return result;
		}

		if (!strcmp(info.control_ipna_str, boardIp)
				|| !strcmp(info.control_ipnb_str, boardIp)
				|| !strcmp(info.transport_ipna_str, boardIp)
				|| !strcmp(info.transport_ipnb_str, boardIp)) {
			smxKey = *cit;
			result = true;
			break;
		}
	}

	return result;
}

bool fixs_ith_snmptrapmessagehandler::getBFDSessionInfoFromBfdMib(
		trapds_var_list_t::iterator& var_it,
		trapds_var_list_t::iterator& var_list_end, std::string& routerName,
		std::string& ipAddress, int& sessionState) const {
	for (; var_it != var_list_end; ++var_it) {
		char buffer[2 * 1024] = { 0 };
		oid_to_string(buffer, FIXS_ITH_ARRAY_SIZE(buffer), var_it->name.oids,
				var_it->name.length);
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "\n#### FOUND OID variable:<%s> ###",
				buffer);

		if (fixs_ith_snmpmanager::oid_compare(var_it->name.oids,
				var_it->name.length, BFD_MIB::oid_trap_smx_IFIndex,
				FIXS_ITH_ARRAY_SIZE(BFD_MIB::oid_trap_smx_IFIndex)) == 0) {
			int if_index = var_it->val.integer;
			FIXS_ITH_LOG(LOG_LEVEL_INFO, "IF_Index fetched from MIB: <%d>",
					if_index);
			continue;
		}

		if (fixs_ith_snmpmanager::oid_compare(var_it->name.oids,
				var_it->name.length, BFD_MIB::oid_trap_smx_SessionDiagnostic,
				FIXS_ITH_ARRAY_SIZE(BFD_MIB::oid_trap_smx_SessionDiagnostic))
				== 0) {
			sessionState = var_it->val.integer;
			FIXS_ITH_LOG(LOG_LEVEL_INFO, "BFD session state from MIB: <%d>",
					sessionState);
			continue;
		}

		if (fixs_ith_snmpmanager::oid_compare(var_it->name.oids,
				var_it->name.length, BFD_MIB::oid_trap_smx_RouterId,
				FIXS_ITH_ARRAY_SIZE(BFD_MIB::oid_trap_smx_RouterId)) == 0) {
			routerName.assign(var_it->val.string);
			FIXS_ITH_LOG(LOG_LEVEL_INFO, "Router Name:<%s> fetched from MIB",
					routerName.c_str());
			continue;
		}

		if (fixs_ith_snmpmanager::oid_compare(var_it->name.oids,
				var_it->name.length, BFD_MIB::oid_trap_smx_PeerIPType,
				FIXS_ITH_ARRAY_SIZE(BFD_MIB::oid_trap_smx_PeerIP)) == 0) {
			int addressType = var_it->val.integer;
			FIXS_ITH_LOG(LOG_LEVEL_INFO, "Address type from MIB: <%d>",
					addressType);
			continue;
		}

		if (fixs_ith_snmpmanager::oid_compare(var_it->name.oids,
				var_it->name.length, BFD_MIB::oid_trap_smx_PeerIP,
				FIXS_ITH_ARRAY_SIZE(BFD_MIB::oid_trap_smx_PeerIP)) == 0) {
			uint32_t ipAddr = 0U;

			// convert form network order to host order
			for (uint8_t index = 0U; index < var_it->val_len; ++index) {
				ipAddr |= (var_it->val.bitstring[index] << (8 * index));
			}

			// convert ip from number to a dot string
			struct in_addr addr;
			addr.s_addr = ipAddr;

			ipAddress.assign(inet_ntoa(addr));

			FIXS_ITH_LOG(LOG_LEVEL_INFO, "IP Address:<%s> fetched from MIB",
					ipAddress.c_str());

			continue;
		}

	}

	return (!routerName.empty() && !ipAddress.empty());
}

void fixs_ith_snmptrapmessagehandler::oid_to_string(char * buf, size_t buf_size,
		const oid * name, size_t name_size) const {
	char * buf_end = buf + buf_size;

	for (int chars_wrote = 0; (name_size--) && (buf < buf_end); buf +=
			chars_wrote, buf_size -= chars_wrote)
		chars_wrote = ::snprintf(buf, buf_size, ".%lu", *name++);

	buf_end[-1] = 0;
}

int fixs_ith_snmptrapmessagehandler::handle_InterfaceLinkDown(
		trapds_var_list_t::iterator & var_it,
		trapds_var_list_t::iterator & /*var_list_end*/,
		const std::string& board_key) const {
	/*
	 * Documentation about LinkDown trap from IF-MIB standard
	 *
	 *	linkDown NOTIFICATION-TYPE
	 *		OBJECTS { ifIndex, ifAdminStatus, ifOperStatus }
	 *		STATUS  current
	 *		DESCRIPTION
	 *			"A linkDown trap signifies that the SNMPv2 entity,
	 *			acting in an agent role, has detected that the
	 *			ifOperStatus object for one of its communication links
	 *			is about to transition into the down state."
	 *		::= { snmpTraps 3 }
	 *
	 *	NOTE: ifIndex SYNTAX INTEGER
	 *				ifAdminStatus SYNTAX INTEGER {
	 *																up(1),			-- ready to pass packets
	 *																down(2),
	 *																testing(3)	-- in some test mode
	 *														 }
	 *				ifOperStatus SYNTAX INTEGER {
	 *															up(1),				-- ready to pass packets
	 *															down(2),
	 *															testing(3),		-- in some test mode
	 *															unknown(4),		-- status can not be determined for some reason.
	 *															dormant(5),
	 *															notPresent(6)	-- some component is missing
	 *														}
	 */

	if (!fixs_ith::workingSet_t::instance()->get_trapdsobserver().is_trap_enabled(
			board_key, trap_mask::LINK_DOWN)) {
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
				"INTERFACE LINK DOWN TRAP for SMX <%s> is DISABLED",
				board_key.c_str());
		return fixs_ith::ERR_TRAP_DISABLED;
	}

	int32_t if_index = -1;
	int32_t if_admin_status = -1;
	int32_t if_oper_status = -1;
	char buffer[2 * 1024] = { 0 };

	/* Get the ifIndex object from trap */
	oid_to_string(buffer, FIXS_ITH_ARRAY_SIZE(buffer), var_it->name.oids,
			var_it->name.length);
	if_index = var_it->val.integer;
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
			"ifIndex object got from SMX Trap LinkDown from switch_board <%s>: if_index == <%d>\n" "#### OID variable:<%s>\n" "#### Type:<%u> value:<%d>",
			board_key.c_str(), if_index, buffer, var_it->type,
			var_it->val.integer);

	++var_it;

	/* Get the ifAdminStatus object from trap */
	oid_to_string(buffer, FIXS_ITH_ARRAY_SIZE(buffer), var_it->name.oids,
			var_it->name.length);
	if_admin_status = var_it->val.integer;
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
			"ifAdminStatus got from SMX Trap LinkDown from switch_board <%s>: if_admin_status == <%s>\n" "#### OID variable:<%s>\n" "#### Type:<%u> value:<%d>",
			board_key.c_str(),
			utl_link_down_admin_status_to_string(if_admin_status), buffer,
			var_it->type, var_it->val.integer);

	++var_it;

	/* Get the ifOperStatus object from trap */
	oid_to_string(buffer, FIXS_ITH_ARRAY_SIZE(buffer), var_it->name.oids,
			var_it->name.length);
	if_oper_status = var_it->val.integer;
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
			"ifOperStatus got from SMX Trap LinkDown from switch_board <%s>: if_oper_status == <%s>\n" "#### OID variable:<%s>\n" "#### Type:<%u> value:<%d>",
			board_key.c_str(),
			utl_link_down_oper_status_to_string(if_oper_status), buffer,
			var_it->type, var_it->val.integer);

	// Retrieve the operative context
	engine::contextAccess_t access(board_key, engine::GET_EXISTING,
			engine::SHARED_ACCESS);
	engine::Context * context = access.getContext();

	if (!context) {
		// The context is not ready.
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"ERROR: context == <NULL>: The context retrieved for SHARED_ACCESS is not ready. " "Interface Port fault alarm not raised!");
		return fixs_ith::ERR_SMX_ALARM_NOT_RAISED;
	}

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "context == <0x%p>",
			reinterpret_cast<void *>(context));

	if (boost::shared_ptr<engine::EthernetPort> if_port =
			context->getEthernetPortByIndex(fixs_ith::TRANSPORT_PLANE,
					if_index)) {
		std::string if_port_dn = if_port->getDN();
		std::string if_port_rdn_value;

		if (!common::utility::getRDNValue(if_port_dn,
				imm::ethernetPort_attribute::RDN.c_str(), if_port_rdn_value)) {
			// ERROR: Fetching the RDN value from the ethernet port object DN
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
					"ERROR: Fetching the RDN value from the ethernet port object DN. if_port_dn == <%s>. " "Interface Port fault alarm not raised!",
					if_port_dn.c_str());
			return fixs_ith::ERR_SMX_ALARM_NOT_RAISED;
		}

		std::string if_port_rdn = common::utility::getRDNAttribute(if_port_dn)
				+ "=" + if_port_rdn_value;

		FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
				"if_port_dn == <%s>, if_port_rdn == <%s>, if_port_rdn_value == <%s>",
				if_port_dn.c_str(), if_port_rdn.c_str(),
				if_port_rdn_value.c_str());

		std::string magazine;
		std::string slot;
		std::string port_name;

		if (common::utility::getEthernetPortPositionFromRdnValue(magazine, slot,
				port_name, if_port_rdn_value)) {
			// ERROR: Ethernet port RDN malformed
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
					"ERROR: Getting the interface port coordinates {magazine, slot, port} from RDN value. " "if_port_dn == <%s>, if_port_rdn == <%s>, if_port_rdn_value == <%s>. " "Interface Port fault alarm not raised!",
					if_port_dn.c_str(), if_port_rdn.c_str(),
					if_port_rdn_value.c_str());
			return fixs_ith::ERR_SMX_ALARM_NOT_RAISED;
		}

		if (is_front_port_to_monitor(port_name))
			fixs_ith::workingSet_t::instance()->getAlarmHandler().raiseInterfacePortFaultAlarm(
					if_port_rdn, magazine, slot, port_name);
		else {
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
					"The IF port at coordinates [MAG:<%s>, SLOT:<%s>, NAME:<%s>] is not a front port. " "No alarms raised for that port.",
					magazine.c_str(), slot.c_str(), port_name.c_str());
		}
	} else {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"ERROR: Interface Port at index <%d> not found in the empty context. " "Interface Port fault alarm not raised!",
				if_index);
		return fixs_ith::ERR_SMX_ALARM_NOT_RAISED;
	}

	return fixs_ith::ERR_NO_ERRORS;
}

int fixs_ith_snmptrapmessagehandler::handle_PimTrap(
		trapds_var_list_t::iterator & var_it,
		trapds_var_list_t::iterator & /*var_list_end*/,
		const std::string& board_key) const {
	if (!fixs_ith::workingSet_t::instance()->get_trapdsobserver().is_trap_enabled(
			board_key, trap_mask::SMX_PIM)) {
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "PIM TRAP for SMX <%s> is DISABLED",
				board_key.c_str());
		return fixs_ith::ERR_TRAP_DISABLED;
	}

	int pimPortPresenceStatus = -1;
	int pimMatchingStatus = -1;
	int32_t if_index = -1;
	char buffer[2 * 1024] = { 0 };

	/* Get the ifIndex object from trap */
	oid_to_string(buffer, FIXS_ITH_ARRAY_SIZE(buffer), var_it->name.oids,
			var_it->name.length);
	if_index = var_it->val.integer;
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
			"ifIndex object got from SMX PIM Status Change Trap  from switch_board <%s>: if_index == <%d>\n" "#### OID variable:<%s>\n" "#### Type:<%u> value:<%d>",
			board_key.c_str(), if_index, buffer, var_it->type,
			var_it->val.integer);

	++var_it;

	/* Get the pimPortPresenceStatus object from trap */
	oid_to_string(buffer, FIXS_ITH_ARRAY_SIZE(buffer), var_it->name.oids,
			var_it->name.length);
	pimPortPresenceStatus = var_it->val.integer;
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
			"pimPortPresenceStatus got from SMX PIM Status Change Trap from switch_board <%s>: pimPortPresenceStatus == <%d>\n" "#### OID variable:<%s>\n" "#### Type:<%u> value:<%d>",
			board_key.c_str(), pimPortPresenceStatus, buffer, var_it->type,
			var_it->val.integer);

	++var_it;
	oid_to_string(buffer, FIXS_ITH_ARRAY_SIZE(buffer), var_it->name.oids,
			var_it->name.length);
	pimMatchingStatus = var_it->val.integer;
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
			"pimMatchingStatus got from SMX PIM Status Change Trap from switch_board <%s>: pimMatchingStatus == <%d>\n" "#### OID variable:<%s>\n" "#### Type:<%u> value:<%d>",
			board_key.c_str(), pimMatchingStatus, buffer, var_it->type,
			var_it->val.integer);

	engine::contextAccess_t access(board_key, engine::GET_EXISTING,
			engine::SHARED_ACCESS);
	engine::Context * context = access.getContext();

	if (!context) {
		// The context is not ready.
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"ERROR: context == <NULL>: The context retrieved for SHARED_ACCESS is not ready. " "PIM fault alarm not raised/ceased!");
		return fixs_ith::ERR_GENERIC;
	}

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "context == <0x%p>",
			reinterpret_cast<void *>(context));

	if (boost::shared_ptr<engine::EthernetPort> if_port =
			context->getEthernetPortByIndex(fixs_ith::TRANSPORT_PLANE,
					if_index)) {
		std::string if_port_dn = if_port->getDN();
		std::string if_port_rdn_value;

		if (!common::utility::getRDNValue(if_port_dn,
				imm::ethernetPort_attribute::RDN.c_str(), if_port_rdn_value)) {
			// ERROR: Fetching the RDN value from the ethernet port object DN
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
					"ERROR: Fetching the RDN value from the ethernet port object DN. if_port_dn == <%s>. " "PIM fault alarm not raised!",
					if_port_dn.c_str());
			return fixs_ith::ERR_GENERIC;
		}

		std::string if_port_rdn = common::utility::getRDNAttribute(if_port_dn)
				+ "=" + if_port_rdn_value;
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
				"if_port_dn == <%s>, if_port_rdn == <%s>, if_port_rdn_value == <%s>",
				if_port_dn.c_str(), if_port_rdn.c_str(),
				if_port_rdn_value.c_str());

		std::string magazine;
		std::string slot;
		std::string port_name;

		if (common::utility::getEthernetPortPositionFromRdnValue(magazine, slot,
				port_name, if_port_rdn_value)) {
			// ERROR: Ethernet port RDN malformed
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
					"ERROR: Getting the interface port coordinates {magazine, slot, port} from RDN value. " "if_port_dn == <%s>, if_port_rdn == <%s>, if_port_rdn_value == <%s>. " "PIM fault alarm not raised/ceased!",
					if_port_dn.c_str(), if_port_rdn.c_str(),
					if_port_rdn_value.c_str());
			return fixs_ith::ERR_GENERIC;
		}

		if (pimPortPresenceStatus == fixs_ith_snmp::ABSENT) {
			fixs_ith::workingSet_t::instance()->getAlarmHandler().ceasePimFaultAlarm(
					if_port_rdn);
		} else if (pimMatchingStatus == fixs_ith_snmp::MISMATCH) {
			fixs_ith::workingSet_t::instance()->getAlarmHandler().raisePimFaultAlarm(
					if_port_rdn, magazine, slot, port_name);
		} else if ((pimPortPresenceStatus == fixs_ith_snmp::PRESENT)
				&& (pimMatchingStatus == fixs_ith_snmp::MATCH)) {
			fixs_ith::workingSet_t::instance()->getAlarmHandler().ceasePimFaultAlarm(
					if_port_rdn);

			// Schedule PIM adaptation procedure
			struct {
				std::string switchboard_key;
				uint16_t ifIndex;
				fixs_ith::switchboard_plane_t sb_plane;
			} op_params;
			op_params.switchboard_key = board_key;
			op_params.ifIndex = if_index;
			op_params.sb_plane = if_port->getPlane();
			fixs_ith::workingSet_t::instance()->schedule_until_completion(
					board_key, operation::PIM_ADAPTATION, 1, 3, &op_params,
					sizeof(op_params));
		} else {
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
					"DEBUG: PIM fault alarm not raised/ceased! with portPresenceStatus== <%d>, portMatchingStatus== <%d> ",
					pimPortPresenceStatus, pimMatchingStatus);
		}
	} else {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"ERROR: Interface Port at index <%d> not found in the empty context. " "PIM fault alarm not raised/ceased!",
				if_index);
		return fixs_ith::ERR_GENERIC;
	}

	return fixs_ith::ERR_NO_ERRORS;
}

int fixs_ith_snmptrapmessagehandler::handle_InterfaceLinkUp(
		trapds_var_list_t::iterator & var_it,
		trapds_var_list_t::iterator & /*var_list_end*/,
		const std::string& board_key) const {
	/*
	 * Documentation about LinkUp trap from IF-MIB standard
	 *
	 *	linkUp NOTIFICATION-TYPE
	 *		OBJECTS { ifIndex, ifAdminStatus, ifOperStatus }
	 *		STATUS  current
	 *		DESCRIPTION
	 *			"A linkUp trap signifies that the SNMPv2 entity,
	 *			acting in an agent role, has detected that the
	 *			ifOperStatus object for one of its communication links
	 *			has transitioned out of the down state."
	 *		::= { snmpTraps 4 }
	 *
	 *	NOTE: ifIndex SYNTAX INTEGER
	 *				ifAdminStatus SYNTAX INTEGER {
	 *																up(1),			-- ready to pass packets
	 *																down(2),
	 *																testing(3)	-- in some test mode
	 *														 }
	 *				ifOperStatus SYNTAX INTEGER {
	 *															up(1),				-- ready to pass packets
	 *															down(2),
	 *															testing(3),		-- in some test mode
	 *															unknown(4),		-- status can not be determined for some reason.
	 *															dormant(5),
	 *															notPresent(6)	-- some component is missing
	 *														}
	 */

	if (!fixs_ith::workingSet_t::instance()->get_trapdsobserver().is_trap_enabled(
			board_key, trap_mask::LINK_UP)) {
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
				"INTERFACE LINK UP TRAP for SMX <%s> is DISABLED",
				board_key.c_str());
		return fixs_ith::ERR_TRAP_DISABLED;
	}

	int32_t if_index = -1;
	int32_t if_admin_status = -1;
	int32_t if_oper_status = -1;
	char buffer[2 * 1024] = { 0 };

	/* Get the ifIndex object from trap */
	oid_to_string(buffer, FIXS_ITH_ARRAY_SIZE(buffer), var_it->name.oids,
			var_it->name.length);
	if_index = var_it->val.integer;
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
			"ifIndex object got from SMX Trap LinkUp from switch_board<%s>: if_index == <%d>\n" "#### OID variable:<%s>\n" "#### Type:<%u> value:<%d>",
			board_key.c_str(), if_index, buffer, var_it->type,
			var_it->val.integer);

	++var_it;

	/* Get the ifAdminStatus object from trap */
	oid_to_string(buffer, FIXS_ITH_ARRAY_SIZE(buffer), var_it->name.oids,
			var_it->name.length);
	if_admin_status = var_it->val.integer;
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
			"ifAdminStatus got from SMX Trap LinkUp from switch_board<%s>: if_admin_status == <%s>\n" "#### OID variable:<%s>\n" "#### Type:<%u> value:<%d>",
			board_key.c_str(),
			utl_link_up_admin_status_to_string(if_admin_status), buffer,
			var_it->type, var_it->val.integer);

	++var_it;

	/* Get the ifOperStatus object from trap */
	oid_to_string(buffer, FIXS_ITH_ARRAY_SIZE(buffer), var_it->name.oids,
			var_it->name.length);
	if_oper_status = var_it->val.integer;
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
			"ifOperStatus got from SMX Trap LinkUp from switch_board <%s>: if_oper_status == <%s>\n" "#### OID variable:<%s>\n" "#### Type:<%u> value:<%d>",
			board_key.c_str(),
			utl_link_up_oper_status_to_string(if_oper_status), buffer,
			var_it->type, var_it->val.integer);

	// Retrieve the Switch/SMX board key to load the correct context
	std::set<std::string> switchBoardKeys;
	if (int call_result =
			fixs_ith::workingSet_t::instance()->get_sbdatamanager().get_switchboard_keys(
					switchBoardKeys)) {
		// ERROR: On retrieving all keys from CS
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"ERROR: Retrieving Switch/SMX board keys from CS: call_result == <%d>. " "Interface Port fault alarm not ceased!",
				call_result);
		return fixs_ith::ERR_SMX_ALARM_NOT_CEASED;
	}

	// Retrieve the operative context
	engine::contextAccess_t access(board_key, engine::GET_EXISTING,
			engine::SHARED_ACCESS);
	engine::Context * context = access.getContext();

	if (!context) {
		// The context is not ready.
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"ERROR: context == <NULL>: The context retrieved for SHARED_ACCESS is not ready. " "Interface Port fault alarm not ceased!");
		return fixs_ith::ERR_SMX_ALARM_NOT_CEASED;
	}

	if (boost::shared_ptr<engine::EthernetPort> if_port =
			context->getEthernetPortByIndex(fixs_ith::TRANSPORT_PLANE,
					if_index)) {
		std::string if_port_dn = if_port->getDN();
		std::string if_port_rdn_value;

		if (!common::utility::getRDNValue(if_port_dn,
				imm::ethernetPort_attribute::RDN.c_str(), if_port_rdn_value)) {
			// ERROR: Fetching the RDN value from the ethernet port object DN
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
					"ERROR: Fetching the RDN value from the ethernet port object DN. if_port_dn == <%s>. " "Interface Port fault alarm not ceased!",
					if_port_dn.c_str());
			return fixs_ith::ERR_SMX_ALARM_NOT_CEASED;
		}

		std::string if_port_rdn = common::utility::getRDNAttribute(if_port_dn)
				+ "=" + if_port_rdn_value;

		fixs_ith::workingSet_t::instance()->getAlarmHandler().ceaseInterfacePortFaultAlarm(
				if_port_rdn);
	} else {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"ERROR: Interface Port at index <%d> not found in the empty context. " "Interface Port fault alarm not ceased!",
				if_index);
		return fixs_ith::ERR_SMX_ALARM_NOT_CEASED;
	}

	return fixs_ith::ERR_NO_ERRORS;
}

const char * fixs_ith_snmptrapmessagehandler::utl_link_down_admin_status_to_string(
		int admin_status) const {
	const char * ADMIN_STATUS_STINGS[] = { "UNDEFINED", "UP(1)", "DOWN(2)",
			"TESTING(3)" };

	if ((admin_status < 1)
			|| (FIXS_ITH_ARRAY_SIZE(ADMIN_STATUS_STINGS)
					<= static_cast<size_t>(admin_status)))
		admin_status = 0;

	return ADMIN_STATUS_STINGS[admin_status];
}

const char * fixs_ith_snmptrapmessagehandler::utl_link_down_oper_status_to_string(
		int oper_status) const {
	const char * OPER_STATUS_STINGS[] = { "UNDEFINED", "UP(1)", "DOWN(2)",
			"TESTING(3)", "UNKNOWN(4)", "DORMANT(5)", "NOT_PRESENT(6)" };

	if ((oper_status < 1)
			|| (FIXS_ITH_ARRAY_SIZE(OPER_STATUS_STINGS)
					<= static_cast<size_t>(oper_status)))
		oper_status = 0;

	return OPER_STATUS_STINGS[oper_status];
}

const char * fixs_ith_snmptrapmessagehandler::utl_link_up_admin_status_to_string(
		int admin_status) const {
	return utl_link_down_admin_status_to_string(admin_status);
}

const char * fixs_ith_snmptrapmessagehandler::utl_link_up_oper_status_to_string(
		int oper_status) const {
	return utl_link_down_oper_status_to_string(oper_status);
}

void fixs_ith_snmptrapmessagehandler::OidToString(oid *iodIn, unsigned int len,
		std::string *strOut) {
	std::stringstream sStr;
	unsigned int x;
	// Init
	x = 0;
	sStr.str("");
	(*strOut) = "";
	// for all element in oid,
	for (x = 0; x < len; ++x) {
		sStr << '.';
		sStr << (int) iodIn[x];
	}
	// Set output string
	(*strOut) = sStr.str();
}

