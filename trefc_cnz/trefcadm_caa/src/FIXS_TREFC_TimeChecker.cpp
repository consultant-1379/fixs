/*
 * FIXS_TREFC_TimeChecker.cpp
 *
 *  Created on: Jan 9, 2012
 *      Author: eanform
 */

#include "FIXS_TREFC_TimeChecker.h"
#include "FIXS_TREFC_Logger.h"



#ifndef __
#define __ std::cout << __FUNCTION__ << "@" << __LINE__ << std::endl;
#endif

// Class FIXS_TREFC_TimeChecker
FIXS_TREFC_TimeChecker::FIXS_TREFC_TimeChecker ()
      : m_running(false),
        m_shutdownEvent(0),
        m_exit(false),
        m_setTime(false),
        m_scxTrap(false),
        m_scxIp(""),
        m_scxSlot(100),
        m_isApzMag(false)
{
	traceTimeChecker = new ACS_TRA_trace("FIXS_TREFC_TimeChecker");
}


FIXS_TREFC_TimeChecker::~FIXS_TREFC_TimeChecker()
{

	stop();

	if (isRunningThread())
	{
		//trace error
		{
			char traceChar[512] = {0};
			snprintf(traceChar, sizeof(traceChar) - 1, "[%s@%d] Cannot stop Thread", __FUNCTION__, __LINE__);
			if (traceTimeChecker->ACS_TRA_ON()) traceTimeChecker->ACS_TRA_event(1,traceChar);
		}


		sleep(1);

	}

	//Delete trace object
	if (traceTimeChecker) {
		delete (traceTimeChecker);
		traceTimeChecker = NULL;
	}
}


void FIXS_TREFC_TimeChecker::setTimeOnAllScx ()
{
	std::cout<<"Calling FIXS_TREFC_Manager::setTimeOnAllScx()"<<std::endl;
	if (FIXS_TREFC_Manager::getInstance()->setTimeOnAllScx() != 0)
	{
		//trace error
		std::cout<<"DBG: set time on SCX failed"<<std::endl;
		{
			char traceChar[512] = {0};
			snprintf(traceChar, sizeof(traceChar) - 1, "[%s@%d] WARNING: Set time on SCX failed", __FUNCTION__, __LINE__);
			if (traceTimeChecker->ACS_TRA_ON()) traceTimeChecker->ACS_TRA_event(1,traceChar);
		}
	}

}


int FIXS_TREFC_TimeChecker::open (void *args)
{
	int result = 0;
	UNUSED(args);
	result = activate();
	sleep(1);
	cout << "FIXS_TREFC_TimeChecker activate: " << result << endl;
	m_setTime = false;
	return result;
}

