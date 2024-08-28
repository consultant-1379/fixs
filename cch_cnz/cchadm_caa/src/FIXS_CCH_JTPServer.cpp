/*
 * FIXS_CCH_JTPServer.cpp
 *
 *  Created on: Oct 20, 2011
 *      Author: eiolbel
 */


// FIXS_CCH_JTPServer
#include "FIXS_CCH_JTPServer.h"

//add other variable to handle the event
const int WAIT_FAILED=-1;
const int WAIT_TIMEOUT= -2;
const int WAIT_OBJECT_0=0;
const int Shutdown_Event_Index=0;
const int Command_Event_Index=1;
const int TimeoutTrap_Event_Index=2;


#ifndef __
#define __ std::cout << __FUNCTION__ << "@" << __LINE__ << std::endl;
#endif


// Class FIXS_CCH_JTPServer
FIXS_CCH_JTPServer::FIXS_CCH_JTPServer()
: m_exit(true),m_hStopGracefully(0),traceObj(NULL)
{
	traceObj = new ACS_TRA_trace("FIXS_CCH_JTPServer");

	FIXS_CCH_logging = FIXS_CCH_DiskHandler::getLogInstance();
}


FIXS_CCH_JTPServer::~FIXS_CCH_JTPServer()
{
	if (traceObj!=NULL){
		delete(traceObj);
		traceObj = NULL;
	}
}


//int FIXS_CCH_JTPServer::waitUntilSignaled (int no, HANDLE *mixedHandles)
int FIXS_CCH_JTPServer::waitUntilSignaled (int no, JTP_HANDLE *mixedHandles)
{
	// The mixedHandles array can contain a mix of valid
	// and invalid handles. Copy all valid handles to an
	// array with only valid handles.
	// The last handle is the StopEvent
	char problemText [PROBLEMDATASIZE] = {0};
	//HANDLE validHandles[33];
	JTP_HANDLE validHandles[33];
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
	//	DWORD status;
	/*
	 * questo while va eliminato?
	 */
	//	while (isRunning())
	//	{
	//int status = WaitForMultipleObjects(validIndex,validHandles,FALSE,10000);
	while (!m_exit) // Set to true when the thread shall exit
	{

		int status = FIXS_CCH_Event::WaitForEvents(validIndex,validHandles,100);

		if (status == WAIT_TIMEOUT)
		{
			//do nothing
			//just check thread status
		}
		else if ( (status >= WAIT_OBJECT_0)  && (status < (WAIT_OBJECT_0 + validIndex - 1)) )
		{
			// One of the handles is signaled
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<  std::endl;
			return 1; // Success
		}
		else if(status == (WAIT_OBJECT_0 + validIndex - 1))
		{
			if (traceObj->ACS_TRA_ON())
			{
				snprintf(problemText, sizeof(problemText) - 1, problemText);
				traceObj->ACS_TRA_event(1,problemText);
			}
			/*	// The StopEvent is signaled
			if (ACS_TRA_ON(traceJTPServer))
			{
				_snprintf(problemText, sizeof(problemText) - 1, problemText);
				ACS_TRA_event(&traceJTPServer, problemText);
			}*/
			return -1;
		}
		//else if (status == WAIT_FAILED || WAIT_ABANDONED)

		else if (status == WAIT_FAILED || WAIT_OBJECT_0)
		{
			if (traceObj->ACS_TRA_ON())
			{
				snprintf(problemText, sizeof(problemText) - 1, "System error");
				traceObj->ACS_TRA_event(1,problemText);
			}
			/*	DWORD lastError = GetLastError();
			if (ACS_TRA_ON(traceJTPServer))
			{
				_snprintf(problemText, sizeof(problemText) - 1, "System error no. =  %d " , lastError);
				ACS_TRA_event(&traceJTPServer, problemText);
			}*/
			m_exit = true;
			return -1;
		}
		else
		{
			if (traceObj->ACS_TRA_ON())
			{
				snprintf(problemText, sizeof(problemText) - 1, "Unknown event is signaled.");
				traceObj->ACS_TRA_event(1,problemText);
			}
			/*	if (ACS_TRA_ON(traceJTPServer))
			{
				_snprintf(problemText, sizeof(problemText) - 1, "Unknown event is signaled.");
				ACS_TRA_event(&traceJTPServer, problemText);
			}*/
			return -1;
		}
	}

	return 0;
}

int FIXS_CCH_JTPServer::open (void *args)
{
	int result = 0;

	UNUSED(args);
	cout << "\n JTP Thread Activate... " << endl;
	m_exit = false;
	activate();

	return result;
}


