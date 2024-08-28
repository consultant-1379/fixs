/*
 * FIXS_CCH_AlarmHandler.cpp
 *
 *  Created on: Nov 15, 2012
 *      Author: xcsrpad
 */

#include "FIXS_CCH_DiskHandler.h"
#include "FIXS_CCH_AlarmHandler.h"


//#include "FIXS_CCH_UpgradeManager.h"
DWORD SIXTY_MINUTES = 3600000;
DWORD Safe_Timeout = 120000;
// Class FIXS_CCH_AlarmHandler 

FIXS_CCH_AlarmHandler::FIXS_CCH_AlarmHandler()
  :  m_cmxTrapTimer(0),m_egem2L2SwitchTrapTimer(0),m_isEgem2L2SwitchTimerOn(false),
      m_isCmxTimerOn(false),m_running(false),m_egem2L2SwitchAlarmTimeoutEvent(0),
      m_cmxAlarmTimeoutEvent(0),m_stopEvent(0),m_shutdownEvent(0),m_exit(false)
{
	traceObj=new ACS_TRA_trace("FIXS_CCH_AlarmHandler");
	m_stopEvent = FIXS_CCH_Event::CreateEvent(false,false,0);
	FIXS_CCH_logging = FIXS_CCH_DiskHandler::getLogInstance();

	initOnStart();
}


FIXS_CCH_AlarmHandler::~FIXS_CCH_AlarmHandler()
{
	if (m_stopEvent)
	{
		::close(m_stopEvent);
	}
	if ( m_cmxAlarmTimeoutEvent && (m_cmxAlarmTimeoutEvent != -1) )
	{
		//CancelWaitableTimer(m_cmxAlarmTimeoutEvent);// do this even if the timer has not started
		//(void) CloseHandle(m_cmxAlarmTimeoutEvent);
		::close(m_cmxAlarmTimeoutEvent);
	}
	if ( m_egem2L2SwitchAlarmTimeoutEvent && (m_egem2L2SwitchAlarmTimeoutEvent != -1) )
	{
		//CancelWaitableTimer(m_scxAlarmTimeoutEvent);// do this even if the timer has not started
		//(void) CloseHandle(m_scxAlarmTimeoutEvent);
		::close(m_egem2L2SwitchAlarmTimeoutEvent);
	}
	delete (traceObj);

	FIXS_CCH_logging = 0;
}



