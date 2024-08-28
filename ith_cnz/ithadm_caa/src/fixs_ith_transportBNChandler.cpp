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
#include <vector>
#include "common/utility.h"
#include "fixs_ith_programmacros.h"
#include "fixs_ith_snmpconstants.h"
#include "fixs_ith_transportBNChandler.h"
#include "fixs_ith_csreader.h"
#include "fixs_ith_logger.h"
#include "imm/imm.h"
#include "fixs_ith_workingset.h"
#include "engine/basicNetworkConfiguration.h"




fixs_ith::ErrorConstants OmSession::open()
{
	if(!_immSessionOpened)
	{

		if (_om_handler.Init() != ACS_CC_SUCCESS) {
			// ERROR initialilzing imm internal om handler
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Internal OM IMM handler init failure: imm last error == %d, imm last error text == '%s'",
					_om_handler.getInternalLastError(), _om_handler.getInternalLastErrorText());
			return fixs_ith::ERR_OM_HANDLER_INIT_FAILURE;
		}
		_immSessionOpened = true;
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG,"Init...om_handler...ok!");
	}
	return fixs_ith::ERR_NO_ERRORS;
}


fixs_ith::ErrorConstants OmSession::close()
{
	if(_immSessionOpened){
		if (_om_handler.Finalize() != ACS_CC_SUCCESS) {
			// ERROR finalizing imm internal om handler
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Internal OM IMM handler finilize failure: imm last error == %d, imm last error text == '%s'",
					_om_handler.getInternalLastError(), _om_handler.getInternalLastErrorText());
			return fixs_ith::ERR_OM_HANDLER_FINALIZE_FAILURE;
		}
		_immSessionOpened = false;
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG,"Finalize...om_handler...ok!");
	}

	return fixs_ith::ERR_NO_ERRORS;
}



fixs_ith::ErrorConstants fixs_ith_transportBNChandler::create_bnc_objects(const std::string& switchboard_key)
{
	fixs_ith::ErrorConstants  call_result = fixs_ith::ERR_NO_ERRORS;

	int failure_counter = 0;
	std::vector<fixs_ith_switchboardinfo>  switch_boards_list;

	if (switchboard_key.empty())
	{
		//No switch board key specified. Load BNC configuration for all switchboards
		fixs_ith::workingSet_t::instance()->get_sbdatamanager().get_all_switchboards_info(switch_boards_list);

		if(switch_boards_list.size()<= 0) { // ERROR: No switch board found in the CS configuration
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "No switch board found in CS configuration");
			return fixs_ith::ERR_CS_NO_BOARD_FOUND;
		}
	}
	else
	{
		fixs_ith_switchboardinfo switch_board;
		if (fixs_ith::workingSet_t::instance()->get_sbdatamanager().get_switchboard_info(switch_board, switchboard_key) != fixs_ith_sbdatamanager::SBM_OK  )
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Switch board with key <%s> found in CS configuration", switchboard_key.c_str());
			return fixs_ith::ERR_CS_NO_BOARD_FOUND;
		}

		switch_boards_list.push_back(switch_board);
	}

	OmSession session;

	// Initialize OmHandler
	if(( call_result = session.open()) != fixs_ith::ERR_NO_ERRORS)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,"create_bnc_objects failed ! Cannot open imm session  call_result == %d", call_result);
		return call_result;
	}

	// create Transport root object if it doesn't exist
	if(!immObject_exist(imm::mom_dn::ROOT_MOC_DN, session) &&
			((call_result = create_transportRootMOCobj(session)) != fixs_ith::ERR_NO_ERRORS))
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,"create_bnc_objects failed ! Cannot create_rootMO object  call_result == %d", call_result);
		session.close();
		return call_result;
	}

	// Now create all BNC transport objects for each switch board in configuration
	for (size_t i = 0; i < switch_boards_list.size(); ++i)
	{
		if((call_result = create_bnc_objects(switch_boards_list[i], session))){
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
					"Call 'create_bnc_objects' failed: create BNC objects failed for switch board in magazine == 0x%08X slot %d: call_result == %d",
					switch_boards_list[i].magazine, switch_boards_list[i].slot_position, call_result);
			// continue for the other switch boards or return immediately with error?
			failure_counter ++;
			
		}
		else
		{
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG,"BNC : creation of bnc objects for slot %d is succesful and call_result == %d", switch_boards_list[i].slot_position, call_result);
		}

	}

	session.close();

	//TODO : specificare su quali switch board è fallita l'operazione.
	if (failure_counter){
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,"Create Transport BNC MO objects failed !");
		return fixs_ith::ERR_CREATE_BNC_OBJECTS;
	}

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG,"Create Transport BNC MO objects successful executed!");
	return fixs_ith::ERR_NO_ERRORS;
}

fixs_ith::ErrorConstants fixs_ith_transportBNChandler::create_bnc_objects(fixs_ith_switchboardinfo & switch_board, OmSession &session)
{
  //  TODO: syncrhonize  the access to this method

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG,"BNC :create_bnc_objects(switchboards");

	fixs_ith::ErrorConstants call_result = fixs_ith::ERR_NO_ERRORS;
	bool failure_on_control = false, failure_on_transport = false;

	//GEP2_GEP5 to GEP7 HWSWAP
	bool isSMX = common::utility::isSwitchBoardSMX();
	bool isCMX = common::utility::isSwitchBoardCMX();

	FIXS_ITH_LOG(LOG_LEVEL_INFO, "Adding BNC objects on CONTROL plane of switch board in magazine == 0x%08X slot %d", switch_board.magazine, switch_board.slot_position);
       
	if (isSMX) {
		// If( BNC_objects(switch_board) == ALREADY EXSIST return fixs_ith::ERR_NO_ERRORS;
		if ((call_result = create_bnc_objects(switch_board,
				fixs_ith::CONTROL_PLANE, session))) {
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
					"Call 'create_bridgeMO' failed: create BNC objects failed on CONTROL plane of switch board in magazine == 0x%08X slot %d: call_result == %d",
					switch_board.magazine, switch_board.slot_position,
					call_result);
			failure_on_control = true;
		}
	}
	FIXS_ITH_LOG(LOG_LEVEL_INFO,
			"Adding BNC objects on TRANSPORT plane of switch board in magazine == 0x%08X slot %d",
			switch_board.magazine, switch_board.slot_position);
	if ((call_result = create_bnc_objects(switch_board,
			fixs_ith::TRANSPORT_PLANE, session))) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"Call 'create_bridgeMO' failed: create BridgeMO failed on TRANSPORT plane of switch board in magazine == 0x%08X slot %d: call_result == %d",
				switch_board.magazine, switch_board.slot_position, call_result);
		failure_on_transport = true;
	}
	if (isSMX) {
		if (!failure_on_control && !failure_on_transport)
			return fixs_ith::ERR_NO_ERRORS;
		if (failure_on_control)
			return fixs_ith::ERR_CREATE_CONTROL_BNC_OBJECTS;
		if (failure_on_transport)
			return fixs_ith::ERR_CREATE_TRANSPORT_BNC_OBJECTS;
	}
	if (isCMX) {
		if (!failure_on_transport)
			return fixs_ith::ERR_NO_ERRORS;
		if (failure_on_transport)
			return fixs_ith::ERR_CREATE_TRANSPORT_BNC_OBJECTS;
	}
	
	return fixs_ith::ERR_CREATE_BNC_OBJECTS;
}

fixs_ith::ErrorConstants fixs_ith_transportBNChandler::create_bnc_objects(fixs_ith_switchboardinfo & switch_board, fixs_ith::switchboard_plane_t plane, OmSession &session)
{
	fixs_ith::ErrorConstants call_result = fixs_ith::ERR_NO_ERRORS;

	if ((call_result= create_bridgeMO (switch_board, plane, session)) && call_result != fixs_ith::ERR_IMM_OBJECT_ALREADY_EXIST){
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"Call 'create_bridgeMO' failed: create BridgeMO failed on switch board in magazine == 0x%08X slot %d: call_result == %d",
				switch_board.magazine, switch_board.slot_position, call_result);
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG,"BNC : create_bridgeMO call result:%d", call_result);
		return call_result;
	}
	FIXS_ITH_LOG(LOG_LEVEL_INFO, "Bridge MO on switch board in magazine 0x%08X, slot %d, plane %s has been %s created",
			 switch_board.magazine, switch_board.slot_position, common::utility::planeToString(plane).c_str(),
			 (call_result == fixs_ith::ERR_IMM_OBJECT_ALREADY_EXIST) ? "already":"successful");

	// Create  ethernet ports under bridge MO
	FIXS_ITH_LOG(LOG_LEVEL_INFO, "Creating all ethernetPort MOs related to Bridge interfaces... ");
	if ((call_result = create_ethernetPortMO (switch_board, plane, session))){
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"Call 'create_ethernetPortMO' failed: create ethernetPortMO failed on switch board in magazine == 0x%08X slot %d: call_result == %d",
				switch_board.magazine, switch_board.slot_position, call_result);
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG,"BNC : create_ethernetPortMO call result:%d", call_result);
		// Continue or return with error???
		return call_result;
	}
	FIXS_ITH_LOG(LOG_LEVEL_INFO, "ethernetPort MOs on switch board in magazine == 0x%08X, slot %d, plane %s have been successful created!",
			switch_board.magazine, switch_board.slot_position, common::utility::planeToString(plane).c_str());

	FIXS_ITH_LOG(LOG_LEVEL_INFO, "Creating all BridgetPort MO under the Bridge ... ");
	if ((call_result= create_bridgePortMO (switch_board, plane, session))){
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"Call 'create_bridgePortMO' failed: create BridgePort MOs failed on switch board in magazine == 0x%08X slot %d: call_result == %d",
				switch_board.magazine, switch_board.slot_position, call_result);
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG,"BNC : create_bridgePortMO call result:%d", call_result);

		return call_result;
	}

	//Create aggregator for SMX
	if (common::utility::isSwitchBoardSMX()) {
		if ((engine::defaultAggregator::PLANE & plane)
				&& (engine::defaultAggregator::MAGAZINE_ID
						& switch_board.magazine_id)) {
			FIXS_ITH_LOG(LOG_LEVEL_INFO, "Creating Aggregator MO for SMX");
			if ((call_result = create_aggregatorMO(switch_board, plane, session))) {
				FIXS_ITH_LOG(LOG_LEVEL_ERROR,
						"Call 'create_bridgePortMO' failed: create AggregatorMO failed for switch board in magazine == 0x%08X slot %d: call_result == %d",
						switch_board.magazine, switch_board.slot_position,
						call_result);
				return call_result;
			}
		}
	} else if (common::utility::isSwitchBoardCMX()) {
		if ((engine::defaultAggregatorCMX::PLANE & plane)
				&& (engine::defaultAggregatorCMX::MAGAZINE_ID
						& switch_board.magazine_id)) {
			FIXS_ITH_LOG(LOG_LEVEL_INFO, "Creating Aggregator MO for CMX... ");
			if ((call_result = create_aggregatorMO(switch_board, plane, session))) {
				FIXS_ITH_LOG(LOG_LEVEL_ERROR,
						"Call 'create_bridgePortMO' failed: create AggregatorMO failed for switch board in magazine == 0x%08X slot %d: call_result == %d",
						switch_board.magazine, switch_board.slot_position,
						call_result);
				FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
						"BNC : create_aggregatorMO call result:%d",
						call_result);

				return call_result;
			}
		}
	}

	// Delete APZ-UPD VLAN first irrespective of GEP5 or GEP7
	// TR HY97127
	if (plane == fixs_ith::TRANSPORT_PLANE) {
		char dnValue[IMM_DN_PATH_SIZE_MAX] = { 0 };
		common::utility::build_vlan_dn(dnValue, "APZ-UPD",
				switch_board.slot_position, switch_board.magazine, plane);
		FIXS_ITH_LOG(LOG_LEVEL_WARN, "deleting APZ-UPD VLAN MO ... %s",
				dnValue);

		if (immObject_exist(dnValue, session)) {
			if ((call_result = delete_objects(dnValue, session))) {
				FIXS_ITH_LOG(LOG_LEVEL_ERROR,
						"Call 'create_vlanMO' failed: delete APZ-UPD VLAN BNC objects failed of switch board == 0x%08X slot %d: call_result == %d",
						switch_board.magazine, switch_board.slot_position,
						call_result);

				call_result = fixs_ith::ERR_DELETE_BNC_OBJECTS;
				FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
						"BNC : delete_objects:APZ-UPD call result:%d",
						call_result);
				return call_result;
			}
			FIXS_ITH_LOG(LOG_LEVEL_WARN, "VLAN MO APZ-UPD deleted ... %s",
					dnValue);
		} else {
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "APZ-UPD VLAN MO Does Not exist");
		}
	}
	//GEP2_GEP5 to GEP7 Dynamic UPD VLAN feature.
	//Create BNC VLANs
	std::vector<operation::vlanInfo> bncVlans = get_bnc_vlans(switch_board,
			plane, _bncVlans);
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "size of bncVlans vector:%d",
			bncVlans.size());
	for (std::vector<operation::vlanInfo>::iterator it = bncVlans.begin();
			it != bncVlans.end(); ++it) {
		FIXS_ITH_LOG(LOG_LEVEL_INFO, "Creating VLAN MO");
		if ((call_result = create_vlanMO(switch_board, plane, (*it), session))) {
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
					"Call 'create_vlanMO' failed: create VlanMO %s failed for switch board in magazine == 0x%08X slot %d: call_result == %d",
					it->getName(), switch_board.magazine,
					switch_board.slot_position, call_result);
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "BNC : create_vlanMO call result:%d",
					call_result);

			return call_result;
		}
	}
	
	// // TR HY37073 BEGIN
	// Delete APZ-UPD VLAN in case node is GEP5 SMX
        //GEP2_GEP5 to GEP7 Dynamic UPD VLAN feature.
	if ((common::utility::is_hwtype_gep5())
			&& (plane == fixs_ith::TRANSPORT_PLANE)) {
		char dnValue[IMM_DN_PATH_SIZE_MAX] = { 0 };
		if (switch_board.slot_position == 0) {
			common::utility::build_vlan_dn(dnValue,
					engine::updVlan::transport::left::NAME.c_str(),
					switch_board.slot_position, switch_board.magazine, plane);
			FIXS_ITH_LOG(LOG_LEVEL_WARN, "deleting VLAN MO ... %s", dnValue);

			if ((call_result = delete_objects(dnValue, session))) {
				FIXS_ITH_LOG(LOG_LEVEL_ERROR,
						"Call 'create_vlanMO' failed: delete UPD VLAN BNC objects failed of switch board == 0x%08X slot %d: call_result == %d",
						switch_board.magazine, switch_board.slot_position,
						call_result);
				call_result = fixs_ith::ERR_DELETE_BNC_OBJECTS;
				return call_result;
			}
			FIXS_ITH_LOG(LOG_LEVEL_WARN, "VLAN MO deleted ... %s", dnValue);
		}

		if (switch_board.slot_position == 25) {
			common::utility::build_vlan_dn(dnValue,
					engine::updVlan::transport::right::NAME.c_str(),
					switch_board.slot_position, switch_board.magazine, plane);
			FIXS_ITH_LOG(LOG_LEVEL_WARN, "deleting VLAN MO ... %s", dnValue);

			if ((call_result = delete_objects(dnValue, session))) {
				FIXS_ITH_LOG(LOG_LEVEL_ERROR,
						"Call 'create_vlanMO' failed: delete UPD VLAN BNC objects failed of switch board == 0x%08X slot %d: call_result == %d",
						switch_board.magazine, switch_board.slot_position,
						call_result);
				call_result = fixs_ith::ERR_DELETE_BNC_OBJECTS;
				return call_result;
			}
			FIXS_ITH_LOG(LOG_LEVEL_WARN, "VLAN MO deleted ... %s", dnValue);
		}

		if (switch_board.slot_position == 26) {
			common::utility::build_vlan_dn(dnValue,
					engine::updVlan::transport::cmxleft::NAME.c_str(),
					switch_board.slot_position, switch_board.magazine, plane);
			FIXS_ITH_LOG(LOG_LEVEL_WARN, "deleting VLAN MO ... %s", dnValue);

			if ((call_result = delete_objects(dnValue, session))) {
				FIXS_ITH_LOG(LOG_LEVEL_ERROR,
						"Call 'create_vlanMO' failed: delete UPD VLAN BNC objects failed of switch board == 0x%08X slot %d: call_result == %d",
						switch_board.magazine, switch_board.slot_position,
						call_result);
				call_result = fixs_ith::ERR_DELETE_BNC_OBJECTS;
				FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
						"BNC : delete_objects:UPD-A call result:%d",
						call_result);
				return call_result;
			}
			FIXS_ITH_LOG(LOG_LEVEL_WARN, "VLAN MO deleted ... %s", dnValue);
		}

		if (switch_board.slot_position == 28) {
			common::utility::build_vlan_dn(dnValue,
					engine::updVlan::transport::cmxright::NAME.c_str(),
					switch_board.slot_position, switch_board.magazine, plane);
			FIXS_ITH_LOG(LOG_LEVEL_WARN, "deleting VLAN MO ... %s", dnValue);

			if ((call_result = delete_objects(dnValue, session))) {
				FIXS_ITH_LOG(LOG_LEVEL_ERROR,
						"Call 'create_vlanMO' failed: delete UPD VLAN BNC objects failed of switch board == 0x%08X slot %d: call_result == %d",
						switch_board.magazine, switch_board.slot_position,
						call_result);
				call_result = fixs_ith::ERR_DELETE_BNC_OBJECTS;
				FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
						"BNC : delete_objects:UPD-B call result:%d",
						call_result);
				return call_result;
			}
			FIXS_ITH_LOG(LOG_LEVEL_WARN, "VLAN MO deleted ... %s", dnValue);
		}

	}

	FIXS_ITH_LOG(LOG_LEVEL_INFO,
			"BNC MOs on switch board in magazine 0x%08X slot %d, plane %s have been successful created!",
			switch_board.magazine, switch_board.slot_position,
			common::utility::planeToString(plane).c_str());
	return call_result;
}

