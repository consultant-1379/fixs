/*
 * FIXS_CCH_JTPHandler.cpp
 *
 *  Created on: Oct 20, 2011
 *      Author: eiolbel
 */

// FIXS_CCH_JTPHandler
#include "FIXS_CCH_JTPHandler.h"

using namespace std;
namespace {

ACE_thread_mutex_t s_cs;

struct Lock
	   {
	         Lock()
	         {
					ACE_OS::thread_mutex_trylock(&s_cs);
	         };

	         ~Lock()
	         {

				ACE_OS::thread_mutex_unlock(&s_cs);
	         };
	   };
}


namespace {

	const int VERSION1 = 1;
	const int VERSION2 = 2;
	enum JTP_Event_Indexes
	{
		JTP_Shutdown_Event_Index = 0,
		JTP_IMBSW_START_Index = 1,   // JTP window from client
		JTP_IMBSW_END_Index = 2,   // JTP window from client
		JTP_IPMI_START_Index = 3,   // JTP window from client
		JTP_IPMI_END_Index = 4,   // JTP window from client
		JTP_POWER_START_Index = 5,   // JTP window from client
		JTP_POWER_END_Index = 6,   // JTP window from client
		JTP_FAN_START_Index = 7,   // JTP window from client
		JTP_FAN_END_Index = 8,   // JTP window from client
		JTP_IMBUPG_START_REQ_index = 9, //JTP Autorization request
		JTP_Number_Of_Events = 10
   };

	//DWORD JTP_Safe_Timeout = 120000;

	static const char EVENT_NAME_SHUTDOWN[]    = "0";
	static const char EVENT_NAME_IMBSW_START[]    = "1";
	static const char EVENT_NAME_IMBSW_END[]    = "2";
	static const char EVENT_NAME_IPMI_START[]    = "3";
	static const char EVENT_NAME_IPMI_END[]    = "4";
	static const char EVENT_NAME_POWER_START[]    = "5";
	static const char EVENT_NAME_POWER_END[]    = "6";
	static const char EVENT_NAME_FAN_START[]    = "7";
	static const char EVENT_NAME_FAN_END[]    = "8";
	static const char EVENT_NAME_START_REQ[]    = "9";




	const int JTP_DELAY_TIME = 60; // Time delay between connect tries.
	const DWORD MILLISEC_DELAY_TIME = JTP_DELAY_TIME * 1000;

	//add other variable to handle the event
	const int WAIT_FAILED = -1;
	const int WAIT_TIMEOUT = -2;
	const int WAIT_OBJECT_0 = 0;
	const int Shutdown_Event_Index = 0;
	const int Command_Event_Index = 1;
	const int TimeoutTrap_Event_Index = 2;
	const int Stop_Event_Index = 3;


	// if you want three tries, you must set to four.
	const int JTP_START_MAX_RETRY = 4; // if set to one no tries are performed.
	const int JTP_END_MAX_RETRY = 4; // if set to zero, infinite tries are performed.
	const int JTP_START_REQUEST_RETRY = 1; //no retry performed for start request


}
#ifndef __
#define __ std::cout << __FUNCTION__ << "@" << __LINE__ << std::endl;
#endif

// Class FIXS_CCH_JTPHandler