int FIXS_CCH_JTPServer::svc ()
{
	//TraceInOut inOut(__FUNCTION__);
//	char tmpStr[512] = {0};
	//HANDLE *Fd;
	JTP_HANDLE *Fd;

	int noOfHandles = 0;
	int dwFail = 0;
	char* applname = ((char*)"AMIUPGAP");

	//	unsigned short systemid = ACS_JTP_Conversation::ALARM_MASTER;
	unsigned short systemid = ACS_JTP_Conversation::SYSTEM_ID_CP_ALARM_MASTER;
	char problemData [PROBLEMDATASIZE];

	//create shutdown event
	//while (  ((m_hStopGracefully = CreateEvent(NULL, FALSE, FALSE, NULL)) == 0) && isRunning()  )

	m_hStopGracefully = FIXS_CCH_Event::OpenNamedEvent(CCH_Util::EVENT_NAME_SHUTDOWN);

    ACS_JTP_Service *myDestinator;

	bool jtpRegistration = false;
	bool shutdown_received = false;

	while (!jtpRegistration && !shutdown_received )
	{
		myDestinator = new ACS_JTP_Service(applname);

		if (!myDestinator->jidrepreq())
		{
			delete myDestinator;
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Reporting the application failed, try later" << std::endl;

			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "Reporting the application failed, try later");
				if (traceObj->ACS_TRA_ON()) traceObj->ACS_TRA_event(1,tmpStr);
			}
			// Unable to initiate and register JTP name in DSD

			// Wait there for 8 seconds and try again.
			int res=FIXS_CCH_Event::WaitForEvents(1, &m_hStopGracefully, 8000);
			if (res == 0 || res == -1)
				shutdown_received = true;
		}
		else
		{
			jtpRegistration = true;
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("JTP registration SUCCESS !!!",LOG_LEVEL_DEBUG);
		}
	}
	if (shutdown_received) return 0;

	if (traceObj->ACS_TRA_ON())
	{
		traceObj->ACS_TRA_event(1,"Executing 'JTPcomm'");
	}
	//	if (ACS_TRA_ON (traceJTPServer)) ACS_TRA_event (&traceJTPServer, "Executing 'JTPcomm'");

	//Get number of handles first...
	myDestinator->getHandles (noOfHandles,Fd);

	//...then actually get handles!
	JTP_HANDLE *Fd2 = new JTP_HANDLE[noOfHandles];
	memset(Fd2, 0, sizeof(JTP_HANDLE)*noOfHandles);
	myDestinator->getHandles (noOfHandles,Fd2);

	//ThreadPool pool(1, 70);

	// Loop for connecting new calls
	while (!dwFail)// && isRunning())
	{

		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Waiting for new calls..." << std::endl;

		// Wait for new calls
		if(waitUntilSignaled(noOfHandles,Fd2) == -1)
		{
			//
			// StopEvent signaled, normal exit.
			//
			dwFail = 0;
			if (traceObj->ACS_TRA_ON())
			{
				snprintf(problemData, sizeof(problemData) - 1, "%s","dwFail set to 0.");
				traceObj->ACS_TRA_event(1, problemData);
			}
			/*	if (ACS_TRA_ON(traceJTPServer))
			{
				_snprintf(problemData, sizeof(problemData) - 1, "%s","dwFail set to 0.");
				ACS_TRA_event(&traceJTPServer, problemData);
			}*/
			break;
		}

		ACS_JTP_Conversation *myConversation = new ACS_JTP_Conversation;

		if (myDestinator->accept(0,myConversation) == false)
		{
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "JTP accept failed" << std::endl;
			// Unable to accept new calls. Write a message in the Eventlog.
			if (traceObj->ACS_TRA_ON())
			{

				snprintf(problemData, sizeof(problemData) - 1, "%s","New JTP Conversation not accepted.");
				traceObj->ACS_TRA_event(1, problemData);
			}
			/*	if (ACS_TRA_ON(traceJTPServer))
			{
				_snprintf(problemData, sizeof(problemData) - 1, "%s","New JTP Conversation not accepted.");
				ACS_TRA_event(&traceJTPServer, problemData);
			}
			 */
			delete myConversation;
		}
		else         // New calls are accepted.
		{
			FIXS_CCH_JTPComm *jtpcomm = new FIXS_CCH_JTPComm(60, systemid, myConversation);
			jtpcomm->conversation();
			delete jtpcomm;
			if (myConversation != NULL) delete myConversation;
		}
	} // End while (!dwFail)

	// Delete handles
		if (Fd2)
			delete[](Fd2);

	if (myDestinator!=NULL){
		delete(myDestinator);
	}

	return dwFail;
}

/*
 void FIXS_CCH_JTPServer::stop ()
 {

	 if (!(m_state & (m_STATE_STOPPED | m_STATE_ABORTED))) m_state |= m_STATE_STOP_REQUESTED;

	 //	if (SetEvent(m_hStopGracefully) == FALSE)
	 if (FIXS_CCH_Event::SetEvent(m_hStopGracefully) == FALSE)
	 {
		 if (ACS_TRA_ON(traceJTPServer))
		{
			char tmpStr[512] = {0};
			_snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Error: Cannot signal shutdown event. last error code: %u", __FUNCTION__, __LINE__, GetLastError());
			ACS_TRA_event(&traceJTPServer, tmpStr);
		}
	 }


 }
 */


void FIXS_CCH_JTPServer::stop ()
{
	 m_exit = true;
	//if (!(m_state & (m_STATE_STOPPED | m_STATE_ABORTED))) m_state |= m_STATE_STOP_REQUESTED;

//	//	if (SetEvent(m_hStopGracefully) == FALSE)
//		 if (FIXS_CCH_Event::SetEvent(m_hStopGracefully) == FALSE)
//	 {
//	/* if (ACS_TRA_ON(traceJTPServer))
//			{
//				char tmpStr[512] = {0};
//				_snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Error: Cannot signal shutdown event. last error code: %u", __FUNCTION__, __LINE__, GetLastError());
//				ACS_TRA_event(&traceJTPServer, tmpStr);
//			}*/
//	if (traceObj->ACS_TRA_ON())
//	{
//		char tmpStr[512] = {0};
//		snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Error: Cannot signal shutdown event.", __FUNCTION__, __LINE__);
//		traceObj->ACS_TRA_event(1,tmpStr);
//	}
//		 }


}
// Additional Declarations
