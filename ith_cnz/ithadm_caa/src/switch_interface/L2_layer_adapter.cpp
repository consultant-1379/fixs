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

#include  "switch_interface/L2_layer_adapter.h"

#include <map>
#include <string>
#include <bitset>
#include <set>

#include <ace/RW_Thread_Mutex.h>
#include <ace/Guard_T.h>

#include "fixs_ith_csreader.h"
#include "fixs_ith_logger.h"
#include "fixs_ith_programmacros.h"
#include "fixs_ith_workingset.h"
#include "fixs_ith_sbdatamanager.h"

using namespace switch_interface::adapters_utils;

namespace switch_interface
{

L2_layer_adapter::L2_layer_adapter(fixs_ith_sbdatamanager * sb_datamanager):
L2_layer()
{
	// if switchboards data manager has not been provided, use the instance defined in the ITH working set
	_sb_datamanager = (sb_datamanager ? sb_datamanager: & fixs_ith::workingSet_t::instance()->get_sbdatamanager());
}

op_result L2_layer_adapter::create_aggregator(const std::string & mag_str, fixs_ith::BoardSlotPosition pos, fixs_ith::switchboard_plane_t plane, uint16_t if_index, const std::string & aggregator_name)
{
	std::string sb_key;
	build_sb_key_from_magazine_and_slot(sb_key, mag_str, pos);
	return create_aggregator(sb_key, plane, if_index, aggregator_name);
}

op_result L2_layer_adapter::create_vlan(const std::string & mag, fixs_ith::BoardSlotPosition pos, fixs_ith::switchboard_plane_t plane, uint16_t vid, const std::string & vlan_name)
{
	std::string sb_key;
	build_sb_key_from_magazine_and_slot(sb_key, mag, pos);
	return create_vlan(sb_key, plane, vid, vlan_name);
}

op_result L2_layer_adapter::create_ipv4subnet(const std::string & mag, fixs_ith::BoardSlotPosition pos, fixs_ith::switchboard_plane_t plane, const std::string & ipv4subnet)
{
	std::string sb_key;
	build_sb_key_from_magazine_and_slot(sb_key, mag, pos);
	return create_ipv4subnet(sb_key, plane, ipv4subnet);
}

op_result L2_layer_adapter::delete_aggregator(const std::string & mag_str, fixs_ith::BoardSlotPosition pos, fixs_ith::switchboard_plane_t plane, uint16_t if_index)
{
	std::string sb_key;
	build_sb_key_from_magazine_and_slot(sb_key, mag_str, pos);
	return delete_aggregator(sb_key, plane, if_index);
}

op_result L2_layer_adapter::delete_vlan(const std::string & mag, fixs_ith::BoardSlotPosition pos, fixs_ith::switchboard_plane_t plane, uint16_t vid)
{
	std::string sb_key;
	build_sb_key_from_magazine_and_slot(sb_key, mag, pos);
	return delete_vlan(sb_key, plane, vid);
}

op_result L2_layer_adapter::delete_ipv4subnet(const std::string & mag, fixs_ith::BoardSlotPosition pos, fixs_ith::switchboard_plane_t plane, const std::string & ipv4subnet)
{
	std::string sb_key;
	build_sb_key_from_magazine_and_slot(sb_key, mag, pos);
	return delete_ipv4subnet(sb_key, plane, ipv4subnet);
}


op_result L2_layer_adapter::create_aggregator(const std::string & switchboard_key, fixs_ith::switchboard_plane_t plane, uint16_t if_index, const std::string & aggregator_name)
{
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Request to CREATE an AGGREGATOR with IF_INDEX '%u' on the '%s' plane of the SWITCH BOARD having key '%s'",
			if_index, plane_descr(plane), switchboard_key.c_str());

	// retrieve the switch board hosting the target BRIDGE
	fixs_ith_switchboardinfo switchboard_info;
	fixs_ith_sbdatamanager::sbm_constant call_result = _sb_datamanager->get_switchboard_info(switchboard_info, switchboard_key);
	if(call_result != fixs_ith_sbdatamanager::SBM_OK)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to to CREATE an AGGREGATOR with IF_INDEX '%u' on the '%s' plane of the SWITCH BOARD having key '%s'. "
				"[Call 'fixs_ith_sbdatamanager::get_switchboard_info()' returned '%d']",
				if_index, plane_descr(plane), switchboard_key.c_str(), call_result);

		return SBIA_SWITCHBOARD_INFO_NOT_AVAILABLE;
	}

	// now use SNMP Manager to create the LAG

	// 1) first check if the LAG is already defined on the BRIDGE
	unsigned timeout_ms = 2000;
	int check_lag_res = fixs_ith::workingSet_t::instance()->get_snmpmanager().check_lag(switchboard_info, plane, if_index, &timeout_ms);
	if(check_lag_res == fixs_ith_snmp::ERR_SNMP_NO_ERRORS)
	{
		// LAG already defined on the BRIDGE
		FIXS_ITH_LOG(LOG_LEVEL_WARN, "a LAG having { IF_INDEX = '%u'} ALREADY EXIST on the '%s' plane of the SWITCH BOARD having key '%s'. ",
					if_index, plane_descr(plane), switchboard_key.c_str());

		return SBIA_OBJ_ALREADY_EXIST;
	}

	// 2) the LAG is not defined on the BRIDGE, or existence check failed. In either case, try to create the LAG
	int create_lag_res = fixs_ith::workingSet_t::instance()->get_snmpmanager().create_lag(switchboard_info, if_index, aggregator_name.c_str(), plane);
	if(create_lag_res < 0)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to to CREATE an AGGREGATOR with IF_INDEX '%u' on the '%s' plane of the SWITCH BOARD having key '%s'. "
						"[Call 'fixs_ith_snmpmanager::create_lag()' returned '%d']",
						if_index, plane_descr(plane), switchboard_key.c_str(), create_lag_res);

		return map_snmpmanager_error(create_lag_res);
	}

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Successfully created the AGGREGATOR with IF_INDEX '%u' on the '%s' plane of the SWITCH BOARD having key '%s'",
				if_index, plane_descr(plane), switchboard_key.c_str());

	return SBIA_OK;
}

op_result L2_layer_adapter::create_vlan(const std::string & switchboard_key, fixs_ith::switchboard_plane_t plane, uint16_t vid, const std::string & vlan_name)
{
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Request to CREATE the VLAN { VID = '%u', VLAN_NAME = '%s'} on the '%s' plane of the SWITCH BOARD having key '%s'",
				vid, vlan_name.c_str(), plane_descr(plane), switchboard_key.c_str());

	// retrieve the switch board hosting the target BRIDGE
	fixs_ith_switchboardinfo switchboard_info;
	fixs_ith_sbdatamanager::sbm_constant call_result = _sb_datamanager->get_switchboard_info(switchboard_info, switchboard_key);
	if(call_result != fixs_ith_sbdatamanager::SBM_OK)
	{
		FIXS_ITH_LOG(LOG_LEVEL_WARN, "Unable to CREATE the VLAN { VID = '%u', VLAN_NAME = '%s'} on the '%s' plane of the SWITCH BOARD having key '%s'. "
				"[Call 'fixs_ith_sbdatamanager::get_switchboard_info()' returned '%d' ]",
				vid, vlan_name.c_str(), plane_descr(plane), switchboard_key.c_str(), call_result);

		return SBIA_SWITCHBOARD_INFO_NOT_AVAILABLE;
	}

	// now use SNMP Manager to create the VLAN

	// 1) first check if the VLAN is already defined on the BRIDGE
	unsigned timeout_ms = 2000;
	int check_vlan_res = fixs_ith::workingSet_t::instance()->get_snmpmanager().check_vlan(switchboard_info, plane, vid, &timeout_ms);
	if(check_vlan_res == fixs_ith_snmp::ERR_SNMP_NO_ERRORS)
	{
		// VLAN already defined on the BRIDGE
		FIXS_ITH_LOG(LOG_LEVEL_WARN, "The VLAN { VID = '%u', VLAN_NAME = '%s'} ALREADY EXIST on the '%s' plane of the SWITCH BOARD having key '%s'. ",
						vid, vlan_name.c_str(), plane_descr(plane), switchboard_key.c_str());

		return SBIA_OBJ_ALREADY_EXIST;
	}

	// 2) the VLAN doesn't exist on the BRIDGE or we were not able to check the existence due to an error. In either case,
	// try to create the VLAN
	int create_vlan_res = fixs_ith::workingSet_t::instance()->get_snmpmanager().create_vlan(switchboard_info, vid, vlan_name.c_str(), plane);
	if(create_vlan_res < 0)
	{
		FIXS_ITH_LOG(LOG_LEVEL_WARN, "Unable to CREATE the VLAN { VID = '%u', VLAN_NAME = '%s'} on the '%s' plane of the SWITCH BOARD having key '%s'. "
				"[Call 'fixs_ith_snmpmanager::create_vlan()' returned '%d'",
				vid, vlan_name.c_str(), plane_descr(plane), switchboard_key.c_str(), create_vlan_res);

		return map_snmpmanager_error(create_vlan_res);
	}

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Successfully created the VLAN { VID = '%u', VLAN_NAME = '%s'} on the '%s' plane of the SWITCH BOARD having key '%s'",
				vid, vlan_name.c_str(), plane_descr(plane), switchboard_key.c_str());

	return SBIA_OK;
}


op_result L2_layer_adapter::create_ipv4subnet(const std::string & switchboard_key, fixs_ith::switchboard_plane_t plane, const std::string & ipv4subnet)
{
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Request to CREATE the IPv4 SUBNET '%s' on the '%s' plane of the SWITCH BOARD having key '%s'",
				ipv4subnet.c_str(), plane_descr(plane), switchboard_key.c_str());

	// retrieve the switch board hosting the target BRIDGE
	fixs_ith_switchboardinfo switchboard_info;
	fixs_ith_sbdatamanager::sbm_constant call_result = _sb_datamanager->get_switchboard_info(switchboard_info, switchboard_key);
	if(call_result != fixs_ith_sbdatamanager::SBM_OK)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to CREATE the IPv4 SUBNET '%s' on the '%s' plane of the SWITCH BOARD having key '%s'. "
				"[Call 'fixs_ith_sbdatamanager::get_switchboard_info()' returned '%d' ]",
				ipv4subnet.c_str(), plane_descr(plane), switchboard_key.c_str(), call_result);

		return SBIA_SWITCHBOARD_INFO_NOT_AVAILABLE;
	}

	// now use SNMP Manager to create the SUBNET

	// 1) first check if the SUBNET is already defined on the BRIDGE
	unsigned timeout_ms = 2000;
	int check_vlan_res = fixs_ith::workingSet_t::instance()->get_snmpmanager().check_ipv4subnet(switchboard_info, plane, ipv4subnet, &timeout_ms);
	if(check_vlan_res == fixs_ith_snmp::ERR_SNMP_NO_ERRORS)
	{
		// SUBNET already defined on the BRIDGE
		FIXS_ITH_LOG(LOG_LEVEL_WARN, "The IPv4 SUBNET '%s' ALREADY EXIST on the '%s' plane of the SWITCH BOARD having key '%s'. ",
						ipv4subnet.c_str(), plane_descr(plane), switchboard_key.c_str());

		return SBIA_OBJ_ALREADY_EXIST;
	}


	// 2) the IPv4 SUBNET doesn't exist on the BRIDGE or we were not able to check the existence due to an error. In either case,
	// try to create the VLAN
	int create_ipv4subnet_res = fixs_ith::workingSet_t::instance()->get_snmpmanager().create_ipv4subnet(switchboard_info, plane, ipv4subnet);
	if(create_ipv4subnet_res < 0)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to CREATE the IPv4 SUBNET '%s' on the '%s' plane of the SWITCH BOARD having key '%s'. "
				"[Call 'fixs_ith_sbdatamanager::create_ipv4subnet()' returned '%d' ]",
				ipv4subnet.c_str(), plane_descr(plane), switchboard_key.c_str(), create_ipv4subnet_res);

		return map_snmpmanager_error(create_ipv4subnet_res);
	}

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Successfully created the IPv4 SUBNET '%s' on the '%s' plane of the SWITCH BOARD having key '%s'.",
				ipv4subnet.c_str(), plane_descr(plane), switchboard_key.c_str());

	return SBIA_OK;
}