FIXS_CCH_JTPHandler::FIXS_CCH_JTPHandler (unsigned char shelf, unsigned char slot, int id)
      : m_exit(false),
        m_shutdownEvent(0),
        m_StopEvent(0),
        m_waitHandles(NULL),
        m_startReqEvent(0),
        m_shelf(0),
        m_slot(0),
        m_stopRetry(NULL),
        m_windowIsOpen(NULL),
        m_cpId(id),
        m_startReqStatus(false)
        //m_startReqEvent(0)
{

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " +++ JTPHANDLER CONSTRUCTOR +++ IN"<< std::endl;
	// Initialize the svc state flag
	svc_run = false;

	_trace = new ACS_TRA_trace("FIXS_CCH_JTPHandler");

	FIXS_CCH_logging = FIXS_CCH_DiskHandler::getLogInstance();

	Lock lock;

	m_waitHandles = new FIXS_CCH_EventHandle[JTP_Number_Of_Events];

	m_shutdownEvent =  FIXS_CCH_Event::OpenNamedEvent(CCH_Util::EVENT_NAME_SHUTDOWN);
	m_waitHandles[0] = m_shutdownEvent;
	m_startReqEvent = FIXS_CCH_Event::CreateEvent(0,false,0);

	char EVENT_IMBSW_START[512] = {0};
	snprintf(EVENT_IMBSW_START, sizeof(EVENT_IMBSW_START) - 1, "FIXS_CCH_JTPHandler%s%p", EVENT_NAME_IMBSW_START,this);
	m_waitHandles[1] = FIXS_CCH_Event::CreateEvent(0, false, EVENT_IMBSW_START);

	char EVENT_IMBSW_END[512] = {0};
	snprintf(EVENT_IMBSW_END, sizeof(EVENT_IMBSW_END) - 1, "FIXS_CCH_JTPHandler%s%p", EVENT_NAME_IMBSW_END,this);
	m_waitHandles[2] = FIXS_CCH_Event::CreateEvent(0, false, EVENT_IMBSW_END);

	char EVENT_IPMI_START[512] = {0};
	snprintf(EVENT_IPMI_START, sizeof(EVENT_IPMI_START) - 1, "FIXS_CCH_JTPHandler%s%p", EVENT_NAME_IPMI_START,this);
	m_waitHandles[3] = FIXS_CCH_Event::CreateEvent(0, false, EVENT_IPMI_START);

	char EVENT_IPMI_END[512] = {0};
	snprintf(EVENT_IPMI_END, sizeof(EVENT_IPMI_END) - 1, "FIXS_CCH_JTPHandler%s%p", EVENT_NAME_IPMI_END,this);
	m_waitHandles[4] = FIXS_CCH_Event::CreateEvent(0, false, EVENT_IPMI_END);

	char EVENT_POWER_START[512] = {0};
	snprintf(EVENT_POWER_START, sizeof(EVENT_POWER_START) - 1, "FIXS_CCH_JTPHandler%s%p", EVENT_NAME_POWER_START,this);
	m_waitHandles[5] = FIXS_CCH_Event::CreateEvent(0, false, EVENT_POWER_START);

	char EVENT_POWER_END[512] = {0};
	snprintf(EVENT_POWER_END, sizeof(EVENT_POWER_END) - 1, "FIXS_CCH_JTPHandler%s%p", EVENT_NAME_POWER_END,this);
	m_waitHandles[6] = FIXS_CCH_Event::CreateEvent(0, false, EVENT_POWER_END);

	char EVENT_FAN_START[512] = {0};
	snprintf(EVENT_FAN_START, sizeof(EVENT_FAN_START) - 1, "FIXS_CCH_JTPHandler%s%p", EVENT_NAME_FAN_START,this);
	m_waitHandles[7] = FIXS_CCH_Event::CreateEvent(0, false, EVENT_FAN_START);

	char EVENT_FAN_END[512] = {0};
	snprintf(EVENT_FAN_END, sizeof(EVENT_FAN_END) - 1, "FIXS_CCH_JTPHandler%s%p", EVENT_NAME_FAN_END,this);
	m_waitHandles[8] = FIXS_CCH_Event::CreateEvent(0, false, EVENT_FAN_END);

	char EVENT_START_REQ[512] = {0};
	snprintf(EVENT_START_REQ, sizeof(EVENT_START_REQ) - 1, "FIXS_CCH_JTPHandler%s%p", EVENT_NAME_START_REQ,this);
	m_waitHandles[9] = FIXS_CCH_Event::CreateEvent(0, false, EVENT_START_REQ);

	m_stopRetry = new bool[JTP_Number_Of_Events];
	for (int i=1; i < JTP_Number_Of_Events; i++) {
		m_stopRetry[i] = false;
	}
	m_windowIsOpen = new bool[JTP_Number_Of_Events];
	for (int i=1; i < JTP_Number_Of_Events; i++) {
		m_windowIsOpen[i] = false;
	}
	this->m_shelf = shelf;
	this->m_slot = slot;

	m_running = false;

	sendingNotification = false;

	//m_startReqEvent = FIXS_CCH_Event::CreateEvent(false,false,0);

	m_StopEvent = FIXS_CCH_Event::CreateEvent(false,false,0);

	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "FIXS_CCH_JTPHandler on MAG: %x SLOT: %x CPid: %d on instance [%p]", m_shelf,m_slot,m_cpId,this);
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
		if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1,tmpStr);
	}

}