//unsigned FIXS_TREFC_TimeChecker::run (void *procArgs)
int FIXS_TREFC_TimeChecker::svc ()
{



	int timeNow = 0;
	int timeLast = 0;
	int diff = 0;
	int count = 0;

	const int Number_Of_Events=1;
	const int WAIT_FAILED=-1;
	const int WAIT_TIMEOUT= -2;
	const int WAIT_OBJECT_0=0;
	const int Shutdown_Event_Index=0;

	//Open shutdown event
	m_shutdownEvent=FIXS_TREFC_Event::OpenNamedEvent(TREFC_Util::EVENT_NAME_SHUTDOWN);

	m_exit = false;
	m_running = true;
	m_setTime = false;


	FIXS_TREFC_EventHandle handleArr[Number_Of_Events] = {m_shutdownEvent};

	//loop
	while ( !m_exit ) // Set to true when the thread shall exit
	{

		int returnValue=FIXS_TREFC_Event::WaitForEvents(Number_Of_Events, handleArr, 3000);

		switch (returnValue)
		{
		case WAIT_FAILED:   // Event failed
		{
			if (traceTimeChecker->ACS_TRA_ON())
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Error: Cannot WaitForMultipleObjects )", __FUNCTION__, __LINE__);
				traceTimeChecker->ACS_TRA_event(1,tmpStr);
			}

			m_exit = true;    // Terminate the sw upgrade thread
			break;
		}

		case (WAIT_TIMEOUT):  // Time out
			{
			//TODO:These tracings seem useless...
//			if (!m_exit)
//			{
//				if (traceTimeChecker->ACS_TRA_ON())
//				{
//					char tmpStr[512] = {0};
//					snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Weak up the timer...", __FUNCTION__, __LINE__);
//					traceTimeChecker->ACS_TRA_event(1,tmpStr);
//				}
//			}
//			else
//			{
//				std::cout << __FUNCTION__ << "@" << __LINE__ << "Time-out will be not signaled !" << std::endl;
//				if (traceTimeChecker->ACS_TRA_ON())
//				{
//					char tmpStr[512] = {0};
//					snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Time-out will be not signaled !", __FUNCTION__, __LINE__);
//					traceTimeChecker->ACS_TRA_event(1,tmpStr);
//				}
//			}

			break;
			}

		case (WAIT_OBJECT_0 + Shutdown_Event_Index):  // Shutdown event
			{

			if (traceTimeChecker->ACS_TRA_ON())
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Received a shutdown event", __FUNCTION__, __LINE__);
				traceTimeChecker->ACS_TRA_event(1,tmpStr);
			}

			m_exit = true;  // Terminate the thread
			m_running = false;
			return 0;
			}
		default:
		{
			if (traceTimeChecker->ACS_TRA_ON())
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1,"[%s@%d] Error: Unexpected event signaled.", __FUNCTION__, __LINE__);
				traceTimeChecker->ACS_TRA_event(1,tmpStr);
			}
			break;
		}
		} // End of switch


		if(m_setTime)
		{
			count++;
			if (count >= 10) // check every 30 sec
			{
				count = 0;


				//#pragma warning( disable : 4267; disable : 4244; disable : 4311 )
				//Get system time
				timeNow = time(NULL);
				//#pragma warning( default : 4267; default : 4244; default : 4311 )

				//Check difference
				diff = abs(timeNow - timeLast);
				timeLast = timeNow;
				if (diff > 60)	//diff should be about 30
				{
					if (traceTimeChecker->ACS_TRA_ON())
					{ //trace
						char traceChar[512] = {0};
						snprintf(traceChar, sizeof(traceChar) - 1, "[%s@%d] WARNING: system data to update. diff is %d sec", __FUNCTION__, __LINE__, diff);
						if (traceTimeChecker->ACS_TRA_ON()) traceTimeChecker->ACS_TRA_event(1,traceChar);
					}
					std::cout<<"DBG: Calling setTimeonAllScx()"<<std::endl;
					setTimeOnAllScx();
				}
			}

		}

		if(m_scxTrap)
		{
			m_scxTrap = false; //reset this flag to false to wait for next cold start/warm restart trap
			timeLast = time(NULL); //reset timeLast to get the right time difference
		}
	}//while
	return 0;

}

 void FIXS_TREFC_TimeChecker::stop ()
{
	m_exit = true;
}

 void FIXS_TREFC_TimeChecker::startTimeSet ()
{
	FIXS_TREFC_Logger::getLogInstance()->Write("[FIXS_TREFC_TimeChecker] Starting Time Checker Thread.");
	m_setTime = true;
}

 void FIXS_TREFC_TimeChecker::stopTimeSet ()
{
	FIXS_TREFC_Logger::getLogInstance()->Write("[FIXS_TREFC_TimeChecker] Stopping Time Checker Thread.");
	m_setTime = false;
}

 //TODO: Nobody call this method. What is it for?
 void FIXS_TREFC_TimeChecker::setScxTrap (bool trap, std::string ip, bool apzMag, unsigned short slot)
{
	m_scxTrap = trap;
	//TODO: What's the use of this variables?
	m_scxIp = ip;
	m_isApzMag = apzMag;
	m_scxSlot = slot;
}

 bool FIXS_TREFC_TimeChecker::getTimeSet ()
{
	return m_setTime;
}

