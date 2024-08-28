#ifndef __INCLUDE_GUARD_CLASS_NETCONF_CONFIG_INSTANCE__
#define __INCLUDE_GUARD_CLASS_NETCONF_CONFIG_INSTANCE__ config_instance

/*
 *	@file config_instance.h
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

#include <vector>

#include "netconf/config_class.h"
#include "netconf/config_property.h"

namespace netconf {
	class config_instance {
		//=======//
		// CTORs //
		//=======//
	public:
		inline config_instance ()
		: _my_class(), _name(0), _id(0), _properties(), _children()
		{}

		inline explicit config_instance (const char * class_name)
		: _my_class(class_name), _name(0), _id(0), _properties(), _children()
		{}

		inline config_instance (const char * class_name, const char * name, const char * id)
		: _my_class(class_name), _name(name), _id(id), _properties(), _children()
		{}

		inline config_instance (
				const char * class_name,
				const char * const * class_attrs,
				const char * name,
				const char * id)
		: _my_class(class_name, class_attrs), _name(name), _id(id), _properties(), _children()
		{}

		config_instance (const config_instance & rhs, const char * id)
		: _my_class(rhs._my_class), _name(rhs._name), _id(id),
			_properties(rhs._properties), _children(rhs._children)
		{}

		config_instance (const config_instance & rhs)
		: _my_class(rhs._my_class), _name(rhs._name), _id(rhs._id),
			_properties(rhs._properties), _children(rhs._children)
		{}

		//======//
		// DTOR //
		//======//
	public:
		inline ~config_instance () {}

		//===========//
		// Functions //
		//===========//
	public:
		const config_class & my_class () const { return _my_class; }
		config_class & my_class () { return _my_class; }

		const char * name () const { return _name; }

		const char * id () const { return _id; }

		inline config_instance & add_child (const config_instance & child) {
			_children.push_back(child);
			return _children[_children.size() - 1];
		}

		inline void add_property (const config_property & prop) {
			_properties.push_back(prop);
		}

		inline void add_property (const char * name, const char * value = 0) {
			add_property(config_property(name, value));
		}

		inline void add_property (const std::vector<config_property> & properties) {
			for (size_t i = 0; i < properties.size(); add_property(properties[i++])) ;
		}

		inline void add_property_type (const char * name, const char * type_name, const char * value = 0) {
			config_property prop(name);
			prop.add_child(type_name, value);

			add_property(prop);
		}

		inline void clear_properties () { _properties.clear(); }

		int to_xml (char * buffer, size_t size) const;

	protected:
		void name (const char * new_value) { _name = new_value; }

		void id (const char * new_value) { _id = new_value; }

		inline int xml_instance_string (char * buffer, size_t size) const {
			if (name() && *name()) {
				const int chars_printed = snprintf(buffer, size, "<%s>%s</%s>", name(), id(), name());
				size && (buffer[size - 1] = 0);
				return chars_printed;
			}

			return 0;
		}

		//===========//
		// Operators //
		//===========//
	public:
		config_instance & operator= (const config_instance & rhs) {
			if (this != &rhs) {
				_my_class = rhs._my_class;
				_name = rhs._name;
				_id = rhs._id;
				_properties = rhs._properties;
				_children = rhs._children;
			}
			return *this;
		}

		//========//
		// Fields //
		//========//
	private:
		config_class _my_class;
		const char * _name;
		const char * _id;
		std::vector<config_property> _properties;
		std::vector<config_instance> _children;
	};
}

#endif /* __INCLUDE_GUARD_CLASS_NETCONF_CONFIG_INSTANCE__ */
