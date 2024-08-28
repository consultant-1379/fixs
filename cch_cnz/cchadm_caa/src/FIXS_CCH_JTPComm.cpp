/*
 * FIXS_CCH_JTPComm.cpp
 *
 *  Created on: Oct 20, 2011
 *      Author: eiolbel
 */

// FIXS_CCH_JTPComm
#include "FIXS_CCH_JTPComm.h"

namespace {

//add other variable to handle the event
const int WAIT_FAILED = -1;
const int WAIT_TIMEOUT = -2;
const int WAIT_OBJECT_0 = 0;
const int Shutdown_Event_Index = 0;
const int Command_Event_Index = 1;
const int TimeoutTrap_Event_Index = 2;

}
#ifndef __
#define __ std::cout << __FUNCTION__ << "@" << __LINE__ << std::endl;
#endif

// Class FIXS_CCH_JTPComm
FIXS_CCH_JTPComm::FIXS_CCH_JTPComm (DWORD interval, unsigned short aSystemid, ACS_JTP_Conversation *conv)
{
	m_interval = interval;
	m_systemid = aSystemid;
	m_conversation = conv;
	m_send_buffer = new char[MAINTENANCE_MAX_BUFFER];
	m_rec_msg = NULL; //pointer to JTP internal buffer
	m_listOfMaitenance = NULL;

	m_rec_length = 0;
	m_rec_data1 = 0;
	m_rec_data2 = 0;

	m_send_length = 0;
	m_send_data1 = 0;
	m_send_data2 = 0;
	m_send_result = 0;

	m_iteratorCount = 0;

	traceObj = new ACS_TRA_trace("FIXS_CCH_JTPComm");
}


FIXS_CCH_JTPComm::~FIXS_CCH_JTPComm()
{
	if (m_send_buffer != NULL) delete [] m_send_buffer;
	if (m_listOfMaitenance != NULL) delete m_listOfMaitenance;

	//Delete trace object
	if (traceObj) {
		delete (traceObj);
		traceObj = NULL;
	}
}

