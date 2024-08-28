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

#include <string.h>
#include <stdio.h>

#include "fixs_ith_logger.h"
#include "common/utility.h"
#include "fixs_ith_programmacros.h"
#include "fixs_ith_sbdatamanager.h"
#include "fixs_ith_workingset.h"
#include "acs_nclib_constants.h"
#include "netconf/netconf.h"
#include "netconf/session.h"
#include "netconf/parser/response.h"
#include "netconf/rdn_path.h"
#include "netconf/edit_config_dom_builder.h"
#include "netconf/get_dom_builder.h"

#include "netconf/manager.h"

namespace netconf {

	const char * ITH_POLICYGROUP_ID = "ITH";

	const char * CREATE_ITH_POLICYGROUP_CONFIG_DOC_TEMPLATE =
			" <ManagedElement xmlns=\"urn:com:ericsson:ecim:comtop\"> "
			"   <managedElementId>1</managedElementId> "
			"   <Ers xmlns=\"urn:com:ericsson:ecim:ERS\"> "
            "     <ersId>1</ersId> "
		    "     <HwProfile> "
            "       <hwProfileId>1</hwProfileId> "
            "       <PolicyGroup> "
            "         <policyGroupId>%s</policyGroupId> "
            "         <filter-stage>INGRESS</filter-stage> "
            "         <priority>1</priority> "
            "         <soft-max-entries>1024</soft-max-entries> "
            "            <condition>IP_PROTOCOL</condition> "
            "            <condition>CLASS_ID_L3</condition> "
            "            <condition>ETH_VLAN_ID</condition> "
            "            <condition>ETH_MAC48_ADDR_DST</condition> "
            "            <condition>IPV4_ADDR_SRC</condition> "
            "            <condition>IPV4_ADDR_DST</condition> "
            "            <condition>L4_PORT_DST</condition> "
            "            <condition>L4_PORT_SRC</condition> "
            "            <condition>CLASS_ID_L2</condition> "
            "            <condition>ETH_TYPE</condition> "
            "            <action>POLICY</action> "
            "            <action>NO_OPERATION</action> "
            "            <action>DROP</action> "
            "            <action>COUNTER</action> "
            "       </PolicyGroup>  "
            "     </HwProfile> "
		    "   </Ers> "
            " </ManagedElement>";

	const char * GET_ITH_POLICYGROUP_CONFIG_DOC_TEMPLATE =
				" <ManagedElement xmlns=\"urn:com:ericsson:ecim:comtop\"> "
				"   <managedElementId>1</managedElementId> "
				"   <Ers xmlns=\"urn:com:ericsson:ecim:ERS\"> "
	            "     <ersId>1</ersId> "
			    "     <HwProfile> "
	            "       <hwProfileId>1</hwProfileId> "
	            "       <PolicyGroup> "
	            "         <policyGroupId>%s</policyGroupId> "
	            "       </PolicyGroup>  "
	            "     </HwProfile> "
			    "   </Ers> "
	            " </ManagedElement>";

	const char * GET_VIRTUAL_ROUTER_CONFIG_DOC_TEMPLATE =
			" <ManagedElement xmlns=\"urn:com:ericsson:ecim:comtop\"> "
			"    <managedElementId>1</managedElementId> "
			"	 <Ers xmlns=\"urn:com:ericsson:ecim:ERS\"> "
			"	     <ersId>1</ersId>"
			"		 <ErsTransport>"
			"	    	<ersTransportId>1</ersTransportId>"
			"			<Ip>"
			"           	<ipId>1</ipId>"
			"				<VirtualRouter>"
			"					<name>%s</name>"
			"				</VirtualRouter>"
			"			</Ip>"
			"		</ErsTransport>"
			"	</Ers>"
			" </ManagedElement>";

	const char* SET_IPINTERFACE_ACL_CONFIG_DOC_TEMPLATE =
				" <ManagedElement xmlns=\"urn:com:ericsson:ecim:comtop\"> "
				"    <managedElementId>1</managedElementId> "
				"	 <Ers xmlns=\"urn:com:ericsson:ecim:ERS\"> "
				"	     <ersId>1</ersId> "
				"		 <ErsTransport> "
				"	    	<ersTransportId>1</ersTransportId> "
				"			<Ip> "
				"           	<ipId>1</ipId> "
				"				<VirtualRouter> "
				"					<virtualRouterId>%s</virtualRouterId> "
				"					<IpInterface>"
                "                       <ipInterfaceId>vlan1.%d</ipInterfaceId> "
            	"                       <pbf-ingress>%s</pbf-ingress> "
			    "					</IpInterface> "
				"				</VirtualRouter>"
				"			</Ip>"
				"		</ErsTransport>"
				"	</Ers>"
				" </ManagedElement>";

	const char* RESET_IPINTERFACE_ACL_CONFIG_DOC_TEMPLATE =
					" <ManagedElement xmlns=\"urn:com:ericsson:ecim:comtop\"> "
					"    <managedElementId>1</managedElementId> "
					"	 <Ers xmlns=\"urn:com:ericsson:ecim:ERS\"> "
					"	     <ersId>1</ersId> "
					"		 <ErsTransport> "
					"	    	<ersTransportId>1</ersTransportId> "
					"			<Ip> "
					"           	<ipId>1</ipId> "
					"				<VirtualRouter> "
					"					<virtualRouterId>%s</virtualRouterId> "
					"					<IpInterface>"
	                "                       <ipInterfaceId>vlan1.%d</ipInterfaceId> "
	            	"                       <pbf-ingress %s></pbf-ingress> "
				    "					</IpInterface> "
					"				</VirtualRouter>"
					"			</Ip>"
					"		</ErsTransport>"
					"	</Ers>"
					" </ManagedElement>";

	// Constructor
	manager::manager () {
		Session::initialize();
		parser::Response::initialize();
	}

	manager::~manager () {
		Session::shutdown();
		parser::Response::finalize();
	}


	int manager::create_router(const fixs_ith_switchboardinfo & switch_board,
			const char * routerDN, bool use_merge)
	{

		FIXS_ITH_LOG(LOG_LEVEL_INFO, "'Create router <%s>' request received ... ", routerDN);

		return apply_router_config_request(switch_board, routerDN,
												(use_merge)?operation::MERGE:operation::CREATE);

	}

	int manager::create_ipv4_interface(const fixs_ith_switchboardinfo & switch_board,
			const char * interfaceDN,
			configdataList & configData, bool use_merge)
	{

		if(!configData.size()) {
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Configuration data not found: cannot create IPV4 Interface <%s>", interfaceDN);
			return ERR_NO_CONFIGURATION_DATA;
		}

		FIXS_ITH_LOG(LOG_LEVEL_INFO, "Creating IPV4 Interface <%s> ", interfaceDN);

		return apply_ipv4_interface_config_request(switch_board, interfaceDN, configData,
				(use_merge)?operation::MERGE:operation::CREATE);

	}



	int manager::get_ipv4_interface (
			configdataList & configData,
			const fixs_ith_switchboardinfo& switch_board,
			const char* objectDN)
	{
		char sb_ipn_addresses [FIXS_ITH_CONFIG_IPN_COUNT][16] = {};

		if(fixs_ith_sbdatamanager::get_switch_board_target_ipns(sb_ipn_addresses[0], sb_ipn_addresses[1], switch_board, fixs_ith::TRANSPORT_PLANE))
			return ERR_IP_ADDRESS_NOT_FOUND;

		if(!configData.size()) {
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Configuration data not found: cannot get info of IPV4 Interface <%s> on SMX having IPNA == %s and IPNB == %s ", objectDN, sb_ipn_addresses[0], sb_ipn_addresses[1]);
			return ERR_NO_CONFIGURATION_DATA;
		}

		char interface_path[XML_PATH_MAX_SIZE] = {0};

		if(build_element_path_from_dn(interface_path, objectDN)) return ERR_BUILD_ELEMENT_PATH;

		FIXS_ITH_LOG(LOG_LEVEL_INFO, "Get IPV4 Interface <%s>: on SMX having IPNA == %s and IPNB == %s ", interface_path, sb_ipn_addresses[0], sb_ipn_addresses[1]);

		char buffer [64 * 1024] = {0};
		netconf::get_dom_builder builder;

		const int buildResult = builder.build_dom(buffer, sizeof(buffer), interface_path);

		if( (0 > buildResult) || (sizeof(buffer) <= static_cast<size_t>(buildResult) ) )
		{
			FIXS_ITH_LOG(LOG_LEVEL_INFO, "Building of xml get request for IPV4 Interface <%s> failed, build_dom returns <%d>", interface_path, buildResult);
			return ERR_BUILD_CONFIG_DOM;
		}

		int sendResult = ERR_SEND_REQUEST_FAILED;;
		::srandom(static_cast<unsigned>(::time(0)));
		int ipn_index = ::random() & 0x01;
		Session session;
		parser::Response response;


		for (int i = 0; ((sendResult == ERR_SEND_REQUEST_FAILED || sendResult == ERR_SSH_USERAUTH_FAILED) && i < FIXS_ITH_CONFIG_IPN_COUNT); ++i, ipn_index = (ipn_index + 1) & 0x01) {
			sendResult = send_request(sb_ipn_addresses[ipn_index],buffer, command::CMD_GET, session);
			if (sendResult == ERR_SSH_USERAUTH_FAILED) {
				FIXS_ITH_LOG(LOG_LEVEL_INFO,
						"RESETTING USER PASSWORD on switch board slot == %d, switch board IPN = '%s'", switch_board.slot_position, sb_ipn_addresses[ipn_index]);

				fixs_ith::workingSet_t::instance()->get_snmpmanager().reset_userpasswd(switch_board, authority_levels::ADVANCED);
			}
		}

		if( (ERR_NO_ERROR == sendResult) && (ERR_NO_ERROR == ( sendResult = get_parsered_answer(session, response)) ) )
		{
			// search interface key to get
			for( configdataList::const_iterator it = configData.begin() ; it != configData.end(); ++it)
			{
				if( strcmp(it->first.c_str(), interfaceIpv4::attribute::KEY) == 0U )
				{
					configdataElement interfaceIpv4MOKey = *it;
					response.getAllAttributeOf(interfaceIpv4::CLASS_NAME, interfaceIpv4MOKey, configData);
					break;
				}
			}
		}

		return sendResult;
	}

	int manager::get_ipv4_address (
				configdataList & config_data,
				const fixs_ith_switchboardinfo & switch_board,
				const char * ipv4_address_dn) {
			const char * const element_name = addressIpv4::CLASS_NAME;
			operation::OperationType op = operation::NO_OPERATION;
			command::CommandType cmd = command::CMD_GET;

			Session session;
			parser::Response response;

			int call_result = handle_config_element(switch_board, ipv4_address_dn, element_name,
					0, op, cmd, 0, session);

			if (call_result) {
				// ERROR: On getting the IPv4Address config element.
				FIXS_ITH_LOG(LOG_LEVEL_ERROR,
					"Call to 'handle_config_element()' failed: Error on getting the IPv4Address "
					"config element info: ipv4_address_dn == <%s>: element_name == <%s>: op == <%d>: "
					"cmd == <%d>: call_result == <%d>",
					ipv4_address_dn, element_name, op, cmd, call_result);

				return call_result;
			}

			if ((call_result = get_parsered_answer(session, response))) {
				FIXS_ITH_LOG(LOG_LEVEL_ERROR,
					"Call to 'get_parsered_answer()' failed: "
					"ipv4_address_dn == <%s>: element_name == <%s>: op == <%d>: cmd == <%d>: "
					"call_result == <%d>",
					ipv4_address_dn, element_name, op, cmd, call_result);

				return call_result;
			}

			configdataList::const_iterator it = config_data.begin();
			while ((it != config_data.end()) && (it->first != addressIpv4::attribute::KEY))
				++it;

			return (
				(it != config_data.end() && response.getAllAttributeOf(element_name, *it, config_data))
					? ERR_NO_ERROR
					: ERR_DATA_MISSING
			);
		}


	int manager::create_ipv4_address(const fixs_ith_switchboardinfo & switch_board,
					const char * addressDN,
					configdataList & configData, bool use_merge)
	{

		if(!configData.size()) {
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Configuration data not found: cannot create IPV4 address <%s> ", addressDN);
			return ERR_NO_CONFIGURATION_DATA;
		}

		FIXS_ITH_LOG(LOG_LEVEL_INFO, "'Create IPV4 Address <%s>' request received ... ", addressDN);

		return apply_ipv4_address_config_request(switch_board, addressDN, configData,
																(use_merge)?operation::MERGE:operation::CREATE);
	}

	int manager::create_dst (
			const fixs_ith_switchboardinfo & switch_board,
			const char * dst_dn,
			const configdataList & dst_properties,
			bool use_merge) {
		const char * const element_name = dst::CLASS_NAME;
		const char * const element_instance_name = dst::attribute::KEY;
		operation::OperationType op = (use_merge ? operation::MERGE : operation::CREATE);
		command::CommandType cmd = command::CMD_EDITCONFIG;

		Session session;
		parser::Response response;

		int call_result = handle_config_element(switch_board, dst_dn, element_name,
				element_instance_name, op, cmd, &dst_properties, session);

		if (call_result) {
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"Call to 'handle_config_element()' failed: Error on creating the DST config element: "
				"dst_dn == <%s>: element_name == <%s>: op == <%d>: cmd == <%d>, call_result == <%d>",
				dst_dn, element_name, op, cmd, call_result);

			return call_result;
		}

		if ((call_result = get_parsered_answer(session, response))) {
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"Call to 'get_parsered_answer()' failed: "
				"dst_dn == <%s>: element_name == <%s>: op == <%d>: cmd == <%d>, call_result == <%d>",
				dst_dn, element_name, op, cmd, call_result);

			return call_result;
		}

		return ERR_NO_ERROR;
	}

	int manager::delete_dst (
			const fixs_ith_switchboardinfo & switch_board,
			const char * dst_dn) {
		const char * const element_name = dst::CLASS_NAME;
		operation::OperationType op = operation::DELETE;
		command::CommandType cmd = command::CMD_EDITCONFIG;

		Session session;
		parser::Response response;

		int call_result = handle_config_element(switch_board, dst_dn, element_name, 0, op, cmd, 0, session);

		if (call_result) {
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"Call to 'handle_config_element()' failed: Error on deleting the DST config element: "
				"dst_dn == <%s>: element_name == <%s>: op == <%d>: cmd == <%d>, call_result == <%d>",
				dst_dn, element_name, op, cmd, call_result);

			return call_result;
		}

		if ((call_result = get_parsered_answer(session, response))) {
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"Call to 'get_parsered_answer()' failed: "
				"dst_dn == <%s>: element_name == <%s>: op == <%d>: cmd == <%d>, call_result == <%d>",
				dst_dn, element_name, op, cmd, call_result);

			return call_result;
		}

		return ERR_NO_ERROR;
	}

	int manager::modify_dst (
			const fixs_ith_switchboardinfo & switch_board,
			const char * dst_dn,
			const configdataList & dst_properties,
			bool use_merge) {
		const char * const element_name = dst::CLASS_NAME;
		const char * const element_instance_name = dst::attribute::KEY;
		operation::OperationType op = (use_merge ? operation::MERGE : operation::REPLACE);
		command::CommandType cmd = command::CMD_EDITCONFIG;

		Session session;
		parser::Response response;

		int call_result = handle_config_element(switch_board, dst_dn, element_name,
				element_instance_name, op, cmd, &dst_properties, session);

		if (call_result) {
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"Call to 'handle_config_element()' failed: Error on modifying the DST config element: "
				"dst_dn == <%s>: element_name == <%s>: op == <%d>: cmd == <%d>, call_result == <%d>",
				dst_dn, element_name, op, cmd, call_result);

			return call_result;
		}

		if ((call_result = get_parsered_answer(session, response))) {
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"Call to 'get_parsered_answer()' failed: "
				"dst_dn == <%s>: element_name == <%s>: op == <%d>: cmd == <%d>, call_result == <%d>",
				dst_dn, element_name, op, cmd, call_result);

			return call_result;
		}

		return ERR_NO_ERROR;
	}

