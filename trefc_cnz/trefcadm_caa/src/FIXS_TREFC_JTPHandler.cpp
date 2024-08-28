/*
 * FIXS_TREFC_JTPHandler.cpp
 *
 *  Created on: Jan 24, 2012
 *      Author: eiolbel
 */

#include "ACS_JTP.h"
#include <iostream>
#include "FIXS_TREFC_Util.h"
#include "FIXS_TREFC_JTPHandler.h"

namespace {

const int VERSION = 1;
const int WAIT_FAILED=-1;
const int WAIT_TIMEOUT= -2;
const int WAIT_OBJECT_0=0;
enum JTP_Event_Indexes
{
	JTP_Shutdown_Event_Index = 0,
	JTP_TIME_SERVICE_START_Index = 1,
	JTP_TIME_SERVICE_STOP_Index = 2,
	JTP_TIME_SERVICE_CAN_START_Index = 3,
	JTP_TIME_REFERENCE_ADD_Index = 4,
	JTP_TIME_REFERENCE_REMOVE_Index = 5,
	JTP_TIME_SERVICE_STATUS_REQUEST_Index = 6,
	JTP_Number_Of_Events = 7
};

DWORD JTP_Safe_Timeout = 120000;

const int JTP_DELAY_TIME = 60; // Time delay between connect tries.
const DWORD MILLISEC_DELAY_TIME = JTP_DELAY_TIME * 1000;

// if you want three tries, you must set to four.
const int JTP_TREFC_MAX_RETRY = 4;
const int JTP_CAN_START_RETRY =1; //means retry set to zero for can start query
}
#ifndef __
#define __ std::cout << __FUNCTION__ << "@" << __LINE__ << std::endl;
#endif

// Class FIXS_TREFC_JTPHandler

FIXS_TREFC_JTPHandler::FIXS_TREFC_JTPHandler (unsigned char shelf, bool slot0, bool slot25)
:        		m_shutdownEvent(0),
        		m_waitHandles(NULL),
        		m_canStartEvent(0),
        		m_exit(false),
        		m_canSendStartR(false),
        		m_shelf(0),
        		m_slot0(false),
        		m_slot25(false),
        		m_servType(0),
        		m_ref(0),
        		ipValue(0),
        		m_retryOn(false)

{

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;

	m_waitHandles = new FIXS_TREFC_EventHandle[JTP_Number_Of_Events];
	m_shutdownEvent =  FIXS_TREFC_Event::OpenNamedEvent(TREFC_Util::EVENT_NAME_SHUTDOWN);
	m_canStartEvent = FIXS_TREFC_Event::CreateEvent(0,false,0);
	m_waitHandles[JTP_Shutdown_Event_Index] = m_shutdownEvent;
	for (int i=1; i < JTP_Number_Of_Events; i++) {
		m_waitHandles[i] = FIXS_TREFC_Event::CreateEvent(0,false,0);
	}

	traceObj = new ACS_TRA_trace("FIXS_TREFC_JTPHandler");

	this->m_shelf = shelf;
	this->m_slot0 = slot0;
	this->m_slot25 = slot25;
}


FIXS_TREFC_JTPHandler::~FIXS_TREFC_JTPHandler()
{
	stop();
	//	if (!waitOnStopState(200, 20))
	//	{
	//		//trace error
	//		{
	//			char traceChar[512] = {0};
	//			_snprintf(traceChar, sizeof(traceChar) - 1, "[%s@%d] Cannot stop Thread", __FUNCTION__, __LINE__);
	//			if (ACS_TRA_ON(traceJTPHandler)) ACS_TRA_event(&traceJTPHandler, traceChar);
	//		}
	//
	//		try { terminate(1); } catch(...) {}
	//		FIXS_TREFC_thread::sleep(100);
	//	}

	if(m_waitHandles) {
		for (int i=1; i < JTP_Number_Of_Events; i++) {
			if (m_waitHandles[i] != 0)
				FIXS_TREFC_Event::CloseEvent(m_waitHandles[i]);
		}

		delete []m_waitHandles;
		m_waitHandles = NULL;
	}

	if(m_canStartEvent) {
		FIXS_TREFC_Event::CloseEvent(m_canStartEvent);
		m_canStartEvent = 0;
	}

	//Delete trace object
	if (traceObj) {
		delete (traceObj);
		traceObj = NULL;
	}

//	if (m_stopRetry) {
//		delete [] m_stopRetry;
//		m_stopRetry = NULL;
//	}

	//	if (m_jtpAccessControlPtr)
	//	{
	//		try
	//		{
	//			DeleteCriticalSection(m_jtpAccessControlPtr);
	//			if(m_jtpAccessControlPtr)
	//			{
	//				delete m_jtpAccessControlPtr;
	//				m_jtpAccessControlPtr = NULL;
	//			}
	//		} catch (...) {}
	//	}
	//## end FIXS_TREFC_JTPHandler::~FIXS_TREFC_JTPHandler%4E3A7708020D_dest.body
}