int FIXS_CCH_JTPComm::conversation ()
{
	char tmpStr[512] = {0};

	//FIXS_CCH_EventHandle PollFd;
	EventHandle PollFd;
	char problemData [PROBLEMDATASIZE];   // Data for event report.
	ACS_JTP_Conversation::JTP_Node node;  // Structure node with CP systemId.
	node.system_id = m_systemid;

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "New call received!" << std::endl;

	// Try to initiate.
	if (m_conversation->State() != ACS_JTP_Conversation::StateConnected)
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "JTP StateConnected failed" << std::endl;

		if (traceObj->ACS_TRA_ON())
		{
			char tmpStr[512] = {0};
			snprintf(problemData, sizeof(problemData) - 1, "%s","APJTP State not connected. `");
			traceObj->ACS_TRA_event(1,tmpStr);
		}

		return -1;
	}

	// OK, we are Connected, send jexinitind
	if (m_conversation->jexinitind(node, m_rec_data1, m_rec_data2) == false)
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "jexinitind failed" << std::endl;
		return -1;
	}

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "JTP connection initiated!" << std::endl;

	m_send_data1 = 0x0100;      // Get jexinitrsp data for R1 version
	m_send_data2 = 0;
	m_send_result = 0;

	// Send jexinitrsp
	if (m_conversation->jexinitrsp(m_send_data1, m_send_data2, m_send_result) == false)
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "jexinitrsp failed" << std::endl;
		return -1;
	}

	PollFd = m_conversation->getHandle();
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Connected - Get data!" << std::endl;

	unsigned short boardListLen = 0;
	FIXS_CCH_UpgradeManager *upgServer = FIXS_CCH_UpgradeManager::getInstance();
	upgServer->swMaintenanceList(m_listOfMaitenance, boardListLen);
	m_iteratorCount = 0;
	bool sendLastMsg = false;

	// Loop: Receive/Receive the data
	while(m_conversation->State() == ACS_JTP_Conversation::StateWaitForData) {

		PollFd = m_conversation->getHandle();

		// Wait unitil our handle is signaled

		//int status = WaitForSingleObject(PollFd,((m_interval+30)*1000));
		int status=FIXS_CCH_Event::WaitForEvents(1, &PollFd, 100);


		if (status == WAIT_TIMEOUT)
		{
			// No data within time limit.
			// Send msg to eventmanager.
			/*	if (ACS_TRA_ON(traceJTPComm))
			{
				ACS_TRA_event(&traceJTPComm,"JTP timeout");
			}*/
			if (traceObj->ACS_TRA_ON())
			{
				traceObj->ACS_TRA_event(1,tmpStr);
			}

			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "JTP timed out" << std::endl;

			m_send_data1 = 0x00F0; // MAINTENANCE LIST INFO
			m_send_data2 = 0;
			m_send_result = 0;
			m_conversation->jexdiscreq(m_send_data1,m_send_data2,m_send_result);

			// We have to give this connection up. We must have a completely
			// new connection in order to get things running (or exit?)
			return -1;
		}

		if (sendLastMsg)
		{
			//if(m_conversation->State() == ACS_JTP_Conversation::StateCreated) {
			m_rec_data1 = m_rec_data2 = m_send_result = 0;
			if (m_conversation->jexdiscind(m_rec_data1,m_rec_data2,m_send_result) == false)
			{
				// jexdiscind failed.

				// We are disconnected. Inform Eventmanager and update CP object status.
				IssueJTPevent(-1);
				return -1;
			}
			else {
				IssueJTPevent(m_send_result);
			}
			//}
			/*else
			{
				std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Wrong State: JTP after last message is not StateCreated" << std::endl;
				if (ACS_TRA_ON(traceJTPComm)) {
					_snprintf(tmpStr, sizeof(tmpStr) - 1, "Wrong State: JTP after last message is not StateCreated");
					ACS_TRA_event (&traceJTPComm, tmpStr);
				}
			}*/
			if (traceObj->ACS_TRA_ON())
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "Wrong State: JTP after last message is not StateCreated");
				traceObj->ACS_TRA_event(1,tmpStr);
			}

			break;
		}

		// Make sure we are in StateWaitForData before using jexdataind
		if(m_conversation->State() != ACS_JTP_Conversation::StateWaitForData) {
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "JTP StateWaitForData failed" << std::endl;
			IssueJTPevent(-2);
			return -1;
		}

		// Get the Data from CP
		m_rec_msg = 0;
		if ( m_conversation->jexdataind(m_rec_data1, m_rec_data2, m_rec_length, m_rec_msg) == false)
		{
			// We assume that we are disconnected and go listening for a new call

			/*	if (ACS_TRA_ON(traceJTPComm))
			{
				ACS_TRA_event(&traceJTPComm,"Maintenance info request by CP is initiated.");
			}*/
			if (traceObj->ACS_TRA_ON())
			{
				traceObj->ACS_TRA_event(1,tmpStr);
			}
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "jexdataind failed" << std::endl;
			IssueJTPevent(-2);
			return -1;
		}

		// Take care of msg from/to JTP
		// Moving from Sleep tp waitforsingleobject.
		DWORD dwreceiveStatus = receiveMsg();
		if(dwreceiveStatus == 0)
		{
			sprintf(problemData,"%s","Process terminated due to a fatal error.");
			/*if (ACS_TRA_ON(traceJTPComm)) {
				ACS_TRA_event (&traceJTPComm, problemData);
			}*/
			if (traceObj->ACS_TRA_ON())
			{
				traceObj->ACS_TRA_event(1,tmpStr);
			}
			m_send_data1 = 0x00F0; // MAINTENANCE LIST INFO
			m_send_data2 = 0;
			m_send_result = 0;
			m_conversation->jexdiscreq(m_send_data1,m_send_data2,m_send_result);
			return -1;
		}

		// Make sure we are in StateJobRunning before using jexdatareq
		if(m_conversation->State() != ACS_JTP_Conversation::StateJobRunning) {
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "JTP StateJobRunning failed" << std::endl;

			// We are disconnected. Inform Eventmanager and update CP object status.

			IssueJTPevent(-2);
			return -1;
		}

		// Send reply to CP
		memset(m_send_buffer, 0xFF, MAINTENANCE_MAX_BUFFER);
		if (boardListLen > MAINTENANCE_FRAME_LEN)
		{
			unsigned short frameType = 0x01; // More Messages follow
			data(frameType, MAINTENANCE_FRAME_LEN);
			boardListLen = boardListLen - MAINTENANCE_FRAME_LEN;
			sendLastMsg = false;
		}
		else
		{
			unsigned short frameType = 0x00; // Last Message of the list
			data(frameType, boardListLen);
			sendLastMsg = true;
		}

		log_JTP_data(boardListLen);
		char *Msg = 0;
		if (boardListLen > 0) Msg = m_send_buffer;
		if ( m_conversation->jexdatareq(m_send_data1, m_send_data2, m_send_length, Msg) == false)
		{
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "jexdatareq failed" << std::endl;

			// We are disconnected. Inform Eventmanager and update CP object status.
			IssueJTPevent(-2);
			return -1;
		}
		else
		{
			/*if (ACS_TRA_ON(traceJTPComm)) {
				_snprintf(tmpStr, sizeof(tmpStr) - 1, "SENT jexdatareq TO THE CP");
				ACS_TRA_event (&traceJTPComm, tmpStr);
			}*/
			if (traceObj->ACS_TRA_ON())
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "SENT jexdatareq TO THE CP");
				traceObj->ACS_TRA_event(1,tmpStr);
			}
		}
	} // End while(StateWaitForData...
	//
	// At this point we do not know if we are disconnected but we know
	// that we want an error code and do a disconnect.
	//
	// Make sure we are in StateJobRunning or StateWaitForData
	if(m_conversation->State() == ACS_JTP_Conversation::StateJobRunning ||
			m_conversation->State() == ACS_JTP_Conversation::StateWaitForData )
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Wrong State: JTP is not StateCreated. CP doesn't change the state" << std::endl;
		/*	if (ACS_TRA_ON(traceJTPComm)) {
			_snprintf(tmpStr, sizeof(tmpStr) - 1, "Wrong State: JTP is not StateCreated. CP doesn't change the state");
			ACS_TRA_event (&traceJTPComm, tmpStr);
		}*/
		if (traceObj->ACS_TRA_ON())
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "Wrong State: JTP is not StateCreated. CP doesn't change the state");
			traceObj->ACS_TRA_event(1,tmpStr);
		}
		m_send_data1 = 0x00F0; // MAINTENANCE LIST INFO
		m_send_data2 = 0;
		m_send_result = 0;
		if(m_conversation->jexdiscreq(m_send_data1,m_send_data2,m_send_result) == false)
		{
			// At this point we realize that we have been disconnected
			// JexDiscInd is the only way to get an error code from JTP
			m_rec_data1 = m_rec_data2 = m_send_result = 0;
			if (m_conversation->jexdiscind(m_rec_data1,m_rec_data2,m_send_result) == false)
			{
				// jexdiscind failed.

				// We are disconnected. Inform Eventmanager and update CP object status.
				IssueJTPevent(-1);
				return -1;
			}
			else {
				// We got an error code from jexdiscind
				// Description:
				//    This is disconnect reception routine that is to be used
				//    when either a client or destinator receives disconnection.
				// Parameters:
				//    U1:OUT         User data 1, which is received from remote side.
				//    U2:OUT         User data 2, which is received from remote side.
				//    Reason:OUT     Reason code, which is received from remote side.
				//                   Reason = 0 means normal disconnect.
				//                   Reason = 1 Congestion in JTP.
				//                   Reason = 2 Breakdown on transport or lower layer.
				//                   Reason = 3 Application not available.
				//                   Reason = (4..65535) is code from application.
				// Return value:
				//    true           The message was received successfully.
				//    false          Receiving of message failed, or faulty state.

				// We are disconnected. Inform Eventmanager and update CP object status.
				IssueJTPevent(m_send_result);
				return -1;
			} // End else {
		} // End if(m_conversation.jexdiscreq
		else
		{
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Wrong State: JTP is not StateCreated. CP doesn't change the state, jexdiscreq failed" << std::endl;
			/*	if (ACS_TRA_ON(traceJTPComm)) {
				_snprintf(tmpStr, sizeof(tmpStr) - 1, "Wrong State: JTP is not StateCreated. CP doesn't change the state, jexdiscreq failed");
				ACS_TRA_event (&traceJTPComm, tmpStr);
		}*/
			if (traceObj->ACS_TRA_ON())
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "Wrong State: JTP is not StateCreated. CP doesn't change the state, jexdiscreq failed");
				traceObj->ACS_TRA_event(1,tmpStr);
			}
		}
	} // End if(m_conversation.State() ==

	return -1;
}

