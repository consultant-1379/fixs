/*
 * FIXS_TREFC_JTPComm.cpp
 *
 *  Created on: Jan 24, 2012
 *      Author: eiolbel
 */

#include <iostream>
#include <stdio.h>
#include "FIXS_TREFC_JTPHandler.h"
#include "FIXS_TREFC_JTPComm.h"

namespace {
const int VERSION = 1;
}
#ifndef __
#define __ std::cout << __FUNCTION__ << "@" << __LINE__ << std::endl;
#endif
//## end module%4E4E0D180355.additionalDeclarations


// Class FIXS_TREFC_JTPComm

FIXS_TREFC_JTPComm::FIXS_TREFC_JTPComm (unsigned short aSystemid, ACS_JTP_Job* job)
: m_systemid(0),
  m_conversation(0),
  m_rec_msg(0),
  m_rec_length(0),
  m_rec_data1(0),
  m_rec_data2(0),
  m_send_length(0),
  m_send_data1(0),
  m_send_data2(0),
  m_send_result(0)
{

	m_systemid = aSystemid;
	m_conversation = job;
	m_send_buffer = new char[MAINTENANCE_MAX_BUFFER];
	m_rec_msg = NULL; //pointer to JTP internal buffer
	traceObj = new ACS_TRA_trace("FIXS_TREFC_JTPComm");
}


FIXS_TREFC_JTPComm::~FIXS_TREFC_JTPComm()
{
	//## begin FIXS_TREFC_JTPComm::~FIXS_TREFC_JTPComm%4E4E0D180355_dest.body preserve=yes
	if (m_send_buffer != NULL) delete [] m_send_buffer;

	//Delete trace object
	if (traceObj) {
		delete (traceObj);
		traceObj = NULL;
	}
	//## end FIXS_TREFC_JTPComm::~FIXS_TREFC_JTPComm%4E4E0D180355_dest.body
}



//## Other Operations (implementation)
void FIXS_TREFC_JTPComm::jobMode ()
{
	//## begin FIXS_TREFC_JTPComm::jobMode%4E4E23EA0385.body preserve=yes
	__;
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "\tEntry ......" << std::endl;
	//char tmpStr[512] = {0};
	//char problemData [PROBLEMDATASIZE];   // Data for event report.
	ACS_JTP_Job::JTP_Node node;  // Structure node with CP systemId.
	//node.systemId = m_systemid;
	node.system_id = m_systemid;
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "New call received!" << std::endl;

	// Try to initiate.
	if (m_conversation->State() != ACS_JTP_Job::StateConnected)
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "JTP StateConnected failed" << std::endl;
		// Unable to connect to APJTP. Conversation stare is not connected.
		// Write a message in the Eventlog.
		if (traceObj->ACS_TRA_ON())
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] APJTP State not connected. ", __FUNCTION__, __LINE__);
			traceObj->ACS_TRA_event(1,tmpStr);
		}
		//		if (ACS_TRA_ON(traceJTPComm))
		//		{
		//			_snprintf(problemData, sizeof(problemData) - 1, "%s","APJTP State not connected. `");
		//			ACS_TRA_event(&traceJTPComm, problemData);
		//		}

		return;
	}
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "\tJTP StateConnected ok" << std::endl;
	// OK, we are Connected, send jexinitind
	if (m_conversation->jinitind(node, m_rec_data1, m_rec_data2, m_rec_length, m_rec_msg) == false)
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "jinitind failed" << std::endl;
		return;
	}
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "\t jinitind ok" << std::endl;

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "JTP connection initiated!" << std::endl;

	m_send_data1 = VERSION << 8 | FIXS_TREFC_JTPHandler::TIME_SERVICE_STATUS_REQUEST; // Maintenance List Info
	m_send_data2 = 0;
	m_send_result = 0;
	std::cout<<" FIXS_TREFC_JTPComm::jobMode..... m_send_data1 :"<< m_send_data1 <<"\t m_rec_data1 :"<<m_rec_data1<<__LINE__<<std::endl;
	if( m_rec_data1 == m_send_data1) // check the request from CP
	{
		std::cout<<" jinitrsp.....U1 :"<< m_send_data1 <<"\t U2 :"<<m_send_data2<<__LINE__<<std::endl;

		//m_send_data1 = 0x0100 | FIXS_TREFC_JTPHandler::TIME_SERVICE_STATUS_REQUEST; // Maintenance List Info
		// Send jinitrsp
		if (m_conversation->jinitrsp(m_send_data1, m_send_data2, m_send_result) == false)
		{
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "jexinitrsp failed" << std::endl;
			return;
		}
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "\tjexinitrsp ok" << std::endl;

		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Connected - Get data!" << std::endl;

		if(m_conversation->State() == ACS_JTP_Job::StateJobRunning)
		{
			// Send the Data to CP
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "\tStateJobRunning" << std::endl;
			char *Msg = 0;
			m_rec_msg = 0;
			setData();
			if(m_send_buffer)	Msg = m_send_buffer;
			std::cout<<" jresultreq.... U1 :"<< m_send_data1 <<"\t U2 :"<<m_send_data2<<__LINE__<<std::endl;
			if ( m_conversation->jresultreq(m_send_data1, m_send_data2, m_send_result,m_send_length, Msg) == false)
			{
				// We assume that we are disconnected and go listening for a new call

				if (traceObj->ACS_TRA_ON())
				{
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Maintenance info request by CP is initiated. ", __FUNCTION__, __LINE__);
					traceObj->ACS_TRA_event(1,tmpStr);
				}

				//				if (ACS_TRA_ON(traceJTPComm))
				//				{
				//					ACS_TRA_event(&traceJTPComm,"Maintenance info request by CP is initiated.");
				//				}
				std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "jresultreq failed" << std::endl;
				return;
			}
			else
				std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "jresultreq ok" << std::endl;
		}
	}
	else
	{
		m_send_result = 1; // Job rejected
		if (m_conversation->jinitrsp(m_send_data1, m_send_data2, m_send_result) == false)
		{
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "jexinitrsp failed" << std::endl;
			return;
		}
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "\tjexinitrsp ok but job is rejected" << std::endl;
	}
	//## end FIXS_TREFC_JTPComm::jobMode%4E4E23EA0385.body
}