int  manager::get_dst (
		configdataList & dstData,
		const fixs_ith_switchboardinfo & switch_board,
		const char * dn_name)
{
	const char * const element_name = dst::CLASS_NAME;
	//const char * const element_instance_name = dst::attribute::KEY;
	operation::OperationType op = operation::NO_OPERATION;
	command::CommandType cmd = command::CMD_GET;

	Session session;
	parser::Response response;

	int call_result = handle_config_element(switch_board, dn_name, element_name,
			0, op, cmd, 0, session);

	if (call_result) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
			"Call to 'handle_config_element()' failed: Error on getting the DST config element: "
			"dn_name == <%s>: element_name == <%s>: op == <%d>: cmd == <%d>, call_result == <%d>",
			dn_name, element_name, op, cmd, call_result);

		return call_result;
	}

	if( ERR_NO_ERROR == ( call_result = get_parsered_answer(session, response)) )
	{
		// search dst key to get
		for( configdataList::const_iterator it = dstData.begin() ; it != dstData.end(); ++it)
		{
			if( strcmp(it->first.c_str(), dst::attribute::KEY) == 0U )
			{
				configdataElement dstMOKey = *it;
				response.getAllAttributeOf(dst::CLASS_NAME, dstMOKey, dstData);
				break;
			}
		}
	}

	return call_result;
}

int manager::get_virtual_router_id(const fixs_ith_switchboardinfo & switch_board, const char* routerName, std::string& routerId)
{
	int result = ERR_SEND_REQUEST_FAILED;

	char sb_ipn_addresses [FIXS_ITH_CONFIG_IPN_COUNT][16] = {};

	if(fixs_ith_sbdatamanager::get_switch_board_target_ipns(sb_ipn_addresses[0], sb_ipn_addresses[1], switch_board, fixs_ith::TRANSPORT_PLANE))
		return ERR_IP_ADDRESS_NOT_FOUND;

	FIXS_ITH_LOG(LOG_LEVEL_INFO, "Get Virtual Router ID of <%s> on switch board having IPNA:<%s> and IPNB:<%s> ", routerName, sb_ipn_addresses[0], sb_ipn_addresses[1]);

	char xmlRequest[64 * 1024] = {0};
	::snprintf(xmlRequest, FIXS_ITH_ARRAY_SIZE(xmlRequest), GET_VIRTUAL_ROUTER_CONFIG_DOC_TEMPLATE, routerName);

	::srandom(static_cast<unsigned>(::time(0)));
	int ipn_index = ::random() & 0x01;
	Session session;
	parser::Response response;


	for (int i = 0; ((result == ERR_SEND_REQUEST_FAILED || result == ERR_SSH_USERAUTH_FAILED) && i < FIXS_ITH_CONFIG_IPN_COUNT); ++i, ipn_index = (ipn_index + 1) & 0x01) {
		result = send_request(sb_ipn_addresses[ipn_index],xmlRequest, command::CMD_GET, session);
		if (result == ERR_SSH_USERAUTH_FAILED) {
			FIXS_ITH_LOG(LOG_LEVEL_INFO,
					"RESETTING USER PASSWORD on switch board slot == %d, switch board IPN = '%s'", switch_board.slot_position, sb_ipn_addresses[ipn_index]);

			fixs_ith::workingSet_t::instance()->get_snmpmanager().reset_userpasswd(switch_board, authority_levels::ADVANCED);
		}
	}

	if(ERR_NO_ERROR != result)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Get Virtual Router ID:<%s> request FAILED. ERROR:<%d>", routerName, result);
		return result;
	}

	result = get_parsered_answer(session, response);

	if(ERR_NO_ERROR != result )
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Error on answer parsing of Get Virtual Router ID:<%s>. ERROR:<%d>", routerName, result);
		return result;
	}

	configdataElement nameAttibute(virtualRouter::attribute::NAME, routerName);

	if( !response.getAttributeOf(virtualRouter::CLASS_NAME, nameAttibute, virtualRouter::attribute::KEY, routerId) )
	{
		FIXS_ITH_LOG(LOG_LEVEL_INFO, "Failed to get Virtual Router Id for <%s>", routerName);
		return ERR_NO_CONFIGURATION_DATA;
	}

	FIXS_ITH_LOG(LOG_LEVEL_INFO, "Router name:<%s> has a Virtual Router ID:<%s>", routerName, routerId.c_str());

	return result;

}

int manager::set_acl_on_interface(const fixs_ith_switchboardinfo& switch_board, const char* routerId, const uint16_t vlanId, const std::string& aclName)
{
	int result = ERR_SEND_REQUEST_FAILED;

	char aclIngressSmxDn[1024] = {0};
	snprintf(aclIngressSmxDn, FIXS_ITH_ARRAY_SIZE(aclIngressSmxDn), "%s,Policy=%s", netconf::switch_mom_dn::POLICIES_BASE_DN, aclName.c_str());


	char xmlRequest[64 * 1024] = {0};
	::snprintf(xmlRequest, FIXS_ITH_ARRAY_SIZE(xmlRequest), SET_IPINTERFACE_ACL_CONFIG_DOC_TEMPLATE,
			routerId, vlanId, aclIngressSmxDn);

	config_request request(operation::MERGE);
	::memset(request.config_doc, 0, FIXS_ITH_ARRAY_SIZE(request.config_doc));
	::strncpy(request.config_doc, xmlRequest, FIXS_ITH_ARRAY_SIZE(request.config_doc) - 1);

	FIXS_ITH_LOG(LOG_LEVEL_INFO, "Set ACL:<%s> on IpInterface:<vlan1.%d> under Virtual Router ID of <%s>", aclName.c_str(), vlanId, routerId);

	Session session;
	parser::Response response;

	result = send_edit_config_request(switch_board, request.config_doc, session);
	if(ERR_NO_ERROR != result)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"Failed to set ACL<%s> on IpInterface:<vlan1.%d>, ERROR:<%d> ", aclName.c_str(), vlanId, result);
		return result;
	}

	// Check if the configuration request has been successful applied on the target switch board
	result = get_parsered_answer(session, response);

	if(ERR_NO_ERROR != result)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,"FAILED to get ACL Set answer, ERROR:<%d>", result);
	}

	return result;
}

int manager::reset_acl_on_interface(const fixs_ith_switchboardinfo & switch_board, const char* routerId, const uint16_t vlanId)
{
	int result = ERR_SEND_REQUEST_FAILED;

	char xmlRequest[64 * 1024] = {0};
	::snprintf(xmlRequest, FIXS_ITH_ARRAY_SIZE(xmlRequest), RESET_IPINTERFACE_ACL_CONFIG_DOC_TEMPLATE,
			routerId, vlanId, operation::OPERATION_XML_ATTR_DELETE);

	config_request request(operation::MERGE);
	::memset(request.config_doc, 0, FIXS_ITH_ARRAY_SIZE(request.config_doc));
	::strncpy(request.config_doc, xmlRequest, FIXS_ITH_ARRAY_SIZE(request.config_doc) - 1);

	FIXS_ITH_LOG(LOG_LEVEL_INFO, "RESET ACL on IpInterface:<vlan1.%d> under Virtual Router ID of <%s>", vlanId, routerId);

	Session session;
	parser::Response response;

	result = send_edit_config_request(switch_board, request.config_doc, session);
	if(ERR_NO_ERROR != result)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"Failed to RESET ACL on IpInterface:<vlan1.%d>, ERROR:<%d> ", vlanId, result);
		return result;
	}

	// Check if the configuration request has been successful applied on the target switch board
	result = get_parsered_answer(session, response);

	if(ERR_NO_ERROR != result)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "FAILED to get ACL RESET answer, ERROR:<%d>", result);
	}

	return result;
}

