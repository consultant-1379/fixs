/*
 *	@file config_dom_builder.cpp
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
#include "netconf/rdn_path.h"
#include "netconf/static_names_mapper.h"
#include "netconf/config_instance_builder.h"
#include "imm/imm.h"

#include "netconf/config_dom_builder.h"

namespace {
	const size_t CLASS_NAME_MAP_SIZE = 17;

	const char * const class_names_map [CLASS_NAME_MAP_SIZE][2] = {
		/*{IMM Name, SMX Name}*/
			{netconf::policies::CLASS_NAME, netconf::policies::CLASS_NAME}
		,	{"routerId", netconf::router::CLASS_NAME}
		,	{"interfaceIPv4Id", netconf::interfaceIpv4::CLASS_NAME}
		,	{"addressIPv4Id", netconf::addressIpv4::CLASS_NAME}
		,	{"dstId", netconf::dst::CLASS_NAME}
		,	{"nextHopId", netconf::nextHop::CLASS_NAME}
		,	{"bfdProfileId", netconf::bfdProfile::CLASS_NAME}
		,	{"bfdSessionIPv4Id", netconf::bfdSessionInfo::CLASS_NAME}
		,	{"vrrpv3SessionId", netconf::vrrpv3IPv4Session::CLASS_NAME}
		,	{"routeTableIPv4StaticId", netconf::routeTableStatic::CLASS_NAME}
		,	{"peerIPv4Id", netconf::peerIpv4::CLASS_NAME}
		, {"routeTableIPv4Id", netconf::routeTable::CLASS_NAME}
		, {netconf::policy::CLASS_NAME, netconf::policy::CLASS_NAME}
		, {netconf::rule::CLASS_NAME, netconf::rule::CLASS_NAME}
		, {netconf::ruleIf::attribute::KEY, netconf::ruleIf::CLASS_NAME}
		, {netconf::ruleThen::attribute::KEY, netconf::ruleThen::CLASS_NAME}
		, {netconf::protVirtIPv4Address::attribute::KEY, netconf::protVirtIPv4Address::CLASS_NAME}
	};

	netconf::static_names_mapper<CLASS_NAME_MAP_SIZE> class_names_mapper(class_names_map);
}

namespace netconf {
	int config_dom_builder::create_dom_tree_from_dn (
			config_instance * (& last_instance),
			config_document & conf_doc,
			char * dn) {
		char * policies_ptr = strcasestr(dn, netconf::policies::CLASS_NAME);
		if (!policies_ptr) policies_ptr = strcasestr(dn, netconf::policies::attribute::KEY);

		rdn_path rdnp(policies_ptr ? policies_ptr : dn);

		int dn_length = rdnp.split();
		if (dn_length < 0) {
			// ERROR: DN path malformed
			return -1;
		}

		config_instance child_instance;
		config_instance_builder builder;

		// Build and add the trasport or the ers configuration element
		builder.build(child_instance, (policies_ptr ? ers::CLASS_NAME : transport::CLASS_NAME ) , "1");
		config_instance * last_inst = &conf_doc.root().add_child(child_instance);

		if (dn_length) {
			for (int i = 0; i < dn_length; ++i) {
				const rdn_path_item & rdn_item = rdnp[i];

				const char * class_name = class_names_mapper.get_value(rdn_item.key());

				if (!class_name) {
					// ERROR: Class name not known on DN
					return -2;
				}

				if (builder.build(child_instance, class_name, rdn_item.value()) < 0) {
					// ERROR: Configuration class element not known on dn parameter
					return -3;
				}

				last_inst = &last_inst->add_child(child_instance);
			}
		}

		last_instance = last_inst;

		return 0;
	}

	int config_dom_builder::build_properties (
			config_instance * instance,
			const std::vector<config_property> & properties) {
		if (strcmp(nextHop::CLASS_NAME, instance->my_class().name())) {
			instance->add_property(properties);
		} else {
			build_properties_for_next_hop(instance, properties);
		}

		return 0;
	}

	int config_dom_builder::add_children_node (
			config_instance * instance,
			const char * node_name,
			const configdataList & node_attributes) {
		config_instance node(node_name);

		for (configdataList::const_iterator cit = node_attributes.begin(); cit != node_attributes.end(); ++cit)
			node.add_property(cit->first.c_str(), cit->second.c_str());

		instance->add_child(node);

		return 0;
	}

	int config_dom_builder::add_children_node_list (
			config_instance * instance,
			const char * list_node_name,
			const std::vector<configdataList> & nodes_attributes) {

		std::vector<configdataList>::const_iterator cit = nodes_attributes.begin();
		while (cit != nodes_attributes.end())
			add_children_node(instance, list_node_name, *cit++);

		return 0;
	}

	int config_dom_builder::build_properties_for_next_hop (
			config_instance * instance,
			const std::vector<config_property> & properties) {
		for (size_t i = 0; i < properties.size(); ++i) {
			const config_property & prop = properties[i];

			if (imm::nextHop_attribute::DISCARD == prop.name()) {
				instance->add_property_type(nextHop::attribute::NEXT_HOP, nextHop::attribute::DISCARD,
						prop.value());
			} else if (imm::nextHop_attribute::ADDRESS == prop.name()) {
				instance->add_property_type(nextHop::attribute::NEXT_HOP, nextHop::attribute::REFERENCE,
						prop.value());
			} else {
				instance->add_property(prop);
			}
		}

		return 0;
	}
}