// create objects on a switch board
fixs_ith::ErrorConstants fixs_ith_transportBNChandler::create_ethernetPortMO(
		fixs_ith_switchboardinfo & switch_board, OmSession &session) {
	fixs_ith::ErrorConstants call_result;
	if (common::utility::isSwitchBoardSMX()) {
		call_result = create_ethernetPortMO(switch_board,
				fixs_ith::CONTROL_PLANE, session);
		if (call_result) {
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
					"Call 'create_ethernetPortMO' failed: create ethernetPort MO failed for CONTROL PLANE of switch board in magazine == 0x%08X slot %d : call_result == %d",
					switch_board.magazine, switch_board.slot_position,
					call_result);
		}
	}

	call_result = create_ethernetPortMO(switch_board, fixs_ith::TRANSPORT_PLANE,
			session);

	if (call_result) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"Call 'create_ethernetPortMO' failed: create ethernetPort MO failed for TRANSPORT PLANE of switch board in magazine == 0x%08X slot %d : call_result == %d",
				switch_board.magazine, switch_board.slot_position, call_result);
	}

	return call_result;

}

fixs_ith::ErrorConstants fixs_ith_transportBNChandler::create_bridgeMO(
		fixs_ith_switchboardinfo & switch_board, OmSession &session) {
	fixs_ith::ErrorConstants return_code = fixs_ith::ERR_NO_ERRORS;
	fixs_ith::ErrorConstants call_result;
	if (common::utility::isSwitchBoardSMX()) {

		call_result = create_bridgeMO(switch_board, fixs_ith::CONTROL_PLANE,
				session);
		if (call_result
				&& call_result != fixs_ith::ERR_IMM_OBJECT_ALREADY_EXIST) {
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
					"Call 'create_bridgeMO' failed: create bridge MO failed for CONTROL PLANE of switch board in magazine == 0x%08X slot %d : call_result == %d",
					switch_board.magazine, switch_board.slot_position,
					call_result);
			return_code = call_result;
		}
	}

	call_result = create_bridgeMO(switch_board, fixs_ith::TRANSPORT_PLANE,
			session);

	if (call_result && call_result != fixs_ith::ERR_IMM_OBJECT_ALREADY_EXIST) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"Call 'create_bridgeMO' failed: create bridge MO failed for TRANSPORT PLANE of switch board in magazine == 0x%08X slot %d : call_result == %d",
				switch_board.magazine, switch_board.slot_position, call_result);
		return_code = call_result;
	}

	return return_code;
}

fixs_ith::ErrorConstants fixs_ith_transportBNChandler::create_bridgePortMO(
		fixs_ith_switchboardinfo & switch_board, OmSession &session) {
	fixs_ith::ErrorConstants call_result;
	if (common::utility::isSwitchBoardSMX()) {
		call_result = create_bridgePortMO(switch_board, fixs_ith::CONTROL_PLANE,
				session);
		if (call_result) {
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
					"Call 'create_bridgePortMO' failed: create bridgePort MO failed for CONTROL PLANE of switch board in magazine == 0x%08X slot %d : call_result == %d",
					switch_board.magazine, switch_board.slot_position,
					call_result);
		}
	}

	call_result = create_bridgePortMO(switch_board, fixs_ith::TRANSPORT_PLANE,
			session);

	if (call_result) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"Call 'create_bridgePortMO' failed: create bridgePort MO failed for TRANSPORT PLANE of switch board in magazine == 0x%08X slot %d : call_result == %d",
				switch_board.magazine, switch_board.slot_position, call_result);
	}
	return call_result;
}

// create  objects on plane
fixs_ith::ErrorConstants fixs_ith_transportBNChandler::create_ethernetPortMO(
		fixs_ith_switchboardinfo & switch_board,
		fixs_ith::switchboard_plane_t plane, OmSession & session) {
	// get ethernet port num configured on switch_board;
	fixs_ith_snmpmanager & snmpmanager =
			fixs_ith::workingSet_t::instance()->get_snmpmanager();

	int32_t ifs_num = 0;
	uint32_t timeout_ms = DEFAULT_SNMP_TIMEOUT_MS;

	fixs_ith::ErrorConstants call_result = fixs_ith::ERR_NO_ERRORS, op_result =
			fixs_ith::ERR_NO_ERRORS;

	if (const int result = snmpmanager.get_network_interfaces_number(ifs_num,
			switch_board, plane, &timeout_ms)) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"Call 'get_ethernetPortData' failed: cannot retrieve Ethernet Port data from  switch board in magazine == 0x%08X slot %d plane == %d: call_result == %d",
				switch_board.magazine, switch_board.slot_position, plane,
				result);
		return fixs_ith::ERR_GET_INTERFACE_INFO_FROM_SB;
	}
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
			"%d interfaces are configured on the switch board in magazine == 0x%08X slot %d plane == %s",
			ifs_num, switch_board.magazine, switch_board.slot_position,
			common::utility::planeToString(plane).c_str());

	//bool switch_board_cmx = common::utility::isSwitchBoardCMX();

	for (int32_t ethport_count = 0, if_index = 1;
			is_valid_ifIndex(if_index, plane) && ethport_count < ifs_num;
			++if_index) {
		operation::ethernetPortInfo ethportData;
		if (const int result = get_ethernetPortData(ethportData, if_index,
				switch_board, plane)) {
			if (result == fixs_ith::ERR_UNDEFINED_INTERFACE) {
				FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
						"Call 'get_ethernetPortData' failed: no interface defined in IF_TABLE at entry %d on switch board in magazine == 0x%08X slot %d plane == %d: call_result == %d",
						if_index, switch_board.magazine,
						switch_board.slot_position, plane, result);
				continue;
			} else {
				FIXS_ITH_LOG(LOG_LEVEL_ERROR,
						"Call 'get_ethernetPortData' failed: cannot retrieve Ethernet Port data from  switch board in magazine == 0x%08X slot %d plane == %d: call_result == %d",
						switch_board.magazine, switch_board.slot_position,
						plane, result);
				// return with error or create the object with default value?
				call_result = fixs_ith::ERR_GET_INTERFACE_INFO_FROM_SB;
				break;
			}
		}
		ethport_count++;
		// build DN and check if it already exists!
		char ethport_dn[IMM_DN_PATH_SIZE_MAX] = { 0 };
		if (common::utility::build_ethport_dn(ethport_dn,
				ethportData.name.c_str(), switch_board.slot_position,
				switch_board.magazine, plane))
			return fixs_ith::ERR_BUILD_OBJECT_DN;

		if (!immObject_exist(ethport_dn, session)
				&& (op_result = create_ethernetPort_obj(ethportData,
						switch_board.magazine, switch_board.slot_position,
						plane, session))) {
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
					"Call 'get_ethernetPortData' failed: cannot create Ethernet Port obj for switch board in magazine == 0x%08X slot %d plane == %d: call_result == %d",
					switch_board.magazine, switch_board.slot_position, plane,
					call_result);
			// return with error or continue?
			call_result = op_result;
			break;
		}
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
				"Created ethernetPortMO having name == %s, macAddress == %02x, snmpIndex == %u",
				ethportData.name.c_str(), ethportData.macAddress.c_str(),
				ethportData.snmpIndex);

	}

	return call_result;
}

fixs_ith::ErrorConstants fixs_ith_transportBNChandler::create_bridgeMO(
		fixs_ith_switchboardinfo & switch_board,
		fixs_ith::switchboard_plane_t plane, OmSession & session) {
	char bridgeRdn[IMM_RDN_PATH_SIZE_MAX] = { 0 };

	if (common::utility::build_bridge_rdn(bridgeRdn, switch_board.slot_position,
			switch_board.magazine, plane) != 0) {
		return fixs_ith::ERR_BUILD_OBJECT_DN;
	}

	char bridgeDN[IMM_DN_PATH_SIZE_MAX] = { 0 };
	::snprintf(bridgeDN, IMM_DN_PATH_SIZE_MAX, "%s,%s", bridgeRdn,
			imm::mom_dn::ROOT_MOC_DN);

	if (immObject_exist(bridgeDN, session))
		return fixs_ith::ERR_IMM_OBJECT_ALREADY_EXIST;

	operation::bridgeInfo bridgeData;
	fixs_ith::ErrorConstants call_result = get_bridgeData(bridgeData,
			switch_board, plane);
	if (call_result) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"Call 'get_bridgeData' failed: cannot retrieve Bridge data from switch board in magazine == 0x%08X slot %d plane == %d: call_result == %d",
				switch_board.magazine, switch_board.slot_position, plane,
				call_result);
		return call_result;
	}
	bridgeData.moDN.assign(bridgeDN);
	return create_bridge_obj(bridgeData, switch_board.magazine,
			switch_board.slot_position, plane, session);
}