FIXS_CCH_JTPHandler::~FIXS_CCH_JTPHandler()
{
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " +++ JTPHANDLER DESTRUCTOR +++ IN"<< std::endl;
//	Lock lock;
	for (int i=1; i < JTP_Number_Of_Events; i++)
	{
		if (m_waitHandles[i])
			FIXS_CCH_Event::CloseEvent(m_waitHandles[i]);
	}

	if (m_StopEvent) FIXS_CCH_Event::CloseEvent(m_StopEvent);

	if (m_startReqEvent)
	{
		FIXS_CCH_Event::CloseEvent(m_startReqEvent);
		m_startReqEvent = 0;
	}

	delete [] m_waitHandles;
	m_waitHandles = NULL;

	delete (_trace);

	if (m_stopRetry) {
		delete[] m_stopRetry;
		m_stopRetry = NULL;
	}

	if (m_windowIsOpen)
	{
		delete[] m_windowIsOpen;
		m_windowIsOpen = NULL;
	}

	FIXS_CCH_logging = 0;

}

 void FIXS_CCH_JTPHandler::boardRestarted (unsigned char msgType)
{
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " +++ BOARDRESTARTED +++ IN"<< std::endl;
	if (msgType == IMBSW_START_UPGRADE) m_stopRetry[JTP_IMBSW_START_Index] = true;
	else if (msgType == IPMI_START_UPGRADE) m_stopRetry[JTP_IPMI_START_Index] = true;
	else if (msgType == POWER_START_UPGRADE) m_stopRetry[JTP_POWER_START_Index] = true;
	else if (msgType == FAN_START_UPGRADE) m_stopRetry[JTP_FAN_START_Index] = true;

}

 void FIXS_CCH_JTPHandler::maintenanceWindow (unsigned char msgType)
{
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " +++ MAINTENANCEWINDOW +++ IN"<< std::endl;
	printf("DBG: [%s@%d] - ++++++++SLOT: %x \n", __FUNCTION__, __LINE__, m_slot);


	std::string tmp("");
//	char tmpStr[512] = {0};
	int event_index = -1;

	if (msgType == IMBSW_START_UPGRADE) { event_index = JTP_IMBSW_START_Index; tmp = "IMBSW_START_UPGRADE"; }
	else if (msgType == IMBSW_END_UPGRADE) { event_index = JTP_IMBSW_END_Index; tmp = "IMBSW_END_UPGRADE"; }
	else if (msgType == IPMI_START_UPGRADE) { event_index = JTP_IPMI_START_Index; tmp = "IPMI_START_UPGRADE"; }
	else if (msgType == IPMI_END_UPGRADE) { event_index = JTP_IPMI_END_Index; tmp = "IPMI_END_UPGRADE"; }
	else if (msgType == POWER_START_UPGRADE) { event_index = JTP_POWER_START_Index; tmp = "POWER_START_UPGRADE"; }
	else if (msgType == POWER_END_UPGRADE) { event_index = JTP_POWER_END_Index; tmp = "POWER_END_UPGRADE"; }
	else if (msgType == FAN_START_UPGRADE) { event_index = JTP_FAN_START_Index; tmp = "FAN_START_UPGRADE"; }
	else if (msgType == FAN_END_UPGRADE) { event_index = JTP_FAN_END_Index; tmp = "FAN_END_UPGRADE"; }
	else if (msgType == IMB_UPG_START_REQ) { event_index = JTP_IMBUPG_START_REQ_index; tmp = "IMB_START_REQ_UPGRADE"; }


		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "Notification for %s on MAG: %x SLOT: %x", tmp.c_str(),m_shelf,m_slot);
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
			if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1,tmpStr);
		}

	if (event_index >= 0 && m_waitHandles[event_index] != 0)
	{
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "Notification event is signalled on instance[%p]",this);
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
			if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1,tmpStr);
		}
		FIXS_CCH_Event::SetEvent(m_waitHandles[event_index]);
	}
	else
	{
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "Notification event is not created again.");
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
				if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1,tmpStr);
			}
	}

}

 bool FIXS_CCH_JTPHandler::sendNotification (unsigned char msgType, int event_index, const int noTries)
{

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " +++ SENDNOTIFICATION +++ IN"<< std::endl;
	bool sendOk = false;
	int tempVersion = VERSION1;

	if( (msgType == IMBSW_START_UPGRADE) || (msgType == IMBSW_END_UPGRADE) ||
		(msgType == IPMI_START_UPGRADE) || (msgType == IPMI_END_UPGRADE) ||
		(msgType == IMB_UPG_START_REQ))
	{
		tempVersion = VERSION2;
	}
	// version is User Data 1 (MSB)  0x01
	// msgType is User Data 1 (LSB)  example 0x1E Power FW Upgrade End. See enum JOB_REQUEST_CODE
	// shelf is User Data 2 (MSB)  0x00 to 0x1F
	// slot is User Data 2 (LSB)   0x00 (slot 0), 0x19 (slot 25), 0x1A (slot 26) or 0x1C (slot 28)
	printf("DBG: [%s@%d] -  +++++SEND NOTIFICATION SLOT: %x \n", __FUNCTION__, __LINE__, this->m_slot);
	if (this->m_slot == 0x00 || this->m_slot == 0x19 || this->m_slot == 0x1A || this->m_slot == 0x1C)  // slot 0 is 0x00, slot 25 is the decimal of 0x19, slot 26 is decimal of 0x1A, slot 28 is decimal of 0x1C
	{
		sendingNotification = true;
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
		printf("DBG: [%s@%d] - SLOT: %x \n", __FUNCTION__, __LINE__, m_slot);
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[MAG: %x  SLOT: %x ] ...before handleNotification call.....tempversion = %d, noTries = %d",m_shelf,m_slot, tempVersion, noTries );
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
		}
		int exitCode = this->handleNotification(tempVersion, msgType, this->m_shelf, this->m_slot, noTries);

		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "EXITCODE: " << exitCode <<std::endl;
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[MAG: %x  SLOT: %x ]++++++++++++++++++HANDLE NOTIFICATION EXIT CODE+++++++++++++",m_shelf,m_slot);
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
		}

		int retryForConf = 1;
		int downgradeVersion = tempVersion;
		while (sendingNotification)
		{
			if (exitCode == 0)
			{
				std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[MAG: %x  SLOT: %x ]Exit code for handleNotification is OK.",m_shelf,m_slot);
					if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
					if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1,tmpStr);
				}

				sendOk = true;
				sendingNotification = false;
			}
			else if (exitCode == FIXS_CCH_JTPClient::JOB_REJECTED) //Not Confermation or jinitreq failed
			{
				std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
				if (msgType == IMB_UPG_START_REQ)
				{
					std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "No retry in case upgrade start request rejected" << std::endl;
					sendingNotification = false;
					break;
				}

				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[MAG: %x  SLOT: %x ] retry to send CP request after FIXS_CCH_JTPClient::JOB rejected or failed jinitreq..",m_shelf,m_slot);
					if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
					if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1,tmpStr);
				}
				if(noTries <= retryForConf)
				{
					sendingNotification = false;
                                        break;
				}

				bool stopRetry = false;
