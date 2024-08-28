/*
 * fixs_ith_cshwctablechangeobserver.cpp
 *
 *  Created on: Jan 27, 2016
 *      Author: xludesi
 */

#include <arpa/inet.h>

#include <ace/Guard_T.h>

#include "fixs_ith_programmacros.h"
#include "fixs_ith_logger.h"
#include "fixs_ith_server.h"
#include "fixs_ith_csreader.h"
#include "fixs_ith_workingset.h"

#include "fixs_ith_cshwctablechangeobserver.h"
#include "common/utility.h"

void fixs_ith_cshwctablechangeobserver::update(
		const ACS_CS_API_HWCTableChange & observer) {
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
			"CS HWC table change notification received: notification items count == %d",
			observer.dataSize);

	for (int i = 0; i < observer.dataSize; ++i) {
		const ACS_CS_API_HWCTableData_R1 & hwc_item = observer.hwcData[i];

		FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
				"  HWC table item #%02d: " "Operation type == %d, magazine == 0x%08X, slot == %u, sysId == %u, sysType == %u, sysNo == %u, fbn == %u, " "side == %u, ipEthA == 0x%08X, ipEthB == 0x%08X",
				i, hwc_item.operationType, hwc_item.magazine, hwc_item.slot,
				hwc_item.sysId, hwc_item.sysType, hwc_item.sysNo, hwc_item.fbn,
				hwc_item.side, htonl(hwc_item.ipEthA), htonl(hwc_item.ipEthB));
	}

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Resetting CS data into the cs_reader...");
	fixs_ith::workingSet_t::instance()->get_cs_reader().reset_info();
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
			"CS data info into the cs_reader cleared successfully");