op_result L2_layer_adapter::delete_vlan(const std::string & switchboard_key, fixs_ith::switchboard_plane_t plane, uint16_t vid)
{
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Request to DELETE the VLAN { VID = '%u'} on the '%s' plane of the SWITCH BOARD having key '%s'",
					vid, plane_descr(plane), switchboard_key.c_str());

	// retrieve the switch board hosting the target BRIDGE
	fixs_ith_switchboardinfo switchboard_info;
	fixs_ith_sbdatamanager::sbm_constant call_result = _sb_datamanager->get_switchboard_info(switchboard_info, switchboard_key);
	if(call_result != fixs_ith_sbdatamanager::SBM_OK)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to DELETE the VLAN { VID = '%u'} on the '%s' plane of the SWITCH BOARD having key '%s'. "
				"[Call 'fixs_ith_sbdatamanager::get_switchboard_info()' returned '%d']",
				vid, plane_descr(plane), switchboard_key.c_str(), call_result);

		return SBIA_SWITCHBOARD_INFO_NOT_AVAILABLE;
	}

	// now use SNMP Manager to delete the VLAN
	int delete_vlan_res = fixs_ith::workingSet_t::instance()->get_snmpmanager().delete_vlan(switchboard_info, vid, plane);
	if(delete_vlan_res < 0)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to DELETE the VLAN { VID = '%u'} on the '%s' plane of the SWITCH BOARD having key '%s'. "
						"[Call 'fixs_ith_snmpmanager::delete_vlan()' returned '%d']",
						vid, plane_descr(plane), switchboard_key.c_str(), delete_vlan_res);

		return map_snmpmanager_error(delete_vlan_res);
	}

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Successfully deleted the VLAN { VID = '%u'} on the '%s' plane of the SWITCH BOARD having key '%s'",
				vid, plane_descr(plane), switchboard_key.c_str());

	return SBIA_OK;
}


op_result L2_layer_adapter::delete_ipv4subnet(const std::string & switchboard_key, fixs_ith::switchboard_plane_t plane, const std::string & ipv4subnet)
{
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Request to DELETE IPV4 Subnet { ipv4 subnet = '%s'} on the '%s' plane of the SWITCH BOARD having key '%s'",
					ipv4subnet.c_str(), plane_descr(plane), switchboard_key.c_str());

	// retrieve the switch board hosting the target BRIDGE
	fixs_ith_switchboardinfo switchboard_info;
	fixs_ith_sbdatamanager::sbm_constant call_result = _sb_datamanager->get_switchboard_info(switchboard_info, switchboard_key);
	if(call_result != fixs_ith_sbdatamanager::SBM_OK)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to DELETE IPV4 subnet { ipv4 subnet = '%s' } on the '%s' plane of the SWITCH BOARD having key '%s'. "
				"[Call 'fixs_ith_sbdatamanager::get_switchboard_info()' returned '%d']",
				ipv4subnet.c_str(), plane_descr(plane), switchboard_key.c_str(), call_result);

		return SBIA_SWITCHBOARD_INFO_NOT_AVAILABLE;
	}

	// now use SNMP Manager to delete the VLAN
	int delete_subnet_res = fixs_ith::workingSet_t::instance()->get_snmpmanager().delete_ipv4subnet(switchboard_info,plane , ipv4subnet);
	if(delete_subnet_res < 0)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to DELETE IPV4 Subnet { ipv4 subnet = '%s'} on the '%s' plane of the SWITCH BOARD having key '%s'. "
						"[Call 'fixs_ith_snmpmanager::delete_vlan()' returned '%d']",
						ipv4subnet.c_str(), plane_descr(plane), switchboard_key.c_str(), delete_subnet_res);

		return map_snmpmanager_error(delete_subnet_res);
	}

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Successfully deleted IPV4 Subnet { ipv4 subnet = '%s'} on the '%s' plane of the SWITCH BOARD having key '%s'",
			ipv4subnet.c_str(), plane_descr(plane), switchboard_key.c_str());

	return SBIA_OK;
}


op_result L2_layer_adapter::get_if_operState(IF_oper_state & if_op_state, uint16_t if_index, const std::string & mag, fixs_ith::BoardSlotPosition pos, fixs_ith::switchboard_plane_t plane, const unsigned int * timeout_ms)
{
	std::string sb_key;
	build_sb_key_from_magazine_and_slot(sb_key, mag, pos);
	return get_if_operState(if_op_state, if_index, sb_key, plane, timeout_ms);
}


op_result L2_layer_adapter::get_if_operState(IF_oper_state & if_op_state, uint16_t if_index, const std::string & switchboard_key, fixs_ith::switchboard_plane_t plane, const unsigned int * timeout_ms)
{
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Request to GET operative state of IF_INDEX '%d' on the '%s' plane of the SWITCH BOARD having key '%s'",
					if_index, plane_descr(plane), switchboard_key.c_str());

	// retrieve the switch board hosting the target BRIDGE
	fixs_ith_switchboardinfo switchboard_info;
	fixs_ith_sbdatamanager::sbm_constant call_result = _sb_datamanager->get_switchboard_info(switchboard_info, switchboard_key);
	if(call_result != fixs_ith_sbdatamanager::SBM_OK)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to GET operative state of IF_INDEX '%u' on the '%s' plane of the SWITCH BOARD having key '%s'. "
				"[Call 'fixs_ith_sbdatamanager::get_switchboard_info()' returned '%d']",
				if_index, plane_descr(plane), switchboard_key.c_str(), call_result);

		return SBIA_SWITCHBOARD_INFO_NOT_AVAILABLE;
	}

	// now use SNMP Manager to get the operative state of the interface
	fixs_ith_snmp::port_status_t port_status = fixs_ith_snmp::PORT_STATUS_DOWN;
	int get_port_op_status_res = fixs_ith::workingSet_t::instance()->get_snmpmanager().get_port_operational_status(port_status, if_index, switchboard_info, plane, timeout_ms);
	if(get_port_op_status_res < 0)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to GET operative state of IF_INDEX '%u' on the '%s' plane of the SWITCH BOARD having key '%s'. "
						"[Call 'fixs_ith_snmpmanager::get_port_operational_status()' returned '%d']",
						if_index, plane_descr(plane), switchboard_key.c_str(), get_port_op_status_res);

		return map_snmpmanager_error(get_port_op_status_res);
	}

	// Fill output parameter
	switch(port_status)
	{
	case fixs_ith_snmp::PORT_STATUS_UP:
		if_op_state = IF_ENABLED;
		break;
	case fixs_ith_snmp::PORT_STATUS_DOWN:
		if_op_state = IF_DISABLED;
		break;
	default:
		if_op_state = IF_STATE_UNKNOWN;
	}

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Successfully GOT the operative state of IF_INDEX '%u' on the '%s' plane of "
				"the SWITCH BOARD having key '%s'. {STATE  == '%s'} ",
					if_index, plane_descr(plane), switchboard_key.c_str(), if_operativeState_descr(if_op_state));

	return SBIA_OK;
}

op_result L2_layer_adapter::get_if_highSpeed(uint32_t& high_speed, uint16_t if_index, const std::string & mag, fixs_ith::BoardSlotPosition pos, fixs_ith::switchboard_plane_t plane, const unsigned int * timeout_ms)
{
	std::string sb_key;
	build_sb_key_from_magazine_and_slot(sb_key, mag, pos);
	return get_if_highSpeed(high_speed, if_index, sb_key, plane, timeout_ms);
}


op_result L2_layer_adapter::get_if_highSpeed(uint32_t& high_speed, uint16_t if_index, const std::string & switchboard_key, fixs_ith::switchboard_plane_t plane, const unsigned int * timeout_ms)
{
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Request to GET high speed of IF_INDEX '%d' on the '%s' plane of the SWITCH BOARD having key '%s'",
					if_index, plane_descr(plane), switchboard_key.c_str());

	// retrieve the switch board hosting the target BRIDGE
	fixs_ith_switchboardinfo switchboard_info;
	fixs_ith_sbdatamanager::sbm_constant call_result = _sb_datamanager->get_switchboard_info(switchboard_info, switchboard_key);
	if(call_result != fixs_ith_sbdatamanager::SBM_OK)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to GET high speed of IF_INDEX '%u' on the '%s' plane of the SWITCH BOARD having key '%s'. "
				"[Call 'fixs_ith_sbdatamanager::get_switchboard_info()' returned '%d']",
				if_index, plane_descr(plane), switchboard_key.c_str(), call_result);

		return SBIA_SWITCHBOARD_INFO_NOT_AVAILABLE;
	}

	// now use SNMP Manager to get the speed of the interface
	int get_interface_speed_res = fixs_ith::workingSet_t::instance()->get_snmpmanager().get_interface_speed(high_speed, if_index, switchboard_info, plane, timeout_ms);
	if(get_interface_speed_res < 0)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to GET high speed of IF_INDEX '%u' on the '%s' plane of the SWITCH BOARD having key '%s'. "
						"[Call 'fixs_ith_snmpmanager::get_port_operational_status()' returned '%d']",
						if_index, plane_descr(plane), switchboard_key.c_str(), get_interface_speed_res);

		return map_snmpmanager_error(get_interface_speed_res);
	}

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Successfully GOT the operative state of IF_INDEX '%u' on the '%s' plane of "
				"the SWITCH BOARD having key '%s'. {SPEED  == '%d'} ",
					if_index, plane_descr(plane), switchboard_key.c_str(), high_speed);

	return SBIA_OK;
}


op_result L2_layer_adapter::get_eth_max_frame_size(uint32_t & eth_max_frame_size, uint16_t if_index, const std::string & mag, fixs_ith::BoardSlotPosition pos, fixs_ith::switchboard_plane_t plane, const unsigned int * timeout_ms)
{
	std::string sb_key;
	build_sb_key_from_magazine_and_slot(sb_key, mag, pos);
	return get_eth_max_frame_size(eth_max_frame_size, if_index, sb_key, plane, timeout_ms);
}

