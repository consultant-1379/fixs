/*
 * fixs_ith_snmpmanager.h
 *
 *  Created on: Feb 2, 2016
 *      Author: xassore
 */


#ifndef HEADER_GUARD_CLASS__fixs_ith_snmpmanager
#define HEADER_GUARD_CLASS__fixs_ith_snmpmanager fixs_ith_snmpmanager

#include <stddef.h>
#include <stdint.h>

#include <vector>
#include <set>

#include <ace/Recursive_Thread_Mutex.h>

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/library/transform_oids.h> // To get using SNMP Version 3

#include "fixs_ith_snmpconstants.h"
#include "fixs_ith_switchboardinfo.h"
#include "fixs_ith_snmpsessioncallback.h"
#include "fixs_ith_snmpmanagercallbacks.h"
#include "fixs_ith_snmpmibdefinitions.h"
#include <ace/Event.h>


using namespace std;

namespace axe_eth_shelf = fixs_ith_snmp::axe::eth::shelf;

/*
 * Forward declarations
 */

class fixs_ith_snmpsessionhandler;


class fixs_ith_snmpmanager {
	//==============//
	// Constructors //
	//==============//
public:
	/** @brief fixs_ith_snmpmanager constructor
	 */
	inline fixs_ith_snmpmanager ()
	{}

private:
	fixs_ith_snmpmanager (const fixs_ith_snmpmanager & rhs);


	//============//
	// Destructor //
	//============//
public:
	virtual inline ~fixs_ith_snmpmanager () {}


	//===========//
	// Functions //
	//===========//
public:
	static inline void initialize (const char * type) { init_snmp(type); }
	static inline void shutdown (const char * type) { snmp_shutdown(type); }

	int send_trap_ack (int switch_board_slot_position, const char * ap_ip, const char * switch_board_ip);

	/*****************************************
	* SNMP operations to handle CMX Logging *
	****************************************/
        int delete_container_file(fixs_ith_switchboardinfo & switch_board,
                        std::string file_name);

        int create_cont_transfer_server_ip(fixs_ith_switchboardinfo & switch_board,
                        const char * server_address);

        int create_cont_transfer_server_path(fixs_ith_switchboardinfo & switch_board,
                        const char * log_path);

        int set_cont_control_notifications(fixs_ith_switchboardinfo & switch_board,
                        int control_value);

        int set_cont_automatic_transfer(fixs_ith_switchboardinfo & switch_board,
                        int control_value);

        int set_cont_file_encryption(fixs_ith_switchboardinfo & switch_board,
                        int encryption_setting);

        int create_cont_file(fixs_ith_switchboardinfo & switch_board,
                        int value);

        /**********************************************
        * SNMP operations to set Port Priority on CMX *
        **********************************************/
	int set_mib_port_default_user_priority(fixs_ith_switchboardinfo & switch_board,
			uint16_t portIndex);


	/*
	***********************************
	* SNMP operations to handle VLANs *
	***********************************
	*/
	int create_vlan(fixs_ith_switchboardinfo & switch_board,
			uint16_t vlan_id, const char * vlan_name,
			fixs_ith::switchboard_plane_t sb_plane);
			
	int delete_vlan(fixs_ith_switchboardinfo & switch_board,
			uint16_t vlan_id,
			fixs_ith::switchboard_plane_t sb_plane);

	int check_vlan(const fixs_ith_switchboardinfo & switch_board,
			const fixs_ith::switchboard_plane_t sb_plane,
			const uint16_t vlan_id,
			const unsigned * timeout_ms = 0);

	int set_vlan_ports(fixs_ith_switchboardinfo & switch_board,
			uint16_t vlan_id,
			const std::set<uint16_t>& tagged_ports,
			const std::set<uint16_t>& untagged_ports,
			fixs_ith::switchboard_plane_t sb_plane);

	int remove_vlan_ports(fixs_ith_switchboardinfo & switch_board,
					uint16_t vlan_id,
					fixs_ith::switchboard_plane_t sb_plane);

	int get_vlan_ports(fixs_ith_snmp::if_bitmap_t & port_list,
				uint16_t vlan_id,
				fixs_ith_snmp::vlanPort_type_t port_type,
				fixs_ith_switchboardinfo & switch_board,
				fixs_ith::switchboard_plane_t sb_plane,
				const unsigned * timeout_ms);

	int get_vlan_ports(fixs_ith_snmp::if_bitmap_t & tagged_port_list,
				fixs_ith_snmp::if_bitmap_t & untagged_port_list,
				uint16_t vlan_id,
				fixs_ith_switchboardinfo & switch_board,
				fixs_ith::switchboard_plane_t sb_plane,
				const unsigned * timeout_ms);

	int get_port_default_vlan(fixs_ith_snmp::qBridge::pvid_t & pvid,
				uint16_t port_number,
				fixs_ith_switchboardinfo & switch_board,
				fixs_ith::switchboard_plane_t sb_plane,
				const unsigned * timeout_ms = 0);

	int get_port_default_vlan(fixs_ith_snmp::qBridge::pvid_t & pvid,
			uint16_t port_number,
			int32_t switch_board_slot,
			uint32_t switch_board_magazine,
			const char * switch_board_ip,
			const unsigned * timeout_ms);

