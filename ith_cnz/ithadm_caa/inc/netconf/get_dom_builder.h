#ifndef __INCLUDE_GUARD_CLASS_NETCONF_GET_DOM_BUILDER__
#define __INCLUDE_GUARD_CLASS_NETCONF_GET_DOM_BUILDER__ get_dom_builder

/*
 *	@file get_dom_builder.h
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
	class get_dom_builder : public config_dom_builder {
		//=======//
		// CTORs //
		//=======//
	public:
		inline get_dom_builder () {}

	private:
		get_dom_builder (const get_dom_builder & rhs);

		//======//
		// DTOR //
		//======//
	public:
		virtual inline ~get_dom_builder () {}

		//===========//
		// Functions //
		//===========//
	public:
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
		get_dom_builder & operator= (const get_dom_builder & rhs);

		//========//
		// Fields //
		//========//
	private:
	};
}

#endif /* __INCLUDE_GUARD_CLASS_NETCONF_GET_DOM_BUILDER__ */