op_result L2_layer_adapter::get_eth_max_frame_size(uint32_t & eth_max_frame_size, uint16_t if_index, const std::string & switchboard_key, fixs_ith::switchboard_plane_t plane, const unsigned int * timeout_ms )
{
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Request to GET max frame size of IF_INDEX '%d' on the '%s' plane of the SWITCH BOARD having key '%s'",
					if_index, plane_descr(plane), switchboard_key.c_str());

	// retrieve the switch board hosting the target BRIDGE
	fixs_ith_switchboardinfo switchboard_info;
	fixs_ith_sbdatamanager::sbm_constant call_result = _sb_datamanager->get_switchboard_info(switchboard_info, switchboard_key);
	if(call_result != fixs_ith_sbdatamanager::SBM_OK)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to GET max frame size of the specified interface. [Call 'fixs_ith_sbdatamanager::get_switchboard_info()' returned '%d' ]", call_result);
		return SBIA_SWITCHBOARD_INFO_NOT_AVAILABLE;
	}

	// now use SNMP Manager to get the operative state of the interface
	int get_eth_max_frame_size_res = fixs_ith::workingSet_t::instance()->get_snmpmanager().get_eth_max_frame_size(eth_max_frame_size, if_index, switchboard_info, plane, timeout_ms);
	if(get_eth_max_frame_size_res < 0)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to get max frame size of the specified interface via SNMP Manager."
				" [Call 'fixs_ith_snmpmager::get_eth_max_frame_size()' returned %", get_eth_max_frame_size_res);
		return map_snmpmanager_error(get_eth_max_frame_size_res);
	}

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Successfully GOT max frame size e of IF_INDEX '%d' on the '%s' plane of "
				"the SWITCH BOARD having key '%s'. {MAX FRAME SIZE  == '%d'} ",
					if_index, plane_descr(plane), switchboard_key.c_str(), eth_max_frame_size);

	return SBIA_OK;
}


op_result L2_layer_adapter::get_eth_if_mau_type(std::vector<bool> & if_mau_type, uint16_t if_index, const std::string & mag, fixs_ith::BoardSlotPosition pos, fixs_ith::switchboard_plane_t plane, const unsigned int * timeout_ms)
{
	std::string sb_key;
	build_sb_key_from_magazine_and_slot(sb_key, mag, pos);
	return get_eth_if_mau_type(if_mau_type, if_index, sb_key, plane, timeout_ms);
}

op_result L2_layer_adapter::get_eth_if_mau_type(std::vector<bool> & if_mau_type, uint16_t if_index, const std::string & switchboard_key, fixs_ith::switchboard_plane_t plane, const unsigned int * timeout_ms )
{
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Request to GET if mau type of IF_INDEX '%d' on the '%s' plane of the SWITCH BOARD having key '%s'",
			if_index, plane_descr(plane), switchboard_key.c_str());

	// retrieve the switch board hosting the target BRIDGE
	fixs_ith_switchboardinfo switchboard_info;
	fixs_ith_sbdatamanager::sbm_constant call_result = _sb_datamanager->get_switchboard_info(switchboard_info, switchboard_key);
	if(call_result != fixs_ith_sbdatamanager::SBM_OK)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to GET if mau type of the specified interface. [Call 'fixs_ith_sbdatamanager::get_switchboard_info()' returned '%d' ]", call_result);
		return SBIA_SWITCHBOARD_INFO_NOT_AVAILABLE;
	}

	// now use SNMP Manager to get the if mau type of the interface
	int get_eth_if_mau_type_res = fixs_ith::workingSet_t::instance()->get_snmpmanager().get_if_mau_type(if_mau_type, if_index, switchboard_info, plane, timeout_ms);
	if(get_eth_if_mau_type_res < 0)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to get if mau type of the specified interface via SNMP Manager."
				" [Call 'fixs_ith_snmpmager::get_if_mau_type()' returned %", get_eth_if_mau_type_res);
		return map_snmpmanager_error(get_eth_if_mau_type_res);
	}

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Successfully GOT if mau type of IF_INDEX '%d' on the '%s' plane of the SWITCH BOARD having key '%s'",
			if_index, plane_descr(plane), switchboard_key.c_str());

	return SBIA_OK;
}


op_result L2_layer_adapter::get_if_operOperating_mode(IF_oper_mode & operating_mode, uint16_t if_index, const std::string & mag, fixs_ith::BoardSlotPosition pos, fixs_ith::switchboard_plane_t plane, const unsigned int * timeout_ms)
{
	std::string sb_key;
	build_sb_key_from_magazine_and_slot(sb_key, mag, pos);
	return get_if_operOperating_mode(operating_mode, if_index, sb_key, plane, timeout_ms);
}

op_result L2_layer_adapter::get_if_operOperating_mode(IF_oper_mode & operating_mode, uint16_t if_index, const std::string & switchboard_key, fixs_ith::switchboard_plane_t plane, const unsigned int * timeout_ms )
{
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Request to GET operOperatingMode of IF_INDEX '%d' on the '%s' plane of the SWITCH BOARD having key '%s'",
					if_index, plane_descr(plane), switchboard_key.c_str());

	// retrieve the switch board hosting the target BRIDGE
	fixs_ith_switchboardinfo switchboard_info;
	fixs_ith_sbdatamanager::sbm_constant call_result = _sb_datamanager->get_switchboard_info(switchboard_info, switchboard_key);
	if(call_result != fixs_ith_sbdatamanager::SBM_OK)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to GET operOperatingMode of the specified interface. [Call 'fixs_ith_sbdatamanager::get_switchboard_info()' returned '%d' ]", call_result);
		return SBIA_SWITCHBOARD_INFO_NOT_AVAILABLE;
	}

	//step 1. use SNMP Manager to get the speed of the interface
	uint32_t if_speed;
	int operation_res = fixs_ith::workingSet_t::instance()->get_snmpmanager().get_interface_speed(if_speed, if_index, switchboard_info, plane, timeout_ms);
	if(operation_res < 0)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to get interface speed of the specified interface via SNMP Manager."
				" [Call 'fixs_ith_snmpmager::get_interface_speed()' returned %", operation_res);
		return map_snmpmanager_error(operation_res);
	}

	//step 2. use SNMP Manager to get the duplex state of the interface
	uint16_t if_duplex_status;
	operation_res = fixs_ith::workingSet_t::instance()->get_snmpmanager().get_eth_duplex_status(if_duplex_status, if_index, switchboard_info, plane, timeout_ms);
	if(operation_res < 0)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to get interface duplex status of the specified interface via SNMP Manager."
				" [Call 'fixs_ith_snmpmager::get_eth_duplex_status()' returned %", operation_res);
		return map_snmpmanager_error(operation_res);
	}

	//step 3. use SNMP Manager to get the master/slave state of the interface
	uint16_t if_masterslave_status;
	operation_res = fixs_ith::workingSet_t::instance()->get_snmpmanager().get_phy_masterslave_status(if_masterslave_status, if_index, switchboard_info, plane, timeout_ms);
	if(operation_res < 0)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to get interface master/slave status of the specified interface via SNMP Manager."
				" [Call 'fixs_ith_snmpmager::get_phy_masterslave_status()' returned %", operation_res);
		return map_snmpmanager_error(operation_res);
	}

	//now calculate the value of operating mode from the attribute's value at steps 1->3

	operating_mode = if_operOperating_mode(if_speed, if_duplex_status, if_masterslave_status);

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Successfully GOT operOperatingMode of IF_INDEX '%d' on the '%s' plane of "
				"the SWITCH BOARD having key '%s'. {operOperatingMode  == '%u'} ",
					if_index, plane_descr(plane), switchboard_key.c_str(), operating_mode);

	return SBIA_OK;
}


op_result L2_layer_adapter::get_if_auto_negotiation_state(int & auto_negotiation_state, uint16_t if_index, const std::string & mag, fixs_ith::BoardSlotPosition pos, fixs_ith::switchboard_plane_t plane, const unsigned int * timeout_ms)
{
	std::string sb_key;
	build_sb_key_from_magazine_and_slot(sb_key, mag, pos);
	return get_if_auto_negotiation_state(auto_negotiation_state, if_index, sb_key, plane, timeout_ms);
}

op_result L2_layer_adapter::get_if_auto_negotiation_state(int & auto_negotiation_state, uint16_t if_index, const std::string & switchboard_key, fixs_ith::switchboard_plane_t plane, const unsigned int * timeout_ms )
{
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Request to GET autoNegotiationState of IF_INDEX '%d' on the '%s' plane of the SWITCH BOARD having key '%s'",
			if_index, plane_descr(plane), switchboard_key.c_str());

	// retrieve the switch board hosting the target BRIDGE
	fixs_ith_switchboardinfo switchboard_info;
	fixs_ith_sbdatamanager::sbm_constant call_result = _sb_datamanager->get_switchboard_info(switchboard_info, switchboard_key);
	if(call_result != fixs_ith_sbdatamanager::SBM_OK)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to GET autoNegotiationState of the specified interface. [Call 'fixs_ith_sbdatamanager::get_switchboard_info()' returned '%d' ]", call_result);
		return SBIA_SWITCHBOARD_INFO_NOT_AVAILABLE;
	}

	// now use SNMP Manager to get the the auto negotiation state of the interface
	int get_if_mau_auto_neg_res = fixs_ith::workingSet_t::instance()->get_snmpmanager().get_if_mau_auto_neg_admin_status(auto_negotiation_state, if_index, switchboard_info, plane, timeout_ms);
	if(get_if_mau_auto_neg_res < 0)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to get if mau auto negotiation status of the specified interface via SNMP Manager."
				" [Call 'fixs_ith_snmpmager::get_if_auto_negotiation_state()' returned %", get_if_mau_auto_neg_res);
		return map_snmpmanager_error(get_if_mau_auto_neg_res);
	}

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Successfully GOT if mau auto negotiation status of IF_INDEX '%d' on the '%s' plane of the SWITCH BOARD having key '%s'",
			if_index, plane_descr(plane), switchboard_key.c_str());

	return SBIA_OK;
}


op_result L2_layer_adapter::get_agg_actor_admin_key(int32_t & agg_actor_admin_key, uint16_t if_index, const std::string & mag, fixs_ith::BoardSlotPosition pos, fixs_ith::switchboard_plane_t plane, const unsigned int * timeout_ms)
{
	std::string sb_key;
	build_sb_key_from_magazine_and_slot(sb_key, mag, pos);
	return get_agg_actor_admin_key(agg_actor_admin_key, if_index, sb_key, plane, timeout_ms);
}


op_result L2_layer_adapter::get_agg_actor_admin_key(int32_t & agg_actor_admin_key, uint16_t if_index, const std::string & switchboard_key, fixs_ith::switchboard_plane_t plane, const unsigned int * timeout_ms )
{
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Request to GET max frame size of IF_INDEX '%d' on the '%s' plane of the SWITCH BOARD having key '%s'",
					if_index, plane_descr(plane), switchboard_key.c_str());

	// retrieve the switch board hosting the target BRIDGE
	fixs_ith_switchboardinfo switchboard_info;
	fixs_ith_sbdatamanager::sbm_constant call_result = _sb_datamanager->get_switchboard_info(switchboard_info, switchboard_key);
	if(call_result != fixs_ith_sbdatamanager::SBM_OK)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to GET aggActorAdminKey of the specified interface. [Call 'fixs_ith_sbdatamanager::get_switchboard_info()' returned '%d' ]", call_result);
		return SBIA_SWITCHBOARD_INFO_NOT_AVAILABLE;
	}

	// now use SNMP Manager to get the operative state of the interface
	int get_agg_actor_admin_key_res = fixs_ith::workingSet_t::instance()->get_snmpmanager().get_agg_actor_admin_key(agg_actor_admin_key, if_index, switchboard_info, plane, timeout_ms);
	if(get_agg_actor_admin_key_res < 0)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to get aggActorAdminKey of the specified interface via SNMP Manager."
				" [Call 'fixs_ith_snmpmager::get_agg_actor_admin_key()' returned %", get_agg_actor_admin_key_res);
		return map_snmpmanager_error(get_agg_actor_admin_key_res);
	}

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Successfully GOT aggActorAdminKey of IF_INDEX '%d' on the '%s' plane of "
				"the SWITCH BOARD having key '%s'. {aggActorAdminKey  == '%d'} ",
					if_index, plane_descr(plane), switchboard_key.c_str(), agg_actor_admin_key);

	return SBIA_OK;
}

