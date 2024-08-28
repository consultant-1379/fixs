/*
 * fixs_ith_csreader.cpp
 *
 *  Created on: Jan 27, 2016
 *      Author: xludesi
 */


#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <set>

#include <ace/Guard_T.h>

#include <ACS_CS_API.h>
#include <ACS_CS_API_Set.h>
#include <ACS_DSD_Server.h>

#include <acs_apgcc_omhandler.h>
#include <acs_apgcc_paramhandling.h>

#include "fixs_ith_programmacros.h"
#include "fixs_ith_logger.h"
#include "fixs_ith_cmdoptionparser.h"
#include "common/utility.h"

#include "fixs_ith_csreader.h"


int fixs_ith_csreader::_environment = -1;


int fixs_ith_csreader::bind_to_cs () {
	ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_sync_object, true);
	if (_hwc) return fixs_ith::ERR_NO_ERRORS;

	ACS_CS_API_HWC * hwc = ACS_CS_API::createHWCInstance();
	if (!hwc) { // ERROR: creating HWC CS instance
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'createHWCInstance' failed: cannot create the HWC instance to access CS configuration data");
		return fixs_ith::ERR_CS_CREATE_HWC_INSTANCE;
	}

	_hwc = hwc;

	return fixs_ith::ERR_NO_ERRORS;
}

int fixs_ith_csreader::unbind () {
	ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_sync_object, true);
	if (!_hwc) return fixs_ith::ERR_NO_ERRORS;

	ACS_CS_API::deleteHWCInstance(_hwc);

	_hwc = 0;

	return fixs_ith::ERR_NO_ERRORS;
}

int fixs_ith_csreader::get_my_ap_node (ACS_DSD_Node & node) {
	if (_my_ap_node.system_id == acs_dsd::SYSTEM_ID_UNKNOWN) {
		ACS_DSD_Server dsd_server;
		ACS_DSD_Node my_ap_node;

		if (dsd_server.get_local_node(my_ap_node)) { // ERROR: getting my AP node by DSD
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
					"Call 'get_local_node' failed: cannot retrieve my AP node info from DSD: DSD last error == %d, DSD laste error text == '%s'",
					dsd_server.last_error(), dsd_server.last_error_text());
			return fixs_ith::ERR_DSD_GET_LOCAL_NODE;
		}

		_my_ap_node = my_ap_node;
	}

	node = _my_ap_node;

	return fixs_ith::ERR_NO_ERRORS;
}

int fixs_ith_csreader::get_my_ipn_addresses (char (& ipna_str) [16], char (& ipnb_str) [16]) {
	if (const int call_result = get_my_ap_all_info()) return call_result;

	::strncpy(ipna_str, _my_ipna_str, FIXS_ITH_ARRAY_SIZE(ipna_str));
	::strncpy(ipnb_str, _my_ipnb_str, FIXS_ITH_ARRAY_SIZE(ipnb_str));

	return fixs_ith::ERR_NO_ERRORS;
}


int fixs_ith_csreader::get_my_magazine (uint32_t & magazine) {
	if (const int call_result = get_my_ap_all_info()) return call_result;

	magazine = _my_magazine;

	return fixs_ith::ERR_NO_ERRORS;
}

int fixs_ith_csreader::get_my_magazine (char (& magazine) [16]) {
	if (const int call_result = get_my_ap_all_info()) return call_result;

	::strncpy(magazine, _my_magazine_str, FIXS_ITH_ARRAY_SIZE(magazine));

	return fixs_ith::ERR_NO_ERRORS;
}

int fixs_ith_csreader::get_my_system_number (uint16_t &sys_no) {
	if (const int call_result = get_my_ap_all_info()) return call_result;

	sys_no = _my_system_number;

	return fixs_ith::ERR_NO_ERRORS;
}

int fixs_ith_csreader::get_other_magazines (std::vector <uint32_t> & magazines) {
	std::vector <fixs_ith_switchboardinfo> other_boards;

	if (const int call_result = get_other_switch_boards_info(other_boards)) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'get_other_switch_boards_info' failed: cannot get other magazine address from CS: call_result == %d", call_result);
		return call_result;
	}

	std::set <uint32_t> values;

	for (size_t i = 0; i < other_boards.size(); ++i) values.insert(other_boards[i].magazine);

	magazines.assign(values.begin(), values.end());

	return fixs_ith::ERR_NO_ERRORS;
}

