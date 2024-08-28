/*
 *	@file fixs_ith_brf_common.cpp
 *	@brief
 *	@author xnicmut (Nicola Muto)
 *	@date 2016-09-07
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
 *	| 2016-001 | 2016-09-07 | xnicmut      | Creation and first revision.        |
 *	+==========+============+==============+=====================================+
 */

#include <fstream>

#include "fixs_ith_programmacros.h"
#include "fixs_ith_logger.h"

#include "fixs_ith_brf_common.h"

namespace {
	const char * const g_config_pso_storage_path_clear = "/usr/share/pso/storage-paths/clear";
	const char * const g_config_pso_storage_application_folder_name = "fixs-ith";

	std::string g_cba_pso_storage_path_clear;

	int get_storage_path_from_file (std::string & out_path, const char * in_file) {
		std::ifstream ifs(in_file);
		char buffer [4 * 1024] = {0};
		int return_code = 0;

		ifs.good() && ifs.getline(buffer, FIXS_ITH_ARRAY_SIZE(buffer));

		if (ifs.good()) {
			out_path = buffer;

			if (out_path.empty()) {
				FIXS_ITH_LOG(LOG_LEVEL_WARN, "Got an empty storage path for the clear repository area");
			} else {
				std::string::const_reverse_iterator crit = out_path.rbegin();
				if ((crit != out_path.rend()) && (*crit != '/')) out_path += '/';
				out_path += g_config_pso_storage_application_folder_name;

				FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "CBA PSO storage path for clear area == <%s>", out_path.c_str());
			}
		}

		if (!ifs.good()) {
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Cannot read the CBA PSO configuration file <%s> to get the storage path for the clear repository",
					in_file);
			return_code = -1;
		}

		ifs.close();

		return return_code;
	}
}

const std::string & fixs_ith_brf_common::cba_pso_storage_path_clear () {
	if (g_cba_pso_storage_path_clear.empty())
		get_storage_path_from_file(g_cba_pso_storage_path_clear, g_config_pso_storage_path_clear);

	return g_cba_pso_storage_path_clear;
}