//				sleep(JTP_DELAY_TIME);
				FIXS_CCH_EventHandle HandlesArray[15];
				int validIndex = 0;

				if (m_waitHandles != NULL) {
					for (int i=0; i < JTP_Number_Of_Events; i++)
					{
						if (m_waitHandles[i] != -1)
						{
							HandlesArray[validIndex++] = m_waitHandles[i];
						}
					}
				}
				HandlesArray[validIndex++] = m_StopEvent;

				//DWORD returnValue = WaitForMultipleObjects(validIndex, HandlesArray, FALSE, MILLISEC_DELAY_TIME);
				int returnValue = FIXS_CCH_Event::WaitForEvents(validIndex,HandlesArray, MILLISEC_DELAY_TIME);

				if (returnValue ==  (WAIT_OBJECT_0 + JTP_IMBSW_END_Index)) stopRetry = true;
				else if (returnValue == (WAIT_OBJECT_0 + JTP_IPMI_END_Index)) stopRetry = true;
				else if (returnValue == (WAIT_OBJECT_0 + JTP_POWER_END_Index)) stopRetry = true;
				else if (returnValue == (WAIT_OBJECT_0 + JTP_FAN_END_Index)) stopRetry = true;
				else if (returnValue == (WAIT_OBJECT_0 + validIndex -1))
				{
					sendingNotification = false;
				//	stop();
					break;
				}
				if (returnValue !=  WAIT_TIMEOUT) {
					std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
					FIXS_CCH_Event::SetEvent(HandlesArray[returnValue - WAIT_OBJECT_0]);
				}

				if (!m_stopRetry[event_index] && !stopRetry)
				{
					std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
					if (noTries < 0) // no retry
					{
						std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
						sendingNotification = false;
					}
					else if (noTries == 0) // Infinite retries
					{
						std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
						exitCode = this->handleNotification(tempVersion, msgType, this->m_shelf, this->m_slot, noTries);
					}
					else if (noTries > retryForConf)
					{
						std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
						{
							char tmpStr[512] = {0};
							snprintf(tmpStr, sizeof(tmpStr) - 1, "Send CP. retry number: %d", retryForConf);
							if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
							if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1,tmpStr);
						}
						exitCode = this->handleNotification(tempVersion, msgType, this->m_shelf, this->m_slot, noTries);
						retryForConf++;
					}
					else
					{// retries are terminated
						std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "**** The retry is terminated ****" << std::endl;
						sendingNotification = false;
					}
				}
				else
				{
					std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " Stop retry. The maintenance window is closed" << std::endl;

					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "Stop retry. The maintenance window will be closed");
						if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
						if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1,tmpStr);
					}
					m_stopRetry[event_index] = false;
					sendingNotification = false;

				}
			}
			else if (exitCode == FIXS_CCH_JTPClient::JOB_FAILED) //Not Result Ind
			{
				std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;

				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "Job failed..");
					if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
					if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1,tmpStr);
				}
				sendingNotification = false;
			}
			else if (exitCode == FIXS_CCH_JTPClient::JOB_MISMATCH) // Protocol version mismatch
			{
				if(msgType == IMB_UPG_START_REQ)
				{
					sendOk = true;
                                        sendingNotification = false;
                                        this->m_startReqStatus = true;

                                        {
                                                char tmpStr[512] = {0};
                                                snprintf(tmpStr, sizeof(tmpStr) - 1, "No retry for start request in case of job mismatch");
                                                if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
                                                if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1,tmpStr);
                                        }
                                        break;	
				}

				if(this->m_slot == 0x1A || this->m_slot == 0x1C)
				{
					sendOk = true;
					sendingNotification = false;
					this->m_startReqStatus = true;

					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "No retry for CMX protocol mismatch");
						if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
						if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1,tmpStr);
					}
					break;
				}

				if (!m_stopRetry[event_index])
				{
					std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
					downgradeVersion--;

					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "retry to send CP request after protocol mismatch. Version is %d", downgradeVersion);
						if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
						if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1,tmpStr);
					}
					exitCode = this->handleNotification(downgradeVersion, msgType, this->m_shelf, this->m_slot, noTries);
					if (downgradeVersion == 0) sendingNotification = false;
				}
				else
				{
					std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " Stop retry after protocol mismatch" << std::endl;

					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "Stop retry protocol mismatch");
						if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
						if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1,tmpStr);
					}

					sendingNotification = false;
				}
			}
			else
			{
				std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;

				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "handleNotification: Wrong exit code");
					if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
					if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1,tmpStr);
				}
				sendingNotification = false;
			}
		}

		if(msgType == IMB_UPG_START_REQ)
		{
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "JTPHANDLER: START REQUEST SIGNALED " << std::endl;
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "Start Request event signaled");
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
				if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1,tmpStr);
			}
			FIXS_CCH_Event::SetEvent(m_startReqEvent);
		}
	}
	else
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;

		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "Wrong slot number. Message type is 0x%X", msgType);
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
			if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1,tmpStr);
		}
	}