//## Other Operations (implementation)
void FIXS_TREFC_JTPHandler::setJtpNotificationEvent (unsigned char msgType, int servType)
{
	__;

	std::string tmp;
	int event_index = -1;
	this->m_servType = servType;
	this->m_retryOn = false;

	if (msgType == TIME_SERVICE_START) { event_index = JTP_TIME_SERVICE_START_Index; tmp = "TIME_SERVICE_START"; }
	else if (msgType == TIME_SERVICE_STOP) { event_index = JTP_TIME_SERVICE_STOP_Index; tmp = "TIME_SERVICE_STOP"; }
	else if (msgType == TIME_SERVICE_CAN_START) { event_index = JTP_TIME_SERVICE_CAN_START_Index; tmp = "TIME_SERVICE_CAN_START"; }
	else if (msgType == TIME_REFERENCE_ADD) { event_index = JTP_TIME_REFERENCE_ADD_Index; tmp = "TIME_REFERENCE_ADD"; }
	else if (msgType == TIME_REFERENCE_REMOVE) { event_index = JTP_TIME_REFERENCE_REMOVE_Index; tmp = "TIME_REFERENCE_REMOVE"; }
	else if (msgType == TIME_SERVICE_STATUS_REQUEST) { event_index = JTP_TIME_SERVICE_STATUS_REQUEST_Index; tmp = "TIME_SERVICE_STATUS_REQUEST"; }

	if (traceObj->ACS_TRA_ON())
	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Notification event is signaled... %s", __FUNCTION__, __LINE__,tmp.c_str());
		traceObj->ACS_TRA_event(1,tmpStr);
	}

	FIXS_TREFC_Event::SetEvent(m_waitHandles[event_index]);

}

int FIXS_TREFC_JTPHandler::svc ()
{

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " IN"<< std::endl;

	//char tmpStr[512] = {0};

	//loop
	while ( !m_exit)// Set to true when the thread shall exit
	{
		// Wait for shutdown and command events
		int returnValue=FIXS_TREFC_Event::WaitForEvents(JTP_Number_Of_Events, m_waitHandles, JTP_Safe_Timeout);

		if (returnValue ==  WAIT_FAILED)   // Event failed
		{
			if (traceObj->ACS_TRA_ON())
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Error: Cannot WaitForMultipleObjects.", __FUNCTION__, __LINE__);
				traceObj->ACS_TRA_event(1,tmpStr);
			}

			m_exit = true;    // Terminate the JTP Handler thread
		}
		else if (returnValue ==  WAIT_TIMEOUT)  // Time out
		{
			std::cout << __FUNCTION__ << "@" << __LINE__ << " JTPHandler is waiting for events..." << std::endl;
		}
		else if (returnValue ==  (WAIT_OBJECT_0 + JTP_Shutdown_Event_Index))  // Shutdown event
		{
			if (traceObj->ACS_TRA_ON())
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Recieved a shutdown event", __FUNCTION__, __LINE__);
				traceObj->ACS_TRA_event(1,tmpStr);
			}

			m_exit = true;    // Terminate the thread
		}
		else if (sendJOBNotificationToCP(returnValue - WAIT_OBJECT_0))
		{
			if (traceObj->ACS_TRA_ON())
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] an event is signalled", __FUNCTION__, __LINE__);
				traceObj->ACS_TRA_event(1,tmpStr);
			}
		}
		else
		{
			if (traceObj->ACS_TRA_ON())
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d]", __FUNCTION__, __LINE__);
				traceObj->ACS_TRA_event(1,tmpStr);
			}
		}

	} // End of the while loop
	return 0;
}

