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

#ifndef ITHADM_CAA_INC_SWITCH_INTERFACE_L2_LAYER_ADAPTER_H_
#define ITHADM_CAA_INC_SWITCH_INTERFACE_L2_LAYER_ADAPTER_H_

#include "switch_interface/L2_layer.h"
#include "fixs_ith_snmpconstants.h"

class fixs_ith_sbdatamanager;
class fixs_ith_switchboardinfo;

namespace switch_interface
{

class L2_layer_adapter: public L2_layer
{
public:
	explicit L2_layer_adapter(fixs_ith_sbdatamanager * sb_datamanager = 0);

	virtual ~L2_layer_adapter() {}

	/**** CREATE operations ***/

	// Create an AGGREGATOR on a BRIDGE
	op_result create_aggregator(const std::string & mag, fixs_ith::BoardSlotPosition pos, fixs_ith::switchboard_plane_t plane, uint16_t if_index, const std::string & aggregator_name);
	op_result create_aggregator(const std::string & switchboard_key, fixs_ith::switchboard_plane_t plane, uint16_t if_index, const std::string & aggregator_name);

	// Create a VLAN on a BRIDGE
	op_result create_vlan(const std::string & mag, fixs_ith::BoardSlotPosition pos, fixs_ith::switchboard_plane_t plane, uint16_t vid, const std::string & vlan_name);
	op_result create_vlan(const std::string & switchboard_key, fixs_ith::switchboard_plane_t plane, uint16_t vid, const std::string & vlan_name);

	// Create an IPv4 subnet on a BRIDGE
	op_result create_ipv4subnet(const std::string & mag, fixs_ith::BoardSlotPosition pos, fixs_ith::switchboard_plane_t plane, const std::string & ipv4subnet);
	op_result create_ipv4subnet(const std::string & switchboard_key, fixs_ith::switchboard_plane_t plane, const std::string & ipv4subnet);


	/**** DELETE operations ****/

	// Delete an AGGREGATOR from a BRIDGE
	op_result delete_aggregator(const std::string & mag, fixs_ith::BoardSlotPosition pos, fixs_ith::switchboard_plane_t plane, uint16_t if_index);
	op_result delete_aggregator(const std::string & /*switchboard_key*/, fixs_ith::switchboard_plane_t /*plane*/, uint16_t /*if_index*/)
	{
		return SBIA_OP_NOT_IMPLEMENTED;
	}

	// Delete a VLAN from a BRIDGE
	op_result delete_vlan(const std::string & mag, fixs_ith::BoardSlotPosition pos, fixs_ith::switchboard_plane_t plane, uint16_t vid);
	op_result delete_vlan(const std::string & switchboard_key, fixs_ith::switchboard_plane_t plane, uint16_t vid);

	// Delete a SUBNET from a BRIDGE
	op_result delete_ipv4subnet(const std::string & mag, fixs_ith::BoardSlotPosition pos, fixs_ith::switchboard_plane_t plane, const std::string & ipv4subnet);
	op_result delete_ipv4subnet(const std::string & switchboard_key, fixs_ith::switchboard_plane_t plane, const std::string & ipv4subnet);


	/**** GET operations ****/

	// Get operative state of a Switchboard INTERFACE
	op_result get_if_operState(IF_oper_state & if_op_state, uint16_t if_index, const std::string & mag, fixs_ith::BoardSlotPosition pos, fixs_ith::switchboard_plane_t plane, const unsigned int * timeout_ms = 0);
	op_result get_if_operState(IF_oper_state & if_op_state, uint16_t if_index, const std::string & switchboard_key, fixs_ith::switchboard_plane_t plane, const unsigned int * timeout_ms = 0);

	op_result get_eth_max_frame_size(uint32_t & eth_max_frame_size, uint16_t if_index, const std::string & mag, fixs_ith::BoardSlotPosition pos, fixs_ith::switchboard_plane_t plane, const unsigned int * timeout_ms = 0);
	op_result get_eth_max_frame_size(uint32_t & eth_max_frame_size, uint16_t if_index, const std::string & switchboard_key, fixs_ith::switchboard_plane_t plane, const unsigned int * timeout_ms = 0);

	op_result get_eth_if_mau_type(std::vector<bool> & if_mau_type, uint16_t if_index, const std::string & mag, fixs_ith::BoardSlotPosition pos, fixs_ith::switchboard_plane_t plane, const unsigned int * timeout_ms = 0);
	op_result get_eth_if_mau_type(std::vector<bool> & if_mau_type, uint16_t if_index, const std::string & switchboard_key, fixs_ith::switchboard_plane_t plane, const unsigned int * timeout_ms = 0);

	//get aggActorAdminKey
	op_result get_agg_actor_admin_key(int32_t & agg_actor_admin_key, uint16_t if_index, const std::string & mag, fixs_ith::BoardSlotPosition pos, fixs_ith::switchboard_plane_t plane, const unsigned int * timeout_ms = 0);
	op_result get_agg_actor_admin_key(int32_t & agg_actor_admin_key, uint16_t if_index, const std::string & switchboard_key, fixs_ith::switchboard_plane_t plane, const unsigned int * timeout_ms = 0);