//why we stop thread only for the following events?
	if ((event_index == JTP_FAN_END_Index)
		|| (event_index == JTP_POWER_END_Index))
	{
		//stop thread
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " stop thread" << std::endl;
		stop();
	} 

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " OUT" << std::endl;

	return sendOk;
}

 int FIXS_CCH_JTPHandler::handleNotification (int version, unsigned short msgType, unsigned char shelf, unsigned char slot, const int noTries)
{
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " +++ HANDLENOTIFICATION +++ IN"<< std::endl;
	printf("DBG: [%s@%d] - +++++++HANDLE NOTIFICATION SLOT: %x \n", __FUNCTION__, __LINE__, slot);

	UNUSED(noTries);

	char* ServiceName = ((char*)"AMIUPGCP");

	/*
	char *ServiceName;

	if(m_cpId == 1)
		ServiceName = "AMIUPGCP";
	else if(m_cpId == 2)
		ServiceName = "AMIUPGCP";
	*/

	unsigned short Len = 0;
	char *Msg = 0;
	unsigned short U1, U2, resCode;

	//TODO ----test----
	Msg = (char*) new (std::nothrow) char[strlen("")+1];
	Len = 0;
	memset(Msg,0,Len);
	strcpy(Msg,"");
	//TODO ----test----

	U1 = version;
	U1 = U1 << 8;
	U1 = U1 | msgType;

	U2 = shelf;
	U2 = U2 << 8;
	U2 = U2 | slot;

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "U1="<< U1 <<" U2="<< U2 <<std::endl;
	int exitCode = 0;

	// Create a JTP_Conversation object with name and maxbuffer
	__
	ACS_JTP_Job J1(ServiceName, Len);
	__
	// Override the default timeout which is 10 seconds
	J1.setTimeOut(10);

	// Override the default no of tries for jinitreq
//	J1.setNoOfTries(noTries);

	// Override the default time between retries
//	J1.setDelayTime(JTP_DELAY_TIME);
	__
	FIXS_CCH_JTPClient *jTPClient = new FIXS_CCH_JTPClient(m_waitHandles[JTP_Shutdown_Event_Index]);
	int result = jTPClient->newJTPClient(Len, J1, U1, U2, Msg, m_cpId);
	__
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<" newJTPClient result =  " << result << std::endl;

	if (result == 0)
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
		int confermation = jTPClient->waitForConfermation(J1, U1, U2);

		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " waitForConfermation result =  " << confermation << std::endl;

		if (confermation == 0)
		{

			//U1 = 0x00;
			//U1 = U1 << 8;
			//U1 = U1 | msgType;
			//Len = 0;
			__
			bool waitRes = jTPClient->waitForJTPResult(J1, Len, U1, U2, Msg, resCode);

			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " waitForJTPResult result =  " << waitRes << std::endl;
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " waitForJTPResult resultCode =  " << resCode << std::endl;

			if (!waitRes)
			{
				std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
				if(resCode == 30) // 30 is 0x001E
				{
					std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " Work around for AP-CP communication fault resCode ="<< resCode<< std::endl;
					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "JOB_MISMATCH Work around for AP-CP communication fault resCode");
						if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
						if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1,tmpStr);
					}
					// Protocol version Mismatch.
					exitCode = FIXS_CCH_JTPClient::JOB_MISMATCH;
				}
				else
				{
					exitCode = FIXS_CCH_JTPClient::JOB_FAILED;
					this->m_startReqStatus = false;
					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "JOB_FAILED JTP waitForJTPResult - Unexpected error");
						if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
						if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1,tmpStr);
					}
				}
			}
			else
			{
				if(msgType == IMB_UPG_START_REQ)
				this->m_startReqStatus = true;

				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "JTP waitForJTPResult - SUCCESSFULL");
					if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
					if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1,tmpStr);
				}
			}
		}
		else
		{
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
			exitCode = confermation;

			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "JTP confirmation NOT OK");
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
				if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1,tmpStr);
			}
		}
	}
	else
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
		exitCode = result;

		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "JTP request NOT OK");
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
			if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1,tmpStr);
		}
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " if (request) = false" << std::endl;
	}
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " OUT"<< std::endl;


	delete jTPClient;
	jTPClient = 0;

	//TODO ----test----
	delete[] Msg;
	Msg = 0;
	//TODO ----test----
	return exitCode;
}