	int get_vlan_name(char (& vlan_name) [fixs_ith_snmp::VLAN_NAME_MAX_LEN + 1],
			uint16_t vlan_id,
			fixs_ith_switchboardinfo & switch_board,
			fixs_ith::switchboard_plane_t sb_plane,
			const unsigned * timeout_ms = 0);

	int get_vlan_name(char (& vlan_name) [fixs_ith_snmp::VLAN_NAME_MAX_LEN + 1],
			uint16_t vlan_id,
			int32_t switch_board_slot,
			uint32_t switch_board_magazine,
			const char * switch_board_ip,
			const unsigned * timeout_ms);
	// *** END VLAN operations ***


	/*
	***********************************
	* SNMP operations to handle SUBNETS *
	***********************************
	*/
	int create_ipv4subnet(const fixs_ith_switchboardinfo & switch_board,
			const fixs_ith::switchboard_plane_t sb_plane,
			const std::string  & ipv4subnet);

	int delete_ipv4subnet(const fixs_ith_switchboardinfo & switch_board,
			const fixs_ith::switchboard_plane_t sb_plane,
			const std::string & ipv4subnet);

	int set_ipv4subnet_vid(const fixs_ith_switchboardinfo & switch_board,
			const fixs_ith::switchboard_plane_t sb_plane,
			const std::string  & ipv4subnet,
			const uint16_t vid);

	int check_ipv4subnet(const fixs_ith_switchboardinfo & switch_board,
				const fixs_ith::switchboard_plane_t sb_plane,
				const std::string  & ipv4subnet,
				const unsigned * timeout_ms = 0);
	//*** END SUBNET operations ***


	/*
	***********************************
	* SNMP operations to handle LAGs **
	***********************************
	*/
	int create_lag(fixs_ith_switchboardinfo & switch_board,
			uint16_t lag_ifIndex, const char * lag_name,
			fixs_ith::switchboard_plane_t sb_plane);

	int add_lag_ports(fixs_ith_switchboardinfo & switch_board,
		    uint16_t lag_ifIndex, const std::set<uint16_t>& port_if_list,
			fixs_ith::switchboard_plane_t sb_plane);

	// Get the ports that are currently members of a LAG
	int get_lag_operMembers(
			fixs_ith_snmp::if_bitmap_t & port_list,
			uint16_t lag_if_index,
			fixs_ith_switchboardinfo & switch_board,
			fixs_ith::switchboard_plane_t sb_plane,
			const unsigned * timeout_ms = 0);

	int get_lag_operMembers(
			fixs_ith_snmp::if_bitmap_t & port_list,
			uint16_t lag_if_index,
			int32_t switch_board_slot,
			uint32_t switch_board_magazine,
			const char * switch_board_ip,
			const unsigned * timeout_ms = 0);

	int check_lag (const fixs_ith_switchboardinfo & switch_board,
			const fixs_ith::switchboard_plane_t sb_plane,
			const uint16_t lag_if_index,
			const unsigned * timeout_ms = 0);

	//*** END LAG operations ***

	/*
	*****************************************************
	* SNMP operations to handle BRIDGEs and INTERFACES **
	*****************************************************
	*/

	// Get the bridge address (6 bytes) of the CONTROL or TRANSPORT bridge hosted by a SMX board
	int get_bridge_address (
			fixs_ith_snmp::mac_address_t & bridge_address,
			fixs_ith_switchboardinfo & switch_board,
			fixs_ith::switchboard_plane_t sb_plane,
			const unsigned * timeout_ms = 0);

	int get_bridge_address (
			fixs_ith_snmp::mac_address_t & bridge_address,
			int32_t switch_board_slot,
			uint32_t switch_board_magazine,
			const char * switch_board_ip,
			const unsigned * timeout_ms = 0);

	int get_bridge_address (
			fixs_ith_snmpsessionhandler & session_handler,
			fixs_ith_snmpsessioncallback & callback,
			const oid * oid_name,
			size_t oid_name_size,
			const unsigned * timeout_ms = 0);

	// Get the number of vlans defined on the CONTROL or the TRANSPORT switch of an SMX board
	int get_number_of_vlans (
			uint32_t & num_of_vlans,
			fixs_ith_switchboardinfo & switch_board,
			fixs_ith::switchboard_plane_t sb_plane,
			const unsigned * timeout_ms = 0);

	int get_number_of_vlans (
			uint32_t & num_of_vlans,
			int32_t switch_board_slot,
			uint32_t switch_board_magazine,
			const char * switch_board_ip,
			const unsigned * timeout_ms = 0);

	int get_number_of_vlans (
			fixs_ith_snmpsessionhandler & session_handler,
			fixs_ith_snmpsessioncallback & callback,
			const oid * oid_name,
			size_t oid_name_size,
			const unsigned * timeout_ms = 0);

	// Get the number of network interfaces on a bridge
	int get_network_interfaces_number (
				int32_t & if_number,
				fixs_ith_switchboardinfo & switch_board,
				fixs_ith::switchboard_plane_t sb_plane,
				const unsigned * timeout_ms = 0);

	int get_network_interfaces_number (
				int32_t & if_number,
				uint32_t board_magazine,
				int32_t switch_board_slot,
				const char * switch_board_ip,
				const unsigned * timeout_ms = 0);

