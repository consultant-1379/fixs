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

#ifndef ITHADM_CAA_INC_SWITCH_INTERFACE_L2_LAYER_H_
#define ITHADM_CAA_INC_SWITCH_INTERFACE_L2_LAYER_H_

#include <string>
#include <set>
#include <stdint.h>

#include "commonDefs.h"

namespace switch_interface
{

class L2_layer
{
public:
	L2_layer() {};

	virtual ~L2_layer() { };


	/**** CREATE operations ***/

	// Create an AGGREGATOR on a BRIDGE
	virtual op_result create_aggregator(const std::string & mag, fixs_ith::BoardSlotPosition pos, fixs_ith::switchboard_plane_t plane, uint16_t if_index, const std::string & aggregator_name) = 0;
	virtual op_result create_aggregator(const std::string & switchboard_key, fixs_ith::switchboard_plane_t plane, uint16_t if_index, const std::string & aggregator_name) = 0;

	// Create a VLAN on a BRIDGE
	virtual op_result create_vlan(const std::string & mag, fixs_ith::BoardSlotPosition pos, fixs_ith::switchboard_plane_t plane, uint16_t vid, const std::string & vlan_name) = 0;
	virtual op_result create_vlan(const std::string & switchboard_key, fixs_ith::switchboard_plane_t plane, uint16_t vid, const std::string & vlan_name) = 0;

	// Create an IPv4 subnet on a BRIDGE
	virtual op_result create_ipv4subnet(const std::string & mag, fixs_ith::BoardSlotPosition pos, fixs_ith::switchboard_plane_t plane, const std::string & ipv4subnet) = 0;
	virtual op_result create_ipv4subnet(const std::string & switchboard_key, fixs_ith::switchboard_plane_t plane, const std::string & ipv4subnet) = 0;


	/**** DELETE operations ****/

	// Delete an AGGREGATOR from a BRIDGE
	virtual op_result delete_aggregator(const std::string & mag, fixs_ith::BoardSlotPosition pos, fixs_ith::switchboard_plane_t plane, uint16_t if_index) = 0;
	virtual op_result delete_aggregator(const std::string & /*switchboard_key*/, fixs_ith::switchboard_plane_t /*plane*/, uint16_t /*if_index*/) = 0;

	// Delete a VLAN from a BRIDGE
	virtual op_result delete_vlan(const std::string & mag, fixs_ith::BoardSlotPosition pos, fixs_ith::switchboard_plane_t plane, uint16_t vid) = 0;
	virtual op_result delete_vlan(const std::string & switchboard_key, fixs_ith::switchboard_plane_t plane, uint16_t vid) = 0;

	// Delete a SUBNET from a BRIDGE
	virtual op_result delete_ipv4subnet(const std::string & mag, fixs_ith::BoardSlotPosition pos, fixs_ith::switchboard_plane_t plane, const std::string & ipv4subnet) = 0;
	virtual op_result delete_ipv4subnet(const std::string & switchboard_key, fixs_ith::switchboard_plane_t plane, const std::string & ipv4subnet) = 0;


	/**** GET operations ****/

	// Get operative state of a Switchboard INTERFACE
	virtual op_result get_if_operState(IF_oper_state & if_op_state, uint16_t if_index, const std::string & mag, fixs_ith::BoardSlotPosition pos, fixs_ith::switchboard_plane_t plane, const unsigned int * timeout_ms = 0) = 0;
	virtual op_result get_if_operState(IF_oper_state & if_op_state, uint16_t if_index, const std::string & switchboard_key, fixs_ith::switchboard_plane_t plane, const unsigned int * timeout_ms = 0) = 0;

	virtual op_result get_eth_max_frame_size(uint32_t & eth_max_frame_size, uint16_t if_index, const std::string & mag, fixs_ith::BoardSlotPosition pos, fixs_ith::switchboard_plane_t plane, const unsigned int * timeout_ms = 0) = 0;
	virtual op_result get_eth_max_frame_size(uint32_t & eth_max_frame_size, uint16_t if_index, const std::string & switchboard_key, fixs_ith::switchboard_plane_t plane, const unsigned int * timeout_ms = 0) = 0;

