/*
 * fixs_ith_snmpmanagercallbacks.h
 *
 *  Created on: Feb 1, 2016
 *      Author: xassore
 */

#ifndef ITHADM_CAA_INC_FIXS_ITH_SNMPMANAGERCALLBACKS_H_
#define ITHADM_CAA_INC_FIXS_ITH_SNMPMANAGERCALLBACKS_H_


#include <iostream>
#include "fixs_ith_snmpmanager.h"
#include "fixs_ith_snmpsessioncallback.h"
#include "fixs_ith_programmacros.h"
#include "fixs_ith_logger.h"


class fixs_ith_snmploggercallback : public fixs_ith_snmpsessioncallback {
	//==============//
	// Constructors //
	//==============//
public:
	inline explicit fixs_ith_snmploggercallback (const char * message_ = 0) : fixs_ith_snmpsessioncallback(), message(message_) {}

	inline fixs_ith_snmploggercallback (
			const char * board_magazine_str_,
			int32_t board_slot_,
			const char * switch_board_ip_
	)
	: fixs_ith_snmpsessioncallback(board_magazine_str_, board_slot_, switch_board_ip_), message(0) {}

	inline fixs_ith_snmploggercallback (
			const char * message_,
			const char * board_magazine_str_,
			int32_t board_slot_,
			const char * switch_board_ip_
	)
	: fixs_ith_snmpsessioncallback(board_magazine_str_, board_slot_, switch_board_ip_), message(message_) {}


	//============//
	// Destructor //
	//============//
public:
	/** @brief fixs_ith_snmpmanager Destructor
	 */
	virtual inline ~fixs_ith_snmploggercallback () {}


	//===========//
	// Functions //
	//===========//
protected:

	virtual int read_smx (netsnmp_variable_list * var_list) const;

	//===========//
	// Operators //
	//===========//
public:
//	virtual int operator() (int operation, netsnmp_session * session, int request_id, netsnmp_pdu * pdu) const;


	//========//
	// Fields //
	//========//
public:
	const char * message;
};


/*******************************************************************************
* This class has the callback funtions to read operative/administrative port status from the mib.
*/
class fixs_ith_getportstatuscallback : public fixs_ith_snmpsessioncallback {
	//==============//
	// Constructors //
	//==============//
public:
	inline fixs_ith_getportstatuscallback (
			fixs_ith_snmp::port_status_t & port_status_, fixs_ith_snmp::status_type_t  status_type_,uint16_t port_ifIndex_,
			const char * board_magazine_str_,int32_t board_slot_, const char * switch_board_ip_
	)
	: fixs_ith_snmpsessioncallback(board_magazine_str_,board_slot_, switch_board_ip_),
	  port_status(&port_status_), status_type(status_type_),port_ifIndex(port_ifIndex_){}
	//============//
	// Destructor //
	//============//
public:
	virtual inline ~fixs_ith_getportstatuscallback () {}

    //===========//
	// Functions //
	//===========//
protected:

	virtual int read_smx (netsnmp_variable_list * var_list) const;

private:
	int read_value (const oid * oid_name, size_t oid_names_size, netsnmp_variable_list * var) const;

	//========//
	// Fields //
	//========//
public:
	fixs_ith_snmp::port_status_t * port_status;
 	fixs_ith_snmp::status_type_t  status_type;
 	uint16_t port_ifIndex;

};

//@
/*******************************************************************************
* This class has the callback funtions to read pim presence status of an interface from the IF-MIB.
*/
class fixs_ith_getpimpresencecallback : public fixs_ith_snmpsessioncallback {
	//==============//
	// Constructors //
	//==============//
public:
	inline fixs_ith_getpimpresencecallback (
			int16_t & port_presencestat_, uint16_t interfaceIdx_,
			const char * board_magazine_str_, int32_t slot_number_, const char * switch_board_ip_
	)
	: fixs_ith_snmpsessioncallback(board_magazine_str_, slot_number_ , switch_board_ip_),
	  port_presencestat(& port_presencestat_), interface_num(interfaceIdx_){}
	//============//
	// Destructor //
	//============//
public:
	virtual inline ~fixs_ith_getpimpresencecallback () {}