op_result  L2_layer_adapter::get_aggregator_operMembers(std::set<uint16_t> & if_members, uint16_t if_index, const std::string & mag, fixs_ith::BoardSlotPosition pos, fixs_ith::switchboard_plane_t plane, const unsigned int * timeout_ms)
{
	std::string sb_key;
	build_sb_key_from_magazine_and_slot(sb_key, mag, pos);
	return get_aggregator_operMembers(if_members, if_index, sb_key, plane, timeout_ms);
}


op_result L2_layer_adapter::get_aggregator_operMembers(std::set<uint16_t> & if_members, uint16_t if_index, const std::string & switchboard_key, fixs_ith::switchboard_plane_t plane, const unsigned int * timeout_ms)
{
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Request to GET operative members of the AGGREGATOR having IF_INDEX '%d' on the '%s' plane of the SWITCH BOARD having key '%s'",
						if_index, plane_descr(plane), switchboard_key.c_str());

	// retrieve the switch board hosting the target BRIDGE
	fixs_ith_switchboardinfo switchboard_info;
	fixs_ith_sbdatamanager::sbm_constant call_result = _sb_datamanager->get_switchboard_info(switchboard_info, switchboard_key);
	if(call_result != fixs_ith_sbdatamanager::SBM_OK)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to GET operative members of the AGGREGATOR having IF_INDEX '%u' on the '%s' plane of "
				"the SWITCH BOARD having key '%s'.[Call 'fixs_ith_sbdatamanager::get_switchboard_info()' returned '%d' ]",
				if_index, plane_descr(plane), switchboard_key.c_str(), call_result);

		return SBIA_SWITCHBOARD_INFO_NOT_AVAILABLE;
	}

	// now use SNMP Manager to get AGGREGATOR's operative members of the interface
	fixs_ith_snmp::if_bitmap_t portlist_bitmap;
	memset(portlist_bitmap,0,FIXS_ITH_ARRAY_SIZE(portlist_bitmap));
	fixs_ith_snmpmanager & snmp_manager = fixs_ith::workingSet_t::instance()->get_snmpmanager();
	int op_res = snmp_manager.get_lag_operMembers(portlist_bitmap, if_index, switchboard_info, plane, timeout_ms);
	if(op_res < 0)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to GET operative members of the AGGREGATOR having IF_INDEX '%u' on the '%s' plane of "
						"the SWITCH BOARD having key '%s'.[Call 'fixs_ith_snmpmanager::get_lag_operMembers()' returned '%d' ]",
						if_index, plane_descr(plane), switchboard_key.c_str(), op_res);

		return map_snmpmanager_error(op_res);
	}

	// fill output parameter (TODO check)
	if_members.clear();
	for(size_t i = 0; i < FIXS_ITH_ARRAY_SIZE(portlist_bitmap); ++i)
	{
		std::bitset<8> current_byte(portlist_bitmap[i]);
		for(int j = 7; j >= 0; --j)
		{
			if(current_byte.test(j))
			{
				if_members.insert(8 * i + 8 - j);
			}
		}
	}

	// prepare LOG string  (((TODO solo per DEBUGGING, da eliminare)))
	std::string log_str;
	for(std::set<uint16_t>::iterator it = if_members.begin(); it != if_members.end(); ++it)
	{
		char curr_index[8] = {0};
		::snprintf(curr_index, sizeof(curr_index),"%s%u", ( (it != if_members.begin()) ? "," : ""), *it);
		log_str.append(curr_index);
	}

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Successfully GOT the operative members of the AGGREGATOR having IF_INDEX '%u' on the '%s' plane of "
				"the SWITCH BOARD having key '%s'. {OPERATIVE MEMBERS LIST  == '%s'} ",
					if_index, plane_descr(plane), switchboard_key.c_str(), log_str.c_str());

	return SBIA_OK;

}

op_result L2_layer_adapter::get_bridge_numberOfVlans(uint32_t & num_vlans, const std::string & mag, fixs_ith::BoardSlotPosition pos, fixs_ith::switchboard_plane_t plane, const unsigned int * timeout_ms )
{
	std::string sb_key;
	build_sb_key_from_magazine_and_slot(sb_key, mag, pos);
	return get_bridge_numberOfVlans(num_vlans, sb_key, plane, timeout_ms);
}


op_result L2_layer_adapter::get_bridge_numberOfVlans(uint32_t & num_vlans, const std::string & switchboard_key, fixs_ith::switchboard_plane_t plane, const unsigned int * timeout_ms)
{
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Request to GET number of VLANs defined on the '%s' plane of the SWITCH BOARD having key '%s'",
					plane_descr(plane), switchboard_key.c_str());

	// retrieve the switch board hosting the target BRIDGE
	fixs_ith_switchboardinfo switchboard_info;
	fixs_ith_sbdatamanager::sbm_constant call_result = _sb_datamanager->get_switchboard_info(switchboard_info, switchboard_key);
	if(call_result != fixs_ith_sbdatamanager::SBM_OK)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to GET number of VLANs defined on the '%s' plane of the SWITCH BOARD having key '%s'. "
					   "[Call 'fixs_ith_sbdatamanager::get_switchboard_info()' returned '%d' ]",
					   plane_descr(plane), switchboard_key.c_str(), call_result);

		return SBIA_SWITCHBOARD_INFO_NOT_AVAILABLE;
	}

	// now use SNMP Manager to get the number of VLANS defined on the specified BRIDGE
	uint32_t outval = 0;
	int op_res = fixs_ith::workingSet_t::instance()->get_snmpmanager().get_number_of_vlans(outval, switchboard_info, plane, timeout_ms);
	if(op_res < 0)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to GET number of VLANs defined on the '%s' plane of the SWITCH BOARD having key '%s'. "
							   "[Call 'fixs_ith_snmpmanager::get_number_of_vlans()' returned '%d' ]",
							   plane_descr(plane), switchboard_key.c_str(), op_res);

		return map_snmpmanager_error(op_res);
	}

	// Fill output parameter
	num_vlans = outval;

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Successfully GOT the number of VLANs defined on the '%s' plane of "
				"the SWITCH BOARD having key '%s'. {NUM_VLANS  == '%u'} ",
					plane_descr(plane), switchboard_key.c_str(), outval);

	return SBIA_OK;
}

op_result  L2_layer_adapter::get_vlan_ports(std::set<uint16_t> & tagged_if_ports, std::set<uint16_t> & untagged_if_ports, uint16_t vid, const std::string & mag, fixs_ith::BoardSlotPosition pos, fixs_ith::switchboard_plane_t plane, const unsigned int * timeout_ms)
{
	std::string sb_key;
	build_sb_key_from_magazine_and_slot(sb_key, mag, pos);
	return get_vlan_ports(tagged_if_ports, untagged_if_ports, vid, sb_key, plane, timeout_ms);
}

op_result  L2_layer_adapter::get_vlan_ports(std::set<uint16_t> & tagged_if_ports, std::set<uint16_t> & untagged_if_ports, uint16_t vid, const std::string & switchboard_key, fixs_ith::switchboard_plane_t plane, const unsigned int * timeout_ms)
{
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Request to GET ALL VLAN PORTS of vlan having vid '%u' on the '%s' plane of the SWITCH BOARD having key '%s'",
						 vid, plane_descr(plane), switchboard_key.c_str());

	// retrieve the switch board hosting the target BRIDGE
	fixs_ith_switchboardinfo switchboard_info;
	fixs_ith_sbdatamanager::sbm_constant call_result = _sb_datamanager->get_switchboard_info(switchboard_info, switchboard_key);
	if(call_result != fixs_ith_sbdatamanager::SBM_OK)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to GET VLAN PORTS of vlan having vid '%u' on the '%s' plane of "
				"the SWITCH BOARD having key '%s'.[Call 'fixs_ith_sbdatamanager::get_switchboard_info()' returned '%d' ]",
				vid, plane_descr(plane), switchboard_key.c_str(), call_result);


		return SBIA_SWITCHBOARD_INFO_NOT_AVAILABLE;
	}

	// now use SNMP Manager to get Vlan ports
	fixs_ith_snmp::if_bitmap_t tagged_portlist_bitmap;
	memset(tagged_portlist_bitmap,0,FIXS_ITH_ARRAY_SIZE(tagged_portlist_bitmap));
	fixs_ith_snmp::if_bitmap_t untagged_portlist_bitmap;
	memset(untagged_portlist_bitmap,0,FIXS_ITH_ARRAY_SIZE(untagged_portlist_bitmap));

	fixs_ith_snmpmanager & snmp_manager = fixs_ith::workingSet_t::instance()->get_snmpmanager();
	// Get first ALL VLAN  PORTS
	int op_res = snmp_manager.get_vlan_ports(tagged_portlist_bitmap, untagged_portlist_bitmap,vid, switchboard_info, plane, timeout_ms);
	if(op_res < 0)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to GET ALL VLAN PORTS of vlan having vid '%u' on the '%s' plane of "
					"the SWITCH BOARD having key '%s'.[Call 'fixs_ith_snmpmanager::get_vlan_ports()' returned '%d' ]",
					vid, plane_descr(plane), switchboard_key.c_str(), op_res);

		return map_snmpmanager_error(op_res);
	}


	// fill tagged_if_ports output parameter (TODO check)
	tagged_if_ports.clear();
	for(size_t i = 0; i < FIXS_ITH_ARRAY_SIZE(tagged_portlist_bitmap); ++i)
	{
		std::bitset<8> current_byte(tagged_portlist_bitmap[i]);
		for(int j = 7; j >= 0; --j)
		{
			if(current_byte.test(j))
			{
				tagged_if_ports.insert(8 * i + 8 - j);
			}
		}
	}

	// prepare LOG string  (((TODO solo per DEBUGGING, da eliminare)))
	std::string log_str;
	for(std::set<uint16_t>::iterator it = tagged_if_ports.begin(); it != tagged_if_ports.end(); ++it)
	{
		char curr_index[8] = {0};
		::snprintf(curr_index, sizeof(curr_index),"%s%u", ( (it != tagged_if_ports.begin()) ? "," : ""), *it);
		log_str.append(curr_index);
	}

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Successfully GOT the TAGGED PORTS of Vlan having vid  '%u' on the '%s' plane of "
				"the SWITCH BOARD having key '%s'. {PORT_LIST  == '%s'} ",
				vid, plane_descr(plane), switchboard_key.c_str(), log_str.c_str());


	// fill output parameter (TODO check)
	untagged_if_ports.clear();
	for(size_t i = 0; i < FIXS_ITH_ARRAY_SIZE(untagged_portlist_bitmap); ++i)
	{
		std::bitset<8> current_byte(untagged_portlist_bitmap[i]);
		for(int j = 7; j >= 0; --j)
		{
			if(current_byte.test(j))
			{
				untagged_if_ports.insert(8 * i + 8 - j);
			}
		}
	}

	// prepare LOG string  (((TODO solo per DEBUGGING, da eliminare)))
	log_str.clear();
	for(std::set<uint16_t>::iterator it = untagged_if_ports.begin(); it != untagged_if_ports.end(); ++it)
	{
		char curr_index[8] = {0};
		::snprintf(curr_index, sizeof(curr_index),"%s%u", ( (it != untagged_if_ports.begin()) ? "," : ""), *it);
		log_str.append(curr_index);
	}

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Successfully GOT the UNTAGGED PORTS of Vlan having vid  '%u' on the '%s' plane of "
				"the SWITCH BOARD having key '%s'. {PORT_LIST  == '%s'} ",
				vid, plane_descr(plane), switchboard_key.c_str(), log_str.c_str());

	return SBIA_OK;

}

