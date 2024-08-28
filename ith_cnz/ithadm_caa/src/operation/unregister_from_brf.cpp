/*
 * unregister_from_brf.cpp
 *
 *  Created on: Sep 12, 2016
 *      Author: xassore
 */

#include "operation/unregister_from_brf.h"
#include "fixs_ith_programconstants.h"

#include "fixs_ith_logger.h"
#include "fixs_ith_workingset.h"

namespace operation {

const int IMM_OBJECT_NOT_EXIST = -12;

unregister_from_brf::unregister_from_brf()
	 : OperationBase(UNREGISTER_FROM_BRF)
	{

	}

	int unregister_from_brf::call()
	{
		FIXS_ITH_LOG(LOG_LEVEL_INFO, " ->>> 'Unregister ITH from BRF as Backup participant' operation requested");

		int call_result = fixs_ith::ERR_NO_ERRORS;


		// unregister as Object Implementer on PersistantDataOwner MO.
		call_result =  fixs_ith::workingSet_t::instance()->unregisterAsBrfPartecipant();
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "BRF Object Implementers registration result:<%d>", call_result );

		if (call_result != 0)
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'BRF Object Implementer Unregistration' failed: cannot unregister from BRF");

		else
		{
			//  delete the BRF object
			if((call_result = delete_brf_participant()) == fixs_ith::ERR_NO_ERRORS)

			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Successful delete BRF object ");
		}

		// set result to caller
		m_operationResult.setErrorCode(call_result);
		setResultToCaller();

		return fixs_ith::ERR_NO_ERRORS;
}


int unregister_from_brf::delete_brf_participant()
{
	int result = fixs_ith::ERR_NO_ERRORS;

	OmHandler om_handler;

	if(om_handler.Init() != ACS_CC_SUCCESS)
	{
		// ERROR initializing imm om handler
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Internal OM IMM handler init failure: imm last error:<%d>, imm last error text:<%s>",
						om_handler.getInternalLastError(), om_handler.getInternalLastErrorText());
		return fixs_ith::ERR_OM_HANDLER_INIT_FAILURE;
	}

	const char* dn = "brfPersistentDataOwnerId=ERIC-APG-FIXS_ITH,brfParticipantContainerId=1";

	if(om_handler.deleteObject(dn) != ACS_CC_SUCCESS)
	{
		int apgcc_error_code = om_handler.getInternalLastError();
		if (apgcc_error_code == IMM_OBJECT_NOT_EXIST )
		{
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "BRF object <%s> DOES NOT EXIST", dn);
		}
		else
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "deleteObject failure: imm last error:<%d>, imm last error text:<%s>",
				apgcc_error_code,om_handler.getInternalLastErrorText());

			result = fixs_ith::ERR_IMM_DELETE_OBJ;
		}
	}

	if (om_handler.Finalize() == ACS_CC_FAILURE)
	{
	// ERROR finalizing imm internal om handler
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Internal OM IMM handler finalize failure: imm last error:<%d>, imm last error text:<%s>",
				om_handler.getInternalLastError(), om_handler.getInternalLastErrorText());
	}

	return result;
}


} /* namespace operation */



