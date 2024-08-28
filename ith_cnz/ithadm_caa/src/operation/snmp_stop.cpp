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

#include "operation/snmp_stop.h"
#include "operation/operation.h"
#include "fixs_ith_programconstants.h"
#include "fixs_ith_programmacros.h"

#include "fixs_ith_logger.h"
#include "fixs_ith_workingset.h"


namespace operation
{
//
//	snmp_trap_unsubscribe::snmp_trap_unsubscribe()
//    : OperationBase(SNMP_TRAP_UNSUBSCRIBE)
//    {
//    }
//
//    int snmp_trap_unsubscribe::call()
//    {
//        FIXS_ITH_LOG(LOG_LEVEL_INFO, "SNMP TRAP  unsubscribe called");
//
//    	FIXS_ITH_LOG(LOG_LEVEL_INFO, "snmp_trap_unsubscribe:: starting unsubscription  to SNMP AGENT...");
//
//    	int call_result = fixs_ith::workingSet_t::instance()->get_snmpmanager().send_trap_unsubscription_request();
//
//    	if (call_result) { // ERROR
//    		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'start' failed: cannot unsubscribe the observer to SNMP TRAP Dispatcher");
//
//    		// TODO Rescheduling the same operation
//    		::sleep(3); // TO BE REMOVED !!!
//    		fixs_ith::workingSet_t::instance()->schedule(operation::SNMP_TRAP_UNSUBSCRIBE);
//
//    		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "SNMP TRAP unsubscribe operation rescheduled successfully");
//    	}
//    	else
//    		FIXS_ITH_LOG(LOG_LEVEL_INFO, "SNMP TRAP unsubscribe : SNMP AGENT unsubscription executed successfully"); // OK
//
//        return fixs_ith::ERR_NO_ERRORS;
//    }

}
