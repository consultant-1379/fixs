#ifndef __INCLUDE_GUARD_CLASS_NETCONF_CONFIG_PROPERTY__
#define __INCLUDE_GUARD_CLASS_NETCONF_CONFIG_PROPERTY__ config_property

/*
 *	@file config_property.h
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

#include <string>
#include <vector>

namespace netconf {
	class config_property {
		//=======//
		// CTORs //
		//=======//
	public:
		inline explicit config_property (const char * name, const char * value = 0)
		: _name(name ? name : ""), _value(value ? value : ""), _children()
		{}

		inline explicit config_property (const std::string & name, const std::string & value = std::string())
		: _name(name), _value(value), _children()
		{}

		inline explicit config_property (
				const char * name,
				const char * type_name,
				const char * type_value)
		: _name(name ? name : ""), _value(), _children()
		{ _children.push_back(config_property(type_name, type_value)); }

		inline explicit config_property (
				const std::string & name,
				const std::string & type_name,
				const std::string & type_value)
		: _name(name), _value(), _children()
		{ _children.push_back(config_property(type_name, type_value)); }

		inline config_property (const config_property & rhs)
		: _name(rhs._name), _value(rhs._value), _children(rhs._children)
		{}

		//======//
		// DTOR //
		//======//
	public:
		inline ~config_property () {}

		//===========//
		// Functions //
		//===========//
	public:
		inline const char * name () const { return _name.c_str(); }

		inline const char * value () const { return _value.c_str(); }
		inline void value (const char * new_value) { _value = (new_value ? new_value : ""); }
		inline void value (const std::string & new_value) { _value = new_value; }

		inline void add_child (const config_property & child) {
			_children.push_back(child);
		}

		inline void add_child (const char * name, const char * value = 0) {
			add_child(config_property(name, value));
		}

		inline void add_child (const std::string & name, const std::string & value = std::string()) {
			add_child(config_property(name, value));
		}

		inline void clear_children () { _children.clear(); }

		int to_xml (char * buffer, size_t size) const;

		//===========//
		// Operators //
		//===========//
	public:
		inline config_property & operator= (const config_property & rhs) {
			if (this != &rhs) { _name = rhs._name; _value = rhs._value; _children = rhs._children; }
			return *this;
		}

		//========//
		// Fields //
		//========//
	private:
		std::string _name;
		std::string _value;
		std::vector<config_property> _children;
	};
}

#endif /* __INCLUDE_GUARD_CLASS_NETCONF_CONFIG_PROPERTY__ */