void FIXS_CCH_JTPComm::IssueJTPevent (int code)
{

	char	problemData [PROBLEMDATASIZE];

	switch (code) {
	case -2:
		sprintf(problemData,"%s","CP Connection Broken. Reconnecting...`");
		break;
	case -1:
		sprintf(problemData,"%s%s","jexdiscind failed. `","CP Connection Broken. Reconnecting...`");
		break;
	case 0:
		sprintf(problemData,"%s%s","Disconnected from CP. `","Reconnecting...`");
		break;
	case 1:
		sprintf(problemData,"%s%s","Congestion in JTP. `","CP Connection Broken. Reconnecting...`");
		break;
	case 2:
		sprintf(problemData,"%s%s","Breakdown on transport or lower layer. `","CP Connection Broken. Reconnecting...`");
		break;
	case 3:
		sprintf(problemData,"%s%s","Application not available. `","CP Connection Broken. Reconnecting...`");
		break;
	default:
		// APMA broke the connection.
		sprintf(problemData,"%s%d%s","disc code from application: `",m_send_result," CP Connection Broken. Reconnecting...`");
		break;
	}

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << problemData << std::endl;
	/*if (ACS_TRA_ON(traceJTPComm)) {
		ACS_TRA_event (&traceJTPComm, problemData);
	}*/

}