op_result  L2_layer_adapter::get_vlan_ports(std::set<uint16_t> & if_ports, uint16_t vid, VLAN_port_type port_type, const std::string & mag, fixs_ith::BoardSlotPosition pos, fixs_ith::switchboard_plane_t plane, const unsigned int * timeout_ms)
{
	std::string sb_key;
	build_sb_key_from_magazine_and_slot(sb_key, mag, pos);
	return get_vlan_ports(if_ports, vid, port_type, sb_key, plane, timeout_ms);
}

op_result  L2_layer_adapter::get_vlan_ports(std::set<uint16_t> & if_ports, uint16_t vid, VLAN_port_type port_type, const std::string & switchboard_key, fixs_ith::switchboard_plane_t plane, const unsigned int * timeout_ms)
{

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Request to GET VLAN %s PORTS of vlan having vid '%u' on the '%s' plane of the SWITCH BOARD having key '%s'",
						(port_type == VLAN_TAGGED) ? "TAGGED": "UNTAGGED",
						 vid, plane_descr(plane), switchboard_key.c_str());

	// retrieve the switch board hosting the target BRIDGE
	fixs_ith_switchboardinfo switchboard_info;
	fixs_ith_sbdatamanager::sbm_constant call_result = _sb_datamanager->get_switchboard_info(switchboard_info, switchboard_key);
	if(call_result != fixs_ith_sbdatamanager::SBM_OK)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to GET VLAN %s PORTS of vlan having vid '%u' on the '%s' plane of "
				"the SWITCH BOARD having key '%s'.[Call 'fixs_ith_sbdatamanager::get_switchboard_info()' returned '%d' ]",
				(port_type == VLAN_TAGGED) ? "TAGGED": "UNTAGGED",
				vid, plane_descr(plane), switchboard_key.c_str(), call_result);

		return SBIA_SWITCHBOARD_INFO_NOT_AVAILABLE;
	}

	// now use SNMP Manager to get Vlan ports
	fixs_ith_snmp::if_bitmap_t portlist_bitmap;
	memset(portlist_bitmap,0,FIXS_ITH_ARRAY_SIZE(portlist_bitmap));
	fixs_ith_snmpmanager & snmp_manager = fixs_ith::workingSet_t::instance()->get_snmpmanager();
	fixs_ith_snmp::vlanPort_type_t snmp_vlan_port_type = (port_type == VLAN_TAGGED)? fixs_ith_snmp::TAGGED: fixs_ith_snmp::UNTAGGED;
	int op_res = snmp_manager.get_vlan_ports(portlist_bitmap, vid, snmp_vlan_port_type, switchboard_info, plane, timeout_ms);
	if(op_res < 0)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to GET VLAN %s PORTS of vlan having vid '%u' on the '%s' plane of "
					"the SWITCH BOARD having key '%s'.[Call 'fixs_ith_snmpmanager::get_vlan_ports()' returned '%d' ]",
					(port_type == VLAN_TAGGED) ? "TAGGED": "UNTAGGED",
					vid, plane_descr(plane), switchboard_key.c_str(), op_res);

		return map_snmpmanager_error(op_res);
	}

	// fill output parameter (TODO check)
	if_ports.clear();
	for(size_t i = 0; i < FIXS_ITH_ARRAY_SIZE(portlist_bitmap); ++i)
	{
		std::bitset<8> current_byte(portlist_bitmap[i]);
		for(int j = 7; j >= 0; --j)
		{
			if(current_byte.test(j))
			{
				if_ports.insert(8 * i + 8 - j);
			}
		}
	}

	// prepare LOG string  (((TODO solo per DEBUGGING, da eliminare)))
	std::string log_str;
	for(std::set<uint16_t>::iterator it = if_ports.begin(); it != if_ports.end(); ++it)
	{
		char curr_index[8] = {0};
		::snprintf(curr_index, sizeof(curr_index),"%s%u", ( (it != if_ports.begin()) ? "," : ""), *it);
		log_str.append(curr_index);
	}

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Successfully GOT the %s PORTS of Vlan having vid  '%u' on the '%s' plane of "
				"the SWITCH BOARD having key '%s'. {PORT_LIST  == '%s'} ",
				(port_type == VLAN_TAGGED) ? "TAGGED": "UNTAGGED",
				vid, plane_descr(plane), switchboard_key.c_str(), log_str.c_str());

	return SBIA_OK;

}

op_result L2_layer_adapter::set_if_adminState(const std::string & mag, fixs_ith::BoardSlotPosition pos, fixs_ith::switchboard_plane_t plane, uint16_t if_index, BP_admin_state bp_admin_state)
{
	std::string sb_key;
	build_sb_key_from_magazine_and_slot(sb_key, mag, pos);
	return set_if_adminState(sb_key, plane, if_index, bp_admin_state);
}

op_result L2_layer_adapter::set_if_adminState(const std::string & switchboard_key, fixs_ith::switchboard_plane_t plane, uint16_t if_index, BP_admin_state bp_admin_state)
{
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Request to SET administrative state of IF_INDEX '%u' on the '%s' plane of the SWITCH BOARD having key '%s'",
						if_index, plane_descr(plane), switchboard_key.c_str());

	// retrieve the switch board hosting the target BRIDGE
	fixs_ith_switchboardinfo switchboard_info;
	fixs_ith_sbdatamanager::sbm_constant call_result = _sb_datamanager->get_switchboard_info(switchboard_info, switchboard_key);
	if(call_result != fixs_ith_sbdatamanager::SBM_OK)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to SET administrative state of IF_INDEX '%u' on the '%s' plane of the SWITCH BOARD having key '%s'"
				"[Call 'fixs_ith_sbdatamanager::get_switchboard_info()' returned '%d' ]",
				if_index, plane_descr(plane), switchboard_key.c_str(), call_result);

		return SBIA_SWITCHBOARD_INFO_NOT_AVAILABLE;
	}

	// now use SNMP Manager to set the administrative state of the interface
	fixs_ith_snmp::port_status_t admin_status =
		((bp_admin_state == BP_LOCKED) ? fixs_ith_snmp::PORT_STATUS_DOWN: fixs_ith_snmp::PORT_STATUS_UP);

	int op_res = fixs_ith::workingSet_t::instance()->get_snmpmanager().set_port_admin_status(switchboard_info, if_index, admin_status,  plane);
	if(op_res < 0)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to SET administrative state of IF_INDEX '%u' on the '%s' plane of the SWITCH BOARD having key '%s'. "
				  "[Call 'fixs_ith_snmpmanager::set_port_admin_status()' returned '%d']",
				  if_index, plane_descr(plane), switchboard_key.c_str(), op_res);

		return map_snmpmanager_error(op_res);
	}

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Successfully SET the administrative state of the IF_INDEX '%u' defined on the '%s' plane of "
				"the SWITCH BOARD having key '%s'",
					if_index, plane_descr(plane), switchboard_key.c_str());

	return SBIA_OK;
}

op_result L2_layer_adapter::set_if_priority(const std::string & mag, fixs_ith::BoardSlotPosition pos, fixs_ith::switchboard_plane_t plane, uint16_t if_index)
{
        std::string sb_key;
        build_sb_key_from_magazine_and_slot(sb_key, mag, pos);
        return set_if_priority(sb_key, plane, if_index);
}

op_result L2_layer_adapter::set_if_priority(const std::string & switchboard_key, fixs_ith::switchboard_plane_t plane, uint16_t if_index)
{
        FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Request to SET default priority of port at IF_INDEX '%u' on the '%s' plane of the SWITCH BOARD having key '%s'",
                                                if_index, plane_descr(plane), switchboard_key.c_str());

        // retrieve the switch board hosting the target BRIDGE
        fixs_ith_switchboardinfo switchboard_info;
        fixs_ith_sbdatamanager::sbm_constant call_result = _sb_datamanager->get_switchboard_info(switchboard_info, switchboard_key);
        if(call_result != fixs_ith_sbdatamanager::SBM_OK)
        {
                FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to SET default priority of port at IF_INDEX '%u' on the '%s' plane of the SWITCH BOARD having key '%s'"
                                "[Call 'fixs_ith_sbdatamanager::get_switchboard_info()' returned '%d' ]",
                                if_index, plane_descr(plane), switchboard_key.c_str(), call_result);

                return SBIA_SWITCHBOARD_INFO_NOT_AVAILABLE;
        }

        // now use SNMP Manager to set the priority of the port
	int op_res = fixs_ith::workingSet_t::instance()->get_snmpmanager().set_mib_port_default_user_priority(switchboard_info, if_index);
        if(op_res < 0)
        {
                FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to SET the priority of ethernet port at IF_INDEX '%u' on the SWITCH BOARD having key '%s'. "
                                  "[Call 'fixs_ith_snmpmanager::.set_port_admin_status()' returned '%d']",
                                  if_index, switchboard_key.c_str(), op_res);

                return map_snmpmanager_error(op_res);
        }
 	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Successfully SET the priority of ethernet port at IF_INDEX '%u' on "
                                "the SWITCH BOARD having key '%s'",
                                        if_index, switchboard_key.c_str());

        return SBIA_OK;
}

op_result L2_layer_adapter::set_eth_max_frame_size(const std::string & mag, fixs_ith::BoardSlotPosition pos, fixs_ith::switchboard_plane_t plane, uint16_t if_index, uint32_t eth_max_frame_size)
{
	std::string sb_key;
	build_sb_key_from_magazine_and_slot(sb_key, mag, pos);
	return set_eth_max_frame_size(sb_key, plane, if_index, eth_max_frame_size);
}

op_result L2_layer_adapter::set_eth_max_frame_size(const std::string & switchboard_key, fixs_ith::switchboard_plane_t plane, uint16_t if_index, uint32_t eth_max_frame_size)
{
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Request to SET MAX FRAME SIZE '%u' on  IF_INDEX '%u' on the '%s' plane of the SWITCH BOARD having key '%s'",
			eth_max_frame_size, if_index, plane_descr(plane), switchboard_key.c_str());

	// retrieve the switch board hosting the target BRIDGE
	fixs_ith_switchboardinfo switchboard_info;
	fixs_ith_sbdatamanager::sbm_constant call_result = _sb_datamanager->get_switchboard_info(switchboard_info, switchboard_key);
	if(call_result != fixs_ith_sbdatamanager::SBM_OK)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to SET MAX FRAME SIZE '%u' on IF_INDEX '%u' on the '%s' plane of the SWITCH BOARD having key '%s'. "
				"[Call 'fixs_ith_sbdatamanager::get_switchboard_info()' returned '%d' ]",
				eth_max_frame_size, if_index, plane_descr(plane), switchboard_key.c_str(), call_result);
		return SBIA_SWITCHBOARD_INFO_NOT_AVAILABLE;
	}

	// now use SNMP Manager to set the max frame size of the interface

	int op_res = fixs_ith::workingSet_t::instance()->get_snmpmanager().set_eth_max_frame_size(switchboard_info, if_index, eth_max_frame_size,  plane);
	if(op_res < 0)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to SET MAX FRAME SIZE '%u' on IF_INDEX '%u' on the '%s' plane of the SWITCH BOARD having key '%s'. "
				"[Call 'fixs_ith_snmpmanager::set_eth_max_frame_size()' returned '%d' ]",
				eth_max_frame_size, if_index, plane_descr(plane), switchboard_key.c_str(), op_res);
		return map_snmpmanager_error(op_res);
	}

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Successfully SET the MAX FRAME SIZE '%u' on the IF_INDEX '%u' defined on the '%s' plane of "
			"the SWITCH BOARD having key '%s'",
			eth_max_frame_size, if_index, plane_descr(plane), switchboard_key.c_str());

	return SBIA_OK;
}

