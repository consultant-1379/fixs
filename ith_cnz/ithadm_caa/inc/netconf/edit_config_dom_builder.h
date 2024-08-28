#ifndef __INCLUDE_GUARD_CLASS_NETCONF_EDIT_CONFIG_DOM_BUILDER__
#define __INCLUDE_GUARD_CLASS_NETCONF_EDIT_CONFIG_DOM_BUILDER__ edit_config_dom_builder

/*
 *	@file edit_config_dom_builder.h
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

#include "netconf/netconf.h"
#include "netconf/config_dom_builder.h"
#include "netconf/config_property.h"

namespace netconf {
	class edit_config_dom_builder : public config_dom_builder {
		//=======//
		// CTORs //
		//=======//
	public:
		inline explicit edit_config_dom_builder (operation::OperationType op = operation::MERGE)
		: config_dom_builder(), _op(op), _children_node_name(0), _children_attributes(0), _last_node_name(0), _last_attributes(0)
		{}

		inline edit_config_dom_builder (
				operation::OperationType op,
				const char * children_node_name,
				const std::vector<configdataList> * children_attributes)
		: config_dom_builder(), _op(op), _children_node_name(children_node_name), _children_attributes(children_attributes),
			_last_node_name(0), _last_attributes(0)
		{}

		inline edit_config_dom_builder (
				operation::OperationType op,
				const char * children_node_name,
				const std::vector<configdataList> * children_attributes,
				const char * last_node_name,
				const configdataList * last_attributes)
		: config_dom_builder(), _op(op), _children_node_name(children_node_name), _children_attributes(children_attributes),
			_last_node_name(last_node_name), _last_attributes(last_attributes)
		{}

	private:
		edit_config_dom_builder (const edit_config_dom_builder & rhs);

		//======//
		// DTOR //
		//======//
	public:
		virtual inline ~edit_config_dom_builder () {}

		//===========//
		// Functions //
		//===========//
	public:
		operation::OperationType op () const { return _op; }
		void op (operation::OperationType new_val) { _op = new_val; }

		using config_dom_builder::build_dom;

	protected:
		virtual int build_dom (
				char * buffer,
				size_t size,
				const char * rdn,
				const std::vector<config_property> * properties);

		//===========//
		// Operators //
		//===========//
	private:
		edit_config_dom_builder & operator= (const edit_config_dom_builder & rhs);

		//========//
		// Fields //
		//========//
	private:
		operation::OperationType _op;
		const char * _children_node_name;
		const std::vector<configdataList> * _children_attributes;
		const char * _last_node_name;
		const configdataList * _last_attributes;
	};
}

#endif /* __INCLUDE_GUARD_CLASS_NETCONF_EDIT_CONFIG_DOM_BUILDER__ */
