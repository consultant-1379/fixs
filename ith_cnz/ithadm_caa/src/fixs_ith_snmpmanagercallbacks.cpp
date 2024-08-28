/*
 * fixs_ith_snmpmanagercallbacks.cpp
 *
 *  Created on: Feb 2, 2016
 *      Author: xassore
 */

#include "fixs_ith_programmacros.h"
#include "fixs_ith_logger.h"
#include "fixs_ith_snmpmanagercallbacks.h"
#include "fixs_ith_snmpmanager.h"
#include "fixs_ith_csreader.h"

#define __FIXS_ITH_UTILS_MEMCPY_INCREMENT__(dest, src, n) ((::memcpy((dest), (src), (n))), (dest[(n)] = 0), (src += (n)))

namespace {
void str_trim_right(char * str, size_t size) {
	if (!size)
		return;

	char * str_tmp = str + (size - 1);

	for (char c = *str_tmp; (str_tmp >= str) && ((c == ' ') || !c); c =
			*--str_tmp)
		;

	if (static_cast<size_t>(++str_tmp - str) < size)
		*str_tmp = 0;
}
}

int fixs_ith_snmploggercallback::read_smx(
		netsnmp_variable_list * var_list) const {
	if (message)
		FIXS_ITH_LOG(LOG_LEVEL_INFO, message);

	FIXS_ITH_LOG(LOG_LEVEL_INFO, "SNMP response pdu data follows");

	char buffer[2 * 1024];

	for (netsnmp_variable_list * var = var_list; var; var =
			var->next_variable) {
		snprint_variable(buffer, FIXS_ITH_ARRAY_SIZE(buffer), var->name,
				var->name_length, var);
		FIXS_ITH_LOG(LOG_LEVEL_INFO, "    %s", buffer);
	}

	return 1; //OK return for NET-SNMP library
}

/*
 * Callback function to read the PORT Status of front port from the mib for SCX boards.
 */
int fixs_ith_getportstatuscallback::read_smx(
		netsnmp_variable_list * var_list) const {
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Reading PORT status from SCX");

	// Arrange the oid names
	oid oid_oper_status[] = { IF_OPER_STATUS, port_ifIndex };
	oid oid_admin_status[] = { IF_ADMIN_STATUS, port_ifIndex };
	oid *oid_name;

	if (status_type == fixs_ith_snmp::OPERATIVE_STATUS)
		oid_name = oid_oper_status;
	else
		oid_name = oid_admin_status;

	return read_value((const oid *) oid_name, FIXS_ITH_ARRAY_SIZE(oid_name),
			var_list);
}

int fixs_ith_getportstatuscallback::read_value(const oid * oid_name,
		size_t oid_names_size, netsnmp_variable_list * var) const {
	char tmp_str[2 * 1024];

	if (fixs_ith_snmpmanager::oid_compare(oid_name, oid_names_size, var->name,
			var->name_length) == 0) {
		int char_printed =
				::snprintf(tmp_str, FIXS_ITH_ARRAY_SIZE(tmp_str),
						"Reading the following POER status info: NET-SNMP variable printout --> ");
		snprint_variable(tmp_str + char_printed,
				FIXS_ITH_ARRAY_SIZE(tmp_str) - char_printed, var->name,
				var->name_length, var);
		FIXS_ITH_LOG(LOG_LEVEL_INFO, tmp_str);

		if (check_pdu_var_type(var->type, ASN_INTEGER))
			return 1;

		if (port_status)
			*port_status =
					static_cast<fixs_ith_snmp::port_status_t>(*var->val.integer);
		else
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
					"The PDU contains a PORT status info but the caller did not requested it");
	} else
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"The received PDU does not contain the expected PORT status value");

	return 1;
}

int fixs_ith_get_number_of_ports_callback::read_smx(
		netsnmp_variable_list * var_list) const {
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Reading 'number of ports' from SMX");

	// Arrange the oid names
	const oid oid_bridge_port_num[] = { FIXS_ITH_SMX_OID_BRIDGE_NUM_PORTS, 0 };
	const oid oid_ethernet_port_num[] = { IF_NUMBER, 0 };
	const oid *oid_name;

	if (port_type == fixs_ith_snmp::BRIDGEPORT)
		oid_name = oid_bridge_port_num;
	else
		oid_name = oid_ethernet_port_num;

	return read_value(oid_name, FIXS_ITH_ARRAY_SIZE(oid_name), var_list);
}

int fixs_ith_get_number_of_ports_callback::read_value(const oid * oid_name,
		size_t oid_names_size, netsnmp_variable_list * var) const {
	char tmp_str[2 * 1024];

	if (fixs_ith_snmpmanager::oid_compare(oid_name, oid_names_size, var->name,
			var->name_length) == 0) {
		int char_printed =
				::snprintf(tmp_str, FIXS_ITH_ARRAY_SIZE(tmp_str),
						"Reading the following 'number of ports' info: NET-SNMP variable printout --> ");
		snprint_variable(tmp_str + char_printed,
				FIXS_ITH_ARRAY_SIZE(tmp_str) - char_printed, var->name,
				var->name_length, var);
		FIXS_ITH_LOG(LOG_LEVEL_INFO, tmp_str);

		if (check_pdu_var_type(var->type, ASN_INTEGER))
			return 1;

		if (number_of_ports)
			*number_of_ports = static_cast<int>(*var->val.integer);
		else
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
					"The PDU contains a 'number of ports' info but the caller did not requested it");
	} else
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"The received PDU does not contain the 'number of ports' value");

	return 1;
}
int fixs_ith_get_brigeaddress_callback::read_smx(
		netsnmp_variable_list * var_list) const {
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Reading Bridge Address from SMX");

	// Arrange the oid names
	const oid oid_name[] = { FIXS_ITH_SMX_OID_BRIDGE_ADDRESS, 0 };

	return read_value(oid_name, FIXS_ITH_ARRAY_SIZE(oid_name), var_list);
}

