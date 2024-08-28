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

#ifndef ITHADM_CAA_INC_FIXS_ITH_TRANSPORTBNCHANDLER_H_
#define ITHADM_CAA_INC_FIXS_ITH_TRANSPORTBNCHANDLER_H_

#include <string>
#include <set>
#include <vector>
#include <acs_apgcc_omhandler.h>
#include "operation/operation.h"
#include "fixs_ith_switchboardinfo.h"
#include "fixs_ith_switchboardinfo.h"
#include "fixs_ith_snmpconstants.h"
#include "operation/operation.h"
#include "engine/basicNetworkConfiguration.h"


namespace BNC_DataStructure {

enum  EthPortOperatingMode {

 Mbps10_HALF = 2,       		//10Mbps speed, half duplex
 Mbps10_FULL = 3,      			//10Mbps speed, full duplex
 Mbps100_HALF = 4 ,    			//100Mbps speed, half duplex
 Mbps100_FULL = 5,     			//100Mbps speed, full duplex
 OneG_FULL = 6 ,        		// 1Gbps speed, full duplex
 OneG_FULL_SLAVE = 7,  			// 1Gbps speed, full duplex, slave mode
 OneG_FULL_MASTER = 8 , 		//1Gbps speed, full duplex, master mode
 TenG_FULL = 9 ,        		// 10Gbps speed, full duplex
 FortyG_FULL  = 12 ,            // 40Gbps speed, full duplex
 TenHG_FULL = 13 ,        		//100Gbps speed, full duplex
 TenG_FULL_MASTER = 10 ,     	//10Gbps speed, full duplex, master mode
 TenG_FULL_SLAVE = 11    		//10Gbps speed, full duplex, slave mode
};

}


class OmSession
{
public:
	OmSession(): _om_handler(), _immSessionOpened(false) {};
	~OmSession() {};

	fixs_ith::ErrorConstants open();
	fixs_ith::ErrorConstants close();

	bool is_opened() {return _immSessionOpened;};

	OmHandler& get_handler() {return _om_handler;};

private:
	OmHandler _om_handler;
	bool _immSessionOpened;
};

class fixs_ith_transportBNChandler {


	// Constructors //
	//==============//
public:
	/** @brief fixs_ith_transportBNChandler constructor
	 */
	inline fixs_ith_transportBNChandler ():_bncVlans() {
		//Build sets of BNC Vlans
		define_static_bnc_vlans();

		{//set of classes to remove
			_remove_classes.push_back(imm::moc_name::CLASS_NEXTHOP);
			_remove_classes.push_back(imm::moc_name::CLASS_DST);
			_remove_classes.push_back(imm::moc_name::CLASS_VRRP_SESSION);
			_remove_classes.push_back(imm::moc_name::CLASS_ADDRESS);
			_remove_classes.push_back(imm::moc_name::CLASS_INTERFACE);
			_remove_classes.push_back(imm::moc_name::CLASS_BFD_SESSION);
			_remove_classes.push_back(imm::moc_name::CLASS_BFD_PROFILE);
			_remove_classes.push_back(imm::moc_name::CLASS_ACL_ENTRY);
			_remove_classes.push_back(imm::moc_name::CLASS_ACL);
			_remove_classes.push_back(imm::moc_name::CLASS_ROUTER);
			_remove_classes.push_back(imm::moc_name::CLASS_SUBNET_VLAN);
			_remove_classes.push_back(imm::moc_name::CLASS_VLAN);
			_remove_classes.push_back(imm::moc_name::CLASS_BRIDGE_PORT);
			_remove_classes.push_back(imm::moc_name::CLASS_AGGREGATOR);
			_remove_classes.push_back(imm::moc_name::CLASS_ETH_PORT);
			_remove_classes.push_back(imm::moc_name::CLASS_BRIDGE);
		}
	};


	//============//
	// Destructor //
	//============//
public:
	/** @brief fixs_ith_transportBNChandler Destructor
	 */
	inline ~fixs_ith_transportBNChandler () {}

	int board_type;


	//===========//
	// Functions //
	//===========//
	fixs_ith::ErrorConstants create_transportRootMOCobj(OmSession &session);
	// create BNC MO for all configured switch boards
	fixs_ith::ErrorConstants create_bnc_objects(const std::string& switchboard_key = "");
	// create BNC MO for a single switch board
	fixs_ith::ErrorConstants create_bnc_objects(const char * sb_key);
	fixs_ith::ErrorConstants create_bnc_objects(fixs_ith_switchboardinfo & switch_board, OmSession &session);
	fixs_ith::ErrorConstants create_bnc_objects(fixs_ith_switchboardinfo & switch_board, fixs_ith::switchboard_plane_t plane, OmSession &session);
	// delete BNC MO for a single switch board
	fixs_ith::ErrorConstants delete_bnc_objects(const std::string& switchboard_key);


public:
	//===========//
	// Operators //
	//===========//
private:

	fixs_ith::ErrorConstants create_ethernetPortMO (fixs_ith_switchboardinfo & switch_board, OmSession &session);
	fixs_ith::ErrorConstants create_bridgeMO (fixs_ith_switchboardinfo & switch_board, OmSession &session);
	fixs_ith::ErrorConstants create_bridgePortMO (fixs_ith_switchboardinfo & switch_board, OmSession &session);

