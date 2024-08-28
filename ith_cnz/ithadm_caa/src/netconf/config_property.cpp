/*
 *	@file config_property.cpp
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

#include <stdio.h>

#include "netconf/netconf.h"

#include "netconf/config_property.h"

namespace netconf {
	int config_property::to_xml (char * buffer, size_t size) const {
		if (!buffer) return -1;
		if (!size) return 0;

		int chars_printed = 0;
		int chars_printed_total = 0;

		if (_children.empty()) {
			chars_printed = ((value() && *value())
				? snprintf(buffer, size, "<%s>%s</%s>", name(), value(), name())
				: snprintf(buffer, size, "<%s %s></%s>", name(), operation::OPERATION_XML_ATTR_DELETE, name())
			);

			if ((chars_printed < 0) || (static_cast<size_t>(chars_printed) >= size)) {
				buffer[size - 1] = 0;
			}
		} else {
			chars_printed = snprintf(buffer, size, "<%s>", name());
			if ((chars_printed < 0) || (static_cast<size_t>(chars_printed) >= size)) {
				buffer[size - 1] = 0;
				return chars_printed;
			}

			chars_printed_total = chars_printed;
			size -= static_cast<size_t>(chars_printed);

			// Now property sub values will be printed
			for (std::vector<config_property>::const_iterator it = _children.begin();
					 it != _children.end();
					 ++it) {
				chars_printed = it->to_xml(buffer + chars_printed_total, size);
				if ((chars_printed < 0) || (static_cast<size_t>(chars_printed) >= size)) {
					buffer[size - 1] = 0;
					return chars_printed + (chars_printed >= 0 ? chars_printed_total : 0);
				}

				chars_printed_total += chars_printed;
				size -= static_cast<size_t>(chars_printed);
			}

			chars_printed = snprintf(buffer + chars_printed_total, size, "</%s>", name());
			if ((chars_printed < 0) || (static_cast<size_t>(chars_printed) >= size)) {
				buffer[size - 1] = 0;
				(chars_printed < 0) && (chars_printed_total = 0);
			}
		}

		return chars_printed_total + chars_printed;
	}
}
