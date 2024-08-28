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
//	****************************************************

#include <cstdio>
#include <string>
#include <vector>
#include <ace/Recursive_Thread_Mutex.h>
#include <ace/Guard_T.h>
#include "fixs_ith_programconstants.h"
#include "fixs_ith_logger.h"
#include "fixs_ith_sbdatamanager.h"
#include "fixs_ith_workingset.h"

fixs_ith_sbdatamanager::sbm_constant fixs_ith_sbdatamanager::load_data_from_cs() {
	ACE_Guard<ACE_Recursive_Thread_Mutex> guard_switchboards(
			_switchboards_mutex);
	return _load_data_from_cs_helper();
}

fixs_ith_sbdatamanager::sbm_constant fixs_ith_sbdatamanager::get_switchboard_info(
		fixs_ith_switchboardinfo & switchboardInfo, const std::string & mag_str,
		fixs_ith::BoardSlotPosition pos) {
	std::string sb_key;
	_build_sb_key_from_magazine_and_slot(sb_key, mag_str, pos);
	FIXS_ITH_LOG(LOG_LEVEL_ERROR, "SWITCH BOARD having key '%s'",
			sb_key.c_str());

	return get_switchboard_info(switchboardInfo, sb_key);
}

fixs_ith_sbdatamanager::sbm_constant fixs_ith_sbdatamanager::get_switchboard_info(
		fixs_ith_switchboardinfo & switchboardInfo,
		const std::string & switchboard_key) {
	ACE_Guard<ACE_Recursive_Thread_Mutex> guard_switchboards(
			_switchboards_mutex);

	// check if switch boards info have been loaded from CS
	if (!_cs_info_loaded) {	// info not yet loaded from CS
		sbm_constant op_res = _load_data_from_cs_helper();
		if (op_res == SBM_OK) {
			_cs_info_loaded = true;
		} else {
			return op_res;
		}
	}

	// search in the internal map the switch board having the provided KEY
	SwitchboardsMap::iterator it = _switchboards.find(switchboard_key);
	if (it == _switchboards.end()) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"Unable to find in the internal map the SWITCH BOARD having key '%s'",
				switchboard_key.c_str());
		return SBM_BOARD_NOT_FOUND; // NO switch board found with the provided KEY
	}

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
			"Found in the internal map the SWITCH BOARD having key '%s'",
			switchboard_key.c_str());

	// fill the output parameter
	switchboardInfo = it->second;

	return SBM_OK;
}

fixs_ith_sbdatamanager::sbm_constant fixs_ith_sbdatamanager::get_switchboard_info(
		fixs_ith_switchboardinfo & switchboardInfo,
		std::string & switch_board_mag_str,
		fixs_ith::magazine_identifier_t mag_id,
		fixs_ith::BoardSlotPosition pos) {
	ACE_Guard<ACE_Recursive_Thread_Mutex> guard_switchboards(
			_switchboards_mutex);

	// check if switch boards info have been loaded from CS
	if (!_cs_info_loaded) {
		// info not yet loaded from CS
		sbm_constant op_res = _load_data_from_cs_helper();
		if (op_res == SBM_OK)
			_cs_info_loaded = true;
		else
			return op_res;
	}

	SwitchboardsMap::const_iterator sbit;
	for (sbit = _switchboards.begin();
			(sbit != _switchboards.end())
					&& ((sbit->second.magazine_id != mag_id)
							|| (sbit->second.slot_position != pos)); ++sbit)
		;

	if (sbit == _switchboards.end()) {
		// WARINING: SMX NOT FOUND
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
				"SWITCH BOARD NOT FOUND into internal map: mag_id == <%d>, pos == <%d>",
				mag_id, pos);
		return SBM_BOARD_NOT_FOUND;
	}

	// fill the output parameter
	switchboardInfo = sbit->second;

	switch_board_mag_str = sbit->first.substr(0,
			sbit->first.find_first_of('_'));

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
			"SWITCH BOARD found into internal map: mag_id == <%d>, pos == <%d>, magazine == <0x%08X>, mag_key == <%s>",
			mag_id, pos, switchboardInfo.magazine,
			switch_board_mag_str.c_str());

	return SBM_OK;
}

