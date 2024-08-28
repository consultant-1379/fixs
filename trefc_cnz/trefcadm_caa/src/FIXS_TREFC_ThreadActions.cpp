/*
 * FIXS_TREFC_ThreadActions.cpp
 *
 *  Created on: Jul 11, 2012
 *      Author: eanform
 */


#include "FIXS_TREFC_ThreadActions.h"
#include "FIXS_TREFC_Logger.h"

using namespace std;


/*============================================================================
	ROUTINE: FIXS_TREFC_ThreadActions
 ============================================================================ */
FIXS_TREFC_ThreadActions::FIXS_TREFC_ThreadActions()
:running(0),rollback(0)
{
	// Initialize the svc state flag
	tExit = false;

	_trace = new ACS_TRA_trace("FIXS_TREFC_ThreadActions");

}

/*============================================================================
	ROUTINE: open
 ============================================================================ */
FIXS_TREFC_ThreadActions::~FIXS_TREFC_ThreadActions()
{
	delete (_trace);
}

/*============================================================================
	ROUTINE: open
 ============================================================================ */
int FIXS_TREFC_ThreadActions::open(void *args)
{
	int result;

	UNUSED(args);

	tExit = false;

	result = activate();
	if(result != 0)
	{
		tExit = true;
	}

  	return result;
}

/*============================================================================
	ROUTINE: svc
 ============================================================================ */
int FIXS_TREFC_ThreadActions::svc()
{
	int res = 0;

	running = true;

	sleep(1);

	if (m_action == UNLOCK)
	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_ThreadActions] Executing UNLOCK action!");
		FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr);

		res = FIXS_TREFC_Manager::getInstance()->trefcStart();
		  bool stop = true;
			if(FIXS_TREFC_Manager::getInstance())		
	                   stop = FIXS_TREFC_Manager::getInstance()->isStopOperation();	
		if (res != EXECUTED)
		{

			switch (res)
			{
				case TREFC_Util::TIMESERVICE_ALREADY_STARTED: break;
				case TREFC_Util::NOT_ALLOWED_BY_CP:
					if(!stop)
					ImmInterface::setServiceResultState(IMM_Util::ADM_NOT_ALLOWED_BY_CP); break;
				case TREFC_Util::TIME_REF_NOT_CONFIGURED:
					if(!stop)
					ImmInterface::setServiceResultState(IMM_Util::ADM_REF_NOT_CONFIGURED); break;
				case TREFC_Util::INCORRECT_SERVICE:
					if(!stop)
					ImmInterface::setServiceResultState(IMM_Util::ADM_SERV_NOT_CONFIGURED); break;
				case TREFC_Util::NO_REFERENCE_EXIST:
					if(!stop)
					ImmInterface::setServiceResultState(IMM_Util::ADM_NO_REF_EXIST); break;
				case TREFC_Util::INVALID_REF:
					if(!stop)
					ImmInterface::setServiceResultState(IMM_Util::ADM_INVALID_REF); break;
				case TREFC_Util::SNMP_FAILURE:
					if(!stop)
					ImmInterface::setServiceResultState(IMM_Util::ADM_SNMP_FAILURE); break;
				case TREFC_Util::INTERNALERR:
					if(!stop)
					ImmInterface::setServiceResultState(IMM_Util::ADM_INTERNAL_ERROR); break;
				case -1:
					{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_ThreadActions] Shutdown state NO Modifications to IMM ");
					FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr,LOG_LEVEL_DEBUG);
					break;
					}
				default:
					if(!stop)
					ImmInterface::setServiceResultState(IMM_Util::ADM_INTERNAL_ERROR);
					break;
			}

			if ((res != TREFC_Util::TIMESERVICE_ALREADY_STARTED) && (res!=-1)) {
				rollback = true;
				char tmpStr[512] = {0};
				if(!stop)
				{
			 	ImmInterface::setServiceAdmState(IMM_Util::LOCKED);
			
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_ThreadActions] UNLOCK action failed: Starting Rollback phase! ");
				FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr,LOG_LEVEL_DEBUG);
				}
			}
		}
		else
		{
			if(!stop)
			{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_ThreadActions] UNLOCK action executed: Setting Result State!");
			FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr);
			
			ImmInterface::setServiceResultState(IMM_Util::EXECUTED);
			}
		}
	}
	else if (m_action == LOCK)
	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_ThreadActions] Executing LOCK action!");
		FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr);
		bool stop = true;
		res = FIXS_TREFC_Manager::getInstance()->trefcStop();
		
		if(FIXS_TREFC_Manager::getInstance())
                  stop = FIXS_TREFC_Manager::getInstance()->isStopOperation();	
		if (res != EXECUTED)
		{
			switch (res)
			{
			case TREFC_Util::TIMESERVICE_ALREADY_STOPPED: break;
			case TREFC_Util::SNMP_FAILURE:
				{
				if(!stop)
				ImmInterface::setServiceResultState(IMM_Util::ADM_SNMP_FAILURE);
				} break;
				
			case TREFC_Util::INTERNALERR:
				{
				if(!stop)
				ImmInterface::setServiceResultState(IMM_Util::ADM_INTERNAL_ERROR); 
				}break;
			case -1:
				{
				 char tmpStr[512] = {0};
                                snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_ThreadActions] Shutdown state NO Modifications to IMM ");
                                FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr,LOG_LEVEL_DEBUG);
				
				break;
				}
			default:
					if(!stop)	
					ImmInterface::setServiceResultState(IMM_Util::ADM_INTERNAL_ERROR);
			}

			if ((res != TREFC_Util::TIMESERVICE_ALREADY_STOPPED) && (res != -1)) {
				rollback = true;
				if(!stop)
				{
				ImmInterface::setServiceAdmState(IMM_Util::LOCKED);
				char tmpStr[512] = {0};	
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_ThreadActions] LOCK action failed: Starting Rollback phase! ");
				FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr,LOG_LEVEL_DEBUG);
				}	
		}
		}
		else
		{
			if(!stop)
			{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_ThreadActions] LOCK action executed: Setting Result State!");
			FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr);

			ImmInterface::setServiceResultState(IMM_Util::EXECUTED);
			}
		}
	}
	else
	{
		cout << " NO ACTION VALUE DEFINED !!!" << endl;
	}

	running = false;
	rollback = false;


	char tmpStr[512] = {0};
	snprintf(tmpStr, sizeof(tmpStr) - 1, "[FIXS_TREFC_ThreadActions] Action Thread terminated. Exiting now...");
	FIXS_TREFC_Logger::getLogInstance()->Write(tmpStr,LOG_LEVEL_DEBUG);
	return 0;
}

