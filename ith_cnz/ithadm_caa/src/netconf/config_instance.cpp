/*
 *	@file config_instance.cpp
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

#include "netconf/config_instance.h"

namespace netconf {
	int config_instance::to_xml (char * buffer, size_t size) const {
		if (!buffer) return -1;
		if (!size) return 0;

		// Start printing the class xml header part
		int chars_printed = my_class().xml_node_header_string(buffer, size);
		if ((chars_printed < 0) || (static_cast<size_t>(chars_printed) >= size)) {
			buffer[size - 1] = 0;
			return chars_printed;
		}

		int chars_printed_total = chars_printed;
		size -= static_cast<size_t>(chars_printed);

		// Now the instance xml node string
		chars_printed = xml_instance_string(buffer + chars_printed_total, size);
		if ((chars_printed < 0) || (static_cast<size_t>(chars_printed) >= size)) {
			buffer[size - 1] = 0;
			return chars_printed + (chars_printed >= 0 ? chars_printed_total : 0);
		}

		chars_printed_total += chars_printed;
		size -= static_cast<size_t>(chars_printed);

		// Now any property, if present, will be printed into the buffer
		for (std::vector<config_property>::const_iterator it = _properties.begin();
				 it != _properties.end(); ++it) {
			chars_printed = it->to_xml(buffer + chars_printed_total, size);
			if ((chars_printed < 0) || (static_cast<size_t>(chars_printed) >= size)) {
				buffer[size - 1] = 0;
				return chars_printed + (chars_printed >= 0 ? chars_printed_total : 0);
			}

			chars_printed_total += chars_printed;
			size -= static_cast<size_t>(chars_printed);
		}

		// Now all child elements will be printed
		for (std::vector<config_instance>::const_iterator it = _children.begin();
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

		// Finally, the class xml footer will be printed
		chars_printed = my_class().xml_node_footer_string(buffer + chars_printed_total, size);
		if ((chars_printed < 0) || (static_cast<size_t>(chars_printed) >= size)) {
			buffer[size - 1] = 0;
		}

		return chars_printed + (chars_printed >= 0 ? chars_printed_total : 0);
	}
}
