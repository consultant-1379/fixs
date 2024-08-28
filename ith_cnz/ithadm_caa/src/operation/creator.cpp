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

#include "operation/creator.h"
#include "operation/noop.h"
#include "operation/start.h"
#include "operation/stop.h"
#include "operation/shutdown.h"
#include "operation/cs_subscribe.h"
#include "operation/cs_unsubscribe.h"
#include "operation/trapds_subscribe.h"
#include "operation/trapds_unsubscribe.h"
#include "operation/register_imm.h"
#include "operation/reload_all_from_imm.h"
#include "operation/reload_context_from_imm.h"
#include "operation/bnc_instantiate.h"
#include "operation/bnc_remove.h"
#include "operation/add_bfd_sessions.h"
#include "operation/remove_bfd_sessions.h"
#include "operation/add_bfd_session_for_nexthop.h"
#include "operation/remove_bfd_session_for_nexthop.h"
#include "operation/check_config_consistency.h"
#include "operation/check_sfpport_consistency.h"
#include "operation/magazine_switchboards_config_restore.h"
#include "operation/cease_alarm.h"

#include "operation/register_to_brf.h"
#include "operation/unregister_from_brf.h"
#include "operation/set_ports_admin_state.h"
#include "operation/pim_adaptation.h"

#include "fixs_ith_programconstants.h"
#include "operation/reload_context_after_cold_start_trap.h"


#include <ace/Method_Request.h>

#include "fixs_ith_logger.h"
#include "fixs_ith_programmacros.h"


namespace operation {

