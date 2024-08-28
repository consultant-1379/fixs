//	********************************************************
//
//	 COPYRIGHT Ericsson 2015
//	All rights reserved.
//
//	The Copyright to the computer program(s) herein
//	is the property of Ericsson 2015.
//	The program(s) may be used and/or copied only with
//	the written permission from Ericsson 2015 or in
//	accordance with the terms and conditions stipulated in
//	the agreement/contract under which the program(s) have
//	been supplied.
//
//	********************************************************
#include "operation/shutdown.h"
#include "fixs_ith_programconstants.h"

#include "fixs_ith_logger.h"

//FIXS_ITH_TRACE_DEFINE(FIXS_ITH_Operation_Shutdown)

namespace operation
{

	Shutdown::Shutdown()
	: OperationBase(SHUTDOWN)
	{
	//	FIXS_ITH_TRACE_FUNCTION;
	}

	int Shutdown::call()
	{
		// FIXS_ITH_TRACE_FUNCTION;
		FIXS_ITH_LOG(LOG_LEVEL_INFO, " ->>> 'Shutdown scheduler' operation ongoing");

		//engine::workingSet_t::instance()->stopMainReactor();

		return fixs_ith::ERR_SVC_DEACTIVATE;
	}

} /* namespace operation */
