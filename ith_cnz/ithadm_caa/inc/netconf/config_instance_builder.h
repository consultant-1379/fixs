#ifndef __INCLUDE_GUARD_CLASS_NETCONF_CONFIG_INSTANCE_BUILDER__
#define __INCLUDE_GUARD_CLASS_NETCONF_CONFIG_INSTANCE_BUILDER__ config_instance_builder

/*
 *	@file config_instance_builder.h
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

#include "netconf/config_instance.h"

namespace netconf {
	class config_instance_builder {
		//=======//
		// CTORs //
		//=======//
	public:
		inline config_instance_builder () {}

	private:
		config_instance_builder (const config_instance_builder & rhs);

		//======//
		// DTOR //
		//======//
	public:
		inline ~config_instance_builder () {}

		//===========//
		// Functions //
		//===========//
	public:
		int build (config_instance & conf_instance, const char * class_name, const char * instance_id);

		//===========//
		// Operators //
		//===========//
	public:
		config_instance_builder & operator= (const config_instance_builder & rhs);

		//========//
		// Fields //
		//========//
	private:
	};
}

#endif /* __INCLUDE_GUARD_CLASS_NETCONF_CONFIG_INSTANCE__ */
