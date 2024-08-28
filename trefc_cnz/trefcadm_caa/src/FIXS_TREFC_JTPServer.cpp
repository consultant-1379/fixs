/*
 * FIXS_TREFC_JTPServer.cpp
 *
 *  Created on: Jan 24, 2012
 *      Author: eiolbel
 */


#include "FIXS_TREFC_JTPServer.h"

const int WAIT_FAILED=-1;
const int WAIT_TIMEOUT= -2;
const int WAIT_OBJECT_0=0;

#ifndef __
#define __ std::cout << __FUNCTION__ << "@" << __LINE__ << std::endl;
#endif



// Class FIXS_TREFC_JTPServer

FIXS_TREFC_JTPServer::FIXS_TREFC_JTPServer()
: m_exit(true),m_hStopGracefully(0)
{
	traceObj = new ACS_TRA_trace("FIXS_TREFC_JTPServer");
}


FIXS_TREFC_JTPServer::~FIXS_TREFC_JTPServer()
{
	//Delete trace object
	if (traceObj) {
		delete (traceObj);
		traceObj = NULL;
	}
}


FIXS_TREFC_JTPServer & FIXS_TREFC_JTPServer::operator=(const FIXS_TREFC_JTPServer &right)
{
	this->m_hStopGracefully = right.m_hStopGracefully;
	return *this;
}



//## Other Operations (implementation)
int FIXS_TREFC_JTPServer::waitUntilSignaled (int no, JTP_HANDLE *mixedHandles)
{
	// The mixedHandles array can contain a mix of valid
	// and invalid handles. Copy all valid handles to an
	// array with only valid handles.
	// The last handle is the StopEvent
	//char problemText [192] = {0};
	JTP_HANDLE validHandles[no+1];
	int validIndex = 0;
	int mixedIndex;
	for (mixedIndex = 0; mixedIndex < no; mixedIndex++)
	{
		//if (mixedHandles[mixedIndex] != INVALID_HANDLE_VALUE)
		if (mixedHandles[mixedIndex] != -1)
		{
			validHandles[validIndex++] = mixedHandles[mixedIndex];
		}
	}

	// Check if there are any valid handles in the array
	if (validIndex == 0)
	{
		return 0;
	}

	// Add the StopEvent at the end of the array
	validHandles[validIndex++] = m_hStopGracefully;

	// Wait until one object becomes signaled
	while (!m_exit) // Set to true when the thread shall exit
	{

		int status = FIXS_TREFC_Event::WaitForEvents(validIndex,validHandles,100);

		if (status == WAIT_TIMEOUT)
		{
			//do nothing
			//just check thread status
		}
		else if ( (status >= WAIT_OBJECT_0)  && (status < (WAIT_OBJECT_0 + validIndex - 1)) )
		{
			// One of the handles is signaled
			return 1; // Success
		}
		else if(status == (WAIT_OBJECT_0 + validIndex - 1))
		{
			// The StopEvent is signaled

			if (traceObj->ACS_TRA_ON())
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] StopEvent is signaled.", __FUNCTION__, __LINE__);
				traceObj->ACS_TRA_event(1,tmpStr);
			}


			return -1;

		}
		else if (status == WAIT_FAILED || WAIT_OBJECT_0)
		{
			//			DWORD lastError = GetLastError();
			if (traceObj->ACS_TRA_ON())
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] System error no.", __FUNCTION__, __LINE__);
				traceObj->ACS_TRA_event(1,tmpStr);
			}
			return -1;
		}
		else
		{
			if (traceObj->ACS_TRA_ON())
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Unknown event is signaled.", __FUNCTION__, __LINE__);
				traceObj->ACS_TRA_event(1,tmpStr);
			}
			return -1;
		}
	}

	return 0;
}

