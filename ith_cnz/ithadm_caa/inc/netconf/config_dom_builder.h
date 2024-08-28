#ifndef __INCLUDE_GUARD_CLASS_NETCONF_CONFIG_DOM_BUILDER__
#define __INCLUDE_GUARD_CLASS_NETCONF_CONFIG_DOM_BUILDER__ config_dom_builder

/*
 *	@file config_dom_builder.h
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

#include <vector>

#include "netconf/config_property.h"
#include "netconf/config_instance.h"
#include "netconf/config_document.h"

namespace netconf {
	class config_dom_builder {
		//=======//
		// CTORs //
		//=======//
	public:
		inline config_dom_builder () {}

	private:
		config_dom_builder (const config_dom_builder & rhs);

		//======//
		// DTOR //
		//======//
	public:
		virtual inline ~config_dom_builder () {}

		//===========//
		// Functions //
		//===========//
	public:
		inline int build_dom (
				char * buffer,
				size_t size,
				const char * rdn) {
			return build_dom(buffer, size, rdn, reinterpret_cast<const std::vector<config_property> *>(0));
		}

		inline int build_dom (
				char * buffer,
				size_t size,
				const char * rdn,
				const std::vector<config_property> & properties) {
			return build_dom(buffer, size, rdn, &properties);
		}

	protected:
		virtual int build_dom (
				char * buffer,
				size_t size,
				const char * rdn,
				const std::vector<config_property> * properties) = 0;

		int create_dom_tree_from_dn (
				config_instance * (& last_instance),
				config_document & conf_doc,
				char * dn);

		int build_properties (config_instance * instance, const std::vector<config_property> & properties);

		int add_children_node (
				config_instance * instance,
				const char * node_name,
				const configdataList & node_attributes);

		int add_children_node_list (
				config_instance * instance,
				const char * list_node_name,
				const std::vector<configdataList> & nodes_attributes);

	private:
		int build_properties_for_next_hop (
				config_instance * instance,
				const std::vector<config_property> & properties);

		//===========//
		// Operators //
		//===========//
	private:
		config_dom_builder & operator= (const config_dom_builder & rhs);

		//========//
		// Fields //
		//========//
	protected:
	};
}

#endif /* __INCLUDE_GUARD_CLASS_NETCONF_CONFIG_DOM_BUILDER__ */