    //===========//
	// Functions //
	//===========//
protected:

	virtual int read_smx(netsnmp_variable_list * var_list) const;

private:
	int read_value (const oid * oid_name, size_t oid_names_size, netsnmp_variable_list * var) const;

	//========//
	// Fields //
	//========//
public:
	int16_t * port_presencestat;
	int16_t interface_num;
};



class fixs_ith_getpimmatchingcallback : public fixs_ith_snmpsessioncallback {
	//==============//
	// Constructors //
	//==============//
public:
	inline fixs_ith_getpimmatchingcallback (
			int16_t & port_matchingstat_, uint16_t interfaceIdx_,
			const char * board_magazine_str_, int32_t slot_number_, const char * switch_board_ip_
	)
	: fixs_ith_snmpsessioncallback(board_magazine_str_, slot_number_ , switch_board_ip_),
	  port_matchingstat(& port_matchingstat_), interface_num(interfaceIdx_){}
	//============//
	// Destructor //
	//============//
public:
	virtual inline ~fixs_ith_getpimmatchingcallback () {}

    //===========//
	// Functions //
	//===========//
protected:

	virtual int read_smx(netsnmp_variable_list * var_list) const;

private:
	int read_value (const oid * oid_name, size_t oid_names_size, netsnmp_variable_list * var) const;

	//========//
	// Fields //
	//========//
public:
	int16_t * port_matchingstat;
	int16_t interface_num;
};
/*******************************************************************************
* This class has the callback funtions to read default vlan id (pvid) of the interfceIdx from the mib.
*/

class fixs_ith_getdefaultVlanidcallback : public fixs_ith_snmpsessioncallback {
	//==============//
	// Constructors //
	//==============//
public:
	inline fixs_ith_getdefaultVlanidcallback (
			fixs_ith_snmp::qBridge::pvid_t & pvid_,  uint16_t port_number_,
			const char * board_magazine_str_, int32_t slot_number_, const char * switch_board_ip_
	)
	: fixs_ith_snmpsessioncallback(board_magazine_str_, slot_number_ , switch_board_ip_), interface_pvid(&pvid_),
	  port_number(port_number_)
	{}
	//============//
	// Destructor //
	//============//
public:
	virtual inline ~fixs_ith_getdefaultVlanidcallback () {}

    //===========//
	// Functions //
	//===========//
protected:

	virtual int read_smx(netsnmp_variable_list * var_list) const;

private:
	int read_value (const oid * oid_name, size_t oid_names_size, netsnmp_variable_list * var) const;

	//========//
	// Fields //
	//========//
public:
	fixs_ith_snmp::qBridge::pvid_t * interface_pvid;
	uint16_t port_number;
};

/*******************************************************************************
* This class has the callback funtions to read physical address of an interface from the IF-MIB.
*/

class fixs_ith_getportphysaddresscallback : public fixs_ith_snmpsessioncallback {
	//==============//
	// Constructors //
	//==============//
public:
	inline fixs_ith_getportphysaddresscallback (
			fixs_ith_snmp::mac_address_t & macaddress_,  uint16_t interfaceIdx_,
			const char * board_magazine_str_, int32_t slot_number_, const char * switch_board_ip_
	)
	: fixs_ith_snmpsessioncallback(board_magazine_str_, slot_number_ , switch_board_ip_), port_address(macaddress_),
	  port_ifIndex(interfaceIdx_)
	{}
	//============//
	// Destructor //
	//============//
public:
	virtual inline ~fixs_ith_getportphysaddresscallback () {}

    //===========//
	// Functions //
	//===========//
protected:

	virtual int read_smx(netsnmp_variable_list * var_list) const;

private:
	int read_value (const oid * oid_name, size_t oid_names_size, netsnmp_variable_list * var) const;

	//========//
	// Fields //
	//========//
public:
	fixs_ith_snmp::mac_address_t & port_address;
	uint16_t port_ifIndex;
};

