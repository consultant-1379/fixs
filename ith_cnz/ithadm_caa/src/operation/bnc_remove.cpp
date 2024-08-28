/*
 * bnc_remove.cpp
 *
 *  Created on: Apr 29, 2016
 *      Author: eanform
 */

#include "operation/bnc_remove.h"
#include "operation/operation.h"
#include "fixs_ith_programconstants.h"
#include "fixs_ith_programmacros.h"

#include "fixs_ith_logger.h"
#include "fixs_ith_workingset.h"

namespace operation
{

/* **************************************
 * class bnc_remove
 * ************************************** */

	bnc_remove::bnc_remove()
    : OperationBase(BNC_OBJECT_REMOVE)
    {
    }

	void bnc_remove::setOperationDetails(const void* op_details)
	{
		const char* key = reinterpret_cast<const char*>(op_details);

		m_switchboard_key.assign(key);
	}

    int bnc_remove::call()
    {
        FIXS_ITH_LOG(LOG_LEVEL_INFO, " ->>> 'Remove switchboard configuration' operation requested");

    	FIXS_ITH_LOG(LOG_LEVEL_INFO, "remove_switchboard_configuration: Removing configuration for switch board '%s' ...",
    			m_switchboard_key.c_str());

    	bool contextCleared = false;

    	{//Empty context
    		engine::contextAccess_t contextAccess(m_switchboard_key, engine::GET_EXISTING, engine::EXCLUSIVE_ACCESS);
    		if (engine::CONTEXT_ACCESS_ACQUIRED == contextAccess.getAccessResult())
    		{
    			contextAccess.setRemovContextOngoing();
    			contextCleared = true;
    		}
    	}//release context lock

    	fixs_ith::ErrorConstants call_result = fixs_ith::ERR_CONFIG_CONTEXT_NOT_ACCESSIBLE;

    	if (contextCleared)
    	{
    		call_result = fixs_ith::workingSet_t::instance()->get_transportBNChandler().delete_bnc_objects(m_switchboard_key);

    		if (call_result)
    		{ // ERROR
    			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'start' failed: cannot remove switchboard configuration");

    		}
    		else
    		{
    			FIXS_ITH_LOG(LOG_LEVEL_INFO, "bnc_instantiate : Removing switchboard configuration' executed successfully"); // OK

    			engine::contextAccess_t contextAccess(m_switchboard_key, engine::GET_EXISTING, engine::EXCLUSIVE_ACCESS);
    			if (engine::CONTEXT_ACCESS_ACQUIRED == contextAccess.getAccessResult())
    			{
    				contextAccess.destroyContext();
    			}
    		}
    	}

    	m_operationResult.setErrorCode(call_result);
  		setResultToCaller();

        return fixs_ith::ERR_NO_ERRORS;
    }
}
