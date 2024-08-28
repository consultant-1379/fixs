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
#include "operation/stop.h"
#include "fixs_ith_workingset.h"
#include "fixs_ith_programconstants.h"

//#include "common/tracer.h"
#include "fixs_ith_logger.h"


//FIXS_ITH_TRACE_DEFINE(FIXS_ITH_Operation_Stop)

namespace operation
{
	Stop::Stop()
	: OperationBase(STOP)
	{
		// FIXS_ITH_TRACE_FUNCTION;
	}

	int Stop::call()
	{
		FIXS_ITH_LOG(LOG_LEVEL_INFO, "Stop executed");

		int immResult = fixs_ith::workingSet_t::instance()->unregisterObjectImplementers();

		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Object Implementers unregistration result:<%d>", immResult);

		// before destroying contexts, we must be sure that all relative "context schedulers" are not active
		int stop_context_schedulers_res = fixs_ith::workingSet_t::instance()->stop_context_schedulers(true);
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Stop context schedulers result:<%d>", stop_context_schedulers_res);

		//destroy contexts
		std::set<std::string> switchboard_keys = fixs_ith::workingSet_t::instance()->getConfigurationManager().getContextKeys();
		for (std::set<std::string>::iterator it = switchboard_keys.begin(); it != switchboard_keys.end(); ++it)
		{
			//Get and lock the context
			engine::contextAccess_t access(*it, engine::GET_EXISTING, engine::EXCLUSIVE_ACCESS);
			if (engine::CONTEXT_ACCESS_ACQUIRED == access.getAccessResult())
			{
				access.destroyContext();
			}
		}

		//Clear configuration manager
		fixs_ith::workingSet_t::instance()->getConfigurationManager().clear();

		// Cease All alarms
		fixs_ith::workingSet_t::instance()->getAlarmHandler().ceaseAll();

		return fixs_ith::ERR_NO_ERRORS;
	}

} /* namespace operation */