int fixs_ith_get_brigeaddress_callback::read_value(const oid * oid_name,
		size_t oid_names_size, netsnmp_variable_list * var) const {
	char tmp_str[2 * 1024];

	if (fixs_ith_snmpmanager::oid_compare(oid_name, oid_names_size, var->name,
			var->name_length) == 0) {
		int char_printed =
				::snprintf(tmp_str, FIXS_ITH_ARRAY_SIZE(tmp_str),
						"Reading the following manager 'bridge address' info: NET-SNMP variable printout --> ");
		snprint_variable(tmp_str + char_printed,
				FIXS_ITH_ARRAY_SIZE(tmp_str) - char_printed, var->name,
				var->name_length, var);
		FIXS_ITH_LOG(LOG_LEVEL_INFO, tmp_str);

		if (check_pdu_var_type(var->type, ASN_OCTET_STR)) {
			FIXS_ITH_LOG(LOG_LEVEL_WARN,
					"The received PDU has a bad type <type ==  %d>\n ",
					var->type);
			return 1;
		}

		if (bridge_address) {
			const uint8_t * tmp = reinterpret_cast<uint8_t*>(var->val.string);
			//char value[fixs_ith_snmp::SNMP_MACADDRESS_FIELD_SIZE + 1] = {0};
			for (unsigned int i = 0;
					i < fixs_ith_snmp::SNMP_MACADDRESS_FIELD_SIZE
							&& i < var->val_len; i++)
				bridge_address[i] = tmp[i];
			//snprintf(value, var->val_len + 1, "%s", var->val.string);
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "FOUND BRIDGE ADDRESS value");

		} else
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
					"The PDU contains a Bridge Address info but the caller did not requested it");
	} else
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"The received PDU does not contain the expected Bridge Address OID");

	return 1;
}

int fixs_ith_get_number_of_vlans_callback::read_smx(
		netsnmp_variable_list * var_list) const {
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Reading 'number of vlans' from SMX");

	// Arrange the oid names
	const oid oid_name[] = { FIXS_ITH_SMX_OID_BRIDGE_NUM_VLANS, 0 };

	return read_value(oid_name, FIXS_ITH_ARRAY_SIZE(oid_name), var_list);
}

int fixs_ith_get_number_of_vlans_callback::read_value(const oid * oid_name,
		size_t oid_names_size, netsnmp_variable_list * var) const {
	char tmp_str[2 * 1024];

	if (fixs_ith_snmpmanager::oid_compare(oid_name, oid_names_size, var->name,
			var->name_length) == 0) {
		int char_printed =
				::snprintf(tmp_str, FIXS_ITH_ARRAY_SIZE(tmp_str),
						"Reading the following 'number of vlans' info: NET-SNMP variable printout --> ");
		snprint_variable(tmp_str + char_printed,
				FIXS_ITH_ARRAY_SIZE(tmp_str) - char_printed, var->name,
				var->name_length, var);
		FIXS_ITH_LOG(LOG_LEVEL_INFO, tmp_str);

		if (check_pdu_var_type(var->type, ASN_UNSIGNED))
			return 1;

		if (number_of_vlans)
			*number_of_vlans = static_cast<uint32_t>(*var->val.integer);
		else
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
					"The PDU contains a 'number of vlans' info but the caller did not requested it");
	} else
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"The received PDU does not contain the 'number of vlans' value");

	return 1;
}

int fixs_ith_get_port_if_index_callback::read_smx(
		netsnmp_variable_list * var_list) const {
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
			"Reading 'brige port interface number' from SMX");

	// Arrange the oid names
	const oid oid_name[] = { FIXS_ITH_SMX_OID_BRIDGEPORT_IF_INDEX, port_number };

	return read_value(oid_name, FIXS_ITH_ARRAY_SIZE(oid_name), var_list);
}

int fixs_ith_get_port_if_index_callback::read_value(const oid * oid_name,
		size_t oid_names_size, netsnmp_variable_list * var) const {
	char tmp_str[2 * 1024];

	if (fixs_ith_snmpmanager::oid_compare(oid_name, oid_names_size, var->name,
			var->name_length) == 0) {
		int char_printed =
				::snprintf(tmp_str, FIXS_ITH_ARRAY_SIZE(tmp_str),
						"Reading the following 'brige port if number' info: NET-SNMP variable printout --> ");
		snprint_variable(tmp_str + char_printed,
				FIXS_ITH_ARRAY_SIZE(tmp_str) - char_printed, var->name,
				var->name_length, var);
		FIXS_ITH_LOG(LOG_LEVEL_INFO, tmp_str);

		if (check_pdu_var_type(var->type, ASN_INTEGER))
			return 1;

		if (if_index)
			*if_index = static_cast<uint32_t>(*var->val.integer);
		else
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
					"The PDU contains a 'interface index' info but the caller did not requested it");
	} else
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"The received PDU does not contain the 'interface index' value");

	return 1;
}

int fixs_ith_getpimpresencecallback::read_smx(
		netsnmp_variable_list * var_list) const {
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
			"Reading 'pim port interface number' from SMX");

	// Arrange the oid names
	const oid oid_name[] =
			{ FIXS_ITH_SMX_OID_PIM_PRESENCE_STATUS, interface_num };

	return read_value(oid_name, FIXS_ITH_ARRAY_SIZE(oid_name), var_list);
}