int fixs_ith_csreader::get_all_magazines (std::vector <uint32_t> & magazines) {
	uint32_t my_magazine = 0;

	if (const int call_result = get_my_magazine(my_magazine)) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'get_my_magazine' failed: cannot get my magazine address from CS: call_result == %d", call_result);
		return call_result;
	}

	std::vector <fixs_ith_switchboardinfo> other_boards;

	if (const int call_result = get_other_switch_boards_info(other_boards)) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'get_other_switch_boards_info' failed: cannot get other magazine address from CS: call_result == %d", call_result);
		return call_result;
	}

	std::set <uint32_t> values;
	values.insert(my_magazine);

	for (size_t i = 0; i < other_boards.size(); ++i) values.insert(other_boards[i].magazine);

	magazines.assign(values.begin(), values.end());

	return fixs_ith::ERR_NO_ERRORS;
}

int fixs_ith_csreader::map_magazine_plug_number (uint32_t & magazine, int mag_plug_number) {
	std::vector <uint32_t> magazines;
	if (const int call_result = get_all_magazines(magazines)) { //ERROR: retrieving magazine addresses
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'get_all_magazines' failed: cannot get all magazine address from CS: "
				"call_result == %d", call_result);
		return call_result;
	}

	int return_code = fixs_ith::ERR_NO_ERRORS;
	size_t i = 0;

	mag_plug_number &= 0x0F;
	for (i = 0; (i < magazines.size()) && ((magazines[i] & 0x0F) != static_cast<uint32_t>(mag_plug_number)); ++i) ;

	if (i < magazines.size()) magazine = magazines[i];
	else return_code = fixs_ith::ERR_CS_GET_MAGAZINE;

	return return_code;
}

int fixs_ith_csreader::get_my_slot (int32_t & slot) {
	if (const int call_result = get_my_ap_all_info()) return call_result;

	slot = _my_slot_position;

	return fixs_ith::ERR_NO_ERRORS;
}


int fixs_ith_csreader::get_my_side(uint16_t & side) {
	if (const int call_result = get_my_ap_all_info()) return call_result;

	side = _my_side;

	return fixs_ith::ERR_NO_ERRORS;
}

int fixs_ith_csreader::get_my_partner_slot (int32_t & slot) {
	if (const int call_result = get_my_ap_all_info()) return call_result;

	slot = _my_partner_slot_position;

	return fixs_ith::ERR_NO_ERRORS;
}


int fixs_ith_csreader::get_environment(int & environment)
{
	if (_environment >= 0) { environment = _environment; return fixs_ith::ERR_NO_ERRORS; }
	ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_sync_object, true);
	if (_environment >= 0) { environment = _environment; return fixs_ith::ERR_NO_ERRORS; }

	ACS_CS_API_CommonBasedArchitecture::ArchitectureValue nodeArchitecture;

	ACS_CS_API_NS::CS_API_Result cs_call_result = ACS_CS_API_NetworkElement::getNodeArchitecture(nodeArchitecture);

	if (cs_call_result != ACS_CS_API_NS::Result_Success) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'getNodeArchitecture' failed: cannot get the NodeArchitecture from CS: cs_call_result == %d", cs_call_result);
		return fixs_ith::ERR_CS_ERROR;
	}

	_environment = (environment = nodeArchitecture);

	return fixs_ith::ERR_NO_ERRORS;
}