fixs_ith_sbdatamanager::sbm_constant fixs_ith_sbdatamanager::get_all_switchboards_info(
		std::vector<fixs_ith_switchboardinfo> & switchboards_info) {
	ACE_Guard<ACE_Recursive_Thread_Mutex> guard_switchboards(
			_switchboards_mutex);

	// check if switch boards info have been loaded from CS
	if (!_cs_info_loaded) {	// info not yet loaded from CS
		sbm_constant op_res = _load_data_from_cs_helper();
		if (op_res == SBM_OK) {
			_cs_info_loaded = true;
		} else {
			return op_res;
		}
	}

	// fill output parameter
	switchboards_info.clear();
	for (SwitchboardsMap::iterator it = _switchboards.begin();
			it != _switchboards.end(); ++it)
		switchboards_info.push_back(it->second);

	return SBM_OK;
}

fixs_ith_sbdatamanager::sbm_constant fixs_ith_sbdatamanager::get_switchboard_plane_addresses(
		char (&switch_board_plane_ipn_str_addresses)[FIXS_ITH_CONFIG_IPN_COUNT][16],
		const std::string & mag_str, fixs_ith::BoardSlotPosition pos,
		fixs_ith::switchboard_plane_t plane) {
	fixs_ith_sbdatamanager::sbm_constant retval = SBM_OK;

	std::string switchboard_key;
	_build_sb_key_from_magazine_and_slot(switchboard_key, mag_str, pos);

	// search in the internal map the switch board having the provided KEY
	ACE_Guard<ACE_Recursive_Thread_Mutex> guard_switchboards(
			_switchboards_mutex);

	SwitchboardsMap::iterator it = _switchboards.find(switchboard_key);
	if (it == _switchboards.end()) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"Unable to find in the internal map the Switch Board having key '%s'",
				switchboard_key.c_str());
		return SBM_BOARD_NOT_FOUND; // NO switch board found with the provided KEY
	}

	// retrieve from switch board info the IP addresses associated to the BRIDGE identified by the 'plane' parameter
	fixs_ith_switchboardinfo & switchboardInfo = (*it).second;
	switch (plane) {
	case fixs_ith::CONTROL_PLANE:
		::strncpy(switch_board_plane_ipn_str_addresses[0],
				switchboardInfo.control_ipna_str,
				FIXS_ITH_ARRAY_SIZE(switch_board_plane_ipn_str_addresses[0]));
		::strncpy(switch_board_plane_ipn_str_addresses[1],
				switchboardInfo.control_ipnb_str,
				FIXS_ITH_ARRAY_SIZE(switch_board_plane_ipn_str_addresses[1]));
		break;

	case fixs_ith::TRANSPORT_PLANE:
		::strncpy(switch_board_plane_ipn_str_addresses[0],
				switchboardInfo.transport_ipna_str,
				FIXS_ITH_ARRAY_SIZE(switch_board_plane_ipn_str_addresses[0]));
		::strncpy(switch_board_plane_ipn_str_addresses[1],
				switchboardInfo.transport_ipnb_str,
				FIXS_ITH_ARRAY_SIZE(switch_board_plane_ipn_str_addresses[1]));
		break;

	default:
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Invalid Plane value: %d ", plane);
		retval = SBM_INVALID_PLANE;
	}

	return retval;
}

fixs_ith_sbdatamanager::sbm_constant fixs_ith_sbdatamanager::get_switchboard_keys(
		std::set<std::string>& switchboardKeys) {
	ACE_Guard<ACE_Recursive_Thread_Mutex> guard_switchboards(
			_switchboards_mutex);

	// check if switch boards info have been loaded from CS
	if (!_cs_info_loaded) {	// info not yet loaded from CS
		sbm_constant op_res = _load_data_from_cs_helper();
		if (op_res == SBM_OK) {
			_cs_info_loaded = true;
		} else {
			return op_res;
		}
	}

	// search in the internal map the switch board having the provided KEY
	for (SwitchboardsMap::iterator it = _switchboards.begin();
			it != _switchboards.end(); ++it) {
		switchboardKeys.insert((*it).first);
	}

	return SBM_OK;
}

