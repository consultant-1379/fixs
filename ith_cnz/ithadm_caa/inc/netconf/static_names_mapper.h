#ifndef __INCLUDE_GUARD_CLASS_NETCONF_STATIC_NAMES_MAPPER__
#define __INCLUDE_GUARD_CLASS_NETCONF_STATIC_NAMES_MAPPER__ static_names_mapper

/*
 *	@file static_names_mapper.h
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

#include <string.h>

namespace netconf {
	template < size_t _T_map_size >
	class static_names_mapper {
		//=======//
		// CTORs //
		//=======//
	public:
		inline explicit static_names_mapper (const char * const (& static_map) [_T_map_size] [2])
		: _static_map(static_map) {}

	private:
		static_names_mapper (const static_names_mapper & rhs);

		//======//
		// DTOR //
		//======//
	public:
		inline ~static_names_mapper () {}

		//===========//
		// Functions //
		//===========//
	public:
		inline const char * get_value (const char * key) const { return get(0, key); }

		inline const char * get_key (const char * value) const { return get(1, value); }

	private:
		const char * get (unsigned key_index, const char * key_name) const {
			if (key_index > 1) return 0;

			unsigned value_index = key_index ^ 0x01;
			unsigned found_index = 0;

			while ((found_index < _T_map_size) &&
							strcmp(_static_map[found_index][key_index], key_name))
				found_index++;

			return ((found_index < _T_map_size) ? _static_map[found_index][value_index] : 0);
		}

		//===========//
		// Operators //
		//===========//
	private:
		static_names_mapper & operator= (const static_names_mapper & rhs);

		//========//
		// Fields //
		//========//
	private:
		const char * const (& _static_map) [_T_map_size] [2];
	};
}

#endif /* __INCLUDE_GUARD_CLASS_NETCONF_STATIC_NAMES_MAPPER__ */