int FIXS_CCH_JTPHandler::svc ()
{
	m_running = true;

	sleep(1);

	FIXS_CCH_EventHandle HandlesArray[15];
	int validIndex = 0;

	if (m_waitHandles != NULL) {
		for (int i=0; i < JTP_Number_Of_Events; i++)
		{
			if (m_waitHandles[i] != -1)
			{
				HandlesArray[validIndex++] = m_waitHandles[i];
			}
		}
	}
	HandlesArray[validIndex++] = m_StopEvent;

	//loop
	while (!m_exit) // Set to true when the thread shall exit
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " +++ SVC: WAITFOREVENTS +++ IN"<< std::endl;
		int returnValue = FIXS_CCH_Event::WaitForEvents(validIndex, HandlesArray, 120000);

		if (m_exit) break;

		else if (returnValue ==  WAIT_FAILED)   // Event failed
		{

			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Error: Cannot WaitForMultipleObjects", __FUNCTION__, __LINE__);
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
			if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1,tmpStr);

			m_exit = true;    // Terminate the JTP Handler thread
		}
		else if (returnValue ==  WAIT_TIMEOUT)  // Time out
		{

			//			{
			//				char tmpStr[512] = {0};
			//				snprintf(tmpStr, sizeof(tmpStr) - 1, " JTPHandler is waiting for events on instance [%p]",this);
			//				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
			//				if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1,tmpStr);
			//			}
		}
		else if (returnValue ==  (WAIT_OBJECT_0 + JTP_Shutdown_Event_Index))  // Shutdown event
		{
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Recieved a shutdown event on instance: %p", __FUNCTION__, __LINE__,this);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
				if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1,tmpStr);
			}
			//FIXS_CCH_Event::ResetEvent(m_waitHandles[0]);
			m_exit = true;    // Terminate the thread
		}
		else if (returnValue ==  (WAIT_OBJECT_0 + validIndex-1))  // Stop event
		{

			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Recieved a stop event", __FUNCTION__, __LINE__);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
				if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1,tmpStr);
			}
			FIXS_CCH_Event::ResetEvent(m_StopEvent);
			m_exit = true;    // Terminate the thread
		}
		else if (stateMachine(returnValue - WAIT_OBJECT_0))
		{

			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d MAG: %x SLOT: %x ] an event is signalled", __FUNCTION__, __LINE__,m_shelf,m_slot);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
				if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1,tmpStr);
			}
		}
		else
		{

			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Error: Unexpected event signaled.", __FUNCTION__, __LINE__);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
				if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1,tmpStr);
			}
		}

	} // End of the while loop

	FIXS_CCH_Event::ResetEvent(m_StopEvent);
	m_running = false;
	return 0;

}

 bool FIXS_CCH_JTPHandler::stateMachine (int event_index)
{
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "????????????????? STATE MACHINE ????????????????????????????" << std::endl;

	FIXS_CCH_Event::ResetEvent(m_waitHandles[event_index]);

	int start_index = event_index;
	unsigned char msgType = 0;
	unsigned char msgEndType = 0;
	typedef enum {WINDOW_START = 0, WINDOW_END = 1, WINDOW_NONE = 2 } Window_Event;
	Window_Event eventType;

	if (event_index == JTP_IMBSW_START_Index)
	{
		msgType = IMBSW_START_UPGRADE;
		msgEndType = IMBSW_END_UPGRADE;
		eventType = WINDOW_START;
//		FIXS_CCH_Event::ResetEvent(m_waitHandles[1]);
	}
	else if (event_index == JTP_IPMI_START_Index)
	{
		msgType = IPMI_START_UPGRADE;
		msgEndType = IPMI_END_UPGRADE;
		eventType = WINDOW_START;
//		FIXS_CCH_Event::ResetEvent(m_waitHandles[3]);
	}
	else if (event_index == JTP_POWER_START_Index)
	{
		msgType = POWER_START_UPGRADE;
		msgEndType = POWER_END_UPGRADE;
		eventType = WINDOW_START;
//		FIXS_CCH_Event::ResetEvent(m_waitHandles[5]);
	}
	else if (event_index == JTP_FAN_START_Index)
	{
		msgType = FAN_START_UPGRADE;
		msgEndType = FAN_END_UPGRADE;
		eventType = WINDOW_START;
//		FIXS_CCH_Event::ResetEvent(m_waitHandles[7]);
	}
	else if (event_index == JTP_IMBSW_END_Index)
	{
		msgType = IMBSW_END_UPGRADE;
		start_index = JTP_IMBSW_START_Index;
		eventType = WINDOW_END;
//		FIXS_CCH_Event::ResetEvent(m_waitHandles[2]);
	}
	else if (event_index == JTP_IPMI_END_Index)
	{
		msgType = IPMI_END_UPGRADE;
		start_index = JTP_IPMI_START_Index;
		eventType = WINDOW_END;
//		FIXS_CCH_Event::ResetEvent(m_waitHandles[4]);
	}
	else if (event_index == JTP_POWER_END_Index)
	{
		msgType = POWER_END_UPGRADE;
		start_index = JTP_POWER_START_Index;
		eventType = WINDOW_END;
//		FIXS_CCH_Event::ResetEvent(m_waitHandles[6]);
	}
	else if (event_index == JTP_FAN_END_Index)
	{
		msgType = FAN_END_UPGRADE;
		start_index = JTP_FAN_START_Index;
		eventType = WINDOW_END;
//		FIXS_CCH_Event::ResetEvent(m_waitHandles[8]);
	}
	else if (event_index == JTP_IMBUPG_START_REQ_index)
	{
		msgType = IMB_UPG_START_REQ;
		eventType = WINDOW_NONE;
	}

	else
	{
		return false;
	}

	//boardRestarted (msgType);

	bool isOpen = m_windowIsOpen[start_index];

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "isOpen: "<< isOpen << std::endl;

	if (!m_windowIsOpen[start_index] && eventType == WINDOW_START)
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "????????????????????????? \n" << std::endl;

		isOpen = sendNotification(msgType, event_index, JTP_START_MAX_RETRY);
	}
	else if (!m_windowIsOpen[start_index] && eventType == WINDOW_END)
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "End arrived. The AP application hasn't opened the maintenance window" << std::endl;

		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "End arrived. The AP application hasn't opened the maintenance window");
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
			if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1,tmpStr);
		}

		sendNotification(msgType, event_index, JTP_END_MAX_RETRY);
		isOpen = false;
	}
	else if (m_windowIsOpen[start_index] && eventType == WINDOW_START)
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "START arrived but the window is already opened. Try to send a JTP end message before a start JTP message" << std::endl;

		int tempVersion = VERSION1;
		if( (msgType == IMBSW_START_UPGRADE) || (msgType == IMBSW_END_UPGRADE) ||
				(msgType == IPMI_START_UPGRADE) || (msgType == IPMI_END_UPGRADE) )
		{
			tempVersion = VERSION2;
		}

		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "START arrived but the window is already opened. Try to send a JTP end message before a start JTP message");
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
			if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1,tmpStr);
		}

		(void) this->handleNotification(tempVersion, msgEndType, this->m_shelf, this->m_slot, JTP_END_MAX_RETRY);
		isOpen = sendNotification(msgType, event_index, JTP_START_MAX_RETRY);
	}
	else if (m_windowIsOpen[start_index] && eventType == WINDOW_END)
	{
		sendNotification(msgType, event_index, JTP_END_MAX_RETRY);
		isOpen = false;
	}

	if(msgType == IMB_UPG_START_REQ)
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " +++++++STATE MACHINE IMB_UPG_START_REQ+++++++" << std::endl;
		sendNotification(msgType, event_index, JTP_START_REQUEST_RETRY);
	}
	else
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "????????????????? STATE MACHINE ????????????????????????????" << std::endl;
		m_windowIsOpen[start_index] = isOpen;
	}

	return true;

}

 void FIXS_CCH_JTPHandler::maintenanceWindow (unsigned char msgType, unsigned char slot)
 {
	 std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " +++ MAINTENANCE WINDOW +++ IN"<< std::endl;
	 m_slot = slot;
	 printf("DBG: [%s@%d] - ++++++++SLOTONGOING FOR maintenance/start: %x \n", __FUNCTION__, __LINE__, m_slot);
	 printf("DBG: [%s@%d] - SLOT: 0x%x \n", __FUNCTION__, __LINE__, this->m_slot);
	 maintenanceWindow(msgType);
 }

 int FIXS_CCH_JTPHandler::open (void *args)
 {
	 int result = 0;
	 UNUSED(args);
	 cout << "\n JTP Thread Activate... " << endl;
	 m_exit = false;

	 activate();

	 {
		 char tmpStr[512] = {0};
		 snprintf(tmpStr, sizeof(tmpStr) - 1, "JTP Thread Activate on instances[%p]",this);
		 if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
		 if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1,tmpStr);
	 }


	 return result;

 }