	int Creator::schedule(const operation::identifier_t id)
	{
		int result = fixs_ith::ERR_NO_ERRORS;

		if(NULL == _scheduler)
		{
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "unable to schedule operation:<%d> request. Invalid scheduler !", id);
			result = fixs_ith::ERR_INVALID_OBJECT;
		}
		else
		{
			OperationBase* operation = make(id);

			if( operation )
			{
				//result = engine::workingSet_t::instance()->getScheduler().enqueue(operation);
				result = _scheduler->enqueue(operation);
			}
			else
			{
				FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "cannot allocate new operation:<%d> request", id);
				result = fixs_ith::ERR_MEMORY_BAD_ALLOC;
			}
		}

		return result;
	}

	int Creator::schedule(const operation::identifier_t id, const void* op_details)
	{
		int result = fixs_ith::ERR_NO_ERRORS;

		if(NULL == _scheduler)
		{
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "unable to schedule operation:<%d> request. Invalid scheduler !", id);
			result = fixs_ith::ERR_INVALID_OBJECT;
		}
		else
		{
			OperationBase* operation = make(id);

			if( operation )
			{
				operation->setOperationDetails(op_details);

				//result = engine::workingSet_t::instance()->getScheduler().enqueue(operation);
				result = _scheduler->enqueue(operation);
			}
			else
			{
				FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "cannot allocate new operation:<%d> request", id);
				result = fixs_ith::ERR_MEMORY_BAD_ALLOC;
			}
		}

		return result;
	}

	int Creator::schedule(const operation::identifier_t id, ACE_Future<operation::result>* op_result, const void* op_details)
	{
		int result = fixs_ith::ERR_NO_ERRORS;

		if(NULL == _scheduler)
		{
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "unable to schedule operation:<%d> request. Invalid scheduler !", id);
			result = fixs_ith::ERR_INVALID_OBJECT;
		}
		else
		{
			OperationBase* operation = make(id);

			if( operation )
			{
				operation->setOperationResultRequest(op_result);
				operation->setOperationDetails(op_details);

				//result = engine::workingSet_t::instance()->getScheduler().enqueue(operation);
				result = _scheduler->enqueue(operation);
			}
			else
			{
				FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "cannot allocate new operation:<%d> request", id);
				result = fixs_ith::ERR_MEMORY_BAD_ALLOC;
			}
		}

		return result;
	}

	int Creator::execute(const operation::identifier_t id, const void* op_details)
	{
		int result = fixs_ith::ERR_NO_ERRORS;

		OperationBase* operation = make(id);

		if( operation )
		{
			operation->setOperationDetails(op_details);

			//result = engine::workingSet_t::instance()->getScheduler().enqueue(operation);
			result = operation->call();
		}
		else
		{
			FIXS_ITH_LOG_ERRNO(errno, LOG_LEVEL_ERROR, "cannot allocate new operation:<%d> request", id);
			result = fixs_ith::ERR_MEMORY_BAD_ALLOC;
		}


		return result;
	}

	OperationBase* Creator::make(const operation::identifier_t id)
	{
		OperationBase* operation;

		switch(id)
		{
			case START:
			{
				operation = new (std::nothrow) Start();
			}
			break;

			case STOP:
			{
				operation = new (std::nothrow) Stop();
			}
			break;

			case SHUTDOWN:
			{
				operation = new (std::nothrow) Shutdown();
			}
			break;

			case CS_SUBSCRIBE:
			{
				operation = new (std::nothrow) cs_subscribe();
			}
			break;

			case CS_UNSUBSCRIBE:
			{
				operation = new (std::nothrow) cs_unsubscribe();
			}
			break;

			case TRAPDS_SUBSCRIBE:
			{
				operation = new (std::nothrow) trapds_subscribe();
			}
			break;

			case TRAPDS_UNSUBSCRIBE:
			{
				operation = new (std::nothrow) trapds_unsubscribe();
			}
			break;

			case BNC_OBJECTS_INSTANTIATE:
			{
				operation = new (std::nothrow) bnc_instantiate();
			}
			break;

			case REGISTER_IMM_IMPLEMENTER:
			{
				operation = new (std::nothrow) register_imm();
			}
			break;

			case UNREGISTER_IMM_IMPLEMENTER:
			{
				//TODO
				operation = new (std::nothrow) NoOp();
			}
			break;

			case RELOAD_ALL_FROM_IMM:
			{
				operation = new (std::nothrow) reload_all_from_imm();
			}
			break;

			case RELOAD_CONTEXT_FROM_IMM:
			{
				operation = new (std::nothrow) reload_context_from_imm();
			}
			break;

			case RELOAD_CONTEXT_AFTER_COLD_START_TRAP:
			{
				operation = new (std::nothrow) reload_context_after_cold_start_trap();
			}
			break;

			case BNC_OBJECT_REMOVE:
			{
				operation = new (std::nothrow) bnc_remove();
			}
			break;

			case ADD_BFD_SESSIONS:
			{
				operation = new (std::nothrow) Add_BFD_Sessions();
			}
			break;

			case REMOVE_BFD_SESSIONS:
			{
				operation = new (std::nothrow) Remove_BFD_Sessions();
			}
			break;

			case ADD_BFD_SESSION_FOR_NEXTHOP:
			{
				operation = new (std::nothrow) Add_BFD_Session_For_NextHop();
			}
			break;

			case REMOVE_BFD_SESSION_FOR_NEXTHOP:
			{
				operation = new (std::nothrow) Remove_BFD_Session_For_NextHop();
			}
			break;

			case CHECK_CONFIG_CONSISTENCY:
			{
				operation = new (std::nothrow) check_config_consistency();
			}
			break;

			case MAGAZINE_SWITCHBOARDS_CONFIG_RESTORE:
			{
				operation = new (std::nothrow) Magazine_switchboards_config_restore();
			}
			break;

			case CEASE_ALARM:
			{
				operation = new (std::nothrow) cease_alarm();
			}
			break;

			case REGISTER_TO_BRF:
			{
				operation = new (std::nothrow) register_to_brf();
			}
			break;

			case UNREGISTER_FROM_BRF:
			{
				operation = new (std::nothrow) unregister_from_brf();
			}
			break;

			case CHECK_SFPPORT_CONSISTENCY:
			{
				operation = new (std::nothrow) check_sfpport_consistency();
			}
			break;

			case SET_PORT_ADMIN_STATE:
			{
				operation = new (std::nothrow) set_ports_admin_state();
			}
			break;

			case PIM_ADAPTATION:
			{
				operation = new (std::nothrow) pim_adaptation();
			}
			break;

			default:
			{
				operation = new (std::nothrow) NoOp();
			}
		}

		return operation;

	}


} /* namespace operation */