op_result L2_layer_adapter::set_eth_auto_negotiation(const std::string & mag, fixs_ith::BoardSlotPosition pos, fixs_ith::switchboard_plane_t plane, uint16_t if_index, bool enable_auto_neg)
{
	std::string sb_key;
	build_sb_key_from_magazine_and_slot(sb_key, mag, pos);
	return set_eth_auto_negotiation(sb_key, plane, if_index, enable_auto_neg);
}

op_result L2_layer_adapter::set_eth_auto_negotiation(const std::string & switchboard_key, fixs_ith::switchboard_plane_t plane, uint16_t if_index, bool enable_auto_neg)
{
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Request to SET AUTONEGOTIATION '%d' on IF_INDEX '%u' on the '%s' plane of the SWITCH BOARD having key '%s'",
			enable_auto_neg, if_index, plane_descr(plane), switchboard_key.c_str());

	// retrieve the switch board hosting the target Ethernet Port
	fixs_ith_switchboardinfo switchboard_info;
	fixs_ith_sbdatamanager::sbm_constant call_result = _sb_datamanager->get_switchboard_info(switchboard_info, switchboard_key);
	if(call_result != fixs_ith_sbdatamanager::SBM_OK)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to SET AUTO NEGOTIATION '%d' on IF_INDEX '%u' on the '%s' plane of the SWITCH BOARD having key '%s'. "
				"[Call 'fixs_ith_sbdatamanager::get_switchboard_info()' returned '%d' ]",
				enable_auto_neg, if_index, plane_descr(plane), switchboard_key.c_str(), call_result);
		return SBIA_SWITCHBOARD_INFO_NOT_AVAILABLE;
	}

	// now use SNMP Manager to set the auto negotiation of the interface

	int op_res = fixs_ith::workingSet_t::instance()->get_snmpmanager().set_if_mau_auto_neg_admin_status(
			enable_auto_neg ? fixs_ith_snmp::FIXS_ITH_MAU_AUTO_NEG_STATUS_ENABLED : fixs_ith_snmp::FIXS_ITH_MAU_AUTO_NEG_STATUS_DISABLED, if_index, switchboard_info, plane);
	if(op_res < 0)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to SET AUTO NEGOTIATION '%d' on IF_INDEX '%u' on the '%s' plane of the SWITCH BOARD having key '%s'. "
				"[Call 'fixs_ith_snmpmanager::set_eth_auto_negotiation()' returned '%d' ]",
				enable_auto_neg, if_index, plane_descr(plane), switchboard_key.c_str(), op_res);
		return map_snmpmanager_error(op_res);
	}

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Successfully SET the AUTO NEGOTIATION '%d' on the IF_INDEX '%u' defined on the '%s' plane of "
			"the SWITCH BOARD having key '%s'",
			enable_auto_neg, if_index, plane_descr(plane), switchboard_key.c_str());

	return SBIA_OK;
}

// set eth mau default type
op_result L2_layer_adapter::set_eth_if_mau_type(const std::string & mag,
		fixs_ith::BoardSlotPosition pos, fixs_ith::switchboard_plane_t plane,
		uint16_t if_index,
		fixs_ith_snmp::IfMauTypeListBitsConstants default_type) 
{
	std::string sb_key;
	build_sb_key_from_magazine_and_slot(sb_key, mag, pos);
	return set_eth_auto_negotiation(sb_key, plane, if_index, default_type);
}

op_result L2_layer_adapter::set_eth_if_mau_type(
		const std::string & switchboard_key,
		fixs_ith::switchboard_plane_t plane, uint16_t if_index,
		fixs_ith_snmp::IfMauTypeListBitsConstants default_type) 
{
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
			"Request to SET IF MAU DEFAULT TYPE on IF_INDEX '%u' on the '%s' plane of the SWITCH BOARD having key '%s'",
			if_index, plane_descr(plane), switchboard_key.c_str());

	// retrieve the switch board hosting the target Ethernet Port
	fixs_ith_switchboardinfo switchboard_info;
	fixs_ith_sbdatamanager::sbm_constant call_result =_sb_datamanager->get_switchboard_info(switchboard_info,
					switchboard_key);
	if (call_result != fixs_ith_sbdatamanager::SBM_OK) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"Unable to SET IF MAU DEFAULT TYPE on IF_INDEX '%u' on the '%s' plane of the SWITCH BOARD having key '%s'. " "[Call 'fixs_ith_sbdatamanager::get_switchboard_info()' returned '%d' ]",
				if_index, plane_descr(plane), switchboard_key.c_str(),
				call_result);
		return SBIA_SWITCHBOARD_INFO_NOT_AVAILABLE;
	}

	// now use SNMP Manager to set the eth mau default type of interface

	int op_res = fixs_ith::workingSet_t::instance()->get_snmpmanager().set_if_mau_default_type(
					default_type, if_index, switchboard_info, plane);
	if (op_res < 0) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"Unable to SET IF MAU DEFAULT TYPE on IF_INDEX '%u' on the '%s' plane of the SWITCH BOARD having key '%s'. " "[Call 'fixs_ith_snmpmanager::set_eth_auto_negotiation()' returned '%d' ]",
				if_index, plane_descr(plane), switchboard_key.c_str(), op_res);
		return map_snmpmanager_error(op_res);
	}

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
			"Successfully SET the IF MAU DEFAULT TYPE on the IF_INDEX '%u' defined on the '%s' plane of " "the SWITCH BOARD having key '%s'",
			if_index, plane_descr(plane), switchboard_key.c_str());

	return SBIA_OK;
}

op_result L2_layer_adapter::set_port_default_vlan(const std::string & mag, fixs_ith::BoardSlotPosition pos, fixs_ith::switchboard_plane_t plane, uint16_t if_index, uint16_t vid)
{
	std::string sb_key;
	build_sb_key_from_magazine_and_slot(sb_key, mag, pos);
	return set_port_default_vlan(sb_key, plane, if_index, vid);
}

op_result L2_layer_adapter::set_port_default_vlan(const std::string & switchboard_key, fixs_ith::switchboard_plane_t plane, uint16_t if_index, uint16_t vid)
{
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Request to SET default VLAN ID %d on  IF_INDEX '%u' on the '%s' plane of the SWITCH BOARD having key '%s'",
						vid, if_index, plane_descr(plane), switchboard_key.c_str());

	// retrieve the switch board hosting the target BRIDGE
	fixs_ith_switchboardinfo switchboard_info;
	fixs_ith_sbdatamanager::sbm_constant call_result = _sb_datamanager->get_switchboard_info(switchboard_info, switchboard_key);
	if(call_result != fixs_ith_sbdatamanager::SBM_OK)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to SET default VLAN ID '%u' on IF_INDEX '%u' on the '%s' plane of the SWITCH BOARD having key '%s'. "
				"[Call 'fixs_ith_sbdatamanager::get_switchboard_info()' returned '%d' ]",
				vid, if_index, plane_descr(plane), switchboard_key.c_str(), call_result);
		return SBIA_SWITCHBOARD_INFO_NOT_AVAILABLE;
	}

	// now use SNMP Manager to set the default VLAN ID of the interface

	int op_res = fixs_ith::workingSet_t::instance()->get_snmpmanager().set_port_default_vlan(switchboard_info, if_index, vid,  plane);
	if(op_res < 0)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to SET default VLAN ID '%u' on IF_INDEX '%u' on the '%s' plane of the SWITCH BOARD having key '%s'. "
						"[Call 'fixs_ith_snmpmanager::set_port_default_vlan()' returned '%d' ]",
						vid, if_index, plane_descr(plane), switchboard_key.c_str(), op_res);
		return map_snmpmanager_error(op_res);
	}

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Successfully SET the the default VLAN ID %d on the IF_INDEX '%u' defined on the '%s' plane of "
				"the SWITCH BOARD having key '%s'",
					vid, if_index, plane_descr(plane), switchboard_key.c_str());

	return SBIA_OK;
}

op_result L2_layer_adapter::set_eth_oam_adminState(const std::string & mag, fixs_ith::BoardSlotPosition pos, fixs_ith::switchboard_plane_t plane, uint16_t if_index, OAM_admin_state oam_admin_state)
{
	std::string sb_key;
	build_sb_key_from_magazine_and_slot(sb_key, mag, pos);
	return set_eth_oam_adminState(sb_key, plane, if_index, oam_admin_state);
}


op_result L2_layer_adapter::set_eth_oam_adminState(const std::string & switchboard_key, fixs_ith::switchboard_plane_t plane, uint16_t if_index, OAM_admin_state oam_admin_state)
{
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Request to SET administrative state of IF_INDEX '%u' on the '%s' plane of the SWITCH BOARD having key '%s'",
			if_index, plane_descr(plane), switchboard_key.c_str());

	// retrieve the switch board hosting the target BRIDGE
	fixs_ith_switchboardinfo switchboard_info;
	fixs_ith_sbdatamanager::sbm_constant call_result = _sb_datamanager->get_switchboard_info(switchboard_info, switchboard_key);
	if(call_result != fixs_ith_sbdatamanager::SBM_OK)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to SET OAM administrative state of IF_INDEX '%u' on the '%s' plane of the SWITCH BOARD having key '%s'"
				"[Call 'fixs_ith_sbdatamanager::get_switchboard_info()' returned '%d' ]",
				if_index, plane_descr(plane), switchboard_key.c_str(), call_result);

		return SBIA_SWITCHBOARD_INFO_NOT_AVAILABLE;
	}

	// now use SNMP Manager to set the administrative state of the interface
	fixs_ith_snmp::oam_status_t admin_status =
			((oam_admin_state == OAM_DISABLED) ? fixs_ith_snmp::OAM_ADMIN_DISABLED: fixs_ith_snmp::OAM_ADMIN_ENABLED);

	int op_res = fixs_ith::workingSet_t::instance()->get_snmpmanager().set_port_oam_admin_status(switchboard_info, if_index, admin_status,  plane);
	if(op_res < 0)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to SET OAM administrative state of IF_INDEX '%u' on the '%s' plane of the SWITCH BOARD having key '%s'. "
				"[Call 'fixs_ith_snmpmanager::set_port_admin_status()' returned '%d']",
				if_index, plane_descr(plane), switchboard_key.c_str(), op_res);

		return map_snmpmanager_error(op_res);
	}

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Successfully SET the OAM administrative state of the IF_INDEX '%u' defined on the '%s' plane of "
			"the SWITCH BOARD having key '%s'",
			if_index, plane_descr(plane), switchboard_key.c_str());

	return SBIA_OK;
}

op_result L2_layer_adapter::set_aggregator_members(const std::string & mag, fixs_ith::BoardSlotPosition pos, fixs_ith::switchboard_plane_t plane, uint16_t if_index, const std::set<uint16_t> & admin_members)
{
	std::string sb_key;
	build_sb_key_from_magazine_and_slot(sb_key, mag, pos);
	return set_aggregator_members(sb_key, plane, if_index, admin_members);
}