	// Get 'operational status' of an interface
	int get_port_operational_status (
			fixs_ith_snmp::port_status_t & port_status,
			uint16_t port_ifIndex,
			fixs_ith_switchboardinfo & switch_board,
			fixs_ith::switchboard_plane_t sb_plane,
			const unsigned * timeout_ms = 0);

	int get_port_operational_status (
			fixs_ith_snmp::port_status_t & port_status,
			uint16_t port_ifIndex,
			uint32_t board_magazine,
			int32_t switch_board_slot,
			const char * switch_board_ip,
			const unsigned * timeout_ms = 0);

	int get_port_operational_status (
			fixs_ith_snmpsessionhandler & session_handler,
			fixs_ith_snmpsessioncallback & callback,
			const oid * oid_name,
			size_t oid_name_size,
			const unsigned * timeout_ms = 0);

	// Get 'administrative status' of an interface
	int get_port_admin_status (
			fixs_ith_snmp::port_status_t & port_status,
			uint16_t port_ifIndex,
			fixs_ith_switchboardinfo & switch_board,
			fixs_ith::switchboard_plane_t sb_plane,
			const unsigned * timeout_ms = 0);

	int get_port_admin_status (
			fixs_ith_snmp::port_status_t & port_status,
			uint16_t port_ifIndex,
			uint32_t board_magazine,
			int32_t switch_board_slot,
			const char * switch_board_ip,
			const unsigned * timeout_ms = 0);
	int get_cont_file_name( 
			fixs_ith_switchboardinfo & switch_board,
			fixs_ith::switchboard_plane_t sb_plane,
			uint16_t port_Index,
			std::string &fileName,
			const unsigned * timeout_ms = 0);
						

	int get_cont_file_name( 
			uint16_t port_Index,
			uint32_t board_magazine,
			int32_t switch_board_slot,
			const char * switch_board_ip,
			std::string &fileName,
			const unsigned * timeout_ms = 0);

	int get_cont_file_name (
                        fixs_ith_snmpsessionhandler & session_handler,
                        fixs_ith_snmpsessioncallback & callback,
                        const oid * oid_name,
                        size_t oid_name_size,
                        const unsigned * timeout_ms = 0);

	int get_mib_object(
			fixs_ith_snmpsessionhandler & session_handler,
			fixs_ith_snmpsessioncallback & callback,
			const oid * oid_name,
			size_t oid_name_size,
			const unsigned * timeout_ms = 0);

	int get_pimPresence_status (
			int16_t & port_presencestatus,
			uint32_t board_magazine,
			int32_t board_slot,
			const char * switch_board_ip,
			const unsigned * timeout_ms,int16_t interface_num);

	int get_pimMatching_status (
			int16_t & port_matchingstatus,
			uint32_t board_magazine,
			int32_t board_slot,
			int16_t interface_num,
			const char * switch_board_ip,
			const unsigned * timeout_ms);//,const std::string & switchboard_key);

	int raiseCeasePimAlarm(
			int if_index,
			int16_t & port_presencestatus,
			int16_t & port_matchingstatus,
			const std::string & switchboard_key);

	// Get the number of ports defined on the CONTROL or the TRANSPORT switch of an SMX board
	int get_number_of_ports(
			int & num_of_ports,
			fixs_ith_switchboardinfo & switch_board,
			fixs_ith::switchboard_plane_t sb_plane,
			const unsigned * timeout_ms = 0);

	int get_number_of_ports (
			int & num_of_ports,
			int32_t switch_board_slot,
			uint32_t switch_board_magazine,
			const char * switch_board_ip,
			const unsigned * timeout_ms = 0);

	int get_number_of_ports (
			fixs_ith_snmpsessionhandler & session_handler,
			fixs_ith_snmpsessioncallback & callback,
			const oid * oid_name,
			size_t oid_name_size,
			const unsigned * timeout_ms = 0);

	// Get the maxFrameSize defined on the CONTROL or the TRANSPORT switch of an SMX board
	int get_eth_max_frame_size (
			uint32_t & eth_frame_size,
			uint16_t if_index,
			fixs_ith_switchboardinfo & switch_board,
			fixs_ith::switchboard_plane_t sb_plane,
			const unsigned * timeout_ms = 0);

	int get_eth_max_frame_size (
			uint32_t & eth_frame_size,
			uint16_t if_index,
			int32_t switch_board_slot,
			uint32_t switch_board_magazine,
			const char * switch_board_ip,
			const unsigned * timeout_ms = 0);

	int get_eth_max_frame_size (
			fixs_ith_snmpsessionhandler & session_handler,
			fixs_ith_snmpsessioncallback & callback,
			const oid * oid_name,
			size_t oid_name_size,
			const unsigned * timeout_ms = 0);

	int get_eth_duplex_status (
			uint16_t & eth_duplex_status,
			uint16_t if_index,
			fixs_ith_switchboardinfo & switch_board,
			fixs_ith::switchboard_plane_t sb_plane,
			const unsigned * timeout_ms = 0);

	int get_eth_duplex_status (
			uint16_t & eth_duplex_status,
			uint16_t if_index,
			int32_t switch_board_slot,
			uint32_t switch_board_magazine,
			const char * switch_board_ip,
			const unsigned * timeout_ms = 0);