int fixs_ith_getpimpresencecallback::read_value(const oid * oid_name,
		size_t oid_names_size, netsnmp_variable_list * var) const {
	char tmp_str[2 * 1024];

	if (fixs_ith_snmpmanager::oid_compare(oid_name, oid_names_size, var->name,
			var->name_length) == 0) {
		int char_printed =
				::snprintf(tmp_str, FIXS_ITH_ARRAY_SIZE(tmp_str),
						"Reading the following 'pim presence' info: NET-SNMP variable printout --> ");
		snprint_variable(tmp_str + char_printed,
				FIXS_ITH_ARRAY_SIZE(tmp_str) - char_printed, var->name,
				var->name_length, var);
		FIXS_ITH_LOG(LOG_LEVEL_INFO, tmp_str);

		if (check_pdu_var_type(var->type, ASN_INTEGER))
			return 1;

		if (port_presencestat)
			*port_presencestat = static_cast<uint32_t>(*var->val.integer);
		else
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
					"The PDU contains a 'pim presence' info but the caller did not requested it");
	} else
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"The received PDU does not contain the 'pim presence' value");

	return 1;
}

int fixs_ith_getpimmatchingcallback::read_smx(
		netsnmp_variable_list * var_list) const {
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Reading 'pim matching' from SMX");

	// Arrange the oid names
	const oid oid_name[] =
			{ FIXS_ITH_SMX_OID_PIM_MATCHING_STATUS, interface_num };

	return read_value(oid_name, FIXS_ITH_ARRAY_SIZE(oid_name), var_list);
}

int fixs_ith_getpimmatchingcallback::read_value(const oid * oid_name,
		size_t oid_names_size, netsnmp_variable_list * var) const {
	char tmp_str[2 * 1024];

	if (fixs_ith_snmpmanager::oid_compare(oid_name, oid_names_size, var->name,
			var->name_length) == 0) {
		int char_printed =
				::snprintf(tmp_str, FIXS_ITH_ARRAY_SIZE(tmp_str),
						"Reading the following 'pim matching' info: NET-SNMP variable printout --> ");
		snprint_variable(tmp_str + char_printed,
				FIXS_ITH_ARRAY_SIZE(tmp_str) - char_printed, var->name,
				var->name_length, var);
		FIXS_ITH_LOG(LOG_LEVEL_INFO, tmp_str);

		if (check_pdu_var_type(var->type, ASN_INTEGER))
			return 1;

		if (port_matchingstat)
			*port_matchingstat = static_cast<uint32_t>(*var->val.integer);
		else
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
					"The PDU contains a 'pim matching' info but the caller did not requested it");
	} else
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"The received PDU does not contain the 'pim matching' value");

	return 1;
}

int fixs_ith_get_interface_speed_callback::read_smx(
		netsnmp_variable_list * var_list) const {
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Reading 'interface speed' from SMX");

	// Arrange the oid names
	const oid oid_name[] = { SNMP_OID_IF_HIGH_SPEED, if_index };

	return read_value(oid_name, FIXS_ITH_ARRAY_SIZE(oid_name), var_list);
}

int fixs_ith_get_interface_speed_callback::read_value(const oid * oid_name,
		size_t oid_names_size, netsnmp_variable_list * var) const {
	char tmp_str[2 * 1024];

	if (fixs_ith_snmpmanager::oid_compare(oid_name, oid_names_size, var->name,
			var->name_length) == 0) {
		int char_printed =
				::snprintf(tmp_str, FIXS_ITH_ARRAY_SIZE(tmp_str),
						"Reading the following 'interface speed' info: NET-SNMP variable printout --> ");
		snprint_variable(tmp_str + char_printed,
				FIXS_ITH_ARRAY_SIZE(tmp_str) - char_printed, var->name,
				var->name_length, var);
		FIXS_ITH_LOG(LOG_LEVEL_INFO, tmp_str);

		if (check_pdu_var_type(var->type, ASN_GAUGE))
			return 1;

		if (if_speed)
			*if_speed = static_cast<uint32_t>(*var->val.integer);
		else
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
					"The PDU contains a 'interface speed' info but the caller did not requested it");
	} else
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"The received PDU does not contain the 'interface speed' value");

	return 1;
}

int fixs_ith_get_eth_frame_size_callback::read_smx(
		netsnmp_variable_list * var_list) const {
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Reading 'eth frame size' from SMX");

	// Arrange the oid names
	const oid oid_name[] = { IF_MAX_FRAME_SIZE, if_index };

	return read_value(oid_name, FIXS_ITH_ARRAY_SIZE(oid_name), var_list);
}

int fixs_ith_get_eth_frame_size_callback::read_value(const oid * oid_name,
		size_t oid_names_size, netsnmp_variable_list * var) const {
	char tmp_str[2 * 1024];

	if (fixs_ith_snmpmanager::oid_compare(oid_name, oid_names_size, var->name,
			var->name_length) == 0) {
		int char_printed =
				::snprintf(tmp_str, FIXS_ITH_ARRAY_SIZE(tmp_str),
						"Reading the following 'eth frame size' info: NET-SNMP variable printout --> ");
		snprint_variable(tmp_str + char_printed,
				FIXS_ITH_ARRAY_SIZE(tmp_str) - char_printed, var->name,
				var->name_length, var);
		FIXS_ITH_LOG(LOG_LEVEL_INFO, tmp_str);

		if (check_pdu_var_type(var->type, ASN_UNSIGNED))
			return 1;

		if (eth_max_frame_size)
			*eth_max_frame_size = static_cast<uint32_t>(*var->val.integer);
		else
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
					"The PDU contains a 'eth max frame size' info but the caller did not requested it");
	} else
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"The received PDU does not contain the 'eth frame size' value");

	return 1;
}