fixs_ith::ErrorConstants fixs_ith_transportBNChandler::create_bridgePortMO(
		fixs_ith_switchboardinfo & switch_board,
		fixs_ith::switchboard_plane_t plane, OmSession & session) {
	// get bridge port num configured on switch_board;
	fixs_ith_snmpmanager & snmpmanager =
			fixs_ith::workingSet_t::instance()->get_snmpmanager();
	int32_t num_of_bridgeports;
	uint32_t timeout_ms = DEFAULT_SNMP_TIMEOUT_MS;
	fixs_ith::ErrorConstants call_result = fixs_ith::ERR_NO_ERRORS, op_result =
			fixs_ith::ERR_NO_ERRORS;

	if (const int ret_code = snmpmanager.get_number_of_ports(num_of_bridgeports,
			switch_board, plane, &timeout_ms)) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"Call 'get_bridgeports_number' failed: cannot retrieve Bridge Ports number from  switch board in magazine == 0x%08X slot %d plane == %d: call_result == %d",
				switch_board.magazine, switch_board.slot_position, plane,
				ret_code);
		return fixs_ith::ERR_GET_BRIDGEPORT_INFO_FROM_SB;
	}

	for (uint16_t portnumber = 1, port_count = 0;
			port_count < num_of_bridgeports; portnumber++) {
		uint16_t if_index = 0;
		if (const int ret_code = snmpmanager.get_bridgeport_if_index(if_index,
				portnumber, switch_board, plane, &timeout_ms)) {
			// check if 'portnumber' is a valid entry in dot1dBasePortTable
			if (ret_code
					== fixs_ith_snmp::ERR_SNMP_PDU_VARIABLE_NO_SUCH_INSTANCE
					|| ret_code
							== fixs_ith_snmp::ERR_SNMP_PDU_VARIABLE_NULL_SIZE) {
				FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
						"Call 'get_bridgeport_if_index' returns with NO SUCH INSTANCE error: no bridge port information in bridge port table at entry == %d  for switch board in magazine == 0x%08X slot %d plane == %d: ret_code == %d",
						portnumber, switch_board.magazine,
						switch_board.slot_position, plane, ret_code);
				// empty entry in dot1dBasePortTable
				continue;
			} else {
				FIXS_ITH_LOG(LOG_LEVEL_ERROR,
						"Call 'get_bridgeport_if_index' failed: cannot get interface index of bridge port %d on switch board in magazine == 0x%08X slot %d plane == %d: ret_code == %d",
						portnumber, switch_board.magazine,
						switch_board.slot_position, plane, ret_code);
				call_result = fixs_ith::ERR_GET_BRIDGEPORT_INFO_FROM_SB;
				break;
			}
		}
		port_count++;
		if (!is_valid_ifIndex(if_index, plane))
			continue;

		operation::bridgePortInfo bridgePortData;
		if ((op_result = get_bridgePortData(bridgePortData, if_index,
				switch_board, plane))) {
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
					"Call 'get_bridgeData' failed: cannot retrieve bridgePort data from  switch board in magazine == 0x%08X slot %d plane == %d: call_result == %d",
					switch_board.magazine, switch_board.slot_position, plane,
					call_result);
			call_result = op_result;
			break;
		}
		// build DN and check if it already exists!
		char bridgePort_dn[IMM_DN_PATH_SIZE_MAX] = { 0 };
		if (common::utility::build_bridgePort_dn(bridgePort_dn,
				bridgePortData.name.c_str(), switch_board.slot_position,
				switch_board.magazine, plane))
			return fixs_ith::ERR_BUILD_OBJECT_DN;

		if (!immObject_exist(bridgePort_dn, session)
				&& (op_result = create_bridgePort_obj(bridgePortData,
						switch_board.magazine, switch_board.slot_position,
						plane, session))) {
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
					"Call 'create_bridgePort_obj' failed: cannot create BridgePort obj on switch board in magazine == 0x%08X slot %d plane == %d: call_result == %d",
					switch_board.magazine, switch_board.slot_position, plane,
					call_result);
			// return with error or continue?
			call_result = op_result;
			break;

		}
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
				"Created bridgePortMO having name == %s, portAddress == %02x, defaultVlanId == %u, adminstate == %d",
				bridgePortData.name.c_str(), bridgePortData.portAddress.c_str(),
				bridgePortData.defaultVlanId, bridgePortData.adminstate);
	}
	return call_result;
}
fixs_ith::ErrorConstants fixs_ith_transportBNChandler::create_aggregatorMO(
		fixs_ith_switchboardinfo & switch_board,
		fixs_ith::switchboard_plane_t plane, OmSession & session) {
	fixs_ith::ErrorConstants call_result = fixs_ith::ERR_NO_ERRORS;

	operation::aggregatorInfo aggregatorData;
	if (fixs_ith::ERR_NO_ERRORS
			!= (call_result = get_aggregatorData(aggregatorData, switch_board,
					plane))) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"Call 'get_aggregatorData' failed: cannot retrieve Aggregator data from  switch board in magazine == 0x%08X slot %d plane == %d: call_result == %d",
				switch_board.magazine, switch_board.slot_position, plane,
				call_result);
	} else {
		//Create the aggregator obj...
		if (const fixs_ith::ErrorConstants result = create_aggregator_obj(
				aggregatorData, switch_board.magazine,
				switch_board.slot_position, plane, session)) {
			if (result != fixs_ith::ERR_IMM_OBJECT_ALREADY_EXIST) {
				FIXS_ITH_LOG(LOG_LEVEL_ERROR,
						"Call 'create_bridgePort_obj' failed: cannot create Aggregator obj for switch board in magazine == 0x%08X slot %d plane == %d: call_result == %d",
						switch_board.magazine, switch_board.slot_position,
						plane, call_result);
				// return with error or continue?
				call_result = result;
			}
		}

		//...And the corresponding BridgePort
		if (fixs_ith::ERR_NO_ERRORS == call_result) {
			operation::bridgePortInfo bridgePortData;
			if (const fixs_ith::ErrorConstants result = get_bridgePortData(
					bridgePortData, aggregatorData.getSnmpIndex(), switch_board,
					plane)) {
				FIXS_ITH_LOG(LOG_LEVEL_ERROR,
						"Call 'get_bridgeData' failed: cannot retrieve Bridge Port data from  switch board in magazine == 0x%08X slot %d plane == %d: call_result == %d",
						switch_board.magazine, switch_board.slot_position,
						plane, call_result);
				call_result = result;
			} else {
				//Overwrite Name
				bridgePortData.name.assign(aggregatorData.getName());

				if (const fixs_ith::ErrorConstants result =
						create_bridgePort_obj(bridgePortData,
								switch_board.magazine,
								switch_board.slot_position, plane, session)) {
					if (result != fixs_ith::ERR_IMM_OBJECT_ALREADY_EXIST) {
						FIXS_ITH_LOG(LOG_LEVEL_ERROR,
								"Call 'create_bridgePort_obj' failed: cannot create Bridge Port obj for switch board in magazine == 0x%08X slot %d plane == %d: call_result == %d",
								switch_board.magazine,
								switch_board.slot_position, plane, call_result);
						// return with error or continue?
						call_result = result;
					}
				}
			}
		}
	}

	return call_result;

}

fixs_ith::ErrorConstants fixs_ith_transportBNChandler::create_vlanMO (fixs_ith_switchboardinfo & switch_board, fixs_ith::switchboard_plane_t plane, operation::vlanInfo vlanData, OmSession & session)
{
	fixs_ith::ErrorConstants call_result = fixs_ith::ERR_NO_ERRORS;

	if(const fixs_ith::ErrorConstants result = create_vlan_obj(vlanData, switch_board.magazine, switch_board.slot_position, plane, session))
	{
		if(result != fixs_ith::ERR_IMM_OBJECT_ALREADY_EXIST)
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
					"Call 'create_bridgePort_obj' failed: cannot create Aggregator obj for switch board in magazine == 0x%08X slot %d plane == %d: call_result == %d",
					switch_board.magazine, switch_board.slot_position, plane, call_result);
			// return with error or continue?
			call_result = result;
		}
	}

	return call_result;
}


// get objects data  from switch board
fixs_ith::ErrorConstants fixs_ith_transportBNChandler::get_ethernetPortData (operation::ethernetPortInfo & Data, uint16_t  if_index,
			fixs_ith_switchboardinfo & switch_board, fixs_ith::switchboard_plane_t plane)
{
	fixs_ith_snmpmanager & snmpmanager = fixs_ith::workingSet_t::instance()->get_snmpmanager();

	uint32_t timeout_ms = DEFAULT_SNMP_TIMEOUT_MS;

	//get port name and check if 'if_index' is a valid entry in IF_TABLE
	char port_name[fixs_ith_snmp::IF_NAME_MAX_LEN] = {0};
	if (const int ret_code = snmpmanager.get_port_name(port_name, if_index, switch_board, plane, &timeout_ms))
	{
		if(ret_code == fixs_ith_snmp::ERR_SNMP_PDU_VARIABLE_NO_SUCH_INSTANCE ||
				ret_code == fixs_ith_snmp::ERR_SNMP_PDU_VARIABLE_NULL_SIZE)
		{
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
					"Call 'get_port_name' returns with NO SUCH INSTANCE error: no interface information in iftable at entry == %d  for switch board in magazine == 0x%08X slot %d plane == %d: call_result == %d",
					if_index, switch_board.magazine, switch_board.slot_position, plane, ret_code);
			return fixs_ith::ERR_UNDEFINED_INTERFACE;
		}
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"Call 'get_port_name' failed: cannot get port name of if_index %d  on switch board in magazine == 0x%08X slot %d plane == %d: call_result == %d",
				if_index, switch_board.magazine, switch_board.slot_position, plane, ret_code);
		return fixs_ith::ERR_GET_INTERFACE_INFO_FROM_SB;
	}

	uint32_t max_frame_size = 0;
	if (snmpmanager.get_eth_max_frame_size(max_frame_size, if_index, switch_board, plane, &timeout_ms))
	{
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG,"Failed to fetch eth max frame size for  port:%d,value:%d",if_index,max_frame_size);
		return fixs_ith::ERR_GET_INTERFACE_INFO_FROM_SB;
	}else { FIXS_ITH_LOG(LOG_LEVEL_DEBUG,"Successfully fetched eth max frame size for  port:%d,value:%d",if_index,max_frame_size); }

	fixs_ith_snmp::mac_address_t port_macaddr;
	if (snmpmanager.get_port_physical_address(port_macaddr, if_index, switch_board, plane, &timeout_ms))
	{
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG,"Failed to fetch physical address for port:%d",if_index);
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG,"Successfully fetched physical address for port:%d",if_index);
		return fixs_ith::ERR_GET_INTERFACE_INFO_FROM_SB;
	}
	 else { FIXS_ITH_LOG(LOG_LEVEL_DEBUG,"Successfully fetched physical address for port:%d",if_index); }

	int autoNegAdminStatus = 0;
	if (snmpmanager.get_if_mau_auto_neg_admin_status(autoNegAdminStatus, if_index, switch_board, plane, &timeout_ms))
	{
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG,"Failed to fetch  autoNegAdminStatus for port:%d,value:%d",if_index,autoNegAdminStatus);
		return fixs_ith::ERR_GET_INTERFACE_INFO_FROM_SB;
	}
	else { FIXS_ITH_LOG(LOG_LEVEL_DEBUG,"Successfully fetched autoNegAdminStatus for port:%d,value:%d",if_index,autoNegAdminStatus); }

	char  imm_bridgeAddr[IMM_MACADDRESS_SIZE_MAX]= {0};
	macAddrToImmformat(imm_bridgeAddr, port_macaddr);
	Data.name.assign(port_name);
	Data.macAddress.assign(imm_bridgeAddr);
	Data.maxFrameSize = max_frame_size;
	Data.snmpIndex = if_index;

	if(Data.isSFPport())
	{
		std::vector<bool> if_mau_type;
		if (snmpmanager.get_if_mau_type(if_mau_type, if_index, switch_board, plane, &timeout_ms))
		{
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG,"Failed to fetch if_mau_type for port:%d", if_index);
			return fixs_ith::ERR_GET_INTERFACE_INFO_FROM_SB;
		}else {  FIXS_ITH_LOG(LOG_LEVEL_DEBUG,"Successfully fetched if_mau_type for port:%d", if_index); }

		Data.autoNegotiate = (((autoNegAdminStatus - 1) != if_mau_type[fixs_ith_snmp::IF_MAU_TYPE_1000_BASE_TFD]) ? imm::AN_LOCKED : imm::AN_UNLOCKED);
	}
	else
	{
		Data.autoNegotiate = ((autoNegAdminStatus - 1) ? imm::AN_LOCKED : imm::AN_UNLOCKED);
	}
 	FIXS_ITH_LOG(LOG_LEVEL_DEBUG," Leaving get_ethernet_port_data()");
	return fixs_ith::ERR_NO_ERRORS;
}