	// Get operative members of an AGGREGATOR
	op_result  get_aggregator_operMembers(std::set<uint16_t> & if_members, uint16_t if_index, const std::string & mag, fixs_ith::BoardSlotPosition pos, fixs_ith::switchboard_plane_t plane, const unsigned int * timeout_ms = 0);
	op_result  get_aggregator_operMembers(std::set<uint16_t> & if_members, uint16_t if_index, const std::string & switchboard_key, fixs_ith::switchboard_plane_t plane, const unsigned int * timeout_ms = 0);

	// Get number of VLANS defined on a BRIDGE
	op_result get_bridge_numberOfVlans(uint32_t & num_vlans, const std::string & mag, fixs_ith::BoardSlotPosition pos, fixs_ith::switchboard_plane_t plane, const unsigned int * timeout_ms = 0);
	op_result get_bridge_numberOfVlans(uint32_t & num_vlans, const std::string & switchboard_key, fixs_ith::switchboard_plane_t plane, const unsigned int * timeout_ms = 0);

	// Get  current bandwidth of a given interface
	op_result get_if_highSpeed(uint32_t& high_speed, uint16_t if_index, const std::string & mag, fixs_ith::BoardSlotPosition pos, fixs_ith::switchboard_plane_t plane, const unsigned int * timeout_ms = 0);
	op_result get_if_highSpeed(uint32_t& high_speed, uint16_t if_index, const std::string & switchboard_key, fixs_ith::switchboard_plane_t plane, const unsigned int * timeout_ms = 0);

	// Get ports members of a VLAN
	op_result  get_vlan_ports(std::set<uint16_t> & if_ports, uint16_t vid, VLAN_port_type port_type, const std::string & mag, fixs_ith::BoardSlotPosition pos, fixs_ith::switchboard_plane_t plane, const unsigned int * timeout_ms = 0);
	op_result  get_vlan_ports(std::set<uint16_t> & if_ports, uint16_t vid, VLAN_port_type port_type, const std::string & switchboard_key, fixs_ith::switchboard_plane_t plane, const unsigned int * timeout_ms = 0);

	// Get all ports members of a VLAN
	op_result  get_vlan_ports(std::set<uint16_t> & tagged_if_ports, std::set<uint16_t> & untagged_if_ports, uint16_t vid, const std::string & mag, fixs_ith::BoardSlotPosition pos, fixs_ith::switchboard_plane_t plane, const unsigned int * timeout_ms = 0);
	op_result  get_vlan_ports(std::set<uint16_t> & tagged_if_ports, std::set<uint16_t> & untagged_if_ports, uint16_t vid, const std::string & switchboard_key, fixs_ith::switchboard_plane_t plane, const unsigned int * timeout_ms = 0);

	// Get  operating mode of a given interface
	op_result get_if_operOperating_mode(IF_oper_mode & operating_mode, uint16_t if_index, const std::string & mag, fixs_ith::BoardSlotPosition pos, fixs_ith::switchboard_plane_t plane, const unsigned int * timeout_ms = 0);
	op_result get_if_operOperating_mode(IF_oper_mode & operating_mode, uint16_t if_index, const std::string & switchboard_key, fixs_ith::switchboard_plane_t plane, const unsigned int * timeout_ms = 0);

	// Get  auto negotiation state of a given interface
	op_result get_if_auto_negotiation_state(int & auto_negotiation_state, uint16_t if_index, const std::string & mag, fixs_ith::BoardSlotPosition pos, fixs_ith::switchboard_plane_t plane, const unsigned int * timeout_ms = 0);
	op_result get_if_auto_negotiation_state(int & auto_negotiation_state, uint16_t if_index, const std::string & switchboard_key, fixs_ith::switchboard_plane_t plane, const unsigned int * timeout_ms = 0);

	/**** SET operations ****/

	// Set the AdminState of a Switchboard INTERFACE
	op_result set_if_adminState(const std::string & mag, fixs_ith::BoardSlotPosition pos, fixs_ith::switchboard_plane_t plane, uint16_t if_index, BP_admin_state bp_admin_state);
	op_result set_if_adminState(const std::string & switchboard_key, fixs_ith::switchboard_plane_t plane, uint16_t if_index, BP_admin_state bp_admin_state);

	// Set the Priority of CMX Switchboard ETHERNET PORT
	op_result set_if_priority(const std::string & mag, fixs_ith::BoardSlotPosition pos, fixs_ith::switchboard_plane_t plane, uint16_t if_index);
	op_result set_if_priority(const std::string & switchboard_key, fixs_ith::switchboard_plane_t plane, uint16_t if_index);

	// Set the if mau default type of CMX Switchboard INTERFACE
	op_result set_eth_if_mau_type(const std::string & mag, fixs_ith::BoardSlotPosition pos, fixs_ith::switchboard_plane_t plane, uint16_t if_index, fixs_ith_snmp::IfMauTypeListBitsConstants default_type);
	op_result set_eth_if_mau_type(const std::string & switchboard_key, fixs_ith::switchboard_plane_t plane, uint16_t if_index,fixs_ith_snmp::IfMauTypeListBitsConstants default_type);
	
