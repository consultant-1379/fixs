//	********************************************************
//
//	 COPYRIGHT Ericsson 2016
//	All rights reserved.
//
//	The Copyright to the computer program(s) herein
//	is the property of Ericsson 2016.
//	The program(s) may be used and/or copied only with
//	the written permission from Ericsson 2016 or in
//	accordance with the terms and conditions stipulated in
//	the agreement/contract under which the program(s) have
//	been supplied.
//
//	********************************************************

#include "operation/trapds_unsubscribe.h"
#include "operation/operation.h"
#include "fixs_ith_programconstants.h"
#include "fixs_ith_programmacros.h"

#include "fixs_ith_logger.h"
#include "fixs_ith_workingset.h"

namespace operation
{

    trapds_unsubscribe::trapds_unsubscribe()
    : OperationBase(TRAPDS_UNSUBSCRIBE)
    {
    }

    int trapds_unsubscribe::call()
    {
        FIXS_ITH_LOG(LOG_LEVEL_INFO, " ->>> 'TRAPDS unsubscribe' operation requested!");

    	FIXS_ITH_LOG(LOG_LEVEL_INFO, "TRAPDS observer: unsubscribing  observer to SNMP TRAP Dispatcher...");

    	int call_result = fixs_ith::workingSet_t::instance()->get_trapdsobserver().unsubscribe();

    	if (call_result) { // ERROR
    		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'start' failed: cannot unsubscribe observer to SNMP TRAP Dispatcher");
    	}
    	else
    		FIXS_ITH_LOG(LOG_LEVEL_INFO, "TRAPDS observer: observer successfully unsubscribed"); // OK

    	// set result to caller
		m_operationResult.setErrorCode(call_result);
		setResultToCaller();

        return fixs_ith::ERR_NO_ERRORS;
    }
} /* namespace operation */