int fixs_ith_get_eth_duplex_status_callback::read_smx(
		netsnmp_variable_list * var_list) const {
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Reading 'eth duplex status' from SMX");

	// Arrange the oid names
	const oid oid_name[] = { SNMP_DOT3_STATSDUPLEX_STATUS, if_index };

	return read_value(oid_name, FIXS_ITH_ARRAY_SIZE(oid_name), var_list);
}

int fixs_ith_get_eth_duplex_status_callback::read_value(const oid * oid_name,
		size_t oid_names_size, netsnmp_variable_list * var) const {
	char tmp_str[2 * 1024];

	if (fixs_ith_snmpmanager::oid_compare(oid_name, oid_names_size, var->name,
			var->name_length) == 0) {
		int char_printed =
				::snprintf(tmp_str, FIXS_ITH_ARRAY_SIZE(tmp_str),
						"Reading the following 'eth duplex status' info: NET-SNMP variable printout --> ");
		snprint_variable(tmp_str + char_printed,
				FIXS_ITH_ARRAY_SIZE(tmp_str) - char_printed, var->name,
				var->name_length, var);
		FIXS_ITH_LOG(LOG_LEVEL_INFO, tmp_str);

		if (check_pdu_var_type(var->type, ASN_INTEGER))
			return 1;

		if (eth_duplex_status)
			*eth_duplex_status = static_cast<uint16_t>(*var->val.integer);
		else
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
					"The PDU contains a 'eth duplex status ' info but the caller did not requested it");
	} else
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"The received PDU does not contain the 'eth duplex status' value");

	return 1;
}

int fixs_ith_get_phyMasterSlave_status_callback::read_smx(
		netsnmp_variable_list * var_list) const {
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Reading 'PHY Master Slave Status' from SMX");

	// Arrange the oid names
	const oid oid_name[] = { SNMP_GEN_EXTPHY_MASTERSLAVE_STATUS, if_index };

	return read_value(oid_name, FIXS_ITH_ARRAY_SIZE(oid_name), var_list);
}

int fixs_ith_get_phyMasterSlave_status_callback::read_value(
		const oid * oid_name, size_t oid_names_size,
		netsnmp_variable_list * var) const {
	char tmp_str[2 * 1024];

	if (fixs_ith_snmpmanager::oid_compare(oid_name, oid_names_size, var->name,
			var->name_length) == 0) {
		int char_printed =
				::snprintf(tmp_str, FIXS_ITH_ARRAY_SIZE(tmp_str),
						"Reading the following 'phy master_slave status' info: NET-SNMP variable printout --> ");
		snprint_variable(tmp_str + char_printed,
				FIXS_ITH_ARRAY_SIZE(tmp_str) - char_printed, var->name,
				var->name_length, var);
		FIXS_ITH_LOG(LOG_LEVEL_INFO, tmp_str);

		if (check_pdu_var_type(var->type, ASN_INTEGER))
			return 1;

		if (phy_master_slave_status)
			*phy_master_slave_status = static_cast<uint16_t>(*var->val.integer);
		else
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
					"The PDU contains a 'phy master_slave status ' info but the caller did not requested it");
	} else
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"The received PDU does not contain the 'phy master_slave status' value");

	return 1;
}

int fixs_ith_get_agg_actor_admin_key_callback::read_smx(
		netsnmp_variable_list * var_list) const {
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Reading 'aggActorAdminKey' from SMX");

	// Arrange the oid names
	const oid oid_name[] =
			{ FIXS_ITH_SMX_OID_LAG_PORT_ACTOR_ADMIN_KEY, if_index };

	return read_value(oid_name, FIXS_ITH_ARRAY_SIZE(oid_name), var_list);
}

int fixs_ith_get_agg_actor_admin_key_callback::read_value(const oid * oid_name,
		size_t oid_names_size, netsnmp_variable_list * var) const {
	char tmp_str[2 * 1024];

	if (fixs_ith_snmpmanager::oid_compare(oid_name, oid_names_size, var->name,
			var->name_length) == 0) {
		int char_printed =
				::snprintf(tmp_str, FIXS_ITH_ARRAY_SIZE(tmp_str),
						"Reading the following 'aggActorAdminKey' info: NET-SNMP variable printout --> ");
		snprint_variable(tmp_str + char_printed,
				FIXS_ITH_ARRAY_SIZE(tmp_str) - char_printed, var->name,
				var->name_length, var);
		FIXS_ITH_LOG(LOG_LEVEL_INFO, tmp_str);

		if (check_pdu_var_type(var->type, ASN_INTEGER))
			return 1;

		if (agg_actor_admin_key)
			*agg_actor_admin_key = static_cast<int32_t>(*var->val.integer);
		else
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
					"The PDU contains a 'aggActorAdminKey' info but the caller did not requested it");
	} else
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"The received PDU does not contain the 'aggActorAdminKey' value");

	return 1;
}

/*
 * Callback function to read the default vlan id  of port_ifIndex from the mib for SMX boards.
 */
int fixs_ith_getdefaultVlanidcallback::read_smx(
		netsnmp_variable_list * var_list) const {
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Reading PORT status from SCX");

	// Arrange the oid names
	const oid oid_name[] = { FIXS_ITH_SMX_OID_PORTVLAN_DOT1QPVID, port_number };

	return read_value(oid_name, FIXS_ITH_ARRAY_SIZE(oid_name), var_list);
}