int fixs_ith_csreader::get_switch_boards_info (std::vector <fixs_ith_switchboardinfo> & boards, uint32_t magazine, bool match) {
	ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_sync_object, true);

	if (const int call_result = bind_to_cs()) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'bind_to_cs' failed: cannot bind the CS API to read configuration data: call_result == %d", call_result);
		return call_result;
	}

	uint32_t my_magazine = 0;

	if (const int call_result = get_my_magazine(my_magazine)) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'get_my_magazine' failed: cannot get my magazine address from CS: call_result == %d", call_result);
		return call_result;
	}

	ACS_CS_API_BoardSearch * bs = ACS_CS_API_HWC::createBoardSearchInstance();
	if (!bs) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'createBoardSearchInstance' failed: cannot create a CS board seach instance to perform a configuration search");
		return fixs_ith::ERR_CS_CREATE_BOARD_SEARCH_INSTANCE;
	}

	// search criteria
	bs->reset();
        if(common::utility::isSwitchBoardCMX()){
           bs->setFBN(SWITCH_BOARD_FBN_CMXB);
        }
        else
	   bs->setFBN(SWITCH_BOARD_FBN_SMXB);

	if (match) bs->setMagazine(magazine);

	// Now I can search
	ACS_CS_API_IdList board_list;
	ACS_CS_API_NS::CS_API_Result cs_call_result = _hwc->getBoardIds(board_list, bs);
	if (cs_call_result != ACS_CS_API_NS::Result_Success) { // ERROR: getting board ids from CS
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'getBoardIds' failed: cannot get switch board ids from CS: cs_call_result == %d", cs_call_result);
		ACS_CS_API_HWC::deleteBoardSearchInstance(bs);
		unbind();
		return fixs_ith::ERR_CS_GET_BOARD_ID;
	}

	//FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "board_list.size() == %zu", board_list.size());

	// Clear the vector
	boards.clear();

	if (board_list.size() <= 0) { // NO switch board found in the system
		if (match)
			FIXS_ITH_LOG(LOG_LEVEL_WARN, "No Switch boards found for the magazine 0x%08X in the CS system configuration table", magazine);
		else
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "No Switch boards found in all other magazines but the one 0x%08X in the CS system configuration table", magazine);

		ACS_CS_API_HWC::deleteBoardSearchInstance(bs);
		unbind();
		return fixs_ith::ERR_NO_ERRORS;
	}

	// Retrieving switch board info
	int undo = 0;
	uint16_t slot_position;
	char ipna_address [16];
	char ipnb_address [16];
	char alias_ipna_address [16];
	char alias_ipnb_address [16];
	uint32_t sb_magazine = magazine;
	fixs_ith_switchboardinfo switch_board_info;
	int return_code = fixs_ith::ERR_NO_ERRORS;

	// Retrieving information for each switch board in my magazine
	for (size_t i = 0; i < board_list.size(); ++i) {
		BoardID board_id = board_list[i];

		if (!match) {
			if ((cs_call_result = _hwc->getMagazine(sb_magazine, board_id)) != ACS_CS_API_NS::Result_Success) {
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'getMagazine' failed: cannot get the magazine address for the current switch board searched (i = %zu): cs_call_result == %d", i, cs_call_result);
				return_code = fixs_ith::ERR_CS_GET_MAGAZINE;
				undo = 1; break;
			}

			if (sb_magazine == magazine) continue;
		}

/*
		// Discard all switch board that do not solve the match criterion
		if (match) {
			if (sb_magazine != magazine) continue;
		} else {

			if (sb_magazine == magazine) continue;
		}
*/

		if ((cs_call_result = _hwc->getSlot(slot_position, board_id)) != ACS_CS_API_NS::Result_Success) { // ERROR: getting the board slot
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'getSlot' failed: cannot get the slot number for the current switch board searched (i = %zu): cs_call_result == %d", i, cs_call_result);
			return_code = fixs_ith::ERR_CS_GET_SLOT;
			undo = 1; break;
		}

		if (const int call_result = get_ip_eth_addresses(ipna_address, ipnb_address, board_id)) {
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'get_ip_eth_addresses' failed: cannot get IPN addresses for the current switch board searched (i = %zu): call_result == %d", i, call_result);
			return_code = call_result;
			undo = 1; break;
		}

		if (const int call_result = get_alias_ip_eth_addresses(alias_ipna_address, alias_ipnb_address, board_id)) {
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'get_ip_eth_addresses' failed: cannot get IPN addresses for the current switch board searched (i = %zu): call_result == %d", i, call_result);
				return_code = call_result;
				undo = 1; break;
		}
		switch_board_info.slot_position = slot_position;
		switch_board_info.magazine = sb_magazine;
		switch_board_info.magazine_id = (my_magazine == switch_board_info.magazine)? fixs_ith::APZ_MAGAZINE: fixs_ith::APT_MAGAZINE;

                if(common::utility::isSwitchBoardSMX()){
			::strncpy(switch_board_info.control_ipna_str, ipna_address, FIXS_ITH_ARRAY_SIZE(switch_board_info.control_ipna_str));
			::strncpy(switch_board_info.control_ipnb_str, ipnb_address, FIXS_ITH_ARRAY_SIZE(switch_board_info.control_ipnb_str));
			::strncpy(switch_board_info.transport_ipna_str, alias_ipna_address, FIXS_ITH_ARRAY_SIZE(switch_board_info.transport_ipna_str));
			::strncpy(switch_board_info.transport_ipnb_str, alias_ipnb_address, FIXS_ITH_ARRAY_SIZE(switch_board_info.transport_ipnb_str));
			}

		else if(common::utility::isSwitchBoardCMX()){
                	::strncpy(switch_board_info.control_ipna_str, alias_ipna_address, FIXS_ITH_ARRAY_SIZE(switch_board_info.control_ipna_str));
	                ::strncpy(switch_board_info.control_ipnb_str, alias_ipnb_address, FIXS_ITH_ARRAY_SIZE(switch_board_info.control_ipnb_str));
	                ::strncpy(switch_board_info.transport_ipna_str, ipna_address, FIXS_ITH_ARRAY_SIZE(switch_board_info.transport_ipna_str));
	                ::strncpy(switch_board_info.transport_ipnb_str, ipnb_address, FIXS_ITH_ARRAY_SIZE(switch_board_info.transport_ipnb_str));
		}

		// Add the switch board info into the vector
		boards.push_back(switch_board_info);
	}

	ACS_CS_API_HWC::deleteBoardSearchInstance(bs);

	if (undo) { // ERROR: in CS get functions
		boards.clear();
		unbind();
		return return_code;
	}

	if (boards.size() <= 0) { // ERROR: No Switch boards found in the CS configuration table
		if (match){
			FIXS_ITH_LOG(LOG_LEVEL_WARN, "No SWITCH boards found for the magazine 0x%08X in the CS configuration table", magazine);
			unbind();
		}
		else FIXS_ITH_LOG(LOG_LEVEL_WARN, "No SWITCH boards found in all other magazine but the one 0x%08X in the CS configuration table", magazine);

	}

	return fixs_ith::ERR_NO_ERRORS;
}