void FIXS_CCH_JTPHandler::stop ()
{
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " +++ STOP +++ IN"<< std::endl;
	m_exit = true;
	if (m_StopEvent)
	{
		int retries=0;
		while (FIXS_CCH_Event::SetEvent(m_StopEvent) == 0)
		{
			if( ++retries > 10) break;
			sleep(1);
		}

		if (retries>10)
		{
			if (_trace->ACS_TRA_ON())
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Error: Cannot signal Stop event.", __FUNCTION__, __LINE__);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
				_trace->ACS_TRA_event(1,tmpStr);
			}
		}
	}
}

void FIXS_CCH_JTPHandler::waitStartReqToFinish ()
{
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " +++ WAITSTARTREQTOFINISH +++ IN"<< std::endl;
	std::cout << "Waiting for Authorization response......." << std::endl;
	//DWORD dwWaitResult = WaitForSingleObject(m_canStartEvent,INFINITE);
	//int dwWaitResult = FIXS_TREFC_Event::WaitForEvents(1, &m_canStartEvent, INFINITE);

	int validIndexNo = 2;
	int validHandles[validIndexNo];

	validHandles[0] = m_startReqEvent;
	validHandles[1] = m_shutdownEvent;

	int returnValue = FIXS_CCH_Event::WaitForEvents(validIndexNo, validHandles, 60000); //1 minute sleep

	if(returnValue == WAIT_OBJECT_0)
	{
		std::cout << "Start Upgrade Req ok......." << std::endl;

		if (_trace->ACS_TRA_ON())
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] event signaled, it can start request over now..", __FUNCTION__, __LINE__);
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
			_trace->ACS_TRA_event(1,tmpStr);
		}
	}

	else if ( returnValue == WAIT_TIMEOUT )
	{
			std::cout << "Upgrade start request timed out......." << std::endl;

			if (_trace->ACS_TRA_ON())
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] event timed out.......", __FUNCTION__, __LINE__);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
				_trace->ACS_TRA_event(1,tmpStr);
			}
	}

	else if(returnValue == validIndexNo - 1)
	{
		if (_trace->ACS_TRA_ON())
					{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Error: shutdown event signaled.", __FUNCTION__, __LINE__);
			_trace->ACS_TRA_event(1,tmpStr);
		}
	}
	else
	{
		if (_trace->ACS_TRA_ON())
					{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Error: Unexpected event signaled.", __FUNCTION__, __LINE__);
			_trace->ACS_TRA_event(1,tmpStr);
		}
	}
}