	int get_eth_duplex_status (
				fixs_ith_snmpsessionhandler & session_handler,
				fixs_ith_snmpsessioncallback & callback,
				const oid * oid_name,
				size_t oid_name_size,
				const unsigned * timeout_ms = 0);

	int get_phy_masterslave_status (
			uint16_t & masterslave_status,
			uint16_t if_index,
			fixs_ith_switchboardinfo & switch_board,
			fixs_ith::switchboard_plane_t sb_plane,
			const unsigned * timeout_ms = 0);

	int get_phy_masterslave_status (
			uint16_t & masterslave_status,
			uint16_t if_index,
			int32_t switch_board_slot,
			uint32_t switch_board_magazine,
			const char * switch_board_ip,
			const unsigned * timeout_ms = 0);

	int get_phy_masterslave_status (
				fixs_ith_snmpsessionhandler & session_handler,
				fixs_ith_snmpsessioncallback & callback,
				const oid * oid_name,
				size_t oid_name_size,
				const unsigned * timeout_ms = 0);

	// Get the aggActorAdminKey
	int get_agg_actor_admin_key (
			int32_t & agg_actor_admin_key,
			uint16_t if_index,
			fixs_ith_switchboardinfo & switch_board,
			fixs_ith::switchboard_plane_t sb_plane,
			const unsigned * timeout_ms = 0);

	int get_agg_actor_admin_key (
			int32_t & agg_actor_admin_key,
			uint16_t if_index,
			int32_t switch_board_slot,
			uint32_t switch_board_magazine,
			const char * switch_board_ip,
			const unsigned * timeout_ms = 0);

	int get_agg_actor_admin_key (
			fixs_ith_snmpsessionhandler & session_handler,
			fixs_ith_snmpsessioncallback & callback,
			const oid * oid_name,
			size_t oid_name_size,
			const unsigned * timeout_ms = 0);

	int get_port_physical_address(fixs_ith_snmp::mac_address_t & port_macaddr,
				const char * port_name,
				fixs_ith_switchboardinfo & switch_board,
				fixs_ith::switchboard_plane_t sb_plane,
				const unsigned * timeout_ms = 0);

    int get_port_physical_address(fixs_ith_snmp::mac_address_t & port_macaddr,
			uint16_t port_ifIndex,
			fixs_ith_switchboardinfo & switch_board,
			fixs_ith::switchboard_plane_t sb_plane,
			const unsigned * timeout_ms = 0);

	int get_port_physical_address(fixs_ith_snmp::mac_address_t & port_macaddr,
			uint16_t port_ifIndex,
			int32_t switch_board_slot,
			uint32_t switch_board_magazine,
			const char * switch_board_ip,
			const unsigned * timeout_ms);

	int get_port_name(char (& port_name)[fixs_ith_snmp::IF_NAME_MAX_LEN],
				uint16_t port_ifIndex,
				fixs_ith_switchboardinfo & switch_board,
				fixs_ith::switchboard_plane_t sb_plane,
				const unsigned * timeout_ms = 0);

	int get_port_name(char (& port_name) [fixs_ith_snmp::IF_NAME_MAX_LEN],
				uint16_t port_ifIndex,
				int32_t switch_board_slot,
				uint32_t switch_board_magazine,
				const char * switch_board_ip,
				const unsigned * timeout_ms);

	int get_bridgeport_if_index (
			uint16_t & if_index,
			uint16_t port_number,
			fixs_ith_switchboardinfo & switch_board,
			fixs_ith::switchboard_plane_t sb_plane,
			const unsigned * timeout_ms = 0);

	int get_bridgeport_if_index (
			uint16_t & if_index,
			uint16_t port_number,
			int32_t switch_board_slot,
			uint32_t switch_board_magazine,
			const char * switch_board_ip,
			const unsigned * timeout_ms = 0);

	int get_bridgeport_if_index (
			fixs_ith_snmpsessionhandler & session_handler,
			fixs_ith_snmpsessioncallback & callback,
			const oid * oid_name,
			size_t oid_name_size,
			const unsigned * timeout_ms = 0);

	// Get the speed of an interface
	int get_interface_speed (
			uint32_t & speed,
			uint16_t if_index,
			fixs_ith_switchboardinfo & switch_board,
			fixs_ith::switchboard_plane_t sb_plane,
			const unsigned * timeout_ms = 0);

	int get_interface_speed (
			uint32_t & speed,
			uint16_t if_index,
			int32_t switch_board_slot,
			uint32_t switch_board_magazine,
			const char * switch_board_ip,
			const unsigned * timeout_ms = 0);

	int get_interface_speed (
			fixs_ith_snmpsessionhandler & session_handler,
			fixs_ith_snmpsessioncallback & callback,
			const oid * oid_name,
			size_t oid_name_size,
			const unsigned * timeout_ms = 0);


	//  *BASIC_TEST* - ONLY FOR BASIC TEST TO BE REMOVED!!!
	int test_port_operations(fixs_ith_switchboardinfo & switch_board, fixs_ith::switchboard_plane_t sb_plane );
	int test_lag_operations(fixs_ith_switchboardinfo & switch_board, fixs_ith::switchboard_plane_t sb_plane );
	int test_vlan_operations(fixs_ith_switchboardinfo & switch_board, fixs_ith::switchboard_plane_t sb_plane );