class fixs_ith_get_number_of_ports_callback : public fixs_ith_snmpsessioncallback {
	//==============//
	// Constructors //
	//==============//
public:
	inline fixs_ith_get_number_of_ports_callback (
			int & number_of_ports_ ,
			fixs_ith_snmp::port_type_t port_type_,
			const char * board_magazine_str_,
			int32_t switch_board_slot,
			const char * switch_board_ip_
	)
	: fixs_ith_snmpsessioncallback(board_magazine_str_, switch_board_slot, switch_board_ip_),
	  number_of_ports(& number_of_ports_),
	  port_type (port_type_)
	{}
	//============//
	// Destructor //
	//============//
public:
	virtual inline ~fixs_ith_get_number_of_ports_callback () {}

    //===========//
	// Functions //
	//===========//
protected:

	virtual int read_smx(netsnmp_variable_list * var_list) const;

private:
	int read_value (const oid * oid_name, size_t oid_names_size, netsnmp_variable_list * var) const;

	//========//
	// Fields //
	//========//
public:
	int * number_of_ports;
	fixs_ith_snmp::port_type_t port_type;

};


/*******************************************************************************
* This class has the callback funtions to read max frame_size of a specific port.
*/
class fixs_ith_get_eth_frame_size_callback : public fixs_ith_snmpsessioncallback {
	//==============//
	// Constructors //
	//==============//
public:
	inline fixs_ith_get_eth_frame_size_callback (
			uint32_t & eth_max_frame_size_ ,
			uint16_t if_index_,
			const char * board_magazine_str_,
			int32_t switch_board_slot,
			const char * switch_board_ip_
	)
	: fixs_ith_snmpsessioncallback(board_magazine_str_, switch_board_slot, switch_board_ip_), eth_max_frame_size(& eth_max_frame_size_), if_index(if_index_)
	{}
	//============//
	// Destructor //
	//============//
public:
	virtual inline ~fixs_ith_get_eth_frame_size_callback () {}

    //===========//
	// Functions //
	//===========//
protected:

	virtual int read_smx(netsnmp_variable_list * var_list) const;

private:
	int read_value (const oid * oid_name, size_t oid_names_size, netsnmp_variable_list * var) const;

	//========//
	// Fields //
	//========//
public:
	uint32_t * eth_max_frame_size;
	uint16_t if_index;
};

/*******************************************************************************
* This class has the callback funtion to read eth duplex status of a specific port.
*/
class fixs_ith_get_eth_duplex_status_callback : public fixs_ith_snmpsessioncallback {
	//==============//
	// Constructors //
	//==============//
public:
	inline fixs_ith_get_eth_duplex_status_callback (
			uint16_t & eth_duplex_status_ ,
			uint16_t if_index_,
			const char * board_magazine_str_,
			int32_t switch_board_slot,
			const char * switch_board_ip_
	)
	: fixs_ith_snmpsessioncallback(board_magazine_str_, switch_board_slot, switch_board_ip_), eth_duplex_status(& eth_duplex_status_), if_index(if_index_)
	{}
	//============//
	// Destructor //
	//============//
public:
	virtual inline ~fixs_ith_get_eth_duplex_status_callback () {}

    //===========//
	// Functions //
	//===========//
protected:

	virtual int read_smx(netsnmp_variable_list * var_list) const;

private:
	int read_value (const oid * oid_name, size_t oid_names_size, netsnmp_variable_list * var) const;

	//========//
	// Fields //
	//========//
public:
	uint16_t * eth_duplex_status;
	uint16_t if_index;
};


/*******************************************************************************
* This class has the callback funtion to read PHY master/slave status of a specificed port.
*/
class fixs_ith_get_phyMasterSlave_status_callback : public fixs_ith_snmpsessioncallback {
	//==============//
	// Constructors //
	//==============//
public:
	inline fixs_ith_get_phyMasterSlave_status_callback (
			uint16_t & phy_master_slave_status_ ,
			uint16_t if_index_,
			const char * board_magazine_str_,
			int32_t switch_board_slot,
			const char * switch_board_ip_
	)
	: fixs_ith_snmpsessioncallback(board_magazine_str_, switch_board_slot, switch_board_ip_), phy_master_slave_status(& phy_master_slave_status_), if_index(if_index_)
	{}
	//============//
	// Destructor //
	//============//
public:
	virtual inline ~fixs_ith_get_phyMasterSlave_status_callback () {}

    //===========//
	// Functions //
	//===========//
protected:

