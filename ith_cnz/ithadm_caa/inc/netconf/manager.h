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

#ifndef ITHADM_CAA_INC_FIXS_ITH_NETCONFMANAGER_H_
#define ITHADM_CAA_INC_FIXS_ITH_NETCONFMANAGER_H_

#include <vector>

#include "fixs_ith_switchboardinfo.h"
#include "netconf/netconf.h"
#include "netconf/session.h"
#include "netconf/parser/response.h"

/**
 * @class manager
 *
 * @brief Implementation
 *
 */
namespace netconf
{
	extern const char * ITH_POLICYGROUP_ID;

	class manager {

		typedef operation::OperationType netconf_oper_t;

		struct config_request {
			netconf_oper_t oper_value;
			char config_doc[64 * 1024];

			config_request(netconf_oper_t op = operation::MERGE) : oper_value(op), config_doc(){};
		};

	public:

		// Constructor
		 manager ();

		// Destructor
		virtual  ~manager ();

		// methods to handle InterfaceIPv4 Class
		int create_router(const fixs_ith_switchboardinfo & switch_board,
				const char * dn_name, bool use_merge = false);

		int create_ipv4_interface(const fixs_ith_switchboardinfo & switch_board,
				const char * dn_name,
				configdataList & interfaceData,
				bool use_merge = false);

		int create_ipv4_address(const fixs_ith_switchboardinfo & switch_board,
				const char * dn_name,
				configdataList & addressData,
				bool use_merge = false);

		int create_dst (
				const fixs_ith_switchboardinfo & switch_board,
				const char * dst_dn,
				const configdataList & dst_properties,
				bool use_merge = false);

		int delete_dst (
				const fixs_ith_switchboardinfo & switch_board,
				const char * dst_dn);

		int modify_dst (
				const fixs_ith_switchboardinfo & switch_board,
				const char * dst_dn,
				const configdataList & dst_properties,
				bool use_merge = false);

		int create_next_hop (
				const fixs_ith_switchboardinfo & switch_board,
				const char * hop_dn,
				configdataList & hop_properties,
				bool use_merge = false);

		int delete_next_hop (
				const fixs_ith_switchboardinfo & switch_board,
				const char * hop_dn,
				const char * peer_address = 0);

		int modify_next_hop (
				const fixs_ith_switchboardinfo & switch_board,
				const char * hop_dn,
				configdataList & hop_properties,
				bool use_merge = false);

		int get_next_hop (
				configdataList & next_hop_data,
				const fixs_ith_switchboardinfo & switch_board,
				const char * hop_dn);

		int create_vrrp_session (
				const fixs_ith_switchboardinfo & switch_board,
				const char * vrrp_session_dn,
				const configdataList & vrrp_session_properties,
				bool use_merge = false);

		int delete_vrrp_session (
				const fixs_ith_switchboardinfo & switch_board,
				const char * vrrp_session_dn);

		int modify_vrrp_session (
				const fixs_ith_switchboardinfo & switch_board,
				const char * vrrp_session_dn,
				const configdataList & vrrp_session_properties,
				bool use_merge = false);

		int get_vrrp_session (
				configdataList & config_data,
				const fixs_ith_switchboardinfo & switch_board,
				const char * vrrp_session_dn);

		int create_vrrp_IPv4_address (
				const fixs_ith_switchboardinfo & switch_board,
				const char * vrrp_ipv4_address_dn,
				const configdataList & vrrp_ipv4_address_properties,
				bool use_merge = false);

		int delete_vrrp_IPv4_address (
				const fixs_ith_switchboardinfo & switch_board,
				const char * vrrp_ipv4_address_dn);

		int modify_vrrp_IPv4_address (
				const fixs_ith_switchboardinfo & switch_board,
				const char * vrrp_ipv4_address_dn,
				const configdataList & vrrp_ipv4_address_properties,
				bool use_merge = false);

		int get_vrrp_IPv4_address (
				configdataList & config_data,
				const fixs_ith_switchboardinfo & switch_board,
				const char * vrrp_ipv4_address_dn);