int  manager::get_ipv4_peer(configdataList & ipv4_peer_data, const fixs_ith_switchboardinfo & switch_board, const char * dn_name)
{
	const char * const element_name = peerIpv4::CLASS_NAME;

	operation::OperationType op = operation::NO_OPERATION;
	command::CommandType cmd = command::CMD_GET;

	Session session;
	parser::Response response;

	int call_result = handle_config_element(switch_board, dn_name, element_name,
			0, op, cmd, 0, session);

	if (call_result) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
			"Call to 'handle_config_element()' failed: Error on getting the PEER IPv4 config element: "
			"peer_dn == <%s>: element_name == <%s>: op == <%d>: cmd == <%d>, call_result == <%d>",
			dn_name, element_name, op, cmd, call_result);

		return call_result;
	}

	if( ERR_NO_ERROR == ( call_result = get_parsered_answer(session, response)) )
	{
		// search  key to get
		for( configdataList::const_iterator it = ipv4_peer_data.begin() ; it != ipv4_peer_data.end(); ++it)
		{
			if( strcmp(it->first.c_str(), dst::attribute::KEY) == 0U )
			{
				configdataElement ipv4_peer_MOKey = *it;
				response.getAllAttributeOf(dst::CLASS_NAME, ipv4_peer_MOKey, ipv4_peer_data);
				break;
			}
		}
	}

	return call_result;
}


	int manager::create_next_hop (
			const fixs_ith_switchboardinfo & switch_board,
			const char * hop_dn,
			configdataList & hop_properties,
			bool use_merge) {
		const char * const element_name = nextHop::CLASS_NAME;
		const char * const element_instance_name = nextHop::attribute::KEY;
		operation::OperationType op = (use_merge ? operation::MERGE : operation::CREATE);
		command::CommandType cmd = command::CMD_EDITCONFIG;

		Session session;
		parser::Response response;

		int call_result = 0;
		char peer_ipv4_dn [2 * 1024] = {0};

		int create_result = create_ipv4_peer_by_next_hop(peer_ipv4_dn, sizeof(peer_ipv4_dn),
				switch_board, hop_dn, hop_properties);

		if (create_result && (create_result != ERR_OBJ_ALREADY_EXISTS)) {
			// ERROR: On creating the linked IPv4 peer configuration element
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"Call to 'create_ipv4_peer_by_next_hop()' failed: Error on creating "
				"the linked peer IPv4 for this hop: "
				"hop_dn == <%s>: element_name == <%s>: op == <%d>: cmd == <%d>: create_result == <%d>",
				hop_dn, element_name, op, cmd, create_result);

			return create_result;
		}

		char * peer_ipv4_dn_ptr = (peer_ipv4_dn[0] ? peer_ipv4_dn : 0);

		// Now it is possibile to create the Next Hop configuration element
		call_result = handle_config_element(switch_board, hop_dn, element_name,
				element_instance_name, op, cmd, &hop_properties, session);

		if (call_result) {
			// ERROR: On creating the Next Hop config element.
			// It needs eventually to delete the IPv4 peer element if it was previously created.
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"Call to 'handle_config_element()' failed: Error on creating the NextHop config element: "
				"hop_dn == <%s>: element_name == <%s>: op == <%d>: cmd == <%d>: "
				"peer_ipv4_dn == <%s>: call_result == <%d>",
				hop_dn, element_name, op, cmd, peer_ipv4_dn, call_result);

			if(create_result != ERR_OBJ_ALREADY_EXISTS)
				peer_ipv4_dn_ptr && delete_ipv4_peer(switch_board, peer_ipv4_dn);

			return call_result;
		}

		if ((call_result = get_parsered_answer(session, response))) {
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"Call to 'get_parsered_answer()' failed: "
				"hop_dn == <%s>: element_name == <%s>: op == <%d>: cmd == <%d>: "
				"peer_ipv4_dn == <%s>: call_result == <%d>",
				hop_dn, element_name, op, cmd, peer_ipv4_dn, call_result);

			peer_ipv4_dn_ptr && delete_ipv4_peer(switch_board, peer_ipv4_dn);

			return call_result;
		}

		return ERR_NO_ERROR;
	}

	int manager::delete_next_hop (
			const fixs_ith_switchboardinfo & switch_board,
			const char * hop_dn,
			const char * peer_address) {
		const char * const element_name = nextHop::CLASS_NAME;
		operation::OperationType op = operation::DELETE;
		command::CommandType cmd = command::CMD_EDITCONFIG;

		Session session;
		parser::Response response;

		int call_result = handle_config_element(switch_board, hop_dn, element_name, 0, op, cmd, 0, session);

		if (call_result) {
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"Call to 'handle_config_element()' failed: Error on deleting the NextHop config element: "
				"hop_dn = <%s>: element_name == <%s>: op == <%d>: cmd == <%d>: call_result == <%d>",
				hop_dn, element_name, op, cmd, call_result);

			return call_result;
		}

		if ((call_result = get_parsered_answer(session, response))) {
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"Call to 'get_parsered_answer()' failed: "
				"hop_dn = <%s>: element_name == <%s>: op == <%d>: cmd == <%d>: call_result == <%d>",
				hop_dn, element_name, op, cmd, call_result);

			return call_result;
		}

		// Here also the possible IPv4 linked peer will be deleted
		if(peer_address)
			delete_ipv4_peer_by_next_hop(switch_board, hop_dn, peer_address);

		return ERR_NO_ERROR;
	}

	int manager::modify_next_hop (
			const fixs_ith_switchboardinfo & switch_board,
			const char * hop_dn,
			configdataList & hop_properties,
			bool use_merge) {
		const char * const element_name = nextHop::CLASS_NAME;
		const char * const element_instance_name = nextHop::attribute::KEY;
		operation::OperationType op = (use_merge ? operation::MERGE : operation::REPLACE);
		command::CommandType cmd = command::CMD_EDITCONFIG;

		Session session;
		parser::Response response;

		int call_result = 0;
		char peer_ipv4_dn [2 * 1024] = {0};

		call_result = modify_ipv4_peer_by_next_hop(peer_ipv4_dn, sizeof(peer_ipv4_dn),
				switch_board, hop_dn, hop_properties);

		if (call_result) {
			// ERROR: On modofying the linked IPv4 peer configuration element
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"Call to 'modify_ipv4_peer_by_next_hop()' failed: Error on modifying "
				"the linked peer IPv4 for this hop: "
				"hop_dn == <%s>: element_name == <%s>: op == <%d>: cmd == <%d>: call_result == <%d>",
				hop_dn, element_name, op, cmd, call_result);

			return call_result;
		}

		// Now it is possibile to modify the Next Hop configuration element
		call_result = handle_config_element(switch_board, hop_dn, element_name,
				element_instance_name, op, cmd, &hop_properties, session);

		if (call_result) {
			// ERROR: On modifying the Next Hop config element.
			// The linked peer IPv4 can be left unchanged.
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"Call to 'handle_config_element()' failed: Error on modifying the NextHop config element: "
				"hop_dn == <%s>: element_name == <%s>: op == <%d>: cmd == <%d>: "
				"peer_ipv4_dn == <%s>: call_result == <%d>",
				hop_dn, element_name, op, cmd, peer_ipv4_dn, call_result);

			return call_result;
		}

		if ((call_result = get_parsered_answer(session, response))) {
			// ERROR: On parsing the netconf response
			// The linked peer IPv4 can be left unchanged.

			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"Call to 'get_parsered_answer()' failed: "
				"hop_dn == <%s>: element_name == <%s>: op == <%d>: cmd == <%d>: "
				"peer_ipv4_dn == <%s>: call_result == <%d>",
				hop_dn, element_name, op, cmd, peer_ipv4_dn, call_result);

			return call_result;
		}

		return ERR_NO_ERROR;
	}

	int manager::create_vrrp_session (
			const fixs_ith_switchboardinfo & switch_board,
			const char * vrrp_session_dn,
			const configdataList & vrrp_session_properties,
			bool use_merge) {
		const char * const element_name = vrrpv3IPv4Session::CLASS_NAME;
		const char * const element_instance_name = vrrpv3IPv4Session::attribute::KEY;
		operation::OperationType op = (use_merge ? operation::MERGE : operation::CREATE);
		command::CommandType cmd = command::CMD_EDITCONFIG;

		Session session;
		parser::Response response;

		int call_result = handle_config_element(switch_board, vrrp_session_dn, element_name,
				element_instance_name, op, cmd, &vrrp_session_properties, session);

		if (call_result) {
			// ERROR: On creating the Vrrpv3IPv4Session config element.
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"Call to 'handle_config_element()' failed: Error on creating the Vrrpv3IPv4Session "
				"config element: vrrp_session_dn == <%s>: element_name == <%s>: op == <%d>: cmd == <%d>: "
				"call_result == <%d>",
				vrrp_session_dn, element_name, op, cmd, call_result);

			return call_result;
		}

		if ((call_result = get_parsered_answer(session, response))) {
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"Call to 'get_parsered_answer()' failed: "
				"vrrp_session_dn == <%s>: element_name == <%s>: op == <%d>: cmd == <%d>: "
				"call_result == <%d>",
				vrrp_session_dn, element_name, op, cmd, call_result);

			return call_result;
		}

		return ERR_NO_ERROR;
	}

	int manager::delete_vrrp_session (
			const fixs_ith_switchboardinfo & switch_board,
			const char * vrrp_session_dn) {
		const char * const element_name = vrrpv3IPv4Session::CLASS_NAME;
		operation::OperationType op = operation::DELETE;
		command::CommandType cmd = command::CMD_EDITCONFIG;

		Session session;
		parser::Response response;

		int call_result = handle_config_element(switch_board, vrrp_session_dn, element_name, 0, op, cmd, 0, session);

		if (call_result) {
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"Call to 'handle_config_element()' failed: Error on deleting the Vrrpv3IPv4Session config "
				"element: vrrp_session_dn == <%s>: element_name == <%s>: op == <%d>: cmd == <%d>, "
				"call_result == <%d>", vrrp_session_dn, element_name, op, cmd, call_result);

			return call_result;
		}

		if ((call_result = get_parsered_answer(session, response))) {
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"Call to 'get_parsered_answer()' failed: "
				"vrrp_session_dn == <%s>: element_name == <%s>: op == <%d>: cmd == <%d>, call_result == <%d>",
				vrrp_session_dn, element_name, op, cmd, call_result);

			return call_result;
		}

		return ERR_NO_ERROR;
	}

	int manager::modify_vrrp_session (
			const fixs_ith_switchboardinfo & switch_board,
			const char * vrrp_session_dn,
			const configdataList & vrrp_session_properties,
			bool use_merge) {
		const char * const element_name = vrrpv3IPv4Session::CLASS_NAME;
		const char * const element_instance_name = vrrpv3IPv4Session::attribute::KEY;
		operation::OperationType op = (use_merge ? operation::MERGE : operation::REPLACE);
		command::CommandType cmd = command::CMD_EDITCONFIG;

		Session session;
		parser::Response response;

		int call_result = handle_config_element(switch_board, vrrp_session_dn, element_name,
				element_instance_name, op, cmd, &vrrp_session_properties, session);

		if (call_result) {
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"Call to 'handle_config_element()' failed: Error on modifying the Vrrpv3IPv4Session "
				"config element: vrrp_session_dn == <%s>: element_name == <%s>: op == <%d>: "
				"cmd == <%d>, call_result == <%d>", vrrp_session_dn, element_name, op, cmd, call_result);

			return call_result;
		}

		if ((call_result = get_parsered_answer(session, response))) {
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"Call to 'get_parsered_answer()' failed: "
				"vrrp_session_dn == <%s>: element_name == <%s>: op == <%d>: cmd == <%d>, call_result == <%d>",
				vrrp_session_dn, element_name, op, cmd, call_result);

			return call_result;
		}

		return ERR_NO_ERROR;
	}

	int manager::get_vrrp_session (
			configdataList & config_data,
			const fixs_ith_switchboardinfo & switch_board,
			const char * vrrp_session_dn) {
		const char * const element_name = vrrpv3IPv4Session::CLASS_NAME;
		operation::OperationType op = operation::NO_OPERATION;
		command::CommandType cmd = command::CMD_GET;

		Session session;
		parser::Response response;

		int call_result = handle_config_element(switch_board, vrrp_session_dn, element_name,
				0, op, cmd, 0, session);

		if (call_result) {
			// ERROR: On getting attributes of a Vrrpv3IPv4Session config element.
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"Call to 'handle_config_element()' failed: Error on getting the Vrrpv3IPv4Session "
				"config element info: vrrp_session_dn == <%s>: element_name == <%s>: op == <%d>: "
				"cmd == <%d>: call_result == <%d>",
				vrrp_session_dn, element_name, op, cmd, call_result);

			return call_result;
		}

		if ((call_result = get_parsered_answer(session, response))) {
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"Call to 'get_parsered_answer()' failed: "
				"vrrp_session_dn == <%s>: element_name == <%s>: op == <%d>: cmd == <%d>: "
				"call_result == <%d>",
				vrrp_session_dn, element_name, op, cmd, call_result);

			return call_result;
		}

		configdataList::const_iterator it = config_data.begin();
		while ((it != config_data.end()) && (it->first != vrrpv3IPv4Session::attribute::KEY))
			++it;

		return (
			(it != config_data.end() && response.getAllAttributeOf(element_name, *it, config_data))
				? ERR_NO_ERROR
				: ERR_DATA_MISSING
		);
	}

	int manager::create_vrrp_IPv4_address (
			const fixs_ith_switchboardinfo & switch_board,
			const char * vrrp_ipv4_address_dn,
			const configdataList & vrrp_ipv4_address_properties,
			bool use_merge) {
		const char * const element_name = protVirtIPv4Address::CLASS_NAME;
		const char * const element_instance_name = protVirtIPv4Address::attribute::KEY;
		operation::OperationType op = (use_merge ? operation::MERGE : operation::CREATE);
		command::CommandType cmd = command::CMD_EDITCONFIG;

		Session session;
		parser::Response response;

		int call_result = handle_config_element(switch_board, vrrp_ipv4_address_dn, element_name,
				element_instance_name, op, cmd, &vrrp_ipv4_address_properties, session);

		if (call_result) {
			// ERROR: On creating the Vrrpv3IPv4Session config element.
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"Call to 'handle_config_element()' failed: Error on creating the ProtVirtIPv4Address "
				"config element: vrrp_ipv4_address_dn == <%s>: element_name == <%s>: op == <%d>: "
				"cmd == <%d>: call_result == <%d>",
				vrrp_ipv4_address_dn, element_name, op, cmd, call_result);

			return call_result;
		}

		if ((call_result = get_parsered_answer(session, response))) {
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"Call to 'get_parsered_answer()' failed: "
				"vrrp_ipv4_address_dn == <%s>: element_name == <%s>: op == <%d>: cmd == <%d>: "
				"call_result == <%d>", vrrp_ipv4_address_dn, element_name, op, cmd, call_result);

			return call_result;
		}

		return ERR_NO_ERROR;
	}

	int manager::delete_vrrp_IPv4_address (
			const fixs_ith_switchboardinfo & switch_board,
			const char * vrrp_ipv4_address_dn) {
		const char * const element_name = protVirtIPv4Address::CLASS_NAME;
		operation::OperationType op = operation::DELETE;
		command::CommandType cmd = command::CMD_EDITCONFIG;

		Session session;
		parser::Response response;

		int call_result = handle_config_element(switch_board, vrrp_ipv4_address_dn, element_name,
				0, op, cmd, 0, session);

		if (call_result) {
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"Call to 'handle_config_element()' failed: Error on deleting the ProtVirtIPv4Address config "
				"element: vrrp_ipv4_address_dn == <%s>: element_name == <%s>: op == <%d>: cmd == <%d>, "
				"call_result == <%d>", vrrp_ipv4_address_dn, element_name, op, cmd, call_result);

			return call_result;
		}

		if ((call_result = get_parsered_answer(session, response))) {
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"Call to 'get_parsered_answer()' failed: "
				"vrrp_ipv4_address_dn == <%s>: element_name == <%s>: op == <%d>: cmd == <%d>, "
				"call_result == <%d>", vrrp_ipv4_address_dn, element_name, op, cmd, call_result);

			return call_result;
		}

		return ERR_NO_ERROR;
	}

	int manager::modify_vrrp_IPv4_address (
			const fixs_ith_switchboardinfo & switch_board,
			const char * vrrp_ipv4_address_dn,
			const configdataList & vrrp_ipv4_address_properties,
			bool use_merge) {
		const char * const element_name = protVirtIPv4Address::CLASS_NAME;
		const char * const element_instance_name = protVirtIPv4Address::attribute::KEY;
		operation::OperationType op = (use_merge ? operation::MERGE : operation::REPLACE);
		command::CommandType cmd = command::CMD_EDITCONFIG;

		Session session;
		parser::Response response;

		int call_result = handle_config_element(switch_board, vrrp_ipv4_address_dn, element_name,
				element_instance_name, op, cmd, &vrrp_ipv4_address_properties, session);

		if (call_result) {
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"Call to 'handle_config_element()' failed: Error on modifying the ProtVirtIPv4Address "
				"config element: vrrp_ipv4_address_dn == <%s>: element_name == <%s>: op == <%d>: "
				"cmd == <%d>, call_result == <%d>", vrrp_ipv4_address_dn, element_name, op, cmd, call_result);

			return call_result;
		}

		if ((call_result = get_parsered_answer(session, response))) {
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"Call to 'get_parsered_answer()' failed: "
				"vrrp_ipv4_address_dn == <%s>: element_name == <%s>: op == <%d>: cmd == <%d>, "
				"call_result == <%d>", vrrp_ipv4_address_dn, element_name, op, cmd, call_result);

			return call_result;
		}

		return ERR_NO_ERROR;
	}

	int manager::get_vrrp_IPv4_address (
			configdataList & config_data,
			const fixs_ith_switchboardinfo & switch_board,
			const char * vrrp_ipv4_address_dn) {
		const char * const element_name = protVirtIPv4Address::CLASS_NAME;
		operation::OperationType op = operation::NO_OPERATION;
		command::CommandType cmd = command::CMD_GET;

		Session session;
		parser::Response response;

		int call_result = handle_config_element(switch_board, vrrp_ipv4_address_dn, element_name,
				0, op, cmd, 0, session);

		if (call_result) {
			// ERROR: On getting the ProtVirtIPv4Address config element.
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"Call to 'handle_config_element()' failed: Error on getting the ProtVirtIPv4Address "
				"config element info: vrrp_ipv4_address_dn == <%s>: element_name == <%s>: op == <%d>: "
				"cmd == <%d>: call_result == <%d>",
				vrrp_ipv4_address_dn, element_name, op, cmd, call_result);

			return call_result;
		}

		if ((call_result = get_parsered_answer(session, response))) {
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"Call to 'get_parsered_answer()' failed: "
				"vrrp_ipv4_address_dn == <%s>: element_name == <%s>: op == <%d>: cmd == <%d>: "
				"call_result == <%d>",
				vrrp_ipv4_address_dn, element_name, op, cmd, call_result);

			return call_result;
		}

		configdataList::const_iterator it = config_data.begin();
		while ((it != config_data.end()) && (it->first != protVirtIPv4Address::attribute::KEY))
			++it;

		return (
			(it != config_data.end() && response.getAllAttributeOf(element_name, *it, config_data))
				? ERR_NO_ERROR
				: ERR_DATA_MISSING
		);
	}


	int manager::get_next_hop (
			configdataList & next_hop_data,
			const fixs_ith_switchboardinfo& switch_board,
			const char* hop_dn)
	{
		char sb_ipn_addresses [FIXS_ITH_CONFIG_IPN_COUNT][16] = {};

		if(fixs_ith_sbdatamanager::get_switch_board_target_ipns(sb_ipn_addresses[0], sb_ipn_addresses[1], switch_board, fixs_ith::TRANSPORT_PLANE))
			return ERR_IP_ADDRESS_NOT_FOUND;

		if(!next_hop_data.size()) {
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Configuration data not found: cannot get info of NEXTHOP <%s> on SMX having IPNA == %s and IPNB == %s ", hop_dn, sb_ipn_addresses[0], sb_ipn_addresses[1]);
			return ERR_NO_CONFIGURATION_DATA;
		}

		char next_hop_path[XML_PATH_MAX_SIZE] = {0};

		if(build_element_path_from_dn( next_hop_path, hop_dn)) return ERR_BUILD_ELEMENT_PATH;

		FIXS_ITH_LOG(LOG_LEVEL_INFO, "Get NEXTHOP <%s>: on SMX having IPNA == %s and IPNB == %s ",  next_hop_path, sb_ipn_addresses[0], sb_ipn_addresses[1]);

		char buffer [64 * 1024] = {0};
		netconf::get_dom_builder builder;

		const int buildResult = builder.build_dom(buffer, sizeof(buffer),  next_hop_path);

		if( (0 > buildResult) || (sizeof(buffer) <= static_cast<size_t>(buildResult) ) )
		{
			FIXS_ITH_LOG(LOG_LEVEL_INFO, "Building of xml get request for NEXTHOP <%s> failed, build_dom returns <%d>", next_hop_path, buildResult);
			return ERR_BUILD_CONFIG_DOM;
		}

		int sendResult = ERR_SEND_REQUEST_FAILED;
		::srandom(static_cast<unsigned>(::time(0)));
		int ipn_index = ::random() & 0x01;

		Session session;
		parser::Response response;

		for (int i = 0; ((sendResult == ERR_SEND_REQUEST_FAILED || sendResult == ERR_SSH_USERAUTH_FAILED) && i < FIXS_ITH_CONFIG_IPN_COUNT); ++i, ipn_index = (ipn_index + 1) & 0x01) {
			sendResult = send_request(sb_ipn_addresses[ipn_index], buffer, command::CMD_GET, session);

			if (sendResult == ERR_SSH_USERAUTH_FAILED) {
				FIXS_ITH_LOG(LOG_LEVEL_INFO,
						"RESETTING USER PASSWORD on switch board slot == %d, switch board IPN = '%s'", switch_board.slot_position, sb_ipn_addresses[ipn_index]);

				fixs_ith::workingSet_t::instance()->get_snmpmanager().reset_userpasswd(switch_board, authority_levels::ADVANCED);
			}
		}

		if( (ERR_NO_ERROR == sendResult) && (ERR_NO_ERROR == ( sendResult = get_parsered_answer(session, response)) ) )
		{
			// search nexthop key to get
			for( configdataList::const_iterator it = next_hop_data.begin() ; it != next_hop_data.end(); ++it)
			{
				if( strcmp(it->first.c_str(), nextHop::attribute::KEY) == 0U )
				{
					configdataElement nextHopMOKey = *it;
					response.getAllAttributeOf(nextHop::CLASS_NAME, nextHopMOKey, next_hop_data);
					break;
				}
			}
		}

		return sendResult;
	}

	int manager::create_ipv4_peer (
			const fixs_ith_switchboardinfo & switch_board,
			const char * peer_dn,
			const configdataList & peer_properties,
			bool use_merge) {
		const char * const element_name = peerIpv4::CLASS_NAME;
		const char * const element_instance_name = peerIpv4::attribute::KEY;
		operation::OperationType op = (use_merge ? operation::MERGE : operation::CREATE);
		command::CommandType cmd = command::CMD_EDITCONFIG;

		Session session;
		parser::Response response;

		int call_result = handle_config_element(switch_board, peer_dn, element_name,
				element_instance_name, op, cmd, &peer_properties, session);

		if (call_result) {
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"Call to 'handle_config_element()' failed: Error on creating the PEER IPv4 config element: "
				"peer_dn == <%s>: element_name == <%s>: op == <%d>: cmd == <%d>, call_result == <%d>",
				peer_dn, element_name, op, cmd, call_result);

			return call_result;
		}

		if ((call_result = get_parsered_answer(session, response))) {
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"Call to 'get_parsered_answer()' failed: "
				"peer_dn == <%s>: element_name == <%s>: op == <%d>: cmd == <%d>, call_result == <%d>",
				peer_dn, element_name, op, cmd, call_result);

			return call_result;
		}

		return ERR_NO_ERROR;
	}

	int manager::delete_ipv4_peer (
			const fixs_ith_switchboardinfo & switch_board,
			const char * peer_dn) {
		const char * const element_name = peerIpv4::CLASS_NAME;
		operation::OperationType op = operation::DELETE;
		command::CommandType cmd = command::CMD_EDITCONFIG;

		Session session;
		parser::Response response;

		int call_result = handle_config_element(switch_board, peer_dn, element_name, 0, op, cmd, 0, session);

		if (call_result) {
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"Call to 'handle_config_element()' failed: Error on deleting the PEER IPv4 config element: "
				"peer_dn == <%s>: element_name == <%s>: op == <%d>: cmd == <%d>, call_result == <%d>",
				peer_dn, element_name, op, cmd, call_result);

			return call_result;
		}

		if ((call_result = get_parsered_answer(session, response))) {
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"Call to 'get_parsered_answer()' failed: "
				"peer_dn == <%s>: element_name == <%s>: op == <%d>: cmd == <%d>, call_result == <%d>",
				peer_dn, element_name, op, cmd, call_result);

			return call_result;
		}

		return ERR_NO_ERROR;
	}

	int manager::modify_ipv4_peer (
			const fixs_ith_switchboardinfo & switch_board,
			const char * peer_dn,
			const configdataList & dst_properties,
			bool use_merge) {
		const char * const element_name = peerIpv4::CLASS_NAME;
		const char * const element_instance_name = peerIpv4::attribute::KEY;
		operation::OperationType op = (use_merge ? operation::MERGE : operation::REPLACE);
		command::CommandType cmd = command::CMD_EDITCONFIG;

		Session session;
		parser::Response response;

		int call_result = handle_config_element(switch_board, peer_dn, element_name,
				element_instance_name, op, cmd, &dst_properties, session);

		if (call_result) {
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"Call to 'handle_config_element()' failed: Error on modifying the PEER IPv4 config element: "
				"peer_dn == <%s>: element_name == <%s>: op == <%d>: cmd == <%d>, call_result == <%d>",
				peer_dn, element_name, op, cmd, call_result);

			return call_result;
		}

		if ((call_result = get_parsered_answer(session, response))) {
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"Call to 'get_parsered_answer()' failed: "
				"peer_dn == <%s>: element_name == <%s>: op == <%d>: cmd == <%d>, call_result == <%d>",
				peer_dn, element_name, op, cmd, call_result);

			return call_result;
		}

		return ERR_NO_ERROR;
	}

