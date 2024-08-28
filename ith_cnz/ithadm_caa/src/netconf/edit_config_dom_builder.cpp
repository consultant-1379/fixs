/*
 *	@file edit_config_dom_builder.cpp
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

#include "netconf/config_document.h"

#include "netconf/edit_config_dom_builder.h"

namespace netconf {
	int edit_config_dom_builder::build_dom (
			char * buffer,
			size_t size,
			const char * rdn,
			const std::vector<config_property> * properties) {
		config_document conf_doc;
		config_instance * last_instance = 0;

		char rdn_buf [16 * 1024] = {0};

		memccpy(rdn_buf, rdn, 0, sizeof(rdn_buf));

		if (const int call_result = create_dom_tree_from_dn(last_instance, conf_doc, rdn_buf)) {
			// ERROR: On creating the dom tree: probably the dn is a bad dn
			return call_result;
		}

		last_instance->my_class().operation(_op);

		properties && build_properties(last_instance, *properties);

		if (_children_node_name && *_children_node_name && _children_attributes && (_children_attributes->size() > 0)) {
			add_children_node_list(last_instance, _children_node_name, *_children_attributes);
		}

		if (_last_node_name && *_last_node_name && _last_attributes && (_last_attributes->size() > 0)) {
			add_children_node(last_instance, _last_node_name, *_last_attributes);
		}

		return conf_doc.to_xml(buffer, size);
	}
}