	virtual int manage_ColdStart (const char * switch_board_IP);
	inline int manage_WarmStart (const char * switch_board_IP) { return manage_ColdStart(switch_board_IP); }

	template <class Type>
	int set_port_mib_object(char * switch_board_ip, oid * oid_name,size_t oid_size,Type value);

    int set_port_admin_status(fixs_ith_switchboardinfo & switch_board,
    			uint16_t port_ifIndex, fixs_ith_snmp::port_status_t port_status,
    			fixs_ith::switchboard_plane_t sb_plane);

	int set_port_admin_status (const char * switch_board_ip, uint16_t port_ifIndex,
				fixs_ith_snmp::port_status_t port_status);

	int set_port_default_vlan(fixs_ith_switchboardinfo & switch_board,
	    		uint16_t port_ifIndex, uint16_t vid,
	    		fixs_ith::switchboard_plane_t sb_plane);

	int set_port_default_vlan(const char * switch_board_ip,
		    	uint16_t port_ifIndex, uint16_t vid);

	int set_port_oam_admin_status(fixs_ith_switchboardinfo & switch_board,
			uint16_t port_ifIndex, fixs_ith_snmp::oam_status_t port_oam_status,
			fixs_ith::switchboard_plane_t sb_plane);

	int set_port_oam_admin_status (const char * switch_board_ip, uint16_t port_ifIndex,
			fixs_ith_snmp::oam_status_t port_oam_status);

	int set_eth_max_frame_size(fixs_ith_switchboardinfo & switch_board,
			uint16_t port_ifIndex, uint32_t eth_max_frame_size,
			fixs_ith::switchboard_plane_t sb_plane);

	int set_eth_max_frame_size(const char * switch_board_ip,
			uint16_t port_ifIndex, uint32_t eth_max_frame_size);

	int enable_vrrp_new_master_notification(fixs_ith_switchboardinfo& switch_board);

	int enable_vrrp_new_master_notification(const char* switch_board_ip);

	int reset_board(const fixs_ith_switchboardinfo& switch_board);
	int reset_board(const fixs_ith_switchboardinfo& switch_board, fixs_ith::board_slotposition_t slot); //TR_HY52528

	int reset_board(char* switch_board_ip, int32_t slot);
	int enable_bfd_session_state_notification(fixs_ith_switchboardinfo& switch_board);

	int enable_bfd_session_state_notification(const char* switch_board_ip);

	int disable_stp_port_status(fixs_ith_switchboardinfo& switch_board, uint16_t port_ifindex,
			fixs_ith::switchboard_plane_t sb_plane);
	int disable_stp_port_status(const char* switch_board_ip, uint16_t port_ifindex);

	int setBridgePortasEdgePort(fixs_ith_switchboardinfo& switch_board, uint16_t port_ifindex,
			fixs_ith::switchboard_plane_t sb_plane);
	int setBridgePortasEdgePort(const char* switch_board_ip, uint16_t port_ifindex);

	int get_if_mau_type (
			std::vector<bool> & if_mau_type,
			uint16_t if_index,
			fixs_ith_switchboardinfo & switch_board,
			fixs_ith::switchboard_plane_t sb_plane,
			const unsigned * timeout_ms = 0);

	int get_if_mau_default_type (
			std::vector<oid> & default_type,
			uint16_t if_index,
			fixs_ith_switchboardinfo & switch_board,
			fixs_ith::switchboard_plane_t sb_plane,
			const unsigned * timeout_ms = 0);

	int get_if_mau_auto_neg_admin_status (
			int & status,
			uint16_t if_index,
			fixs_ith_switchboardinfo & switch_board,
			fixs_ith::switchboard_plane_t sb_plane,
			const unsigned * timeout_ms = 0);

	int set_if_mau_default_type (
			fixs_ith_snmp::IfMauTypeListBitsConstants default_type,
			uint16_t if_index,
			fixs_ith_switchboardinfo & switch_board,
			fixs_ith::switchboard_plane_t sb_plane);

	int set_if_mau_auto_neg_admin_status (
			fixs_ith_snmp::if_Mau_Auto_Neg_Admin_States status,
			uint16_t if_index,
			fixs_ith_switchboardinfo & switch_board,
			fixs_ith::switchboard_plane_t sb_plane);


	//virtual int get_my_switch_board_info (std::vector<fixs_ith_switchboardinfo> & board_infos);

//	virtual int get_all_switch_board_info (std::vector<fixs_ith_switchboardinfo> & board_infos);
//	int get_switch_boards_info (std::vector<fixs_ith_switchboardinfo> & board_infos, uint32_t magazine);

	int reset_userpasswd(const fixs_ith_switchboardinfo & switch_board, const uint16_t userType);

	static int map_net_snmp_error_stat (long net_snmp_error_stat);

	int start_smx_communication(fixs_ith_switchboardinfo & switch_board);
	int getBoardState(fixs_ith_switchboardinfo& switch_board, uint8_t& shelf_manager_control_state); //TR_HY52528
	int getBoardState(
	               uint8_t& shelf_manager_control_state,
	               uint32_t board_magazine,
	               int32_t board_slot,
	               const char * switch_board_ip);      //TR_HY52528