fixs_ith::ErrorConstants fixs_ith_transportBNChandler::get_bridgeData (operation::bridgeInfo & Data,
				fixs_ith_switchboardinfo & switch_board, fixs_ith::switchboard_plane_t plane)
{
	fixs_ith_snmpmanager & snmpmanager = fixs_ith::workingSet_t::instance()->get_snmpmanager();
	uint32_t timeout_ms = DEFAULT_SNMP_TIMEOUT_MS;

	fixs_ith_snmp::mac_address_t snmp_bridgeAddr;
	if (snmpmanager.get_bridge_address(snmp_bridgeAddr,switch_board, plane,&timeout_ms))
		return fixs_ith::ERR_GET_BRIDGE_INFO_FROM_SB;

	int32_t num_of_ports = 0;
		if (snmpmanager.get_number_of_ports(num_of_ports, switch_board, plane, &timeout_ms))
			return fixs_ith::ERR_GET_BRIDGE_INFO_FROM_SB;

	uint32_t num_of_vlans = 0;
		if (snmpmanager.get_number_of_vlans(num_of_vlans, switch_board,plane, &timeout_ms))
			return fixs_ith::ERR_GET_BRIDGE_INFO_FROM_SB;

//  // TODO:  retrieve info about subnetVlan  ????
//	int32_t std::set<std::string> ipv4SubnetVlan;
//	if (snmpmanager.get_ipv4SubnetVlan(???, switch_board,sb_plane))
//			return fixs_ith::ERR_GET_BRIDGE_INFO_FROM_SB;
//  Data.ipv4SubnetVlan = .......;

	char numOfPorts_str[16] = {0};
	ACE_OS::snprintf(numOfPorts_str,FIXS_ITH_ARRAY_SIZE(numOfPorts_str), "%i",num_of_ports );

	char numOfVlans_str[16]= {0};
	ACE_OS::snprintf(numOfVlans_str,FIXS_ITH_ARRAY_SIZE(numOfPorts_str), "%u",num_of_vlans );

	char  imm_bridgeAddr[IMM_MACADDRESS_SIZE_MAX]= {0};
	macAddrToImmformat(imm_bridgeAddr, snmp_bridgeAddr);

	Data.bridgeAddress.assign(imm_bridgeAddr);
	Data.numberOfPorts.assign(numOfPorts_str);
	Data.numberOfVlans.assign(numOfVlans_str);

	return fixs_ith::ERR_NO_ERRORS;


}


fixs_ith::ErrorConstants fixs_ith_transportBNChandler::get_bridgePortData (operation::bridgePortInfo & Data, uint16_t if_index,
				fixs_ith_switchboardinfo & switch_board, fixs_ith::switchboard_plane_t plane)
{
	fixs_ith_snmpmanager & snmpmanager = fixs_ith::workingSet_t::instance()->get_snmpmanager();
	uint32_t timeout_ms = DEFAULT_SNMP_TIMEOUT_MS;

	char port_name[fixs_ith_snmp::IF_NAME_MAX_LEN] = {0};
	if (snmpmanager.get_port_name(port_name, if_index, switch_board, plane, &timeout_ms))
		return fixs_ith::ERR_GET_BRIDGEPORT_INFO_FROM_SB;

	fixs_ith_snmp::port_status_t admin_status;
	if(is_system_enabled_if(switch_board.magazine_id, plane, if_index))
	{
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG,"Bridge Port <%d> on switch board in magazine == 0x%08X slot %d plane == %d is a 'System enabled port' set 'admin state' to ENABLED",
								if_index, switch_board.magazine, switch_board.slot_position, plane);
		admin_status = fixs_ith_snmp::PORT_STATUS_UP;
	}
	else
	{
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG,"Bridge Port <%d> on switch board in magazine == 0x%08X slot %d plane == %d is not a 'System enabled port' get 'admin state' from switch",
										if_index, switch_board.magazine, switch_board.slot_position, plane);
		if (snmpmanager.get_port_admin_status(admin_status, if_index, switch_board, plane, &timeout_ms))
			return fixs_ith::ERR_GET_BRIDGEPORT_INFO_FROM_SB;
	}

	fixs_ith_snmp::mac_address_t port_addr;
	if (snmpmanager.get_port_physical_address(port_addr, if_index, switch_board, plane, &timeout_ms))
		return fixs_ith::ERR_GET_BRIDGEPORT_INFO_FROM_SB;

	char  imm_bridgeAddr[IMM_MACADDRESS_SIZE_MAX]= {0};
	macAddrToImmformat(imm_bridgeAddr, port_addr);

	Data.name.assign(port_name);
	Data.adminstate = (admin_status == fixs_ith_snmp::PORT_STATUS_UP)? imm::UNLOCKED : imm::LOCKED;
	Data.defaultVlanId = get_default_vlan_id(plane);
	Data.portAddress.assign(imm_bridgeAddr);

	return fixs_ith::ERR_NO_ERRORS;
}

bool fixs_ith_transportBNChandler::is_system_enabled_if(const fixs_ith::magazine_identifier_t magType, const fixs_ith::switchboard_plane_t & plane, const uint16_t& if_index)
{
	if (fixs_ith::CONTROL_PLANE == plane)
	{
		return engine::control::ENABLED_INTERFACES_INDEXES_SET.count(if_index) > 0;
	}
	else if (fixs_ith::TRANSPORT_PLANE == plane)
	{
		if(common::utility::isSwitchBoardSMX())
		{
			return (magType == fixs_ith::APT_MAGAZINE) ? engine::transport::ENABLED_INTERFACES_INDEXES_SET.count(if_index) > 0 &&
				engine::defaultAggregator::AGG_MEMBER_INDEXES_SET.count(if_index) == 0:
				engine::transport::ENABLED_INTERFACES_INDEXES_SET.count(if_index) > 0;
		}
		else if(common::utility::isSwitchBoardCMX()){

                        return (magType == fixs_ith::APT_MAGAZINE) ? engine::transport::ENABLED_INTERFACES_INDEXES_SET_CMX.count(if_index) > 0 &&
                                engine::defaultAggregatorCMX::AGG_MEMBER_INDEXES_SET.count(if_index) == 0:
                                engine::transport::ENABLED_INTERFACES_INDEXES_SET_CMX.count(if_index) > 0;
		}
	}

	return false;
}

uint16_t fixs_ith_transportBNChandler::get_default_vlan_id(const fixs_ith::switchboard_plane_t & plane)
{
	if (fixs_ith::CONTROL_PLANE == plane)
	{
		return engine::defaultVlan::control::VID;
	}
	else
	{
		return engine::defaultVlan::transport::VID;
	}
}

fixs_ith::ErrorConstants fixs_ith_transportBNChandler::get_aggregatorData(
		operation::aggregatorInfo & aggregatorInfo,
		fixs_ith_switchboardinfo & switch_board,
		fixs_ith::switchboard_plane_t plane) {
	fixs_ith::ErrorConstants call_result = fixs_ith::ERR_NO_ERRORS;
	if (common::utility::isSwitchBoardSMX()) {
		aggregatorInfo.name.assign(engine::defaultAggregator::NAME);
		aggregatorInfo.plane = engine::defaultAggregator::PLANE;
		aggregatorInfo.snmpIndex = engine::defaultAggregator::SNMP_INDEX;

		for (uint16_t i = 0;
				i
						< FIXS_ITH_ARRAY_SIZE(
								engine::defaultAggregator::AGG_MEMBER_INDEXES);
				++i) {
			operation::ethernetPortInfo ethportData;
			uint16_t if_index = engine::defaultAggregator::AGG_MEMBER_INDEXES[i];

			if (const int result = get_ethernetPortData(ethportData, if_index,
					switch_board, plane)) {
				if (result == fixs_ith::ERR_UNDEFINED_INTERFACE) {
					FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
							"Call 'get_ethernetPortData' failed: no interface defined in IF_TABLE at entry %d on switch board in magazine == 0x%08X slot %d plane == %d: call_result == %d",
							if_index, switch_board.magazine,
							switch_board.slot_position, plane, result);
					continue;
				} else {
					FIXS_ITH_LOG(LOG_LEVEL_ERROR,
							"Call 'get_ethernetPortData' failed: cannot retrieve Ethernet Port data from  switch board in magazine == 0x%08X slot %d plane == %d: call_result == %d",
							switch_board.magazine, switch_board.slot_position,
							plane, result);
					// return with error or create the object with default value?
					call_result = fixs_ith::ERR_GET_INTERFACE_INFO_FROM_SB;
					break;
				}
			}

			char rdnValue[IMM_RDN_PATH_SIZE_MAX] = { 0 };
			common::utility::build_port_rdn(rdnValue, ethportData.name.c_str(),
					switch_board.slot_position, switch_board.magazine, plane);

			char dnValue[IMM_DN_PATH_SIZE_MAX] = { 0 };
			::snprintf(dnValue, IMM_DN_PATH_SIZE_MAX, "%s,%s", rdnValue,
					imm::mom_dn::ROOT_MOC_DN);

			aggregatorInfo.aggMember.insert(dnValue);
		}

	} else if (common::utility::isSwitchBoardCMX()) {
		aggregatorInfo.name.assign(engine::defaultAggregatorCMX::NAME);
		aggregatorInfo.plane = engine::defaultAggregatorCMX::PLANE;
		aggregatorInfo.snmpIndex = engine::defaultAggregatorCMX::SNMP_INDEX;

		for (uint16_t i = 0;
				i
						< FIXS_ITH_ARRAY_SIZE(
								engine::defaultAggregatorCMX::AGG_MEMBER_INDEXES);
				++i) {
			operation::ethernetPortInfo ethportData;
			uint16_t if_index =
					engine::defaultAggregatorCMX::AGG_MEMBER_INDEXES[i];
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
					"Default Aggregator CMX in aggregatorData:%d", if_index);

			if (const int result = get_ethernetPortData(ethportData, if_index,
					switch_board, plane)) {
				if (result == fixs_ith::ERR_UNDEFINED_INTERFACE) {
					FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
							"Call 'get_ethernetPortData' failed: no interface defined in IF_TABLE at entry %d on switch board in magazine == 0x%08X slot %d plane == %d: call_result == %d",
							if_index, switch_board.magazine,
							switch_board.slot_position, plane, result);
					continue;
				} else {
					FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
							"Failed to create aggregator obj for port:%d , return value: %d",
							if_index, result);
					FIXS_ITH_LOG(LOG_LEVEL_ERROR,
							"Call 'get_ethernetPortData' failed: cannot retrieve Ethernet Port data from  switch board in magazine == 0x%08X slot %d plane == %d: call_result == %d",
							switch_board.magazine, switch_board.slot_position,
							plane, result);
					// return with error or create the object with default value?
					call_result = fixs_ith::ERR_GET_INTERFACE_INFO_FROM_SB;
					break;
				}
			} // close of main if

			char rdnValue[IMM_RDN_PATH_SIZE_MAX] = { 0 };
			common::utility::build_port_rdn(rdnValue, ethportData.name.c_str(),
					switch_board.slot_position, switch_board.magazine, plane);

			char dnValue[IMM_DN_PATH_SIZE_MAX] = { 0 };
			::snprintf(dnValue, IMM_DN_PATH_SIZE_MAX, "%s,%s", rdnValue,
					imm::mom_dn::ROOT_MOC_DN);

			aggregatorInfo.aggMember.insert(dnValue);
		}
	}

	return call_result;

}

std::vector<operation::vlanInfo> fixs_ith_transportBNChandler::get_bnc_vlans (fixs_ith_switchboardinfo & switch_board, fixs_ith::switchboard_plane_t plane, std::vector<engine::static_vlan_t> vlan_list)
{
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG,"Entered get_bnc_vlans()");
	std::vector<operation::vlanInfo> bncVlans;

	std::string switchboard_id = common::utility::build_sb_key_from_magazine_and_slot(switch_board.magazine,switch_board.slot_position);
	engine::contextAccess_t contextAccess(switchboard_id, engine::GET_EXISTING, engine::SHARED_ACCESS);
	engine::Context* context = contextAccess.getContext();

	if (context)
	{
		for(std::vector<engine::static_vlan_t>::iterator it = vlan_list.begin(); it != vlan_list.end(); ++it)
		{
			if ((it->plane & plane) &&
					(it->magazine_id & switch_board.magazine_id) &&
					(it->slot == switch_board.slot_position) )
			{
				operation::vlanInfo vlanEntry;

				vlanEntry.name.assign(it->name);
				vlanEntry.plane = it->plane;
				vlanEntry.vId = it->vid;

				//tagged ports
				for (std::set<uint16_t>::iterator it_port = it->taggedPortsIndexes.begin(); it_port != it->taggedPortsIndexes.end(); ++it_port)
				{
					boost::shared_ptr<engine::BridgePort> bridgePort = context->getBridgePortByIndex(plane, *it_port);
					if (bridgePort)
					{
						vlanEntry.taggedBridgePorts.insert(bridgePort->getBridgePortDN());
					}
				}

				//untagged ports
				for (std::set<uint16_t>::iterator it_port = it->untaggedPortsIndexes.begin(); it_port != it->untaggedPortsIndexes.end(); ++it_port)
				{
					boost::shared_ptr<engine::BridgePort> bridgePort = context->getBridgePortByIndex(plane, *it_port);
					if (bridgePort)
					{
						vlanEntry.untaggedBridgePorts.insert(bridgePort->getBridgePortDN());
					}
				}

				bncVlans.push_back(vlanEntry);
			}
		}
	}

	return bncVlans;

}