op_result L2_layer_adapter::set_aggregator_members(const std::string & switchboard_key, fixs_ith::switchboard_plane_t plane, uint16_t if_index, const std::set<uint16_t> & admin_members)
{
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Request to SET members of the AGGREGATOR having IF_INDEX '%u' on the '%s' plane of the SWITCH BOARD having key '%s'",
							if_index, plane_descr(plane), switchboard_key.c_str());

	std::set<uint16_t>::iterator iter;

	for(iter=admin_members.begin(); iter!=admin_members.end();++iter){
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, " ADD to AGGREGATOR having IF_INDEX '%u' the following port '%u'",
				if_index, *iter);
	}


	// retrieve the switch board hosting the target BRIDGE
	fixs_ith_switchboardinfo switchboard_info;
	fixs_ith_sbdatamanager::sbm_constant call_result = _sb_datamanager->get_switchboard_info(switchboard_info, switchboard_key);
	if(call_result != fixs_ith_sbdatamanager::SBM_OK)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to SET members of the AGGREGATOR having IF_INDEX '%u' on the '%s' plane of the SWITCH BOARD having key '%s'. "
				"[Call 'fixs_ith_sbdatamanager::get_switchboard_info()' returned '%d' ]",
				if_index, plane_descr(plane), switchboard_key.c_str(), call_result);

		return SBIA_SWITCHBOARD_INFO_NOT_AVAILABLE;
	}

	// now use SNMP Manager to set AGGREGATOR's members
	fixs_ith_snmpmanager & snmp_manager = fixs_ith::workingSet_t::instance()->get_snmpmanager();
	int op_res = snmp_manager.add_lag_ports(switchboard_info, if_index, admin_members, plane);
	if(op_res < 0)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to SET members of the AGGREGATOR having IF_INDEX '%u' on the '%s' plane of the SWITCH BOARD having key '%s'. "
						"[Call 'fixs_ith_snmpmanager::add_lag_ports()' returned '%d' ]",
						if_index, plane_descr(plane), switchboard_key.c_str(), op_res);

		return map_snmpmanager_error(op_res);
	}

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Successfully SET the members of the AGGREGATOR having IF_INDEX '%u' on the '%s' plane of "
				"the SWITCH BOARD having key '%s'", if_index, plane_descr(plane), switchboard_key.c_str());

	return SBIA_OK;
}


op_result L2_layer_adapter::set_vlan_members(const std::string & mag, fixs_ith::BoardSlotPosition pos, fixs_ith::switchboard_plane_t plane, uint16_t vid, const std::set<uint16_t> & tagged_members, const std::set<uint16_t> & untagged_members)
{
	std::string sb_key;
	build_sb_key_from_magazine_and_slot(sb_key, mag, pos);
	return set_vlan_members(sb_key, plane, vid, tagged_members, untagged_members);
}


op_result L2_layer_adapter::set_vlan_members(const std::string & switchboard_key, fixs_ith::switchboard_plane_t plane, uint16_t vid, const std::set<uint16_t> & tagged_members, const std::set<uint16_t> & untagged_members)
{
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Request to SET vlan members of the VLAN {vid == '%u'} on the '%s' plane of the SWITCH BOARD having key '%s'",
								vid, plane_descr(plane), switchboard_key.c_str());

	// retrieve the switch board hosting the target BRIDGE
	fixs_ith_switchboardinfo switchboard_info;
	fixs_ith_sbdatamanager::sbm_constant call_result = _sb_datamanager->get_switchboard_info(switchboard_info, switchboard_key);
	if(call_result != fixs_ith_sbdatamanager::SBM_OK)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to SET vlan members of the VLAN {vid == '%u'} on the '%s' plane of the SWITCH BOARD having key '%s'. "
				"[Call 'fixs_ith_sbdatamanager::get_switchboard_info()' returned '%d' ]",
				vid, plane_descr(plane), switchboard_key.c_str(), call_result);

		return SBIA_SWITCHBOARD_INFO_NOT_AVAILABLE;
	}

	// now use SNMP Manager to set VLAN's tagged members
	fixs_ith_snmpmanager & snmp_manager = fixs_ith::workingSet_t::instance()->get_snmpmanager();
	int op_res = snmp_manager.set_vlan_ports(switchboard_info, vid, tagged_members, untagged_members, plane);
	if(op_res < 0)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to SET vlan members of the VLAN {vid == '%u'} on the '%s' plane of the SWITCH BOARD having key '%s'. "
						"[Call 'fixs_ith_snmpmanager::set_vlan_ports()' returned '%d' ]",
						vid, plane_descr(plane), switchboard_key.c_str(), op_res);

		return map_snmpmanager_error(op_res);
	}

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Successfully SET the vlan members of the VLAN {vid == '%u'} on the '%s' plane of "
				"the SWITCH BOARD having key '%s'",	vid, plane_descr(plane), switchboard_key.c_str());

	return SBIA_OK;
}

op_result L2_layer_adapter::set_ipv4subnet_vlanid(const std::string & mag, fixs_ith::BoardSlotPosition pos, fixs_ith::switchboard_plane_t plane, const std::string & ipv4subnet, uint16_t vlanid)
{
	std::string sb_key;
	build_sb_key_from_magazine_and_slot(sb_key, mag, pos);
	return set_ipv4subnet_vlanid(sb_key, plane, ipv4subnet, vlanid);
}


op_result L2_layer_adapter::set_ipv4subnet_vlanid(const std::string & switchboard_key, fixs_ith::switchboard_plane_t plane, const std::string & ipv4subnet, uint16_t vlanid)
{
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Request to SET VID '%u' for IPV4 Subnet '%s' on the '%s' plane of the SWITCH BOARD having key '%s'",
						vlanid, ipv4subnet.c_str(),plane_descr(plane), switchboard_key.c_str());

	// retrieve the switch board hosting the target BRIDGE
	fixs_ith_switchboardinfo switchboard_info;
	fixs_ith_sbdatamanager::sbm_constant call_result = _sb_datamanager->get_switchboard_info(switchboard_info, switchboard_key);
	if(call_result != fixs_ith_sbdatamanager::SBM_OK)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to SET VID '%u' for IPV4 Subnet '%s' on the '%s' plane of the SWITCH BOARD having key '%s'"
				"[Call 'fixs_ith_sbdatamanager::get_switchboard_info()' returned '%d' ]",
				vlanid, ipv4subnet.c_str(), plane_descr(plane), switchboard_key.c_str(), call_result);

		return SBIA_SWITCHBOARD_INFO_NOT_AVAILABLE;
	}


	int op_res = fixs_ith::workingSet_t::instance()->get_snmpmanager().set_ipv4subnet_vid(switchboard_info,plane,ipv4subnet,vlanid);

	if(op_res < 0)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Unable to SET VID '%u' for IPV4 Subnet '%s' on the '%s' plane of the SWITCH BOARD having key '%s'. "
				  "[Call 'fixs_ith_snmpmanager::set_ipv4subnet_vid()' returned '%d']",
				  vlanid,ipv4subnet.c_str(), plane_descr(plane), switchboard_key.c_str(), op_res);

		return map_snmpmanager_error(op_res);
	}

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Successfully SET VID '%u' for IPV4 Subnet '%s' on the '%s' plane of "
				"the SWITCH BOARD having key '%s'",
					vlanid, ipv4subnet.c_str(), plane_descr(plane), switchboard_key.c_str());

	return SBIA_OK;

}

op_result L2_layer_adapter::enable_vrrp_new_master_notification(const std::string& switchboard_key)
{
	// retrieve the switch board hosting the target BRIDGE
	fixs_ith_switchboardinfo switchboard_info;
	fixs_ith_sbdatamanager::sbm_constant call_result = _sb_datamanager->get_switchboard_info(switchboard_info, switchboard_key);

	if(fixs_ith_sbdatamanager::SBM_OK != call_result)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Failed to Enable VRRP MASTER CHANGE NOTIFICATION of the SWITCH BOARD having key <%s>"
				"[Call 'fixs_ith_sbdatamanager::get_switchboard_info()' returned <%d> ]",
				switchboard_key.c_str(), call_result);

		return SBIA_SWITCHBOARD_INFO_NOT_AVAILABLE;
	}

	int op_res = fixs_ith::workingSet_t::instance()->get_snmpmanager().enable_vrrp_new_master_notification(switchboard_info);

	if(op_res < 0)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Failed to Enable VRRP MASTER CHANGE NOTIFICATION of the SWITCH BOARD having key <%s>. "
				"[Call 'fixs_ith_snmpmanager::enable_vrrp_new_master_notification()' returned '%d']",
				 switchboard_key.c_str(), op_res);

		return map_snmpmanager_error(op_res);
	}

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Successfully Enabled VRRP MASTER CHANGE NOTIFICATION of the SWITCH BOARD having key <%s>",
			 switchboard_key.c_str());

	return SBIA_OK;

}

op_result L2_layer_adapter::enable_bfd_session_state_notification(const std::string& switchboard_key)
{
	// retrieve the switch board hosting the target board
	fixs_ith_switchboardinfo switchboard_info;
	fixs_ith_sbdatamanager::sbm_constant call_result = _sb_datamanager->get_switchboard_info(switchboard_info, switchboard_key);

	if(fixs_ith_sbdatamanager::SBM_OK != call_result)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Failed to Enable BFD SESSION UP/DOWN NOTIFICATION of the SWITCH BOARD having key <%s>"
				"[Call 'fixs_ith_sbdatamanager::get_switchboard_info()' returned <%d> ]",
				switchboard_key.c_str(), call_result);

		return SBIA_SWITCHBOARD_INFO_NOT_AVAILABLE;
	}

	int op_res = fixs_ith::workingSet_t::instance()->get_snmpmanager().enable_bfd_session_state_notification(switchboard_info);

	if(op_res < 0)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Failed to Enable BFD SESSION UP/DOWN NOTIFICATION of the SWITCH BOARD having key <%s>. "
				"[Call 'fixs_ith_snmpmanager::enable_vrrp_new_master_notification()' returned '%d']",
				 switchboard_key.c_str(), op_res);

		return map_snmpmanager_error(op_res);
	}

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Successfully Enabled BFD SESSION UP/DOWN NOTIFICATION of the SWITCH BOARD having key <%s>",
			 switchboard_key.c_str());

	return SBIA_OK;
}

op_result L2_layer_adapter::get_board_status(const std::string& switchboard_key, unsigned int timeout_ms)
{
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "L2_layer_adapter:get_board_status");
	// retrieve the switch board info
	fixs_ith_switchboardinfo switchboard_info;
	fixs_ith_sbdatamanager::sbm_constant call_result = _sb_datamanager->get_switchboard_info(switchboard_info, switchboard_key);

	if(fixs_ith_sbdatamanager::SBM_OK != call_result)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Failed to get board info for <%s>"
				"[Call 'fixs_ith_sbdatamanager::get_switchboard_info()' returned <%d> ]",
				switchboard_key.c_str(), call_result);

		return SBIA_SWITCHBOARD_INFO_NOT_AVAILABLE;
	}

	uint32_t outval = 0U;
	int op_res;
	if(common::utility::isSwitchBoardSMX())
	{
		op_res = fixs_ith::workingSet_t::instance()->get_snmpmanager().get_number_of_vlans(outval, switchboard_info, fixs_ith::CONTROL_PLANE, &timeout_ms);
	}
	if(common::utility::isSwitchBoardCMX())
        {
		op_res = fixs_ith::workingSet_t::instance()->get_snmpmanager().get_number_of_vlans(outval, switchboard_info, fixs_ith::TRANSPORT_PLANE, &timeout_ms);
	}

	if(op_res < 0)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Failed to connect towards <%s>. "
				"[Call 'fixs_ith_snmpmanager::enable_vrrp_new_master_notification()' returned '%d']",
				switchboard_key.c_str(), op_res);

		return map_snmpmanager_error(op_res);
	}

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "SWITCH BOARD having key <%s> is UP", switchboard_key.c_str());

	return SBIA_OK;
}

