/*
 * FIXS_CCH_TrapTimer.cpp
 *
 *  Created on: Aug 9, 2011
 *      Author: qrenbor
 */

#include "FIXS_CCH_TrapTimer.h"

FIXS_CCH_TrapTimer::~FIXS_CCH_TrapTimer()
{
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
	FIXS_CCH_Event::CloseEvent(eventToNotify);
	//FIXS_CCH_Event::CloseEvent(m_shutdownEvent);
	delete(trace);

	FIXS_CCH_logging = 0;
	// TODO Auto-generated destructor stub
}

FIXS_CCH_TrapTimer::FIXS_CCH_TrapTimer()
{
	 trace=new ACS_TRA_trace("FIXS_CCH_TrapTimer");
	 FIXS_CCH_logging = FIXS_CCH_DiskHandler::getLogInstance();
	 eventToNotify = -1;
	 m_StopEvent = -1;
	 sec = 0;
	 timerId = -1;
	 m_running = false;
	 m_exit = true;
	 m_shutdownEvent = -1;
	 std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
}

int FIXS_CCH_TrapTimer::setTrapTime(int sec)
{
	this->sec = sec;
	return timerId;
}

int FIXS_CCH_TrapTimer::stopTimer()
{
	m_exit = true;
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;

	if (m_StopEvent)
	{
		int retries=0;
		cout << __FUNCTION__ <<" " << __LINE__ << endl;
		while (FIXS_CCH_Event::SetEvent(m_StopEvent) == 0)
		{
			cout << "Set StopEvent.... retries: " << retries << endl;
			if( ++retries > 10) break;
			sleep(1);
		}

		if (retries>10)
		{
			if (trace->ACS_TRA_ON())
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Error: Cannot signal Stop event.", __FUNCTION__, __LINE__);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
				trace->ACS_TRA_event(1,tmpStr);
			}
		}
	}

	cout << "\n#################################################################" << endl;
	cout << " 	   TRAP TIMER stopped !!! "	 << endl;
	cout << "#################################################################\n" << endl;

	return 0;
}


int FIXS_CCH_TrapTimer::closeHandle()
{
	FIXS_CCH_Event::CloseEvent(eventToNotify);
	FIXS_CCH_Event::CloseEvent(m_StopEvent);
	eventToNotify=-1;
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;

	return true;
}


int FIXS_CCH_TrapTimer::getEvent()
{
	if(eventToNotify == -1)
	{
		eventToNotify=FIXS_CCH_Event::CreateEvent(false,false,0);
		std::cout<<"FIXS_CCH_TrapTimer: Creating timer event! "<<eventToNotify<<"\n";
		m_StopEvent=FIXS_CCH_Event::CreateEvent(false,false,0);
	}

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;

	return eventToNotify;

}

int FIXS_CCH_TrapTimer::open(void *args)
{
	UNUSED(args);
	int result = 0;

	m_exit = false;

	cout << "FIXS_CCH_TrapTimer try to activate..." << endl;
	//result = this->ACE_Task_Base::activate();
	activate();


	if (trace->ACS_TRA_ON())
	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "FIXS_CCH_TrapTimer [%s@%d] try to activate thread", __FUNCTION__, __LINE__);
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
		trace->ACS_TRA_event(1,tmpStr);
	}

	cout << "\n#################################################################" << endl;
	cout << "		TRAP TIMER activate !!! "	 << endl;
	cout << "#################################################################\n" << endl;

	return result;
}


