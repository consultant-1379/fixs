/*
 * FIXS_CMXH_TrapHandler.cpp
 *  
 */

#include "FIXS_CMXH_TrapHandler.h"
#include "FIXS_CMXH_Logger.h"
using namespace std;
/*============================================================================
	ROUTINE: FMS_CPF_CmdHandler
 ============================================================================ */
FIXS_CMXH_TrapHandler::FIXS_CMXH_TrapHandler()
:trapManager(0)
{
	shutdownEvent = FIXS_CMXH_Event::OpenNamedEvent(CMXH_Util::EVENT_NAME_SHUTDOWN);

	trapManager = new FIXS_CMXH_SNMPTrapReceiver();

	setSubscribe = false;
	shutdownSubscribe = false;

 FIXS_CMXH_logging = FIXS_CMXH_Logger::getLogInstance();
}

/*============================================================================
	ROUTINE: open
 ============================================================================ */
FIXS_CMXH_TrapHandler::~FIXS_CMXH_TrapHandler()
{
	if (trapManager)
	{
		if (setSubscribe) trapManager->unsubscribe();
		sleep(1);
		delete(trapManager);
		trapManager = NULL;
	}
	FIXS_CMXH_logging = 0;
}

/*============================================================================
	ROUTINE: open
 ============================================================================ */
int FIXS_CMXH_TrapHandler::open(void *args)
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
int FIXS_CMXH_TrapHandler::svc()
{
	int result = 0;
	sleep(1);
	if(FIXS_CMXH_logging)  FIXS_CMXH_logging->Write("try to subscribe on TRAPDS !!!",LOG_LEVEL_DEBUG);
	ACS_TRAPDS::ACS_TRAPDS_API_Result res = ACS_TRAPDS::Result_Failure;
	while(!setSubscribe && !shutdownSubscribe)
	{
		res = trapManager->subscribe(CMXH_Util::FIXS_CMXH_PROCESS_NAME);
		if (res == ACS_TRAPDS::Result_Success)
		{
			std::cout << "\n------------------------------------------------------------"<< std::endl;
			std::cout << "                 TRAP Subscriber SUCCESS !!!               " << std::endl;
			std::cout << "------------------------------------------------------------\n"<< std::endl;
			if(FIXS_CMXH_logging)  FIXS_CMXH_logging->Write("TRAP Subscriber SUCCESS !!!",LOG_LEVEL_DEBUG);
			setSubscribe = true;
			shutdownSubscribe = true;

		}
		else
		{
			std::cout << "\n------------------------------------------------------------"<< std::endl;
			std::cout << "                 TRAP Subscriber FAILED !!!               " << std::endl;
			std::cout << "------------------------------------------------------------\n"<< std::endl;

			int eventIndex = FIXS_CMXH_Event::WaitForEvents(1,&shutdownEvent,6000);

			if (eventIndex == 0)
			{
				shutdownSubscribe = true;
			}
			if (eventIndex == FIXS_CMXH_Event_RC_ERROR)
			{
				shutdownSubscribe = true;
			}
		}
	}

	return result;
}