fixs_ith::ErrorConstants fixs_ith_transportBNChandler::create_transportRootMOCobj(OmSession &session)
{

	if(!session.is_opened())
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,"IMM session not opened: cannot create Transport Root object");
		return fixs_ith::ERR_OM_HANDLER_NOT_INITIALIZED;
	}

	/*The vector of attributes*/
	std::vector<ACS_CC_ValuesDefinitionType> attrList;
	/*the attributes*/
	ACS_CC_ValuesDefinitionType attrRdn;

	/*Fill the rdn Attribute */

	char attrdn[IMM_ATTR_NAME_MAX_SIZE]= {0};
	strncpy(attrdn, imm::root_moc_attribute::RDN.c_str(),IMM_ATTR_NAME_MAX_SIZE-1) ;
	attrRdn.attrName = attrdn;
	attrRdn.attrType = ATTR_STRINGT;
	attrRdn.attrValuesNum = 1;

	char rdnValue[IMM_RDN_PATH_SIZE_MAX] = {0};
	strncpy(rdnValue, imm::mom_dn::ROOT_MOC_DN,IMM_ATTR_NAME_MAX_SIZE-1) ;
	void *attrRdnValue[1] = {reinterpret_cast<void*>(rdnValue)};
	attrRdn.attrValues = attrRdnValue;

	attrList.push_back(attrRdn);

	char class_name[IMM_CLASS_NAME_MAX_SIZE]= {0};
	strncpy(class_name, imm::moc_name::CLASS_TRANSPORT.c_str(),IMM_CLASS_NAME_MAX_SIZE-1) ;
	ACS_CC_ReturnType res;
	res = session.get_handler().createObject(class_name,0, attrList );
	if (res == ACS_CC_FAILURE) {

		int imm_error_code = session.get_handler().getInternalLastError();
		if (imm_error_code == -14) { // -14 == IMM OBJECT ALREADY EXIST
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "createObject %s FAILURE  error_code == %d errorText == '%s", imm::moc_name::CLASS_TRANSPORT.c_str(),
					imm_error_code, session.get_handler().getInternalLastErrorText());
			return fixs_ith::ERR_NO_ERRORS;
		}
		else {
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "createObject %s FAILURE  error_code == %d errorText == '%s", imm::moc_name::CLASS_TRANSPORT.c_str(),
				imm_error_code, session.get_handler().getInternalLastErrorText());
			return fixs_ith::ERR_IMM_CREATE_OBJ;
		}

	}
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Transport Root - %s - successful created! ", imm::mom_dn::ROOT_MOC_DN);
	return fixs_ith::ERR_NO_ERRORS;
}

fixs_ith::ErrorConstants fixs_ith_transportBNChandler::create_ethernetPort_obj(operation::ethernetPortInfo & ethportData,
		uint32_t sb_magazine, int32_t sb_slot, fixs_ith::switchboard_plane_t plane, OmSession &session)
{
	if(!session.is_opened())
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,"IMM session not opened: cannot create ethernetPort object");
		return fixs_ith::ERR_OM_HANDLER_NOT_INITIALIZED;
	}

	/*Fill the rdn Attribute */
	char rdnValue[IMM_RDN_PATH_SIZE_MAX] = {0};
	common::utility::build_port_rdn(rdnValue, ethportData.name.c_str(), sb_slot, sb_magazine, plane);

	char magazine_str[16] = {0};
	fixs_ith_csreader::uint32_to_ip_format(magazine_str, sb_magazine);
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "create_ethernetPort_obj < %s > on magazine %s - slot %d - plane %s", rdnValue, magazine_str, sb_slot, common::utility::planeToString(plane).c_str());

	/*The vector of attributes*/
	std::vector<ACS_CC_ValuesDefinitionType> attrList;
	/*the attributes*/
	ACS_CC_ValuesDefinitionType attrRdn;
	ACS_CC_ValuesDefinitionType attr1;
	ACS_CC_ValuesDefinitionType attr2;
	ACS_CC_ValuesDefinitionType attr3;
	ACS_CC_ValuesDefinitionType attr4;

	// RDN
	char attrdn[IMM_ATTR_NAME_MAX_SIZE]= {0};
	strncpy(attrdn, imm::ethernetPort_attribute::RDN.c_str(),IMM_ATTR_NAME_MAX_SIZE-1);
	attrRdn.attrName = attrdn;
	attrRdn.attrType = ATTR_STRINGT;
	attrRdn.attrValuesNum = 1;
	void* attrRdnValue[1] = {reinterpret_cast<void*>(rdnValue)};
	attrRdn.attrValues =	attrRdnValue;

	// macAddress
	char name_attr1[IMM_ATTR_NAME_MAX_SIZE]= {0};
	strncpy(name_attr1, imm::ethernetPort_attribute::MAC_ADDRESS.c_str(),IMM_ATTR_NAME_MAX_SIZE-1);
	attr1.attrName = name_attr1;
	attr1.attrType = ATTR_STRINGT;
	attr1.attrValuesNum = 1;
	const unsigned int bufferSize= 256U;
	char tmpValue[bufferSize] = {0};
	ACE_OS::snprintf(tmpValue, bufferSize, "%s", ethportData.macAddress.c_str());
	void* tmpValueArray[1] = { reinterpret_cast<void*>(tmpValue) };
	attr1.attrValues = tmpValueArray;

	// maxFrameSize
	char name_attr2[IMM_ATTR_NAME_MAX_SIZE]= {0};
	strncpy(name_attr2, imm::ethernetPort_attribute::MAX_FRAME_SIZE.c_str(),IMM_ATTR_NAME_MAX_SIZE-1);
	attr2.attrName = name_attr2;
	attr2.attrValuesNum = 1;
	attr2.attrType = ATTR_UINT32T;
	void* attr2Value[1] = {reinterpret_cast<void*>(&ethportData.maxFrameSize)};
	attr2.attrValues = attr2Value;

	// snmpIndex
	char name_attr3[IMM_ATTR_NAME_MAX_SIZE]= {0};
	strncpy(name_attr3, imm::ethernetPort_attribute::SNMP_INDEX.c_str(),IMM_ATTR_NAME_MAX_SIZE-1);
	attr3.attrName = name_attr3;
	attr3.attrType = ATTR_UINT32T;
	attr3.attrValuesNum = 1;
	void* attr3Value[1] = {reinterpret_cast<void*>(&ethportData.snmpIndex)};
	attr3.attrValues = attr3Value;

	// autoNegotiate
	char name_attr4[IMM_ATTR_NAME_MAX_SIZE]= {0};
	strncpy(name_attr4, imm::ethernetPort_attribute::AUTO_NEGOTIATE.c_str(),IMM_ATTR_NAME_MAX_SIZE-1);
	attr4.attrName = name_attr4;
	attr4.attrValuesNum = 1;
	attr4.attrType = ATTR_INT32T;
	int32_t autoNegotiate_value = ethportData.autoNegotiate;
	void* attr4Value[1] = {reinterpret_cast<void*>(&autoNegotiate_value)};
	attr4.attrValues = attr4Value;


	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "ETH_PORT create: IF_INDEX: %u, MAC ADDRESS: %s, MAX FRAME SIZE: %u, AUTO NEGOTIATE: %d",
			ethportData.snmpIndex, ethportData.macAddress.c_str(), ethportData.maxFrameSize, ethportData.autoNegotiate);

	attrList.push_back(attrRdn);
	attrList.push_back(attr1);
	attrList.push_back(attr2);
	attrList.push_back(attr3);
	attrList.push_back(attr4);

	char class_name[IMM_CLASS_NAME_MAX_SIZE]= {0};
	strncpy(class_name, imm::moc_name::CLASS_ETH_PORT.c_str(),IMM_CLASS_NAME_MAX_SIZE-1) ;
	ACS_CC_ReturnType res;
	res = session.get_handler().createObject( class_name, imm::mom_dn::ROOT_MOC_DN, attrList );
	if (res == ACS_CC_FAILURE) {
		int imm_error_code = session.get_handler().getInternalLastError();
		if (imm_error_code == -14) { // -14 == IMM OBJECT ALREADY EXIST
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "createObject - class_name == %s parent_name == %s - FAILURE error_code == %d errorText == '%s", imm::moc_name::CLASS_ETH_PORT.c_str(), imm::mom_dn::ROOT_MOC_DN,
					imm_error_code, session.get_handler().getInternalLastErrorText());
			return fixs_ith::ERR_IMM_OBJECT_ALREADY_EXIST;
		}else {
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "createObject - class_name == %s parent_name == %s - FAILURE error_code == %d errorText == '%s", imm::moc_name::CLASS_ETH_PORT.c_str(), imm::mom_dn::ROOT_MOC_DN,
					session.get_handler().getInternalLastError(), session.get_handler().getInternalLastErrorText());
			return fixs_ith::ERR_IMM_CREATE_OBJ;
		}
	}
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "create_ethernetPort_obj magazine %s - slot %d  successful executed!", magazine_str, sb_slot);
	return fixs_ith::ERR_NO_ERRORS;;
}

fixs_ith::ErrorConstants fixs_ith_transportBNChandler::create_bridge_obj(operation::bridgeInfo & bridgeData,
		uint32_t sb_magazine, int32_t sb_slot, fixs_ith::switchboard_plane_t plane, OmSession &session)
{
	fixs_ith::ErrorConstants call_result = fixs_ith::ERR_NO_ERRORS;

	if(!session.is_opened())
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,"IMM session not opened: cannot create bridge object");
		return fixs_ith::ERR_OM_HANDLER_NOT_INITIALIZED;
	}
	char rdnValue[IMM_RDN_PATH_SIZE_MAX] = {0};
	common::utility::build_bridge_rdn(rdnValue, sb_slot, sb_magazine, plane);

	char magazine_str[16] = {0};
	fixs_ith_csreader::uint32_to_ip_format(magazine_str, sb_magazine);

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "create_bridge_obj < %s > on magazine %s - slot %d - plane %s", rdnValue, magazine_str, sb_slot, common::utility::planeToString(plane).c_str());

	/*The vector of attributes*/
	std::vector<ACS_CC_ValuesDefinitionType> attrList;
	/*the attributes*/
	ACS_CC_ValuesDefinitionType attrRdn;
	ACS_CC_ValuesDefinitionType attrBridgeAddress;

	/*Fill the rdn Attribute */
	char attrdn[IMM_ATTR_NAME_MAX_SIZE]= {0};
	strncpy(attrdn, imm::bridge_attribute::RDN.c_str(),IMM_ATTR_NAME_MAX_SIZE-1) ;
	attrRdn.attrName = attrdn;
	attrRdn.attrType = ATTR_STRINGT;
	attrRdn.attrValuesNum = 1;
	void *attrRdnvalue[1] = {reinterpret_cast<void*>(rdnValue)};
	attrRdn.attrValues = attrRdnvalue;


	/*Fill the address Attribute */
	char name_attr1[IMM_ATTR_NAME_MAX_SIZE]= {0};
	strncpy(name_attr1, imm::bridge_attribute::ADDRESS.c_str(),IMM_ATTR_NAME_MAX_SIZE-1) ;
	attrBridgeAddress.attrName = name_attr1;
	attrBridgeAddress.attrType = ATTR_STRINGT;
	attrBridgeAddress.attrValuesNum = 1;

	const unsigned int bufferSize= 256U;
	char attr1Value[bufferSize] = {0};
	ACE_OS::snprintf(attr1Value, bufferSize,"%s", bridgeData.bridgeAddress.c_str() );
	void* attr1ValueAddr[1] = { reinterpret_cast<void*>(attr1Value) };
	attrBridgeAddress.attrValues = attr1ValueAddr;

	/*Fill the subnet vlans Attribute */
	ACS_CC_ValuesDefinitionType subnetvlan_attribute;
	char name_attr4[IMM_ATTR_NAME_MAX_SIZE]= {0};
	strncpy(name_attr4, imm::bridge_attribute::SUBNET_VLAN_REF.c_str(),IMM_ATTR_NAME_MAX_SIZE-1) ;
	subnetvlan_attribute.attrName = name_attr4;
	subnetvlan_attribute.attrType = ATTR_NAMET;
	subnetvlan_attribute.attrValuesNum = 0;
	subnetvlan_attribute.attrValues = NULL;

	// set attributes list
	attrList.push_back(attrRdn);
	attrList.push_back(attrBridgeAddress);
	attrList.push_back(subnetvlan_attribute);

	char class_name[IMM_CLASS_NAME_MAX_SIZE]= {0};
	strncpy(class_name, imm::moc_name::CLASS_BRIDGE.c_str(),IMM_CLASS_NAME_MAX_SIZE-1) ;
	ACS_CC_ReturnType res;
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG,"calling createObject Class Name : <%s>, Parent DN: <%s>", class_name, imm::mom_dn::ROOT_MOC_DN);
//	res = session.get_handler().createObject(class_name, imm::mom_dn::ROOT_MOC_DN, attrList );
	res = session.get_handler().createObject("AxeInfrastructureTransportMBridge", imm::mom_dn::ROOT_MOC_DN, attrList );
	if (res == ACS_CC_FAILURE) {
		int imm_error_code = session.get_handler().getInternalLastError();
		if (imm_error_code == -14) { // -14 == IMM OBJECT ALREADY EXIST
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "createObject - class_name == %s parent_name == %s - FAILURE error_code == %d errorText == '%s", imm::moc_name::CLASS_BRIDGE.c_str(), imm::mom_dn::ROOT_MOC_DN,
					imm_error_code, session.get_handler().getInternalLastErrorText());
			return fixs_ith::ERR_NO_ERRORS;
		}else {
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "createObject - class_name == %s parent_name == %s - FAILURE error_code == %d errorText == '%s", imm::moc_name::CLASS_BRIDGE.c_str(), imm::mom_dn::ROOT_MOC_DN,
					session.get_handler().getInternalLastError(), session.get_handler().getInternalLastErrorText());
			return fixs_ith::ERR_IMM_CREATE_OBJ;
		}
	}
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "create_bridge_obj magazine %s - slot %d  successful executed!", magazine_str, sb_slot);
	return call_result;
}