DWORD FIXS_CCH_JTPComm::receiveMsg ()
{

//	char tmpStr[512] = {0};
	int result = 0; // NOT SUCCESS;


	// Trace point.
	/*	if (ACS_TRA_ON (traceJTPComm))
	{
		ACS_TRA_event (&traceJTPComm, "Executing `receiveMsg'");
	}
	if (ACS_TRA_ON (ACS_CHB_JTPdata))
	{
		log_JTP_data((unsigned char *)rec_msg,rec_length,rec_data1,rec_data2,0);
	}*/

	// MAINTENANCE LIST REQUEST
	if (m_rec_data1 == 0x00F0)
	{
		/*	if (ACS_TRA_ON(traceJTPComm)) {
			_snprintf(tmpStr, sizeof(tmpStr) - 1, "MAINTENANCE LIST REQUEST");
			ACS_TRA_event (&traceJTPComm, tmpStr);
		}*/

		result = 1; // SUCCESS
	}
	else
	{
		/*if (ACS_TRA_ON(traceJTPComm)) {
			_snprintf(tmpStr, sizeof(tmpStr) - 1, "RECEIVED: THE CODE IS NOT FOR MAINTENANCE LIST");
			ACS_TRA_event (&traceJTPComm, tmpStr);
		}*/

		result = 0; // NOT SUCCESS
	}

	return result;
}

void FIXS_CCH_JTPComm::data (unsigned short frameType, unsigned short frameLen)
{
//	char tmpStr[512] = {0};

	/*	if (ACS_TRA_ON(traceJTPComm))
	{
		ACS_TRA_event (&traceJTPComm, "Executing `data'");
	}

	if (ACS_TRA_ON(traceJTPComm)) {
		_snprintf(tmpStr, sizeof(tmpStr) - 1, "frameLen = %d",frameLen);
		ACS_TRA_event (&traceJTPComm, tmpStr);
	}
	 */

	if (traceObj->ACS_TRA_ON())
	{
		traceObj->ACS_TRA_event(1,"Executing `data'");
	}
	if (traceObj->ACS_TRA_ON())
	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "frameLen = %d",frameLen);
		traceObj->ACS_TRA_event(1,tmpStr);
	}

	// Send data
	m_send_data1 = frameType << 8 | 0xF1; // Maintenance List Info
	m_send_data2 = frameLen;
	if (frameLen > 0)
	{
		//m_send_length = frameLen * 3 + 1;
		m_send_length = MAINTENANCE_MAX_BUFFER;  //m_send_length = 0x0040 (64 bytes)
	}
	else
	{
		//m_send_length = 0xFFFF; // Buffer not used
		m_send_length = 0; // Buffer not used
	}
	// The data to send is a buffer with the following format:
	// [0]   = Shelf no of first SCXB
	// [1]   = Slot no of first SCXB
	// [2]   = 0x10 (Power Upgrade), 0x11 (Fan Upgrade), 0x20 (IPMI FW Upgrade), 0x21 (SW Upgrade)
	// . . .
	// [(m-1)*3]   = Shelf no of m-th SCXB
	// [(m-1)*3+1] = Slot no of m-th SCXB
	// [(m-1)*3+2] = code as above
	// . . .
	// [(N-1)*3]   = Shelf no of the last SCXB
	// [(N-1)*3+1] = Slot no of the last SCXB
	// [(N-1)*3+2] = code as above
	// . . .
	// [(N-1)*3+3 up to 63] = 0xFF (SPARE)
	for (int i = 0; i < frameLen; i++)
	{
		m_send_buffer[i*3] = m_listOfMaitenance[m_iteratorCount].mag;
		m_send_buffer[i*3 + 1] = m_listOfMaitenance[m_iteratorCount].slot;
		m_send_buffer[i*3 + 2] = m_listOfMaitenance[m_iteratorCount].type;
		m_iteratorCount++;
	}

	return;
}

// Additional Declarations

void FIXS_CCH_JTPComm::log_JTP_data(int boardListLen)
{
	char tmpStr[512] = {0};

	snprintf(tmpStr, sizeof(tmpStr) - 1, "m_send_data1: %d, m_send_data2: %d, m_send_length: %d, BUFFER", m_send_data1, m_send_data2, m_send_length);
	if (boardListLen > 0)
	{
		char item[6];
		for (int i = 0; i < m_send_length; i++)
		{
			snprintf(item, sizeof(item) - 1, "0x%02X",m_send_buffer[i]);
			item[6] = 0;
			strcat(tmpStr, ",");
			strcat(tmpStr, item);
		}
	}
	else
	{
		strcat(tmpStr, " NOT USED");
	}

	//if (ACS_TRA_ON(traceJTPComm)) ACS_TRA_event(&traceJTPComm, tmpStr);

	if (traceObj->ACS_TRA_ON())
	{
		traceObj->ACS_TRA_event(1,tmpStr);

	}
}
