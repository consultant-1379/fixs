/*
 * FIXS_TREFC_JTPClient.cpp
 *
 *  Created on: Jan 24, 2012
 *      Author: eiolbel
 */

#include <iostream>
#include <sstream>
#include "FIXS_TREFC_JTPHandler.h"
#include "FIXS_TREFC_JTPClient.h"

//## end module%4E3BDB3603A9.additionalDeclarations


// Class FIXS_TREFC_JTPClient

//## begin FIXS_TREFC_JTPClient::JOB_REJECTED%4E3BE2320084.attr preserve=no  public: static  int {UC} -1
const  int FIXS_TREFC_JTPClient::JOB_REJECTED = -1;
//## end FIXS_TREFC_JTPClient::JOB_REJECTED%4E3BE2320084.attr

//## begin FIXS_TREFC_JTPClient::JOB_FAILED%4E3BE2BC0219.attr preserve=no  public: static  int {UC} -2
const  int FIXS_TREFC_JTPClient::JOB_FAILED = -2;
//## end FIXS_TREFC_JTPClient::JOB_FAILED%4E3BE2BC0219.attr

//## begin FIXS_TREFC_JTPClient::JOB_MISMATCH%4E3BE2DF0047.attr preserve=no  public: static  int {UC} -3
const  int FIXS_TREFC_JTPClient::JOB_MISMATCH = -3;
//## end FIXS_TREFC_JTPClient::JOB_MISMATCH%4E3BE2DF0047.attr

FIXS_TREFC_JTPClient::FIXS_TREFC_JTPClient (JTP_HANDLE aShutDown)
: m_shutdown(0)
{
	//## begin FIXS_TREFC_JTPClient::FIXS_TREFC_JTPClient%4E3BDB9002F6.body preserve=yes
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " "<< std::endl;
	m_shutdown = aShutDown;
	traceObj = new ACS_TRA_trace("FIXS_TREFC_JTPClient");
	//## end FIXS_TREFC_JTPClient::FIXS_TREFC_JTPClient%4E3BDB9002F6.body
}


FIXS_TREFC_JTPClient::~FIXS_TREFC_JTPClient()
{
	//## begin FIXS_TREFC_JTPClient::~FIXS_TREFC_JTPClient%4E3BDB3603A9_dest.body preserve=yes
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " "<< std::endl;
	//Delete trace object
	if (traceObj) {
		delete (traceObj);
		traceObj = NULL;
	}
	//## end FIXS_TREFC_JTPClient::~FIXS_TREFC_JTPClient%4E3BDB3603A9_dest.body
}



