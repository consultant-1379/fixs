/*
 * FIXS_TREFC_AlarmHandler.cpp
 *
 *  Created on: Nov 28, 2012
 *      Author: xcsrpad
 */


#include "FIXS_TREFC_AlarmHandler.h"
#include "FIXS_TREFC_Manager.h"

//#include "FIXS_TREFC_UpgradeManager.h"
TREFC_Util::DWORD SIXTY_MINUTES = 3600000;
TREFC_Util::DWORD Safe_Timeout = 120000;
// Class FIXS_TREFC_AlarmHandler 

FIXS_TREFC_AlarmHandler::FIXS_TREFC_AlarmHandler()
  :  m_isAlarmTimerOn(false),m_running(false),
      m_stopEvent(0),m_shutdownEvent(0),m_exit(false)
{
	traceObj=new ACS_TRA_trace("FIXS_TREFC_AlarmHandler");
	m_stopEvent = FIXS_TREFC_Event::CreateEvent(false,false,0);
	initOnStart();
}


FIXS_TREFC_AlarmHandler::~FIXS_TREFC_AlarmHandler()
{
	if (m_stopEvent)
	{
		::close(m_stopEvent);
	}
	
	delete (traceObj);
}



//## Other Operations (implementation)
 int FIXS_TREFC_AlarmHandler::open (void *args)
 {
	 int result = 0;

	 UNUSED(args);
	 
	 m_exit = false;
	 cout << __FUNCTION__ << "@" << __LINE__ << " FIXS_TREFC_AlarmHandler try to activate..." << endl;

	 {//log
		 char traceChar[512] = {0};
		 snprintf(traceChar, sizeof(traceChar) - 1,"FIXS_TREFC_AlarmHandler thread started on instance [%p]. \n",this);
		 if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,traceChar);
	 }

	 activate();

	
	 return result;
 }
 int FIXS_TREFC_AlarmHandler::svc()
{
	bool result;
	const int WAIT_FAILED=-1;
	const int WAIT_TIMEOUT= -2;
	const int WAIT_OBJECT_0=0;
	
	const int Shutdown_Event_Index=0;
	const int StopEvent_Index=1;
	const int Number_Of_Alarm_Events=2;
	if (m_exit) return result;

	//loop
	EventHandle handleArr[Number_Of_Alarm_Events] = {m_shutdownEvent, m_stopEvent};
	m_running = true;	
	while ( !m_exit ) // Set to true when the thread shall exit
	{
		int returnValue = FIXS_TREFC_Event::WaitForEvents(Number_Of_Alarm_Events, handleArr, SIXTY_MINUTES);

		
		switch (returnValue)
		{
			case WAIT_FAILED:   // Event failed
			{
				if (traceObj->ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Error: Cannot WaitForMultipleObjects", __FUNCTION__, __LINE__);
					traceObj->ACS_TRA_event(1,tmpStr);
				}
				m_exit = true;   // Terminate the thread
				break;
			}
			case WAIT_TIMEOUT:  // Time out
			{
				
				cout << __FUNCTION__ <<" " << __LINE__ << endl;
				m_isAlarmTimerOn = false;
				FIXS_TREFC_Manager::getInstance()->checkAlarmStatus(true);        
				if (traceObj->ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d]Alarm timer  timed out..", __FUNCTION__, __LINE__);
					//if(FIXS_TREFC_logging)  FIXS_TREFC_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
					traceObj->ACS_TRA_event(1,tmpStr);
				}
				m_exit = true;   // Terminate the thread
				break;
			}
			case (WAIT_OBJECT_0 + Shutdown_Event_Index):  // Shutdown event
			{
				if (traceObj->ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Recieved a shutdown event", __FUNCTION__, __LINE__);					
					traceObj->ACS_TRA_event(1,tmpStr);
				}
				m_exit = true;      // Terminate the thread
				 break;
			}
			
			case (WAIT_OBJECT_0 + StopEvent_Index):  // Stop event
			{
				FIXS_TREFC_Event::ResetEvent(m_stopEvent);
				if (traceObj->ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "FIXS_TREFC_AlarmHandler [%s@%d] Recieved a stop event", __FUNCTION__, __LINE__);
					traceObj->ACS_TRA_event(1,tmpStr);
				}
				m_exit = true;// Stop the thread
				break;
			}
			default:
			{
				if (traceObj->ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1,"[%s@%d] Error: Unexpected event signaled.", __FUNCTION__, __LINE__);
					traceObj->ACS_TRA_event(1,tmpStr);
				}

			  break;
			}
		} // End of switch
	}//while
	
	m_running = false;

	
	return 0;
}

void FIXS_TREFC_AlarmHandler::initOnStart ()
{
	//shutdown event
	 if (m_shutdownEvent == 0)
	 {
		 m_shutdownEvent=FIXS_TREFC_Event::OpenNamedEvent(TREFC_Util::EVENT_NAME_SHUTDOWN);
		 if(m_shutdownEvent == FIXS_TREFC_Event_RC_ERROR)
		 {
			 if (traceObj->ACS_TRA_ON())
			 {
				 char traceChar[512] = {0};
				 snprintf(traceChar, sizeof(traceChar) - 1,"[%s@%d] error creating shutdown event. \n", __FUNCTION__, __LINE__);			
				 traceObj->ACS_TRA_event(1,traceChar);
			 }
		 }
	 }	  
}

 void FIXS_TREFC_AlarmHandler::startAlarmTimer ()
{
	cout << __FUNCTION__ <<" " << __LINE__ << endl;
	m_isAlarmTimerOn = true;
	open();	
}

 void FIXS_TREFC_AlarmHandler::cancelAlarmTimer ()
{
	m_isAlarmTimerOn = false;
	
	if (m_stopEvent)
	{
		int retries=0;
		cout << __FUNCTION__ <<" " << __LINE__ << endl;
		while (FIXS_TREFC_Event::SetEvent(m_stopEvent) == 0)
		{
			cout << "Set StopEvent.... retries: " << retries << endl;
			if( ++retries > 10) break;
			sleep(1);
		}

		if (retries>10)
		{
			if (traceObj->ACS_TRA_ON())
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Error: Cannot signal Stop event.", __FUNCTION__, __LINE__);			
				traceObj->ACS_TRA_event(1,tmpStr);
			}
		}
	}

	cout << "\n#################################################################" << endl;
	cout << " 	   ALARM TIMER stopped !!! "	 << endl;
	cout << "#################################################################\n" << endl;	
}
void FIXS_TREFC_AlarmHandler::stop ()
{
	m_exit = true;
	if (m_stopEvent)
	{
		int retries=0;
		cout << __FUNCTION__ <<" " << __LINE__ << endl;
		while (FIXS_TREFC_Event::SetEvent(m_stopEvent) == 0)
		{
			cout << "Set StopEvent in AlarmHandler.... retries: " << retries << endl;
			if( ++retries > 10) break;
			sleep(1);
		}

		if (retries>10)
		{
			if (traceObj->ACS_TRA_ON())
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Error: Cannot signal Stop event.", __FUNCTION__, __LINE__);				
				traceObj->ACS_TRA_event(1,tmpStr);
			}
		}
	}
	FIXS_TREFC_Event::CloseEvent(m_stopEvent);
}