int fixs_ith_getdefaultVlanidcallback::read_value(const oid * oid_name,
		size_t oid_names_size, netsnmp_variable_list * var) const {
	char tmp_str[2 * 1024];

	if (fixs_ith_snmpmanager::oid_compare(oid_name, oid_names_size, var->name,
			var->name_length) == 0) {
		int char_printed =
				::snprintf(tmp_str, FIXS_ITH_ARRAY_SIZE(tmp_str),
						"Reading the following PVID value: NET-SNMP variable printout --> ");
		snprint_variable(tmp_str + char_printed,
				FIXS_ITH_ARRAY_SIZE(tmp_str) - char_printed, var->name,
				var->name_length, var);
		FIXS_ITH_LOG(LOG_LEVEL_INFO, tmp_str);

		if (check_pdu_var_type(var->type, ASN_GAUGE))
			return 1;

		if (interface_pvid)
			*interface_pvid =
					static_cast<fixs_ith_snmp::qBridge::pvid_t>(*var->val.integer);
		else
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
					"The PDU contains a Default vlan id  info but the caller did not requested it");
	} else
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"The received PDU does not contain the expected default vlan id value");

	return 1;
}

/*
 * Callback function to read the default vlan id  of port_ifIndex from the mib for SMX boards.
 */
int fixs_ith_getportphysaddresscallback::read_smx(
		netsnmp_variable_list * var_list) const {
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Reading PORT status from SCX");

	// Arrange the oid names
	const oid oid_name[] = { IF_PHYSICAL_ADDRESS, port_ifIndex };

	return read_value(oid_name, FIXS_ITH_ARRAY_SIZE(oid_name), var_list);
}

int fixs_ith_getportphysaddresscallback::read_value(const oid * oid_name,
		size_t oid_names_size, netsnmp_variable_list * var) const {
	char tmp_str[2 * 1024];

	if (fixs_ith_snmpmanager::oid_compare(oid_name, oid_names_size, var->name,
			var->name_length) == 0) {
		int char_printed =
				::snprintf(tmp_str, FIXS_ITH_ARRAY_SIZE(tmp_str),
						"Reading the following manager IP address info: NET-SNMP variable printout --> ");
		snprint_variable(tmp_str + char_printed,
				FIXS_ITH_ARRAY_SIZE(tmp_str) - char_printed, var->name,
				var->name_length, var);
		FIXS_ITH_LOG(LOG_LEVEL_INFO, tmp_str);

		if (check_pdu_var_type(var->type, ASN_OCTET_STR)) {
			if (var->type == 0x81 && var->val_len == 0) {
				FIXS_ITH_LOG(LOG_LEVEL_INFO,
						"The received PDU contains an empty Bridge Address value");
				return (error_code =
						fixs_ith_snmp::ERR_SNMP_PDU_VARIABLE_NULL_SIZE);
			}
			FIXS_ITH_LOG(LOG_LEVEL_WARN,
					"The received PDU has a bad type <type ==  %d>\n ",
					var->type);
			return 1;
		}

		if (port_address) {
			const unsigned char * tmp =
					reinterpret_cast<unsigned char *>(var->val.string);
			//	char value[fixs_ith_snmp::SNMP_MACADDRESS_FIELD_SIZE + 1] = {0};
			for (unsigned int i = 0;
					i < fixs_ith_snmp::SNMP_MACADDRESS_FIELD_SIZE
							&& i < var->val_len; i++)
				port_address[i] = tmp[i];
			//	snprintf(value, var->val_len + 1, "%s", var->val.string);
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "FOUND BRIDGE ADDRESS");

		} else
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
					"The PDU contains a Bridge Address info but the caller did not requested it");
	} else
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"The received PDU does not contain the expected Bridge Address OID");

	return 1;
}

int fixs_ith_getportNamecallback::read_smx(
		netsnmp_variable_list * var_list) const {
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Reading PORT name from SCX");

	// Arrange the oid names
	const oid oid_name[] = { SNMP_OID_IF_NAME, if_index };

	return read_value(oid_name, FIXS_ITH_ARRAY_SIZE(oid_name), var_list);
}

int fixs_ith_getportNamecallback::read_value(const oid * oid_name,
		size_t oid_names_size, netsnmp_variable_list * var) const {
	char tmp_str[2 * 1024];

	if (fixs_ith_snmpmanager::oid_compare(oid_name, oid_names_size, var->name,
			var->name_length) == 0) {
		int char_printed =
				::snprintf(tmp_str, FIXS_ITH_ARRAY_SIZE(tmp_str),
						"Reading the following interface info: NET-SNMP variable printout --> ");
		snprint_variable(tmp_str + char_printed,
				FIXS_ITH_ARRAY_SIZE(tmp_str) - char_printed, var->name,
				var->name_length, var);
		FIXS_ITH_LOG(LOG_LEVEL_INFO, tmp_str);

		if (check_pdu_var_type(var->type, ASN_OCTET_STR)) {
			if (var->type == 0x81 && var->val_len == 0) {
				FIXS_ITH_LOG(LOG_LEVEL_INFO,
						"The received PDU contains port name value");
				return (error_code =
						fixs_ith_snmp::ERR_SNMP_PDU_VARIABLE_NULL_SIZE);
			}
			FIXS_ITH_LOG(LOG_LEVEL_WARN,
					"The received PDU has a bad type <type ==  %d>\n ",
					var->type);
			return 1;
		}
		if (port_name) {
			const unsigned char * tmp =
					reinterpret_cast<unsigned char *>(var->val.string);
			char value[fixs_ith_snmp::IF_NAME_MAX_LEN + 1] = { 0 };
			for (unsigned int i = 0;
					i < fixs_ith_snmp::IF_NAME_MAX_LEN && i < var->val_len; i++)
				port_name[i] = tmp[i];
			snprintf(value, var->val_len + 1, "%s", var->val.string);
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "FOUND PORT NAME");

		} else
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
					"The PDU contains a port name info but the caller did not requested it");
	} else
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"The received PDU does not contain the expected port name OID");

	return 1;
}