//## Other Operations (implementation)
int FIXS_TREFC_JTPClient::newJTPClient (unsigned short Len, ACS_JTP_Job &J1, unsigned short U1, unsigned short U2, char *Msg)
{
	//## begin FIXS_TREFC_JTPClient::newJTPClient%4E3BDD310333.body preserve=yes
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " In"<< std::endl;
	int OKRequest = 0;
	//char *RMsg = 0;
	unsigned short R, RU1, RU2;
	//char tmpStr[512] = {0};

//	ACS_JTP_Job::JTP_Node* pNode = 0;
//	pNode = new ACS_JTP_Job::JTP_Node;

	ACS_JTP_Job::JTP_Node pNode;

	//---------------------------------------------------------------------------------------------------------
	//Create vector for reachable nodes.
	std::vector<ACS_JTP_Job::JTP_Node> nodes;

	//Create vector for unreachable nodes.
	std::vector<ACS_JTP_Job::JTP_Node> notReachNodes;

	cout << "\nCalling query method to get the reachable and unreachable nodes from DSD\n";
	//Find out the set on reachable and unreachable nodes.
	J1.query( ACS_JTP_Job::SYSTEM_TYPE_CP, nodes, notReachNodes);

	cout << "\nPrinting the resulting of query method\n";
	//printing the set of reachable nodes.
	for ( unsigned int ctr = 0 ; ctr < nodes.size() ; ctr++)
	{
		cout << "\nReachable Node:"<< ctr << "\n";
		cout << "Node.system_id = "<< nodes[ctr].system_id << "\n";
		cout << "Node.system_type = "<< nodes[ctr].system_type << "\n";
		cout << "Node.node_state = " << nodes[ctr].node_state << "\n";
		cout << "Node.node_name = " << nodes[ctr].node_name << "\n";
		cout << "Node.system_name = " << nodes[ctr].system_name << "\n";
		cout << "Node.node_side = " <<  nodes[ctr].node_side << "\n";
	}

	//printing the set of unreachable nodes.
//	for(unsigned int ctr = 0 ; ctr < notReachNodes.size(); ctr++)
//	{
//		cout << "\nUnReachable Node: " << ctr <<"\n";
//		cout << "Node.system_id = " << notReachNodes[ctr].system_id << "\n";
//		cout << "Node.system_type = " << notReachNodes[ctr].system_type << "\n";
//		cout << "Node.node_state = " << notReachNodes[ctr].node_state << "\n";
//		cout << "Node.node_name = " << notReachNodes[ctr].node_name << "\n";
//		cout << "Node.system_name = " << notReachNodes[ctr].system_name << "\n";
//		cout << "Node.node_side = " << notReachNodes[ctr].node_side << "\n";
//	}
	//---------------------------------------------------------------------------------------------


	//pNode->systemId = ACS_JTP_Job::ALARM_MASTER;
	//pNode->systemId = 1001;
	if (nodes.size() > 0) {
		pNode.system_id = nodes[0].system_id;
		pNode.node_state = nodes[0].node_state;
		pNode.system_type = nodes[0].system_type;
		strcpy(pNode.node_name , nodes[0].node_name);
		strcpy(pNode.system_name, nodes[0].system_name );
		pNode.node_side = nodes[0].node_side ;
	} else {
		OKRequest = JOB_REJECTED;
	}


	if (OKRequest == 0) {
		if (J1.jinitreq(&pNode, U1, U2, Len, Msg) == false)
		{// This case is for Job Rejected
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " This case is for Job Rejected"<< std::endl;
			J1.jfault(RU1, RU2, R);
			//std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "(jfault) The job did not initiate" << std::endl;
			std::stringstream ss;
			ss << "(jfault) The job did not initiate" << " ";
			ss << "U1 = " << U1 << ", RU1 = " << RU1 << " ";
			ss << "U2 = " << U2 << ", RU2 = " << RU2 << " ";
			ss << "R = " << R;
			if (traceObj->ACS_TRA_ON())
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d]", __FUNCTION__, __LINE__);
				traceObj->ACS_TRA_event(1,tmpStr);
			}
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<
					" jfault =  " << ss.str().c_str() << std::endl;

			if (R == 30) // 30 is 0x001E
			{
				// Protocol version Mismatch.
				OKRequest = JOB_MISMATCH;
			}
			else
			{
				OKRequest = JOB_REJECTED;
			}
		}
	}

//	delete pNode;
//	pNode = 0;

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " Out"<< std::endl;
	return OKRequest;
	//## end FIXS_TREFC_JTPClient::newJTPClient%4E3BDD310333.body
}

int FIXS_TREFC_JTPClient::waitForConfermation (ACS_JTP_Job &J1, unsigned short U1, unsigned short U2)
{
	//## begin FIXS_TREFC_JTPClient::waitForConfermation%4E3BE08F0342.body preserve=yes
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " In"<< std::endl;
	int OKConfermation = 0;
	unsigned short R, RU1, RU2;
	//char tmpStr[512] = {0};

	// jinitconf can not fail if jexinitreq returns true
	J1.jinitconf(RU1, RU2, R);
	// Do something with the result if you like
	if ( R != 0 || (U1 != RU1) || (U2 != RU2))
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " jinitconf, The data is modified" << std::endl;
		std::stringstream ss;
		ss << "U1 = " << U1 << ", RU1 = " << RU1 << " ";
		ss << "U2 = " << U2 << ", RU2 = " << RU2 << " ";
		ss << "R = " << R << " ";
		ss << "jinitconf, The data is modified";
		if (traceObj->ACS_TRA_ON())
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d]", __FUNCTION__, __LINE__);
			traceObj->ACS_TRA_event(1,tmpStr);
		}
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<
				" jfault =  " << ss.str().c_str() << std::endl;

		if (R == 30) // 30 is 0x001E
		{
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " Protocol version Mismatch"<< std::endl;
			// Protocol version Mismatch.
			OKConfermation = JOB_MISMATCH;
		}
		else
		{
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " "<< std::endl;
			OKConfermation = JOB_REJECTED;
		}
	}
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " Out"<< std::endl;
	return OKConfermation;
	//## end FIXS_TREFC_JTPClient::waitForConfermation%4E3BE08F0342.body
}