	//int switch_board_delete (uint32_t magazine, int32_t slot_pos);
	//int switch_board_add (fixs_ith_switchboardinfo & new_switch_board);
	//int switch_board_change (fixs_ith_switchboardinfo & switch_board);
	template <class Type>
	int set_mib_object (fixs_ith_snmpsessionhandler & session_handler, oid * oid_name ,size_t oid_size,Type oidValue);
	//int get_switch_board (fixs_ith_switchboardinfo & switch_board, int slot_num);
	inline bool isCommunicationFailure(int error){
		if(error == fixs_ith_snmp::ERR_SNMP_REQUEST_NOT_SENT ||
		   error == fixs_ith_snmp::ERR_SNMP_TIMEOUT ||
		   error == fixs_ith_snmp::ERR_SNMP_PDU_CREATE ||
		   error == fixs_ith_snmp::ERR_SNMP_CALLBACK_NO_MESSAGE_RECEIVED ||
		   error == fixs_ith_snmp::ERR_SNMP_SESSION_READ ||
		   error == fixs_ith_snmp::ERR_SNMP_SESSION_OPEN ||
		   error == fixs_ith_snmp::ERR_SNMP_SELECT){
			return true;
		}
		return false;
	};

private:

	// helper method to check if an OID is defined on a Switch board
	int check_oid( const oid * oid_name,
				   size_t oid_name_size,
				   uint32_t switch_board_magazine,
				   int32_t switch_board_slot,
				   const char * switch_board_ip,
				   const unsigned * timeout_ms = 0);


	// To get the value of an OID of type 'PortList'
	template < int PORTLIST_ARRAY_SIZE >
	int get_portlist(
			uint8_t (& port_list) [PORTLIST_ARRAY_SIZE],
			const oid * oid_name,
			size_t oid_name_size,
			fixs_ith_switchboardinfo & switch_board,
			fixs_ith::switchboard_plane_t sb_plane,
			const unsigned * timeout_ms = 0);

	template < int PORTLIST_ARRAY_SIZE >
	int get_portlist(
			uint8_t (& port_list) [PORTLIST_ARRAY_SIZE],
			const oid * oid_name,
			size_t oid_name_size,
			int32_t switch_board_slot,
			uint32_t switch_board_magazine,
			const char * switch_board_ip,
			const unsigned * timeout_ms = 0);

	int get_portlist(
			fixs_ith_snmpsessionhandler & session_handler,
			fixs_ith_snmpsessioncallback & callback,
			const oid * oid_name,
			size_t oid_name_size,
			const unsigned * timeout_ms = 0);


	//int load_my_switch_boards_info_from_cs (bool force = false);
//	int load_other_switch_boards_info_from_cs (bool force = false);
	//int load_switch_boards_info_from_cs (bool force = false);
//	int load_switch_boards_info_from_agents (bool force = false);

	//int no_my_switch_board_found_check () const;
	//int no_other_switch_board_found_check () const;

	int send_get_request (fixs_ith_snmpsessionhandler & session_handler, const oid * oid_name, size_t oid_name_size, const unsigned * timeout_ms);

	int send_set_request (fixs_ith_snmpsessionhandler & session_handler, const oid * oid_name, size_t oid_name_size, const char * oid_value, const unsigned * timeout_ms);
	int send_set_request (fixs_ith_snmpsessionhandler & session_handler, const oid * oid_name, size_t oid_name_size, int oid_value, const unsigned * timeout_ms);
	int send_set_request (fixs_ith_snmpsessionhandler & session_handler, const oid * oid_name, size_t oid_name_size, u_char asn1_val_type, const u_char * value, size_t value_size, const unsigned * timeout_ms);
	template <class Type>
	int set_mib_object(char * switch_board_ip, oid * oid_name,size_t oid_size,Type value);

	int set_active_manager_trap_subscription (
				const char * switch_board_ip,
				const char * trapreceiver_ip, int request_value );

	int create_vlan(const char * switch_board_ip, uint16_t vlan_id, const char * vlan_name);

	typedef oid ipv4subnet_index_oid_t [11];
	int build_ipv4subnet_index_oid(ipv4subnet_index_oid_t & ipv4subnet_index_oid, const char * ipv4subnet);

	int create_ipv4subnet(const char * switch_board_ip, const std::string  & ipv4subnet);

	int set_qVlanStaticRowStatus(
			fixs_ith_snmp::snmptarget::addr_row_status_t row_status,
			fixs_ith_snmpsessionhandler & session_handler,
			oid * oid_name,
			size_t oid_name_size,
			const unsigned * timeout_ms = 0);

	int set_qVlanStaticName (
				const char * vlan_name,
				fixs_ith_snmpsessionhandler & session_handler,
				oid * oid_name,
				size_t oid_name_size,
				const unsigned * timeout_ms = 0
				);
	
	int set_cont_file_to_delete(
                		const char * switch_board_ip,
                		std::string file_name);

	int set_delete_cont_file (
				std::string file_name,
				fixs_ith_snmpsessionhandler & session_handler,
				const oid * oid_name,
				size_t oid_name_size,
				const unsigned * timeout_ms = 0);