int fixs_ith_csreader::get_my_switch_boards_info (std::vector <fixs_ith_switchboardinfo> & boards) {
	uint32_t my_magazine;

	if (const int call_result = get_my_magazine(my_magazine)) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'get_my_magazine' failed: cannot get my magazine address from CS: call_result == %d", call_result);
		return call_result;
	}


	return get_switch_boards_info(boards, my_magazine, true);
}

int fixs_ith_csreader::get_other_switch_boards_info (std::vector <fixs_ith_switchboardinfo> & boards) {
	uint32_t my_magazine;

	if (const int call_result = get_my_magazine(my_magazine)) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'get_my_magazine' failed: cannot get my magazine address from CS: call_result == %d", call_result);
		return call_result;
	}

	return get_switch_boards_info(boards, my_magazine, false);
}


int fixs_ith_csreader::reset_info () {
	ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_sync_object, true);

	_my_ap_node.system_id = acs_dsd::SYSTEM_ID_UNKNOWN;
	*_my_ipna_str = 0;
	*_my_ipnb_str = 0;
	_my_magazine = 0;
	*_my_magazine_str = 0;
	_my_slot_position = -1;
	_my_partner_slot_position = -1;
	_my_ap_info_loaded = 0;
	_my_side = 0;

	unbind();

	return fixs_ith::ERR_NO_ERRORS;
}

int fixs_ith_csreader::uint32_to_ip_format (char (& value_ip) [16], uint32_t value) {
	// Converting value address into IP dotted-quad format "ddd.ddd.ddd.ddd"
	in_addr addr;
	addr.s_addr = value;
	errno = 0;
	if (!::inet_ntop(AF_INET, &addr, value_ip, FIXS_ITH_ARRAY_SIZE(value_ip))) { // ERROR: in conversion
		FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR,
				"Call 'inet_ntop' failed: cannot convert uint32 address (0x%08X) in the dotted-quad format 'ddd.ddd.ddd.ddd'", value);
		value_ip[0] = 0;
		return fixs_ith::ERR_IP_ADDRESS_CONVERSION;
	}

	return fixs_ith::ERR_NO_ERRORS;
}

int fixs_ith_csreader::uint32_to_ip_format (char (& value_ip) [16], uint32_t value, char separator) {
	uint8_t ip_fields [4];

	uint32_to_ip_fields(ip_fields, value);

	::snprintf(value_ip, FIXS_ITH_ARRAY_SIZE(value_ip), "%u%c%u%c%u%c%u",
			ip_fields[0], separator, ip_fields[1], separator, ip_fields[2], separator, ip_fields[3]);

	return fixs_ith::ERR_NO_ERRORS;
}

int fixs_ith_csreader::ip_format_to_uint32 (uint32_t & value, const char * value_ip) {
	in_addr addr;

	errno = 0;
	if (const int call_result = ::inet_pton(AF_INET, value_ip, &addr) <= 0) { // ERROR: in conversion
		if (call_result) FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "Call 'inet_pton' failed");
		else FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'inet_pton' failed: value_ip parameter does not contain a valid network string address");

		return fixs_ith::ERR_IP_ADDRESS_CONVERSION;
	}

	value = addr.s_addr;

	return fixs_ith::ERR_NO_ERRORS;
}

int fixs_ith_csreader::uint32_to_ip_fields (uint8_t & ip1, uint8_t & ip2, uint8_t & ip3, uint8_t & ip4, uint32_t value) {
	uint8_t * p = reinterpret_cast<uint8_t *>(&value);

	ip1 = *p++; ip2 = *p++; ip3 = *p++; ip4 = *p++;

	return fixs_ith::ERR_NO_ERRORS;
}

int fixs_ith_csreader::uint32_to_ip_fields (uint8_t (& ip_fields) [4], uint32_t value) {
	uint8_t * p = reinterpret_cast<uint8_t *>(&value);

	for (size_t i = 0; i < FIXS_ITH_ARRAY_SIZE(ip_fields); ip_fields[i++] = *p++) ;

	return fixs_ith::ERR_NO_ERRORS;
}