bool FIXS_TREFC_JTPClient::waitForJTPResult (ACS_JTP_Job &J1, unsigned short Len, unsigned short U1, unsigned short U2, char *Msg)
{
	//## begin FIXS_TREFC_JTPClient::waitForJTPResult%4E3BE1360111.body preserve=yes
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " "<< std::endl;
	bool OKResult = true;
	char *RMsg = 0;
	unsigned short R, RU1, RU2, RLen, canstartRes = 0;
	TREFC_Util::EnvironmentType environment = TREFC_Util::getEnvironment();

	//	char tmpStr[512] = {0};

	//HANDLE PollFd;
	JTP_HANDLE PollFd;
	//HANDLE fd = J1.getHandle();
	JTP_HANDLE fd = J1.getHandle();
	PollFd = fd;
	//DWORD status;
	int status;
	
	if(environment == TREFC_Util::SMX) {
		canstartRes = SMX_URC;
	}
	else {
		canstartRes = SCX_URC;
	}
	canstartRes = canstartRes << 8;
	canstartRes = canstartRes | 0x0000;

	//HANDLE * waitHandles = new HANDLE[2];
	JTP_HANDLE * waitHandles = new JTP_HANDLE[2];
	waitHandles[0] = PollFd;
	waitHandles[1] = m_shutdown;
	const int WAIT_TIMEOUT= -2;
	const int WAIT_OBJECT_0=0;

	status=FIXS_TREFC_Event::WaitForEvents(2, waitHandles, 60000);

	delete [] waitHandles;
	if (status == WAIT_OBJECT_0) // PollFd event
	{
		bool res = J1.jresultind(RU1, RU2, R, RLen, RMsg);
		if( res && (FIXS_TREFC_JTPHandler::TIME_SERVICE_CAN_START == RU1))
		{
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " check RU2 in jresultind for canstart query" << std::endl;
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " RU2 :"<<RU2<<"\t canstartRes :"<<canstartRes<<"\t NO_URC :"<<NO_URC<<std::endl;
			if(RU2 != NO_URC && RU2 != canstartRes)
				OKResult = false;
		}
		else if (res == false || R != 0)
		{// These case are for Job Failed and Job Timeout. Result code of Job Failed covers also Job Timeout.
			J1.jfault(RU1, RU2, R);
			//std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "(jfault) The job did not complete" << std::endl;
			std::stringstream ss;
			ss << "(jfault) The job did not complete" << " ";
			ss << "jresultind, was expected" << " ";
			ss << "R = " << R;
			if (traceObj->ACS_TRA_ON())
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d]", __FUNCTION__, __LINE__);
				traceObj->ACS_TRA_event(1,tmpStr);
			}

			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<
					" jfault =  " << ss.str().c_str() << std::endl;

			OKResult = false;
		}
		else if ((U1 != RU1) ||(U2 != RU2) || (Len != RLen) // Do something with the data
				|| (memcmp(Msg, RMsg, Len) != 0))
		{
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " jresultind, The data is modified" << std::endl;
			std::stringstream ss;
			ss << "U1 = " << U1 << ", RU1 = " << RU1 << " ";
			ss << "U2 = " << U2 << ", RU2 = " << RU2 << " ";
			ss << "Len = " << Len << ", RLen = " << RLen << " ";
			ss << "jresultind, The data is modified";
			std::cout<<ss.str();
			if (traceObj->ACS_TRA_ON())
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d]", __FUNCTION__, __LINE__);
				traceObj->ACS_TRA_event(1,tmpStr);
			}
			OKResult = false;
		}
	}
	else if (status == WAIT_OBJECT_0 + 1) // Shutdown event
	{
		J1.jtermreq();
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " Shutdown signalled" << std::endl;
		std::stringstream ss;
		ss << "jresultind, Shutdown signalled";
		if (traceObj->ACS_TRA_ON())
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d]", __FUNCTION__, __LINE__);
			traceObj->ACS_TRA_event(1,tmpStr);
		}
	}
	else if (status == WAIT_TIMEOUT) // Timeout event
	{
		J1.jtermreq();
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " jresultind, The call timed out" << std::endl;
		std::stringstream ss;
		ss << "jresultind, The call timed out";
		if (traceObj->ACS_TRA_ON())
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d]", __FUNCTION__, __LINE__);
			traceObj->ACS_TRA_event(1,tmpStr);
		}
	}
	else //unknown error of event
	{
		J1.jtermreq();
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "jresultind, unknown error signalled" << std::endl;
		std::stringstream ss;
		ss << "jresultind, unknown error signalled";
		if (traceObj->ACS_TRA_ON())
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d]", __FUNCTION__, __LINE__);
			traceObj->ACS_TRA_event(1,tmpStr);
		}

		OKResult = false;
	}

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " "<< std::endl;
	return OKResult;
	//## end FIXS_TREFC_JTPClient::waitForJTPResult%4E3BE1360111.body
}

// Additional Declarations