fixs_ith::ErrorConstants fixs_ith_transportBNChandler::create_bridgePort_obj(operation::bridgePortInfo & bridgePortData,
		uint32_t sb_magazine, int32_t sb_slot, fixs_ith::switchboard_plane_t plane, OmSession & session)
{
	if(!session.is_opened())
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,"IMM session not opened: cannot create bridgePort object");
		return fixs_ith::ERR_OM_HANDLER_NOT_INITIALIZED;
	}

	char bridgeRdn[IMM_RDN_PATH_SIZE_MAX] = {0};

	if(common::utility::build_bridge_rdn(bridgeRdn, sb_slot, sb_magazine, plane)){
		return fixs_ith::ERR_BUILD_OBJECT_DN;
	}

	char rdnValue[IMM_RDN_PATH_SIZE_MAX] = {0};
	::snprintf(rdnValue, IMM_RDN_PATH_SIZE_MAX, "%s=%s", imm::bridgePort_attribute::RDN.c_str(), bridgePortData.name.c_str());

	char magazine_str[16] = {0};
	fixs_ith_csreader::uint32_to_ip_format(magazine_str, sb_magazine);
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "create_bridgePort_obj < %s > on magazine %s - slot %d - plane %s", rdnValue, magazine_str, sb_slot, common::utility::planeToString(plane).c_str());

	char parentDN[IMM_DN_PATH_SIZE_MAX] = {0};
	::snprintf(parentDN, IMM_DN_PATH_SIZE_MAX, "%s,%s", bridgeRdn,imm::mom_dn::ROOT_MOC_DN);

	/*The vector of attributes*/
	std::vector<ACS_CC_ValuesDefinitionType> attrList;
	/*the attributes*/
	ACS_CC_ValuesDefinitionType attrRdn;
	ACS_CC_ValuesDefinitionType attr1;
	ACS_CC_ValuesDefinitionType attr2;
	ACS_CC_ValuesDefinitionType attr3;

	/*Fill the rdn Attribute */
	char attrdn[IMM_ATTR_NAME_MAX_SIZE]= {0};
	strncpy(attrdn, imm::bridgePort_attribute::RDN.c_str(),IMM_ATTR_NAME_MAX_SIZE-1) ;
	attrRdn.attrName = attrdn;
	attrRdn.attrType = ATTR_STRINGT;
	attrRdn.attrValuesNum = 1;
	void *attrRdnvalue[1] = {reinterpret_cast<void*>(rdnValue)};
	attrRdn.attrValues = attrRdnvalue;


	/*Fill the port address Attribute */
	char name_attr1[IMM_ATTR_NAME_MAX_SIZE]= {0};
	strncpy(name_attr1, imm::bridgePort_attribute::PORT_ADDRESS.c_str(),IMM_ATTR_NAME_MAX_SIZE-1) ;
	attr1.attrName = name_attr1;
	attr1.attrType = ATTR_STRINGT;
	attr1.attrValuesNum = 1;

	const unsigned int bufferSize= 256U;
	char tmpValue[bufferSize] = {0};
	ACE_OS::snprintf(tmpValue, bufferSize,"%s", bridgePortData.portAddress.c_str() );
	void* tmpValueAddr[1] = { reinterpret_cast<void*>(tmpValue) };
	attr1.attrValues = tmpValueAddr;

	/*Fill the admin state Attribute */
	char name_attr2[IMM_ATTR_NAME_MAX_SIZE]= {0};
	strncpy(name_attr2, imm::bridgePort_attribute::ADM_STATE.c_str(),IMM_ATTR_NAME_MAX_SIZE-1) ;
	attr2.attrName = name_attr2;
	attr2.attrType = ATTR_INT32T;
	attr2.attrValuesNum = 1;
	void *attr2Value[1] = {reinterpret_cast<void*>(&bridgePortData.adminstate)};
	attr2.attrValues = attr2Value;

	/*Fill the default vlan id Attribute */
	char name_attr3[IMM_ATTR_NAME_MAX_SIZE]= {0};
	strncpy(name_attr3, imm::bridgePort_attribute::DEFAULT_VLAN_ID.c_str(),IMM_ATTR_NAME_MAX_SIZE-1) ;
	attr3.attrName = name_attr3;
	attr3.attrType = ATTR_UINT32T;
	attr3.attrValuesNum = 1;
	void * attr3Value[1] = {reinterpret_cast<void*>(&bridgePortData.defaultVlanId)};
	attr3.attrValues = attr3Value;

	/*Fill the default operative state (RUNTIME) Attribute */
	ACS_CC_ValuesDefinitionType attr4;
	char name_attr4[IMM_ATTR_NAME_MAX_SIZE]= {0};
	strncpy(name_attr4, imm::bridgePort_attribute::OPER_STATE.c_str(),IMM_ATTR_NAME_MAX_SIZE-1) ;
	attr4.attrName = name_attr4;
	attr4.attrType = ATTR_INT32T;
	attr4.attrValuesNum = 0;
	attr4.attrValues = NULL;


	attrList.push_back(attrRdn);
	attrList.push_back(attr1);
	attrList.push_back(attr2);
	attrList.push_back(attr3);
	attrList.push_back(attr4);

	char class_name[IMM_CLASS_NAME_MAX_SIZE]= {0};
	strncpy(class_name, imm::moc_name::CLASS_BRIDGE_PORT.c_str(),IMM_CLASS_NAME_MAX_SIZE-1) ;
	ACS_CC_ReturnType res;
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG,"calling createObject Class Name : <%s>, Parent DN: <%s>", class_name, parentDN);
	res = session.get_handler().createObject( class_name, parentDN, attrList );
	if (res == ACS_CC_FAILURE) {
		int imm_error_code = session.get_handler().getInternalLastError();
		if (imm_error_code == -14) { // -14 == IMM OBJECT ALREADY EXIST
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "createObject - class_name == %s parent_name == %s - FAILURE error_code == %d errorText == '%s", imm::moc_name::CLASS_BRIDGE_PORT.c_str(), parentDN,
					imm_error_code, session.get_handler().getInternalLastErrorText());
			return fixs_ith::ERR_IMM_OBJECT_ALREADY_EXIST;
		}else {
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "createObject - class_name == %s parent_name == %s - FAILURE error_code == %d errorText == '%s", imm::moc_name::CLASS_BRIDGE_PORT.c_str(), parentDN,
					session.get_handler().getInternalLastError(), session.get_handler().getInternalLastErrorText());
			return fixs_ith::ERR_IMM_CREATE_OBJ;
		}
	}
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "create_bridgePort_obj magazine %s - slot %d  successful executed!", magazine_str, sb_slot);
	return fixs_ith::ERR_NO_ERRORS;;
}


fixs_ith::ErrorConstants fixs_ith_transportBNChandler::create_aggregator_obj(operation::aggregatorInfo & aggregatorData,
			uint32_t sb_magazine, int32_t sb_slot, fixs_ith::switchboard_plane_t plane, OmSession & session)
{
	fixs_ith::ErrorConstants call_result = fixs_ith::ERR_NO_ERRORS;

	if(!session.is_opened())
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,"IMM session not opened: cannot create bridge object");
		return fixs_ith::ERR_OM_HANDLER_NOT_INITIALIZED;
	}

	/*Fill the rdn Attribute */
	char rdnValue[IMM_RDN_PATH_SIZE_MAX] = {0};
	common::utility::build_aggregator_rdn(rdnValue, aggregatorData.name.c_str(), sb_slot, sb_magazine, plane);

	char magazine_str[16] = {0};
	fixs_ith_csreader::uint32_to_ip_format(magazine_str, sb_magazine);
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "create_aggregator_obj < %s > on magazine %s - slot %d - plane %s", rdnValue, magazine_str, sb_slot, common::utility::planeToString(plane).c_str());

	/*The vector of attributes*/
	std::vector<ACS_CC_ValuesDefinitionType> attrList;
	/*the attributes*/
	ACS_CC_ValuesDefinitionType attrRdn;
	ACS_CC_ValuesDefinitionType attr1;
	ACS_CC_ValuesDefinitionType attr2;

	char attrdn[IMM_ATTR_NAME_MAX_SIZE]= {0};
	strncpy(attrdn, imm::aggregator_attribute::RDN.c_str(),IMM_ATTR_NAME_MAX_SIZE-1) ;
	attrRdn.attrName = attrdn;
	attrRdn.attrType = ATTR_STRINGT;
	attrRdn.attrValuesNum = 1;
	void *attrRdnvalue[1] = {reinterpret_cast<void*>(rdnValue)};
	attrRdn.attrValues = attrRdnvalue;

	// attr1 = adminAggMember
	char name_attr1[IMM_ATTR_NAME_MAX_SIZE]= {0};
	strncpy(name_attr1, imm::aggregator_attribute::AGG_MEMBER.c_str(),IMM_ATTR_NAME_MAX_SIZE-1) ;
	attr1.attrName = name_attr1;
	attr1.attrType = ATTR_NAMET;
	attr1.attrValuesNum = aggregatorData.aggMember.size();

	void **attr1Value = new void*[aggregatorData.aggMember.size()];
	std::vector<char*> attr1ValueVector(aggregatorData.aggMember.size(), 0);

	uint16_t j = 0;
	for (std::set<std::string>::iterator it = aggregatorData.aggMember.begin(); it != aggregatorData.aggMember.end();  ++it )
	{
		attr1ValueVector[j] = new char[IMM_DN_PATH_SIZE_MAX];
		memset(attr1ValueVector[j], 0, IMM_DN_PATH_SIZE_MAX);
		strcpy(attr1ValueVector[j],it->c_str());
		attr1Value[j] = reinterpret_cast<void*>(attr1ValueVector[j]);
		++j;
	}
	attr1.attrValues = attr1Value;

	char name_attr2[IMM_ATTR_NAME_MAX_SIZE]= {0};
	strncpy(name_attr2, imm::aggregator_attribute::SNMP_INDEX.c_str(),IMM_ATTR_NAME_MAX_SIZE-1) ;
	attr2.attrName = name_attr2;
	attr2.attrType = ATTR_UINT32T;
	attr2.attrValuesNum = 1;
	void *attr2Value[1] = {reinterpret_cast<void*>(&aggregatorData.snmpIndex)};
	attr2.attrValues = attr2Value;

	attrList.push_back(attrRdn);
	attrList.push_back(attr1);
	attrList.push_back(attr2);

	char class_name[IMM_CLASS_NAME_MAX_SIZE]= {0};
	strncpy(class_name, imm::moc_name::CLASS_AGGREGATOR.c_str(),IMM_CLASS_NAME_MAX_SIZE-1) ;
	ACS_CC_ReturnType res;
	res = session.get_handler().createObject( class_name, imm::mom_dn::ROOT_MOC_DN, attrList );

	//Delete dynamic objects, regardless of the result
	for (std::vector<char*>::iterator it = attr1ValueVector.begin(); it != attr1ValueVector.end(); ++it)
	{
		delete [] *it;
	}
	delete [] attr1Value;

	if (res == ACS_CC_FAILURE) {
		int imm_error_code = session.get_handler().getInternalLastError();
		if (imm_error_code == -14) { // -14 == IMM OBJECT ALREADY EXIST
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "createObject - class_name == %s parent_name == %s - FAILURE error_code == %d errorText == '%s", imm::moc_name::CLASS_ETH_PORT.c_str(), imm::mom_dn::ROOT_MOC_DN,
					imm_error_code, session.get_handler().getInternalLastErrorText());
			return fixs_ith::ERR_IMM_OBJECT_ALREADY_EXIST;
		}else {
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "createObject - class_name == %s parent_name == %s - FAILURE error_code == %d errorText == '%s", imm::moc_name::CLASS_ETH_PORT.c_str(), imm::mom_dn::ROOT_MOC_DN,
					session.get_handler().getInternalLastError(), session.get_handler().getInternalLastErrorText());
			return fixs_ith::ERR_IMM_CREATE_OBJ;
		}
	}
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "create_aggregator_obj magazine %s - slot %d  successful executed!", magazine_str, sb_slot);
	return call_result;
}