bool FIXS_TREFC_JTPHandler::sendJOBNotificationToCP (int event_index)
{

	unsigned char msgType = 0;


	if (event_index == JTP_TIME_SERVICE_START_Index)
	{
		msgType = TIME_SERVICE_START;
		FIXS_TREFC_Event::ResetEvent(m_waitHandles[1]);
	}
	else if (event_index == JTP_TIME_SERVICE_STOP_Index)
	{
		msgType = TIME_SERVICE_STOP;
		FIXS_TREFC_Event::ResetEvent(m_waitHandles[2]);
	}
	else if (event_index == JTP_TIME_SERVICE_CAN_START_Index)
	{
		msgType = TIME_SERVICE_CAN_START;
		FIXS_TREFC_Event::ResetEvent(m_waitHandles[3]);
	}
	else if (event_index == JTP_TIME_REFERENCE_ADD_Index)
	{
		msgType = TIME_REFERENCE_ADD;
		FIXS_TREFC_Event::ResetEvent(m_waitHandles[4]);
	}
	else if (event_index == JTP_TIME_REFERENCE_REMOVE_Index)
	{
		msgType = TIME_REFERENCE_REMOVE;
		FIXS_TREFC_Event::ResetEvent(m_waitHandles[5]);
	}
	else if (event_index == JTP_TIME_SERVICE_STATUS_REQUEST_Index)
	{
		msgType = TIME_SERVICE_STATUS_REQUEST;
		FIXS_TREFC_Event::ResetEvent(m_waitHandles[6]);
	}
	else
	{
		return false;
	}
	if(msgType == TIME_SERVICE_CAN_START)
		sendNotification(msgType, event_index, JTP_CAN_START_RETRY);
	else
		sendNotification(msgType, event_index, JTP_TREFC_MAX_RETRY);

	return true;
}

void FIXS_TREFC_JTPHandler::stop ()
{
	//## begin FIXS_TREFC_JTPHandler::stop
	m_exit = true;
//	if (m_waitHandles[JTP_Shutdown_Event_Index] != 0)
//	{
//		if (FIXS_TREFC_Event::SetEvent(m_waitHandles[JTP_Shutdown_Event_Index]) == false)
//		{
//			if (traceObj->ACS_TRA_ON())
//			{
//				char tmpStr[512] = {0};
//				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Error: Cannot signal shutdown event.", __FUNCTION__, __LINE__);
//				traceObj->ACS_TRA_event(1,tmpStr);
//			}
//			//			if (ACS_TRA_ON(traceJTPHandler))
//			//			{
//			//				char tmpStr[512] = {0};
//			//				_snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Error: Cannot signal shutdown event. last error code: %u", __FUNCTION__, __LINE__, GetLastError());
//			//				ACS_TRA_event(&traceJTPHandler, tmpStr);
//			//			}
//		}
//	}
	//## end FIXS_TREFC_JTPHandler::stop
}

void FIXS_TREFC_JTPHandler::waitCanStartQueryToFinish ()
{
	std::cout << "Waiting for can start query response......." << std::endl;
	//DWORD dwWaitResult = WaitForSingleObject(m_canStartEvent,INFINITE);
	//int dwWaitResult = FIXS_TREFC_Event::WaitForEvents(1, &m_canStartEvent, INFINITE);

	int validIndexNo=2;
	int validHandles[validIndexNo];

	validHandles[0] = m_canStartEvent;
	validHandles[1] = m_shutdownEvent;

	int dwWaitResult = FIXS_TREFC_Event::WaitForEvents(validIndexNo, validHandles, 10000);

	if(dwWaitResult == WAIT_OBJECT_0)
	{
		std::cout << "Can start query over now......." << std::endl;

		if (traceObj->ACS_TRA_ON())
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] : event signaled, it can start query over now..", __FUNCTION__, __LINE__);
			traceObj->ACS_TRA_event(1,tmpStr);
		}
	}
	else
	{
		if(dwWaitResult == validIndexNo-1)
		{
			if (traceObj->ACS_TRA_ON())
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Error: shutdown event signaled.", __FUNCTION__, __LINE__);
				traceObj->ACS_TRA_event(1,tmpStr);
			}

		}
		else

			if (traceObj->ACS_TRA_ON())
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Error: Unexpected event signaled.", __FUNCTION__, __LINE__);
				traceObj->ACS_TRA_event(1,tmpStr);
			}
	}
}