int fixs_ith_getvlanNamecallback::read_smx(
		netsnmp_variable_list * var_list) const {
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Reading VLAN NAME from SMX");

	// Arrange the oid names
	const oid oid_name[] = { FIXS_ITH_SMX_OID_VLAN_STATIC_NAME, vlanId };

	return read_value(oid_name, FIXS_ITH_ARRAY_SIZE(oid_name), var_list);
}

int fixs_ith_getvlanNamecallback::read_value(const oid * oid_name,
		size_t oid_names_size, netsnmp_variable_list * var) const {
	char tmp_str[2 * 1024];

	if (fixs_ith_snmpmanager::oid_compare(oid_name, oid_names_size, var->name,
			var->name_length) == 0) {
		int char_printed =
				::snprintf(tmp_str, FIXS_ITH_ARRAY_SIZE(tmp_str),
						"Reading the following vlname  info: NET-SNMP variable printout --> ");
		snprint_variable(tmp_str + char_printed,
				FIXS_ITH_ARRAY_SIZE(tmp_str) - char_printed, var->name,
				var->name_length, var);
		FIXS_ITH_LOG(LOG_LEVEL_INFO, tmp_str);

		if (check_pdu_var_type(var->type, ASN_APPLICATION)) {
			if (var->type == 0x81 && var->val_len == 0) {
				FIXS_ITH_LOG(LOG_LEVEL_INFO,
						"The received PDU contains an vlan name value");
				return (error_code =
						fixs_ith_snmp::ERR_SNMP_PDU_VARIABLE_NULL_SIZE);
			}
			FIXS_ITH_LOG(LOG_LEVEL_WARN,
					"The received PDU has a bad type <type ==  %d>\n ",
					var->type);
			return 1;
		}
		if (vlan_name) {
			const unsigned char * tmp =
					reinterpret_cast<unsigned char *>(var->val.string);
			char value[fixs_ith_snmp::VLAN_NAME_MAX_LEN + 1] = { 0 };
			for (unsigned int i = 0;
					i < fixs_ith_snmp::VLAN_NAME_MAX_LEN && i < var->val_len;
					i++)
				vlan_name[i] = tmp[i];
			snprintf(value, var->val_len + 1, "%s", var->val.string);
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "FOUND VLAN NAME");

		} else
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
					"The PDU contains a vlan name info but the caller did not requested it");
	} else
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"The received PDU does not contain the expected vlan name OID");

	return 1;
}

int fixs_ith_get_if_mau_type_callback::read_smx(
		netsnmp_variable_list * var_list) const {
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
			"Reading 'interface MAU type value' from SMX");

	// Arrange the oid names
	const oid oid_name[] = { IF_MAU_TYPE_LIST_BITS, if_index, 1 };

	return read_value(oid_name, FIXS_ITH_ARRAY_SIZE(oid_name), var_list);
}

int fixs_ith_get_if_mau_type_callback::read_value(const oid * oid_name,
		size_t oid_names_size, netsnmp_variable_list * var) const {
	char tmp_str[2 * 1024];

	if (fixs_ith_snmpmanager::oid_compare(oid_name, oid_names_size, var->name,
			var->name_length) == 0) {
		int char_printed =
				::snprintf(tmp_str, FIXS_ITH_ARRAY_SIZE(tmp_str),
						"Reading the following 'if MAU type' info: NET-SNMP variable printout --> ");
		snprint_variable(tmp_str + char_printed,
				FIXS_ITH_ARRAY_SIZE(tmp_str) - char_printed, var->name,
				var->name_length, var);
		FIXS_ITH_LOG(LOG_LEVEL_INFO, tmp_str);

		if (check_pdu_var_type(var->type, ASN_OCTET_STR)) {
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
					"Call to 'check_pdu_var_type' failed: Received var-type == <%u>: expected type == <%u>: if_index == <%u>",
					var->type, ASN_OCTET_STR, if_index);
			return 1;
		}

		if (if_mau_type) {
			const uint8_t * octets =
					reinterpret_cast<uint8_t *>(var->val.string);
			const size_t octets_size = var->val_len;
			std::vector<bool> & mau_types = *if_mau_type;

			mau_types.clear();
			mau_types.reserve(octets_size << 3);

//			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "octets_size == <%lu>: (octets_size << 3) == <%lu>: mau_types.size() == <%lu>, mau_types.capacity() == <%lu>",
//					octets_size, (octets_size << 3), mau_types.size(), mau_types.capacity());

//			int j = 0;
			for (size_t i = 0; i < octets_size; ++i) {
				const uint16_t octet = static_cast<uint16_t>(*octets++);
				for (uint16_t mask = 0x0080; mask; mask >>= 1) {
//					bool value = ((octet & mask) ? true : false);

					mau_types.push_back((octet & mask) ? true : false);

//					FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "\n\ti == <%lu>: octet == <%02X>: mask == <%02X>: mau_types.size() == <%lu>: "
//							"mau_types[CURRENT] == <%u>: value == <%u>: (octet & mask) == <%04X>",
//							i, octet, mask, mau_types.size(), static_cast<unsigned>(mau_types[j++]), value, (octet & mask));
				}
			}

//			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "octets_size == <%lu>: (octets_size << 3) == <%lu>: mau_types.size() == <%lu>",
//					octets_size, (octets_size << 3), mau_types.size());
		} else
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
					"The PDU contains a 'if MAU type' value but the caller did not requested it");
	} else
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"The received PDU does not contain the 'IF MAU TYPE' value for if index <%u>",
				if_index);

	return 1;
}