	virtual op_result get_eth_if_mau_type(std::vector<bool> & if_mau_type, uint16_t if_index, const std::string & mag, fixs_ith::BoardSlotPosition pos, fixs_ith::switchboard_plane_t plane, const unsigned int * timeout_ms = 0) = 0;
	virtual op_result get_eth_if_mau_type(std::vector<bool> & if_mau_type, uint16_t if_index, const std::string & switchboard_key, fixs_ith::switchboard_plane_t plane, const unsigned int * timeout_ms = 0) = 0;

	// Get operative members of an AGGREGATOR
	virtual op_result  get_aggregator_operMembers(std::set<uint16_t> & if_members, uint16_t if_index, const std::string & mag, fixs_ith::BoardSlotPosition pos, fixs_ith::switchboard_plane_t plane, const unsigned int * timeout_ms = 0) = 0;
	virtual op_result  get_aggregator_operMembers(std::set<uint16_t> & if_members, uint16_t if_index, const std::string & switchboard_key, fixs_ith::switchboard_plane_t plane, const unsigned int * timeout_ms = 0) = 0;

	// Get number of VLANS defined on a BRIDGE
	virtual op_result get_bridge_numberOfVlans(uint32_t & num_vlans, const std::string & mag, fixs_ith::BoardSlotPosition pos, fixs_ith::switchboard_plane_t plane, const unsigned int * timeout_ms = 0) = 0;
	virtual op_result get_bridge_numberOfVlans(uint32_t & num_vlans, const std::string & switchboard_key, fixs_ith::switchboard_plane_t plane, const unsigned int * timeout_ms = 0) = 0;

	// Get operative state of a Switchboard INTERFACE
	virtual op_result get_if_highSpeed(uint32_t& high_speed, uint16_t if_index, const std::string & mag, fixs_ith::BoardSlotPosition pos, fixs_ith::switchboard_plane_t plane, const unsigned int * timeout_ms = 0) = 0;
	virtual op_result get_if_highSpeed(uint32_t& high_speed, uint16_t if_index, const std::string & switchboard_key, fixs_ith::switchboard_plane_t plane, const unsigned int * timeout_ms = 0) = 0;

	// Get  operating mode of a a Switchboard INTERFACE
	virtual op_result get_if_operOperating_mode(IF_oper_mode & operating_mode, uint16_t if_index, const std::string & mag, fixs_ith::BoardSlotPosition pos, fixs_ith::switchboard_plane_t plane, const unsigned int * timeout_ms = 0) = 0;
	virtual op_result get_if_operOperating_mode(IF_oper_mode & operating_mode, uint16_t if_index, const std::string & switchboard_key, fixs_ith::switchboard_plane_t plane, const unsigned int * timeout_ms = 0) = 0;

	// Get  operating mode of a a Switchboard INTERFACE
	virtual op_result get_if_auto_negotiation_state(int & auto_negotiation_state, uint16_t if_index, const std::string & mag, fixs_ith::BoardSlotPosition pos, fixs_ith::switchboard_plane_t plane, const unsigned int * timeout_ms = 0) = 0;
	virtual op_result get_if_auto_negotiation_state(int & auto_negotiation_state, uint16_t if_index, const std::string & switchboard_key, fixs_ith::switchboard_plane_t plane, const unsigned int * timeout_ms = 0) = 0;

	// Get ports members of a VLAN
	virtual op_result  get_vlan_ports(std::set<uint16_t> & if_ports, uint16_t vid, VLAN_port_type port_type,const std::string & mag, fixs_ith::BoardSlotPosition pos, fixs_ith::switchboard_plane_t plane, const unsigned int * timeout_ms = 0) = 0;
	virtual op_result  get_vlan_ports(std::set<uint16_t> & if_ports, uint16_t vid, VLAN_port_type port_type, const std::string & switchboard_key, fixs_ith::switchboard_plane_t plane, const unsigned int * timeout_ms = 0) = 0;


