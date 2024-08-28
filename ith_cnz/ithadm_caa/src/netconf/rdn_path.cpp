/*
 *	@file rdn_path.cpp
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

#include <ctype.h>

#include "netconf/rdn_path.h"

namespace netconf {
	const rdn_path_item & rdn_path::_dummy_rdn_item = rdn_path_item(0, 0);

	int rdn_path::split () {
		if (!_splitted) {
			_items.clear();

			if (_rdn) {
				char * key = 0;
				char * value = 0;
				char * next_item = _rdn;

				int parse_result = 0;
				int items_count = 0;

				while (*next_item && ((parse_result = parse_rdn_item(next_item, key, value, next_item)) == 0)) {
					items_count++;
					*(value - 1) = 0;
					*next_item && (*(next_item - 1) = 0);
					_items.push_back(rdn_path_item(key, value));
				}

				if ((parse_result < -1) || ((parse_result == -1) && items_count)) {
					// ERROR: On parsing: RDN path not well formed
					_items.clear();
					return -1;
				}
			}

			_splitted = true;
		}

		return static_cast<int>(_items.size());
	}

	int rdn_path::parse_rdn_item (char * rdn, char * (& key), char * (& value), char * (& next_item)) {
		char c = 0;

		// Trimming spaces
		while ((c = *rdn) && isblank(c)) ++rdn;

		if (!c) return -1; // Empty item

		if ((c == _items_separator) || (c == _key_value_separator)) return -2; // Malformed RDN item

		char * key_return = rdn++;

		// Parse the key name
		while ((c = *rdn++) && (c != _key_value_separator)) ;

		if (c != _key_value_separator) return -2; // Malformed RDN item

		char * value_return = rdn;

		// Parse the item value
		while ((c = *rdn++) && (c != _items_separator)) ;

		if (((rdn - 1) == value_return) || (c && !*rdn)) return -2; // Malformed RDN item

		key = key_return;
		value = value_return;
		next_item = rdn - (c ? 0 : 1);

		return ((c && !*next_item) ? -2 : 0);
	}
}