	int create_cont_transfer_server_ip(
                		const char * switch_board_ip,
                		const char * server_address);

	int set_server_ip_address (
				const char * server_address,
				fixs_ith_snmpsessionhandler & session_handler,
				const oid * oid_name,
				size_t oid_name_size,
				const unsigned * timeout_ms=0);

	int create_cont_transfer_server_path(
                		const char * switch_board_ip,
                		const char * log_path);

	int set_server_path (
                		const char * log_path,
                		fixs_ith_snmpsessionhandler & session_handler,
                		const oid * oid_name,
                		size_t oid_name_size,
                		const unsigned * timeout_ms = 0);

	int set_cont_control_notifications(
				const char * switch_board_ip,
				int control_value);
								
	int set_control_notifications_container(
				int control_value,
				fixs_ith_snmpsessionhandler & session_handler,
				const oid * oid_name,
				size_t oid_name_size,
				const unsigned * timeout_ms = 0);

	int set_cont_automatic_transfer(
				const char * switch_board_ip,
				int control_value);

	int set_automatic_transfer_cont_file(
				int control_value,
				fixs_ith_snmpsessionhandler & session_handler,
				const oid * oid_name,
				size_t oid_name_size,
				const unsigned * timeout_ms = 0);

	int set_cont_file_encryption(
				const char * switch_board_ip,
				int encryption_setting);

	int set_encryption_setting_cont_file(
				int encryption_setting,
				fixs_ith_snmpsessionhandler & session_handler,
				const oid * oid_name,
				size_t oid_name_size,
				const unsigned * timeout_ms = 0);

	int create_cont_file(
                                const char * switch_board_ip,
                                int value);

        int set_container_file_creation(
                                int value,
                                fixs_ith_snmpsessionhandler & session_handler,
                                const oid * oid_name,
                                size_t oid_name_size,
                                const unsigned * timeout_ms = 0);

	int set_mib_port_default_user_priority(
				const char * switch_board_ip,
				uint16_t portIndex);

	int set_mib_port_default_user_priority(
				int priority,
				fixs_ith_snmpsessionhandler & session_handler,
				const oid * oid_name,
				size_t oid_name_size,
				const unsigned * timeout_ms = 0);
                                

	int set_vlan_ports(const char * switch_board_ip,
					uint16_t vlan_id, const std::set<uint16_t>& tagged_ports,
					const std::set<uint16_t>& untagged_ports);

	int set_vlan_ports(const char * switch_board_ip,
						uint16_t vlan_id,
						const fixs_ith_snmp::if_bitmap_t & if_bitmap_tagged,
						const fixs_ith_snmp::if_bitmap_t & if_bitmap_untagged);

        int set_vlan_ports(const char * switch_board_ip,
                                                uint16_t vlan_id,
                                                const fixs_ith_snmp::if_bitmap_t_cmx & if_bitmap_tagged,
                                                const fixs_ith_snmp::if_bitmap_t_cmx & if_bitmap_untagged);


	int remove_vlan_ports(const char * switch_board_ip,
						uint16_t vlan_id);

	int delete_vlan(const char * switch_board_ip,uint16_t vlan_id);

	int get_vlan_ports(fixs_ith_snmp::if_bitmap_t & port_list,
				uint16_t vlan_id,
				fixs_ith_snmp::vlanPort_type_t port_type,
				int32_t switch_board_slot,
				uint32_t switch_board_magazine,
				const char * switch_board_ip,
				const unsigned * timeout_ms);

	int get_vlan_untagged_ports(fixs_ith_snmp::if_bitmap_t & port_list,
				uint16_t vlan_id,
				fixs_ith_switchboardinfo & switch_board,
				fixs_ith::switchboard_plane_t sb_plane,
				const unsigned * timeout_ms);

	int get_vlan_tagged_ports(fixs_ith_snmp::if_bitmap_t & port_list,
					uint16_t vlan_id,
					fixs_ith_switchboardinfo & switch_board,
					fixs_ith::switchboard_plane_t sb_plane,
					const unsigned * timeout_ms);
	int create_lag (char * switch_board_ip, uint16_t lag_ifIndex, const char * lag_ifname);

	int add_lag_ports(const char * switch_board_ip, uint16_t lag_ifIndex, const std::set<uint16_t>& port_if_list);

	int delete_ipv4subnet(const char * switch_board_ip, const std::string & ipv4subnet);

	int set_ipv4subnet_vid(const char * switch_board_ip,
				const std::string  & ipv4subnet,
				const uint16_t vid);

	int reset_userpasswd(const char * switch_board_ip, const uint16_t userAccount);

	int open_snmp_session_handler (fixs_ith_snmpsessionhandler & session_handler, fixs_ith_snmpsessioncallback & callback, const char * switch_board_ip);
	int create_snmp_pdu (netsnmp_pdu * (& snmp_pdu), int command);
	int add_snmp_null_var (netsnmp_pdu * pdu, const oid * name, size_t name_size);

	//int get_switch_board_info_ptr_by_ip(fixs_ith_switchboardinfo * & p_switch_board_info_ptr,  bool & scb_is_in_local_magazine, const char * switch_board_ip);