	/**** SET operations ****/

	// Set the AdminState of a Switchboard INTERFACE
	virtual op_result set_if_adminState(const std::string & mag, fixs_ith::BoardSlotPosition pos, fixs_ith::switchboard_plane_t plane, uint16_t if_index, BP_admin_state bp_admin_state) = 0;
	virtual op_result set_if_adminState(const std::string & switchboard_key, fixs_ith::switchboard_plane_t plane, uint16_t if_index, BP_admin_state bp_admin_state) = 0;

	// Set the MaxFrameSize of a Switchboard ETHERNET PORT
	virtual op_result set_eth_max_frame_size(const std::string & mag, fixs_ith::BoardSlotPosition pos, fixs_ith::switchboard_plane_t plane, uint16_t if_index, uint32_t eth_max_frame_size) = 0;
	virtual op_result set_eth_max_frame_size(const std::string & switchboard_key, fixs_ith::switchboard_plane_t plane, uint16_t if_index, uint32_t eth_max_frame_size) = 0;

	// Set the auto negotiation of a Switchboard ETHERNET PORT
	virtual op_result set_eth_auto_negotiation(const std::string & mag, fixs_ith::BoardSlotPosition pos, fixs_ith::switchboard_plane_t plane, uint16_t if_index, bool enable_auto_neg) = 0;
	virtual op_result set_eth_auto_negotiation(const std::string & switchboard_key, fixs_ith::switchboard_plane_t plane, uint16_t if_index, bool enable_auto_neg) = 0;

	// Set the Default VLAN of a Switchboard INTERFACE
	virtual op_result set_port_default_vlan(const std::string & mag, fixs_ith::BoardSlotPosition pos, fixs_ith::switchboard_plane_t plane, uint16_t if_index, uint16_t vid) = 0;
	virtual op_result set_port_default_vlan(const std::string & switchboard_key, fixs_ith::switchboard_plane_t plane, uint16_t if_index, uint16_t vid) = 0;

	// Set the members of an AGGREGATOR on a BRIDGE
	virtual op_result set_aggregator_members(const std::string & mag, fixs_ith::BoardSlotPosition pos, fixs_ith::switchboard_plane_t plane, uint16_t if_index, const std::set<uint16_t> & admin_members) = 0;
	virtual op_result set_aggregator_members(const std::string & switchboard_key, fixs_ith::switchboard_plane_t plane, uint16_t if_index, const std::set<uint16_t> & admin_members) = 0;

	// Set the members of a VLAN on a BRIDGE
	virtual op_result set_vlan_members(const std::string & mag, fixs_ith::BoardSlotPosition pos, fixs_ith::switchboard_plane_t plane, uint16_t vid, const std::set<uint16_t> & tagged_members, const std::set<uint16_t> & untagged_members) = 0;
	virtual op_result set_vlan_members(const std::string & switchboard_key, fixs_ith::switchboard_plane_t plane, uint16_t vid, const std::set<uint16_t> & tagged_members, const std::set<uint16_t> & untagged_members) = 0;

	// Set the VLID of a SUBNET defined on a BRIDGE
	virtual op_result set_ipv4subnet_vlanid(const std::string & mag, fixs_ith::BoardSlotPosition pos, fixs_ith::switchboard_plane_t plane, const std::string & ipv4subnet, uint16_t vlanid) = 0;
	virtual op_result set_ipv4subnet_vlanid(const std::string & switchboard_key, fixs_ith::switchboard_plane_t plane, const std::string & ipv4subnet, uint16_t vlanid) = 0;
};

}

#endif /* ITHADM_CAA_INC_SWITCH_INTERFACE_L2_LAYER_H_ */