	virtual int read_smx(netsnmp_variable_list * var_list) const;

private:
	int read_value (const oid * oid_name, size_t oid_names_size, netsnmp_variable_list * var) const;

	//========//
	// Fields //
	//========//
public:
	uint16_t * phy_master_slave_status;
	uint16_t if_index;
};

/*******************************************************************************
* This class has the callback funtion to read the current administrative value
* of the Key for the specified aggregation Port.
*/
class fixs_ith_get_agg_actor_admin_key_callback  : public fixs_ith_snmpsessioncallback {
	//==============//
	// Constructors //
	//==============//
public:
	inline fixs_ith_get_agg_actor_admin_key_callback  (
			int32_t & agg_actor_admin_key ,
			uint16_t if_index_,
			const char * board_magazine_str_,
			int32_t switch_board_slot,
			const char * switch_board_ip_
	)
	: fixs_ith_snmpsessioncallback(board_magazine_str_, switch_board_slot, switch_board_ip_), agg_actor_admin_key(& agg_actor_admin_key), if_index(if_index_)
	{}
	//============//
	// Destructor //
	//============//
public:
	virtual inline ~fixs_ith_get_agg_actor_admin_key_callback  () {}

    //===========//
	// Functions //
	//===========//
protected:

	virtual int read_smx(netsnmp_variable_list * var_list) const;

private:
	int read_value (const oid * oid_name, size_t oid_names_size, netsnmp_variable_list * var) const;

	//========//
	// Fields //
	//========//
public:
	int32_t * agg_actor_admin_key;
	uint16_t if_index;
};

/*******************************************************************************
* This class has the callback funtions to read interface (port) name.
*/

class fixs_ith_getportNamecallback : public fixs_ith_snmpsessioncallback {
	//==============//
	// Constructors //
	//==============//
public:
	inline fixs_ith_getportNamecallback (
			char (& port_name_) [fixs_ith_snmp::IF_NAME_MAX_LEN ], // 32 ASCII characters with null end character provided by the caller
	        uint16_t if_index_,
			const char * board_magazine_str_,
			int32_t board_slot_,
			const char * switch_board_ip_
	)
	: fixs_ith_snmpsessioncallback(board_magazine_str_, board_slot_, switch_board_ip_),
		port_name(port_name_), if_index(if_index_){}

	//============//
	// Destructor //
	//============//
public:
	virtual inline ~fixs_ith_getportNamecallback () {}


	//===========//
	// Functions //
	//===========//
protected:

	//	int read_boarddata_scx (netsnmp_variable_list * var_list) const;
	virtual int read_smx (netsnmp_variable_list * var_list) const;

private:
	int read_value (const oid * oid_name, size_t oid_names_size, netsnmp_variable_list * var) const;
	//===========//
	// Operators //
	//===========//
public:
	//========//
	// Fields //
	//========//
public:

	char (& port_name) [fixs_ith_snmp::IF_NAME_MAX_LEN];
	uint16_t if_index;
};


/*******************************************************************************
* This class has the callback funtions to read vlan name.
*/

class fixs_ith_getvlanNamecallback : public fixs_ith_snmpsessioncallback {
	//==============//
	// Constructors //
	//==============//
public:
	inline fixs_ith_getvlanNamecallback (
			char (& vlan_name_) [fixs_ith_snmp::VLAN_NAME_MAX_LEN + 1], // 32 ASCII characters with null end character provided by the caller
			uint16_t vlanId_,
			const char * board_magazine_str_,
			int32_t board_slot_,
			const char * switch_board_ip_
	)
	: fixs_ith_snmpsessioncallback(board_magazine_str_, board_slot_, switch_board_ip_),
		vlan_name(vlan_name_), vlanId(vlanId_){}

	//============//
	// Destructor //
	//============//
public:
	virtual inline ~fixs_ith_getvlanNamecallback () {}


	//===========//
	// Functions //
	//===========//
protected:

	//	int read_boarddata_scx (netsnmp_variable_list * var_list) const;
	virtual int read_smx (netsnmp_variable_list * var_list) const;

private:
	int read_value (const oid * oid_name, size_t oid_names_size, netsnmp_variable_list * var) const;
	//===========//
	// Operators //
	//===========//
public:
	//========//
	// Fields //
	//========//
public:

