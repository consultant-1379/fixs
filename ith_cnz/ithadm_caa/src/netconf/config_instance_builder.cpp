/*
 *	@file config_instance_builder.cpp
 *	@brief
 *	@author xnicmut (Nicola Muto)
 *	@date 2016-05-04
 *
 *	COPYRIGHT Ericsson AB, 2016
 *	All rights reserved.
 *
 *	The information in this document is the property of Ericsson.
 *	Except as specifically authorized in writing by Ericsson, the receiver of
 *	this document shall keep the information contained herein confidential and
 *	shall protect the same in whole or in part from disclosure and dissemination
 *	to third parties. Disclosure and disseminations to the receivers employees
 *	shall only be made on a strict need to know basis.
 *
 *	REVISION INFO
 *	+==========+============+==============+=====================================+
 *	| REV      | DATE       | AUTHOR       | DESCRIPTION                         |
 *	+==========+============+==============+=====================================+
 *	| 2016-001 | 2016-05-04 | xnicmut      | Creation and first revision.        |
 *	+==========+============+==============+=====================================+
 */

#include <string.h>

#include "netconf/netconf.h"

#include "netconf/config_instance_builder.h"

#if !defined (ERI_ARRAY_SIZE)
#	define ERI_ARRAY_SIZE(array) (sizeof(array)/sizeof(*(array)))
#endif /* !defined (ERI_ARRAY_SIZE) */

namespace {
	const int INTERNAL_NETCONF_ARRAY_SIZE = 20;

	const char * netconf_instance_class_names [INTERNAL_NETCONF_ARRAY_SIZE] = {
		netconf::managedElement::CLASS_NAME,
		netconf::ers::CLASS_NAME,
		netconf::transport::CLASS_NAME,
		netconf::router::CLASS_NAME,
		netconf::interfaceIpv4::CLASS_NAME,
		netconf::addressIpv4::CLASS_NAME,
		netconf::bfdSessionInfo::CLASS_NAME,
		netconf::bfdProfile::CLASS_NAME,
		netconf::routeTableStatic::CLASS_NAME,
		netconf::peerIpv4::CLASS_NAME,
		netconf::dst::CLASS_NAME,
		netconf::nextHop::CLASS_NAME,
		netconf::vrrpv3IPv4Session::CLASS_NAME,
		netconf::protVirtIPv4Address::CLASS_NAME,
		netconf::routeTable::CLASS_NAME,
		netconf::policies::CLASS_NAME,
		netconf::policy::CLASS_NAME,
		netconf::rule::CLASS_NAME,
		netconf::ruleIf::CLASS_NAME,
		netconf::ruleThen::CLASS_NAME
	};

	const char * netconf_instance_names [INTERNAL_NETCONF_ARRAY_SIZE] = {
		netconf::managedElement::attribute::KEY,
		netconf::ers::attribute::KEY,
		netconf::transport::attribute::KEY,
		netconf::router::attribute::KEY,
		netconf::interfaceIpv4::attribute::KEY,
		netconf::addressIpv4::attribute::KEY,
		netconf::bfdSessionInfo::attribute::KEY,
		netconf::bfdProfile::attribute::KEY,
		netconf::routeTableStatic::attribute::KEY,
		netconf::peerIpv4::attribute::KEY,
		netconf::dst::attribute::KEY,
		netconf::nextHop::attribute::KEY,
		netconf::vrrpv3IPv4Session::attribute::KEY,
		netconf::protVirtIPv4Address::attribute::KEY,
		netconf::routeTable::attribute::KEY,
		netconf::policies::attribute::KEY,
		netconf::policy::attribute::KEY,
		netconf::rule::attribute::KEY,
		netconf::ruleIf::attribute::KEY,
		netconf::ruleThen::attribute::KEY
	};

	const char * const MANAGED_ELEMENT_CLASS_ATTRIBUTES [] = {
		"xmlns=\"urn:com:ericsson:ecim:comtop\"",
		0
	};

	const char * const ERS_CLASS_ATTRIBUTES [] = {
		"xmlns=\"urn:com:ericsson:ecim:ERS\"",
		0
	};

	const char * const ROUTER_ELEMENT_CLASS_ATTRIBUTES [] = {
		"xmlns=\"urn:com:ericsson:ecim:ECIM_T_L3_Router\"",
		0
	};

	const char * const INTERFACE_IPV4_ELEMENT_CLASS_ATTRIBUTES [] = {
		"xmlns=\"urn:com:ericsson:ecim:ECIM_T_L3_IPv4Interface\"",
		0
	};

	const char * const BFD_SESSION_INFO_ELEMENT_CLASS_ATTRIBUTES [] = {
			"xmlns=\"urn:com:ericsson:ecim:SMX_T_BFD_IPv4BfdSession\"",
		0
	};

	const char * const VRRPV3_IPV4_SESSION_ELEMENT_CLASS_ATTRIBUTES [] = {
		"xmlns=\"urn:com:ericsson:ecim:ECIM_T_Ipv4IfVrrpv3Session\"",
		0
	};

	const char * const * netconf_instance_class_attributes [INTERNAL_NETCONF_ARRAY_SIZE] = {
		MANAGED_ELEMENT_CLASS_ATTRIBUTES,
		ERS_CLASS_ATTRIBUTES,
		0,
		ROUTER_ELEMENT_CLASS_ATTRIBUTES,
		INTERFACE_IPV4_ELEMENT_CLASS_ATTRIBUTES,
		0,
		BFD_SESSION_INFO_ELEMENT_CLASS_ATTRIBUTES,
		0,
		0,
		0,
		0,
		0,
		VRRPV3_IPV4_SESSION_ELEMENT_CLASS_ATTRIBUTES,
		0,
		0,
		0,
		0,
		0,
		0,
		0
	};
}

namespace netconf {
	int config_instance_builder::build (
			config_instance & conf_instance,
			const char * class_name,
			const char * instance_id) {
		if (!class_name || !*class_name || !instance_id) {
			// ERROR: Class name not provided
			return -1;
		}

		const int class_names_size = ERI_ARRAY_SIZE(netconf_instance_class_names);
		int i = 0;

		while ((i < class_names_size) && strcmp(netconf_instance_class_names[i], class_name)) ++i;

		if (i >= class_names_size) return -2; // ERROR: Class name not known

		conf_instance = config_instance(class_name, netconf_instance_class_attributes[i], netconf_instance_names[i], instance_id);

		return 0;
	}
}