fixs_ith_sbdatamanager::sbm_constant fixs_ith_sbdatamanager::_build_sb_key_from_magazine_and_slot(
		std::string & key, const std::string & mag_str,
		fixs_ith::BoardSlotPosition slot_pos) {
	// build the key combining MAGAZINE string and SLOT position
	char mag_slot_str[32] = { 0 };
	::snprintf(mag_slot_str, FIXS_ITH_ARRAY_SIZE(mag_slot_str), "%s_%d",
			mag_str.c_str(), slot_pos);

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
			"@RollingStone _build_sb_key_from_magazine_and_slot %s",
			mag_slot_str);
	// return the key in the output parameter
	key.assign(mag_slot_str);
	return SBM_OK;
}

fixs_ith_sbdatamanager::sbm_constant fixs_ith_sbdatamanager::_load_data_from_cs_helper() {
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Loading switch boards data from CS ...");

	// Fetch from CS the collection of the switch boards defined in the HWC table
	std::vector<fixs_ith_switchboardinfo> cs_switch_boards;
	int call_result =
			fixs_ith::workingSet_t::instance()->get_cs_reader().get_switch_boards_info(
					cs_switch_boards);
	if (call_result != fixs_ith::ERR_NO_ERRORS) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"Unable to load switch boards info from CS: call_result == %d",
				call_result);
		return SBM_LOAD_FROM_CS_ERROR;
	}

	_switchboards.clear();  // start clearing current content of the map

	for (std::vector<fixs_ith_switchboardinfo>::iterator it =
			cs_switch_boards.begin(); it != cs_switch_boards.end(); ++it) {
		// get MAGAZINE string (example: 1.2.0.4)
		char mag_str[16] = { 0 };
		fixs_ith_csreader::uint32_to_ip_format(mag_str, (*it).magazine, '.');

		// build the key for the switch board, combining MAGAZINE string and SLOT position
		char mag_slot_str[32] = { 0 };
		::snprintf(mag_slot_str, FIXS_ITH_ARRAY_SIZE(mag_slot_str), "%s_%d",
				mag_str, (*it).slot_position);
		std::string sb_key_str(mag_slot_str);

		// insert the entry for the switch board in the internal map
		_switchboards.insert(SwitchboardsMap::value_type(sb_key_str, *it));

		FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
				"Found switch board entry: MAG == %s, SLOT = %d", mag_str,
				(*it).slot_position);
	}

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
			"Successfully loaded switch boards data from CS.");

	return SBM_OK;
}

fixs_ith_sbdatamanager::sbm_constant fixs_ith_sbdatamanager::build_sb_key_from_magazine_and_slot(
		std::string & key, uint32_t magazine, int32_t slot_pos) {
	char mag_str[16] = { 0 };
	fixs_ith_csreader::uint32_to_ip_format(mag_str, magazine, '.');

	fixs_ith::board_slotposition_t sb_pos;
	if (common::utility::isSwitchBoardSMX()) {
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "SMX slot_pos %d", slot_pos);
		sb_pos =
				(slot_pos == 0) ?
						fixs_ith::BOARD_SMX_LEFT : fixs_ith::BOARD_SMX_RIGHT;
	}

	if (common::utility::isSwitchBoardCMX()) {
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "CMX slot_pos %d", slot_pos);
		sb_pos =
				(slot_pos == 26) ?
						fixs_ith::BOARD_CMX_LEFT : fixs_ith::BOARD_CMX_RIGHT;
	}

	return _build_sb_key_from_magazine_and_slot(key, mag_str, sb_pos);

}