		int get_ipv4_interface (
				configdataList & configData,
				const fixs_ith_switchboardinfo& switch_board,
				const char* objectDN);

		int get_ipv4_address (
				configdataList & data,
				const fixs_ith_switchboardinfo & switch_board,
				const char * dn_name);

		int get_dst (
				configdataList & dstData,
				const fixs_ith_switchboardinfo & switch_board,
				const char * dn_name);

		int modify_ipv4_interface(const fixs_ith_switchboardinfo & switch_board,
				const char * dn_name,
				configdataList & interfaceData,
				bool use_merge = false);

		int modify_ipv4_address(const fixs_ith_switchboardinfo & switch_board,
				const char * dn_name,
				configdataList & addressData,
				bool use_merge = false);

		int delete_router(const fixs_ith_switchboardinfo & switch_board,
				const char * routerDN);

		int delete_ipv4_interface(const fixs_ith_switchboardinfo & switch_board,
				const char * dn_name);

		int delete_ipv4_address(const fixs_ith_switchboardinfo & switch_board,
				const char * dn_name);

		int create_bfd_profile(const fixs_ith_switchboardinfo & switch_board,
				const char * dn_name,
				const configdataList & bfdProfileData,
				bool use_merge = false);

		int modify_bfd_profile(const fixs_ith_switchboardinfo & switch_board,
				const char * dn_name,
				const configdataList & bfdProfileData,
				bool use_merge = false);

		int delete_bfd_profile(const fixs_ith_switchboardinfo & switch_board,
				const char * dn_name);

		int get_bfd_sessionInfo(configdataList & configData, const fixs_ith_switchboardinfo& switch_board,
				const char* dn_name);

		int delete_acl_ipv4 (
				const fixs_ith_switchboardinfo & switch_board,
				const char * dn_name);

		int create_acl_entry (
				const fixs_ith_switchboardinfo & switch_board,
				const char * dn_name,
				const configdataList & attributes,
				const std::vector<configdataList> & conditions,
				const configdataList & action,
				bool use_merge = false);

		int modify_acl_entry (
				const fixs_ith_switchboardinfo & switch_board,
				const char * dn_name,
				const configdataList & attributes,
				const std::vector<configdataList> & conditions,
				const configdataList & action,
				bool use_merge = false);

		int delete_acl_entry (
				const fixs_ith_switchboardinfo & switch_board,
				const char * dn_name);

		// Enable ACL handling on a switch board (create the POLICY GROUP)
		int enable_acl(const fixs_ith_switchboardinfo & switch_board);

		// Get ACL handling status on a switch board. If successful ( return value == 0) returns in the output parameter the status of ACL handling (true --> ENABLED;  false -->DISABLED)
		int get_acl_status(const fixs_ith_switchboardinfo & switch_board, bool & acl_status);

		int attach_acl_to_interface(
				const fixs_ith_switchboardinfo & switch_board,
				const char* routerName,
				const uint16_t vlanId,
				const std::string& aclName);

		int detach_acl_to_interface(
				const fixs_ith_switchboardinfo & switch_board,
				const char* routerName,
				const uint16_t vlanId );

		int test_l3_operation();

	protected:
		int create_ipv4_peer (
				const fixs_ith_switchboardinfo & switch_board,
				const char * peer_dn,
				const configdataList & peer_properties,
				bool use_merge = false);

		int delete_ipv4_peer (
				const fixs_ith_switchboardinfo & switch_board,
				const char * peer_dn);

		int modify_ipv4_peer (
				const fixs_ith_switchboardinfo & switch_board,
				const char * peer_dn,
				const configdataList & dst_properties,
				bool use_merge = false);

	private:
		manager (const manager & rhs);

// **************************** //
// *** private methods
// **************************** //

		int get_virtual_router_id(const fixs_ith_switchboardinfo & switch_board, const char* routerName, std::string& routerId);

		int set_acl_on_interface(const fixs_ith_switchboardinfo & switch_board, const char* routerId, const uint16_t vlanId, const std::string& aclName);