int fixs_ith_csreader::ip_format_to_ip_fields (uint8_t & ip1, uint8_t & ip2, uint8_t & ip3, uint8_t & ip4, const char * value_ip) {
	uint32_t ip = 0;

	if (const int call_result = ip_format_to_uint32(ip, value_ip)) return call_result;

	return uint32_to_ip_fields(ip1, ip2, ip3, ip4, ip);
}

int fixs_ith_csreader::ip_format_to_ip_fields (uint8_t (& ip_fields) [4], const char * value_ip) {
	uint32_t ip = 0;

	if (const int call_result = ip_format_to_uint32(ip, value_ip)) return call_result;

	return uint32_to_ip_fields(ip_fields, ip);
}

int fixs_ith_csreader::get_my_ap_all_info () {
	if (_my_ap_info_loaded) return fixs_ith::ERR_NO_ERRORS;
	ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_sync_object, true);
	if (_my_ap_info_loaded) return fixs_ith::ERR_NO_ERRORS;

	if (const int call_result = bind_to_cs()) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'bind_to_cs' failed: cannot bind the CS API to read configuration data: call_result == %d", call_result);
		return fixs_ith::ERR_CS_API_BINDING;
	}

	ACS_CS_API_BoardSearch * bs = ACS_CS_API_HWC::createBoardSearchInstance();
	if (!bs) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'createBoardSearchInstance' failed: cannot create a CS board seach instance to perform a configuration search");
		return fixs_ith::ERR_CS_CREATE_BOARD_SEARCH_INSTANCE;
	}

	ACS_DSD_Node my_node;

	if (const int call_result = get_my_ap_node(my_node)) { // ERROR: getting my ap node info from DSD
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'get_my_ap_node' failed: cannot retrieve my AP node information from configuration: call_result == %d", call_result);
		ACS_CS_API_HWC::deleteBoardSearchInstance(bs);
		unbind();
		return call_result;
	}

	// Check the my_node side.
	if ((my_node.node_side != acs_dsd::NODE_SIDE_A) && (my_node.node_side != acs_dsd::NODE_SIDE_B)) { // WARNING: undefined my_node state. Cannot continue
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Undefined my node side: there are problems in the system: cannot continue to get my AP info");
		ACS_CS_API_HWC::deleteBoardSearchInstance(bs);
		unbind();
		return fixs_ith::ERR_NODE_SIDE_UNDEFINED;
	}

	//FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Searching criteria: my_node.system_id == %d, my_node.node_side == %d", my_node.system_id, my_node.node_side);

	// search criteria
	bs->reset();
	bs->setFBN(ACS_CS_API_HWC_NS::FBN_APUB);
	bs->setSysType(ACS_CS_API_HWC_NS::SysType_AP);
	bs->setSysId(my_node.system_id);
