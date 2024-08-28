/*
 * FIXS_CCH_TrapHandler.cpp
 *
 *  Created on: Mar 8, 2012
 *      Author: eanform
 */

#include "FIXS_CCH_TrapHandler.h"

using namespace std;

/*============================================================================
	ROUTINE: FMS_CPF_CmdHandler
 ============================================================================ */
FIXS_CCH_TrapHandler::FIXS_CCH_TrapHandler()
:trapManager(0),
 svc_run(false)
{
	shutdownEvent = FIXS_CCH_Event::OpenNamedEvent(CCH_Util::EVENT_NAME_SHUTDOWN);

	trapManager = new FIXS_CCH_SNMPTrapReceiver();

	FIXS_CCH_logging = FIXS_CCH_DiskHandler::getLogInstance();

	setSubscribe = false;
	shutdownSubscribe = false;
}

/*============================================================================
	ROUTINE: open
 ============================================================================ */
FIXS_CCH_TrapHandler::~FIXS_CCH_TrapHandler()
{
	if (trapManager)
	{
//		sleep(1);
		delete(trapManager);
		trapManager = NULL;
	}

	FIXS_CCH_logging = 0;
}

/*============================================================================
	ROUTINE: open
 ============================================================================ */
int FIXS_CCH_TrapHandler::open(void *args)
{
	int result;

	UNUSED(args);

	setSubscribe = false;
	shutdownSubscribe = false;
	result = activate(THR_DETACHED);

    if(0 != result)
    {
//    	shutdownSubscribe = true;
    }
  	return result;
}

/*============================================================================
	ROUTINE: svc
 ============================================================================ */
int FIXS_CCH_TrapHandler::svc()
{
	svc_run = true;
	int result = 0;
	sleep(1);

	if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("try to subscribe on TRAPDS !!!",LOG_LEVEL_DEBUG);

	ACS_TRAPDS::ACS_TRAPDS_API_Result res = ACS_TRAPDS::Result_Failure;
	while(!shutdownSubscribe)
	{
		int timeout = 0;
		if (!setSubscribe)
		{
			res = trapManager->subscribe(CCH_Util::FIXS_CCH_PROCESS_NAME);
			if (res == ACS_TRAPDS::Result_Success)
			{
				std::cout << "\n------------------------------------------------------------"<< std::endl;
				std::cout << "                 TRAP Subscriber SUCCESS !!!               " << std::endl;
				std::cout << "------------------------------------------------------------\n"<< std::endl;
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("TRAP Subscriber SUCCESS !!!",LOG_LEVEL_DEBUG);
				setSubscribe = true;
				//			shutdownSubscribe = true;
			}
			else
			{
				std::cout << "\n------------------------------------------------------------"<< std::endl;
				std::cout << "                 TRAP Subscriber FAILED !!!               " << std::endl;
				std::cout << "------------------------------------------------------------\n"<< std::endl;
				timeout = 6000;
			}

		}


		int eventIndex = FIXS_CCH_Event::WaitForEvents(1,&shutdownEvent,timeout);

		if (eventIndex == 0)
		{
			shutdownSubscribe = true;
		}
		if (eventIndex == FIXS_CCH_Event_RC_ERROR)
		{
			shutdownSubscribe = true;
		}

	}

	if (setSubscribe)
	{
		std::cout << "\n------------------------------------------------------------"<< std::endl;
		std::cout << "                 Unsubscribing from TRAPDS !!!                " << std::endl;
		std::cout << "------------------------------------------------------------\n"<< std::endl;
		trapManager->unsubscribe();
	}

	svc_run = false;
	return result;
}