bool FIXS_TREFC_JTPHandler::checkCanSendStartR ()
{
	//## begin FIXS_TREFC_JTPHandler::checkCanSendStartR%4E3B7E1401E5.body preserve=yes
	return this->m_canSendStartR;
	//## end FIXS_TREFC_JTPHandler::checkCanSendStartR%4E3B7E1401E5.body
}

void FIXS_TREFC_JTPHandler::setReferenceValues (unsigned long ipVal, unsigned short refType)
{
	//## begin FIXS_TREFC_JTPHandler::setReferenceValues%4E3B7E2C0009.body preserve=yes
	std::cout <<"********************** setting the reference values **********************"<<std::endl;
	this->ipValue = ipVal;
	this->m_ref = refType;
	//## end FIXS_TREFC_JTPHandler::setReferenceValues%4E3B7E2C0009.body
}

bool FIXS_TREFC_JTPHandler::sendNotification (unsigned char msgType, int /*event_index*/, const int noTries)
{
	//## begin FIXS_TREFC_JTPHandler::sendNotification%4E3B8C160387.body preserve=yes
	//char tmpStr[512] = {0};

	bool sendOk = false;
	// version is User Data 1 (MSB)  0x01
	// msgType is User Data 1 (LSB)  example 0x1E Power FW Upgrade End. See enum JOB_REQUEST_CODE
	// shelf is User Data 2 (MSB)  0x00 to 0x1F
	// slot is User Data 2 (LSB)   0x00 (slot 0) or 0x19 (slot 25)

	if ((this->m_slot0 == true) || (this->m_slot25 == true))  // slot 0 is 0x00, slot 25 is the decimal of 0x19
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
		int exitCode = this->handleNotification(VERSION, msgType,noTries);
		int retryForConf = 1;
		int downgradeVersion = VERSION;
		bool sendNotification = true;
		this->m_retryOn = true;
		while (sendNotification)
		{
			if (exitCode == 0)
			{
				std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;

				if (traceObj->ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Exit code for handleNotification is OK.", __FUNCTION__, __LINE__);
					traceObj->ACS_TRA_event(1,tmpStr);
				}

				sendOk = true;
				sendNotification = false;
			}
			else if (exitCode == FIXS_TREFC_JTPClient::JOB_REJECTED) //Not Confirmation or jinitreq failed
			{
				std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
				if( msgType == TIME_SERVICE_CAN_START)
				{
					std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "No retry in case Can start query failed........ " << std::endl;
					sendNotification = false;
					if (traceObj->ACS_TRA_ON())
					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Can start query rejected or failed on jinitreq. No retries in this case.", __FUNCTION__, __LINE__);
						traceObj->ACS_TRA_event(1,tmpStr);
					}

					break;
				}

				if (traceObj->ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] retry to send CP request after FIXS_TREFC_JTPClient::JOB rejected or failed jinitreq..", __FUNCTION__, __LINE__);
					traceObj->ACS_TRA_event(1,tmpStr);
				}

				int returnValue=FIXS_TREFC_Event::WaitForEvents(JTP_Number_Of_Events, m_waitHandles, MILLISEC_DELAY_TIME);

				if (returnValue !=  WAIT_TIMEOUT && returnValue != WAIT_FAILED)
					FIXS_TREFC_Event::SetEvent(m_waitHandles[returnValue - WAIT_OBJECT_0]);

				std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;

				if (noTries < 0) // no retry
					sendNotification = false;
				else if ((noTries > retryForConf)&&(this->m_retryOn))
				{
					if (traceObj->ACS_TRA_ON())
					{
						char tmpStr[512] = {0};
						snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Send CP. retry number: %d", __FUNCTION__, __LINE__, retryForConf);
						traceObj->ACS_TRA_event(1,tmpStr);
					}
					exitCode = this->handleNotification(VERSION, msgType, noTries);
					retryForConf++;
				}
				else
				{// retries are terminated
					std::cout << " ========================================= " << std::endl;
					std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
					std::cout << "**** The retry is terminated ****" << std::endl;
					std::cout << " ========================================= " << std::endl;
					//raise event
					int problem = 35101;
					std::string cause = "JTP COMMUNICATION FAULT";
					std::string data = "JTP Notification to CP Failed";
					std::string text("");
					switch(msgType)
					{
					case TIME_SERVICE_START:
						text = "Time Service Started Notification - All tries failed.";
						break;
					case TIME_SERVICE_STOP:
						text = "Time Service Stopped Notification - All tries failed.";
						break;
					case TIME_REFERENCE_ADD:
						text = "Time Reference Added Notification - All tries failed.";
						break;
					case TIME_REFERENCE_REMOVE:
						text = "Time Reference Removed Notification - All tries failed.";
						break;
					default:
						text = "All tries failed for JTP notification.";
						break;
					}
					TREFC_Util::sendEvent (problem, cause, data, text);
					sendNotification = false;
				}
			}
			else if (exitCode == FIXS_TREFC_JTPClient::JOB_FAILED) //Not Result Ind
			{
				if (traceObj->ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Job failed..", __FUNCTION__, __LINE__);
					traceObj->ACS_TRA_event(1,tmpStr);
				}
				sendNotification = false;
			}
			else if (exitCode == FIXS_TREFC_JTPClient::JOB_MISMATCH) // Protocol version mismatch
			{

				downgradeVersion--;
				std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
				if (traceObj->ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] retry to send CP request after protocol mismatch. Version is %d", __FUNCTION__, __LINE__, downgradeVersion);
					traceObj->ACS_TRA_event(1,tmpStr);
				}
				exitCode = this->handleNotification(downgradeVersion, msgType, noTries);
				if (downgradeVersion == 0) sendNotification = false;
			}
			else
			{
				if (traceObj->ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] handleNotification: Wrong exit code", __FUNCTION__, __LINE__);
					traceObj->ACS_TRA_event(1,tmpStr);
				}

				sendNotification = false;
			}
		}
		this->m_retryOn = false;
		if(msgType == TIME_SERVICE_CAN_START)
		{
			if (traceObj->ACS_TRA_ON())
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Can start query finished...event is signalled...", __FUNCTION__, __LINE__);
				traceObj->ACS_TRA_event(1,tmpStr);
			}
			FIXS_TREFC_Event::SetEvent(m_canStartEvent);
		}
	}
	else
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
		if (traceObj->ACS_TRA_ON())
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Wrong slot number. Message type is 0x%X", __FUNCTION__, __LINE__, msgType);
			traceObj->ACS_TRA_event(1,tmpStr);
		}
	}
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " OUT" << std::endl;

	return sendOk;
}