int manager::modify_ipv4_interface(const fixs_ith_switchboardinfo & switch_board,
		const char * interfaceDN, configdataList & interfaceData, bool use_merge)
{

	FIXS_ITH_LOG(LOG_LEVEL_INFO, "'Modify IPV4 Interface <%s>' request received ... ", interfaceDN);

	if(!interfaceData.size()) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Configuration data not found: cannot modify IPV4 Interface <%s> ", interfaceDN);
		return ERR_NO_CONFIGURATION_DATA;
	}

	return apply_ipv4_interface_config_request(switch_board, interfaceDN, interfaceData,
												(use_merge)?operation::MERGE: operation::REPLACE);

}

int  manager::modify_ipv4_address(const fixs_ith_switchboardinfo & switch_board,
				const char * addressDn,
				configdataList & addressData,
				bool use_merge)
{
	FIXS_ITH_LOG(LOG_LEVEL_INFO, "'Modify IPV4 Address <%s>' request received ... ", addressDn);

	if(!addressData.size()) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Configuration data not found: cannot modify IPV4 Address <%s> ", addressDn);
		return ERR_NO_CONFIGURATION_DATA;
	}

	return apply_ipv4_address_config_request(switch_board, addressDn, addressData,
												(use_merge)?operation::MERGE: operation::REPLACE);
}


int manager::delete_router(const fixs_ith_switchboardinfo & switch_board, const char * routerDN)
{

	FIXS_ITH_LOG(LOG_LEVEL_INFO, "'Delete router <%s>' request received ... ", routerDN);

	return apply_router_config_request(switch_board, routerDN, operation::DELETE);

}


int manager::delete_ipv4_interface(const fixs_ith_switchboardinfo & switch_board, const char * interfaceDN)
{

	FIXS_ITH_LOG(LOG_LEVEL_INFO, "'Delete IPV4 Interface <%s>' request received ... ", interfaceDN);
	configdataList configData;

	return apply_ipv4_interface_config_request(switch_board, interfaceDN, configData, operation::DELETE);
}


int manager::delete_ipv4_address(const fixs_ith_switchboardinfo & switch_board, const char * addressDN)
{

	FIXS_ITH_LOG(LOG_LEVEL_INFO, "'Delete IPV4 Address <%s>' request received ... ", addressDN);

	configdataList configData;

	return apply_ipv4_address_config_request(switch_board, addressDN, configData, operation::DELETE);

}

int manager::create_bfd_profile(const fixs_ith_switchboardinfo & switch_board,
		const char * dn_name,
		const configdataList & bfdProfileData,
		bool use_merge)
{

	const char * const element_name = bfdProfile::CLASS_NAME;
	const char * const element_instance_name = bfdProfile::attribute::KEY;
	operation::OperationType op = (use_merge ? operation::MERGE : operation::CREATE);
	command::CommandType cmd = command::CMD_EDITCONFIG;

	Session session;
	parser::Response response;

	int call_result = handle_config_element(switch_board, dn_name, element_name,
			element_instance_name, op, cmd, &bfdProfileData, session);

	if (call_result) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
			"Call to 'handle_config_element()' failed: Error on creating the BFD Profile config element: "
			"dn_name == <%s>: element_name == <%s>: op == <%d>: cmd == <%d>, call_result == <%d>",
			dn_name, element_name, op, cmd, call_result);

		return call_result;
	}

	if ((call_result = get_parsered_answer(session, response))) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
			"Call to 'get_parsered_answer()' failed: "
			"dn_name == <%s>: element_name == <%s>: op == <%d>: cmd == <%d>, call_result == <%d>",
			dn_name, element_name, op, cmd, call_result);

		return call_result;
	}

	return ERR_NO_ERROR;


}

int manager::modify_bfd_profile(const fixs_ith_switchboardinfo & switch_board,
		const char * dn_name,
		const configdataList & bfdProfileData,
		bool use_merge)
{

	const char * const element_name = bfdProfile::CLASS_NAME;
	const char * const element_instance_name = bfdProfile::attribute::KEY;
	operation::OperationType op = (use_merge ? operation::MERGE : operation::REPLACE);
	command::CommandType cmd = command::CMD_EDITCONFIG;

	Session session;
	parser::Response response;

	int call_result = handle_config_element(switch_board, dn_name, element_name,
			element_instance_name, op, cmd, &bfdProfileData, session);

	if (call_result) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
			"Call to 'handle_config_element()' failed: Error on modifying the BFD Profile config element: "
			"dn_name == <%s>: element_name == <%s>: op == <%d>: cmd == <%d>, call_result == <%d>",
			dn_name, element_name, op, cmd, call_result);

		return call_result;
	}

	if ((call_result = get_parsered_answer(session, response))) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
			"Call to 'get_parsered_answer()' failed: "
			"dn_name == <%s>: element_name == <%s>: op == <%d>: cmd == <%d>, call_result == <%d>",
			dn_name, element_name, op, cmd, call_result);

		return call_result;
	}

	return ERR_NO_ERROR;
}

int manager::delete_bfd_profile(const fixs_ith_switchboardinfo & switch_board,
		const char * dn_name)
{

	const char * const element_name = bfdProfile::CLASS_NAME;
	operation::OperationType op = operation::DELETE;
	command::CommandType cmd = command::CMD_EDITCONFIG;

	Session session;
	parser::Response response;

	int call_result = handle_config_element(switch_board, dn_name, element_name,
			0, op, cmd, 0, session);

	if (call_result) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
			"Call to 'handle_config_element()' failed: Error on deleting the BFD Profile config element: "
			"dn_name == <%s>: element_name == <%s>: op == <%d>: cmd == <%d>, call_result == <%d>",
			dn_name, element_name, op, cmd, call_result);

		return call_result;
	}

	if ((call_result = get_parsered_answer(session, response))) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
			"Call to 'get_parsered_answer()' failed: "
			"dst_dn == <%s>: element_name == <%s>: op == <%d>: cmd == <%d>, call_result == <%d>",
			dn_name, element_name, op, cmd, call_result);

		return call_result;
	}

	return ERR_NO_ERROR;
}

int manager::get_bfd_sessionInfo(configdataList & configData, const fixs_ith_switchboardinfo& switch_board,
				const char* dn_name )
{
	const char * const element_name = bfdSessionInfo::CLASS_NAME;
	//const char * const element_instance_name = dst::attribute::KEY;
	operation::OperationType op = operation::NO_OPERATION;
	command::CommandType cmd = command::CMD_GET;

	Session session;
	parser::Response response;

	int call_result = handle_config_element(switch_board, dn_name, element_name,
			0, op, cmd, 0, session);

	if (call_result) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
			"Call to 'handle_config_element()' failed: Error on getting the BFD Session Info config element: "
			"dn_name == <%s>: element_name == <%s>: op == <%d>: cmd == <%d>, call_result == <%d>",
			dn_name, element_name, op, cmd, call_result);

		return call_result;
	}

	if( ERR_NO_ERROR == ( call_result = get_parsered_answer(session, response)) )
	{
		// search dst key to get
		for( configdataList::const_iterator it = configData.begin() ; it != configData.end(); ++it)
		{
			if( strcmp(it->first.c_str(), bfdSessionInfo::attribute::KEY) == 0U )
			{
				configdataElement bfdSessionMOKey = *it;
				response.getAllAttributeOf(element_name, bfdSessionMOKey, configData);
				break;
			}
		}
	}

	return call_result;
}

int manager::create_acl_entry (
		const fixs_ith_switchboardinfo & switch_board,
		const char * dn_name,
		const configdataList & attributes,
		const std::vector<configdataList> & conditions,
		const configdataList & action,
		bool use_merge) {
	const char * const element_name = rule::CLASS_NAME;
	const char * const element_instance_name = rule::attribute::KEY;
	operation::OperationType op = (use_merge ? operation::MERGE : operation::CREATE);
	command::CommandType cmd = command::CMD_EDITCONFIG;

	Session session;
	parser::Response response;

	int call_result = handle_config_element_acl(switch_board, dn_name, element_name, element_instance_name, op, cmd, &attributes, &conditions, &action, session);

	if (call_result) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
			"Call to 'handle_config_element_acl()' failed: Error on creating the ACL ENTRY config element: "
			"dn_name == <%s>: element_name == <%s>: op == <%d>: cmd == <%d>, call_result == <%d>",
			dn_name, element_name, op, cmd, call_result);

		return call_result;
	}

	if ((call_result = get_parsered_answer(session, response))) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
			"Call to 'get_parsered_answer()' failed: "
			"dn_name == <%s>: element_name == <%s>: op == <%d>: cmd == <%d>, call_result == <%d>",
			dn_name, element_name, op, cmd, call_result);

		return call_result;
	}

	return ERR_NO_ERROR;
}

