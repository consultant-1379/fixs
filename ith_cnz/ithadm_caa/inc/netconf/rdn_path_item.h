#ifndef __INCLUDE_GUARD_CLASS_NETCONF_RDN_PATH_ITEM__
#define __INCLUDE_GUARD_CLASS_NETCONF_RDN_PATH_ITEM__ rdn_path_item

/*
 *	@file rdn_path_item.h
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

namespace netconf {
	class rdn_path_item {
		//=======//
		// CTORs //
		//=======//
	public:
		inline rdn_path_item (const char * key, const char * value)
		: _key(key), _value(value) {}

		inline rdn_path_item (const rdn_path_item & rhs)
		: _key(rhs._key), _value(rhs._value) {}

		//======//
		// DTOR //
		//======//
	public:
		inline ~rdn_path_item () {}

		//===========//
		// Functions //
		//===========//
	public:
		inline const char * key () const { return _key; }

		inline const char * value () const { return _value; }

		inline void set (const char * key, const char * value) { _key = key; _value = value; }

		//===========//
		// Operators //
		//===========//
	public:
		inline rdn_path_item & operator= (const rdn_path_item & rhs) {
			if (this != &rhs) { _key = rhs._key; _value = rhs._value; }
			return *this;
		}

		//========//
		// Fields //
		//========//
	private:
		const char * _key;
		const char * _value;
	};
}

#endif /* __INCLUDE_GUARD_CLASS_NETCONF_RDN_PATH_ITEM__ */