//## Other Operations (implementation)
 int FIXS_CCH_AlarmHandler::open (void *args)
 {
	 int result = 0;

	 UNUSED(args);
	 
	 m_exit = false;
	 cout << __FUNCTION__ << "@" << __LINE__ << " FIXS_CCH_AlarmHandler try to activate..." << endl;

	 {//log
		 char traceChar[512] = {0};
		 snprintf(traceChar, sizeof(traceChar) - 1,"FIXS_CCH_AlarmHandler thread started on instance [%p]. \n",this);
		 if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_DEBUG);
		 if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,traceChar);
	 }

	 activate();

	
	 return result;
 }
 int FIXS_CCH_AlarmHandler::svc()
{
	bool result;
        const int WAIT_FAILED=-1;
	const int WAIT_TIMEOUT= -2;
	const int WAIT_OBJECT_0=0;
	
	 const int Shutdown_Event_Index=0;
	 const int Egem2L2SwitchAlarm_Event_Index=1;
	 const int CmxAlarm_Event_Index=2;
	 const int StopEvent_Index=3;
	 const int Number_Of_Alarm_Events=4;
	if (m_exit) return result;

	//loop
	EventHandle handleArr[Number_Of_Alarm_Events] = {m_shutdownEvent,  m_egem2L2SwitchAlarmTimeoutEvent, m_cmxAlarmTimeoutEvent, m_stopEvent};
	
	while ( !m_exit ) // Set to true when the thread shall exit
	{
		//wait for 2s
		int returnValue = FIXS_CCH_Event::WaitForEvents(Number_Of_Alarm_Events, handleArr, Safe_Timeout);

		
		switch (returnValue)
		{
			case WAIT_FAILED:   // Event failed
			{
				if (traceObj->ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Error: Cannot WaitForMultipleObjects", __FUNCTION__, __LINE__);
					if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
					traceObj->ACS_TRA_event(1,tmpStr);
				}
				m_exit = true;   // Terminate the thread
				break;
			}
			case WAIT_TIMEOUT:  // Time out
			{
				if (traceObj->ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Waiting for events...", __FUNCTION__, __LINE__);
					traceObj->ACS_TRA_event(1,tmpStr);
				}
				break;
			}
			case (WAIT_OBJECT_0 + Shutdown_Event_Index):  // Shutdown event
			{
				if (traceObj->ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Recieved a shutdown event", __FUNCTION__, __LINE__);
					if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
					traceObj->ACS_TRA_event(1,tmpStr);
				}
				m_exit = true;      // Terminate the thread
				 //FIXS_CCH_Event::ResetEvent(m_shutdownEvent);
				 break;
			}
			case (WAIT_OBJECT_0 + Egem2L2SwitchAlarm_Event_Index):   // Scx timer timed out
			{
				m_isEgem2L2SwitchTimerOn = false;
				
				environment = FIXS_CCH_DiskHandler::getEnvironment();
				if ((environment == CCH_Util::SINGLECP_SMX) || (environment == CCH_Util::MULTIPLECP_SMX))
				{
					FIXS_CCH_UpgradeManager::getInstance()->checkSMXswVr(CCH_Util::SMX_ALARM_TIMEOUT);
				}
				else{
					FIXS_CCH_UpgradeManager::getInstance()->checkAllSCXandCMXSwVer(CCH_Util::SCXB, CCH_Util::CMXB, CCH_Util::SCX_ALARM_TIMEOUT);
				}
				if (traceObj->ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Scx or Smx alignment event timed out", __FUNCTION__, __LINE__);
					if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_WARN);
					traceObj->ACS_TRA_event(1,tmpStr);
				}
			 
				FIXS_CCH_Event::ResetEvent(m_egem2L2SwitchAlarmTimeoutEvent);
				cancelEgem2L2SwitchAlarmTimer();							
				   break;
			}
			case (WAIT_OBJECT_0 + CmxAlarm_Event_Index):    // Cmx timer timed out
			{
				m_isCmxTimerOn = false;
				FIXS_CCH_UpgradeManager::getInstance()->checkAllSCXandCMXSwVer(CCH_Util::SCXB, CCH_Util::CMXB, CCH_Util::CMX_ALARM_TIMEOUT);//rAJI
             
				if (traceObj->ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Cmx alignment event timed out", __FUNCTION__, __LINE__);
					if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_WARN);
					traceObj->ACS_TRA_event(1,tmpStr);
				}
			 
				FIXS_CCH_Event::ResetEvent(m_cmxAlarmTimeoutEvent);
				cancelCmxAlarmTimer();
				break;

			}
			case (WAIT_OBJECT_0 + StopEvent_Index):  // Stop event
			{
				FIXS_CCH_Event::ResetEvent(m_stopEvent);
				if (traceObj->ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "FIXS_CCH_AlarmHandler [%s@%d] Recieved a stop event", __FUNCTION__, __LINE__);
					if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_WARN);
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
					if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
					traceObj->ACS_TRA_event(1,tmpStr);
				}

			  break;
			}
		} // End of switch
	}//while
	
	m_running = false;
	resetEvents();
	
	return 0;
}

 void FIXS_CCH_AlarmHandler::stop ()
{
	m_exit = true;
	if (m_stopEvent)
	{
		int retries=0;
		cout << __FUNCTION__ <<" " << __LINE__ << endl;
		while (FIXS_CCH_Event::SetEvent(m_stopEvent) == 0)
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
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
				traceObj->ACS_TRA_event(1,tmpStr);
			}
		}
	}
	if (m_cmxTrapTimer!=NULL)
	 {
		 if (m_cmxTrapTimer->isRunningThread())
	     	 m_cmxTrapTimer->stopTimer();
	 }
	 int retry = 0;
	 if (m_cmxTrapTimer!=NULL)
	 {
		 while (m_cmxTrapTimer->isRunningThread() && retry < 5){
			 cout<< " cmxTrapTimer THREAD is still running " << endl;
			 sleep(1);
			 retry++;
		 }
		 m_cmxTrapTimer->closeHandle();
		 delete (m_cmxTrapTimer);
		 m_cmxTrapTimer = NULL;
	 }
	 retry = 0;
	 if (m_egem2L2SwitchTrapTimer!=NULL)
         {
                 if (m_egem2L2SwitchTrapTimer->isRunningThread())
                 m_egem2L2SwitchTrapTimer->stopTimer();
         }

	 if (m_egem2L2SwitchTrapTimer)
	 {
		 while (m_egem2L2SwitchTrapTimer->isRunningThread() && retry < 5){
			 cout<< " scx/smx TrapTimer THREAD is still running " << endl;
			 sleep(1);
			 retry++;
		 }
		 m_egem2L2SwitchTrapTimer->closeHandle();
		 delete (m_egem2L2SwitchTrapTimer);
		 m_egem2L2SwitchTrapTimer = NULL;
	 }
	
}

 void FIXS_CCH_AlarmHandler::initOnStart ()
{
	 m_egem2L2SwitchTrapTimer = new FIXS_CCH_TrapTimer();
	 m_cmxTrapTimer = new FIXS_CCH_TrapTimer();
	//shutdown event
	 if (m_shutdownEvent == 0)
	 {
		 m_shutdownEvent=FIXS_CCH_Event::OpenNamedEvent(CCH_Util::EVENT_NAME_SHUTDOWN);
		 if(m_shutdownEvent == FIXS_CCH_Event_RC_ERROR)
		 {
			 if (traceObj->ACS_TRA_ON())
			 {
				 char traceChar[512] = {0};
				 snprintf(traceChar, sizeof(traceChar) - 1,"[%s@%d] error creating shutdown event. \n", __FUNCTION__, __LINE__);
				 if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_ERROR);
				 traceObj->ACS_TRA_event(1,traceChar);
			 }
		 }
	 }
	//initialize timeout events
	  m_egem2L2SwitchAlarmTimeoutEvent= m_egem2L2SwitchTrapTimer->getEvent();
	  {
		 char traceChar[512] = {0};
		 snprintf(traceChar, sizeof(traceChar) - 1,"[%s@%d] get Event for Scx/Smx Alarm time out[%d]. \n", __FUNCTION__, __LINE__,m_egem2L2SwitchAlarmTimeoutEvent);
		 if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_DEBUG);
		 if (traceObj->ACS_TRA_ON()) traceObj->ACS_TRA_event(1,traceChar);
	 }
	  m_cmxAlarmTimeoutEvent= m_cmxTrapTimer->getEvent();
	  {
		 char traceChar[512] = {0};
		 snprintf(traceChar, sizeof(traceChar) - 1,"[%s@%d] get Event for CmxAlarm time out[%d]. \n", __FUNCTION__, __LINE__,m_cmxAlarmTimeoutEvent);
		 if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_DEBUG);
		 if (traceObj->ACS_TRA_ON()) traceObj->ACS_TRA_event(1,traceChar);
	 }
}

 void FIXS_CCH_AlarmHandler::startEgem2L2SwitchAlarmTimer ()
{
	if(m_egem2L2SwitchTrapTimer!=NULL)
	{
		m_egem2L2SwitchTrapTimer->setTrapTime(SIXTY_MINUTES);
		m_egem2L2SwitchTrapTimer->open();
 	
		if(m_egem2L2SwitchAlarmTimeoutEvent == -1)
		 {
			if (traceObj->ACS_TRA_ON())
		 	{
			 char tmpStr[512] = {0};
			 snprintf(tmpStr, sizeof(tmpStr) - 1,"[%s@%d] Synchronization ERROR: cannot set WaitableTimer.", __FUNCTION__, __LINE__);
			 if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
			 traceObj->ACS_TRA_event(1,tmpStr);
		 	}
		 
	 	}
		else
		{
			m_isEgem2L2SwitchTimerOn = true;
		}
	}
}

 void FIXS_CCH_AlarmHandler::cancelEgem2L2SwitchAlarmTimer ()
{

	 if ((m_egem2L2SwitchAlarmTimeoutEvent == -1) || (m_egem2L2SwitchAlarmTimeoutEvent == 0))
	 {
		 if (traceObj->ACS_TRA_ON())
		 {
			 char traceChar[512] = {0};
			 snprintf(traceChar, sizeof(traceChar) - 1,"[%s@%d] m_egem2L2SwitchAlarmTimeoutEvent OFF ", __FUNCTION__, __LINE__);
			 if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_DEBUG);
			 traceObj->ACS_TRA_event(1,traceChar);
		 }
     	}
	 else 
	 {
		if(m_egem2L2SwitchTrapTimer!=NULL)
		{
			 m_egem2L2SwitchTrapTimer->stopTimer();
		 	if (traceObj->ACS_TRA_ON())
		 	{
			 char traceChar[512] = {0};
			 snprintf(traceChar, sizeof(traceChar) - 1,"[%s@%d] m_egem2L2SwitchAlarmTimeoutEvent stopped !", __FUNCTION__, __LINE__);
			 if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_DEBUG);
			 traceObj->ACS_TRA_event(1,traceChar);
		 	}

			m_isEgem2L2SwitchTimerOn = false;
	 	}	 
	}
}