	// Set the MaxFrameSize of a Switchboard ETHERNET PORT
	op_result set_eth_max_frame_size(const std::string & mag, fixs_ith::BoardSlotPosition pos, fixs_ith::switchboard_plane_t plane, uint16_t if_index, uint32_t eth_max_frame_size);
	op_result set_eth_max_frame_size(const std::string & switchboard_key, fixs_ith::switchboard_plane_t plane, uint16_t if_index, uint32_t eth_max_frame_size);

	// Set the auto negotiation of a Switchboard ETHERNET PORT
	op_result set_eth_auto_negotiation(const std::string & mag, fixs_ith::BoardSlotPosition pos, fixs_ith::switchboard_plane_t plane, uint16_t if_index, bool enable_auto_neg);
	op_result set_eth_auto_negotiation(const std::string & switchboard_key, fixs_ith::switchboard_plane_t plane, uint16_t if_index, bool enable_auto_neg);

	// Set the Default VLAN of a Switchboard INTERFACE
	op_result set_port_default_vlan(const std::string & mag, fixs_ith::BoardSlotPosition pos, fixs_ith::switchboard_plane_t plane, uint16_t if_index, uint16_t vid);
	op_result set_port_default_vlan(const std::string & switchboard_key, fixs_ith::switchboard_plane_t plane, uint16_t if_index, uint16_t vid);

	// Set the OAM admin state
	op_result set_eth_oam_adminState(const std::string & mag, fixs_ith::BoardSlotPosition pos, fixs_ith::switchboard_plane_t plane, uint16_t if_index, OAM_admin_state oam_admin_state);
	op_result set_eth_oam_adminState(const std::string & switchboard_key, fixs_ith::switchboard_plane_t plane, uint16_t if_index, OAM_admin_state oam_admin_state);

	// Set the members of an AGGREGATOR on a BRIDGE
	op_result set_aggregator_members(const std::string & mag, fixs_ith::BoardSlotPosition pos, fixs_ith::switchboard_plane_t plane, uint16_t if_index, const std::set<uint16_t> & admin_members);
	op_result set_aggregator_members(const std::string & switchboard_key, fixs_ith::switchboard_plane_t plane, uint16_t if_index, const std::set<uint16_t> & admin_members);

	// Set the members of a VLAN on a BRIDGE
	op_result set_vlan_members(const std::string & mag, fixs_ith::BoardSlotPosition pos, fixs_ith::switchboard_plane_t plane, uint16_t vid, const std::set<uint16_t> & tagged_members, const std::set<uint16_t> & untagged_members);
	op_result set_vlan_members(const std::string & switchboard_key, fixs_ith::switchboard_plane_t plane, uint16_t vid, const std::set<uint16_t> & tagged_members, const std::set<uint16_t> & untagged_members);

	// Set the VLID of a SUBNET defined on a BRIDGE
	op_result set_ipv4subnet_vlanid(const std::string & mag, fixs_ith::BoardSlotPosition pos, fixs_ith::switchboard_plane_t plane, const std::string & ipv4subnet, uint16_t vlanid);
	op_result set_ipv4subnet_vlanid(const std::string & switchboard_key, fixs_ith::switchboard_plane_t plane, const std::string & ipv4subnet, uint16_t vlanid);

	// Enable VRRP new master notification
	op_result enable_vrrp_new_master_notification(const std::string& switchboard_key);

	// Enable BFD session up/down notification
	op_result enable_bfd_session_state_notification(const std::string& switchboard_key);
	// Test board connectivity : SBIA_OK means board up, other value means down
	op_result get_board_status(const std::string& switchboard_key, unsigned int timeout_ms = DEFAULT_SNMP_TIMEOUT_MS);
	op_result get_board_status(fixs_ith_switchboardinfo & switchboard_info, unsigned int timeout_ms = DEFAULT_SNMP_TIMEOUT_MS);

	op_result reset_board(const std::string& switchboard_key);
    op_result reset_board(const std::string& switchboard_key, fixs_ith::board_slotposition_t slot); //TR_HY52528

	op_result disable_rstp(const std::string& switchboard_key, fixs_ith::switchboard_plane_t plane, uint16_t if_index);
	op_result configure_bp_port_as_edgeport(const std::string& switchboard_key, fixs_ith::switchboard_plane_t plane, uint16_t if_index);

private:

	L2_layer_adapter(const L2_layer_adapter &);

	L2_layer_adapter & operator=(const L2_layer_adapter &);

	op_result map_snmpmanager_error(int snmpmanager_error);

	IF_oper_mode if_operOperating_mode(uint32_t if_speed, uint16_t if_duplex_status,uint16_t if_masterslave_status);

	fixs_ith_sbdatamanager * _sb_datamanager;
};

}

#endif /* ITHADM_CAA_INC_SWITCH_INTERFACE_L2_LAYER_ADAPTER_H_ */