	char (& vlan_name) [fixs_ith_snmp::VLAN_NAME_MAX_LEN + 1];
	uint16_t vlanId;
};

/*******************************************************************************
* This class has the callback functions to read MAC address of a Bridge from the mib.
*/

class fixs_ith_get_brigeaddress_callback : public fixs_ith_snmpsessioncallback {
	//==============//
	// Constructors //
	//==============//
public:
	inline fixs_ith_get_brigeaddress_callback (
			fixs_ith_snmp::mac_address_t & mac_address,
			const char * board_magazine_str_,
			int32_t switch_board_slot,
			const char * switch_board_ip_
	)
	: fixs_ith_snmpsessioncallback (board_magazine_str_, switch_board_slot, switch_board_ip_), bridge_address(mac_address)
	{  }

	//============//
	// Destructor //
	//============//
public:
	virtual inline ~fixs_ith_get_brigeaddress_callback () {}

    //===========//
	// Functions //
	//===========//
protected:

	virtual int read_smx(netsnmp_variable_list * var_list) const;

private:
	int read_value (const oid * oid_name, size_t oid_names_size, netsnmp_variable_list * var) const;

	//========//
	// Fields //
	//========//
public:
	fixs_ith_snmp::mac_address_t & bridge_address;
};


class fixs_ith_get_number_of_vlans_callback : public fixs_ith_snmpsessioncallback {
	//==============//
	// Constructors //
	//==============//
public:
	inline fixs_ith_get_number_of_vlans_callback (
			uint32_t & number_of_vlans_ ,
			const char * board_magazine_str_,
			int32_t switch_board_slot,
			const char * switch_board_ip_
	)
	: fixs_ith_snmpsessioncallback(board_magazine_str_, switch_board_slot, switch_board_ip_), number_of_vlans(& number_of_vlans_)
	{}
	//============//
	// Destructor //
	//============//
public:
	virtual inline ~fixs_ith_get_number_of_vlans_callback () {}

    //===========//
	// Functions //
	//===========//
protected:

	virtual int read_smx(netsnmp_variable_list * var_list) const;

private:
	int read_value (const oid * oid_name, size_t oid_names_size, netsnmp_variable_list * var) const;

	//========//
	// Fields //
	//========//
public:
	uint32_t * number_of_vlans;
};

class fixs_ith_get_port_if_index_callback : public fixs_ith_snmpsessioncallback {
	//==============//
	// Constructors //
	//==============//
public:
	inline fixs_ith_get_port_if_index_callback (
			uint16_t & if_index_ ,
			uint16_t port_number_,
			const char * board_magazine_str_,
			int32_t switch_board_slot_,
			const char * switch_board_ip_
	)
	: fixs_ith_snmpsessioncallback(board_magazine_str_, switch_board_slot_, switch_board_ip_), if_index(& if_index_), port_number(port_number_)
	{}
	//============//
	// Destructor //
	//============//
public:
	virtual inline ~fixs_ith_get_port_if_index_callback () {}

    //===========//
	// Functions //
	//===========//
protected:

	virtual int read_smx(netsnmp_variable_list * var_list) const;

private:
	int read_value (const oid * oid_name, size_t oid_names_size, netsnmp_variable_list * var) const;

	//========//
	// Fields //
	//========//
public:
	uint16_t * if_index;
	uint16_t port_number;
};



class fixs_ith_get_interface_speed_callback : public fixs_ith_snmpsessioncallback {
	//==============//
	// Constructors //
	//==============//
public:
	inline fixs_ith_get_interface_speed_callback (
			uint32_t & speed_ ,
			uint16_t if_index_,
			const char * board_magazine_str_,
			int32_t switch_board_slot,
			const char * switch_board_ip_
	)
	: fixs_ith_snmpsessioncallback(board_magazine_str_, switch_board_slot, switch_board_ip_), if_speed(& speed_), if_index(if_index_)
	{}
	//============//
	// Destructor //
	//============//
public:
	virtual inline ~fixs_ith_get_interface_speed_callback () {}

    //===========//
	// Functions //
	//===========//
protected:

