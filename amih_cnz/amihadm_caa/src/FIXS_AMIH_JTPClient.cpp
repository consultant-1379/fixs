//	*****************************************************************************
//
//	.NAME
//	    FIXS_AMIH_JTPClient
//	.LIBRARY 3C++
//	.PAGENAME FIXS_CCH
//	.HEADER  FIXS_CCH
//
//	.COPYRIGHT
//	 COPYRIGHT Ericsson 2010.
//	 All rights reserved.
//
//	The Copyright to the computer program(s) herein
//	is the property of Ericsson 2009.
//	The program(s) may be used and/or copied only with
//	the written permission from Ericsson 2009 or in
//	accordance with the terms and conditions stipulated in the
//	agreement/contract under which the program(s) have been
//	supplied.
//
//	.DESCRIPTION
//	   See below
//
//	*****************************************************************************


#include "ACS_TRA_trace.h"
#include <iostream>
#include <sstream>
#include "FIXS_AMIH_Event.h"
#include "FIXS_AMIH_Util.h"

//## end module%4C08D20B0368.includes

// FIXS_AMIH_JTPClient
#include "FIXS_AMIH_JTPClient.h"
//## begin module%4C08D20B0368.additionalDeclarations preserve=yes
#include "FIXS_AMIH_JTPHandler.h"

// Class FIXS_AMIH_JTPClient

FIXS_AMIH_JTPClient::FIXS_AMIH_JTPClient ()
{
	_trace = new ACS_TRA_trace("FIXS_AMIH_JTPClient");
}


FIXS_AMIH_JTPClient::~FIXS_AMIH_JTPClient()
{
}

 int FIXS_AMIH_JTPClient::newJTPClient (unsigned short Len, ACS_JTP_Job &J1, unsigned short U1, unsigned short U2, char *Msg)
{
	int OKRequest = 0;

	unsigned short R, RU1, RU2;
	
	ACS_JTP_Job::JTP_Node* pNode;
	pNode = new ACS_JTP_Job::JTP_Node;
	
	// pNode->system_id = ACS_JTP_Job::SYSTEM_ID_CP_ALARM_MASTER; ??????????

	pNode->system_id = 1001;  // SINGLE CP

	if (J1.jinitreq(pNode, U1, U2, Len, Msg) == false)
	{
		// This case is for Job Rejected
		J1.jfault(RU1, RU2, R);

		std::stringstream ss;
		ss << "(jfault) The job did not initiate" << " ";
		ss << "U1 = " << U1 << ", RU1 = " << RU1 << " ";
		ss << "U2 = " << U2 << ", RU2 = " << RU2 << " ";
		ss << "R = " << R << " ";

		printTRA(ss.str());

		if (R == 30) // 30 is 0x001E
		{
			// Protocol version Mismatch.
			OKRequest = FIXS_AMIH_JTPHandler::JOB_MISMATCH;
		}
		else 
		{
			OKRequest = FIXS_AMIH_JTPHandler::JOB_REJECTED;
		}
	}
	delete pNode;
	
	return OKRequest;
}


 bool FIXS_AMIH_JTPClient::waitForJTPResult (ACS_JTP_Job &J1, unsigned short Len, unsigned short U1, unsigned short U2, char *Msg)
 {
	 bool OKResult = true;
	 char *RMsg = 0;
	 unsigned short R, RU1, RU2, RLen;

	 int PollFd = J1.getHandle();

	 int status;

	 int number_of_waitHandles=2;

	 int waitHandles[number_of_waitHandles];

	 waitHandles[0] =PollFd;
	 waitHandles[1]= FIXS_AMIH_Event::OpenNamedEvent(AMIH_Util::EVENT_NAME_SHUTDOWN);

	 const int WAIT_SHUTDOWN	=  1;
	 const int WAIT_TIMEOUT		= -2;
	 const int WAIT_POOLFD		=  0;

	 status = FIXS_AMIH_Event::WaitForEvents(number_of_waitHandles,waitHandles,60000);

	 if (status == WAIT_POOLFD) // PollFd event
	 {
		 bool res = J1.jresultind(RU1, RU2, R, RLen, RMsg);

		 if (res == false || R != 0)
		 {
			 // These case are for Job Failed and Job Timeout. Result code of Job Failed covers also Job Timeout.

			 J1.jfault(RU1, RU2, R);

			 std::string mesg="(jfault) The job did not complete jresultind, was expected ";
			 std::stringstream ss;
			 ss << mesg << ss << "R = " << R << " ";

			 printTRA(ss.str());

			 OKResult = false;
		 }
		 else if ((U1 != RU1) ||(U2 != RU2) || (Len != RLen) // Do something with the data
				 || (memcmp(Msg, RMsg, Len) != 0))
		 {

			 std::stringstream ss;

			 ss << "U1 = " << U1 << ", RU1 = " << RU1 << " ";
			 ss << "U2 = " << U2 << ", RU2 = " << RU2 << " ";
			 ss << "Len = " << Len << ", RLen = " << RLen << " ";
			 ss << "jresultind, The data is modified" << " ";

			 printTRA(ss.str());

			 OKResult = false;
		 }

	 }
	 else if (status == WAIT_SHUTDOWN) // Shutdown event
	 {
		 J1.jtermreq();

		 std::stringstream ss;

		 ss << "jresultind, Shutdown signalled " << " ";

		 printTRA(ss.str());
	 }
	 else if (status == WAIT_TIMEOUT) // Timeout event
	 {
		 J1.jtermreq();

		 std::stringstream ss;

		 ss << "jresultind, The call timed out" << " ";

		 printTRA(ss.str());
	 }
	 else //unknown error of event
	 {
		 J1.jtermreq();

		 std::stringstream ss;

		 ss << "jresultind, unknown error signalled " << " ";

		 printTRA(ss.str());

		 OKResult = false;
	 }

	 return OKResult;
 }


 int FIXS_AMIH_JTPClient::waitForConfermation (ACS_JTP_Job &J1, unsigned short U1, unsigned short U2)
{
	int OKConfermation = 0;
	unsigned short R, RU1, RU2;
	
	J1.jinitconf(RU1, RU2, R);

	// Do something with the result if you like

	if ( R != 0 || (U1 != RU1) || (U2 != RU2))
	{
		//std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "jinitconf, The data is modified" << std::endl;
		std::stringstream ss;
		ss << "U1 = " << U1 << ", RU1 = " << RU1 << " ";
		ss << "U2 = " << U2 << ", RU2 = " << RU2 << " ";
		ss << "R = " << R << " ";
		ss << "jinitconf, The data is modified" << " ";

		printTRA(ss.str());

		if (R == 30) // 30 is 0x001E
		{
			OKConfermation = FIXS_AMIH_JTPHandler::JOB_MISMATCH;
		}
		else
		{
			OKConfermation = FIXS_AMIH_JTPHandler::JOB_REJECTED;
		}
	}
	return OKConfermation;
}

 void FIXS_AMIH_JTPClient::printTRA(std::string mesg)
 {
 	if (_trace->ACS_TRA_ON())
 	{
 		char tmpStr[512] = {0};
 		snprintf(tmpStr, sizeof(tmpStr) - 1, mesg.c_str());
 		_trace->ACS_TRA_event(1, tmpStr);

 	}
 }