	int get_switch_board_target_ipns (char (& switch_board_ipna) [16], char (& switch_board_ipnb) [16], const fixs_ith_switchboardinfo & switch_board,fixs_ith::switchboard_plane_t sb_plane);

	int get_if_mau_type (
			std::vector<bool> & if_mau_type,
			uint16_t if_index,
			int32_t switch_board_slot,
			uint32_t switch_board_magazine,
			const char * switch_board_ip,
			const unsigned * timeout_ms = 0);

	int get_if_mau_type (
			fixs_ith_snmpsessionhandler & session_handler,
			fixs_ith_snmpsessioncallback & callback,
			const oid * oid_name,
			size_t oid_name_size,
			const unsigned * timeout_ms = 0);

	int get_if_mau_default_type (
			std::vector<oid> & default_type,
			uint16_t if_index,
			int32_t switch_board_slot,
			uint32_t switch_board_magazine,
			const char * switch_board_ip,
			const unsigned * timeout_ms = 0);

	int get_if_mau_default_type (
			fixs_ith_snmpsessionhandler & session_handler,
			fixs_ith_snmpsessioncallback & callback,
			const oid * oid_name,
			size_t oid_name_size,
			const unsigned * timeout_ms = 0);

	int get_if_mau_auto_neg_admin_status (
			int & status,
			uint16_t if_index,
			int32_t switch_board_slot,
			uint32_t switch_board_magazine,
			const char * switch_board_ip,
			const unsigned * timeout_ms = 0);

	int get_if_mau_auto_neg_admin_status (
			fixs_ith_snmpsessionhandler & session_handler,
			fixs_ith_snmpsessioncallback & callback,
			const oid * oid_name,
			size_t oid_name_size,
			const unsigned * timeout_ms = 0);

	int set_if_mau_default_type (
			fixs_ith_snmp::IfMauTypeListBitsConstants default_type,
			uint16_t if_index,
			const char * switch_board_ip);

	int set_if_mau_default_type (
			oid * oid_name,
			size_t oid_name_size,
			oid * oid_value,
			size_t oid_value_size,
			const char * switch_board_ip);

	int set_if_mau_auto_neg_admin_status (
			fixs_ith_snmp::if_Mau_Auto_Neg_Admin_States status,
			uint16_t if_index,
			const char * switch_board_ip);

	int set_if_mau_auto_neg_admin_status (
			oid * oid_name,
			size_t oid_name_size,
			fixs_ith_snmp::if_Mau_Auto_Neg_Admin_States status,
			const char * switch_board_ip);






/*	inline bool isCommunicationFailure(int error){
		if(error == fixs_ith_snmp::ERR_SNMP_REQUEST_NOT_SENT ||
		   error == fixs_ith_snmp::ERR_SNMP_TIMEOUT ||
		   error == fixs_ith_snmp::ERR_SNMP_PDU_CREATE ||
		   error == fixs_ith_snmp::ERR_SNMP_CALLBACK_NO_MESSAGE_RECEIVED ||
		   error == fixs_ith_snmp::ERR_SNMP_SESSION_READ ||
		   error == fixs_ith_snmp::ERR_SNMP_SESSION_OPEN ||
		   error == fixs_ith_snmp::ERR_SNMP_SELECT){
			return true;
		}
		return false;
	};*/

	//===================//
	// Utility Functions //
	//===================//
public:
//	inline int slot_position_from_index (size_t index) const {
//		return ((index < _my_switch_board_infos.size()) ? _my_switch_board_infos[index].slot_position : -1);
//	}
//
//	inline size_t index_from_slot_position (int pos) const {
//		size_t i = 0;
//		while ((i < _my_switch_board_infos.size()) && (_my_switch_board_infos[i].slot_position != pos)) ++i;
//		return (i < _my_switch_board_infos.size()) ? i : -1;
//
//		while ((i < fixs_ith_snmp::SWITCH_BOARD_COUNT_PER_MAGAZINE) && (_my_switch_board_infos[i].slot_position != pos)) ++i;
//		return (i < fixs_ith_snmp::SWITCH_BOARD_COUNT_PER_MAGAZINE) ? i : -1;
//	}

	static int oid_to_str (char * buf, size_t size, const oid * id, size_t id_length);
	static int oid_compare (const oid * id1, size_t id1_length, const oid * id2, size_t id2_length);

    static int get_ifIndex_bitmap(const std::set<uint16_t>& ifIndexlist, fixs_ith_snmp::if_bitmap_t & ifIndex_bitmap);
    static int get_ifIndex_bitmap(const std::set<uint16_t>& ifIndexlist, fixs_ith_snmp::if_bitmap_t_cmx & ifIndex_bitmap);
	//===========//
	// Operators //
	//===========//
private:
	fixs_ith_snmpmanager & operator= (const fixs_ith_snmpmanager & rhs);


	//========//
	// Fields //
	//========//
protected:
	// Access synchronization controllers
	//ACE_Recursive_Thread_Mutex _sync_object;
    //fixs_ith_server * _ith_server;
	//std::vector<fixs_ith_switchboardinfo> _my_switch_board_infos;
	//std::vector<fixs_ith_switchboardinfo> _other_switch_board_infos;

	static const char * _snmp_v3_passphrase;
};


#endif /* ITHADM_CAA_INC_FIXS_ITH_SNMPMANAGER_H_ */
