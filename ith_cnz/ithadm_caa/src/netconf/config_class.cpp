/*
 *	@file config_class.cpp
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

#include "netconf/config_class.h"

namespace netconf {
	const char * config_class::operation_attr () const {
		const char * op_name = 0;

		switch (_operation) {
		case operation::NO_OPERATION: op_name = operation::OPERATION_XML_ATTR_NO_OPERATION; break;
		case operation::MERGE: op_name = operation::OPERATION_XML_ATTR_MERGE; break;
		case operation::REPLACE: op_name = operation::OPERATION_XML_ATTR_REPLACE; break;
		case operation::CREATE: op_name = operation::OPERATION_XML_ATTR_CREATE; break;
		case operation::DELETE: op_name = operation::OPERATION_XML_ATTR_DELETE; break;
		default: op_name = operation::OPERATION_XML_ATTR_UNKNOWN;
		}

		return op_name;
	}

	int config_class::xml_node_header_string (char * buffer, size_t size) const {
		int chars_printed = snprintf(buffer, size, "<%s", name());

		if ((chars_printed < 0) || (static_cast<size_t>(chars_printed) >= size)) {
			buffer[size - 1] = 0;
			return chars_printed;
		}

		int chars_printed_total = chars_printed;
		size -= static_cast<size_t>(chars_printed);

		if (_static_attributes && *_static_attributes) {
			// There are some xml node attributes to be printed
			const char * const * attrs_it = _static_attributes;

			for (const char * pattr = *attrs_it++ ; pattr; pattr = *attrs_it++) {
				chars_printed = snprintf(buffer + chars_printed_total, size, " %s", pattr);

				if ((chars_printed < 0) || (static_cast<size_t>(chars_printed) >= size)) {
					buffer[size - 1] = 0;
					return chars_printed + (chars_printed >= 0 ? chars_printed_total : 0);
				}

				chars_printed_total += chars_printed;
				size -= static_cast<size_t>(chars_printed);
			}
		}

		chars_printed = snprintf(buffer + chars_printed_total, size,
				((_operation != operation::NO_OPERATION) ? " %s>" : ">"), operation_attr());
/* REMARK: ALTERNATIVE CODE
		chars_printed = ((_operation != operation::NO_OPERATION)
				? snprintf(buffer + chars_printed_total, size, " %s>", operation_attr())
				: snprintf(buffer + chars_printed_total, size, ">")
		);
*/

		if ((chars_printed < 0) || (static_cast<size_t>(chars_printed) >= size)) {
			buffer[size - 1] = 0;
		}

		return chars_printed + (chars_printed >= 0 ? chars_printed_total : 0);
	}
}