fixs_ith_sbdatamanager::sbm_constant fixs_ith_sbdatamanager::add_switchboard(
		fixs_ith_switchboardinfo & sb_info) {
	ACE_Guard<ACE_Recursive_Thread_Mutex> guard_switchboards(
			_switchboards_mutex);

	// check if switch boards info have been loaded from CS
	if (_cs_info_loaded) {
		int32_t sb_slot = sb_info.slot_position;
		uint32_t sb_magazine = sb_info.magazine;

		uint32_t my_magazine;
		int call_result =
				fixs_ith::workingSet_t::instance()->get_cs_reader().get_my_magazine(
						my_magazine);
		if (call_result != fixs_ith::ERR_NO_ERRORS) {
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
					"Unable to load switch boards info from CS: call_result == %d",
					call_result);
			return SBM_LOAD_FROM_CS_ERROR;
		}

		sb_info.magazine_id =
				(sb_info.magazine == my_magazine) ?
						fixs_ith::APZ_MAGAZINE : fixs_ith::APT_MAGAZINE;

		char mag_str[16] = { 0 };
		fixs_ith_csreader::uint32_to_ip_format(mag_str, sb_magazine, '.');

		// build the key for the switch board, combining MAGAZINE string and SLOT position
		char mag_slot_str[32] = { 0 };
		::snprintf(mag_slot_str, FIXS_ITH_ARRAY_SIZE(mag_slot_str), "%s_%d",
				mag_str, sb_slot);
		std::string sb_key_str(mag_slot_str);

		// insert the entry for the switch board in the internal map
		_switchboards.insert(SwitchboardsMap::value_type(sb_key_str, sb_info));
	}

	return SBM_OK;
}

fixs_ith_sbdatamanager::sbm_constant fixs_ith_sbdatamanager::remove_switchboard(
		const std::string & switchboard_key) {
	ACE_Guard<ACE_Recursive_Thread_Mutex> guard_switchboards(
			_switchboards_mutex);

	SwitchboardsMap::iterator it = _switchboards.find(switchboard_key);
	if (it != _switchboards.end()) {
		_switchboards.erase(it);
	}

	return SBM_OK;
}

fixs_ith_sbdatamanager::sbm_constant fixs_ith_sbdatamanager::search_for_magazine(
		fixs_ith::magazine_identifier_t mag_id) {
	ACE_Guard<ACE_Recursive_Thread_Mutex> guard_switchboards(
			_switchboards_mutex);

	// check if switch boards info have been loaded from CS
	if (!_cs_info_loaded) {
		// info not yet loaded from CS
		sbm_constant op_res = _load_data_from_cs_helper();
		if (op_res == SBM_OK)
			_cs_info_loaded = true;
		else
			return op_res;
	}

	SwitchboardsMap::const_iterator sbit;
	for (sbit = _switchboards.begin();
			(sbit != _switchboards.end())
					&& (sbit->second.magazine_id != mag_id); ++sbit)
		;

	if (sbit == _switchboards.end()) {
		// WARINING: SMX NOT FOUND
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
				"NO SWITCH BOARDS FOUND into internal map: mag_id == <%d>",
				mag_id);
		return SBM_MAGAZINE_NOT_FOUND;
	}

	return SBM_OK;
}

fixs_ith_sbdatamanager::sbm_constant fixs_ith_sbdatamanager::reset_info() {
	ACE_Guard<ACE_Recursive_Thread_Mutex> guard_switchboards(
			_switchboards_mutex);

	// check if switch boards info have been loaded from CS
	if (_cs_info_loaded) {
		_switchboards.clear();
		_cs_info_loaded = false;
	}

	return SBM_OK;
}

fixs_ith_sbdatamanager::sbm_constant fixs_ith_sbdatamanager::get_switch_board_target_ipns(
		char (&switch_board_ipna)[16], char (&switch_board_ipnb)[16],
		const fixs_ith_switchboardinfo & switch_board,
		fixs_ith::switchboard_plane_t sb_plane) {
	switch (sb_plane) {
	case fixs_ith::CONTROL_PLANE:
		::strncpy(switch_board_ipna, switch_board.control_ipna_str,
				FIXS_ITH_ARRAY_SIZE(switch_board_ipna));
		::strncpy(switch_board_ipnb, switch_board.control_ipnb_str,
				FIXS_ITH_ARRAY_SIZE(switch_board_ipnb));
		break;
	case fixs_ith::TRANSPORT_PLANE:
		::strncpy(switch_board_ipna, switch_board.transport_ipna_str,
				FIXS_ITH_ARRAY_SIZE(switch_board_ipna));
		::strncpy(switch_board_ipnb, switch_board.transport_ipnb_str,
				FIXS_ITH_ARRAY_SIZE(switch_board_ipnb));
		break;
	default:
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"Cannot get target ipns: passed switch board plane value (%d) is not correct ",
				sb_plane);
		return SBM_INVALID_PLANE;
	}

	return SBM_OK;
} //+