	virtual int read_smx(netsnmp_variable_list * var_list) const;

private:
	int read_value (const oid * oid_name, size_t oid_names_size, netsnmp_variable_list * var) const;

	//========//
	// Fields //
	//========//
public:
	uint32_t * if_speed;
	uint16_t if_index;
};

/*******************************************************************************
* This class has the callback funtion to read the interface MAU type value of a specific port.
*/
class fixs_ith_get_if_mau_type_callback : public fixs_ith_snmpsessioncallback {
	//==============//
	// Constructors //
	//==============//
public:
	inline fixs_ith_get_if_mau_type_callback (
			std::vector<bool> & if_mau_type_,
			uint16_t if_index_,
			const char * board_magazine_str_,
			int32_t switch_board_slot,
			const char * switch_board_ip_
	)
	: fixs_ith_snmpsessioncallback(board_magazine_str_, switch_board_slot, switch_board_ip_), if_mau_type(& if_mau_type_), if_index(if_index_)
	{}
	//============//
	// Destructor //
	//============//
public:
	virtual inline ~fixs_ith_get_if_mau_type_callback () {}

    //===========//
	// Functions //
	//===========//
protected:
	virtual int read_smx (netsnmp_variable_list * var_list) const;

private:
	int read_value (const oid * oid_name, size_t oid_names_size, netsnmp_variable_list * var) const;

	//========//
	// Fields //
	//========//
public:
	std::vector<bool> * if_mau_type;
	uint16_t if_index;
};

/*******************************************************************************
* This class has the callback funtion to read the interface MAU type default value of a specific port.
*/
class fixs_ith_get_if_mau_default_type_callback : public fixs_ith_snmpsessioncallback {
	//==============//
	// Constructors //
	//==============//
public:
	inline fixs_ith_get_if_mau_default_type_callback (
			std::vector<oid> & if_mau_default_type_,
			uint16_t if_index_,
			const char * board_magazine_str_,
			int32_t switch_board_slot,
			const char * switch_board_ip_
	)
	: fixs_ith_snmpsessioncallback(board_magazine_str_, switch_board_slot, switch_board_ip_),
		mau_default_type(& if_mau_default_type_), if_index(if_index_)
	{}
	//============//
	// Destructor //
	//============//
public:
	virtual inline ~fixs_ith_get_if_mau_default_type_callback () {}

    //===========//
	// Functions //
	//===========//
protected:
	virtual int read_smx (netsnmp_variable_list * var_list) const;

private:
	int read_value (const oid * oid_name, size_t oid_names_size, netsnmp_variable_list * var) const;

	//========//
	// Fields //
	//========//
public:
	std::vector<oid> * mau_default_type;
	uint16_t if_index;
};

/*******************************************************************************
* This class has the callback funtion to read the interface MAU AUTO NEG ADMIN STATUS value of a specific port.
*/
class fixs_ith_get_if_mau_auto_neg_admin_status_callback : public fixs_ith_snmpsessioncallback {
	//==============//
	// Constructors //
	//==============//
public:
	inline fixs_ith_get_if_mau_auto_neg_admin_status_callback (
			int & status_,
			uint16_t if_index_,
			const char * board_magazine_str_,
			int32_t switch_board_slot,
			const char * switch_board_ip_
	)
	: fixs_ith_snmpsessioncallback(board_magazine_str_, switch_board_slot, switch_board_ip_), status(& status_), if_index(if_index_)
	{}
	//============//
	// Destructor //
	//============//
public:
	virtual inline ~fixs_ith_get_if_mau_auto_neg_admin_status_callback () {}

    //===========//
	// Functions //
	//===========//
protected:
	virtual int read_smx (netsnmp_variable_list * var_list) const;

private:
	int read_value (const oid * oid_name, size_t oid_names_size, netsnmp_variable_list * var) const;

	//========//
	// Fields //
	//========//
public:
	int * status;
	uint16_t if_index;
};

