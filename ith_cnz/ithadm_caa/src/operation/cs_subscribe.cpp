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

#include "operation/cs_subscribe.h"
#include "fixs_ith_programconstants.h"

#include "fixs_ith_logger.h"
#include "fixs_ith_workingset.h"


namespace operation
{

    cs_subscribe::cs_subscribe()
    : OperationBase(CS_SUBSCRIBE)
    {
    }

    int cs_subscribe::call()
    {
        FIXS_ITH_LOG(LOG_LEVEL_INFO, " ->>> 'CS subscription' operation requested");

    	FIXS_ITH_LOG(LOG_LEVEL_INFO, "CS HWC table changes observer: starting observing for HWC table change notification from CS interface...");

    	int call_result = fixs_ith::workingSet_t::instance()->get_cs_hwctablechangeobserver().start();

    	if (call_result) { // ERROR
    		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'start' failed: cannot start the observer to receive notifications from CS for HWC table change notifications");
    	}
    	else
    		FIXS_ITH_LOG(LOG_LEVEL_INFO, "CS HWC table changes observer: observer started successfully"); // OK

    	// set result to caller
    	m_operationResult.setErrorCode(call_result);
    	setResultToCaller();

        return fixs_ith::ERR_NO_ERRORS;
    }
} /* namespace operation */