//	bs->setSide(my_node.node_side == acs_dsd::NODE_SIDE_A ? ACS_CS_API_HWC_NS::Side_A : ACS_CS_API_HWC_NS::Side_B);

	// Now I can search
	ACS_CS_API_IdList board_list;
	ACS_CS_API_NS::CS_API_Result cs_call_result = _hwc->getBoardIds(board_list, bs);
	if (cs_call_result != ACS_CS_API_NS::Result_Success) { // ERROR: getting board ids from CS
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'getBoardIds' failed: cannot get AP board ids from CS: cs_call_result == %d", cs_call_result);
		ACS_CS_API_HWC::deleteBoardSearchInstance(bs);
		unbind();
		return fixs_ith::ERR_CS_GET_BOARD_ID;
	}

	//FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "board_list.size() == %zu", board_list.size());

	// Should there be at least one board
	if (board_list.size() <= 0) { // ERROR: no boards found
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "No AP boards found in the CS system configuration table");
		ACS_CS_API_HWC::deleteBoardSearchInstance(bs);
		unbind();
		return fixs_ith::ERR_CS_NO_BOARD_FOUND;
	}

	for (size_t i = 0; i < board_list.size(); ++i) {
		BoardID board_id = board_list[i];
		uint16_t side = 0;

		if ((cs_call_result = _hwc->getSide(side, board_id)) != ACS_CS_API_NS::Result_Success) { // ERROR: getting side
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'getSide' failed: cannot retrieve the side of the board id == %u from CS: cs_call_result == %d",
					board_id, cs_call_result);
			ACS_CS_API_HWC::deleteBoardSearchInstance(bs);
			unbind();
			return fixs_ith::ERR_CS_GET_SIDE;
		}
		if (((side == ACS_CS_API_HWC_NS::Side_A) && (my_node.node_side == acs_dsd::NODE_SIDE_B)) || ((side == ACS_CS_API_HWC_NS::Side_B) && (my_node.node_side == acs_dsd::NODE_SIDE_A))) {
			// This is my partner node. I must save the slot position only

			//... my partner slot position
			uint16_t slot_position;
			if ((cs_call_result = _hwc->getSlot(slot_position, board_id)) != ACS_CS_API_NS::Result_Success) { // ERROR: getting the board slot
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'getSlot' failed: cannot get my partner slot number from CS: cs_call_result == %d", cs_call_result);
				ACS_CS_API_HWC::deleteBoardSearchInstance(bs);
				unbind();
				return fixs_ith::ERR_CS_GET_SLOT;
			}

			_my_partner_slot_position = slot_position;
		} else if (((side == ACS_CS_API_HWC_NS::Side_A) && (my_node.node_side == acs_dsd::NODE_SIDE_A)) || ((side == ACS_CS_API_HWC_NS::Side_B) && (my_node.node_side == acs_dsd::NODE_SIDE_B))) {
			// This is my node. I must save a lot of info
			_my_side = side;
			// Now I can get my IPN addresses ...
			char ipna_address [16];
			char ipnb_address [16];
			if (const int call_result = get_ip_eth_addresses(ipna_address, ipnb_address, board_id)) {
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'get_ip_eth_addresses' failed: cannot get IPN addresses for my AP: call_result == %d", call_result);
				ACS_CS_API_HWC::deleteBoardSearchInstance(bs);
				unbind();
				return call_result;
			}

			//... my magazine address ...
			uint32_t magazine;
			if ((cs_call_result = _hwc->getMagazine(magazine, board_id)) != ACS_CS_API_NS::Result_Success) { // ERROR: getting magazine info
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'getMagazine' failed: cannot retrieve my AP magazine address from CS: cs_call_result == %d", cs_call_result);
				ACS_CS_API_HWC::deleteBoardSearchInstance(bs);
				unbind();
				return fixs_ith::ERR_CS_GET_MAGAZINE;
			}

			//... my slot position
			uint16_t slot_position;
			if ((cs_call_result = _hwc->getSlot(slot_position, board_id)) != ACS_CS_API_NS::Result_Success) { // ERROR: getting the board slot
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'getSlot' failed: cannot get my slot number from CS: cs_call_result == %d", cs_call_result);
				ACS_CS_API_HWC::deleteBoardSearchInstance(bs);
				unbind();
				return fixs_ith::ERR_CS_GET_SLOT;
			}

			//... my system number
			uint16_t system_no;
			if ((cs_call_result = _hwc->getSysNo(system_no, board_id)) != ACS_CS_API_NS::Result_Success) { // ERROR: getting the system number
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'getSysNo' failed: cannot get my system number from CS: cs_call_result == %d", cs_call_result);
				ACS_CS_API_HWC::deleteBoardSearchInstance(bs);
				unbind();
				return fixs_ith::ERR_CS_GET_SYSNO;
			}

			// Converting magazine address into IP dotted-quad format "ddd.ddd.ddd.ddd"
			char magazine_str[16];

			if (const int call_result = uint32_to_ip_format(magazine_str, magazine)) {
				FIXS_ITH_LOG(LOG_LEVEL_ERROR,
						"Call 'uint32_to_ip_format' failed: cannot convert magazine address (0x%08X) in the dotted-quad format 'ddd.ddd.ddd.ddd'", magazine);
				ACS_CS_API_HWC::deleteBoardSearchInstance(bs);
				return call_result;
			}


			// Now I can get my BGCI addresses if we're in BSP environment
			int env = -1;
			if (const int call_result = get_environment(env))
			{
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'get_environment' failed: cannot get my BGCI addresses from CS.");
				ACS_CS_API_HWC::deleteBoardSearchInstance(bs);
				unbind();
				return call_result;
			}
			// Saving all info into the internal class fields
			::strncpy(_my_ipna_str, ipna_address, FIXS_ITH_ARRAY_SIZE(_my_ipna_str));
			::strncpy(_my_ipnb_str, ipnb_address, FIXS_ITH_ARRAY_SIZE(_my_ipnb_str));

			_my_magazine = magazine;
			::strncpy(_my_magazine_str, magazine_str, FIXS_ITH_ARRAY_SIZE(_my_magazine_str));
			_my_slot_position = slot_position;
			_my_system_number = system_no;
		}
	}

	ACS_CS_API_HWC::deleteBoardSearchInstance(bs);

	_my_ap_info_loaded = 1;

	return fixs_ith::ERR_NO_ERRORS;
}