//	int loading_board_info_todo = 1;

	for (int i = 0; i < observer.dataSize; ++i) {
		const ACS_CS_API_HWCTableData_R1 & hwc_item = observer.hwcData[i];
		ACS_CS_API_TableChangeOperation::OpType op_type = hwc_item.operationType;
		const uint32_t magazine = htonl(hwc_item.magazine);
		const uint16_t slot = hwc_item.slot;
		char magazine_str[16] = { 0 };
		const uint16_t fbn = hwc_item.fbn;

		fixs_ith_csreader::uint32_to_ip_format(magazine_str, magazine);
		// Is this board a switch board?
		if (fbn == fixs_ith_csreader::SWITCH_BOARD_FBN_SMXB) { // YES
			fixs_ith_switchboardinfo switch_board;
			switch_board.magazine = magazine;
			switch_board.slot_position = slot;
			fixs_ith_csreader::uint32_to_ip_format(
					switch_board.control_ipna_str, htonl(hwc_item.ipEthA));
			fixs_ith_csreader::uint32_to_ip_format(
					switch_board.control_ipnb_str, htonl(hwc_item.ipEthB));
			fixs_ith_csreader::uint32_to_ip_format(
					switch_board.transport_ipna_str, htonl(hwc_item.aliasEthA));
			fixs_ith_csreader::uint32_to_ip_format(
					switch_board.transport_ipnb_str, htonl(hwc_item.aliasEthB));

			if (op_type == ACS_CS_API_TableChangeOperation::Delete) { // Deleting a switch board
				FIXS_ITH_LOG(LOG_LEVEL_INFO,
						"Deleting this switch board from SNMP manager internal structures: magazine == 0x%08X, slot == %d, ControlIPNA == '%s', ControlIPNB == '%s', TransportIPNA == '%s', TransportIPNB == '%s'",
						magazine, slot, switch_board.control_ipna_str,
						switch_board.control_ipnb_str,
						switch_board.transport_ipna_str,
						switch_board.transport_ipnb_str);

				std::string switchboard_key;
				common::utility::build_sb_key_from_magazine_and_slot(
						switchboard_key, magazine_str,
						switch_board.slot_position);

				fixs_ith::workingSet_t::instance()->get_sbdatamanager().remove_switchboard(
						switchboard_key);

				fixs_ith::workingSet_t::instance()->schedule_until_completion(
						switchboard_key, operation::BNC_OBJECT_REMOVE, 0, 3,
						switchboard_key.c_str(),
						(switchboard_key.length() + 1) / sizeof(uint8_t));
				/* TODO implementation
				 if (const int call_result = _fixs_ith_server->snmp_manager->switch_board_delete(magazine, slot)) // ERROR: Deleting the switch board
				 FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'switch_board_delete' failed: switch board not deleted correctly: call_result == %d", call_result);
				 else FIXS_ITH_LOG(LOG_LEVEL_INFO, "Switch board successfully deleted");
				 */
			} else if (op_type == ACS_CS_API_TableChangeOperation::Add) { // Adding a new switch board
				FIXS_ITH_LOG(LOG_LEVEL_INFO,
						"Adding this new switch board to the sbdatamanager internal structure: magazine == 0x%08X, slot == %d, ControlIPNA == '%s', ControlIPNB == '%s', TransportIPNA == '%s', TransportIPNB == '%s'",
						magazine, slot, switch_board.control_ipna_str,
						switch_board.control_ipnb_str,
						switch_board.transport_ipna_str,
						switch_board.transport_ipnb_str);

				fixs_ith::workingSet_t::instance()->get_sbdatamanager().add_switchboard(
						switch_board);

				FIXS_ITH_LOG(LOG_LEVEL_INFO,
						"Adding BNC objects for this new switch board in IMM ...");

				std::string switchboard_key;
				common::utility::build_sb_key_from_magazine_and_slot(
						switchboard_key, magazine_str,
						switch_board.slot_position);

				fixs_ith::workingSet_t::instance()->schedule_until_completion(
						switchboard_key, operation::BNC_OBJECTS_INSTANTIATE, 0,
						3, switchboard_key.c_str(),
						(switchboard_key.length() + 1) / sizeof(uint8_t));

			} else { // Changing a switch board
				FIXS_ITH_LOG(LOG_LEVEL_INFO,
						"Changing this switch board into the SNMP manager internal structures: magazine == 0x%08X, slot == %d, ControlIPNA == '%s', ControlIPNB == '%s', TransportIPNA == '%s', TransportIPNB == '%s'",
						magazine, slot, switch_board.control_ipna_str,
						switch_board.control_ipnb_str,
						switch_board.transport_ipna_str,
						switch_board.transport_ipnb_str);
				/* TODO implementation
				 if (const int call_result = _fixs_ith_server->snmp_manager->switch_board_change(switch_board)) // ERROR: Changing the switch board
				 FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'switch_board_change' failed: switch board not changed correctly: call_result == %d", call_result);
				 else FIXS_ITH_LOG(LOG_LEVEL_INFO, "Switch board successfully changed");
				 */
			}
		} else if (fbn == fixs_ith_csreader::SWITCH_BOARD_FBN_CMXB) {
			// YES
			fixs_ith_switchboardinfo switch_board;
			switch_board.magazine = magazine;
			switch_board.slot_position = slot;
			fixs_ith_csreader::uint32_to_ip_format(
					switch_board.control_ipna_str, htonl(hwc_item.aliasEthA));
			fixs_ith_csreader::uint32_to_ip_format(
					switch_board.control_ipnb_str, htonl(hwc_item.aliasEthB));
			fixs_ith_csreader::uint32_to_ip_format(
					switch_board.transport_ipna_str, htonl(hwc_item.ipEthA));
			fixs_ith_csreader::uint32_to_ip_format(
					switch_board.transport_ipnb_str, htonl(hwc_item.ipEthB));

			if (op_type == ACS_CS_API_TableChangeOperation::Delete) { // Deleting a switch board
				FIXS_ITH_LOG(LOG_LEVEL_INFO,
						"Deleting CMX board information from SNMP manager internal structures: magazine == 0x%08X, slot == %d, ControlIPNA == '%s', ControlIPNB == '%s', TransportIPNA == '%s', TransportIPNB == '%s'",
						magazine, slot, switch_board.control_ipna_str,
						switch_board.control_ipnb_str,
						switch_board.transport_ipna_str,
						switch_board.transport_ipnb_str);

				std::string switchboard_key;
				common::utility::build_sb_key_from_magazine_and_slot(
						switchboard_key, magazine_str,
						switch_board.slot_position);

				fixs_ith::workingSet_t::instance()->get_sbdatamanager().remove_switchboard(
						switchboard_key);

				fixs_ith::workingSet_t::instance()->schedule_until_completion(
						switchboard_key, operation::BNC_OBJECT_REMOVE, 0, 3,
						switchboard_key.c_str(),
						(switchboard_key.length() + 1) / sizeof(uint8_t));
				FIXS_ITH_LOG(LOG_LEVEL_INFO,
						"Operation is scheduled to delete the BNC objects for CMX board on slot ==%d",
						slot);
			} else if (op_type == ACS_CS_API_TableChangeOperation::Add) { // Adding a new switch board
				FIXS_ITH_LOG(LOG_LEVEL_INFO,
						"Adding this new switch board to the sbdatamanager internal structure: magazine == 0x%08X, slot == %d, ControlIPNA == '%s', ControlIPNB == '%s', TransportIPNA == '%s', TransportIPNB == '%s'",
						magazine, slot, switch_board.control_ipna_str,
						switch_board.control_ipnb_str,
						switch_board.transport_ipna_str,
						switch_board.transport_ipnb_str);

				fixs_ith::workingSet_t::instance()->get_sbdatamanager().add_switchboard(
						switch_board);

				FIXS_ITH_LOG(LOG_LEVEL_INFO,
						"Adding BNC objects for this new switch board in IMM ...");

				std::string switchboard_key;
				common::utility::build_sb_key_from_magazine_and_slot(
						switchboard_key, magazine_str,
						switch_board.slot_position);

				fixs_ith::workingSet_t::instance()->schedule_until_completion(
						switchboard_key, operation::BNC_OBJECTS_INSTANTIATE, 0,
						3, switchboard_key.c_str(),
						(switchboard_key.length() + 1) / sizeof(uint8_t));
				FIXS_ITH_LOG(LOG_LEVEL_INFO,
						"Operation is scheduled to create the BNC objects for CMX board on slot ==%d",
						slot);

			} else { // Changing a switch board
				FIXS_ITH_LOG(LOG_LEVEL_INFO,
						"Changing this switch board into the SNMP manager internal structures: magazine == 0x%08X, slot == %d, ControlIPNA == '%s', ControlIPNB == '%s', TransportIPNA == '%s', TransportIPNB == '%s'",
						magazine, slot, switch_board.control_ipna_str,
						switch_board.control_ipnb_str,
						switch_board.transport_ipna_str,
						switch_board.transport_ipnb_str);
			}

		} else {
			//INSERT HERE OTHER CUSTOMIZED OPERATION //
		}

		FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
				"HW Inventory: HWC table notification from CS: op_type == '%s', board_magazine == '%s', board_slot == %u",
				hwc_table_change_op_type_str(op_type), magazine_str, slot);
	}
}