int fixs_ith_get_if_mau_default_type_callback::read_smx(
		netsnmp_variable_list * var_list) const {
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
			"Reading 'interface MAU default type value' from SMX");

	// Arrange the oid names
	const oid oid_name[] = { IF_MAU_DEFAULT_TYPE, if_index, 1 };

	return read_value(oid_name, FIXS_ITH_ARRAY_SIZE(oid_name), var_list);
}

int fixs_ith_get_if_mau_default_type_callback::read_value(const oid * oid_name,
		size_t oid_names_size, netsnmp_variable_list * var) const {
	char tmp_str[2 * 1024];

	if (fixs_ith_snmpmanager::oid_compare(oid_name, oid_names_size, var->name,
			var->name_length) == 0) {
		int char_printed = ::snprintf(tmp_str, FIXS_ITH_ARRAY_SIZE(tmp_str),
				"Reading the following 'if MAU default type' info: "
						"NET-SNMP variable printout --> ");
		snprint_variable(tmp_str + char_printed,
				FIXS_ITH_ARRAY_SIZE(tmp_str) - char_printed, var->name,
				var->name_length, var);
		FIXS_ITH_LOG(LOG_LEVEL_INFO, tmp_str);

		if (check_pdu_var_type(var->type, ASN_OBJECT_ID)) {
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
					"Call to 'check_pdu_var_type' failed: Received var-type == <%u>: expected type == <%u>: if_index == <%u>",
					var->type, ASN_OBJECT_ID, if_index);
			return 1;
		}

		if (mau_default_type) {
			const oid * oid_octets = var->val.objid;
			const size_t octets_size = var->val_len;
			std::vector<oid> & mau_default_types = *mau_default_type;

//			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "SNMP INFO: var->name_length == <%lu>: var->val_len == <%lu>", var->name_length, var->val_len);

			mau_default_types.clear();
			mau_default_types.reserve(octets_size);

//			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "octets_size == <%lu>: mau_default_types.size() == <%lu>, mau_default_types.capacity() == <%lu>",
//					octets_size, mau_default_types.size(), mau_default_types.capacity());

			// Load the resulting vector with the obj id values
			if (octets_size >= 2)
				mau_default_types.assign(oid_octets,
						oid_octets
								+ ((*oid_octets | *(oid_octets + 1)) ? 9 : 2));

			FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
					"octets_size == <%lu>: mau_default_types.size() == <%lu>",
					octets_size, mau_default_types.size());
		} else
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
					"The PDU contains a 'if MAU default type' value but the caller did not requested it");
	} else
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"The received PDU does not contain the 'IF MAU DEFAULT TYPE' value for if index <%u>",
				if_index);

	return 1;
}

int fixs_ith_get_if_mau_auto_neg_admin_status_callback::read_smx(
		netsnmp_variable_list * var_list) const {
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
			"Reading 'interface MAU AUTO NEG ADMIN STATUS' from SMX");

	// Arrange the oid names
	const oid oid_name[] = { IF_MAU_AUTO_NEG_ADMIN_STATUS, if_index, 1 };

	return read_value(oid_name, FIXS_ITH_ARRAY_SIZE(oid_name), var_list);
}

int fixs_ith_get_if_mau_auto_neg_admin_status_callback::read_value(
		const oid * oid_name, size_t oid_names_size,
		netsnmp_variable_list * var) const {
	char tmp_str[2 * 1024];

	if (fixs_ith_snmpmanager::oid_compare(oid_name, oid_names_size, var->name,
			var->name_length) == 0) {
		int char_printed = ::snprintf(tmp_str, FIXS_ITH_ARRAY_SIZE(tmp_str),
				"Reading the following 'if MAU AUTO NEG ADMIN STATUS' info: "
						"NET-SNMP variable printout --> ");
		snprint_variable(tmp_str + char_printed,
				FIXS_ITH_ARRAY_SIZE(tmp_str) - char_printed, var->name,
				var->name_length, var);
		FIXS_ITH_LOG(LOG_LEVEL_INFO, tmp_str);

		if (check_pdu_var_type(var->type, ASN_INTEGER)) {
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
					"Call to 'check_pdu_var_type' failed: Received var-type == <%u>: expected type == <%u>: if_index == <%u>",
					var->type, ASN_INTEGER, if_index);
			return 1;
		}

		if (status) {
			*status = static_cast<int>(*(var->val.integer));

			FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
					"SNMP INFO: var->name_length == <%lu>: var->val_len == <%lu>: status == <%d>",
					var->name_length, var->val_len, *status);
		} else
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
					"The PDU contains a 'if MAU auto neg admin status' value but the caller did not requested it");
	} else
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"The received PDU does not contain the 'IF MAU AUTO NEG ADMIN STATUS' value for if index <%u>",
				if_index);

	return 1;
}

int fixs_ith_checkoidcallback::read_smx(
		netsnmp_variable_list * var_list) const {
	return read_value(oid_name, oid_name_size, var_list);
}