op_result L2_layer_adapter::get_board_status(fixs_ith_switchboardinfo & switchboard_info, unsigned int timeout_ms)
{
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, " L2_layer_adapter:get_board_status(switchboard info)");
	uint32_t outval = 0U;
	int op_res;
	if(common::utility::isSwitchBoardSMX())
        {
		op_res = fixs_ith::workingSet_t::instance()->get_snmpmanager().get_number_of_vlans(outval, switchboard_info, fixs_ith::CONTROL_PLANE, &timeout_ms);
	}

        if(common::utility::isSwitchBoardCMX())
        {
                op_res = fixs_ith::workingSet_t::instance()->get_snmpmanager().get_number_of_vlans(outval, switchboard_info, fixs_ith::TRANSPORT_PLANE, &timeout_ms);
        }


	if(op_res < 0)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Failed to connect towards <0x%08X>. mag_id == <%d>, slot == <%d>. "
				"[Call 'get_number_of_vlans' returned '%d']",
				switchboard_info.magazine, switchboard_info.magazine_id, switchboard_info.slot_position, op_res);

		return map_snmpmanager_error(op_res);
	}

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "SWITCH BOARD at <0x%08X> and slot <%d> is UP", switchboard_info.magazine, switchboard_info.slot_position);

	return SBIA_OK;
}

op_result L2_layer_adapter::reset_board(const std::string& switchboard_key)
{
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "L2_layer_adapter:reset_board");
	fixs_ith_switchboardinfo switchboard_info;
	fixs_ith_sbdatamanager::sbm_constant call_result = _sb_datamanager->get_switchboard_info(switchboard_info, switchboard_key);

	if(fixs_ith_sbdatamanager::SBM_OK != call_result)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Failed to reset the SWITCH BOARD having key <%s>"
				"[Call 'fixs_ith_sbdatamanager::get_switchboard_info()' returned <%d> ]",
				switchboard_key.c_str(), call_result);

		return SBIA_SWITCHBOARD_INFO_NOT_AVAILABLE;
	}

	int op_res = fixs_ith::workingSet_t::instance()->get_snmpmanager().reset_board(switchboard_info);

	if(op_res < 0)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Failed to RESET the SWITCH BOARD having key <%s>. "
				"[Call 'fixs_ith_snmpmanager::reset_board()' returned '%d']",
				switchboard_key.c_str(), op_res);

		return map_snmpmanager_error(op_res);
	}

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Successfully RESET the SWITCH BOARD having key <%s>", switchboard_key.c_str() );
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "L2_layer_adapter:Leaving reset_board");

	return SBIA_OK;
}

//TR_HY52528 BEGIN
op_result L2_layer_adapter::reset_board(const std::string& switchboard_key, fixs_ith::board_slotposition_t slot)
{
        fixs_ith_switchboardinfo switchboard_info;
        fixs_ith_sbdatamanager::sbm_constant call_result = _sb_datamanager->get_switchboard_info(switchboard_info, switchboard_key);

        if(fixs_ith_sbdatamanager::SBM_OK != call_result)
        {
                FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Failed to reset the SWITCH BOARD having key <%s>"
                                "[Call 'fixs_ith_sbdatamanager::get_switchboard_info()' returned <%d> ]",
                                switchboard_key.c_str(), call_result);

                return SBIA_SWITCHBOARD_INFO_NOT_AVAILABLE;
        }

        int op_res = fixs_ith::workingSet_t::instance()->get_snmpmanager().reset_board(switchboard_info, slot);

        if(op_res < 0)
        {
                FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Failed to RESET the SWITCH BOARD having key <%s>. "
                                "[Call 'fixs_ith_snmpmanager::reset_board()' returned '%d']",
                                switchboard_key.c_str(), op_res);

                return map_snmpmanager_error(op_res);
        }

        FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Successfully RESET the SWITCH BOARD having key <%s>", switchboard_key.c_str() );

        return SBIA_OK;
}
//TR_HY52528 END

op_result L2_layer_adapter::disable_rstp(const std::string& switchboard_key, fixs_ith::switchboard_plane_t plane, uint16_t if_index)
{
	// retrieve the switch board hosting the target board
	fixs_ith_switchboardinfo switchboard_info;
	fixs_ith_sbdatamanager::sbm_constant call_result = _sb_datamanager->get_switchboard_info(switchboard_info, switchboard_key);

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Request to Disable RSTP on Bridge Port IF_INDEX '%u' on the %s plane of the SWITCH BOARD having key '%s'",
				if_index, common::utility::planeToString(plane).c_str(), switchboard_key.c_str());

	if(fixs_ith_sbdatamanager::SBM_OK != call_result)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Failed to Disable RSTP on Bridge Port IF_INDEX '%u' of the SWITCH BOARD having key <%s>"
				"[Call 'fixs_ith_sbdatamanager::get_switchboard_info()' returned <%d> ]", if_index,
				switchboard_key.c_str(), call_result);

		return SBIA_SWITCHBOARD_INFO_NOT_AVAILABLE;
	}

	int op_res = fixs_ith::workingSet_t::instance()->get_snmpmanager().disable_stp_port_status(switchboard_info, if_index, plane);

	if(op_res < 0)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Failed to Disable RSTP on Bridge Port IF_INDEX '%u' of the SWITCH BOARD having key <%s>. "
				"[Call 'fixs_ith_snmpmanager::disable_stp_port_status()' returned '%d']", if_index,
				 switchboard_key.c_str(), op_res);

		return map_snmpmanager_error(op_res);
	}

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Successfully Disable RSTP on Bridge Port IF_INDEX '%u' of the SWITCH BOARD having key <%s>", if_index,
			 switchboard_key.c_str());

	return SBIA_OK;
}


op_result L2_layer_adapter::configure_bp_port_as_edgeport(const std::string& switchboard_key, fixs_ith::switchboard_plane_t plane, uint16_t if_index)
{
	// retrieve the switch board hosting the target board
	fixs_ith_switchboardinfo switchboard_info;
	fixs_ith_sbdatamanager::sbm_constant call_result = _sb_datamanager->get_switchboard_info(switchboard_info, switchboard_key);

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Request to Configure Bridge Port IF_INDEX '%u' as EDGE PORT on the %s plane of SWITCH BOARD having key '%s'",
				if_index, common::utility::planeToString(plane).c_str(), switchboard_key.c_str());

	if(fixs_ith_sbdatamanager::SBM_OK != call_result)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Failed to Configure Bridge Port IF_INDEX '%u' as EDGE PORT on the SWITCH BOARD having key <%s>"
				"[Call 'fixs_ith_sbdatamanager::get_switchboard_info()' returned <%d> ]", if_index,
				switchboard_key.c_str(), call_result);

		return SBIA_SWITCHBOARD_INFO_NOT_AVAILABLE;
	}

	int op_res = fixs_ith::workingSet_t::instance()->get_snmpmanager().setBridgePortasEdgePort(switchboard_info, if_index, plane);

	if(op_res < 0)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Failed to Configure Bridge Port IF_INDEX '%u' as EDGE PORT on the SWITCH BOARD having key <%s>. "
				"[Call 'fixs_ith_snmpmanager::disable_stp_port_status()' returned '%d']", if_index,
				 switchboard_key.c_str(), op_res);

		return map_snmpmanager_error(op_res);
	}

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Successfully Configure Bridge Port IF_INDEX '%u' as EDGE PORT on the SWITCH BOARD having key <%s>", if_index,
			 switchboard_key.c_str());

	return SBIA_OK;
}


IF_oper_mode L2_layer_adapter::if_operOperating_mode(uint32_t if_speed, uint16_t if_duplex_status,uint16_t if_masterslave_status)
{
	IF_oper_mode operating_mode = IF_OPERMODE_UNKNOWN;

	switch(if_speed)
	{
	// 10 Mbps
	case 10:
		if(if_duplex_status == fixs_ith_snmp::OPERMODE_FULLDUPLEX ) operating_mode = IF_OPERMODE_10M_FULL;
		if(if_duplex_status == fixs_ith_snmp::OPERMODE_HALFDUPLEX ) operating_mode = IF_OPERMODE_10M_HALF;
		break;

	//100Mbps
	case 100:
		if(if_duplex_status == fixs_ith_snmp::OPERMODE_FULLDUPLEX ) operating_mode =  IF_OPERMODE_100M_FULL;
		if(if_duplex_status == fixs_ith_snmp::OPERMODE_HALFDUPLEX ) operating_mode =  IF_OPERMODE_100M_HALF;
		break;
    //1Gbps
	case 1000:
		if(if_duplex_status == fixs_ith_snmp::OPERMODE_FULLDUPLEX){
			switch(if_masterslave_status){
			case fixs_ith_snmp::PORTSTATUS_MASTER: operating_mode = IF_OPERMODE_1G_FULL_MASTER; break;

			case fixs_ith_snmp::PORTSTATUS_SLAVE: operating_mode = IF_OPERMODE_1G_FULL_SLAVE; break;

			default: operating_mode = IF_OPERMODE_1G_FULL; break;
			}
		}
		break;
    //10 Gbps
	case 10*1000 :
	if(if_duplex_status == fixs_ith_snmp::OPERMODE_FULLDUPLEX){
		switch(if_masterslave_status){
		case fixs_ith_snmp::PORTSTATUS_MASTER: operating_mode = IF_OPERMODE_10G_FULL_MASTER; break;

		case fixs_ith_snmp::PORTSTATUS_SLAVE: operating_mode = IF_OPERMODE_10G_FULL_SLAVE; break;

		default: operating_mode = IF_OPERMODE_10G_FULL; break;
		}
	}
	break;
    //40 Gbps
	case 40*1000:
	if(if_duplex_status == fixs_ith_snmp::OPERMODE_FULLDUPLEX) operating_mode = IF_OPERMODE_40G_FULL;
	break;
    //100 Gbps
	case 100*1000:
	if(if_duplex_status == fixs_ith_snmp::OPERMODE_FULLDUPLEX) operating_mode = IF_OPERMODE_100G_FULL;
	break;

	default:
		break;
	}
	return operating_mode;
}



// TR_HY50894 BEGIN
op_result L2_layer_adapter::map_snmpmanager_error(int snmpmanager_error)
{
	op_result sbia_error = SBIA_ERROR;

	switch(snmpmanager_error)
	{
	case fixs_ith_snmp::ERR_SNMP_NO_ERRORS:
		sbia_error = SBIA_OK;
		break;
	case fixs_ith_snmp::ERR_SNMP_PDU_VARIABLE_NO_SUCH_INSTANCE:
		sbia_error = SBIA_OBJ_NOT_FOUND;
		break;
	case fixs_ith_snmp::ERR_SNMP_PDU_VARIABLE_NO_SUCH_OBJECT:
	{
		if(common::utility::isSwitchBoardCMX())
			sbia_error = SBIA_OBJ_NOT_FOUND;
		else
			sbia_error = SBIA_ERROR;
	}
	break;
	default:
		sbia_error = SBIA_ERROR;
		break;
	}
	// TR_HY50894 END
	return sbia_error;    
}

}


