#ifndef __INCLUDE_GUARD_CLASS_NETCONF_CONFIG_CLASS__
#define __INCLUDE_GUARD_CLASS_NETCONF_CONFIG_CLASS__ config_class

/*
 *	@file config_class.h
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

#include <stddef.h>
#include <stdio.h>

#include "netconf/netconf.h"

namespace netconf {
	class config_class {
		//=========//
		// Friends //
		//=========//
		friend class config_instance;

		//=======//
		// CTORs //
		//=======//
	public:
		inline config_class ()
		: _name("NO_NAME"), _static_attributes(0), _operation(operation::NO_OPERATION)
		{}

		inline explicit config_class (const char * name, const char * const * static_attributes = 0)
		: _name(name), _static_attributes(static_attributes), _operation(operation::NO_OPERATION)
		{}

		inline explicit config_class (const char * name, operation::OperationType op)
		: _name(name), _static_attributes(0), _operation(op)
		{}

		inline explicit config_class (
				const char * name,
				const char * const * static_attributes,
				operation::OperationType op)
		: _name(name), _static_attributes(static_attributes), _operation(op)
		{}

		inline config_class (const config_class & rhs)
		: _name(rhs._name), _static_attributes(rhs._static_attributes), _operation(rhs._operation)
		{}

		//======//
		// DTOR //
		//======//
	public:
		inline ~config_class () {}

		//===========//
		// Functions //
		//===========//
	public:
		inline const char * name () const { return _name; }

		inline const char * const * static_attributes () const { return _static_attributes; }

		inline operation::OperationType operation () const { return _operation; }
		inline void operation (operation::OperationType new_value) { _operation = new_value; }

		const char * operation_attr () const;

	protected:
		int xml_node_header_string (char * buffer, size_t size) const;

		inline int xml_node_footer_string (char * buffer, size_t size) const {
			const int chars_printed = snprintf(buffer, size, "</%s>", name());
			size && (buffer[size - 1] = 0);
			return chars_printed;
		}

		//===========//
		// Operators //
		//===========//
	public:
		inline config_class & operator= (const config_class & rhs) {
			if (this != &rhs) {
				_name = rhs._name;
				_static_attributes = rhs._static_attributes;
				_operation = rhs._operation;
			}
			return *this;
		}

		//========//
		// Fields //
		//========//
	private:
		const char * _name;
		const char * const * _static_attributes;
		operation::OperationType _operation;
	};
}

#endif /* __INCLUDE_GUARD_CLASS_NETCONF_CONFIG_CLASS__ */