int FIXS_TREFC_JTPHandler::handleNotification (int version, unsigned short msgType, const int /*noTries*/)
{
	//char tmpStr[512] = {0};

	//char* ServiceName = ((char*)"AMITRCCP");

	char ServiceName[10] = {0};
	snprintf(ServiceName, sizeof(ServiceName) - 1, "AMITRCCP");


	unsigned short Len = 0;
	char *Msg = 0;
	unsigned short U1, U2;

	U1 = version;
	U1 = U1 << 8;
	U1 = U1 | msgType;

	if(msgType == TIME_SERVICE_CAN_START)
	{
		if(this->m_servType == 1)
			U2 = NTP;
		else if(this->m_servType == 2)
			U2 = NTPHP;
		else if(this->m_servType == 3)
			U2 = NTPHP_GPS;

		U2 = U2 << 8;

	}

	else if(msgType == TIME_SERVICE_START || msgType == TIME_SERVICE_STOP)
	{
		std::cout <<" ....... service start or stop notification... "<<__FUNCTION__<< "\t"<<__LINE__<<std::endl;
		if(this->m_servType == 1)
			U2 = NTP;
		else if(this->m_servType == 2)
			U2 = NTPHP;
		else if(this->m_servType == 3)
			U2 = NTPHP_GPS;

		U2 = U2 << 8;

		if ((this->m_slot0 == true) && (this->m_slot25 == true))
			U2 = U2 | 0x0002;
		else
			U2 = U2 | 0x0001;

		Msg = new char[64];
		if ((this->m_slot0 == true) && (this->m_slot25 == true))
		{
			Msg[0] = this->m_shelf;
			Msg[1] = (char) 0x00;
			Msg[2] = this->m_shelf;
			Msg[3] = (char) 0x19;
			Len = 4;
		}
		else if(this->m_slot0 == true)
		{
			Msg[0] = this->m_shelf;
			Msg[1] = (char) 0x00;
			Len = 2;
		}
		else
		{
			Msg[0] = this->m_shelf;
			Msg[1] = (char) 0x19;
			Len = 2;
		}
	}
	else if(msgType == TIME_REFERENCE_ADD || msgType == TIME_REFERENCE_REMOVE)
	{
		std::cout <<"********************** ADD or REMOVE ENTRY NOTIFICATION **********************"<<std::endl;
		std::cout <<" ....... add or remove tref notification..... "<<__FUNCTION__<< "\t"<<__LINE__<<std::endl;
		if(this->m_servType == 1)
			U2 = NTP;
		else if(this->m_servType == 2)
			U2 = NTPHP;
		else if(this->m_servType == 3)
			U2 = NTPHP_GPS;

		U2 = U2 << 8;
	}
	else
	{
		std::cout << "ERROR: Unknown message type. Cannot handle JTP notification.";
		return 1;
	}

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "U1="<< U1 <<" U2="<< U2 <<std::endl;
	int exitCode = 0;

	// Create a JTP_Job object with name and maxbuffer__
	ACS_JTP_Job J1(ServiceName, Len);
	__
	// Override the default timeout which is 10 seconds
	J1.setTimeOut(10);

	// Override the default no of tries for jinitreq
	//J1.setNoOfTries(noTries);

	// Override the default time between retries
	//J1.setDelayTime(JTP_DELAY_TIME);

	if(msgType == TIME_SERVICE_CAN_START)
		this->m_canSendStartR = false;
	__
	FIXS_TREFC_JTPClient *jTPClient = new FIXS_TREFC_JTPClient(m_waitHandles[JTP_Shutdown_Event_Index]);
	int result = jTPClient->newJTPClient(Len, J1, U1, U2, Msg);
	__
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<
	" newJTPClient result =  " << result << std::endl;

	if (result == 0)
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
		int confermation = jTPClient->waitForConfermation(J1, U1, U2);

		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " waitForConfermation result =  " << confermation << std::endl;

		if (confermation == 0)
		{
			__
			U1 = 0x00;
			U1 = U1 << 8;
			U1 = U1 | msgType;
			Len = 0;

			bool waitRes = jTPClient->waitForJTPResult(J1, Len, U1, U2, Msg);

			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " waitForJTPResult result =  " << waitRes << std::endl;

			if (!waitRes)
			{
				std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
				exitCode = FIXS_TREFC_JTPClient::JOB_FAILED;
				if (traceObj->ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] JTP waitForJTPResult - Unexpected error.", __FUNCTION__, __LINE__);
					traceObj->ACS_TRA_event(1,tmpStr);
				}
			}
			else
			{
				__
				if(msgType == TIME_SERVICE_CAN_START)
					this->m_canSendStartR = true;
				if (traceObj->ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] JTP waitForJTPResult - SUCCESSFULL", __FUNCTION__, __LINE__);
					traceObj->ACS_TRA_event(1,tmpStr);
				}
			}
		}
		else
		{
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
			exitCode = confermation;
			if (traceObj->ACS_TRA_ON())
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] JTP confirmation NOT OK", __FUNCTION__, __LINE__);
				traceObj->ACS_TRA_event(1,tmpStr);
			}
		}
	}
	else
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
		exitCode = result;
		if (traceObj->ACS_TRA_ON())
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] JTP request NOT OK", __FUNCTION__, __LINE__);
			traceObj->ACS_TRA_event(1,tmpStr);
		}
	}

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " OUT"<< std::endl;
	delete jTPClient;
	if(Msg) delete [] Msg;

	return exitCode;
	//## end FIXS_TREFC_JTPHandler::handleNotification%4E3B8D2B0121.body
}


int FIXS_TREFC_JTPHandler::open (void *args)
{
	int result = 0;
	UNUSED(args);
	m_exit = false;
	result = activate();
	cout << "FIXS_TREFC_JTPHandler activate: " << result << endl;
	return result;
}

void FIXS_TREFC_JTPHandler::updateApzScx(unsigned short slot, bool add) {

	if (slot == 0)
		this->m_slot0 = add;
	else if (slot == 25)
		this->m_slot25 = add;
}

// Additional Declarations