int fixs_ith_checkoidcallback::read_value(const oid * oid_name,
		size_t oid_names_size, netsnmp_variable_list * var) const {
	char tmp_str[2 * 1024];

	if (fixs_ith_snmpmanager::oid_compare(oid_name, oid_names_size, var->name,
			var->name_length) == 0) {
		int char_printed = ::snprintf(tmp_str, FIXS_ITH_ARRAY_SIZE(tmp_str),
				"Reading the following info: NET-SNMP variable printout --> ");
		snprint_variable(tmp_str + char_printed,
				FIXS_ITH_ARRAY_SIZE(tmp_str) - char_printed, var->name,
				var->name_length, var);
		FIXS_ITH_LOG(LOG_LEVEL_INFO, tmp_str);

		FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
				"The received PDU variable has varType == %02x", var->type);
		if (var->type == 0x81) {
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
					"The received PDU variable refers: 'No Such Instance currently exists at this OID type 'variable type == %d",
					var->type);
			error_code = fixs_ith_snmp::ERR_SNMP_PDU_VARIABLE_NO_SUCH_INSTANCE;
		} else if (var->type == 0x80) {
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG,
					"The received PDU variable refers: 'No Such Object available on this agent at this OID type 'variable type == %d",
					var->type);
			error_code = fixs_ith_snmp::ERR_SNMP_PDU_VARIABLE_NO_SUCH_OBJECT;
		}
	} else {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"The received PDU response does not refer to the expected OID");
	}

	return 1;
}

/*
 * Callback function to read the container file name for CMX boards.
 * TO DO : A new separate callback shall be written for CMX, for
 * GEP2/GEP5 HW Swap feature, smx callback is used also for CMX
 */

int fixs_ith_getcontainerfilename::read_smx(
		netsnmp_variable_list * var_list) const {
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Reading Container file name on CMXB");

	// Arrange the oid names
	const oid oid_name[] = { FIXS_ITH_CMX_CONT_FILE_NAME, port_Index };

	return read_value(oid_name, FIXS_ITH_ARRAY_SIZE(oid_name), var_list);
}

int fixs_ith_getcontainerfilename::read_value(const oid * oid_name,
		size_t oid_names_size, netsnmp_variable_list * var) const {
	char tmp_str[2 * 1024];

	if (fixs_ith_snmpmanager::oid_compare(oid_name, oid_names_size, var->name,
			var->name_length) == 0) {
		int char_printed =
				::snprintf(tmp_str, FIXS_ITH_ARRAY_SIZE(tmp_str),
						"Reading the following interface info: NET-SNMP variable printout --> ");
		snprint_variable(tmp_str + char_printed,
				FIXS_ITH_ARRAY_SIZE(tmp_str) - char_printed, var->name,
				var->name_length, var);
		FIXS_ITH_LOG(LOG_LEVEL_INFO, tmp_str);

		if (check_pdu_var_type(var->type, ASN_OCTET_STR)) {
			if (var->type == 0x81 && var->val_len == 0) {
				FIXS_ITH_LOG(LOG_LEVEL_INFO,
						"The received PDU contains container name value as empty");
				return (error_code =
						fixs_ith_snmp::ERR_SNMP_PDU_VARIABLE_NULL_SIZE);
			}
			FIXS_ITH_LOG(LOG_LEVEL_WARN,
					"The received PDU has a bad type <type ==  %d>\n ",
					var->type);
			return 1;
		}
		const unsigned char * tmp =
				reinterpret_cast<unsigned char *>(var->val.string);
		fileName = std::string((char *) tmp);
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "FOUND CONTAINER NAME:%s",
				fileName.c_str());
		char value[fixs_ith_snmp::CONTAINER_FILE_NAME_MAX_VALUE + 1] = { 0 };
		snprintf(value, var->val_len + 1, "%s", var->val.string);
		fileName = std::string((char *) value);
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, " FOUND CONTAINER NAME:%s",
				fileName.c_str());
	} else
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"The received PDU does not contain the expected container name OID");

	return 1;
}

//TR_HY52528 BEGIN
int fixs_ith_get_shelf_manager_control_state_callback::read_smx (netsnmp_variable_list * var_list) const {
	FIXS_ITH_LOG(LOG_LEVEL_WARN, "Reading shelf manager control state from SMX");

	// Arrange the oid names
	const oid oid_name [] = { FIXS_ITH_SMX_OID_SHELF_CTRL_MGR };

	return read_value(oid_name, FIXS_ITH_ARRAY_SIZE(oid_name), var_list);
}

int fixs_ith_get_shelf_manager_control_state_callback::read_value (const oid * oid_name, size_t oid_names_size, netsnmp_variable_list * var) const {
	char tmp_str [2 * 1024];

	if (fixs_ith_snmpmanager::oid_compare(oid_name, oid_names_size, var->name, var->name_length) == 0) {
		int char_printed = ::snprintf(tmp_str, FIXS_ITH_ARRAY_SIZE(tmp_str), "Reading the following 'shelf manager control state' info: NET-SNMP variable printout --> ");
		snprint_variable(tmp_str + char_printed, FIXS_ITH_ARRAY_SIZE(tmp_str) - char_printed, var->name, var->name_length, var);
		FIXS_ITH_LOG(LOG_LEVEL_WARN, tmp_str);

		if (check_pdu_var_type(var->type, ASN_OCTET_STR)) {
			FIXS_ITH_LOG(LOG_LEVEL_WARN, "The received PDU has a bad type <type ==  %d>\n ", var->type);
			return 1;
		}

		char *pchBuffer = reinterpret_cast<char*>(var->val.string);
		snprint_hexstring(pchBuffer, SNMP_MAXBUF, var->val.string, var->val_len);
		std::string shelfMgrCtrlState = pchBuffer;

		if (shelfMgrCtrlState.length() >= 2)
		{
			std::string tmp = shelfMgrCtrlState.substr(0,2);
			*shelf_manager_control_state = ::atoi(tmp.c_str());
		}
		else
		{
			*shelf_manager_control_state = ::atoi(shelfMgrCtrlState.c_str());
		}

	} else FIXS_ITH_LOG(LOG_LEVEL_ERROR, "The received PDU does not contain the expected Shelf Manager OID");

	return 1;
}
//TR_HY52528 END
