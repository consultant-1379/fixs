/*
 * FIXS_TREFC_TrapSubscriber.cpp
 *
 *  Created on: Jan 4, 2012
 *      Author: eanform
 */

#include "FIXS_TREFC_TrapSubscriber.h"
#include "FIXS_TREFC_Event.h"
#include "FIXS_TREFC_Util.h"

/*============================================================================
	ROUTINE: CONSTRUCTOR
 ============================================================================ */
FIXS_TREFC_TrapSubscriber::FIXS_TREFC_TrapSubscriber(FIXS_TREFC_SNMPTrapReceiver* trapManager)
:tExit(false),
 m_trapManager(trapManager),
 shutdownEvent(-1)
{
	_trace = new ACS_TRA_trace("FIXS_TREFC_TrapSubscriber");
}

/*============================================================================
	ROUTINE: DESTRUCTOR
 ============================================================================ */
FIXS_TREFC_TrapSubscriber::~FIXS_TREFC_TrapSubscriber()
{
	//Delete Trap receiver
//	if (m_trapManager) {
//		delete m_trapManager;
//		m_trapManager = NULL;
//	}

	//Delete Trace object
	if (_trace) {
		delete _trace;
		_trace = NULL;
	}

	tExit = true;

}

/*============================================================================
	ROUTINE: open
 ============================================================================ */
int FIXS_TREFC_TrapSubscriber::open(void *args)
{
	int result = 0;

	UNUSED(args);

	cout << __FUNCTION__ << "@" << __LINE__ << " Try to activate..." << endl;
	result = activate();

    return result;
}

/*============================================================================
	ROUTINE: close
 ============================================================================ */
int FIXS_TREFC_TrapSubscriber::close ()
{
    return 0;
}

/*============================================================================
	ROUTINE: svc
 ============================================================================ */
int FIXS_TREFC_TrapSubscriber::svc()
{
	shutdownEvent = FIXS_TREFC_Event::OpenNamedEvent(TREFC_Util::EVENT_NAME_SHUTDOWN);

	int result = 0;
	ACS_TRAPDS::ACS_TRAPDS_API_Result res;

	int waitRes = -1;
	bool setSubscribe = false;

	while (!setSubscribe)
	{
		res = m_trapManager->subscribe(TREFC_Util::FIXS_TREFC_PROCESS_NAME);
		if (res == ACS_TRAPDS::Result_Success)
		{
			std::cout << __FUNCTION__ << "@" << __LINE__ << "trapReceiver subscribe success !!! " << std::endl;
			printTRA("TrapReceiver subscribe success !!!");
			setSubscribe = true;

		}
		else
		{
			std::cout << __FUNCTION__ << "@" << __LINE__ << "TrapReceiver subscribe failure !!! " << std::endl;
			printTRA("TrapReceiver subscribe failure !!! ");

			waitRes = FIXS_TREFC_Event::WaitForEvents(1, &shutdownEvent, 6000);

			if (waitRes == 0)
			{
				setSubscribe = true;
			}
			if (waitRes == FIXS_TREFC_Event_RC_ERROR)
			{
				setSubscribe = true;
			}

		}
	}

	return result;


}

void FIXS_TREFC_TrapSubscriber::printTRA(std::string mesg)
{
	if (_trace->ACS_TRA_ON())
	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, mesg.c_str());
		_trace->ACS_TRA_event(1, tmpStr);

	}
}