int fixs_ith_cshwctablechangeobserver::start(
		ACS_CS_API_SubscriptionMgr * subscription_manager) {
	ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_sync_object, true);

	if (_state == OBSERVER_STATE_STARTED) {
		FIXS_ITH_LOG(LOG_LEVEL_WARN,
				"Observer " FIXS_ITH_STRINGIZE(fixs_ith_cshwctablechangeobserver) " already started");
		return fixs_ith::ERR_NO_ERRORS;
	}

	// Getting the subscription manager if not provided into the subscription_manager parameter
	subscription_manager || (subscription_manager =
			ACS_CS_API_SubscriptionMgr::getInstance());

	if (!subscription_manager) { // ERROR: getting the subscription manager from CS API
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"Call 'getInstance' failed: cannot get the subscription manager from the CS-API");
		return fixs_ith::ERR_CS_GET_INSTANCE;
	}

	FIXS_ITH_LOG(LOG_LEVEL_INFO,
			"Subscribing ITH server to the CS HWC table change notification interface...");

	ACS_CS_API_NS::CS_API_Result cs_call_result =
			subscription_manager->subscribeHWCTableChanges(*this);

	if (cs_call_result != ACS_CS_API_NS::Result_Success) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"Call 'subscribeHWCTableChanges' failed: cs_call_result == %d",
				cs_call_result);
		return fixs_ith::ERR_CS_ERROR;
	}

	FIXS_ITH_LOG(LOG_LEVEL_INFO,
			"ITH server successfully subscribed to the CS HWC table change notification interface");

	_state = OBSERVER_STATE_STARTED;

	return fixs_ith::ERR_NO_ERRORS;
}

int fixs_ith_cshwctablechangeobserver::stop(
		ACS_CS_API_SubscriptionMgr * subscription_manager) {
	ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_sync_object, true);

	if (_state == OBSERVER_STATE_STOPPED)
		return fixs_ith::ERR_NO_ERRORS;

	// Getting the subscription manager if not provided into the subscription_manager parameter
	subscription_manager || (subscription_manager =
			ACS_CS_API_SubscriptionMgr::getInstance());

	if (!subscription_manager) { // ERROR: getting the subscription manager from CS API
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"Call 'getInstance' failed: cannot get the subscription manager from the CS-API");
		return fixs_ith::ERR_CS_GET_INSTANCE;
	}

	FIXS_ITH_LOG(LOG_LEVEL_INFO,
			"Unsubscribing ITH server from the CS HWC table change notification interface...");

	ACS_CS_API_NS::CS_API_Result cs_call_result =
			subscription_manager->unsubscribeHWCTableChanges(*this);

	if (cs_call_result != ACS_CS_API_NS::Result_Success) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"Call 'unsubscribeHWCTableChanges' failed: cs_call_result == %d",
				cs_call_result);
		return fixs_ith::ERR_CS_ERROR;
	}

	FIXS_ITH_LOG(LOG_LEVEL_INFO,
			"ITH server successfully unsubscribed from the CS HWC table change notification interface");

	_state = OBSERVER_STATE_STOPPED;

	return fixs_ith::ERR_NO_ERRORS;
}

const char * fixs_ith_cshwctablechangeobserver::hwc_table_change_op_type_str(
		ACS_CS_API_TableChangeOperation::OpType op_type) const {
	switch (op_type) {
	case ACS_CS_API_TableChangeOperation::Add:
		return "ADD";
	case ACS_CS_API_TableChangeOperation::Change:
		return "CHANGE";
	case ACS_CS_API_TableChangeOperation::Delete:
		return "DELETE";
	case ACS_CS_API_TableChangeOperation::Unspecified:
		return "UNSPECIFIED";
	}

	return "UNKNOWN";
}