template < int PORTLIST_ARRAY_SIZE >
class fixs_ith_get_portlist_callback : public fixs_ith_snmpsessioncallback {
	//==============//
	// Constructors //
	//==============//
public:
	inline fixs_ith_get_portlist_callback (
			uint8_t (& port_list_) [PORTLIST_ARRAY_SIZE],
			const oid * oid_name_,
			size_t oid_name_size_,
			const char * board_magazine_str_,
			int32_t switch_board_slot,
			const char * switch_board_ip_
	)
	: fixs_ith_snmpsessioncallback(board_magazine_str_, switch_board_slot, switch_board_ip_),
	  port_list(port_list_), oid_name(oid_name_), oid_name_size(oid_name_size_)
	{}
	//============//
	// Destructor //
	//============//
public:
	virtual inline ~fixs_ith_get_portlist_callback () {}

    //===========//
	// Functions //
	//===========//
protected:

	virtual int read_smx(netsnmp_variable_list * var_list) const
	{
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Reading an OID of type 'Port List' from SMX");
		return read_value(oid_name, oid_name_size, var_list);
	}

private:

	int read_value (const oid * oid_name, size_t oid_names_size, netsnmp_variable_list * var) const
	{
		// check that the arrived PDU response is relative to the expected OID
		if (fixs_ith_snmpmanager::oid_compare(oid_name, oid_names_size, var->name, var->name_length) == 0)
		{
			char tmp_str [2 * 1024];

			int char_printed = ::snprintf(tmp_str, FIXS_ITH_ARRAY_SIZE(tmp_str), "Reading the following 'PortList': NET-SNMP variable printout --> ");
			snprint_variable(tmp_str + char_printed, FIXS_ITH_ARRAY_SIZE(tmp_str) - char_printed, var->name, var->name_length, var);
			FIXS_ITH_LOG(LOG_LEVEL_INFO, tmp_str);

			// check type of PDU variable contained in the PDU response
			if(var->type == 0x81 ){
				FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "The received PDU variable refers: 'No Such Instance currently exists at this OID type 'variable type == %d", var->type);
				error_code = fixs_ith_snmp::ERR_SNMP_PDU_VARIABLE_NO_SUCH_INSTANCE;
				return 1;
			}
			else if( var->type == 0x80)
			{
				FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "The received PDU variable refers: 'No Such Object available on this agent at this OID type 'variable type == %d", var->type);
				error_code = fixs_ith_snmp::ERR_SNMP_PDU_VARIABLE_NO_SUCH_OBJECT;
				return 1;
			}
			else if (var->type != ASN_OCTET_STR)
			{ // ERROR: Unexpected type for the PDU variable
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "The received PDU variable is not an ASN_OCTET_STR type variable: var->type == %u", static_cast<unsigned>(var->type));
				error_code = fixs_ith_snmp::ERR_SNMP_PDU_VARIABLE_BAD_TYPE;
				return 1;
			}

			// Check the size of the data inside the PDU variable
			if (var->val_len > PORTLIST_ARRAY_SIZE) { // ERROR: data size not correct
				FIXS_ITH_LOG(LOG_LEVEL_ERROR,
						"The received PDU variable size is greater than expected size: var->val_len == %zu, expected size == %d",
						var->val_len, PORTLIST_ARRAY_SIZE);
				error_code = fixs_ith_snmp::ERR_SNMP_PDU_VARIABLE_BAD_SIZE;
				return 1;
			}

			// ok, fill the output variable with the data retrieved form PDU variable
			char * octet_string = reinterpret_cast<char *>(var->val.string);
			memset(port_list, 0, PORTLIST_ARRAY_SIZE);
			memcpy(port_list, octet_string, var->val_len);
		}
		else
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "The received PDU does not contain the expected OID");
		}

		return 1;
	}

	//========//
	// Fields //
	//========//
public:
	uint8_t (& port_list) [PORTLIST_ARRAY_SIZE];
	const oid * oid_name;
	size_t oid_name_size;
};


class fixs_ith_checkoidcallback : public fixs_ith_snmpsessioncallback {
	//==============//
	// Constructors //
	//==============//
public:
	inline fixs_ith_checkoidcallback (
			const oid * oid_name_,
			size_t oid_name_size_,
			const char * board_magazine_str_,
			int32_t switch_board_slot,
			const char * switch_board_ip_
	)
	: fixs_ith_snmpsessioncallback(board_magazine_str_, switch_board_slot, switch_board_ip_), oid_name(oid_name_), oid_name_size(oid_name_size_)
	{}
	//============//
	// Destructor //
	//============//
public:
	virtual inline ~fixs_ith_checkoidcallback () {}

