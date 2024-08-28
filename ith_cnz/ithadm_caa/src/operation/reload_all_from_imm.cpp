/*
 * reload_all_from_imm.cpp
 *
 *  Created on: Apr 4, 2016
 *      Author: estevol
 */

#include "operation/reload_all_from_imm.h"
#include "fixs_ith_programconstants.h"

#include "fixs_ith_logger.h"
#include "fixs_ith_workingset.h"

namespace operation {

reload_all_from_imm::reload_all_from_imm()
: OperationBase(RELOAD_ALL_FROM_IMM)
{

}

reload_all_from_imm::~reload_all_from_imm()
{

}

int reload_all_from_imm::call()
{
	FIXS_ITH_LOG(LOG_LEVEL_INFO, " ->>> 'RELOAD ALL FROM IMM' operation requested");

	// Empty key for common objects
	std::string commonObjectKey;

	int call_result = fixs_ith::workingSet_t::instance()->getConfigurationManager().loadConfiguration(commonObjectKey);

	if (fixs_ith::ERR_NO_ERRORS != call_result)
	{
		FIXS_ITH_LOG(LOG_LEVEL_INFO, " ->>> 'RELOAD COMMON OBJ FROM IMM' operation failed");
	}
	else
	{
		// Load Switchboard Keys from CS
		std::set<std::string> switchBoardKeys;
		call_result =  fixs_ith::workingSet_t::instance()->get_sbdatamanager().get_switchboard_keys(switchBoardKeys);

		if (call_result)
		{ // ERROR
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'RELOAD ALL FROM IMM' failed: cannot load switchboard keys from CS");
		}
		else
		{
			FIXS_ITH_LOG(LOG_LEVEL_INFO, "Loaded SwitchBoard keys from CS. Reloading Contexts from IMM"); // OK

			for (std::set<std::string>::iterator it = switchBoardKeys.begin(); it != switchBoardKeys.end(); ++it)
			{
				const char * key = it->c_str();

				if (fixs_ith::workingSet_t::instance()->getConfigurationManager().addToReloadInProgressSet(key))
				{
					FIXS_ITH_LOG(LOG_LEVEL_INFO, "Scheduling reload for Context <%s>", key); // OK

					fixs_ith::workingSet_t::instance()->schedule_until_completion(key, RELOAD_CONTEXT_FROM_IMM, 0, 10, key, (strlen(key) + 1)/sizeof(uint8_t));
				}
			}
		}
	}

	// set result to caller
	m_operationResult.setErrorCode(call_result);
	setResultToCaller();

	return fixs_ith::ERR_NO_ERRORS;
}


} /* namespace operation */