	fixs_ith::ErrorConstants create_ethernetPortMO (fixs_ith_switchboardinfo & switch_board, fixs_ith::switchboard_plane_t plane, OmSession &session);
	fixs_ith::ErrorConstants create_bridgeMO (fixs_ith_switchboardinfo & switch_board, fixs_ith::switchboard_plane_t plane, OmSession &session);
	fixs_ith::ErrorConstants create_bridgePortMO (fixs_ith_switchboardinfo & switch_board, fixs_ith::switchboard_plane_t plane, OmSession &session);
	fixs_ith::ErrorConstants create_aggregatorMO (fixs_ith_switchboardinfo & switch_board, fixs_ith::switchboard_plane_t plane, OmSession &session);
	fixs_ith::ErrorConstants create_vlanMO (fixs_ith_switchboardinfo & switch_board, fixs_ith::switchboard_plane_t plane, operation::vlanInfo vlanData, OmSession &session);

	fixs_ith::ErrorConstants get_ethernetPortData (operation::ethernetPortInfo & Data, uint16_t  if_index,
			fixs_ith_switchboardinfo & switch_board, fixs_ith::switchboard_plane_t plane);

	fixs_ith::ErrorConstants get_bridgeData (operation::bridgeInfo & Data,
				fixs_ith_switchboardinfo & switch_board, fixs_ith::switchboard_plane_t plane);

	fixs_ith::ErrorConstants get_bridgePortData (operation::bridgePortInfo & Data, uint16_t if_index,
				fixs_ith_switchboardinfo & switch_board, fixs_ith::switchboard_plane_t plane);

	fixs_ith::ErrorConstants get_aggregatorData (operation::aggregatorInfo & ,
					fixs_ith_switchboardinfo & switch_board, fixs_ith::switchboard_plane_t plane);

	fixs_ith::ErrorConstants create_ethernetPort_obj(operation::ethernetPortInfo & ethportData,
			uint32_t sb_magazine, int32_t sb_slot, fixs_ith::switchboard_plane_t plane, OmSession &session);

	fixs_ith::ErrorConstants create_bridge_obj(operation::bridgeInfo & bridgeData,
			uint32_t sb_magazine, int32_t sb_slot, fixs_ith::switchboard_plane_t plane, OmSession &session);

	fixs_ith::ErrorConstants create_bridgePort_obj(operation::bridgePortInfo & bridgeportData,
			uint32_t sb_magazine, int32_t sb_slot, fixs_ith::switchboard_plane_t plane, OmSession &session);

	fixs_ith::ErrorConstants create_aggregator_obj(operation::aggregatorInfo & ,
			uint32_t sb_magazine, int32_t sb_slot, fixs_ith::switchboard_plane_t plane, OmSession &session);

	fixs_ith::ErrorConstants create_vlan_obj(operation::vlanInfo & vlanData,
				uint32_t sb_magazine, int32_t sb_slot, fixs_ith::switchboard_plane_t plane, OmSession &session);

	void define_static_bnc_vlans ();

	void define_static_default_vlans ();

	std::vector<operation::vlanInfo> get_bnc_vlans (fixs_ith_switchboardinfo & switch_board, fixs_ith::switchboard_plane_t plane, std::vector<engine::static_vlan_t> vlan_list);

	bool get_objs_to_remove (std::vector<std::string>& remove_objs, const std::string& switchboard_key, OmSession &session);

	bool immObject_exist(const char * objdn, OmSession & session );

	//========== //
	// utilities //
	//========== //

//	int build_bridge_name(char (& bridge_name)[IMM_BRIDGE_NAME_SIZE_MAX], int32_t slot, uint32_t magazine,  fixs_ith::switchboard_plane_t plane);
//	int build_bridge_rdn(char (& rdn)[IMM_RDN_PATH_SIZE_MAX], int32_t slot, uint32_t magazine,  fixs_ith::switchboard_plane_t plane);
//	int build_port_rdn(char (& rdn)[IMM_RDN_PATH_SIZE_MAX], const char * if_name, int32_t slot, uint32_t magazine,  fixs_ith::switchboard_plane_t plane);
//	int build_aggregator_rdn(char (& rdn)[IMM_RDN_PATH_SIZE_MAX], const char * if_name, int32_t slot, uint32_t magazine,  fixs_ith::switchboard_plane_t plane);
	void macAddrToImmformat(char (& immAddress)[IMM_MACADDRESS_SIZE_MAX], fixs_ith_snmp::mac_address_t  & bridge_addr);
	bool is_valid_ifIndex(int32_t ifIndex, fixs_ith::switchboard_plane_t plane);
//	int build_ethport_dn(char (& ethport_dn)[IMM_DN_PATH_SIZE_MAX], const char * if_name, int32_t slot, uint32_t magazine,  fixs_ith::switchboard_plane_t plane);
//	int build_bridgePort_dn(char (& bridgePort_dn)[IMM_DN_PATH_SIZE_MAX], const char * port_name, int32_t slot, uint32_t magazine,  fixs_ith::switchboard_plane_t plane);
//	int build_vlan_dn(char (& vlna_dn)[IMM_DN_PATH_SIZE_MAX], const char * name, int32_t slot, uint32_t magazine,  fixs_ith::switchboard_plane_t plane);
	// TODO: ELIMINARE
	void deleteAllObjects(OmSession & session );

	fixs_ith::ErrorConstants delete_objects(const std::string& dn, OmSession &session, ACS_APGCC_ScopeT scope = ACS_APGCC_SUBTREE);

	bool is_system_enabled_if(const fixs_ith::magazine_identifier_t magType, const fixs_ith::switchboard_plane_t & plane, const uint16_t& if_index);

	uint16_t get_default_vlan_id(const fixs_ith::switchboard_plane_t & plane);
private:
	//========//
	// Fields //
	//========//

	std::vector<engine::static_vlan_t> _bncVlans;
	std::vector<std::string> _remove_classes;

};


#endif /* ITHADM_CAA_INC_FIXS_ITH_TRANSPORTBNCHANDLER_H_ */