		int reset_acl_on_interface(const fixs_ith_switchboardinfo & switch_board, const char* routerId, const uint16_t vlanId);

		int get_ipv4_peer(configdataList & ipv4_peer_data, const fixs_ith_switchboardinfo & switch_board, const char * dn_name);

		int build_peer_ipv4_dns_by_hop_dn (
				char * peer_dn,
				size_t size,
				char * peer_smx_dn,
				size_t size_smx,
				const char * hop_dn,
				const char * peer_ipv4_id);

		int handle_ipv4_peer_by_next_hop (
				char * peer_dn,
				size_t size,
				char * peer_smx_dn,
				size_t size_smx,
				const fixs_ith_switchboardinfo & switch_board,
				const char * hop_dn,
				const char * peer_ipv4_id,
				operation::OperationType op,
				command::CommandType cmd,
				configdataList * properties,
				bool use_merge = false);

		int create_ipv4_peer_by_next_hop (
				char * peer_dn,
				size_t size,
				const fixs_ith_switchboardinfo & switch_board,
				const char * hop_dn,
				configdataList & hop_properties);

		int delete_ipv4_peer_by_next_hop (
				const fixs_ith_switchboardinfo & switch_board,
				const char * hop_dn,
				const char * peer_address);

		int modify_ipv4_peer_by_next_hop (
				char * peer_dn,
				size_t size,
				const fixs_ith_switchboardinfo & switch_board,
				const char * hop_dn,
				configdataList & hop_properties);

		int handle_config_element (
				const fixs_ith_switchboardinfo & switch_board,
				const char * dn_name,
				const char * element_name,
				const char * element_instance_name,
				operation::OperationType op,
				command::CommandType cmd,
				const configdataList * properties,
				Session &session);

		int handle_config_element_acl (
				const fixs_ith_switchboardinfo & switch_board,
				const char * dn_name,
				const char * element_name,
				const char * element_instance_name,
				operation::OperationType op,
				command::CommandType cmd,
				const configdataList * properties,
				const std::vector<configdataList> * conditions,
				const configdataList * action,
				Session &session);

		int apply_router_config_request(const fixs_ith_switchboardinfo & switch_board,
				const char * routerDN, operation::OperationType oper);

		int apply_ipv4_interface_config_request(const fixs_ith_switchboardinfo & switch_board,
				const char * interfaceDN, configdataList & configData, operation::OperationType oper);

		int apply_ipv4_address_config_request(const fixs_ith_switchboardinfo & switch_board,
				const char * addressDN, configdataList & configData, operation::OperationType oper);

		int prepare_router_edit_config_request(config_request * request, const char * routerDN);

		int prepare_interfaceIPv4_edit_config_request(config_request * request, const char * interfaceDN, configdataList & configData);

		int prepare_addressIPv4_edit_config_request(config_request * request, const char * ipv4AddressDN, configdataList & configData);

		int send_edit_config_request(const fixs_ith_switchboardinfo & switch_board,
				const std::string & xml_data, Session & session);

		int send_request(const char * switch_board_ip,
				const std::string & xml_data, command::CommandType command, Session &);

		int build_element_path_from_dn (char * interface_path, const char * interfaceDN);

		int build_element_path_from_dn_acl (char * element_path, size_t size, const char * element_dn);

		std::string get_vlan_path(const std::string & encaps_vid);

		void build_bfdProfileRef_from_dn( std::string & bfdProfileRef_str, const char * element_dn);

		int get_parsered_answer(Session &, parser::Response &);

		int errorcode_from_errortag_msg(Session &);

		//int test_l3_operation();
		int test_route_static_operations(fixs_ith_switchboardinfo &  switch_board);
		int test_routerAndInterface(fixs_ith_switchboardinfo & switch_board);

// **************************** //
// *** private attributes
// **************************** //
	private:

	};
}

#endif /* ITHADM_CAA_INC_FIXS_ITH_NETCONFMANAGER_H_ */