void FIXS_TREFC_JTPComm::setData ()
{
	//## begin FIXS_TREFC_JTPComm::setData%4E4E240B0018.body preserve=yes
	__;
	int serviceType = 0, serviceStatus = 0, res = 0;
	unsigned short i = 0;
	std::map<std::string,std::string> tRefList;
	FIXS_TREFC_Manager::getInstance()->getTrefcData(serviceStatus,serviceType,tRefList);
	//	if (ACS_TRA_ON(traceJTPComm))
	//	{
	//		ACS_TRA_event (&traceJTPComm, "Executing `data'");
	//	}

	m_send_data1 = 0x0000;
	m_send_data1 = m_send_data1 | FIXS_TREFC_JTPHandler::TIME_SERVICE_STATUS_REQUEST; // Maintenance List Info

	if(serviceStatus == 0)
		m_send_data2 = 0x00;		// ENABLED
	else
		m_send_data2 = 0x01;		// DISABLED

	m_send_data2 = m_send_data2 << 8;

	if(serviceStatus == 0)  // Time service is Enabled
	{
		if( serviceType == 0)
			m_send_data2 = m_send_data2 | 0x0000;		// NTP
		else if( serviceType == 1)
			m_send_data2 = m_send_data2 | 0x0001;		// NTPHP
		else if( serviceType == 2)
					m_send_data2 = m_send_data2 | 0x0002; // NTPHP with GPS

		unsigned char scxMag ;
		bool slot0 = false ,slot25=false;
		res = FIXS_TREFC_Manager::getInstance()->GetAPZSwitchMagazineSlot(scxMag,slot0,slot25);

		if ( slot0 && slot25 )
			m_send_buffer[0] = 0x02;
		else if( slot0 || slot25 )
			m_send_buffer[0] = 0x01;
		else
			m_send_buffer[0] = 0x00;

		if ( slot0 && slot25 )
		{
			m_send_buffer[1] = scxMag;
			m_send_buffer[2] = (char) 0x00; // slot-0
			m_send_buffer[3] = scxMag;
			m_send_buffer[4] = (char) 0x19; // slot-25
		}
		else if( slot0 || slot25 )
		{
			m_send_buffer[1] = scxMag;
			if( slot0 )
				m_send_buffer[2] = (char) 0x00; // slot-0
			else if( slot25 )
				m_send_buffer[2] = (char) 0x19; // slot-25
			m_send_buffer[3] = (char) 0x00;
			m_send_buffer[4] = (char) 0x00;
		}
		else
		{
			m_send_buffer[1] = (char) 0x00;
			m_send_buffer[2] = (char) 0x00;
			m_send_buffer[3] = (char) 0x00;
			m_send_buffer[4] = (char) 0x00;
		}

		m_send_buffer[5] = (int)tRefList.size();

		std::map<std::string, std::string>::iterator it;
		unsigned long ipValue = 0;
		if( tRefList.empty()) m_send_length = 6;
		else
		{
			for (i=6,it = tRefList.begin(); it != tRefList.end() ; it++)
			{
				TREFC_Util::checkIPAddresses(it->first.c_str(),ipValue);
				m_send_buffer[i++] = ((unsigned char)(ipValue >> 24)) & 0xFF;
				m_send_buffer[i++] = ((unsigned char)(ipValue >> 16)) & 0xFF;
				m_send_buffer[i++] = ((unsigned char)(ipValue >> 8)) & 0xFF;
				m_send_buffer[i++] = ((unsigned char)ipValue) & 0xFF;
			}
			m_send_length = i ;
		}
	}
	else
	{
		m_send_length = 0;
	}
	std::cout<<" Buffer length :"<<m_send_length<<std::endl;
}

// Additional Declarations