int manager::modify_acl_entry (
		const fixs_ith_switchboardinfo & switch_board,
		const char * dn_name,
		const configdataList & attributes,
		const std::vector<configdataList> & conditions,
		const configdataList & action,
		bool use_merge) {
	const char * const element_name = rule::CLASS_NAME;
	const char * const element_instance_name = rule::attribute::KEY;
	operation::OperationType op = (use_merge ? operation::MERGE : operation::REPLACE);
	command::CommandType cmd = command::CMD_EDITCONFIG;

	Session session;
	parser::Response response;

	int call_result = handle_config_element_acl(switch_board, dn_name, element_name, element_instance_name, op, cmd, &attributes, &conditions, &action, session);

	if (call_result) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
			"Call to 'handle_config_element_acl()' failed: Error on modifying the ACL ENTRY config element: "
			"dn_name == <%s>: element_name == <%s>: op == <%d>: cmd == <%d>, call_result == <%d>",
			dn_name, element_name, op, cmd, call_result);

		return call_result;
	}

	if ((call_result = get_parsered_answer(session, response))) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
			"Call to 'get_parsered_answer()' failed: "
			"dn_name == <%s>: element_name == <%s>: op == <%d>: cmd == <%d>, call_result == <%d>",
			dn_name, element_name, op, cmd, call_result);

		return call_result;
	}

	return ERR_NO_ERROR;
}

int manager::delete_acl_entry (const fixs_ith_switchboardinfo & switch_board, const char * dn_name) {
	const char * const element_name = rule::CLASS_NAME;
	operation::OperationType op = operation::DELETE;
	command::CommandType cmd = command::CMD_EDITCONFIG;

	Session session;
	parser::Response response;

	int call_result = handle_config_element_acl(switch_board, dn_name, element_name, 0, op, cmd, 0, 0, 0, session);

	if (call_result) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
			"Call to 'handle_config_element_acl()' failed: Error on deleting the ACL ENTRY config element: "
			"dn_name == <%s>: element_name == <%s>: op == <%d>: cmd == <%d>, call_result == <%d>",
			dn_name, element_name, op, cmd, call_result);

		return call_result;
	}

	if ((call_result = get_parsered_answer(session, response))) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
			"Call to 'get_parsered_answer()' failed: "
			"dn_name == <%s>: element_name == <%s>: op == <%d>: cmd == <%d>, call_result == <%d>",
			dn_name, element_name, op, cmd, call_result);

		return call_result;
	}

	return ERR_NO_ERROR;
}

int manager::delete_acl_ipv4 (const fixs_ith_switchboardinfo & switch_board, const char * dn_name) {
	const char * const element_name = policy::CLASS_NAME;
	operation::OperationType op = operation::DELETE;
	command::CommandType cmd = command::CMD_EDITCONFIG;

	Session session;
	parser::Response response;

	int call_result = handle_config_element_acl(switch_board, dn_name, element_name, 0, op, cmd, 0, 0, 0, session);

	if (call_result) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
			"Call to 'handle_config_element_acl()' failed: Error on deleting the ACL POLICY config element: "
			"dn_name == <%s>: element_name == <%s>: op == <%d>: cmd == <%d>, call_result == <%d>",
			dn_name, element_name, op, cmd, call_result);

		return call_result;
	}

	if ((call_result = get_parsered_answer(session, response))) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
			"Call to 'get_parsered_answer()' failed: "
			"dn_name == <%s>: element_name == <%s>: op == <%d>: cmd == <%d>, call_result == <%d>",
			dn_name, element_name, op, cmd, call_result);

		return call_result;
	}

	return ERR_NO_ERROR;
}

int manager::enable_acl(const fixs_ith_switchboardinfo & switch_board)
{
	FIXS_ITH_LOG(LOG_LEVEL_INFO, "Enabling ACL on switch board in magazine '%u', slot '%d' (creating '%s' PolicyGroup) ... ", switch_board.magazine, switch_board.slot_position, ITH_POLICYGROUP_ID);

	config_request request(operation::MERGE);

	char buffer[64 * 1024] = {0};
	::snprintf(buffer, FIXS_ITH_ARRAY_SIZE(buffer), CREATE_ITH_POLICYGROUP_CONFIG_DOC_TEMPLATE, ITH_POLICYGROUP_ID);

	::memset(request.config_doc, 0, FIXS_ITH_ARRAY_SIZE(request.config_doc));
	::strncpy(request.config_doc, buffer, FIXS_ITH_ARRAY_SIZE(request.config_doc) - 1);

	Session session;
	parser::Response response;

	if (const int send_result = send_edit_config_request(switch_board, request.config_doc, session)){
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
							"Call 'send_edit_config_request' failed: send_result == %d", send_result);

		return send_result;
	}

	// Check if the configuration request has been successful applied on the target switch board
	if(const int get_result = get_parsered_answer(session, response)){
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"Call 'get_parsered_answer' failed: error_code == %d", get_result);
		return get_result;
	}

	FIXS_ITH_LOG(LOG_LEVEL_INFO, "Successfully enabled ACL on switch board in magazine '%u', slot '%d' (creating '%s' PolicyGroup) ... ", switch_board.magazine, switch_board.slot_position, ITH_POLICYGROUP_ID);
	return ERR_NO_ERROR;
}

int manager::get_acl_status(const fixs_ith_switchboardinfo & switch_board, bool & acl_status)
{
	int result = ERR_NO_ERROR;

	char sb_ipn_addresses [FIXS_ITH_CONFIG_IPN_COUNT][16] = {};

	if(fixs_ith_sbdatamanager::get_switch_board_target_ipns(sb_ipn_addresses[0], sb_ipn_addresses[1], switch_board, fixs_ith::TRANSPORT_PLANE))
		return ERR_IP_ADDRESS_NOT_FOUND;

	FIXS_ITH_LOG(LOG_LEVEL_INFO, "Get info about PolicyGroup having ID '%s' on switch board having IPNA == %s and IPNB == %s ", ITH_POLICYGROUP_ID, sb_ipn_addresses[0], sb_ipn_addresses[1]);

	char buffer [64 * 1024] = {0};
	::snprintf(buffer, FIXS_ITH_ARRAY_SIZE(buffer), GET_ITH_POLICYGROUP_CONFIG_DOC_TEMPLATE, ITH_POLICYGROUP_ID);

	int sendResult = ERR_SEND_REQUEST_FAILED;;
	::srandom(static_cast<unsigned>(::time(0)));
	int ipn_index = ::random() & 0x01;
	Session session;
	parser::Response response;

	for (int i = 0; ((sendResult == ERR_SEND_REQUEST_FAILED || sendResult == ERR_SSH_USERAUTH_FAILED) && i < FIXS_ITH_CONFIG_IPN_COUNT); ++i, ipn_index = (ipn_index + 1) & 0x01) {
		sendResult = send_request(sb_ipn_addresses[ipn_index],buffer, command::CMD_GET, session);

		if (sendResult == ERR_SSH_USERAUTH_FAILED) {
			FIXS_ITH_LOG(LOG_LEVEL_INFO,
					"RESETTING USER PASSWORD on switch board slot == %d, switch board IPN = '%s'", switch_board.slot_position, sb_ipn_addresses[ipn_index]);

			fixs_ith::workingSet_t::instance()->get_snmpmanager().reset_userpasswd(switch_board, authority_levels::ADVANCED);
		}
	}
	if(ERR_NO_ERROR == sendResult)
	{
		int get_answer_res = get_parsered_answer(session, response);
		if(get_answer_res == ERR_NO_ERROR )
		{
			acl_status = true; // Policy group found, so ACL handling is available
			FIXS_ITH_LOG(LOG_LEVEL_INFO, "Found  PolicyGroup '%s' on switch board having IPNA == %s and IPNB == %s ", ITH_POLICYGROUP_ID, sb_ipn_addresses[0], sb_ipn_addresses[1]);
		}
		else
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "An error occurred while parsing the answer received for [GET PolicyGroup '%s'] request sent to switch board having IPNA == %s and IPNB == %s. ERROR_CODE == %d ", ITH_POLICYGROUP_ID, sb_ipn_addresses[0], sb_ipn_addresses[1], get_answer_res);
			result = get_answer_res;	// an error occurred (nothing can be said about ACL handling status)
		}
	}
	else if(ERR_DATA_MISSING == sendResult)
	{
		acl_status = false; // Policy group NOT found, so ACL handling is NOT available
		FIXS_ITH_LOG(LOG_LEVEL_INFO, "NOT found  PolicyGroup '%s' on switch board having IPNA == %s and IPNB == %s ", ITH_POLICYGROUP_ID, sb_ipn_addresses[0], sb_ipn_addresses[1]);
	}
	else
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "An error occurred while sending GET request to retrieve PolicyGroup '%s' on switch board having IPNA == %s and IPNB == %s. ERROR_CODE = %d ", ITH_POLICYGROUP_ID, sb_ipn_addresses[0], sb_ipn_addresses[1], sendResult);
		result = sendResult;	// unexpected error (nothing can be said about ACL handling status)
	}

	return result;
}

int manager::attach_acl_to_interface(const fixs_ith_switchboardinfo& switch_board,
				const char* routerName,
				const uint16_t vlanId,
				const std::string& aclName)
{
	std::string routerId;
	int result = get_virtual_router_id(switch_board, routerName, routerId);

	if(ERR_NO_ERROR != result)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Failed to get Virtual Router of router:<%s> ", routerName);
		return result;
	}

	FIXS_ITH_LOG(LOG_LEVEL_INFO, "Virtual Router<%s> of router:<%s> ", routerId.c_str(), routerName);

	result = set_acl_on_interface(switch_board, routerId.c_str(), vlanId, aclName);

	return result;
}

int manager::detach_acl_to_interface(const fixs_ith_switchboardinfo& switch_board,
				const char* routerName,
				const uint16_t vlanId)
{
	std::string routerId;
	int result = get_virtual_router_id(switch_board, routerName, routerId);

	if(ERR_NO_ERROR != result)
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Failed to get Virtual Router of router:<%s> ", routerName);
		return result;
	}

	FIXS_ITH_LOG(LOG_LEVEL_INFO, "Virtual Router<%s> of router:<%s> ", routerId.c_str(), routerName);

	result = reset_acl_on_interface(switch_board, routerId.c_str(), vlanId);

	return result;
}

// *** PRIVATE METHODS  *** //

int manager::apply_router_config_request(const fixs_ith_switchboardinfo & switch_board,
			const char * routerDN, operation::OperationType oper)
	{

		FIXS_ITH_LOG(LOG_LEVEL_INFO, "Applying Router Configuration Request on <%s> ", routerDN);

		config_request request(oper);

		if (const int result = prepare_router_edit_config_request(&request, routerDN)) {

			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'prepare_router_edit_config_request' failed: routerDN == <%s>: error_code == %d: ",
					routerDN, result);

			return ERR_PREPARE_EDIT_CONFIG_REQUEST;
		}

		Session session;
		parser::Response response;

		if (const int send_result = send_edit_config_request(switch_board, request.config_doc, session)){
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
								"Call 'send_edit_config_request' failed: send_result == %d", send_result);

			return send_result;


		}

		// Check if the configuration request has been successful applied on the target switch board
		if(const int get_result = get_parsered_answer(session, response)){
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
					"Call 'get_parsered_answer' failed: error_code == %d", get_result);
			return get_result;
		}

		FIXS_ITH_LOG(LOG_LEVEL_INFO, "Router Configuration Request successful applied");
		return ERR_NO_ERROR;
	}

int manager::apply_ipv4_interface_config_request(const fixs_ith_switchboardinfo & switch_board,
			const char * interfaceDN, configdataList & configData, operation::OperationType oper)
{

	FIXS_ITH_LOG(LOG_LEVEL_INFO, "Applying IPV4 Interface Configuration Request on <%s> ", interfaceDN);

	config_request request(oper);

	if (const int result = prepare_interfaceIPv4_edit_config_request(&request, interfaceDN, configData)) {

		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'prepare_interfaceIPv4_edit_config_request' failed: interfaceDN == <%s>: error_code == %d: ",
				interfaceDN, result);

		return ERR_PREPARE_EDIT_CONFIG_REQUEST;
	}

	Session session;
	parser::Response response;

	if (const int send_result = send_edit_config_request(switch_board, request.config_doc, session)){
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
							"Call 'send_edit_config_request' failed: error_code == %d", send_result);
		return send_result;
	}

	// Check if the configuration request has been successful applied on the target switch board
	if(const int get_result = get_parsered_answer(session, response)){
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"Call 'get_parsered_answer' failed: error_code == %d", get_result);
		return get_result;
	}

	return ERR_NO_ERROR;
}