int FIXS_TREFC_JTPServer::svc()
{
	//char tmpStr[512] = {0};
	//sleep(1);
	JTP_HANDLE *Fd;
	int noOfHandles = 0;
	int dwFail = 0;

	char* applname = ((char*)"AMITRCAP");

	unsigned short systemid = ACS_JTP_Conversation::SYSTEM_ID_CP_ALARM_MASTER;

	//Open shutdown event
	m_hStopGracefully =  FIXS_TREFC_Event::OpenNamedEvent(TREFC_Util::EVENT_NAME_SHUTDOWN);

	ACS_JTP_Service *myDestinator;

	bool jtpRegistration = false;
	bool shutdown_received = false;

	while (!jtpRegistration && !shutdown_received )
	{
		myDestinator = new ACS_JTP_Service(applname);

		if (myDestinator->jidrepreq() == false)
		{
			delete(myDestinator);
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Reporting the application failed, try later" << std::endl;
			if (traceObj->ACS_TRA_ON())
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Reporting the application failed, try later.", __FUNCTION__, __LINE__);
				traceObj->ACS_TRA_event(1,tmpStr);
			}
			// Unable to initiate and register JTP name in DSD

			// Wait there for 5 seconds and try again.
			int res=FIXS_TREFC_Event::WaitForEvents(1, &m_hStopGracefully, 5000);
			if (res == 0 || res == -1)
				shutdown_received = true;
		}
		else
		{
			jtpRegistration = true;
		}
	}

	if (shutdown_received)
		return 0;

	if (traceObj->ACS_TRA_ON())
	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Executing ...........", __FUNCTION__, __LINE__);
		traceObj->ACS_TRA_event(1,tmpStr);
	}

	// Get handles for new calls

	//Get number of handles first...
	myDestinator->getHandles (noOfHandles,Fd);

	//...then actually get handles!
	JTP_HANDLE *Fd2 = new JTP_HANDLE[noOfHandles];
	memset(Fd2, 0, sizeof(JTP_HANDLE)*noOfHandles);
	myDestinator->getHandles (noOfHandles,Fd2);

	// Loop for connecting new calls
	while (!dwFail)
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Waiting for new calls..." << std::endl;

		// Wait for new calls
		if(waitUntilSignaled(noOfHandles,Fd2) == -1)
		{
			// StopEvent signaled, normal exit.
			dwFail = 0;
			if (traceObj->ACS_TRA_ON())
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] dwFail set to 0.", __FUNCTION__, __LINE__);
				traceObj->ACS_TRA_event(1,tmpStr);
			}
			break;
		}

		//Job call to be implemented.
		ACS_JTP_Job *myJob = new ACS_JTP_Job;
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "signal is received from CP..." << std::endl;
		if (traceObj->ACS_TRA_ON())
		{
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Signal is received from CP...", __FUNCTION__, __LINE__);
			traceObj->ACS_TRA_event(1,tmpStr);
		}

		if (myDestinator->accept(myJob,0) == false)
		{
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "JTP accept failed" << std::endl;
			// Unable to accept new calls. Write a message in the Eventlog.
			if (traceObj->ACS_TRA_ON())
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] New JTP Job not accepted.", __FUNCTION__, __LINE__);
				traceObj->ACS_TRA_event(1,tmpStr);
			}

			delete myJob;
		}
		else         // New calls are accepted.
		{
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "JTP accepted" << std::endl;
			FIXS_TREFC_JTPComm *jtpcomm = new FIXS_TREFC_JTPComm(systemid, myJob);
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "JTP comm object created" << std::endl;
			jtpcomm->jobMode();
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "JTP communication is done" << std::endl;
			delete jtpcomm;
			if (myJob != NULL) delete myJob;
		}

	} // End while (!dwFail)

	// Delete handles
	if (Fd2)
		delete[](Fd2);

	//Delete JTP Server
	if (myDestinator)
		delete(myDestinator);

	return dwFail;
}

void FIXS_TREFC_JTPServer::stop ()
{
	m_exit = true;
}


int FIXS_TREFC_JTPServer::open (void *args)
{
	int result = 0;
	UNUSED(args);
	m_exit = false;
	result = activate();
	cout << "FIXS_TREFC_JTPServer activate: " << result << endl;
	sleep(1);
	return result;
}

// Additional Declarations
