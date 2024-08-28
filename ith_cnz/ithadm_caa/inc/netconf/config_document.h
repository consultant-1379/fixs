#ifndef __INCLUDE_GUARD_CLASS_NETCONF_CONFIG_DOCUMENT__
#define __INCLUDE_GUARD_CLASS_NETCONF_CONFIG_DOCUMENT__ config_document

/*
 *	@brief
 *	@file config_document.h
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

#include "netconf/config_instance.h"
#include "netconf/config_instance_builder.h"

namespace netconf {
	class config_document {
		//=======//
		// CTORs //
		//=======//
	public:
		inline config_document () : _root() {
			config_instance root_instance_init;
			config_instance_builder builder;

			// Build the ManagedElement root configuration element with default id == "1"
			builder.build(root_instance_init, managedElement::CLASS_NAME, "1");

			_root = root_instance_init;
		}

		inline explicit config_document (const config_instance & root_element)
		: _root(root_element) {}

	private:
		config_document (const config_document & rhs);

		//======//
		// DTOR //
		//======//
	public:
		inline ~config_document () {}

		//===========//
		// Functions //
		//===========//
	public:
		inline config_instance & root () { return _root; }

		inline int to_xml (char * buffer, size_t size) const {
			return _root.to_xml(buffer, size);
		}

		//===========//
		// Operators //
		//===========//
	private:
		config_document & operator= (const config_document & rhs);

		//========//
		// Fields //
		//========//
	private:
		config_instance _root;
	};
}

#endif /* __INCLUDE_GUARD_CLASS_NETCONF_CONFIG_DOCUMENT__ */