fixs_ith::ErrorConstants fixs_ith_transportBNChandler::create_vlan_obj(operation::vlanInfo & vlanData,
			uint32_t sb_magazine, int32_t sb_slot, fixs_ith::switchboard_plane_t plane, OmSession & session)
{
	fixs_ith::ErrorConstants call_result = fixs_ith::ERR_NO_ERRORS;

	if(!session.is_opened())
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,"IMM session not opened: cannot create bridge object");
		return fixs_ith::ERR_OM_HANDLER_NOT_INITIALIZED;
	}

	/*Build dn value */
	char dnValue[IMM_DN_PATH_SIZE_MAX] = {0};
	common::utility::build_vlan_dn(dnValue, vlanData.name.c_str(), sb_slot, sb_magazine, plane);

	//fill rdn
	char rdnValue[IMM_RDN_PATH_SIZE_MAX] = {0};
	::snprintf(rdnValue, IMM_RDN_PATH_SIZE_MAX, "%s=%s", imm::vlan_attribute::RDN.c_str(),vlanData.name.c_str());

	char magazine_str[16] = {0};
	fixs_ith_csreader::uint32_to_ip_format(magazine_str, sb_magazine);
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "create_vlan_obj < %s > on magazine %s - slot %d - plane %s", rdnValue, magazine_str, sb_slot, common::utility::planeToString(plane).c_str());

	//fill parent
	char parent[IMM_DN_PATH_SIZE_MAX] = {0};
	strcpy(parent, common::utility::getParentDN(dnValue).c_str()) ;

	/*The vector of attributes*/
	std::vector<ACS_CC_ValuesDefinitionType> attrList;
	/*the attributes*/
	ACS_CC_ValuesDefinitionType attrRdn;
	ACS_CC_ValuesDefinitionType attr1;
	ACS_CC_ValuesDefinitionType attr2;
	ACS_CC_ValuesDefinitionType attr3;

	char attrdn[IMM_ATTR_NAME_MAX_SIZE]= {0};
	strncpy(attrdn, imm::vlan_attribute::RDN.c_str(),IMM_ATTR_NAME_MAX_SIZE-1) ;
	attrRdn.attrName = attrdn;
	attrRdn.attrType = ATTR_STRINGT;
	attrRdn.attrValuesNum = 1;
	void *attrRdnvalue[1] = {reinterpret_cast<void*>(rdnValue)};
	attrRdn.attrValues = attrRdnvalue;

	// attr1 = taggedBridgePorts
	char name_attr1[IMM_ATTR_NAME_MAX_SIZE]= {0};
	strncpy(name_attr1, imm::vlan_attribute::TAGGED_PORTS.c_str(),IMM_ATTR_NAME_MAX_SIZE-1) ;
	attr1.attrName = name_attr1;
	attr1.attrType = ATTR_NAMET;
	attr1.attrValuesNum = vlanData.taggedBridgePorts.size();

	void **attr1Value = new void*[vlanData.taggedBridgePorts.size()];
	std::vector<char*> attr1ValueVector(vlanData.taggedBridgePorts.size(), 0);

	uint16_t j = 0;
	for (std::set<std::string>::iterator it = vlanData.taggedBridgePorts.begin(); it != vlanData.taggedBridgePorts.end();  ++it )
	{
		attr1ValueVector[j] = new char[IMM_DN_PATH_SIZE_MAX];
		memset(attr1ValueVector[j], 0, IMM_DN_PATH_SIZE_MAX);
		strcpy(attr1ValueVector[j],it->c_str());
		attr1Value[j] = reinterpret_cast<void*>(attr1ValueVector[j]);
		++j;
	}
	attr1.attrValues = attr1Value;

	// attr2 = untaggedBridgePorts
	char name_attr2[IMM_ATTR_NAME_MAX_SIZE]= {0};
	strncpy(name_attr2, imm::vlan_attribute::UNTAGGED_PORTS.c_str(),IMM_ATTR_NAME_MAX_SIZE-1) ;
	attr2.attrName = name_attr2;
	attr2.attrType = ATTR_NAMET;
	attr2.attrValuesNum = vlanData.untaggedBridgePorts.size();

	void **attr2Value = new void*[vlanData.untaggedBridgePorts.size()];
	std::vector<char*> attr2ValueVector(vlanData.untaggedBridgePorts.size(), 0);

	j = 0;
	for (std::set<std::string>::iterator it = vlanData.untaggedBridgePorts.begin(); it != vlanData.untaggedBridgePorts.end();  ++it )
	{
		attr2ValueVector[j] = new char[IMM_DN_PATH_SIZE_MAX];
		memset(attr2ValueVector[j], 0, IMM_DN_PATH_SIZE_MAX);
		strcpy(attr2ValueVector[j],it->c_str());
		attr2Value[j] = reinterpret_cast<void*>(attr2ValueVector[j]);
		++j;
	}
	attr2.attrValues = attr2Value;

	// attr2 = vId
	char name_attr3[IMM_ATTR_NAME_MAX_SIZE]= {0};
	strncpy(name_attr3, imm::vlan_attribute::VLAN_ID.c_str(),IMM_ATTR_NAME_MAX_SIZE-1) ;
	attr3.attrName = name_attr3;
	attr3.attrType = ATTR_UINT32T;
	attr3.attrValuesNum = 1;
	void *attr3Value[1] = {reinterpret_cast<void*>(&vlanData.vId)};
	attr3.attrValues = attr3Value;

	attrList.push_back(attrRdn);
	attrList.push_back(attr1);
	attrList.push_back(attr2);
	attrList.push_back(attr3);

	char class_name[IMM_CLASS_NAME_MAX_SIZE]= {0};
	strncpy(class_name, imm::moc_name::CLASS_VLAN.c_str(),IMM_CLASS_NAME_MAX_SIZE-1) ;
	ACS_CC_ReturnType res;
	res = session.get_handler().createObject( class_name, parent, attrList );

	//Delete dynamic objects, regardless of the result
	for (std::vector<char*>::iterator it = attr1ValueVector.begin(); it != attr1ValueVector.end(); ++it)
	{
		delete [] *it;
	}
	delete [] attr1Value;

	//Delete dynamic objects, regardless of the result
	for (std::vector<char*>::iterator it2 = attr2ValueVector.begin(); it2 != attr2ValueVector.end(); ++it2)
	{
		delete [] *it2;
	}
	delete [] attr2Value;

	if (res == ACS_CC_FAILURE) {
		int imm_error_code = session.get_handler().getInternalLastError();
		if (imm_error_code == -14) { // -14 == IMM OBJECT ALREADY EXIST
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "createObject - class_name == %s parent_name == %s - FAILURE error_code == %d errorText == '%s", imm::moc_name::CLASS_VLAN.c_str(), parent,
					imm_error_code, session.get_handler().getInternalLastErrorText());
			return fixs_ith::ERR_IMM_OBJECT_ALREADY_EXIST;
		}else {
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "createObject - class_name == %s parent_name == %s - FAILURE error_code == %d errorText == '%s", imm::moc_name::CLASS_VLAN.c_str(), parent,
					session.get_handler().getInternalLastError(), session.get_handler().getInternalLastErrorText());
			return fixs_ith::ERR_IMM_CREATE_OBJ;
		}
	}
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "create_vlan_obj magazine %s - slot %d  successful executed!", magazine_str, sb_slot);
	return call_result;
}

//  *****************************************  //
//  **********   Utility Functions  *********  //
//  *****************************************  //


bool fixs_ith_transportBNChandler::immObject_exist(const char * objdn, OmSession & session )
{
	if(!objdn) return false;

	ACS_APGCC_ImmObject imm_object;

	// check MO instance presence invoking 'getObject()'
	imm_object.objName.assign(objdn);
	ACS_CC_ReturnType getObject_res = session.get_handler().getObject(&imm_object);

	if(getObject_res != ACS_CC_SUCCESS)
	{
		FIXS_ITH_LOG(LOG_LEVEL_INFO, "Object with DN = %s NOT EXIST!", objdn);
		return false;
	}
	FIXS_ITH_LOG(LOG_LEVEL_INFO, "Object with DN = %s EXIST.", objdn);
	return true;
}

bool fixs_ith_transportBNChandler::is_valid_ifIndex(int32_t ifIndex, fixs_ith::switchboard_plane_t plane){

	if(ifIndex <= 0) return false;
	if(plane == fixs_ith::CONTROL_PLANE && ifIndex > fixs_ith_snmp::CONTROL_INTERFACE_INDEX_MAX_VALUE) return false;
	if(plane == fixs_ith::TRANSPORT_PLANE && ifIndex > fixs_ith_snmp::TRANSPORT_INTERFACE_INDEX_MAX_VALUE) return false;

	return true;
}

void  fixs_ith_transportBNChandler::macAddrToImmformat(char (& immAddress)[IMM_MACADDRESS_SIZE_MAX], fixs_ith_snmp::mac_address_t  & bridge_addr)
{
	for(int i=0; i< IMM_MACADDRESS_SIZE_MAX; i++) immAddress[i]=0;

	::sprintf(immAddress, "%02x:%02x:%02x:%02x:%02x:%02x", bridge_addr[0],bridge_addr[1],bridge_addr[2],
				bridge_addr[3],bridge_addr[4],bridge_addr[5]);
}