int manager::apply_ipv4_address_config_request(const fixs_ith_switchboardinfo & switch_board,
			const char * addressDN, configdataList & configData, operation::OperationType oper)
{
	FIXS_ITH_LOG(LOG_LEVEL_INFO, "Applying IPV4 Address Configuration Request on <%s> ", addressDN);

	config_request request(oper);

	if (const int result = prepare_addressIPv4_edit_config_request(&request, addressDN, configData)) {

		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'prepare_addressIPv4_edit_config_request' failed: addressDN == <%s>: error_code == %d: ",
				addressDN, result);

		return ERR_PREPARE_EDIT_CONFIG_REQUEST;
	}

	Session session;
	parser::Response response;

	if (const int send_result = send_edit_config_request(switch_board, request.config_doc, session)){
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
							"Call 'send_edit_config_request' failed: error_code == %d", send_result);
		return send_result;
	}

	// Check if the configuration request has been successful applied on the target switch board
	if(const int get_result = get_parsered_answer(session, response)){
		FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"Call 'get_parsered_answer' failed: error_code == %d", get_result);
		return get_result;
	}

	return ERR_NO_ERROR;
}


	int manager::send_edit_config_request(const fixs_ith_switchboardinfo & switch_board,
			const std::string & xml_data, Session& session)
	{
		char sb_ipn_addresses [FIXS_ITH_CONFIG_IPN_COUNT][16] = {};

		if(fixs_ith_sbdatamanager::get_switch_board_target_ipns(sb_ipn_addresses[0], sb_ipn_addresses[1], switch_board, fixs_ith::TRANSPORT_PLANE))
				return netconf::ERR_IP_ADDRESS_NOT_FOUND;

		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Sending Edit Configuration Request to switch board in slot == %d, having IPNA = '%s' IPNB == '%s'",
								switch_board.slot_position, sb_ipn_addresses[0], sb_ipn_addresses[1]);

		int call_result = ERR_SEND_REQUEST_FAILED;
		::srandom(static_cast<unsigned>(::time(0)));
		int ipn_index = ::random() & 0x01;

		for (int i = 0; ((call_result == ERR_SEND_REQUEST_FAILED || call_result == ERR_SSH_USERAUTH_FAILED) && i < FIXS_ITH_CONFIG_IPN_COUNT); ++i, ipn_index = (ipn_index + 1) & 0x01) {
			call_result = send_request(sb_ipn_addresses[ipn_index],xml_data, command::CMD_EDITCONFIG, session);
			if (call_result != ERR_NO_ERROR) {
				FIXS_ITH_LOG(LOG_LEVEL_WARN,
						"Call 'send_request' failed : switch board slot == %d, switch board IPN = '%s': call_result == %d",
							switch_board.slot_position, sb_ipn_addresses[ipn_index], call_result);

				if (call_result == ERR_SSH_USERAUTH_FAILED) {
					FIXS_ITH_LOG(LOG_LEVEL_INFO,
							"RESETTING USER PASSWORD on switch board slot == %d, switch board IPN = '%s'", switch_board.slot_position, sb_ipn_addresses[ipn_index]);

					 fixs_ith::workingSet_t::instance()->get_snmpmanager().reset_userpasswd(switch_board, authority_levels::ADVANCED);
				}
			}
		}
		return call_result;
	}

	int manager::build_peer_ipv4_dns_by_hop_dn (
			char * peer_dn,
			size_t size,
			char * peer_smx_dn,
			size_t size_smx,
			const char * hop_dn,
			const char * peer_ipv4_id) {
		*peer_dn = 0;

		char hop_dn_buf [8 * 1024] = {0};
		memccpy(hop_dn_buf, hop_dn, 0, sizeof(hop_dn_buf));

		rdn_path rdnp(hop_dn_buf);

		int hop_dn_length = rdnp.split();
		if (hop_dn_length <= 0) {
			// ERROR: HOP DN path malformed
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"Bad or malformed NextHop dn: hop_dn == <%s>: hop_dn_length == <%d>", hop_dn, hop_dn_length);

			return ERR_BAD_DN;
		}

		int router_index = 0;

		// Search for the router config element
		for (router_index = 0;
				 (router_index < hop_dn_length) && (imm::router_attribute::RDN != rdnp[router_index].key());
				 ++router_index) ;
		if (router_index >= hop_dn_length) {
			// ERROR: Router config element not found
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"Router config element not found: hop_dn == <%s>: hop_dn_length == <%d>", hop_dn, hop_dn_length);

			return ERR_BAD_DN;
		}

		snprintf(peer_dn, size, "%s=%s,%s=%s",
		                 peerIpv4::attribute::KEY, peer_ipv4_id,
		                 imm::router_attribute::RDN.c_str(),
						 rdnp[router_index].value());

		snprintf(peer_smx_dn, size_smx, "%s=1,%s=1,%s=%s,%s=%s",
		                 managedElement::CLASS_NAME, transport::CLASS_NAME,
		                 router::CLASS_NAME, rdnp[router_index].value(),
		                 peerIpv4::CLASS_NAME, peer_ipv4_id);

		FIXS_ITH_LOG(LOG_LEVEL_INFO,
			"IPv4 peer DN built successful: peer_dn == <%s>: peer_smx_dn == <%s>: hop_dn == <%s>",
			peer_dn, peer_smx_dn, hop_dn);

		return ERR_NO_ERROR;
	}

	int manager::handle_ipv4_peer_by_next_hop (
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
			bool use_merge) {
		FIXS_ITH_LOG(LOG_LEVEL_INFO,
			"hop_dn = <%s>: op == <%d>: cmd == <%d>: use_merge == <%d>", hop_dn, op, cmd, use_merge);

		*peer_dn = 0;

		int call_result = build_peer_ipv4_dns_by_hop_dn(peer_dn, size, peer_smx_dn, size_smx, hop_dn, peer_ipv4_id);
		if (call_result) {
			// ERROR: on building the linked peer IPv4 DN for this Hop DN
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"Call to 'build_peer_ipv4_dn_by_hop_dn()' failed: on building the linked IPv4 peer "
				"DN for the Next Hop: hop_dn = <%s>: op == <%d>: cmd == <%d>: use_merge == <%d>: "
				"call_result == <%d>", hop_dn, op, cmd, use_merge, call_result);

			return call_result;
		}

		FIXS_ITH_LOG(LOG_LEVEL_INFO,
			"OP <%d> ongoing: peer_dn == <%s>: peer_smx_dn == <%s>: hop_dn == <%s>: cmd == <%d>: "
			"use_merge == <%d>", op, peer_dn, peer_smx_dn, hop_dn, cmd, use_merge);

		// Applying operation op to the linked IPv4 peer on the SMX device
		switch (op) {
		case operation::DELETE:
			call_result = delete_ipv4_peer(switch_board, peer_dn);
			break;
		case operation::REPLACE:
			call_result = modify_ipv4_peer(switch_board, peer_dn, *properties, use_merge);
			break;
		case operation::NO_OPERATION:
			call_result = get_ipv4_peer(*properties, switch_board, peer_dn);
			break;
		default:
			call_result = create_ipv4_peer(switch_board, peer_dn, *properties, use_merge);
			break;
		}
		if (call_result) {
			// ERROR: Applying OP to the linked peer IPv4
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"Call to '{create/delete/modify}_ipv4_peer()' failed: applying OP to the linked IPv4 "
				"peer for the Next Hop: hop_dn = <%s>: op == <%d>: peer_dn == <%s>:"
				"peer_smx_dn == <%s>: cmd == <%d>: use_merge == <%d>: call_result == <%d>",
				hop_dn, op, peer_dn, peer_smx_dn, cmd, use_merge, call_result);

			return call_result;
		}

		return ERR_NO_ERROR;
	}

	int manager::create_ipv4_peer_by_next_hop (
				char * peer_dn,
				size_t size,
				const fixs_ith_switchboardinfo & switch_board,
				const char * hop_dn,
				configdataList & hop_properties) {
		operation::OperationType op = operation::CREATE;
		command::CommandType cmd = command::CMD_EDITCONFIG;

		*peer_dn = 0;

		// Search for a possible next hop address property
		configdataList::iterator address_it = hop_properties.begin();
		while ((address_it != hop_properties.end()) &&
					 (address_it->first != imm::nextHop_attribute::ADDRESS)) ++address_it;

		if (address_it == hop_properties.end())
			return ERR_NO_ERROR; // No IPv4 peer address found: no needs to create a peer

		// build the ID of the peer using 'address' property and replacing '.' with '_' (example: 192_168_1_2 )
		std::string peer_ipv4_id = address_it->second;
		std::replace(peer_ipv4_id.begin(), peer_ipv4_id.end(),'.','_');

		FIXS_ITH_LOG(LOG_LEVEL_INFO,
			"This hop has a reference to a peer IPv4: "
			"a new peer IPv4 will be eventually created (if not already present): op == <%d>: hop_dn == <%s>", op, hop_dn);

		configdataList peer_properties;
		peer_properties.push_back(configdataElement(peerIpv4::attribute::ADDRESS, address_it->second));

		char peer_smx_dn [2 * 1024] = {0};

		// check if the peer already exists
		configdataList tmp_properties;
		tmp_properties.push_back(configdataElement(peerIpv4::attribute::ADDRESS, address_it->second));
		int call_result = handle_ipv4_peer_by_next_hop(peer_dn, size, peer_smx_dn, sizeof(peer_smx_dn),
						switch_board, hop_dn, peer_ipv4_id.c_str(), operation::NO_OPERATION, command::CMD_GET, & tmp_properties, true);

		if(call_result == ERR_NO_ERROR) {
			// OK, the peer object already exists. Set the address_it->second to the DN of the existing peerIPv4 element
			address_it->second = peer_smx_dn;
			return  ERR_OBJ_ALREADY_EXISTS;
		}

		if(call_result != ERR_DATA_MISSING) {
			// unable to check for peer existence
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"Call to 'handle_ipv4_peer_by_next_hop()' failed: on handling the linked IPv4 peer "
				"for the Next Hop: op == <%d>: hop_dn == <%s>: cmd == <%d>: "
				"call_result == <%d>", op, hop_dn, cmd, call_result);

			return call_result;
		}

		//  the peer doesn't exist (call_result == ERR_DATA_MISSING), try to create it
		call_result = handle_ipv4_peer_by_next_hop(peer_dn, size, peer_smx_dn, sizeof(peer_smx_dn),
				switch_board, hop_dn, peer_ipv4_id.c_str(), op, cmd, &peer_properties, true);
		if (call_result) {
			// ERROR: on building the linked peer IPv4 DN for this Hop DN
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"Call to 'handle_ipv4_peer_by_next_hop()' failed: on handling the linked IPv4 peer "
				"for the Next Hop: op == <%d>: hop_dn == <%s>: cmd == <%d>: "
				"call_result == <%d>", op, hop_dn, cmd, call_result);

			return call_result;
		}

		FIXS_ITH_LOG(LOG_LEVEL_INFO,
			"op == <%d>: hop_dn == <%s>: cmd == <%d>: peer_dn == <%s>: peer_smx_dn == <%s>",
			op, hop_dn, cmd, peer_dn, peer_smx_dn);

		// All right. Can set the address_it->second to the DN of the peerIPv4 element
		address_it->second = peer_smx_dn;

		return ERR_NO_ERROR;
	}

	int manager::delete_ipv4_peer_by_next_hop (
				const fixs_ith_switchboardinfo & switch_board,
				const char * hop_dn,
				const char * peer_address) {
		operation::OperationType op = operation::DELETE;
		command::CommandType cmd = command::CMD_EDITCONFIG;

		FIXS_ITH_LOG(LOG_LEVEL_INFO,
			"A possible peer IPv4 linked config element "
			"should be deleted: building its DN by hop_dn: op == <%d>: hop_dn == <%s>", op, hop_dn);

		if(!peer_address)
			return ERR_BAD_OBJECT_ID;

		std::string peer_ipv4_id(peer_address);
		std::replace(peer_ipv4_id.begin(), peer_ipv4_id.end(),'.','_');

		char peer_ipv4_dn [2 * 1024] = {0};
		char peer_smx_dn [2 * 1024] = {0};

		int call_result = handle_ipv4_peer_by_next_hop(peer_ipv4_dn, sizeof(peer_ipv4_dn),
				peer_smx_dn, sizeof(peer_smx_dn), switch_board, hop_dn, peer_ipv4_id.c_str(), op, cmd, 0);
		if (call_result) {
			// ERROR: on deleting the linked peer IPv4 DN for this Hop DN
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"Call to 'handle_ipv4_peer_by_next_hop()' failed: on deleting the linked IPv4 peer "
				"for the Next Hop: op == <%d>: hop_dn == <%s>: cmd == <%d>: "
				"call_result == <%d>", op, hop_dn, cmd, call_result);

			return call_result;
		}

		return ERR_NO_ERROR;
	}

	int manager::modify_ipv4_peer_by_next_hop (
			char * peer_dn,
			size_t size,
			const fixs_ith_switchboardinfo & switch_board,
			const char * hop_dn,
			configdataList & hop_properties) {
		operation::OperationType op = operation::REPLACE;
		command::CommandType cmd = command::CMD_EDITCONFIG;

		*peer_dn = 0;

		// Search for a possible next hop address property
		configdataList::iterator address_it = hop_properties.begin();
		while ((address_it != hop_properties.end()) &&
					 (address_it->first != imm::nextHop_attribute::ADDRESS)) ++address_it;

		if (address_it == hop_properties.end())
			return ERR_NO_ERROR; // No IPv4 peer address found: no needs to modify or create a peer

		FIXS_ITH_LOG(LOG_LEVEL_INFO,
			"This hop has a reference to a peer IPv4: "
			"Performing a change on an old peer IPv4 or a new peer IPv4 will be created: "
			"op == <%d>: hop_dn == <%s>", op, hop_dn);

		// build the ID of the peer using 'address' property and replacing '.' with '_' (example: 192_168_1_2 )
		std::string peer_ipv4_id = address_it->second;
		std::replace(peer_ipv4_id.begin(), peer_ipv4_id.end(),'.','_');

		configdataList peer_properties;
		peer_properties.push_back(configdataElement(peerIpv4::attribute::ADDRESS, address_it->second));

		char peer_smx_dn [2 * 1024] = {0};

		int call_result = handle_ipv4_peer_by_next_hop(peer_dn, size, peer_smx_dn, sizeof(peer_smx_dn),
				switch_board, hop_dn, peer_ipv4_id.c_str(), op, cmd, &peer_properties, true);
		if (call_result) {
			// ERROR: on modifying or creating the linked peer IPv4 DN for this Hop DN
			FIXS_ITH_LOG(LOG_LEVEL_ERROR,
				"Call to 'handle_ipv4_peer_by_next_hop()' failed: on handling the linked IPv4 peer "
				"for the Next Hop: op == <%d>: hop_dn == <%s>: cmd == <%d>: "
				"call_result == <%d>", op, hop_dn, cmd, call_result);

			return call_result;
		}

		FIXS_ITH_LOG(LOG_LEVEL_INFO,
			"op == <%d>: hop_dn == <%s>: cmd == <%d>: peer_dn == <%s>: peer_smx_dn == <%s>",
			op, hop_dn, cmd, peer_dn, peer_smx_dn);

		// All right. Can set the address_it->second to the DN of the peerIPv4 element
		address_it->second = peer_smx_dn;

		return ERR_NO_ERROR;
	}

	int manager::handle_config_element (
			const fixs_ith_switchboardinfo & switch_board,
			const char * dn_name,
			const char * element_name,
			const char * element_instance_name,
			operation::OperationType op,
			command::CommandType cmd,
			const configdataList * properties,
			Session &session) {
		char sb_ipn_addresses [FIXS_ITH_CONFIG_IPN_COUNT][16] = {0};

		if (fixs_ith_sbdatamanager::get_switch_board_target_ipns(
					sb_ipn_addresses[0], sb_ipn_addresses[1], switch_board, fixs_ith::TRANSPORT_PLANE))
			return netconf::ERR_IP_ADDRESS_NOT_FOUND;

		if (properties && properties->empty()) {
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Configuration data not found: "
				"dn_name == <%s>: element_name == <%s>: op == <%d>: cmd == <%d>: IPNA == <%s>: IPNB == <%s>",
				dn_name, element_name, op, cmd, sb_ipn_addresses[0], sb_ipn_addresses[1]);

			return ERR_NO_CONFIGURATION_DATA;
		}

		char rdn_path [XML_PATH_MAX_SIZE] = {0};

		if (build_element_path_from_dn(rdn_path, dn_name))
			return ERR_BUILD_ELEMENT_PATH;

		FIXS_ITH_LOG(LOG_LEVEL_INFO, "Configuration job ongoing: "
			"dn_name == <%s>: element_name == <%s>: op == <%d>: cmd == <%d>: IPNA == <%s>: IPNB == <%s>: "
			"rdn_path == <%s>",
			dn_name, element_name, op, cmd, sb_ipn_addresses[0], sb_ipn_addresses[1], rdn_path);

		std::vector<netconf::config_property> netconf_properties;
		char buffer [64 * 1024] = {0};
		int build_result = 0;

		if (properties) {
			for (configdataList::const_iterator it = properties->begin(); it != properties->end(); it++) {
				if (it->first == element_instance_name) continue;
				netconf_properties.push_back(netconf::config_property(it->first, it->second));
			}
			build_result = netconf::edit_config_dom_builder(op).build_dom(buffer, sizeof(buffer), rdn_path,
					netconf_properties);
		} else {
			switch (cmd) {
			case command::CMD_GET:
			case command::CMD_GETCONFIG:
				build_result = netconf::get_dom_builder().build_dom(buffer, sizeof(buffer), rdn_path);
				break;
			case command::CMD_EDITCONFIG:
				build_result = netconf::edit_config_dom_builder(op).build_dom(buffer, sizeof(buffer), rdn_path);
				break;
			default:
				build_result = -1;
				break;
			}
		}

		if ((build_result < 0) || (static_cast<size_t>(build_result) >= sizeof(buffer))) {
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Configuration document build failed: "
				"dn_name == <%s>: element_name == <%s>: op == <%d>: cmd == <%d>: IPNA == <%s>: IPNB == <%s>: "
				"rdn_path == <%s>: build_result == <%d>",
				dn_name, element_name, op, cmd, sb_ipn_addresses[0], sb_ipn_addresses[1], rdn_path, build_result);

			return ERR_BUILD_CONFIG_DOM;
		}

		int call_result = ERR_SEND_REQUEST_FAILED;
		::srandom(static_cast<unsigned>(::time(0)));
		int ipn_index = ::random() & 0x01;

		for (int i = 0; ((call_result == ERR_SEND_REQUEST_FAILED || call_result == ERR_SSH_USERAUTH_FAILED) && i < FIXS_ITH_CONFIG_IPN_COUNT); ++i, ipn_index = (ipn_index + 1) & 0x01) {
			call_result = send_request(sb_ipn_addresses[ipn_index],buffer, cmd, session);
			if (call_result) {
				FIXS_ITH_LOG(LOG_LEVEL_ERROR,
					"Call to 'send_request()' failed: dn_name == <%s>: element_name == <%s>: "
					"op == <%d>: cmd == <%d>: IPNA == <%s>: IPNB == <%s>: rdn_path == <%s>: "
					"call_result == <%d>", dn_name, element_name, op, cmd, sb_ipn_addresses[0],
					sb_ipn_addresses[1], rdn_path, call_result);

				if (call_result == ERR_SSH_USERAUTH_FAILED) {
					FIXS_ITH_LOG(LOG_LEVEL_INFO,
							"RESETTING USER PASSWORD on switch board slot == %d, switch board IPN = '%s'", switch_board.slot_position, sb_ipn_addresses[ipn_index]);

					 fixs_ith::workingSet_t::instance()->get_snmpmanager().reset_userpasswd(switch_board, authority_levels::ADVANCED);
				}
			}
		}

		return (call_result ? call_result : ERR_NO_ERROR);
	}

	int manager::handle_config_element_acl (
			const fixs_ith_switchboardinfo & switch_board,
			const char * dn_name,
			const char * element_name,
			const char * element_instance_name,
			operation::OperationType op,
			command::CommandType cmd,
			const configdataList * properties,
			const std::vector<configdataList> * conditions,
			const configdataList * action,
			Session &session) {
		char sb_ipn_addresses [FIXS_ITH_CONFIG_IPN_COUNT][16] = {0};

		if (fixs_ith_sbdatamanager::get_switch_board_target_ipns(
					sb_ipn_addresses[0], sb_ipn_addresses[1], switch_board, fixs_ith::TRANSPORT_PLANE))
			return netconf::ERR_IP_ADDRESS_NOT_FOUND;

		if (properties && properties->empty()) {
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Configuration data not found: "
				"dn_name == <%s>: element_name == <%s>: op == <%d>: cmd == <%d>: IPNA == <%s>: IPNB == <%s>",
				dn_name, element_name, op, cmd, sb_ipn_addresses[0], sb_ipn_addresses[1]);

			return ERR_NO_CONFIGURATION_DATA;
		}

		int call_result = 0;
		char rdn_path [XML_PATH_MAX_SIZE] = {0};

		if ((call_result = build_element_path_from_dn_acl(rdn_path, sizeof(rdn_path), dn_name))) {
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'build_element_path_from_dn_acl()' failed: "
				"dn_name == <%s>: element_name == <%s>: op == <%d>: cmd == <%d>: IPNA == <%s>: IPNB == <%s>, ",
				dn_name, element_name, op, cmd, sb_ipn_addresses[0], sb_ipn_addresses[1]);

			return ERR_BUILD_ELEMENT_PATH;
		}

		FIXS_ITH_LOG(LOG_LEVEL_INFO, "Configuration job ongoing: "
			"dn_name == <%s>: element_name == <%s>: op == <%d>: cmd == <%d>: IPNA == <%s>: IPNB == <%s>: "
			"rdn_path == <%s>",
			dn_name, element_name, op, cmd, sb_ipn_addresses[0], sb_ipn_addresses[1], rdn_path);

		std::vector<netconf::config_property> netconf_properties;
		char buffer [64 * 1024] = {0};
		int build_result = 0;

		if (properties) {
			for (configdataList::const_iterator it = properties->begin(); it != properties->end(); it++) {
				if (it->first == element_instance_name) continue;
				netconf_properties.push_back(netconf::config_property(it->first, it->second));
			}
			build_result = netconf::edit_config_dom_builder(op, "If", conditions, "Then", action).build_dom(buffer, sizeof(buffer), rdn_path,
					netconf_properties);
		} else {
			switch (cmd) {
			case command::CMD_GET:
			case command::CMD_GETCONFIG:
				build_result = netconf::get_dom_builder().build_dom(buffer, sizeof(buffer), rdn_path);
				break;
			case command::CMD_EDITCONFIG:
				build_result = netconf::edit_config_dom_builder(op, "If", conditions, "Then", action).build_dom(buffer, sizeof(buffer), rdn_path);
				break;
			default:
				build_result = -1;
				break;
			}
		}

		if ((build_result < 0) || (static_cast<size_t>(build_result) >= sizeof(buffer))) {
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Configuration document build failed: "
				"dn_name == <%s>: element_name == <%s>: op == <%d>: cmd == <%d>: IPNA == <%s>: IPNB == <%s>: "
				"rdn_path == <%s>: build_result == <%d>",
				dn_name, element_name, op, cmd, sb_ipn_addresses[0], sb_ipn_addresses[1], rdn_path, build_result);

			return ERR_BUILD_CONFIG_DOM;
		}

		call_result = ERR_SEND_REQUEST_FAILED;
		::srandom(static_cast<unsigned>(::time(0)));
		int ipn_index = ::random() & 0x01;


		for (int i = 0; ((call_result == ERR_SEND_REQUEST_FAILED || call_result == ERR_SSH_USERAUTH_FAILED) && i < FIXS_ITH_CONFIG_IPN_COUNT);
				++i, ipn_index = (ipn_index + 1) & 0x01) {
			call_result = send_request(sb_ipn_addresses[ipn_index],buffer, cmd, session);
			if (call_result) {
				FIXS_ITH_LOG(LOG_LEVEL_ERROR,
						"Call to 'send_request()' failed: dn_name == <%s>: element_name == <%s>: "
						"op == <%d>: cmd == <%d>: IPNA == <%s>: IPNB == <%s>: rdn_path == <%s>: "
						"call_result == <%d>", dn_name, element_name, op, cmd, sb_ipn_addresses[0],
						sb_ipn_addresses[1], rdn_path, call_result);

				if (call_result == ERR_SSH_USERAUTH_FAILED) {
					FIXS_ITH_LOG(LOG_LEVEL_INFO,
							"RESETTING USER PASSWORD on switch board slot == %d, switch board IPN = '%s'", switch_board.slot_position, sb_ipn_addresses[ipn_index]);

					fixs_ith::workingSet_t::instance()->get_snmpmanager().reset_userpasswd(switch_board, authority_levels::ADVANCED);
				}
			}
		}

		return (call_result ? call_result : ERR_NO_ERROR);
	}

	int manager::send_request(const char * switch_board_ip,
			const std::string & xml_data, command::CommandType cmd, Session& session)
	{
		FIXS_ITH_LOG (LOG_LEVEL_DEBUG, "'Sending request <request id == %d> xml_data == \"%s\"  on switch board %s", cmd, xml_data.c_str(), switch_board_ip);

		session.setRemoteAddress(switch_board_ip);

		int oper_result= session.performOperation(xml_data, cmd);

		// check if send xml command was successful
		if(oper_result != SUCCESS ){
			if (oper_result == FAILED && session.get_error_code()==acs_nclib::ERR_LIBSSH_USERAUTH_PASSWORD_METHOD_FAILED){
					FIXS_ITH_LOG(LOG_LEVEL_ERROR, "'performOperation' failed with error <ERR_LIBSSH_USERAUTH_PASSWORD_METHOD_FAILED> ! Cannot send NETCONF request to switch board.");
					return ERR_SSH_USERAUTH_FAILED;
			}
			if (oper_result == FAILED){
				FIXS_ITH_LOG(LOG_LEVEL_ERROR, "'performOperation' failed! Cannot send NETCONF request to switch board.");
				return ERR_SEND_REQUEST_FAILED;
			}
			return errorcode_from_errortag_msg(session);
		}

		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, " NETCONF request <request id == %d> SUCCESSFUL sent.", cmd);
		return ERR_NO_ERROR;
	}

	int manager::get_parsered_answer(Session &session, parser::Response &response)
	{
		std::string answer;

		answer= session.getXmlAnswer();

		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "'getXmlAnswer'returns answer == '%s'", answer.c_str());

		// check now if the command had a successful answer
		if(!response.parse(answer)) return ERR_PARSER_ANSWER_FAILED;

		return ERR_NO_ERROR;
	}

	int manager::errorcode_from_errortag_msg(Session &session)
	{
		std::string errmsg;
		errmsg= session.getErrorTag();

		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "'getErrTagMessage'returns error == '%s'", errmsg.c_str());

		if(!strcmp(errmsg.c_str(), netconf_errTagMessage::ERR_DATA_EXISTS)) return ERR_OBJ_ALREADY_EXISTS;
		if(!strcmp(errmsg.c_str(), netconf_errTagMessage::ERR_DATA_MISSING)) return ERR_DATA_MISSING;

		return ERR_OPERATION_FAILED;
	}

	int manager::build_element_path_from_dn (char * element_path, const char * element_dn) {
		char rdn_buf [16 * 1024] = {0};
		memccpy(rdn_buf, element_dn, 0, sizeof(rdn_buf));

		netconf::rdn_path rdnp(rdn_buf);

		int dn_length = rdnp.split();

		if (dn_length <=  1) return -1; // ERROR: DN path malformed

		int num_chars = 0, num_chars_tot = 0;

		int s_dn_lenght = dn_length -1;
		// if rdnp[dn_lenght-1] contains Transport root key: skip it!
		if(!strcmp(rdnp[dn_length-1].key(), imm::root_moc_attribute::RDN.c_str()))
			--s_dn_lenght;

		for (int i = s_dn_lenght; i >= 0; --i) {
			const rdn_path_item & rdn_item = rdnp[i];

			num_chars = ::snprintf(element_path + num_chars_tot, XML_PATH_MAX_SIZE,
				"%s=%s,", rdn_item.key(), rdn_item.value());
			num_chars_tot += num_chars;
		}

		// cut last ',' char and return to caller
		return (element_path[num_chars_tot - 1] = 0);
	}

	int manager::build_element_path_from_dn_acl (char * element_path, size_t size, const char * element_dn) {
		char rdn_buf [16 * 1024] = {0};
		memccpy(rdn_buf, element_dn, 0, sizeof(rdn_buf));

		netconf::rdn_path rdnp(rdn_buf);

		int dn_length = rdnp.split();

		if (dn_length <= 1) return -1; // ERROR: DN path malformed

		std::string imm_router_name;
		std::string imm_acl_ipv4_name;
		std::string imm_acl_entry_ipv4_name;

		// Extract all elements names
		while (--dn_length >= 0) {
			// The MOM root will be removed
			if (! strcmp(rdnp[dn_length].key(), imm::root_moc_attribute::RDN.c_str()))
				continue;
			else if (! strcmp(rdnp[dn_length].key(), imm::router_attribute::RDN.c_str()))
				imm_router_name = common::utility::getRouterNameFromRdnValue(rdnp[dn_length].value());
			else if (! strcmp(rdnp[dn_length].key(), imm::aclIpv4_attribute::RDN.c_str()))
				imm_acl_ipv4_name = rdnp[dn_length].value();
			else if (! strcmp(rdnp[dn_length].key(), imm::aclEntry_attribute::RDN.c_str()))
				imm_acl_entry_ipv4_name = rdnp[dn_length].value();
			else // ERROR: DN path malformed
				return -1;
		}

		// Here we can build the SMX configuration path to request the netconf create operation
		char element_path_format [128] = "%s";
		if (! imm_acl_ipv4_name.empty()) {
			strncpy(element_path_format + 2, ",%s=%s%c%s", sizeof(element_path_format) - 2);
			if (! imm_acl_entry_ipv4_name.empty())
				strncpy(element_path_format + 12, ",%s=%s", sizeof(element_path_format) - 12);
			element_path_format[sizeof(element_path_format) - 1] = 0;
		}

		int chars_wrote = snprintf(element_path, size, element_path_format, netconf::switch_mom_dn::POLICIES_BASE_DN,
				netconf::policy::CLASS_NAME, imm_router_name.c_str(), common::utility::parserTag::underscore,
				imm_acl_ipv4_name.c_str(), netconf::rule::CLASS_NAME, imm_acl_entry_ipv4_name.c_str());

		if ((chars_wrote <= 0) || (  static_cast<size_t>(chars_wrote) >= size)) // ERROR: Building the SMX DN Path
			return -2;

		return 0;
	}

	void manager::build_bfdProfileRef_from_dn( std::string & bfdProfileRef_str, const char * element_dn)
	{
		char  bfd_prof_ref[XML_PATH_MAX_SIZE] = {0};

		std::string router_name;
		common::utility::getRDNValue(element_dn, imm::router_attribute::RDN.c_str(), router_name);

		// get SMX Router name, e.g: "oam"
		std::string smxRouterName = common::utility::getRouterIdFromRouterName(router_name);

		std::string bfd_profile_name;
		common::utility::getRDNValue(element_dn, imm::bfdProfile_attribute::RDN.c_str(), bfd_profile_name);


		::snprintf(bfd_prof_ref, XML_PATH_MAX_SIZE, "%s,Router=%s,BfdProfile=%s",
				netconf::switch_mom_dn::TRANSPORT_DN,smxRouterName.c_str(), bfd_profile_name.c_str());

		bfdProfileRef_str.assign(bfd_prof_ref);
	}

	std::string manager::get_vlan_path(const std::string & encaps_vid)
	{
		std::string sb_encaps_str;
		char  sb_encaps[XML_PATH_MAX_SIZE] = {0};

		//std::string id_value = common::utility::getIdValueFromRdn(imm_encaps);

		::snprintf(sb_encaps, XML_PATH_MAX_SIZE, "%s,Vlan=%s",netconf::switch_mom_dn::BRIDGE_DN,encaps_vid.c_str());

		 sb_encaps_str.assign(sb_encaps);

		return sb_encaps_str;
	}