    //===========//
	// Functions //
	//===========//
protected:

	virtual int read_smx(netsnmp_variable_list * var_list) const;

private:
	int read_value (const oid * oid_name, size_t oid_names_size, netsnmp_variable_list * var) const;

	//========//
	// Fields //
	//========//

public:
	const oid * oid_name;
	size_t oid_name_size;
};

/**************************************************************************************
* This class has the callback funtions to read container file name of CMXB from the mib.
****************************************************************************************/
class fixs_ith_getcontainerfilename : public fixs_ith_snmpsessioncallback {
        //==============//
        // Constructors //
        //==============//
public:
        inline fixs_ith_getcontainerfilename (
			std::string &fileName_, 
			uint16_t port_Index_,
                        const char * board_magazine_str_, int32_t board_slot_, const char * switch_board_ip_
        )
        : fixs_ith_snmpsessioncallback(board_magazine_str_,board_slot_, switch_board_ip_),
          fileName(fileName_), port_Index(port_Index_){}
        //============//
        // Destructor //
        //============//
public:
        virtual inline ~fixs_ith_getcontainerfilename () {}

    //===========//
        // Functions //
        //===========//
protected:

        virtual int read_smx (netsnmp_variable_list * var_list) const;

private:
        int read_value (const oid * oid_name, size_t oid_names_size, netsnmp_variable_list * var) const;

        //========//
        // Fields //
        //========//
public:
	std::string &fileName;
        uint16_t port_Index;

};

/*****************************************************************************
* This class has the callback funtions to fetch CMX port priority from the mib.
******************************************************************************/
class fixs_ith_getportdefaultuserpriority : public fixs_ith_snmpsessioncallback {
        //==============//
        // Constructors //
        //==============//
public:
        inline fixs_ith_getportdefaultuserpriority (
			uint32_t &priority_,
                        uint16_t port_Index_,
                        const char * board_magazine_str_, int32_t board_slot_, const char * switch_board_ip_
        )
        : fixs_ith_snmpsessioncallback(board_magazine_str_,board_slot_, switch_board_ip_),
          priority(&priority_), port_Index(port_Index_){}
        //============//
        // Destructor //
        //============//
public:
        virtual inline ~fixs_ith_getportdefaultuserpriority () {}

    //===========//
        // Functions //
        //===========//
protected:

        virtual int read_smx (netsnmp_variable_list * var_list) const;

private:
        int read_value (const oid * oid_name, size_t oid_names_size, netsnmp_variable_list * var) const;

        //========//
        // Fields //
        //========//
public:
	uint32_t * priority;
        uint16_t port_Index;

};

/*******************************************************************************
* This class has the callback funtion to read master/slave status of a specificed Board.
*/
//TR_HY52528   BEGIN
class fixs_ith_get_shelf_manager_control_state_callback : public fixs_ith_snmpsessioncallback {
       //==============//
       // Constructors //
       //==============//
public:
       inline fixs_ith_get_shelf_manager_control_state_callback (
                       uint8_t & shelf_manager_control_state_ ,
                       const char * board_magazine_str_,
                       int32_t switch_board_slot,
                       const char * switch_board_ip_
       )
       : fixs_ith_snmpsessioncallback(board_magazine_str_, switch_board_slot, switch_board_ip_), shelf_manager_control_state(&shelf_manager_control_state_)
       {}
       //============//
       // Destructor //
       //============//
public:
       virtual inline ~fixs_ith_get_shelf_manager_control_state_callback () {}

    //===========//
       // Functions //
       //===========//
protected:

       virtual int read_smx(netsnmp_variable_list * var_list) const;

private:
       int read_value (const oid * oid_name, size_t oid_names_size, netsnmp_variable_list * var) const;

       //========//
       // Fields //
       //========//
public:
       uint8_t * shelf_manager_control_state;
};
//TR_HY52528 END


#endif /* ITHADM_CAA_INC_FIXS_ITH_SNMPMANAGERCALLBACKS_H_ */