// ONLY for BASIC TEST scope
void fixs_ith_transportBNChandler::deleteAllObjects(OmSession & session)
{
	ACS_CC_ReturnType result;
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Calling deleteAllObjects on DN = %s",imm::mom_dn::ROOT_MOC_DN);

	result=session.get_handler().deleteObject(imm::mom_dn::ROOT_MOC_DN, ACS_APGCC_SUBTREE);
	UNUSED(result);

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "deleteAllObjects on DN = %s executed with error_code == %d errorText == '%s",imm::mom_dn::ROOT_MOC_DN,
								session.get_handler().getInternalLastError(), session.get_handler().getInternalLastErrorText());

}
void fixs_ith_transportBNChandler::define_static_bnc_vlans ()
{

	//GEP2_GEP5 to GEP7 HWSWAP
	bool isSMX = common::utility::isSwitchBoardSMX();
	bool isCMX = common::utility::isSwitchBoardCMX();

	{//CONTROL LEFT
		engine::static_vlan_t defaultVlan;

		defaultVlan.name.assign(engine::defaultVlan::control::left::NAME);
		defaultVlan.plane = engine::defaultVlan::control::PLANE;
		defaultVlan.slot = engine::defaultVlan::control::left::SLOT;
		defaultVlan.vid = engine::defaultVlan::control::VID;
		defaultVlan.magazine_id = engine::defaultVlan::control::MAGAZINE_ID;
		defaultVlan.untaggedPortsIndexes = engine::defaultVlan::control::UNTAGGED_PORTS_INDEXES_SET;

		_bncVlans.push_back(defaultVlan);

		engine::static_vlan_t tipc;
		if ( (tipc.vid = common::utility::getVlanIdFromFile(FIXS_ITH_TIPC_FILE_PATH)) != imm::vlanId_range::UNDEFINED )
		{
			tipc.name.assign(engine::tipcVlan::control::NAME);
			tipc.plane = engine::tipcVlan::control::PLANE;
			tipc.slot = fixs_ith::BOARD_SMX_LEFT;
			tipc.magazine_id = engine::tipcVlan::control::MAGAZINE_ID;
			tipc.taggedPortsIndexes = engine::tipcVlan::control::TAGGED_PORTS_INDEXES_SET;

			_bncVlans.push_back(tipc);
		}
	}

	{//CONTROL RIGHT
		engine::static_vlan_t defaultVlan;

		defaultVlan.name.assign(engine::defaultVlan::control::right::NAME);
		defaultVlan.plane = engine::defaultVlan::control::PLANE;
		defaultVlan.slot = engine::defaultVlan::control::right::SLOT;
		defaultVlan.vid = engine::defaultVlan::control::VID;
		defaultVlan.magazine_id = engine::defaultVlan::control::MAGAZINE_ID;
		defaultVlan.untaggedPortsIndexes = engine::defaultVlan::control::UNTAGGED_PORTS_INDEXES_SET;

		_bncVlans.push_back(defaultVlan);

		engine::static_vlan_t tipc;
		if ( (tipc.vid = common::utility::getVlanIdFromFile(FIXS_ITH_TIPC_FILE_PATH)) != imm::vlanId_range::UNDEFINED )
		{

			tipc.name.assign(engine::tipcVlan::control::NAME);
			tipc.plane = engine::tipcVlan::control::PLANE;
			tipc.slot = fixs_ith::BOARD_SMX_RIGHT;
			tipc.magazine_id = engine::tipcVlan::control::MAGAZINE_ID;
			tipc.taggedPortsIndexes = engine::tipcVlan::control::TAGGED_PORTS_INDEXES_SET;

			_bncVlans.push_back(tipc);
		}
	}

	{//TRANSPORT LEFT

		engine::static_vlan_t defaultVlan;
		fixs_ith::board_slotposition_t SLOT;

		defaultVlan.name.assign(engine::defaultVlan::transport::left::NAME);
		defaultVlan.plane = engine::defaultVlan::transport::PLANE;
		defaultVlan.slot = engine::defaultVlan::transport::left::SLOT;
		defaultVlan.vid = engine::defaultVlan::control::VID;
		defaultVlan.magazine_id = engine::defaultVlan::transport::MAGAZINE_ID;
		
		if (isCMX) {
			SLOT = fixs_ith::BOARD_CMX_LEFT;
			defaultVlan.slot = SLOT;
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
					"Modified the slot value in case of CMX::Transport_Left");
		}

		_bncVlans.push_back(defaultVlan);

		//engine::static_vlan_t drbd;
		if (isSMX) {
			engine::static_vlan_t drbd;
			if ((drbd.vid = common::utility::getVlanIdFromFile(
					FIXS_ITH_DRBD_FILE_PATH)) != imm::vlanId_range::UNDEFINED) {
				drbd.name.assign(engine::drbdVlan::transport::NAME);
				drbd.plane = engine::drbdVlan::transport::PLANE;
				drbd.slot = fixs_ith::BOARD_SMX_LEFT;
				drbd.magazine_id = engine::drbdVlan::transport::MAGAZINE_ID;
				drbd.taggedPortsIndexes =
						engine::drbdVlan::transport::TAGGED_PORTS_INDEXES_SET;
			}
			_bncVlans.push_back(drbd);
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
					"Pushed the drbdvlan for SMX::Transport_Left");
		}
		else if (isCMX) {
			engine::static_vlan_t drbd;
			drbd.vid = 1206;
			drbd.name.assign(engine::drbdVlan::transport::NAME);
			drbd.plane = engine::drbdVlan::transport::PLANE;
			drbd.slot = fixs_ith::BOARD_CMX_LEFT;
			drbd.magazine_id = engine::drbdVlan::transport::MAGAZINE_ID;
			drbd.taggedPortsIndexes =
					engine::drbdVlan::transport::TAGGED_PORTS_INDEXES_SET_CMX;
			_bncVlans.push_back(drbd);
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
					"Pushed the drbdvlan for CMX::Transport_Left");

		}

		//engine::static_vlan_t oam;
		if (isSMX) {
			engine::static_vlan_t oam;
			if ((oam.vid = common::utility::getVlanIdFromFile(
					FIXS_ITH_OAM_FILE_PATH)) != imm::vlanId_range::UNDEFINED) {
				oam.name.assign(engine::apgOamVlan::transport::NAME);
				oam.plane = engine::apgOamVlan::transport::PLANE;
				oam.slot = fixs_ith::BOARD_SMX_LEFT;
				oam.magazine_id = engine::apgOamVlan::transport::MAGAZINE_ID;
				oam.taggedPortsIndexes =
						engine::apgOamVlan::transport::TAGGED_PORTS_INDEXES_SET;

			}
			_bncVlans.push_back(oam);
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
					"Pushed the Oamvlan for SMX::Transport_Left");
		}

		else if (isCMX) {
			engine::static_vlan_t oam;
			oam.vid = 900;
			oam.name.assign(engine::apgOamVlan::transport::NAME);
			oam.plane = engine::apgOamVlan::transport::PLANE;
			oam.slot = fixs_ith::BOARD_CMX_LEFT;
			oam.taggedPortsIndexes =
					engine::apgOamVlan::transport::TAGGED_PORTS_INDEXES_SET_CMX;
			_bncVlans.push_back(oam);
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
					"Pushed the Oamvlan for CMX::Transport_Left");

		}

		// EVOET-INGRESS VLAN, only for CMX 
		engine::static_vlan_t evoet;
		if (isCMX) {
			evoet.vid = engine::evoEtIngress::transport::VID;
			evoet.name.assign(engine::evoEtIngress::transport::NAME);
			evoet.plane = engine::evoEtIngress::transport::PLANE;
			evoet.slot = fixs_ith::BOARD_CMX_LEFT;
			evoet.magazine_id = engine::evoEtIngress::transport::MAGAZINE_ID;
			evoet.taggedPortsIndexes =
					engine::evoEtIngress::transport::TAGGED_PORTS_INDEXES_SET;

			_bncVlans.push_back(evoet);

			FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
					"Pushed the EvoET-INGRESS Vlan for CMX::Transport_Left");
		}

		// TR HY37073
		//GEP2_GEP5 to GEP7 Dynamic UPD VLAN feature.
		if (!common::utility::is_hwtype_gep5()) {
			if (isSMX) {
				engine::static_vlan_t upd;
				upd.vid = engine::updVlan::transport::left::VID_LEFT;
				upd.name.assign(engine::updVlan::transport::left::NAME);
				upd.plane = engine::updVlan::transport::PLANE;
				upd.slot = fixs_ith::BOARD_SMX_LEFT;
				upd.magazine_id = engine::updVlan::transport::MAGAZINE_ID;
				upd.taggedPortsIndexes =
						engine::updVlan::transport::TAGGED_PORTS_INDEXES_SET;

				_bncVlans.push_back(upd);
			}

			else if (isCMX) {
				engine::static_vlan_t upd;
				upd.vid = engine::updVlan::transport::cmxleft::VID_LEFT;
				upd.name.assign(engine::updVlan::transport::cmxleft::NAME);
				upd.plane = engine::updVlan::transport::PLANE;
				upd.slot = fixs_ith::BOARD_CMX_LEFT;
				upd.magazine_id = engine::updVlan::transport::MAGAZINE_ID;
				upd.taggedPortsIndexes =
						engine::updVlan::transport::TAGGED_PORTS_INDEXES_SET_CMX;

				_bncVlans.push_back(upd);
			}
		}
		//GEP2_GEP5 to GEP7 Dynamic UPD VLAN END
	}

	{                //TRANSPORT RIGHT
		engine::static_vlan_t defaultVlan;
		fixs_ith::board_slotposition_t SLOT;

		defaultVlan.name.assign(engine::defaultVlan::transport::right::NAME);
		defaultVlan.plane = engine::defaultVlan::transport::PLANE;
		defaultVlan.vid = engine::defaultVlan::control::VID;
		defaultVlan.slot = engine::defaultVlan::transport::right::SLOT;
		defaultVlan.magazine_id = engine::defaultVlan::transport::MAGAZINE_ID;

		if (isCMX) {
			SLOT = fixs_ith::BOARD_CMX_RIGHT;
			defaultVlan.slot = SLOT;
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
					"Modified the slot value in case of CMX::Transport_Right");
		}

		_bncVlans.push_back(defaultVlan);

		// engine::static_vlan_t drbd;
		if (isSMX) {
			engine::static_vlan_t drbd;
			if ((drbd.vid = common::utility::getVlanIdFromFile(
					FIXS_ITH_DRBD_FILE_PATH)) != imm::vlanId_range::UNDEFINED) {
				drbd.name.assign(engine::drbdVlan::transport::NAME);
				drbd.plane = engine::drbdVlan::transport::PLANE;
				drbd.slot = fixs_ith::BOARD_SMX_RIGHT;
				drbd.magazine_id = engine::drbdVlan::transport::MAGAZINE_ID;
				drbd.taggedPortsIndexes =
						engine::drbdVlan::transport::TAGGED_PORTS_INDEXES_SET;
			}
			_bncVlans.push_back(drbd);
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
					"Pushed the drbdvlan for SMX::Transport_Right");
		}

		else if (isCMX) {
			engine::static_vlan_t drbd;
			drbd.vid = 1206;
			drbd.name.assign(engine::drbdVlan::transport::NAME);
			drbd.plane = engine::drbdVlan::transport::PLANE;
			drbd.slot = fixs_ith::BOARD_CMX_RIGHT;
			drbd.magazine_id = engine::drbdVlan::transport::MAGAZINE_ID;
			drbd.taggedPortsIndexes =
					engine::drbdVlan::transport::TAGGED_PORTS_INDEXES_SET_CMX;

			_bncVlans.push_back(drbd);
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
					"Pushed the drbdvlan for CMX::Transport_Right");
		}

		//engine::static_vlan_t oam;
		if (isSMX) {
			engine::static_vlan_t oam;
			if ((oam.vid = common::utility::getVlanIdFromFile(
					FIXS_ITH_OAM_FILE_PATH)) != imm::vlanId_range::UNDEFINED) {
				oam.name.assign(engine::apgOamVlan::transport::NAME);
				oam.plane = engine::apgOamVlan::transport::PLANE;
				oam.slot = fixs_ith::BOARD_SMX_RIGHT;
				oam.magazine_id = engine::apgOamVlan::transport::MAGAZINE_ID;
				oam.taggedPortsIndexes =
						engine::apgOamVlan::transport::TAGGED_PORTS_INDEXES_SET;
			}

			_bncVlans.push_back(oam);
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
					"Pushed the Oamvlan for SMX::Transport_Right");
		}

		else if (isCMX) {
			engine::static_vlan_t oam;
			oam.vid = 900;
			oam.name.assign(engine::apgOamVlan::transport::NAME);
			oam.plane = engine::apgOamVlan::transport::PLANE;
			oam.slot = fixs_ith::BOARD_CMX_RIGHT;
			oam.taggedPortsIndexes =
					engine::apgOamVlan::transport::TAGGED_PORTS_INDEXES_SET_CMX;
			_bncVlans.push_back(oam);
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
					"Pushed the Oamvlan for CMX::Transport_Left");

		}

		engine::static_vlan_t evoet;
		if (isCMX) {
			evoet.vid = engine::evoEtIngress::transport::VID;
			evoet.name.assign(engine::evoEtIngress::transport::NAME);
			evoet.plane = engine::evoEtIngress::transport::PLANE;
			evoet.slot = fixs_ith::BOARD_CMX_RIGHT;
			evoet.magazine_id = engine::evoEtIngress::transport::MAGAZINE_ID;
			evoet.taggedPortsIndexes =
					engine::evoEtIngress::transport::TAGGED_PORTS_INDEXES_SET;

			_bncVlans.push_back(evoet);

			FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
					"Pushed the EvoET-INGRESS Vlan for CMX::Transport_Left");
		}

		// TR HY37073
		//GEP2_GEP5 to GEP7 Dynamic UPD VLAN feature.
		if (!common::utility::is_hwtype_gep5()) {
			if (isSMX) {
				engine::static_vlan_t upd;
				upd.vid = engine::updVlan::transport::right::VID_RIGHT;
				upd.name.assign(engine::updVlan::transport::right::NAME);
				upd.plane = engine::updVlan::transport::PLANE;
				upd.slot = fixs_ith::BOARD_SMX_RIGHT;
				upd.magazine_id = engine::updVlan::transport::MAGAZINE_ID;
				upd.taggedPortsIndexes =
						engine::updVlan::transport::TAGGED_PORTS_INDEXES_SET;

				_bncVlans.push_back(upd);
			}

			else if (isCMX) {
				engine::static_vlan_t upd;
				upd.vid = engine::updVlan::transport::cmxright::VID_RIGHT;
				upd.name.assign(engine::updVlan::transport::cmxright::NAME);
				upd.plane = engine::updVlan::transport::PLANE;
				upd.slot = fixs_ith::BOARD_CMX_RIGHT;
				upd.magazine_id = engine::updVlan::transport::MAGAZINE_ID;
				upd.taggedPortsIndexes =
						engine::updVlan::transport::TAGGED_PORTS_INDEXES_SET_CMX;
				_bncVlans.push_back(upd);
			}
		}
		//GEP2_GEP5 to GEP7 Dynamic UPD VLAN END
	}
}

fixs_ith::ErrorConstants fixs_ith_transportBNChandler::delete_bnc_objects(const std::string& switchboard_key)
{
	fixs_ith::ErrorConstants  call_result = fixs_ith::ERR_NO_ERRORS;

	OmSession session;

	// Initialize OmHandler
	if(( call_result = session.open()) != fixs_ith::ERR_NO_ERRORS)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,"create_bnc_objects failed ! Cannot open imm session  call_result == %d", call_result);
		return call_result;
	}

	std::vector<std::string> removeMOs;

	if (ACS_CC_SUCCESS != get_objs_to_remove (removeMOs, switchboard_key, session))
	{
		return fixs_ith::ERR_DELETE_BNC_OBJECTS;
	}

	for (std::vector<std::string>::iterator it = removeMOs.begin(); it != removeMOs.end() && fixs_ith::ERR_NO_ERRORS == call_result; ++it)
	{
		if ((call_result = delete_objects (*it, session)))
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
					"Call 'delete objects' failed: delete BNC objects failed of switch board %s: call_result == %d",
					switchboard_key.c_str(), call_result);

			call_result = fixs_ith::ERR_DELETE_BNC_OBJECTS;
		}
	}

	session.close();

	return call_result;

}

fixs_ith::ErrorConstants fixs_ith_transportBNChandler::delete_objects(const std::string& dn, OmSession & session, ACS_APGCC_ScopeT scope)
{
	fixs_ith::ErrorConstants call_result = fixs_ith::ERR_NO_ERRORS;
	ACS_CC_ReturnType res = ACS_CC_SUCCESS; // TR_HY37073

	if(!session.is_opened())
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,"IMM session not opened: cannot create bridge object");
		return fixs_ith::ERR_OM_HANDLER_NOT_INITIALIZED;
	}

	if (immObject_exist(dn.c_str(), session))
	{
		res = session.get_handler().deleteObject(dn.c_str(), scope);
	}

	if (res == ACS_CC_FAILURE)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "createObject - DN == %s - FAILURE error_code == %d errorText == '%s", dn.c_str(),
				session.get_handler().getInternalLastError(), session.get_handler().getInternalLastErrorText());
		return fixs_ith::ERR_IMM_DELETE_OBJ;

	}

	return call_result;
}

bool fixs_ith_transportBNChandler::get_objs_to_remove (std::vector<std::string>& remove_objs, const std::string& switchboard_key, OmSession & session)
{
	ACS_CC_ReturnType result = ACS_CC_SUCCESS;
	std::vector<std::string> rdnList;

	for (std::vector<std::string>::iterator it = _remove_classes.begin(); it != _remove_classes.end() && ACS_CC_SUCCESS == result; ++it)
	{
		result = imm::getClassInstances(&session.get_handler(), *it, switchboard_key, remove_objs);
		if( ACS_CC_SUCCESS != result)
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Could not fetch objects for %s class.", (*it).c_str());

		}
	}

	return result;

}