int FIXS_CCH_TrapTimer::svc()
{

	const int Number_Of_Events = 2;
	const int WAIT_FAILED = -1;
	const int WAIT_TIMEOUT = -2;
	const int WAIT_OBJECT_0 = 0;
	const int Shutdown_Event_Index = 0;
	const int Stop_Event_Index = 1;
//	sleep(1);
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;

	m_shutdownEvent = FIXS_CCH_Event::OpenNamedEvent(CCH_Util::EVENT_NAME_SHUTDOWN);

	FIXS_CCH_EventHandle handleArr[Number_Of_Events] = {m_shutdownEvent, m_StopEvent};

	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "FIXS_CCH_TrapTimer [%s@%d] Thread activated on event: [%d] ", __FUNCTION__, __LINE__,getEvent());
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_INFO);
		if (trace->ACS_TRA_ON())trace->ACS_TRA_event(1,tmpStr);
	}

	m_running = true;

	while ( ! m_exit ) // Set to true when the thread shall exit
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
		// Wait for shutdown and command events
		int returnValue = FIXS_CCH_Event::WaitForEvents(Number_Of_Events, handleArr, sec);
		switch (returnValue)
		{
		case WAIT_FAILED:   // Event failed
			{
				std::cout << "WAIT_FAILED" << std::endl;
				//std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " ScxUpgrade WAIT_FAILED... GetLastError() == " << GetLastError() << std::endl;
				if (trace->ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "FIXS_CCH_TrapTimer [%s@%d] Error: Cannot WaitForMultipleObjects )", __FUNCTION__, __LINE__);
					if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
					trace->ACS_TRA_event(1,tmpStr);
				}

				m_exit = true;    // Terminate the sw upgrade thread
				break;
			}

			case (WAIT_TIMEOUT):  // Time out
			{
				if (! m_exit) {
					std::cout << __FUNCTION__ << "@" << __LINE__ << " Time-out signaled !" << std::endl;

					FIXS_CCH_Event::SetEvent(eventToNotify);

					if (trace->ACS_TRA_ON())
					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "FIXS_CCH_TrapTimer [%s@%d] Weak up the timer on event: [%d] ", __FUNCTION__, __LINE__,getEvent());
						if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
						trace->ACS_TRA_event(1,tmpStr);
					}
				}
				else {
					std::cout << __FUNCTION__ << "@" << __LINE__ << "Time-out will be not signaled !" << std::endl;
					if (trace->ACS_TRA_ON())
					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "FIXS_CCH_TrapTimer [%s@%d] Time-out will be not signaled on event: [%d] !", __FUNCTION__, __LINE__,getEvent());
						if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
						trace->ACS_TRA_event(1,tmpStr);
					}
				}

//				m_exit = true;

				break;
			}

			case (WAIT_OBJECT_0 + Shutdown_Event_Index):  // Shutdown event
			{
				std::cout << __FUNCTION__ << "@" << __LINE__ << " FIXS_CCH_TrapTimer Shutdown_Event_Index..." << std::endl;
				//FIXS_CCH_Event::ResetEvent(m_shutdownEvent);
				if (trace->ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "FIXS_CCH_TrapTimer [%s@%d] Recieved a shutdown event", __FUNCTION__, __LINE__);
					if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
					trace->ACS_TRA_event(1,tmpStr);
				}

				m_exit = true;    // Terminate the thread
				break;
			}
			case (WAIT_OBJECT_0 + Stop_Event_Index):  // Stop Timer event
			{
				std::cout << __FUNCTION__ << "@" << __LINE__ << " FIXS_CCH_TrapTimer Stop_Event_Index..." << std::endl;
				FIXS_CCH_Event::ResetEvent(m_StopEvent);
				if (trace->ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "FIXS_CCH_TrapTimer [%s@%d] Recieved a stop event", __FUNCTION__, __LINE__);
					if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
					trace->ACS_TRA_event(1,tmpStr);
				}

				m_exit = true;    // Stop the thread
				break;
			}

		} // End of switch

	} // End of the while loop

	if (trace->ACS_TRA_ON())
	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "FIXS_CCH_TrapTimer [%s@%d] Exiting from while on event [%d]", __FUNCTION__, __LINE__,getEvent());
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
		trace->ACS_TRA_event(1,tmpStr);
	}
	std::cout << __FUNCTION__ << "@" << __LINE__ << " FIXS_CCH_TrapTimer Exiting from while() ..." << std::endl;

	m_running = false;

	return 0;
}