void FIXS_CCH_AlarmHandler::startCmxAlarmTimer ()
{
	if(m_cmxTrapTimer!=NULL)
	{
    		m_cmxTrapTimer->setTrapTime(SIXTY_MINUTES);
		m_cmxTrapTimer->open();
	
 	
		if(m_cmxAlarmTimeoutEvent == -1)
	 	{
			 if (traceObj->ACS_TRA_ON())
		 	{
			 char tmpStr[512] = {0};
			 snprintf(tmpStr, sizeof(tmpStr) - 1,"[%s@%d] Synchronization ERROR: cannot set WaitableTimer.", __FUNCTION__, __LINE__);
			 if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
			 traceObj->ACS_TRA_event(1,tmpStr);
		 	}
		 
		 }
		else
		{
			m_isCmxTimerOn = true;
		}
	}
}
void FIXS_CCH_AlarmHandler::cancelCmxAlarmTimer ()
{  
	
	if ((m_cmxAlarmTimeoutEvent == -1) || (m_cmxAlarmTimeoutEvent == 0))
	 {
		 if (traceObj->ACS_TRA_ON())
		 {
			 char traceChar[512] = {0};
			 snprintf(traceChar, sizeof(traceChar) - 1,"[%s@%d] m_cmxAlarmTimeoutEvent OFF ", __FUNCTION__, __LINE__);
			 if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_DEBUG);
			 traceObj->ACS_TRA_event(1,traceChar);
		 }
	 }
	 else 
	 {
		if(m_cmxTrapTimer!=NULL)
		{
			 m_cmxTrapTimer->stopTimer();
			 if (traceObj->ACS_TRA_ON())
			 {
			 char traceChar[512] = {0};
			 snprintf(traceChar, sizeof(traceChar) - 1,"[%s@%d] m_cmxAlarmTimeoutEvent stopped !", __FUNCTION__, __LINE__);
			 if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_DEBUG);
			 traceObj->ACS_TRA_event(1,traceChar);
		 	}

			m_isCmxTimerOn = false;
	 	}
	}	 
}
void FIXS_CCH_AlarmHandler::resetEvents()
{
	FIXS_CCH_Event::ResetEvent(m_stopEvent);

	FIXS_CCH_Event::ResetEvent(m_cmxAlarmTimeoutEvent);
	FIXS_CCH_Event::ResetEvent(m_egem2L2SwitchAlarmTimeoutEvent);
}