int manager::prepare_router_edit_config_request(config_request * request, const char * routerDN)
{
	char router_path[XML_PATH_MAX_SIZE] = {0};

	if(build_element_path_from_dn(router_path, routerDN))
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'build_element_path_from_dn' failed!: Cannot prepare Edit Configuration Request for Router with DN <%s>", routerDN);
		return ERR_BUILD_ELEMENT_PATH;
	}

	FIXS_ITH_LOG(LOG_LEVEL_INFO, "Preparing Edit Configuration Request for Router <%s> ", router_path);


	netconf::edit_config_dom_builder builder(request->oper_value);

	std::vector<netconf::config_property> properties;

	char buffer [64 * 1024] = {0};
	int call_result =
		builder.build_dom(buffer, sizeof(buffer), router_path, properties);

	if ((call_result < 0) || (static_cast<size_t>(call_result) >= sizeof(buffer))) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'build_dom' failed: router_path == <%s>: call_result == %d: ",
				router_path, call_result);
		return ERR_BUILD_CONFIG_DOM;
	}

	strncpy(request->config_doc, buffer, sizeof(request->config_doc));

	FIXS_ITH_LOG(LOG_LEVEL_INFO, "Edit Configuration Request successful prepared!");
	return ERR_NO_ERROR;
}