int fixs_ith_csreader::get_ip_eth_addresses (char (& ipna_address) [16], char (& ipnb_address) [16],  BoardID board_id)
{
	ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_sync_object, true);

	uint32_t ipn_addresses[2];

	for (int i = 0; i < 2; ++i){

		int cs_call_result = (i ? _hwc->getIPEthB(ipn_addresses[i], board_id): _hwc->getIPEthA(ipn_addresses[i], board_id));
                if (cs_call_result != ACS_CS_API_NS::Result_Success) {
			// ERROR: getting the IPN addresses
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
					"Call 'getIPEth%c' failed: cannot get an IPN%c address from CS: cs_call_result == %d", 'A' + i, 'A' + i, cs_call_result);
			return fixs_ith::ERR_CS_GET_IP_ETH;
		}
    }

	// Converting IP addresses in the dotted-quad format "ddd.ddd.ddd.ddd"
	char ip_str[2][16];

	for (int i = 0; i < 2; ++i) {
		uint32_t address = htonl(ipn_addresses[i]);
		if (const int call_result = uint32_to_ip_format(ip_str[i], address)) {
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
					"Call 'uint32_to_ip_format' failed: cannot convert IP address (0x%08X) in the dotted-quad format 'ddd.ddd.ddd.ddd'", address);
			return call_result;
		}
	}

	::strncpy(ipna_address, ip_str[0], FIXS_ITH_ARRAY_SIZE(ipna_address));
	::strncpy(ipnb_address, ip_str[1], FIXS_ITH_ARRAY_SIZE(ipnb_address));

	return fixs_ith::ERR_NO_ERRORS;
}

int fixs_ith_csreader::get_alias_ip_eth_addresses (char (& alias_ipna_address) [16], char (& alias_ipnb_address) [16], BoardID board_id)
{
	ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_sync_object, true);

	uint32_t alias_ipn_addresses[2];

	for (int i = 0; i < 2; ++i){

		int cs_call_result = (i ? _hwc->getAliasEthA(alias_ipn_addresses[i], board_id): _hwc->getAliasEthB(alias_ipn_addresses[i], board_id));
		if (cs_call_result != ACS_CS_API_NS::Result_Success) {
			// ERROR: getting the IPN addresses
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
					"Call 'getIPEth%c' failed: cannot get an AliasIPN%c address from CS: cs_call_result == %d", 'A' + i, 'A' + i, cs_call_result);
			return fixs_ith::ERR_CS_GET_IP_ETH;
		}
	}

	// Converting IP addresses in the dotted-quad format "ddd.ddd.ddd.ddd"
	char alias_ip_str[2][16];

	for (int i = 0; i < 2; ++i) {
		uint32_t alias_address = htonl(alias_ipn_addresses[i]);

		if (const int call_result = uint32_to_ip_format(alias_ip_str[i], alias_address)) {
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
					"Call 'uint32_to_ip_format' failed: cannot convert IP address (0x%08X) in the dotted-quad format 'ddd.ddd.ddd.ddd'", alias_address);
			return call_result;
		}
	}

	::strncpy(alias_ipna_address, alias_ip_str[0], FIXS_ITH_ARRAY_SIZE(alias_ipna_address));
	::strncpy(alias_ipnb_address, alias_ip_str[1], FIXS_ITH_ARRAY_SIZE(alias_ipnb_address));

	return fixs_ith::ERR_NO_ERRORS;
}

