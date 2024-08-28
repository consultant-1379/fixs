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
//	********************************************************

#ifndef ITHADM_CAA_INC_FIXS_ITH_SBDATAMANAGER_H_
#define ITHADM_CAA_INC_FIXS_ITH_SBDATAMANAGER_H_

#include <map>
#include <set>
#include <ace/Recursive_Thread_Mutex.h>

#include "fixs_ith_programmacros.h"
#include "fixs_ith_switchboardinfo.h"

class fixs_ith_sbdatamanager
{
public:

	enum sbm_constant
	{
		SBM_OK = 0,
		SBM_GENERIC_ERROR = -1,
		SBM_LOAD_FROM_CS_ERROR = -2,
		SBM_BOARD_NOT_FOUND = -3,
		SBM_INVALID_PLANE = -4,
		SBM_MAGAZINE_NOT_FOUND = -5
	};

	fixs_ith_sbdatamanager(): _switchboards(), _switchboards_mutex(), _cs_info_loaded(false) {};

	virtual ~fixs_ith_sbdatamanager() {};

	// Initializes the object reading data from CS
	sbm_constant  load_data_from_cs();

	// Get info about a switch board
	sbm_constant  get_switchboard_info(fixs_ith_switchboardinfo & switchboardInfo, const std::string & mag_str, fixs_ith::BoardSlotPosition pos);
	sbm_constant  get_switchboard_info(fixs_ith_switchboardinfo & switchboardInfo, const std::string & switchboard_key);
	sbm_constant  get_switchboard_info (
			fixs_ith_switchboardinfo & switchboardInfo,
			std::string & switch_board_mag_str,
			fixs_ith::magazine_identifier_t mag_id,
			fixs_ith::BoardSlotPosition pos);

	// Get info about all switch boards
	sbm_constant  get_all_switchboards_info(std::vector<fixs_ith_switchboardinfo> & switchboards_info);

    // Get IP addresses of the CONTROL or TRANSPORT plane of a Switch board
	sbm_constant  get_switchboard_plane_addresses(char (& switch_board_plane_ipn_str_addresses) [FIXS_ITH_CONFIG_IPN_COUNT][16], const std::string & mag_str, fixs_ith::BoardSlotPosition pos, fixs_ith::switchboard_plane_t plane);
	static sbm_constant  get_switch_board_target_ipns (char (& switch_board_ipna) [16], char (& switch_board_ipnb) [16], const fixs_ith_switchboardinfo & switch_board,fixs_ith::switchboard_plane_t sb_plane);
	// Get Switch board IDs
	sbm_constant get_switchboard_keys(std::set<std::string>& switchboardKeys);
	sbm_constant add_switchboard(fixs_ith_switchboardinfo & sb_info);
	sbm_constant remove_switchboard(const std::string & switchboard_key);
	sbm_constant build_sb_key_from_magazine_and_slot(std::string & key, uint32_t mag_str, int32_t slot_pos);

	sbm_constant search_for_magazine (fixs_ith::magazine_identifier_t mag_id);

	sbm_constant reset_info();

private:
	fixs_ith_sbdatamanager(const fixs_ith_sbdatamanager &);
	fixs_ith_sbdatamanager & operator=(const fixs_ith_sbdatamanager &);

	// *** Helper methods ***
	sbm_constant _build_sb_key_from_magazine_and_slot(std::string & key, const std::string & mag_str, fixs_ith::BoardSlotPosition slot_pos);

	sbm_constant _load_data_from_cs_helper();

	typedef std::map<std::string, fixs_ith_switchboardinfo> SwitchboardsMap;
	SwitchboardsMap _switchboards;							// keep info about the Switch boards in the system
	ACE_Recursive_Thread_Mutex _switchboards_mutex;		    // used to synchronize access to Switch boards map

	bool _cs_info_loaded;								// indicates if switch boards info were loaded from CS
};

#endif /* ITHADM_CAA_INC_FIXS_ITH_SBDATAMANAGER_H_ */