int manager::prepare_interfaceIPv4_edit_config_request(config_request * request, const char * interfaceDN, configdataList & configData)
{

	char interface_path[XML_PATH_MAX_SIZE] = {0};

	if(build_element_path_from_dn(interface_path, interfaceDN))
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'build_element_path_from_dn' failed!: Cannot prepare Edit Configuration Request for IPV4 Interface with DN <%s>", interfaceDN);
		return ERR_BUILD_ELEMENT_PATH;
	}

	FIXS_ITH_LOG(LOG_LEVEL_INFO, "Preparing Edit Configuration Request for IPV4 Interface <%s> ", interface_path);

	netconf::edit_config_dom_builder builder(request->oper_value);
	std::vector<netconf::config_property> properties;

	std::string  encapsvid_value;
	for ( configdataList::const_iterator it = configData.begin() ; it != configData.end(); it++){
        if(!strcmp(it->first.c_str(), interfaceIpv4::attribute::KEY)){
        	continue;
        }
        if(!strcmp(it->first.c_str(), interfaceIpv4::attribute::ENCAPSULATION_VID)){
        	encapsvid_value.assign(it->second);
            continue;
        }
        if(!strcmp(it->first.c_str(), interfaceIpv4::attribute::ENCAPSULATION)){
        	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "IMM ENCAPSULATION ATTRIBUTE FOUND IN CONFIGUARTION DATA '%s'",it->second.c_str());
        	continue;
        }
        if(!strcmp(it->first.c_str(), interfaceIpv4::attribute::BFD_PROFILE)){
        	std::string bfdProfile_reference;
        	build_bfdProfileRef_from_dn(bfdProfile_reference,it->second.c_str());
        	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "XML BFD PROFILE ATTRIBUTE is '%s'",bfdProfile_reference.c_str());
        	properties.push_back(netconf::config_property(interfaceIpv4::attribute::BFD_PROFILE, bfdProfile_reference.c_str()));
            continue;
        }
        properties.push_back(netconf::config_property(it->first.c_str(), it->second.c_str()));
	}

	if(encapsvid_value.size() != 0) {
		std::string encaps_value;
		encaps_value.assign(get_vlan_path(encapsvid_value));
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "XML ENCAPSULATION ATTRIBUTE is '%s'",encaps_value.c_str());
		properties.push_back(netconf::config_property(interfaceIpv4::attribute::ENCAPSULATION, encaps_value.c_str()));
	}

	char buffer [64 * 1024] = {0};
	int call_result =
		builder.build_dom(buffer, sizeof(buffer), interface_path, properties);

	if ((call_result < 0) || (static_cast<size_t>(call_result) >= sizeof(buffer))) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'build_dom' failed: interface_path == <%s>: call_result == %d: ",
					interface_path, call_result);
		return ERR_BUILD_CONFIG_DOM;
	}

	strncpy(request->config_doc, buffer, sizeof(request->config_doc));

	FIXS_ITH_LOG(LOG_LEVEL_INFO, "Edit Configuration Request successful prepared!");
	return ERR_NO_ERROR;

}


int manager::prepare_addressIPv4_edit_config_request(config_request * request, const char * addressDN, configdataList & configData)
{
	char address_path[XML_PATH_MAX_SIZE] = {0};

	if(build_element_path_from_dn(address_path, addressDN))
	{
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'build_element_path_from_dn' failed!: Cannot prepare Edit Configuration Request for IPV4 Address with DN <%s>", addressDN);
		return ERR_BUILD_ELEMENT_PATH;
	}

	FIXS_ITH_LOG(LOG_LEVEL_INFO, "Preparing Edit Configuration Request for IPV4 Address <%s> ", address_path);


	netconf::edit_config_dom_builder builder(request->oper_value);

	std::vector<netconf::config_property> properties;

	for ( configdataList::const_iterator it = configData.begin() ; it != configData.end(); it++){
		if(!strcmp(it->first.c_str(), addressIpv4::attribute::KEY)){
			continue;
		}
		properties.push_back(netconf::config_property(it->first.c_str(), it->second.c_str()));
	}

	char buffer [64 * 1024] = {0};
	int call_result =
		builder.build_dom(buffer, sizeof(buffer), address_path, properties);

	if ((call_result < 0) || (static_cast<size_t>(call_result) >= sizeof(buffer))) {
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'build_dom' failed: address_path == <%s>: call_result == %d: ",
				address_path, call_result);
		return ERR_BUILD_CONFIG_DOM;
	}

	strncpy(request->config_doc, buffer, sizeof(request->config_doc));

	FIXS_ITH_LOG(LOG_LEVEL_INFO, "Edit Configuration Request successful prepared!");
	return ERR_NO_ERROR;
}

	int manager::test_l3_operation()
	{
		std::vector<fixs_ith_switchboardinfo> boardsList;
		fixs_ith::workingSet_t::instance()->get_cs_reader().get_my_switch_boards_info(boardsList);

	test_routerAndInterface(boardsList[0]);
	test_route_static_operations(boardsList[0]);

	return 0;
}


int manager::test_routerAndInterface(fixs_ith_switchboardinfo & switch_board)
{

//	// DELETE ROUTER PIPPO
//		const char routerDN[] = "routerId=PIPPO,AxeInfrastructureTransportMinfrastructureTransportMId=1";
//		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "'test_l3_operation():: Starting execution of 'Create router <%s>'",routerDN );
//	int result = delete_router(switch_board,routerDN);
//	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "'test_l3_operation():: Create router <%s> ' returns %d",routerDN,result );
//	// END
	int result =0;
	// CREATE A ROUTER
	const char routerDN[] = "routerId=PLUTO,AxeInfrastructureTransportMinfrastructureTransportMId=1";
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "'test_l3_operation():: Starting execution of 'Create router <%s>'",routerDN );
	result = create_router(switch_board,routerDN,true);
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "'test_l3_operation():: Create router <%s> ' returns %d",routerDN,result );
	// END

	// CREATE AN IPV4 INTERFACE
	const char ipv4InterfaceDN[] = "interfaceIPv4Id=if_pluto,routerId=PLUTO,AxeInfrastructureTransportMinfrastructureTransportMId=1";
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "'test_l3_operation():: Starting execution of 'Create IPV4 INTERFACE <%s>'",ipv4InterfaceDN);
	configdataList ipv4Interf_configdata;

	ipv4Interf_configdata.push_back(std::make_pair(netconf::interfaceIpv4::attribute::KEY,"if_pluto"));
	ipv4Interf_configdata.push_back(std::make_pair(netconf::interfaceIpv4::attribute::MAC,"auto"));
	ipv4Interf_configdata.push_back(std::make_pair(netconf::interfaceIpv4::attribute::ADMIN_STATE,"UNLOCKED"));
	ipv4Interf_configdata.push_back(std::make_pair(netconf::interfaceIpv4::attribute::ENCAPSULATION,
		"vlanId=Prova,bridgeId=15.0.0.0_0_T,AxeInfrastructureTransportMinfrastructureTransportMId=1"));
	ipv4Interf_configdata.push_back(std::make_pair(netconf::interfaceIpv4::attribute::ENCAPSULATION_VID,"10"));

	result = create_ipv4_interface(switch_board,ipv4InterfaceDN,ipv4Interf_configdata);
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "'test_l3_operation():: Create InterfaceIPv4 <%s> ' returns %d",ipv4InterfaceDN,result );
	//END

	// CREATE AN IPV4 Address
	const char ipv4addressDN[] = "addressIPv4Id=primary,interfaceIPv4Id=if_pluto,routerId=PLUTO,AxeInfrastructureTransportMinfrastructureTransportMId=1";
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "'test_l3_operation():: Starting execution of 'Create IPV4 ADDRESS  <%s> '",ipv4addressDN);
	configdataList ipv4Address_configdata;

	ipv4Address_configdata.push_back(std::make_pair(netconf::addressIpv4::attribute::KEY,"primary"));
	ipv4Address_configdata.push_back(std::make_pair(netconf::addressIpv4::attribute::ADDRESS,"10.33.24.130/28"));

	result = create_ipv4_address(switch_board,ipv4addressDN,ipv4Address_configdata);
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "'test_l3_operation():: Create Address IPV4 <%s> ' returns %d",ipv4addressDN,result );

	//END

	// Modify AN IPV4 Address

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "\n test_l3_operation():: Starting execution of 'Modify IPV4 ADDRESS  <%s> '",ipv4addressDN);
	ipv4Address_configdata.clear();

	ipv4Address_configdata.push_back(std::make_pair(netconf::addressIpv4::attribute::KEY,"primary"));
	ipv4Address_configdata.push_back(std::make_pair(netconf::addressIpv4::attribute::ADDRESS,"10.33.24.130/27"));

	result = modify_ipv4_address(switch_board,ipv4addressDN,ipv4Address_configdata);
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "'test_l3_operation():: Create Address IPV4 <%s> ' returns %d",ipv4addressDN,result );

	//END


	// MODIFY ADMIN STATE OF IPV4 INTERFACE
	//const char m_ipv4InterfaceDN[] = "interfaceIPv4Id=if_test_3,routerId=PIPPO,AxeInfrastructureTransportMinfrastructureTransportMId=1";
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "'test_l3_operation():: Starting execution of 'Modify IPV4 INTERFACE <%s>'",ipv4InterfaceDN);
	ipv4Interf_configdata.clear();

	ipv4Interf_configdata.push_back(std::make_pair(netconf::interfaceIpv4::attribute::KEY,"if_pluto"));
	//ipv4Interf_configdata.push_back(std::make_pair(netconf::interfaceIpv4::attribute::MAC,"auto"));
	ipv4Interf_configdata.push_back(std::make_pair(netconf::interfaceIpv4::attribute::ADMIN_STATE,"LOCKED"));
	//ipv4Interf_configdata.push_back(std::make_pair(netconf::interfaceIpv4::attribute::ENCAPSULATION,
	//		"vlanId=900,bridgeId=15.0.0.0_0_T,AxeInfrastructureTransportMinfrastructureTransportMId=1"));

	result = modify_ipv4_interface(switch_board,ipv4InterfaceDN,ipv4Interf_configdata);
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "'test_l3_operation():: Modify InterfaceIPv4 <%s> ' returns %d",ipv4InterfaceDN,result );

   //END

//	sleep(60);

//	 //DELETE IPV4 ADDRESS "Primary"
//	const char ipv4addressDN[] = "addressIPv4Id=primary,interfaceIPv4Id=if_test,routerId=PIPPO,AxeInfrastructureTransportMinfrastructureTransportMId=1";
//
//	const char ipv4InterfaceDN[] = "interfaceIPv4Id=if_test,routerId=PIPPO,AxeInfrastructureTransportMinfrastructureTransportMId=1";
//	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "'test_l3_operation():: Starting execution of 'Delete IPV4 Address <%s>'",ipv4addressDN);
//
//	result = delete_ipv4_address(switch_board,ipv4addressDN);
//	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "'test_l3_operation():: Delete InterfaceIPv4 <%s> ' returns %d",ipv4addressDN,result );
//
//	// END

//	//DELETE IPV4 INTERFACE  "if_test"
//
//	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "'test_l3_operation():: Starting execution of 'Delete IPV4 INTERFACE <%s>'",ipv4InterfaceDN);
//
//	result = delete_ipv4_interface(switch_board,ipv4InterfaceDN);
//	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "'test_l3_operation():: Delete InterfaceIPv4 <%s> ' returns %d",ipv4InterfaceDN,result );
//
//	//END

	 return 0;
	}



int manager::test_route_static_operations(fixs_ith_switchboardinfo &  switch_board)
{
	// CREATE DST
	const char dstDN[] = "dstId=PLUTO_DST,routeTableIPv4StaticId=1,routerId=PLUTO,AxeInfrastructureTransportMinfrastructureTransportMId=1";
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Starting execution of 'Create DST <%s>'",dstDN );


	configdataList dst_configdata;
	dst_configdata.push_back(std::make_pair(netconf::dst::attribute::KEY,"PLUTO_DST"));
	dst_configdata.push_back(std::make_pair(netconf::dst::attribute::DST,"10.33.24.128/28"));

	int result = create_dst(switch_board,dstDN,dst_configdata,true);

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Create DST <%s> returns %d",dstDN,result );
	// END

	// CREATE NEXTHOP
	const char nexthopDN[] = "nextHopId=PLUTO_NEXTHOP,dstId=PLUTO_DST,routeTableIPv4StaticId=1,routerId=PLUTO,AxeInfrastructureTransportMinfrastructureTransportMId=1";
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Starting execution of 'Create NEXTHOP <%s>'",nexthopDN);
	configdataList nexthop_data;

	nexthop_data.push_back(std::make_pair(netconf::nextHop::attribute::KEY,"PLUTO_NEXTHOP"));
	nexthop_data.push_back(std::make_pair(netconf::nextHop::attribute::ADDRESS,"10.33.23.70"));
	nexthop_data.push_back(std::make_pair(netconf::nextHop::attribute::ADMIN_DISTANCE,"2"));
	nexthop_data.push_back(std::make_pair(netconf::nextHop::attribute::BFD_ENABLE,"true"));

	result = create_next_hop(switch_board,nexthopDN,nexthop_data, true);

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Create NEXTHOP  <%s> returns %d",nexthopDN,result );
	//END

	// GET DST
	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Starting execution of 'GET DST <%s>'",dstDN );

	configdataList dst_data;
	dst_data.push_back(std::make_pair(netconf::dst::attribute::KEY,"PLUTO_DST"));

	result = get_dst(dst_data, switch_board, dstDN);

	FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Get DST <%s> returns %d",dstDN,result );

	if( result == 0)
	{
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "DUMP DST attributes (num of attribute == %d) :", dst_data.size());

		for ( configdataList::const_iterator it = dst_data.begin() ; it != dst_data.end(); it++){

			FIXS_ITH_LOG_IMPL(LOG_LEVEL_DEBUG, "\t %s: %s",it->first.c_str(), it->second.c_str());
		}
	}
	// END

	return 0;
	}


}