int fixs_ith_csreader::get_switch_boards_info (std::vector <fixs_ith_switchboardinfo> & boards) {
	ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_sync_object, true);

	if (const int call_result = bind_to_cs()) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'bind_to_cs' failed: cannot bind the CS API to read configuration data: call_result == %d", call_result);
		return call_result;
	}

	uint32_t my_magazine;
	if (const int call_result = get_my_magazine(my_magazine))
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'get_my_magazine' failed: cannot get my magazine: call_result == %d", call_result);
		return call_result;
	}

	ACS_CS_API_BoardSearch * bs = ACS_CS_API_HWC::createBoardSearchInstance();
	if (!bs) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'createBoardSearchInstance' failed: cannot create a CS board seach instance to perform a configuration search");
		return fixs_ith::ERR_CS_CREATE_BOARD_SEARCH_INSTANCE;
	}

	// search criteria
	bs->reset();
        if(common::utility::isSwitchBoardCMX()){
	   bs->setFBN(SWITCH_BOARD_FBN_CMXB);
        }else { bs->setFBN(SWITCH_BOARD_FBN_SMXB); }

	// Now I can search
	ACS_CS_API_IdList board_list;
	ACS_CS_API_NS::CS_API_Result cs_call_result = _hwc->getBoardIds(board_list, bs);
	if (cs_call_result != ACS_CS_API_NS::Result_Success) { // ERROR: getting board ids from CS
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'getBoardIds' failed: cannot get switch board ids from CS: cs_call_result == %d", cs_call_result);
		ACS_CS_API_HWC::deleteBoardSearchInstance(bs);
		unbind();
		return fixs_ith::ERR_CS_GET_BOARD_ID;
	}

	//FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "board_list.size() == %zu", board_list.size());

	// Clear the vector
	boards.clear();

	if (board_list.size() <= 0) { // NO switch board found in the system
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "No Switch boards found in in the CS system configuration table");

		ACS_CS_API_HWC::deleteBoardSearchInstance(bs);
		unbind();
		return fixs_ith::ERR_NO_ERRORS;
	}

	// Retrieving switch board info
	int undo = 0;
	uint16_t slot_position;
	char ipna_address [16];
	char ipnb_address [16];
	char alias_ipna_address [16];
	char alias_ipnb_address [16];
	uint32_t sb_magazine = 0;
	fixs_ith_switchboardinfo switch_board_info;
	int return_code = fixs_ith::ERR_NO_ERRORS;

	// Retrieving information for each switch board in my magazine
	for (size_t i = 0; i < board_list.size(); ++i) {
		BoardID board_id = board_list[i];

		if ((cs_call_result = _hwc->getMagazine(sb_magazine, board_id)) != ACS_CS_API_NS::Result_Success) {
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'getMagazine' failed: cannot get the magazine address for the current switch board searched (i = %zu): cs_call_result == %d", i, cs_call_result);
				return_code = fixs_ith::ERR_CS_GET_MAGAZINE;
				undo = 1; break;
		}

		if ((cs_call_result = _hwc->getSlot(slot_position, board_id)) != ACS_CS_API_NS::Result_Success) { // ERROR: getting the board slot
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'getSlot' failed: cannot get the slot number for the current switch board searched (i = %zu): cs_call_result == %d", i, cs_call_result);
			return_code = fixs_ith::ERR_CS_GET_SLOT;
			undo = 1; break;
		}

		if (const int call_result = get_ip_eth_addresses(ipna_address, ipnb_address, board_id)) {
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'get_ip_eth_addresses' failed: cannot get IPN addresses for the current switch board searched (i = %zu): call_result == %d", i, call_result);
			return_code = call_result;
			undo = 1; break;
		}
		if (const int call_result = get_alias_ip_eth_addresses(alias_ipna_address, alias_ipnb_address, board_id)) {
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'get_ip_eth_addresses' failed: cannot get IPN addresses for the current switch board searched (i = %zu): call_result == %d", i, call_result);
			return_code = call_result;
			undo = 1; break;
		}
		switch_board_info.slot_position = slot_position;
		switch_board_info.magazine = sb_magazine;
		switch_board_info.magazine_id = (my_magazine == switch_board_info.magazine)? fixs_ith::APZ_MAGAZINE: fixs_ith::APT_MAGAZINE;

          	if(common::utility::isSwitchBoardSMX())
		{
			::strncpy(switch_board_info.control_ipna_str, ipna_address, FIXS_ITH_ARRAY_SIZE(switch_board_info.control_ipna_str));
			::strncpy(switch_board_info.control_ipnb_str, ipnb_address, FIXS_ITH_ARRAY_SIZE(switch_board_info.control_ipnb_str));
			::strncpy(switch_board_info.transport_ipna_str, alias_ipna_address, FIXS_ITH_ARRAY_SIZE(switch_board_info.transport_ipna_str));
			::strncpy(switch_board_info.transport_ipnb_str, alias_ipnb_address, FIXS_ITH_ARRAY_SIZE(switch_board_info.transport_ipnb_str));
		}
                else if(common::utility::isSwitchBoardCMX())
                {
                        ::strncpy(switch_board_info.control_ipna_str, alias_ipna_address, FIXS_ITH_ARRAY_SIZE(switch_board_info.control_ipna_str));
                        ::strncpy(switch_board_info.control_ipnb_str, alias_ipnb_address, FIXS_ITH_ARRAY_SIZE(switch_board_info.control_ipnb_str));
                        ::strncpy(switch_board_info.transport_ipna_str, ipna_address, FIXS_ITH_ARRAY_SIZE(switch_board_info.transport_ipna_str));
                        ::strncpy(switch_board_info.transport_ipnb_str, ipnb_address, FIXS_ITH_ARRAY_SIZE(switch_board_info.transport_ipnb_str));
                }


		// Add the switch board info into the vector
		boards.push_back(switch_board_info);
	}

	ACS_CS_API_HWC::deleteBoardSearchInstance(bs);

	if (undo) { // ERROR: in CS get functions
		boards.clear();
		unbind();
		return return_code;
	}

	if (boards.size() <= 0) { // ERROR: No Switch boards found in the CS configuration table

		FIXS_ITH_LOG(LOG_LEVEL_WARN, "No SWITCH boards found in all magazines");

	}

	return fixs_ith::ERR_NO_ERRORS;
}