void FIXS_CCH_JTPHandler::waitStartReqToFinish (bool &shutdownsignaled)
{
	shutdownsignaled =false;
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " +++ WAITSTARTREQTOFINISH +++ IN"<< std::endl;
	std::cout << "Waiting for Authorization response......." << std::endl;
	//DWORD dwWaitResult = WaitForSingleObject(m_canStartEvent,INFINITE);
	//int dwWaitResult = FIXS_TREFC_Event::WaitForEvents(1, &m_canStartEvent, INFINITE);

	int validIndexNo = 2;
	int validHandles[validIndexNo];

	validHandles[0] = m_startReqEvent;
	validHandles[1] = m_shutdownEvent;

	int returnValue = FIXS_CCH_Event::WaitForEvents(validIndexNo, validHandles, 60000); //1 minute sleep

	if(returnValue == WAIT_OBJECT_0)
	{
		std::cout << "Start Upgrade Req ok......." << std::endl;

		if (_trace->ACS_TRA_ON())
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] event signaled, it can start request over now..", __FUNCTION__, __LINE__);
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
			_trace->ACS_TRA_event(1,tmpStr);
		}
	}

	else if ( returnValue == WAIT_TIMEOUT )
	{
			std::cout << "Upgrade start request timed out......." << std::endl;

			if (_trace->ACS_TRA_ON())
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] event timed out.......", __FUNCTION__, __LINE__);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
				_trace->ACS_TRA_event(1,tmpStr);
			}
	}

	else if(returnValue == validIndexNo - 1)
	{
		shutdownsignaled = true;
		if (_trace->ACS_TRA_ON())
					{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Error: shutdown event signaled.", __FUNCTION__, __LINE__);
			_trace->ACS_TRA_event(1,tmpStr);
		}
	}
	else
	{
		if (_trace->ACS_TRA_ON())
					{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Error: Unexpected event signaled.", __FUNCTION__, __LINE__);
			_trace->ACS_TRA_event(1,tmpStr);
		}
	}
}


bool FIXS_CCH_JTPHandler::checkStartReqStatus ()
{
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " +++ CHECKSTARTREQSTATUS +++ IN"<< std::endl;
	return this->m_startReqStatus;
}

void FIXS_CCH_JTPHandler::setStartReqStatus (bool value)
{

	this->m_startReqStatus = value;
}

