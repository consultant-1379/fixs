#ifndef __INCLUDE_GUARD_CLASS_NETCONF_RDN_PATH__
#define __INCLUDE_GUARD_CLASS_NETCONF_RDN_PATH__ rdn_path

/*
 *	@file rdn_path.h
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

#include <vector>

#include "netconf/rdn_path_item.h"

namespace netconf {
	class rdn_path {
		//=======//
		// CTORs //
		//=======//
	public:
		inline explicit rdn_path (char * rdn, char items_separator = ',')
		: _rdn(rdn), _splitted(false), _items_separator(items_separator),
			_key_value_separator('='), _items()
		{}

		inline explicit rdn_path (char * rdn, char items_separator, char key_value_separator)
		: _rdn(rdn), _splitted(false), _items_separator(items_separator),
			_key_value_separator(key_value_separator), _items()
		{}

	private:
		rdn_path (const rdn_path & rhs);

		//======//
		// DTOR //
		//======//
	public:
		inline ~rdn_path () {}

		//===========//
		// Functions //
		//===========//
	public:
		size_t items_count () { _splitted || split(); return _items.size(); }

		int split ();

		inline const rdn_path_item & rdn_item_at (unsigned index) const {
			return ((index < _items.size()) ? _items[index] : _dummy_rdn_item);
		}

	private:
		int parse_rdn_item (char * rdn, char * (& key), char * (& value), char * (& next_item));

		//===========//
		// Operators //
		//===========//
	public:
		inline const rdn_path_item & operator[] (unsigned index) const { return rdn_item_at(index); }

	private:
		rdn_path & operator= (const rdn_path & rhs);

		//========//
		// Fields //
		//========//
	private:
		char * _rdn;
		bool _splitted;
		char _items_separator;
		char _key_value_separator;
		std::vector<rdn_path_item> _items;

		static const rdn_path_item & _dummy_rdn_item;
	};
}

#endif /* __INCLUDE_GUARD_CLASS_NETCONF_RDN_PATH__ */
